/*
 * W25Q128JV.h
 *
 *  Created on: 12 Aug 2022
 *      Author: Dominik Kerschat
 */

#ifndef W25Q128JV_W25Q128JV_H_
#define W25Q128JV_W25Q128JV_H_
#include "stm32f4xx_hal.h"

// instructions table (standard SPI instructions)
#define SPIFLASH_WRITE_ENABLE 0x06 // write enable
#define SPIFLASH_VOLSR_ENABLE 0x50 // volatile SR write enable
#define SPIFLASH_WRITE_DISABLE 0x04 // write disable
#define SPIFLASH_RELEASE_POWERDOWN 0xAB // release power down followed by dummy bytes and the device ID
#define SPIFLASH_MFANDDEVID 0x90 // dummy bytes followed by 0x00, the manufacturer ID and device ID
#define SPIFLASH_JEDECID 0x9F // MF, JEDEC and DEVICE ID
#define SPIFLASH_UNIQUEID 0x4B // dummy bytes followed by the unique ID
#define SPIFLASH_READDATA 0x03 // read data followed by memory address bytes, flash respons with data
#define SPIFLASH_FASTREAD 0x0B // read multiple data bytes: address bytes, dummy byte, falsh respons with data until /CS is inavtice again
#define SPIFLASH_PAGEPROGRAM 0x02 // write data to a page with a starting address
#define SPIFLASH_SECTORERASE 0x20 // 4KB
#define SPIFALSH_BLOCKERASE32 0x52 // 32KB
#define SPIFLASH_BLOCKERASE64 0xD8 // 64KB
#define SPIFLASH_CHIPERASE 0xC7 // or 0x60 ?
#define SPIFLASH_READSR1 0x05 // read status register 1
#define SPIFLASH_WRITESR1 0x01 // write status register 1 (can be used to write to SR1&SR2)
#define SPIFLASH_READSR2 0x35 // read status register 2
#define SPIFLASH_WRITESR2 0x32 // write status register 2
#define SPIFLASH_READSR3 0x15 // read status register 3
#define SPIFLASH_WRITESR3 0x11 // write status register 3
#define SPIFLASH_READSFDPREG 0x5A // read SFDP register
#define SPIFLASH_ERASESECREG 0x44 // erase security register
#define SPIFLASH_PROGSECREG 0x42 // program security register
#define SPIFLASH_READSECREG 0x48 // read security register
#define SPIFLASH_GLOBALBLOCKLOCK 0x7E // lock all blocks
#define SPIFLASH_GLOBALBLOCKUNLOCK 0x98 // unlock all blocks
#define SPIFLASH_READBLOCKLOCK 0x3D // read block lock from address
#define SPIFLASH_INDIVBLOCKLOCK 0x36 // lock individual block by address
#define SPIFLASH_INDIVBLOCKUNLOCK 0x39 // unlock individual block by addess
#define SPIFLASH_ERASEPROGSUSPEND 0x75 // suspend all program or erase operations
#define SPIFLASH_ERASEPROGRESUME 0x7A // resume all program or erase operations
#define SPIFLASH_POWERDOWN 0xB9 // power down flash memory (has to be released before use again)
#define SPIFLASH_ENABLERESET 0x66 // enable a device reset
#define SPIFLASH_RESETDEVICE 0x99 // reset the device (has to be enabled)

typedef struct
{
	uint8_t isBusy; // indicates if there is currently a transfer ongoing
	GPIO_TypeDef *csPort; // the gpio port of the cs pin
	uint16_t csPin; // the cs pin
	uint8_t uniqueID[8]; // the devices unique id is stored here
	SPI_HandleTypeDef *spiHandle; // the SPI bus connected to the memory
	uint8_t dma_transfer;
	uint16_t dma_length; // the length of the last dma read operation
} FLASHMEM;

enum {flashmem_none = 0, flashmem_dmaread = 1, flashmem_dmawrite = 2};

// device management functions
uint8_t FLASHMEM_STRUCT_INIT(FLASHMEM *memory, GPIO_TypeDef *csport, uint16_t cspin, SPI_HandleTypeDef *spibus);
uint8_t FLASHMEM_READ_UNIQUEID(FLASHMEM *memory);
uint8_t FLASHMEM_POWER_UP(FLASHMEM *memory); // release the memory from power down mode
uint8_t FLASHMEM_POWER_UP_DEVID(FLASHMEM *memory, uint8_t *devid); // release the memory from power down and read device id
uint8_t FLASHMEM_POWER_DOWN(FLASHMEM *memory); // set memory to power down mode
uint8_t FLASHMEM_READ_JEDECID(FLASHMEM *memory, uint8_t *data); // read the jedec ID of the memory

// staus register managment
uint8_t FLASHMEM_READ_STATUSREG1(FLASHMEM *memory, uint8_t *data); // read the statusregister 1 content
uint8_t FLASHMEM_WRITE_STATUSREG1(FLASHMEM *memory, uint8_t settings); // write settings byte to status register
uint8_t FLASHMEM_WRITE_ENABLE(FLASHMEM *memory); // send write enable instruction
uint8_t FLASHMEM_WRITE_DISABLE(FLASHMEM *memory); // send write disable instruction
uint8_t FLASHMEM_READ_STATUSREG2(FLASHMEM *memory, uint8_t *data);
uint8_t FLASHMEM_WRITE_STATUSREG2(FLASHMEM *memory, uint8_t settings); // this register contains security register settings that can only be programmed once !!!
uint8_t FLASHMEM_READ_STATUSREG3(FLASHMEM *memory, uint8_t *data);
uint8_t FLASHMEM_WRITE_STATUSREG3(FLASHMEM *memory, uint8_t settings);
uint8_t FLASHMEM_WRITE_VOL_ENABLE(FLASHMEM *memory); // enables writing to the volatile registers without degrading the non-volatile write performance


// Data transfer functions
uint8_t FLASHMEM_READ_DATA(FLASHMEM *memory, uint8_t *address, uint8_t *data, uint16_t length);
uint8_t FLASHMEM_WRITE_DATA(FLASHMEM *memory, uint8_t *address, uint8_t *data, uint16_t length);
uint8_t FLASHMEM_ERASE_DATA(FLASHMEM *memory, uint8_t *address);
uint8_t FLASHMEM_GLOBAL_BLOCKLOCK(FLASHMEM *memory); // lock all blocks
uint8_t FLASHMEM_GLOBAL_BLOCKUNLOCK(FLASHMEM *memory); /// unlock all blocks
uint8_t FLASHMEM_INDIVIDUAL_BLOCKLOCK(FLASHMEM *memory, uint8_t *address); // lock indivudual blocks
uint8_t FLASHMEM_INDIVIDUAL_BLOCKUNLOCK(FLASHMEM *memory, uint8_t *address); // unlock individual blocks
uint8_t FLASHMEM_DMA_READ_DATA(FLASHMEM *memory, uint8_t *address, uint8_t *databuffer, uint16_t length);
uint8_t FLASHMEM_DMA_WRITE_DATA(FLASHMEM *memory, uint8_t *address, uint8_t *data, uint16_t length);

uint8_t FLASHMEM_DMA_COMPLETE(FLASHMEM *memory, uint8_t *databuffer); //DMA Complete function has to be called by the DMA TxRx Complete and Tx Compelte Callbacks

#endif /* W25Q128JV_W25Q128JV_H_ */


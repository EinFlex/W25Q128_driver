/*
 * W25Q128JV.c
 *
 *  Created on: 12 Aug 2022
 *      Author: Dominik Kerschat
 */
#include "W25Q128JV.h"
#include "stm32f4xx_hal.h"

uint8_t FLASHMEM_STRUCT_INIT(FLASHMEM *memory, GPIO_TypeDef *csport, uint16_t cspin, SPI_HandleTypeDef *spibus)
{
	memory->csPort = csport;
	memory->csPin = cspin;
	memory->spiHandle = spibus;
	memory->dma_transfer = flashmem_none;
	memory->dma_length = 0;
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, SET); // write the cs pin high (inactive)
	return 1;
}

uint8_t FLASHMEM_READ_UNIQUEID(FLASHMEM *memory)
{
	uint8_t txdata[5] = {SPIFLASH_UNIQUEID,0x00,0x00,0x00,0x00};
	// sending the unique ID command plus 4 dummy bytes
	// read unique ID directly to the memory typdef struct
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, RESET); // set cs pin active
	uint8_t rxbuffer[13];
	if (HAL_SPI_TransmitReceive(memory->spiHandle,txdata, rxbuffer,13, HAL_MAX_DELAY) == HAL_OK)
	{
		HAL_GPIO_WritePin(memory->csPort, memory->csPin, SET);
		for (int i = 0; i < 8; i++)
		{
			memory->uniqueID[i] = rxbuffer[i+5];
		}
		return 1;
	}
	else
		HAL_GPIO_WritePin(memory->csPort, memory->csPin, SET);
	return 0;
}

uint8_t FLASHMEM_POWER_UP(FLASHMEM *memory) // release the memory from power down mode
{
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, RESET);
	uint8_t txcmd = SPIFLASH_RELEASE_POWERDOWN;
	uint8_t ret = 0;
	if (HAL_SPI_Transmit(memory->spiHandle,&txcmd, 1, HAL_MAX_DELAY) == HAL_OK)
		ret = 1;
	HAL_Delay(1);
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, SET);
	return ret;
}

uint8_t FLASHMEM_POWER_UP_DEVID(FLASHMEM * memory, uint8_t *devid)
{
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, RESET);
	uint8_t txcmd[4] = {SPIFLASH_RELEASE_POWERDOWN, 0x00,0x00,0x00};
	uint8_t ret = 0;
	uint8_t rxbuffer[5];
	if (HAL_SPI_TransmitReceive(memory->spiHandle, txcmd, rxbuffer, 5, HAL_MAX_DELAY) == HAL_OK)
		ret = 1;
	HAL_Delay(1);
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, SET);
	devid[0] = rxbuffer[4];
	return ret;
}

uint8_t FLASHMEM_POWER_DOWN(FLASHMEM *memory) // set memory to power down mode
{
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, RESET);
	uint8_t txcmd = SPIFLASH_POWERDOWN;
	uint8_t ret = 0;
	if(HAL_SPI_Transmit(memory->spiHandle,&txcmd, 1, HAL_MAX_DELAY) == HAL_OK)
		ret = 1;
	HAL_Delay(1);
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, SET);
	return ret;
}

uint8_t FLASHMEM_READ_JEDECID(FLASHMEM *memory, uint8_t *data)
{
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, RESET);
	uint8_t txcmd = SPIFLASH_JEDECID;
	uint8_t ret = 0;
	uint8_t rxbuffer[4];
	if (HAL_SPI_TransmitReceive(memory->spiHandle, &txcmd, rxbuffer, 4, HAL_MAX_DELAY) == HAL_OK)
		ret = 1;
	HAL_Delay(1);
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, SET);
	for (int i = 0; i < 3; i++)
	{
		data[i] = rxbuffer[i+1];
	}
	return ret;
}

uint8_t FLASHMEM_READ_STATUSREG1(FLASHMEM *memory, uint8_t *data) // read the statusregister 1 content
{
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, RESET);
	uint8_t txcmd = SPIFLASH_READSR1;
	uint8_t ret = 0;
	uint8_t rxbuffer[2];
	if (HAL_SPI_TransmitReceive(memory->spiHandle, &txcmd, rxbuffer, 2, HAL_MAX_DELAY) == HAL_OK)
		ret = 1;
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, SET);
	data[0] = rxbuffer[1];
	return ret;
}

uint8_t FLASHMEM_WRITE_STATUSREG1(FLASHMEM *memory, uint8_t settings) // write settings byte to status register
{
	uint8_t ret = FLASHMEM_WRITE_ENABLE(memory); // enable write operations / disable is done atomatically
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, RESET);
	// writeable bits in SR1: SEC, TB, BP [6:2]
	// SEC: 0 -> protect 64KB blocks, 1 -> protect 4KB Sectors
	// TB: 0 -> protect blocks from TOP, 1 -> protect blocks form bottom
	// BP2,BP1,BP0: Define whic blocks to protect (see Memory Protection Table)
	settings &= 0x7C; // only use the bits that can be written
	uint8_t txcmd[2] = {SPIFLASH_WRITESR1, settings};
	if (HAL_SPI_Transmit(memory->spiHandle, txcmd, 2, HAL_MAX_DELAY) == HAL_OK)
		ret = 1;
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, SET);
	return ret;
}

uint8_t FLASHMEM_WRITE_ENABLE(FLASHMEM *memory) // send write enable instruction
{
	uint8_t ret = 0;
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, RESET);
	uint8_t txcmd = SPIFLASH_WRITE_ENABLE;
	if (HAL_SPI_Transmit(memory->spiHandle, &txcmd, 1, HAL_MAX_DELAY) == HAL_OK)
		ret = 1;
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, SET);
	return ret;
}

uint8_t FLASHMEM_WRITE_DISABLE(FLASHMEM *memory)
{
	uint8_t ret = 0;
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, RESET);
	uint8_t txcmd = SPIFLASH_WRITE_DISABLE;
	if (HAL_SPI_Transmit(memory->spiHandle, &txcmd, 1, HAL_MAX_DELAY) == HAL_OK)
		ret = 1;
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, SET);
	return ret;
}

uint8_t FLASHMEM_WRITE_VOL_ENABLE(FLASHMEM *memory) // enables writing to the volatile status registers
{
	uint8_t ret = 0;
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, RESET);
	uint8_t txcmd = SPIFLASH_VOLSR_ENABLE;
	if (HAL_SPI_Transmit(memory->spiHandle, &txcmd, 1, HAL_MAX_DELAY) == HAL_OK)
		ret = 1;
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, SET);
	return ret;
}

uint8_t FLASHMEM_READ_STATUSREG2(FLASHMEM *memory, uint8_t *data)
{
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, RESET);
	uint8_t txcmd = SPIFLASH_READSR2;
	uint8_t ret = 0;
	uint8_t rxbuffer[2];
	if (HAL_SPI_TransmitReceive(memory->spiHandle, &txcmd, rxbuffer, 2, HAL_MAX_DELAY) == HAL_OK)
		ret = 1;
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, SET);
	data[0] = rxbuffer[1];
	return ret;
}

uint8_t FLASHMEM_WRITE_STATUSREG2(FLASHMEM *memory, uint8_t settings)
{
	uint8_t ret = FLASHMEM_WRITE_ENABLE(memory); // enable write operations
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, RESET);
	// writeable bits in SR2:
	// CMP:
	// QE:
	// LB3,LB2,LB1
	// SRL:
	//settings &= 0x7B; // only use the bits that can be written
	settings &= 0x43; // no security bits can be overwritten @WIP for DEV only
	uint8_t txcmd[2] = {SPIFLASH_WRITESR2, settings};
	if (HAL_SPI_Transmit(memory->spiHandle, txcmd, 2, HAL_MAX_DELAY) == HAL_OK)
		ret = 1;
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, SET);
	return ret;
}

uint8_t FLASHMEM_READ_STATUSREG3(FLASHMEM *memory, uint8_t *data)
{
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, RESET);
	uint8_t txcmd = SPIFLASH_READSR3;
	uint8_t ret = 0;
	uint8_t rxbuffer[2];
	if (HAL_SPI_TransmitReceive(memory->spiHandle, &txcmd, rxbuffer, 2, HAL_MAX_DELAY) == HAL_OK)
		ret = 1;
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, SET);
	data[0] = rxbuffer[1];
	return ret;
}

uint8_t FLASHMEM_WRITE_STATUSREG3(FLASHMEM *memory, uint8_t settings)
{
	uint8_t ret = FLASHMEM_WRITE_ENABLE(memory); // enable write operations
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, RESET);
	// writeable bits in SR3:
	// HOLD/RST:
	// DRV1:
	// DRV0,LB2,LB1:
	// WPS:
	settings &= 0xE4; // no security bits can be overwritten @WIP for DEV only
	uint8_t txcmd[2] = {SPIFLASH_WRITESR3, settings};
	if (HAL_SPI_Transmit(memory->spiHandle, txcmd, 2, HAL_MAX_DELAY) == HAL_OK)
		ret = 1;
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, SET);
	return ret;
}

uint8_t FLASHMEM_READ_DATA(FLASHMEM *memory, uint8_t *address, uint8_t *data, uint16_t length)
{
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, RESET);
	uint8_t txcmd[4] = {SPIFLASH_READDATA, address[0],address[1], address[2]};
	uint8_t ret = 0;
	uint8_t rxbuffer[4+length];
	if (HAL_SPI_TransmitReceive(memory->spiHandle, txcmd, rxbuffer, (4 + length), HAL_MAX_DELAY) == HAL_OK)
		ret = 1;
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, SET);
	for(int i = 0; i < length; i++)
	{
		data[i] = rxbuffer[i+4];
	}
	return ret;
}

uint8_t FLASHMEM_WRITE_DATA(FLASHMEM *memory, uint8_t *address, uint8_t *data, uint16_t length)
{
	FLASHMEM_WRITE_ENABLE(memory);
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, RESET);
	uint8_t txcmd[4+length];
	txcmd[0] = SPIFLASH_PAGEPROGRAM, txcmd[1] = address[0], txcmd[2] = address[1], txcmd[3] = address[2];
	for (int i = 0; i < length; i++)
	{
		txcmd[i+4] = data[i];
	}
	uint8_t ret = 0;
	if (HAL_SPI_Transmit(memory->spiHandle, txcmd, (4 + length), HAL_MAX_DELAY) == HAL_OK)
		ret = 1;
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, SET);
	return ret;
}

uint8_t FLASHMEM_ERASE_SECTOR(FLASHMEM *memory, uint8_t *address)
{
	uint8_t ret = FLASHMEM_WRITE_ENABLE(memory); // enable write operations
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, RESET);
	uint8_t txcmd[4];
		txcmd[0] = SPIFLASH_SECTORERASE, txcmd[1] = address[0], txcmd[2] = address[1], txcmd[3] = address[2];
	if (HAL_SPI_Transmit(memory->spiHandle, txcmd, 2, HAL_MAX_DELAY) == HAL_OK)
		ret = 1;
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, SET);
	return ret;
}

uint8_t FLASHMEM_GLOBAL_BLOCKLOCK(FLASHMEM *memory) // lock all blocks
{
	uint8_t ret = 0;
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, RESET);
	uint8_t txcmd = SPIFLASH_GLOBALBLOCKLOCK;
	if (HAL_SPI_Transmit(memory->spiHandle, &txcmd, 1, HAL_MAX_DELAY) == HAL_OK)
		ret = 1;
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, SET);
	return ret;
}

uint8_t FLASHMEM_GLOBAL_BLOCKUNLOCK(FLASHMEM *memory) /// unlock all blocks
{
	uint8_t ret = 0;
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, RESET);
	uint8_t txcmd = SPIFLASH_GLOBALBLOCKUNLOCK;
	if (HAL_SPI_Transmit(memory->spiHandle, &txcmd, 1, HAL_MAX_DELAY) == HAL_OK)
		ret = 1;
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, SET);
	return ret;
}

uint8_t FLASHMEM_INDIVIDUAL_BLOCKLOCK(FLASHMEM *memory, uint8_t *address) // lock indivudual blocks
{
	uint8_t ret = 0;
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, RESET);
	uint8_t txcmd[4] = {SPIFLASH_INDIVBLOCKLOCK, address[0],address[1],address[2]};
	if (HAL_SPI_Transmit(memory->spiHandle, txcmd, 4, HAL_MAX_DELAY) == HAL_OK)
		ret = 1;
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, SET);
	return ret;
}

uint8_t FLASHMEM_INDIVIDUAL_BLOCKUNLOCK(FLASHMEM *memory, uint8_t *address) // unlock individual blocks
{
	uint8_t ret = 0;
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, RESET);
	uint8_t txcmd[4] = {SPIFLASH_INDIVBLOCKUNLOCK, address[0],address[1],address[2]};
	if (HAL_SPI_Transmit(memory->spiHandle, txcmd, 4, HAL_MAX_DELAY) == HAL_OK)
		ret = 1;
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, SET);
	return ret;
}

uint8_t FLASHMEM_DMA_READ_DATA(FLASHMEM *memory, uint8_t *address, uint8_t *databuffer, uint16_t length)
{
	uint8_t ret = 0;
	if (memory->dma_transfer == flashmem_none)
	{
		memory->dma_length = length;
		HAL_GPIO_WritePin(memory->csPort, memory->csPin, RESET);
		uint8_t txcmd[4] = {SPIFLASH_READDATA, address[0],address[1], address[2]};
		memory->dma_transfer = flashmem_dmaread;
		if (HAL_SPI_TransmitReceive_DMA(memory->spiHandle, txcmd, databuffer, 4+length) == HAL_OK)
			ret = 1;
	}
	return ret;
}

uint8_t FLASHMEM_DMA_WRITE_DATA(FLASHMEM *memory, uint8_t *address, uint8_t *data, uint16_t length)
{
	uint8_t ret = 0;
	if (memory->dma_transfer == flashmem_none)
	{
		HAL_GPIO_WritePin(memory->csPort, memory->csPin, RESET);
		uint8_t txcmd[4+length];
		txcmd[0] = SPIFLASH_PAGEPROGRAM, txcmd[1] = address[0], txcmd[2] = address[1], txcmd[3] = address[2];
		memory->dma_transfer = flashmem_dmawrite;
		for (int i = 0; i < length; i++)
		{
			txcmd[i+4] = data[i];
		}
		if (HAL_SPI_Transmit_DMA(memory->spiHandle, txcmd, 4+length) == HAL_OK)
			ret = 1;
	}
	return ret;
}

uint8_t FLASHMEM_DMA_COMPLETE(FLASHMEM *memory, uint8_t *databuffer)
{
	HAL_GPIO_WritePin(memory->csPort, memory->csPin, SET);
	if (memory->dma_transfer == flashmem_dmaread)
	{
		for(int i = 0; i < memory->dma_length; i++)
		{
			databuffer[i] = databuffer[i+4];
		}
	}
	memory->dma_transfer = flashmem_none;
	memory->dma_length = 0;
	return 1;
}

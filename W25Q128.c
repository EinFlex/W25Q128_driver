#include "W25Q128.h"

// Initialize the Flash Memory, check WHO AM I
void W25_Flash_Init(W25Q128_FLASH_TypeDef * hflash)
{
    // to get Manuf/Device ID
    // 90h, Dummy x2, 00h, MF Byte, ID Byte
    uint8_t datatx[] = {0x90, 0x00,0x00,0x00,0x00,0x00};
    uint8_t datarx[] = {0x00,0x00};
    HAL_GPIO_WritePin(hflash->CS_pin, GPIO_Set);
    if (HAL_SPI_TransmitReceive(hflash->hspi, datatx, datarx, 6, 100) != HAL_OK)
    {
        HAL_GPIO_WritePin(hflash->CS_pin, GPIO_Reset);
        hflash->status=W25_NOK;
        return;
    }
    if (datarx[0] == 0xEF && datarx[1] == 0x17)
    {
        // Flash confirmed
        // to read unique ID
        // 4Bh, Dummyx4, UID63-0
        datatx = {0x4B, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
        datarx = uint8_t[8];
        if (HAL_SPI_TransmitReceive(hflash->hspi, datatx, datarx,13,100) != HAL_OK)
        {
            HAL_GPIO_WritePin(hflash->CS_pin, GPIO_Reset);
            hflash->status=W25_NOK;
            return;
        }
        else
        {
            HAL_GPIO_WritePin(hflash->CS_pin, GPIO_Reset);
            hflash->uniqueID = datarx;
            hflas->status=W25_InitComplete;
        }
    }
    else
    {
        HAL_GPIO_WritePin(hflash->CS_pin, GPIO_Reset);
        hflash->status=W25_NOK;
        return;
    }
}

void W25_Flash_ReadStatus(W25Q128_FLASH_TypeDef * hflash)
{
    // read all staus registers
    HAL_GPIO_WritePin(hflash->CS_pin, GPIO_Set);
    uint8_t command = 0x05;
    uint8_t[8] status1;
    uint8_t[8] status2;
    uint8_t[8] status3;
    HAL_SPI_TransmitReceive(hflash->hspi, command, status1, 1, 100);
    command = 0x35;
    HAL_SPI_TransmitReceive(hflash->hspi, command, status2, 1, 100);
    command = 0x15;
    HAL_SPI_TransmitReceive(hflash->hspi, command, status3, 1, 100);
    HAL_GPIO_WritePin(hflash->CS_pin, GPIO_Reset);
    // WIP populate flash typedef with status register info
}
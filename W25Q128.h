#ifndef W25Q128
#define W25Q128

typedef struct{
    SPI_HandleTypeDef * hspi;
    uint16_t CS_pin;
    uint16_t WP_pin; // IO2
    uint16_t HR_pin; // IO3 | Hold or Reset Pin (active low)
    uint8_t enableQSpi; // weather Quad SPI should be enabled or disabled on the next Init
    uint8_t [] uniqueID; // array to store the unique id
    W25_StatusTypeDef status;
}W25Q128_FLASH_TypeDef;

typedef enum{
    W25_InitComplete;
    W25_SWProtected; // when SW write protection is active
    W25_HWProtected; // when HW write protection is active (SW protection could also be active)
    W25_NOK; // any failure
    W25_Writing; // when a erase/write is in progress
    W25_WEL; // when the Write enable latch is active but no write is in progress
}W25_StatusTypeDef;

void W25_Flash_Init(W25Q128_FLASH_TypeDef * hflash);
void W25_Flash_ReadStatus(W25Q128_FLASH_TypeDef * hflash);
#endif

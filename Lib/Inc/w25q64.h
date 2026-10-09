#ifndef W25Q64_H
#define W25Q64_H

#include "stm32f1xx_hal.h"

#define W25Q64_CS_ENABLE  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
#define W25Q64_CS_DISABLE HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

void W25Q64_Init(void);
void W25Q64_Wait_Busy(void);
void W25Q64_Write_Enable(void);
void W25Q64_Erase_64K(uint8_t block);
void W25Q64_Write_Page(uint8_t* data, uint16_t page, uint16_t length);
void W25Q64_Write(uint8_t* data, uint32_t address, uint32_t length);
void W25Q64_Read_Data(uint8_t* buffer, uint32_t address, uint16_t length);

#endif

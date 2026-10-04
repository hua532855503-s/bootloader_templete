#ifndef AT24C02_H
#define AT24C02_H

#include "stm32f1xx_hal.h"

#define AT24C02_WADDR 0xA0
#define AT24C02_RADDR 0xA1

uint8_t AT24C02_Write_Byte(uint8_t addr, uint8_t data);
uint8_t AT24C02_Write_Page(uint8_t addr, uint8_t *data);
uint8_t AT24C02_Read_Date(uint8_t addr, uint8_t *data, uint16_t len);
void AT24C02_Read_OTASetting(void);
void AT24C02_Write_OTASetting(void);

#endif

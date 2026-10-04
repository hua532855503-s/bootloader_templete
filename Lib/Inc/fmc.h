#ifndef FMC_H
#define FMC_H

#include "stm32f1xx_hal.h"

void STM32_Erase_Flash(uint16_t start, uint16_t num);
void STM32_Write_Flash(uint32_t addr, uint32_t *data, uint32_t num);

#endif

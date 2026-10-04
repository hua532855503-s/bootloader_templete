#ifndef DELAY_H
#define DELAY_H

#include "stm32f1xx_hal.h"

void DWT_Init(void);
void Delay_us(uint32_t us);
void Delay_ms(uint32_t ms);

#endif

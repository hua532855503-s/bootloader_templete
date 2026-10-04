#ifndef BOOT_H
#define BOOT_H

#include "stm32f1xx_hal.h"

typedef void (*load_a)(void);

void Bootloader_Branch(void);
void LOAD_A(uint32_t addr);
void Bootloader_Clear(void);

#endif
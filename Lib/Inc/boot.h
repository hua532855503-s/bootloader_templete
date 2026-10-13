#ifndef BOOT_H
#define BOOT_H

#include "stm32f1xx_hal.h"

typedef void (*load_a)(void);

void Xmodem_Send_Cmd(uint8_t cmd);
void Bootloader_Branch(void);
void LOAD_A(uint32_t addr);
void Bootloader_Clear(void);
uint8_t Bootloader_Enter(uint8_t timeout);
void Bootloader_Info(void);
void Bootloader_Event(uint8_t *event, uint16_t len);
void Bootloader_Process_Byte(uint8_t byte);

#endif
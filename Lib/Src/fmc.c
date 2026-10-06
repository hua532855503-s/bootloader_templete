#include "fmc.h"
#include <stdio.h>
#include <string.h>

void STM32_Erase_Flash(uint16_t start, uint16_t num)
{
  if (start + num > 64) return;// 1. 地址范围校验（C8T6 共 64 页，0~63）

  HAL_FLASH_Unlock();

  FLASH_EraseInitTypeDef eraseInitStruct;
  eraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  eraseInitStruct.PageAddress = 0x08000000 + (start * 1024); // 起始页首地址
  eraseInitStruct.NbPages     = num;                         // 一次擦 num 页

  uint32_t pageError = 0;
  if (HAL_FLASHEx_Erase(&eraseInitStruct, &pageError) != HAL_OK)
  {
    printf("Flash Erase Error: %lu\r\n", pageError);
  }

  HAL_FLASH_Lock();
}

void STM32_Write_Flash(uint32_t addr, uint32_t *data, uint32_t num)
{
  HAL_FLASH_Unlock();

  uint8_t *bytes = (uint8_t *)data;
  while (num >= 4)
  {
    uint32_t word;
    memcpy(&word, bytes, sizeof(word));
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, word);
    num -= 4;
    addr += 4;
    bytes += 4;
  }

  if (num != 0)
  {
    uint32_t last_word = 0xFFFFFFFF;
    memcpy(&last_word, bytes, num);
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, last_word);
  }

  HAL_FLASH_Lock();
}

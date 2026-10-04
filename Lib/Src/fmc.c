#include "fmc.h"

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
      // pageError 保存出错的页地址（成功为 0xFFFFFFFF）
  }

  HAL_FLASH_Lock();
}

void STM32_Write_Flash(uint32_t addr, uint32_t *data, uint32_t num)
{
  HAL_FLASH_Unlock();

  while (num)
  {
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, *data);
    num   -=  4;
    addr  +=  4;
    data  ++;
  }

  HAL_FLASH_Lock();
}

#include "delay.h"

void DWT_Init(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void Delay_us(uint32_t us)
{
  uint32_t start = DWT->CYCCNT;
  uint32_t ticks = us * (SystemCoreClock / 1000000); // 72MHz下即 us * 72
  while ((DWT->CYCCNT - start) < ticks);
}

void Delay_ms(uint32_t ms)
{
  while (ms--)
  {
    Delay_us(1000);
  }
}

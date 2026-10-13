#include "delay.h"

void DWT_Init(void)
{
  /* 开启 Cortex-M3 的 DWT 周期计数器，用于实现微秒级延时。 */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void Delay_us(uint32_t us)
{
  /* 根据当前系统主频换算需要等待的 CPU 周期数。 */
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

#include "w25q64.h"
#include "spi.h"

void W25Q64_Init(void)
{
  /* SPI 片选默认置高，确保外部 Flash 处于未选中状态。 */
  W25Q64_CS_DISABLE;
}

void W25Q64_Wait_Busy(void)
{
  /* 轮询状态寄存器 BUSY 位，等待擦除或写入操作结束。 */
  uint8_t status;
  do
  {
    W25Q64_CS_ENABLE;
    /* 0x05：读取状态寄存器 1，bit0 为 BUSY 忙状态标志。 */
    uint8_t cmd = 0x05;
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi1, &status, 1, HAL_MAX_DELAY);
    W25Q64_CS_DISABLE;
  } while (status & 0x01);
}

void W25Q64_Write_Enable(void)
{
  /* 每次擦除或页编程前都必须发送写使能指令。 */
  W25Q64_CS_ENABLE;
  /* 0x06：写使能，设置状态寄存器中的 WEL 位。 */
  uint8_t cmd = 0x06;
  HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
  W25Q64_CS_DISABLE;
}

void W25Q64_Erase_64K(uint8_t block)
{
  /* 擦除指定的 64 KB 块，地址由块号换算得到。 */
  uint8_t cmd[4];
  /* 0xD8：64 KB 块擦除，后跟 24 位起始地址。 */
  cmd[0] = 0xD8;
  cmd[1] = (block*64*1024 >> 16) & 0xFF;
  cmd[2] = (block*64*1024 >> 8) & 0xFF;
  cmd[3] = (block*64*1024) & 0xFF;

  W25Q64_Write_Enable();
  W25Q64_CS_ENABLE;
  HAL_SPI_Transmit(&hspi1, cmd, 4, HAL_MAX_DELAY);
  W25Q64_CS_DISABLE;
  W25Q64_Wait_Busy();
}

void W25Q64_Write_Page(uint8_t* data, uint16_t page, uint16_t length)
{
  /* 向单个 256 字节页写入数据，调用者需保证不跨页。 */
  if (length > 256) return;

  uint8_t cmd[4];
  /* 0x02：页编程，后跟 24 位地址和最多 256 字节数据。 */
  cmd[0] = 0x02;
  cmd[1] = (page*256 >> 16) & 0xFF;
  cmd[2] = (page*256 >> 8) & 0xFF;
  cmd[3] = (page*256) & 0xFF;

  W25Q64_Write_Enable();
  W25Q64_CS_ENABLE;
  HAL_SPI_Transmit(&hspi1, cmd, 4, HAL_MAX_DELAY);
  HAL_SPI_Transmit(&hspi1, data, length, HAL_MAX_DELAY);
  W25Q64_CS_DISABLE;
  W25Q64_Wait_Busy();
}

void W25Q64_Write(uint8_t* data, uint32_t address, uint32_t length)
{
  /* 自动按页边界拆分写入，避免一次编程跨越 W25Q64 页边界。 */
  while (length > 0)
  {
    uint32_t page_offset = address & 0xFF;
    uint32_t write_length = 256U - page_offset;

    if (write_length > length)
    {
      write_length = length;
    }

    uint8_t cmd[4];
    /* 0x02：页编程；按页边界拆分后发送本次数据。 */
    cmd[0] = 0x02;
    cmd[1] = (address >> 16) & 0xFF;
    cmd[2] = (address >> 8) & 0xFF;
    cmd[3] = address & 0xFF;

    W25Q64_Write_Enable();
    W25Q64_CS_ENABLE;
    HAL_SPI_Transmit(&hspi1, cmd, 4, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&hspi1, data, (uint16_t)write_length, HAL_MAX_DELAY);
    W25Q64_CS_DISABLE;
    W25Q64_Wait_Busy();

    address += write_length;
    data += write_length;
    length -= write_length;
  }
}

void W25Q64_Read_Data(uint8_t* buffer, uint32_t address, uint16_t length)
{
  /* 从指定地址连续读取数据。 */
  uint8_t cmd[4];
  /* 0x03：普通数据读取，后跟 24 位地址，再连续读取数据。 */
  cmd[0] = 0x03;
  cmd[1] = (address >> 16) & 0xFF;
  cmd[2] = (address >> 8) & 0xFF;
  cmd[3] = address & 0xFF;

  W25Q64_CS_ENABLE;
  HAL_SPI_Transmit(&hspi1, cmd, 4, HAL_MAX_DELAY);
  HAL_SPI_Receive(&hspi1, buffer, length, HAL_MAX_DELAY);
  W25Q64_CS_DISABLE;
}

#include "w25q64.h"
#include "spi.h"

void W25Q64_Init(void)
{
  W25Q64_CS_DISABLE;
}

void W25Q64_Wait_Busy(void)
{
  uint8_t status;
  do
  {
    W25Q64_CS_ENABLE;
    uint8_t cmd = 0x05; // Read Status Register command
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi1, &status, 1, HAL_MAX_DELAY);
    W25Q64_CS_DISABLE;
  } while (status & 0x01); // Check if the busy bit is set
}

void W25Q64_Write_Enable(void)
{
  W25Q64_CS_ENABLE;
  uint8_t cmd = 0x06; // Write Enable command
  HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
  W25Q64_CS_DISABLE;
}

void W25Q64_Erase_64K(uint8_t block)
{
  uint8_t cmd[4];
  cmd[0] = 0xD8; // Erase 64K command
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
  if (length > 256) return; // Page size is 256 bytes

  uint8_t cmd[4];
  cmd[0] = 0x02; // Page Program command
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
  while (length > 0)
  {
    uint32_t page_offset = address & 0xFFU;
    uint32_t write_length = 256U - page_offset;

    if (write_length > length)
    {
      write_length = length;
    }

    uint8_t cmd[4];
    cmd[0] = 0x02; // Page Program command
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
  uint8_t cmd[4];
  cmd[0] = 0x03; // Read Data command
  cmd[1] = (address >> 16) & 0xFF;
  cmd[2] = (address >> 8) & 0xFF;
  cmd[3] = address & 0xFF;

  W25Q64_CS_ENABLE;
  HAL_SPI_Transmit(&hspi1, cmd, 4, HAL_MAX_DELAY);
  HAL_SPI_Receive(&hspi1, buffer, length, HAL_MAX_DELAY);
  W25Q64_CS_DISABLE;
}

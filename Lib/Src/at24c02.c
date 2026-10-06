#include "at24c02.h"
#include "iic.h"
#include "main.h"
#include "delay.h"

#include <string.h>

uint8_t AT24C02_Write_Byte(uint8_t addr, uint8_t data)
{
  IIC_Start();
  IIC_Send_Byte(AT24C02_WADDR);
  if (IIC_Wait_Ack()) return -1;
  IIC_Send_Byte(addr);
  if (IIC_Wait_Ack()) return -1;
  IIC_Send_Byte(data);
  if (IIC_Wait_Ack()) return -1;
  IIC_Stop();
  return 0;
}

uint8_t AT24C02_Write_Page(uint8_t addr, uint8_t *data)
{
  IIC_Start();
  IIC_Send_Byte(AT24C02_WADDR);
  if (IIC_Wait_Ack()) return -1;
  IIC_Send_Byte(addr);
  if (IIC_Wait_Ack()) return -1;
  for (uint8_t i = 0; i < 8; i++)
  {
    IIC_Send_Byte(data[i]);
    if (IIC_Wait_Ack()) return -1;
  }
  IIC_Stop();
  return 0;
}

uint8_t AT24C02_Read_Date(uint8_t addr, uint8_t *data, uint16_t len)
{
  if (len > 256) return -1; // AT24C02 max size is 256 bytes
  IIC_Start();
  IIC_Send_Byte(AT24C02_WADDR);
  if (IIC_Wait_Ack()) return -1;
  IIC_Send_Byte(addr);
  if (IIC_Wait_Ack()) return -1;
  IIC_Start();
  IIC_Send_Byte(AT24C02_RADDR);
  if (IIC_Wait_Ack()) return -1;
  for (uint16_t i = 0; i < len; i++)
  {
    data[i] = IIC_Read_Byte(i == len - 1 ? 0 : 1);
  }
  IIC_Stop();
  return 0;
}

void AT24C02_Read_OTASetting(void)
{
  memset(&OTA_Info, 0, OTA_SETTING_SIZE);
  AT24C02_Read_Date(0, (uint8_t *)&OTA_Info, OTA_SETTING_SIZE);
}

void AT24C02_Write_OTASetting(void)
{
  for (uint8_t i = 0; i < OTA_SETTING_SIZE; i += 8)
  {
    AT24C02_Write_Page(i, ((uint8_t *)&OTA_Info) + i);
    Delay_ms(5); // Wait for EEPROM write cycle
  }
}

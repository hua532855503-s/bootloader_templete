#include "iic.h"
#include "delay.h"
#include <stdint.h>

void IIC_Init(void)
{
  IIC_SCL_H;
  IIC_SDA_H;
}

void IIC_Start(void)
{
  IIC_SDA_H;
  IIC_SCL_H;
  Delay_us(2);
  IIC_SDA_L;
  Delay_us(2);
  IIC_SCL_L;
  Delay_us(2);
}

void IIC_Stop(void)
{
  IIC_SDA_L;
  IIC_SCL_H;
  Delay_us(2);
  IIC_SDA_H;
}

void IIC_Send_Byte(uint8_t byte)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (byte & 0x80)
    {
      IIC_SDA_H;
    }
    else
    {
      IIC_SDA_L;
    }

    Delay_us(2);
    IIC_SCL_H;
    Delay_us(2);
    IIC_SCL_L;

    if (i == 7)
    {
      IIC_SDA_H;
    }
    
    byte <<= 1;
    Delay_us(2);
  }
}

uint8_t IIC_Read_Byte(uint16_t ack)
{
  uint8_t byte = 0;

  for (uint8_t i = 0; i < 8; i++)
  {
    byte <<= 1;
    IIC_SCL_H;
    Delay_us(2);
    if (IIC_SDA_READ)
    {
      byte |= 0x01;
    }
    IIC_SCL_L;
    Delay_us(2);
  }

  if (ack)
  {
    IIC_Ack();
  }
  else
  {
    IIC_NAck();
  }

  return byte;
}

uint8_t IIC_Wait_Ack(void)
{
  IIC_SDA_H;
  Delay_us(2);
  IIC_SCL_H;
  Delay_us(2);
  uint8_t ack = IIC_SDA_READ;
  IIC_SCL_L;
  Delay_us(2);
  return ack;
}

void IIC_Ack(void)
{
  IIC_SDA_L;
  Delay_us(2);
  IIC_SCL_H;
  Delay_us(2);
  IIC_SCL_L;
  Delay_us(2);
  IIC_SDA_H;
}

void IIC_NAck(void)
{
  IIC_SDA_H;
  Delay_us(2);
  IIC_SCL_H;
  Delay_us(2);
  IIC_SCL_L;
  Delay_us(2);
}

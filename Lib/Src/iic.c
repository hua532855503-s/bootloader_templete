#include "iic.h"
#include "delay.h"

#include <stdint.h>

void IIC_Init(void)
{
  /* 软件 I2C 空闲状态为 SCL、SDA 同时释放为高电平。 */
  IIC_SCL_H;
  IIC_SDA_H;
}

void IIC_Start(void)
{
  /* 产生 I2C 起始条件：SCL 为高时将 SDA 从高拉低。 */
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
  /* 产生 I2C 停止条件：SCL 为高时将 SDA 从低释放为高。 */
  IIC_SDA_L;
  IIC_SCL_H;
  Delay_us(2);
  IIC_SDA_H;
}

void IIC_Send_Byte(uint8_t byte)
{
  /* 高位在先发送一个字节，最后一个时钟后释放 SDA 以接收 ACK。 */
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

  /* 连续读取 8 位数据，读取完成后由主机发送 ACK 或 NACK。 */
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
  /* 释放 SDA 后读取从机在第 9 个时钟上的应答电平。 */
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
  /* 主机通过拉低 SDA 向从机确认已收到数据。 */
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
  /* 主机保持 SDA 为高，表示结束本次读取。 */
  IIC_SDA_H;
  Delay_us(2);
  IIC_SCL_H;
  Delay_us(2);
  IIC_SCL_L;
  Delay_us(2);
}

#ifndef IIC1_H
#define IIC1_H

#include "stm32f1xx_hal.h"

#define IIC_SCL_H HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET)
#define IIC_SCL_L HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET)

#define IIC_SDA_H HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET)
#define IIC_SDA_L HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET)

#define IIC_SDA_READ HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6)

void IIC_Init(void);
void IIC_Start(void);
void IIC_Stop(void);
void IIC_Send_Byte(uint8_t byte);
void IIC_Send_Ack(uint8_t ack);
uint8_t IIC_Read_Byte(uint16_t ack);
uint8_t IIC_Wait_Ack(void);
void IIC_Ack(void);
void IIC_NAck(void);

#endif

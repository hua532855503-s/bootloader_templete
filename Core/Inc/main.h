/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef struct {
  uint32_t OTA_Flag;       /* OTA 请求标志。 */
  uint32_t File_Size[11];  /* 外部 Flash 各文件块的有效长度。 */
  uint8_t OTA_Ver[32];     /* OTA 版本字符串。 */
} OTA_Setting;

typedef struct {
  uint8_t Update_buf[FLASH_PAGE_SIZE]; /* Xmodem 页缓存。 */
  uint32_t W25Q64_Block;               /* 当前使用的外部 Flash 块号。 */
  uint32_t Xmodem_Timer;               /* Xmodem 握手计时器。 */
  uint32_t Xmodem_Num;                 /* 已接收的数据包数量。 */
  uint32_t Xmodem_CRC;                 /* 最近一次数据包的 CRC。 */
} Update_Setting;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
extern OTA_Setting OTA_Info;
extern Update_Setting Update_Info;
extern uint32_t Boot_Flag;
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */
#define FLASH_START_ADDR              0x08000000
#define FLASH_PAGE_NUMBER             64
#define FLASH_PAGEB_NUMBER            24
#define FLASH_PAGEA_NUMBER            (FLASH_PAGE_NUMBER - FLASH_PAGEB_NUMBER)
/* B 区存放 Bootloader，A 区存放待启动的用户应用。 */
#define FLASH_PAGEA_START_ADDR        (FLASH_START_ADDR + FLASH_PAGEB_NUMBER * FLASH_PAGE_SIZE)

#define OTA_SET_FLAG                  0xAABB1122
#define OTA_SETTING_SIZE              sizeof(OTA_Setting)
#define OTA_VERSION_INPUT_LEN         26
#define OTA_INIT_VERSION              "VER-0.0.0-0000/00/00-00:00"

#define XMODEM_ACK                    0x06
#define XMODEM_NAK                    0x15
#define XMODEM_CAN                    0x18
#define XMODEM_DATA_SIZE              128U
#define XMODEM_FRAME_SIZE             133U

/* Boot_Flag 的各状态位，允许多个标志同时生效。 */
#define UPDATEA_FLAG                  0x00000001
#define IAP_XMODEMC_FLAG              0x00000002
#define IAP_XMODEMD_FLAG              0x00000004
#define SET_VER_FLAG                  0x00000008
#define OUTFLASH_DOWNLOAD_FLAG        0x00000010
#define FLASH_DOWNLOAD_FLAG           0x00000020
#define OUTFLASH_XMODEM_FLAG          0x00000040
#define ESP8266_XMODEM_FLAG           0x00000080
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

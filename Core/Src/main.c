/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "delay.h"
#include "iic.h"
#include "at24c02.h"
#include "w25q64.h"
#include "fmc.h"
#include "boot.h"

#include <string.h>
#include <stdint.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern uint8_t UART1_RX_BUF[UART_RX_SIZE];
extern uint8_t UART2_RX_BUF[UART_RX_SIZE];
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
OTA_Setting OTA_Info;
Update_Setting Update_Info;
uint32_t Boot_Flag;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  DWT_Init();
  UART_RX_PtrInit();
  IIC_Init();
  W25Q64_Init();
  /* 从 AT24C02 读取 OTA 标志、版本号和各文件大小。 */
  AT24C02_Read_OTASetting();
  if (OTA_Info.OTA_Flag == 0xFFFFFFFFU)
  {
    /* EEPROM 处于擦除态时写入初始版本号，后续启动不再覆盖用户版本。 */
    memset(&OTA_Info, 0, OTA_SETTING_SIZE);
    memcpy(OTA_Info.OTA_Ver, OTA_INIT_VERSION, OTA_VERSION_INPUT_LEN);
    AT24C02_Write_OTASetting();
  }
  
  /* 使用 DMA 循环接收串口数据，关闭半传输中断避免无用中断。 */
  HAL_UARTEx_ReceiveToIdle_DMA(&huart1, UART1_RX_BUF, UART_RX_SIZE);
  __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
  HAL_UARTEx_ReceiveToIdle_DMA(&huart2, UART2_RX_BUF, UART_RX_SIZE);
  __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);

  /* 检测升级请求；无请求时尝试跳转到 A 区应用。 */
  Bootloader_Branch();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    Delay_ms(10);

    /* 从 DMA 环形缓冲区取出数据，交给 Bootloader 状态机处理。 */
    uint8_t rx_byte;
    while (UART2_RX_Read(&rx_byte, 1) == 1)
    {
      Bootloader_Process_Byte(rx_byte);
    }

    if (Boot_Flag & UPDATEA_FLAG)
    {
      /* OTA 模式下，将外部 Flash 中选定的文件复制到内部 Flash A 区。 */
      uint32_t len = OTA_Info.File_Size[Update_Info.W25Q64_Block];
      uint16_t surplus = len % FLASH_PAGE_SIZE;

      UART2_SendString("长度 ");
      UART2_SendU32(len);
      UART2_SendString(" 字节\r\n");
      if (len != 0 && len <= FLASH_PAGEA_NUMBER * FLASH_PAGE_SIZE && len <= 64 * FLASH_PAGE_SIZE)
      {
        /* 复制前擦除整个 A 区，之后按页读取和写入。 */
        STM32_Erase_Flash(FLASH_PAGEB_NUMBER, FLASH_PAGEA_NUMBER);
        for (uint32_t i = 0; i < len / FLASH_PAGE_SIZE; i++)
        {
          W25Q64_Read_Data(Update_Info.Update_buf, Update_Info.W25Q64_Block * 64 * FLASH_PAGE_SIZE + i * FLASH_PAGE_SIZE, FLASH_PAGE_SIZE);
          STM32_Write_Flash(FLASH_PAGEA_START_ADDR + i * FLASH_PAGE_SIZE, (uint32_t*)Update_Info.Update_buf, FLASH_PAGE_SIZE);
        }
        if (surplus != 0)
        {
          W25Q64_Read_Data(Update_Info.Update_buf, Update_Info.W25Q64_Block * 64 * FLASH_PAGE_SIZE + len / FLASH_PAGE_SIZE * FLASH_PAGE_SIZE, surplus);
          STM32_Write_Flash(FLASH_PAGEA_START_ADDR + len / FLASH_PAGE_SIZE * FLASH_PAGE_SIZE,(uint32_t*)Update_Info.Update_buf, surplus);
        }
        if (Update_Info.W25Q64_Block == 0)
        {
          /* 块 0 表示当前 OTA 文件，成功搬运后清除 OTA 请求标志。 */
          OTA_Info.OTA_Flag = 0;
          AT24C02_Write_OTASetting();
        }
        NVIC_SystemReset();
      }
      else
      {
        UART2_SendString("长度错误\r\n");
        Boot_Flag &= ~UPDATEA_FLAG;
      }
    }

    if (Boot_Flag & IAP_XMODEMC_FLAG)
    {
      /* Xmodem 发送端未开始时，周期性发送字符 C 请求 CRC 模式。 */
      if (Update_Info.Xmodem_Timer >= 100)
      {
        Xmodem_Send_Cmd('C');
        Update_Info.Xmodem_Timer = 0;
      }
      Update_Info.Xmodem_Timer++;
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

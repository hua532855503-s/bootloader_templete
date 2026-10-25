#include "esp8266.h"
#include "main.h"
#include "usart.h"
#include "at24c02.h"
#include "w25q64.h"
#include "boot.h"

#include <string.h>

#define ESP8266_RESPONSE_TIMEOUT 3000U  /* ESP8266 普通 AT 命令的响应等待时间。 */
#define ESP8266_DOWNLOAD_TIMEOUT 120000U /* OTA 固件下载的最长等待时间。 */

/* 清空 ESP8266 接收缓冲区。 */
static void ESP8266_Discard_Rx(void)
{
  uint8_t data[32];

  while (UART1_RX_Read(data, sizeof(data)) != 0U)
  {
    /* 通过移动 UART1 的读指针消费残留数据。 */
  }
}

/* 在指定时间内读取一行 ESP8266 响应，并去除行尾回车换行。 */
static uint8_t ESP8266_Read_Line(char *line, uint16_t size, uint32_t timeout)
{
  uint32_t start = HAL_GetTick();
  uint16_t length = 0;
  uint8_t data;

  while ((HAL_GetTick() - start) < timeout)
  {
    if (UART1_RX_Read(&data, 1) == 1U)
    {
      if (data == '\n')
      {
        if (length != 0U && line[length - 1U] == '\r')
        {
          length--;
        }
        line[length] = '\0';
        UART2_SendString("[ESP8266] ");
        UART2_SendString(line);
        UART2_SendString("\r\n");
        return 1;
      }

      if (length + 1U < size)
      {
        line[length++] = (char)data;
      }
    }
  }

  line[0] = '\0';
  return 0;
}

/* 发送 AT 命令，持续读取响应直到匹配期望的确认字符串。 */
static uint8_t ESP8266_Send_Cmd(const char *cmd, const char *ack)
{
  char line[64];

  ESP8266_Discard_Rx();
  if (HAL_UART_Transmit(&huart1, (uint8_t *)cmd, (uint16_t)strlen(cmd), 1000) != HAL_OK)
  {
    return 0;
  }

  while (ESP8266_Read_Line(line, sizeof(line), ESP8266_RESPONSE_TIMEOUT))
  {
    if (strstr(line, ack) != NULL)
    {
      return 1;
    }
  }

  return 0;
}

/* 检查 ESP8266 是否在线，并配置 OTA 版本和固件下载地址。 */
uint8_t ESP8266_Init(void)
{
  uint8_t retry = 0;

  /* 先确认模块在线，连续失败超过重试次数后放弃本次 OTA。 */
  while (!ESP8266_Send_Cmd("AT\r\n", "OK"))
  {
    if (++retry > 10U)
    {
      return 0;
    }
    HAL_Delay(500);
  }
  
  /* 配置版本信息和固件文件的下载地址。 */
  ESP8266_Send_Cmd("AT+SetVerUrl=http://10.117.18.10:8081/version.txt\r\n", "OK");
  ESP8266_Send_Cmd("AT+SetFwUrl=http://10.117.18.10:8081/LED.bin\r\n", "OK");

  return 1;
}

/* 查询服务器版本并通过 Xmodem 将新固件下载到外部 Flash。 */
void ESP8266_Download(void)
{
  char version[64];
  char size_text[32];
  uint32_t file_size = 0;
  uint32_t start;
  uint32_t last_handshake;
  uint8_t data;
  uint8_t version_received = 0;

  /*
   * OTA 标志表示外部 Flash 中已有待应用固件。
   * 必须先让 Bootloader_Branch() 完成搬运，不能在本次启动中再次下载。
   */
  if (OTA_Info.OTA_Flag == OTA_SET_FLAG)
  {
    UART2_SendString("检测到待应用OTA，跳过重复下载\r\n");
    return;
  }

  /* 初始化 ESP8266，并设置本次 OTA 使用的服务器地址。 */
  if (!ESP8266_Init())
  {
    UART2_SendString("ESP8266初始化失败\r\n");
    return;
  }
  /* 请求服务器返回最新固件版本。 */
  ESP8266_Discard_Rx();
  {
    const char command[] = "AT+GetNewVersion\r\n";
    if (HAL_UART_Transmit(&huart1, (uint8_t *)command, sizeof(command) - 1U, 1000) != HAL_OK)
    {
      UART2_SendString("获取OTA版本失败\r\n");
      return;
    }
  }

  /* 只接受固定格式的版本字符串，避免把异常响应当作版本号。 */
  start = HAL_GetTick();
  while ((HAL_GetTick() - start) < ESP8266_RESPONSE_TIMEOUT && ESP8266_Read_Line(version, sizeof(version), ESP8266_RESPONSE_TIMEOUT))
  {
    if (strncmp(version, "VER-", 4) == 0)
    {
      version_received = 1;
      break;
    }
  }
  if (!version_received || strlen(version) != OTA_VERSION_INPUT_LEN)
  {
    UART2_SendString("OTA版本格式错误\r\n");
    return;
  }

  /* 当前版本与服务器版本一致时，无需重复下载。 */
  if (memcmp(version, OTA_Info.OTA_Ver, OTA_VERSION_INPUT_LEN) == 0)
  {
    UART2_SendString("OTA版本未更新\r\n");
    return;
  }

  /* 获取待下载文件大小，并校验响应只包含十进制数字。 */
  ESP8266_Discard_Rx();
  {
    const char command[] = "AT+GetBinSize\r\n";
    if (HAL_UART_Transmit(&huart1, (uint8_t *)command, sizeof(command) - 1U, 1000) != HAL_OK ||
      !ESP8266_Read_Line(size_text, sizeof(size_text), ESP8266_RESPONSE_TIMEOUT))
    {
      UART2_SendString("获取固件大小失败\r\n");
      return;
    }
  }

  for (uint8_t i = 0; size_text[i] != '\0'; i++)
  {
    if (size_text[i] < '0' || size_text[i] > '9')
    {
      UART2_SendString("固件大小格式错误\r\n");
      return;
    }
    file_size = file_size * 10U + (uint32_t)(size_text[i] - '0');
  }

  /* 固件必须非空，且不能超过 A 区和外部 Flash 单块的容量。 */
  if (file_size == 0U || file_size > FLASH_PAGEA_NUMBER * FLASH_PAGE_SIZE || file_size > 64U * FLASH_PAGE_SIZE)
  {
    UART2_SendString("固件大小超出范围\r\n");
    return;
  }

  /*
   * 清空目标外部 Flash 块，并切换到 Xmodem 接收状态。
   * 接收到的每个数据帧由 Bootloader_Process_Byte() 统一处理。
   */
  ESP8266_Discard_Rx();
  W25Q64_Erase_64K(0);
  Update_Info.W25Q64_Block = 0;
  Update_Info.Xmodem_Timer = 0;
  Update_Info.Xmodem_Num = 0;
  Boot_Flag |= IAP_XMODEMC_FLAG | IAP_XMODEMD_FLAG | OUTFLASH_XMODEM_FLAG | ESP8266_XMODEM_FLAG;

  /* 通知 ESP8266 开始发送固件二进制数据。 */
  {
    const char command[] = "AT+GetBin\r\n";
    if (HAL_UART_Transmit(&huart1, (uint8_t *)command, sizeof(command) - 1U, 1000) != HAL_OK)
    {
      Boot_Flag &= ~(IAP_XMODEMC_FLAG | IAP_XMODEMD_FLAG | OUTFLASH_XMODEM_FLAG | ESP8266_XMODEM_FLAG);
      UART2_SendString("启动固件下载失败\r\n");
      return;
    }
  }

  /*
   * OTA 数据通过 UART1 接收；下载期间定时发送 'C'，
   * 用于启动或恢复 ESP8266 与 Bootloader 之间的 Xmodem 传输。
   */
  start = HAL_GetTick();
  last_handshake = start;
  while ((Boot_Flag & IAP_XMODEMD_FLAG) != 0U && (HAL_GetTick() - start) < ESP8266_DOWNLOAD_TIMEOUT)
  {
    while (UART1_RX_Read(&data, 1) == 1U)
    {
      Bootloader_Process_Byte(data);
    }

    if ((HAL_GetTick() - last_handshake) >= 1000U)
    {
      Xmodem_Send_Cmd('C');
      last_handshake = HAL_GetTick();
    }
  }

  /* 超时或未收到有效数据时取消传输，并清除本次 OTA 的状态标志。 */
  if ((Boot_Flag & IAP_XMODEMD_FLAG) != 0U || Update_Info.Xmodem_Num == 0U)
  {
    Xmodem_Send_Cmd(XMODEM_CAN);
    Xmodem_Send_Cmd(XMODEM_CAN);
    Boot_Flag &= ~(IAP_XMODEMC_FLAG | IAP_XMODEMD_FLAG | OUTFLASH_XMODEM_FLAG | ESP8266_XMODEM_FLAG);
    UART2_SendString("OTA下载超时\r\n");
    return;
  }

  /*
   * Xmodem 接收完成后保存文件大小和版本号。
   * 设置 OTA 标志后复位，由启动流程负责把外部 Flash 固件搬运到 A 区。
   */
  OTA_Info.File_Size[0] = file_size;
  memcpy(OTA_Info.OTA_Ver, version, OTA_VERSION_INPUT_LEN);
  OTA_Info.OTA_Ver[OTA_VERSION_INPUT_LEN] = '\0';
  OTA_Info.OTA_Flag = OTA_SET_FLAG;
  AT24C02_Write_OTASetting();
  HAL_Delay(100);
  Boot_Flag &= ~(OUTFLASH_XMODEM_FLAG | ESP8266_XMODEM_FLAG);
  UART2_SendString("OTA下载完成，准备更新\r\n");
  NVIC_SystemReset();
}

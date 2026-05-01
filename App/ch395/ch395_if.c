/**
  ******************************************************************************
  * @file    : ch395_if.c
  * @brief   : CH395驱动接口实现
  * @note    基于CH395串口模式通信
  ******************************************************************************
  */
#include "ch395_if.h"
#include "ch395_inc.h"
#include "log_uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

extern UART_HandleTypeDef huart3;

/* 引脚宏 */
#define RSTI_H()  HAL_GPIO_WritePin(CH_RSTI_GPIO_Port, CH_RSTI_Pin, GPIO_PIN_SET)
#define RSTI_L()  HAL_GPIO_WritePin(CH_RSTI_GPIO_Port, CH_RSTI_Pin, GPIO_PIN_RESET)
#define GET_INT() HAL_GPIO_ReadPin(CH_INT_GPIO_Port, CH_INT_Pin)
#define GET_LINK() HAL_GPIO_ReadPin(CH_LINK_GPIO_Port, CH_LINK_Pin)

/* 设备配置 */
static const uint8_t s_ip[4]   = {192,168,100,86};
static const uint8_t s_gw[4]   = {192,168,100,1};
static const uint8_t s_mask[4] = {255,255,255,0};
static const uint8_t s_mac[6]  = {0x02,0x00,0x00,0x00,0x00,0x01};

/* 串口底层 */
static void wr_cmd(uint8_t cmd) { uint8_t b[2]={0x57,cmd}; HAL_UART_Transmit(&huart3,b,2,100); }
static void wr_byte(uint8_t d) { HAL_UART_Transmit(&huart3,&d,1,10); }
static uint8_t rd_byte(void) { uint8_t d; HAL_UART_Receive(&huart3,&d,1,10); return d; }
static void wr_data(const uint8_t *d,uint16_t n) { HAL_UART_Transmit(&huart3,(uint8_t*)d,n,500); }
static void rd_data(uint8_t *d,uint16_t n) { HAL_UART_Receive(&huart3,d,n,500); }

static void wr_reg(uint8_t reg,uint8_t val) { wr_cmd(CMD01_WRITE_REG); wr_byte(reg); wr_byte(val); }
static uint8_t rd_reg(uint8_t reg) { wr_cmd(CMD01_READ_REG); wr_byte(reg); return rd_byte(); }

static int hw_reset(void)
{
    RSTI_L(); HAL_Delay(50); RSTI_H();
    uint32_t to = xTaskGetTickCount() + pdMS_TO_TICKS(2000);
    while (GET_INT() != GPIO_PIN_RESET) {
        if (xTaskGetTickCount() > to) { Log_Printf("[CH395] Reset TIMEOUT\r\n"); return -1; }
        vTaskDelay(10);
    }
    Log_Printf("[CH395] Reset OK\r\n");
    return 0;
}

int CH395_IF_Init(void)
{
    if (hw_reset() != 0) return -1;

    /* 检测芯片 */
    wr_cmd(CMD01_GET_IC_VER);
    uint8_t ver = rd_byte();
    Log_Printf("[CH395] IC ver=0x%02X\r\n", ver);

    /* 设置MAC */
    wr_cmd(CMD10_SET_MAC_ADDR);
    wr_data(s_mac, 6);

    /* 设置IP/网关/掩码 */
    wr_cmd(CMD04_SET_IP_ADDR);    wr_data(s_ip, 4);
    wr_cmd(CMD04_SET_GATEWAY_IP); wr_data(s_gw, 4);
    wr_cmd(CMD04_SET_MASK_ADDR);  wr_data(s_mask, 4);

    /* 初始化协议栈 */
    wr_cmd(CMD0W_INIT_CH395);
    uint32_t to = xTaskGetTickCount() + pdMS_TO_TICKS(1000);
    while (GET_INT() != GPIO_PIN_RESET) {
        if (xTaskGetTickCount() > to) { Log_Printf("[CH395] Init TIMEOUT\r\n"); return -1; }
        vTaskDelay(10);
    }

    vTaskDelay(200);
    Log_Printf("[CH395] Link=%s\r\n", GET_LINK() ? "UP" : "DOWN");
    Log_Printf("[CH395] IP=%d.%d.%d.%d\r\n", s_ip[0],s_ip[1],s_ip[2],s_ip[3]);
    return 0;
}

void CH395_IF_Process(void)
{
    if (GET_INT() == GPIO_PIN_RESET) {
        wr_cmd(CMD01_GET_GLOB_INT_STATUS_ALL);
        (void)rd_byte();
    }
}

uint8_t CH395_SetSocketProtType(uint8_t i, uint8_t t)  { wr_reg(SOCK_REG_BASE + i*SOCK_REG_SIZE, t); return CH395_ERR_SUCCESS; }

uint8_t CH395_SetSocketDestIP(uint8_t i, const uint8_t ip[4])
{
    uint8_t base = 0x18 + i*SOCK_REG_SIZE;
    for(int j=0;j<4;j++) wr_reg(base+j, ip[j]);
    return CH395_ERR_SUCCESS;
}

uint8_t CH395_SetSocketDestPort(uint8_t i, uint16_t p)
{
    uint8_t base = 0x16 + i*SOCK_REG_SIZE;
    wr_reg(base, p&0xFF); wr_reg(base+1, (p>>8)&0xFF);
    return CH395_ERR_SUCCESS;
}

uint8_t CH395_SetSocketSourcePort(uint8_t i, uint16_t p)
{
    uint8_t base = 0x14 + i*SOCK_REG_SIZE;
    wr_reg(base, p&0xFF); wr_reg(base+1, (p>>8)&0xFF);
    return CH395_ERR_SUCCESS;
}

uint8_t CH395_OpenSocket(uint8_t i)   { wr_cmd(CMD0W_OPEN_SOCKET);  wr_byte(i); vTaskDelay(20); return CH395_ERR_SUCCESS; }
uint8_t CH395_CloseSocket(uint8_t i)  { wr_cmd(CMD0W_CLOSE_SOCKET); wr_byte(i); vTaskDelay(20); return CH395_ERR_SUCCESS; }
uint8_t CH395_TCPConnect(uint8_t i)   { wr_cmd(CMD0W_TCP_CONNECT);  wr_byte(i); vTaskDelay(20); return CH395_ERR_SUCCESS; }

uint8_t CH395_GetSocketStatus(uint8_t i)
{
    wr_cmd(CMD01_GET_SOCKET_STATUS); wr_byte(i);
    return rd_byte();
}

uint16_t CH395_GetSocketRecvLen(uint8_t i)
{
    wr_cmd(CMD02_GET_RECV_LEN); wr_byte(i);
    uint8_t lo = rd_byte(), hi = rd_byte();
    return (uint16_t)(hi<<8)|lo;
}

uint16_t CH395_SocketRecvData(uint8_t i, uint8_t *buf, uint16_t len)
{
    wr_cmd(CMD03_READ_RECV_DATA); wr_byte(i);
    wr_byte(len&0xFF); wr_byte((len>>8)&0xFF);
    rd_data(buf, len);
    return len;
}

uint8_t CH395_SocketSendData(uint8_t i, const uint8_t *data, uint16_t len)
{
    wr_cmd(CMD03_WRITE_SEND_DATA); wr_byte(i);
    wr_byte(len&0xFF); wr_byte((len>>8)&0xFF);
    wr_data(data, len);
    wr_cmd(CMD0W_TCP_SEND); wr_byte(i);
    return CH395_ERR_SUCCESS;
}

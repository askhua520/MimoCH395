/**
  ******************************************************************************
  * @file    : modbus_master.c
  * @brief   : Modbus RTU 主机 - 轮询采集4路传感器
  * @note    传感器地址3~6, 功能码03, 寄存器0~3: 烟感/水浸/温度/湿度
  ******************************************************************************
  */
#include "modbus_master.h"
#include "log_uart.h"
#include "app_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

/* CRC16 Modbus */
static uint16_t crc16(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (int j = 0; j < 8; j++)
            crc = (crc & 1) ? ((crc >> 1) ^ 0xA001) : (crc >> 1);
    }
    return crc;
}

/* 状态机 */
typedef enum { ST_IDLE, ST_SEND, ST_WAIT, ST_PARSE, ST_ERROR, ST_NEXT } State_e;

/* 私有变量 */
static UART_HandleTypeDef *s_huart;
static GPIO_TypeDef       *s_en_port;
static uint16_t            s_en_pin;
static SensorData_t  s_sensors[MB_MST_MAX_SENSORS];
static uint8_t       s_count, s_idx;
static uint8_t       s_tx[32], s_rx[128];
static volatile uint16_t s_rx_len;
static State_e       s_state;
static volatile uint16_t s_tmr;
static uint8_t       s_retry;

static void rs485_tx(void) { HAL_GPIO_WritePin(s_en_port, s_en_pin, GPIO_PIN_SET); }
static void rs485_rx(void) { HAL_GPIO_WritePin(s_en_port, s_en_pin, GPIO_PIN_RESET); }

static uint16_t build_req(uint8_t addr)
{
    s_tx[0] = addr;
    s_tx[1] = 0x03;
    s_tx[2] = 0; s_tx[3] = 0;   /* reg start = 0 */
    s_tx[4] = 0; s_tx[5] = 4;   /* reg count = 4 */
    uint16_t c = crc16(s_tx, 6);
    s_tx[6] = c & 0xFF;
    s_tx[7] = (c >> 8) & 0xFF;
    return 8;
}

static int parse_resp(SensorData_t *s)
{
    if (s_rx_len < 5) return -3;
    uint16_t cc = crc16(s_rx, s_rx_len - 2);
    uint16_t cr = s_rx[s_rx_len - 2] | (s_rx[s_rx_len - 1] << 8);
    if (cc != cr) return -1;
    if (s_rx[1] & 0x80) return -2;
    if (s_rx[2] != 8) return -3;
    for (int i = 0; i < 4; i++)
        s->regs[i] = (s_rx[3 + i * 2] << 8) | s_rx[4 + i * 2];
    return 0;
}

void Modbus_Master_Init(UART_HandleTypeDef *huart, GPIO_TypeDef *en_port, uint16_t en_pin)
{
    s_huart = huart;
    s_en_port = en_port;
    s_en_pin = en_pin;
    s_count = SENSOR_COUNT;
    for (uint8_t i = 0; i < s_count; i++)
    {
        s_sensors[i].addr = SENSOR_ADDR_START + i;
        memset(s_sensors[i].regs, 0, 8);
        s_sensors[i].online = 0;
        s_sensors[i].error_count = 0;
    }
    s_state = ST_IDLE;
    s_idx = 0;
    s_retry = 0;
    rs485_rx();
    Log_Printf("[MBM] Init: %d sensors (addr %d~%d)\r\n",
               s_count, SENSOR_ADDR_START, SENSOR_ADDR_START + s_count - 1);
}

void Modbus_Master_Process(void)
{
    SensorData_t *sn;
    uint16_t flen;
    int ret;

    switch (s_state)
    {
    case ST_IDLE:
        sn = &s_sensors[s_idx];
        flen = build_req(sn->addr);
        rs485_tx();
        HAL_UART_Transmit(s_huart, s_tx, flen, 50);
        while (__HAL_UART_GET_FLAG(s_huart, UART_FLAG_TC) == RESET) {}
        rs485_rx();
        s_rx_len = 0;
        memset(s_rx, 0, sizeof(s_rx));
        HAL_UART_Receive_IT(s_huart, &s_rx[0], 1);
        s_tmr = 0;
        s_state = ST_WAIT;
        break;

    case ST_WAIT:
        if (s_rx_len >= 13) { s_state = ST_PARSE; }
        else if (s_tmr >= 50) { s_state = ST_ERROR; }
        break;

    case ST_PARSE:
        sn = &s_sensors[s_idx];
        ret = parse_resp(sn);
        if (ret == 0)
        {
            sn->online = 1;
            sn->error_count = 0;
            sn->last_update = xTaskGetTickCount();
            Log_Printf("[MBM] #%d OK smoke=%d water=%d T=%.1f H=%.1f\r\n",
                       sn->addr, sn->regs[0], sn->regs[1],
                       sn->regs[2] / 10.0f, sn->regs[3] / 10.0f);
        }
        else
        {
            Log_Printf("[MBM] #%d parse err %d\r\n", sn->addr, ret);
            if (++sn->error_count >= 3) sn->online = 0;
        }
        s_state = ST_NEXT;
        break;

    case ST_ERROR:
        sn = &s_sensors[s_idx];
        if (++s_retry < 3)
        {
            Log_Printf("[MBM] #%d timeout, retry %d\r\n", sn->addr, s_retry);
            s_state = ST_IDLE;
        }
        else
        {
            Log_Printf("[MBM] #%d OFFLINE\r\n", sn->addr);
            if (++sn->error_count >= 3) sn->online = 0;
            s_retry = 0;
            s_state = ST_NEXT;
        }
        break;

    case ST_NEXT:
        s_idx = (s_idx + 1) % s_count;
        s_retry = 0;
        s_state = ST_IDLE;
        break;

    default: s_state = ST_IDLE; break;
    }
}

void Modbus_Master_Timer10ms(void)
{
    if (s_state == ST_WAIT) s_tmr++;
}

SensorData_t *Modbus_Master_GetSensor(uint8_t idx)
{
    return (idx < s_count) ? &s_sensors[idx] : NULL;
}

uint8_t Modbus_Master_GetSensorCount(void) { return s_count; }

/* UART4接收回调 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART4 && s_state == ST_WAIT)
    {
        if (s_rx_len < sizeof(s_rx) - 1)
        {
            s_rx_len++;
            HAL_UART_Receive_IT(s_huart, &s_rx[s_rx_len], 1);
        }
    }
    if (huart->Instance == USART1)
    {
        Modbus_Slave_RxCallback();
    }
}

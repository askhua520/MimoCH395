/**
  ******************************************************************************
  * @file    : modbus_slave.c
  * @brief   : Modbus RTU 从机 - 汇总4路传感器数据
  * @note    从机地址=1, 功能码03, 寄存器0~15
  ******************************************************************************
  */
#include "modbus_slave.h"
#include "log_uart.h"
#include "app_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <string.h>

static uint16_t slv_crc16(const uint8_t *d, uint16_t n)
{
    uint16_t c = 0xFFFF;
    for (uint16_t i = 0; i < n; i++) {
        c ^= d[i];
        for (int j = 0; j < 8; j++)
            c = (c & 1) ? ((c >> 1) ^ 0xA001) : (c >> 1);
    }
    return c;
}

static UART_HandleTypeDef *s_huart;
static GPIO_TypeDef       *s_en_port;
static uint16_t            s_en_pin;
static uint16_t s_regs[SENSOR_COUNT * 4];
static SemaphoreHandle_t s_mutex;
static uint8_t  s_rx[64], s_tx[128];
static volatile uint16_t s_rx_len;
static volatile uint8_t  s_frame_rdy;
static volatile uint16_t s_idle_cnt;

static void tx_en(void)  { HAL_GPIO_WritePin(s_en_port, s_en_pin, GPIO_PIN_SET); }
static void tx_dis(void) { HAL_GPIO_WritePin(s_en_port, s_en_pin, GPIO_PIN_RESET); }

static uint16_t build_resp(uint8_t addr, uint16_t rs, uint16_t rc)
{
    uint16_t p = 0;
    s_tx[p++] = addr;
    s_tx[p++] = 0x03;
    s_tx[p++] = (uint8_t)(rc * 2);
    for (uint16_t i = 0; i < rc; i++) {
        uint16_t v = ((rs + i) < SENSOR_COUNT * 4) ? s_regs[rs + i] : 0;
        s_tx[p++] = (v >> 8) & 0xFF;
        s_tx[p++] = v & 0xFF;
    }
    uint16_t c = slv_crc16(s_tx, p);
    s_tx[p++] = c & 0xFF;
    s_tx[p++] = (c >> 8) & 0xFF;
    return p;
}

static uint16_t build_exc(uint8_t addr, uint8_t fc, uint8_t ec)
{
    s_tx[0] = addr; s_tx[1] = fc | 0x80; s_tx[2] = ec;
    uint16_t c = slv_crc16(s_tx, 3);
    s_tx[3] = c & 0xFF; s_tx[4] = (c >> 8) & 0xFF;
    return 5;
}

static void process_req(void)
{
    if (s_rx_len < 4) return;
    uint16_t cc = slv_crc16(s_rx, s_rx_len - 2);
    uint16_t cr = s_rx[s_rx_len - 2] | (s_rx[s_rx_len - 1] << 8);
    if (cc != cr) return;

    uint8_t addr = s_rx[0], fc = s_rx[1];
    if (addr != SLAVE_ADDR && addr != 0) return;

    uint16_t rl = 0;
    if (fc == 0x03) {
        if (s_rx_len < 8) { rl = build_exc(addr, fc, 0x03); }
        else {
            uint16_t rs = (s_rx[2] << 8) | s_rx[3];
            uint16_t rc = (s_rx[4] << 8) | s_rx[5];
            if (rs + rc > SENSOR_COUNT * 4) rl = build_exc(addr, fc, 0x02);
            else if (rc == 0 || rc > 125) rl = build_exc(addr, fc, 0x03);
            else {
                if (xSemaphoreTake(s_mutex, 10) == pdTRUE) {
                    rl = build_resp(addr, rs, rc);
                    xSemaphoreGive(s_mutex);
                } else rl = build_exc(addr, fc, 0x04);
            }
        }
    } else rl = build_exc(addr, fc, 0x01);

    if (rl > 0) {
        tx_en();
        HAL_UART_Transmit(s_huart, s_tx, rl, 50);
        while (__HAL_UART_GET_FLAG(s_huart, UART_FLAG_TC) == RESET) {}
        tx_dis();
    }
}

void Modbus_Slave_Init(UART_HandleTypeDef *huart, GPIO_TypeDef *en_port, uint16_t en_pin)
{
    s_huart = huart; s_en_port = en_port; s_en_pin = en_pin;
    memset(s_regs, 0, sizeof(s_regs));
    s_rx_len = 0; s_frame_rdy = 0; s_idle_cnt = 0;
    if (!s_mutex) s_mutex = xSemaphoreCreateMutex();
    tx_dis();
    HAL_UART_Receive_IT(s_huart, &s_rx[0], 1);
    Log_Printf("[MBS] Init: addr=%d, regs=0~%d\r\n", SLAVE_ADDR, SENSOR_COUNT * 4 - 1);
}

void Modbus_Slave_Process(void)
{
    if (s_frame_rdy) {
        s_frame_rdy = 0;
        process_req();
        s_rx_len = 0;
        HAL_UART_Receive_IT(s_huart, &s_rx[0], 1);
    }
}

void Modbus_Slave_Timer10ms(void)
{
    if (s_rx_len > 0 && !s_frame_rdy) {
        if (++s_idle_cnt >= 5) { s_frame_rdy = 1; s_idle_cnt = 0; }
    } else s_idle_cnt = 0;
}

void Modbus_Slave_UpdateRegs(uint16_t addr, const uint16_t *data, uint16_t len)
{
    if (xSemaphoreTake(s_mutex, 10) == pdTRUE) {
        for (uint16_t i = 0; i < len; i++)
            if (addr + i < SENSOR_COUNT * 4) s_regs[addr + i] = data[i];
        xSemaphoreGive(s_mutex);
    }
}

void Modbus_Slave_RxCallback(void)
{
    if (s_rx_len < sizeof(s_rx) - 1) {
        s_rx_len++;
        s_idle_cnt = 0;
        HAL_UART_Receive_IT(s_huart, &s_rx[s_rx_len], 1);
    }
}

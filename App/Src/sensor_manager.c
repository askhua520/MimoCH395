/**
  ******************************************************************************
  * @file    : sensor_manager.c
  * @brief   : 传感器管理 - 从Modbus主机取数据，同步到从机寄存器
  ******************************************************************************
  */
#include "sensor_manager.h"
#include "modbus_master.h"
#include "modbus_slave.h"
#include "log_uart.h"
#include "app_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

static SensorInfo_t s_info[MB_MST_MAX_SENSORS];
static uint8_t      s_count;
static volatile uint8_t s_tick;

static void sync_from_master(void)
{
    for (uint8_t i = 0; i < s_count; i++) {
        SensorData_t *raw = Modbus_Master_GetSensor(i);
        if (!raw) continue;
        SensorInfo_t *inf = &s_info[i];
        inf->addr = raw->addr;
        inf->online = raw->online;
        inf->last_update = raw->last_update;
        if (raw->online) {
            inf->smoke_alarm = (raw->regs[0] != 0) ? 1 : 0;
            inf->water_alarm = (raw->regs[1] != 0) ? 1 : 0;
            inf->temperature = (int16_t)raw->regs[2];
            inf->humidity    = raw->regs[3];
        }
    }
}

static void sync_to_slave(void)
{
    uint16_t regs[SENSOR_COUNT * 4];
    for (uint8_t i = 0; i < s_count; i++) {
        regs[i * 4 + 0] = s_info[i].smoke_alarm;
        regs[i * 4 + 1] = s_info[i].water_alarm;
        regs[i * 4 + 2] = (uint16_t)s_info[i].temperature;
        regs[i * 4 + 3] = s_info[i].humidity;
    }
    Modbus_Slave_UpdateRegs(0, regs, s_count * 4);
}

void Sensor_Manager_Init(void)
{
    s_count = SENSOR_COUNT;
    memset(s_info, 0, sizeof(s_info));
    for (uint8_t i = 0; i < s_count; i++)
        s_info[i].addr = SENSOR_ADDR_START + i;
    Log_Printf("[SEN] Init: %d sensors\r\n", s_count);
}

void Sensor_Manager_Process(void)
{
    if (!s_tick) return;
    s_tick = 0;
    sync_from_master();
    sync_to_slave();
}

void Sensor_Manager_Timer100ms(void) { s_tick = 1; }

SensorInfo_t *Sensor_Manager_GetInfo(uint8_t idx)
{
    return (idx < s_count) ? &s_info[idx] : NULL;
}

uint8_t Sensor_Manager_GetCount(void) { return s_count; }

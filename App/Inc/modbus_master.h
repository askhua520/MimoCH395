/**
  ******************************************************************************
  * @file    : modbus_master.h
  * @brief   : Modbus RTU 主机
  ******************************************************************************
  */
#ifndef __MODBUS_MASTER_H
#define __MODBUS_MASTER_H

#include "main.h"
#include <stdint.h>

#define MB_MST_MAX_SENSORS      8

typedef struct {
    uint8_t  addr;
    uint16_t regs[4];
    uint8_t  online;
    uint8_t  error_count;
    uint32_t last_update;
} SensorData_t;

void Modbus_Master_Init(UART_HandleTypeDef *huart, GPIO_TypeDef *en_port, uint16_t en_pin);
void Modbus_Master_Process(void);
void Modbus_Master_Timer10ms(void);
SensorData_t *Modbus_Master_GetSensor(uint8_t index);
uint8_t Modbus_Master_GetSensorCount(void);

#endif

/**
  ******************************************************************************
  * @file    : sensor_manager.h
  * @brief   : 传感器管理
  ******************************************************************************
  */
#ifndef __SENSOR_MANAGER_H
#define __SENSOR_MANAGER_H

#include "main.h"
#include <stdint.h>

typedef struct {
    uint8_t  addr;
    uint8_t  smoke_alarm;
    uint8_t  water_alarm;
    int16_t  temperature;
    uint16_t humidity;
    uint8_t  online;
    uint32_t last_update;
} SensorInfo_t;

void Sensor_Manager_Init(void);
void Sensor_Manager_Process(void);
void Sensor_Manager_Timer100ms(void);
SensorInfo_t *Sensor_Manager_GetInfo(uint8_t index);
uint8_t Sensor_Manager_GetCount(void);

#endif

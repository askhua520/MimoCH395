/**
  ******************************************************************************
  * @file    : modbus_slave.h
  * @brief   : Modbus RTU 从机
  ******************************************************************************
  */
#ifndef __MODBUS_SLAVE_H
#define __MODBUS_SLAVE_H

#include "main.h"
#include <stdint.h>

void Modbus_Slave_Init(UART_HandleTypeDef *huart, GPIO_TypeDef *en_port, uint16_t en_pin);
void Modbus_Slave_Process(void);
void Modbus_Slave_Timer10ms(void);
void Modbus_Slave_UpdateRegs(uint16_t reg_addr, const uint16_t *data, uint16_t len);
void Modbus_Slave_RxCallback(void);

#endif

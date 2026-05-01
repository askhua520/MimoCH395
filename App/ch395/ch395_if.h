/**
  ******************************************************************************
  * @file    : ch395_if.h
  * @brief   : CH395驱动接口
  ******************************************************************************
  */
#ifndef __CH395_IF_H
#define __CH395_IF_H

#include "main.h"
#include <stdint.h>

#define CH395_ERR_SUCCESS       0x00
#define CH395_ERR_TIMEOUT       0x11
#define CH395_SOCKET_TCP        0x00
#define CH395_SOCKET_ESTABLISHED 0x08

int      CH395_IF_Init(void);
void     CH395_IF_Process(void);
uint8_t  CH395_SetSocketProtType(uint8_t idx, uint8_t type);
uint8_t  CH395_SetSocketDestIP(uint8_t idx, const uint8_t ip[4]);
uint8_t  CH395_SetSocketDestPort(uint8_t idx, uint16_t port);
uint8_t  CH395_SetSocketSourcePort(uint8_t idx, uint16_t port);
uint8_t  CH395_OpenSocket(uint8_t idx);
uint8_t  CH395_CloseSocket(uint8_t idx);
uint8_t  CH395_TCPConnect(uint8_t idx);
uint8_t  CH395_GetSocketStatus(uint8_t idx);
uint16_t CH395_GetSocketRecvLen(uint8_t idx);
uint16_t CH395_SocketRecvData(uint8_t idx, uint8_t *buf, uint16_t len);
uint8_t  CH395_SocketSendData(uint8_t idx, const uint8_t *data, uint16_t len);

#endif

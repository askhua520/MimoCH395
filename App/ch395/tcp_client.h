/**
  ******************************************************************************
  * @file    : tcp_client.h
  * @brief   : CH395 TCP客户端 - 4路Socket
  ******************************************************************************
  */
#ifndef __TCP_CLIENT_H
#define __TCP_CLIENT_H

#include "main.h"
#include <stdint.h>

#define TCP_MAX_SOCKETS         4

typedef enum {
    TCP_SOCK_CLOSED = 0,
    TCP_SOCK_CONNECTING,
    TCP_SOCK_CONNECTED,
    TCP_SOCK_ERROR
} TCP_Sock_State_e;

void TCP_Client_Init(void);
int  TCP_Client_Connect(uint8_t idx, const char *ip, uint16_t port);
int  TCP_Client_Send(uint8_t idx, const uint8_t *data, uint16_t len);
int  TCP_Client_Recv(uint8_t idx, uint8_t *buf, uint16_t buf_size);
void TCP_Client_Close(uint8_t idx);
TCP_Sock_State_e TCP_Client_GetState(uint8_t idx);
void TCP_Client_Process(void);

#endif

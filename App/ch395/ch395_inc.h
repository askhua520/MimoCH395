/**
  ******************************************************************************
  * @file    : ch395_inc.h
  * @brief   : CH395寄存器与命令定义
  ******************************************************************************
  */
#ifndef __CH395_INC_H
#define __CH395_INC_H

/* 命令码 */
#define CMD01_GET_IC_VER        0x01
#define CMD01_READ_REG          0x08
#define CMD01_WRITE_REG         0x09
#define CMD0W_INIT_CH395        0x05
#define CMD01_GET_GLOB_INT_STATUS_ALL  0x09
#define CMD10_SET_MAC_ADDR      0x21
#define CMD04_SET_IP_ADDR       0x22
#define CMD04_SET_GATEWAY_IP    0x23
#define CMD04_SET_MASK_ADDR     0x24
#define CMD0W_OPEN_SOCKET       0x2B
#define CMD0W_CLOSE_SOCKET      0x2C
#define CMD0W_TCP_CONNECT       0x2D
#define CMD0W_TCP_DISCONNECT    0x2F
#define CMD01_GET_SOCKET_STATUS 0x30
#define CMD02_GET_RECV_LEN      0x31
#define CMD03_READ_RECV_DATA    0x32
#define CMD03_WRITE_SEND_DATA   0x33
#define CMD0W_TCP_SEND          0x34

/* 全局中断状态位 */
#define GINT_STAT_UNREACH       0x01
#define GINT_STAT_IP_CONFLICT   0x02
#define GINT_STAT_PHY_CHANGE    0x04
#define GINT_STAT_SOCK0         0x10
#define GINT_STAT_SOCK1         0x20
#define GINT_STAT_SOCK2         0x40
#define GINT_STAT_SOCK3         0x80

/* Socket寄存器基地址偏移 */
#define SOCK_REG_BASE           0x10
#define SOCK_REG_SIZE           0x20

#endif

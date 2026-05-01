/**
  ******************************************************************************
  * @file    : tcp_client.c
  * @brief   : CH395 TCP客户端 - 4路Socket管理
  ******************************************************************************
  */
#include "tcp_client.h"
#include "ch395_if.h"
#include "log_uart.h"
#include "app_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define RX_BUF_SZ   2048
#define CONN_TO     5000

typedef struct {
    TCP_Sock_State_e st;
    uint8_t  sid;
    char     ip[16];
    uint16_t port;
    uint32_t conn_t;
    uint8_t  rxb[RX_BUF_SZ];
    volatile uint16_t rxh, rxt;
    SemaphoreHandle_t mtx;
} Sock_t;

static Sock_t s_sk[TCP_MAX_SOCKETS];

static int parse_ip(const char *s, uint8_t ip[4])
{
    return sscanf(s, "%hhu.%hhu.%hhu.%hhu", &ip[0],&ip[1],&ip[2],&ip[3]) == 4 ? 0 : -1;
}

static uint16_t rb_avail(Sock_t *s) { int32_t n=(int32_t)s->rxh-(int32_t)s->rxt; return (uint16_t)(n<0?n+RX_BUF_SZ:n); }
static void rb_write(Sock_t *s, const uint8_t *d, uint16_t n) { for(uint16_t i=0;i<n;i++){s->rxb[s->rxh]=d[i];s->rxh=(s->rxh+1)%RX_BUF_SZ;} }
static uint16_t rb_read(Sock_t *s, uint8_t *b, uint16_t mx) { uint16_t a=rb_avail(s),n=a<mx?a:mx; for(uint16_t i=0;i<n;i++){b[i]=s->rxb[s->rxt];s->rxt=(s->rxt+1)%RX_BUF_SZ;} return n; }

void TCP_Client_Init(void)
{
    for(int i=0;i<TCP_MAX_SOCKETS;i++){
        s_sk[i].st=TCP_SOCK_CLOSED; s_sk[i].sid=i; s_sk[i].port=0;
        s_sk[i].rxh=0; s_sk[i].rxt=0; s_sk[i].conn_t=0;
        memset(s_sk[i].ip,0,16);
        if(!s_sk[i].mtx) s_sk[i].mtx=xSemaphoreCreateMutex();
    }
    Log_Printf("[TCP] Init: %d sockets\r\n", TCP_MAX_SOCKETS);
}

int TCP_Client_Connect(uint8_t idx, const char *ip, uint16_t port)
{
    if(idx>=TCP_MAX_SOCKETS) return -1;
    Sock_t *s=&s_sk[idx];

    if(s->st==TCP_SOCK_CONNECTED) return 0;

    if(s->st==TCP_SOCK_CONNECTING){
        if((xTaskGetTickCount()-s->conn_t)>=(TickType_t)pdMS_TO_TICKS(CONN_TO)){
            CH395_CloseSocket(s->sid); s->st=TCP_SOCK_ERROR; return -1;
        }
        if(CH395_GetSocketStatus(s->sid)==CH395_SOCKET_ESTABLISHED){
            s->st=TCP_SOCK_CONNECTED; Log_Printf("[TCP] Sock%d OK\r\n",idx); return 0;
        }
        return -1;
    }

    strncpy(s->ip,ip,15); s->ip[15]=0; s->port=port;
    s->rxh=0; s->rxt=0;

    uint8_t ip4[4]; parse_ip(ip,ip4);
    CH395_SetSocketProtType(s->sid,CH395_SOCKET_TCP);
    CH395_SetSocketDestIP(s->sid,ip4);
    CH395_SetSocketDestPort(s->sid,port);
    CH395_SetSocketSourcePort(s->sid,2000+idx);
    CH395_OpenSocket(s->sid);
    CH395_TCPConnect(s->sid);
    s->st=TCP_SOCK_CONNECTING; s->conn_t=xTaskGetTickCount();
    Log_Printf("[TCP] Sock%d -> %s:%d\r\n",idx,ip,port);
    return -1;
}

int TCP_Client_Send(uint8_t idx, const uint8_t *data, uint16_t len)
{
    if(idx>=TCP_MAX_SOCKETS||s_sk[idx].st!=TCP_SOCK_CONNECTED) return -1;
    Sock_t *s=&s_sk[idx];
    if(xSemaphoreTake(s->mtx,1000)!=pdTRUE) return -1;
    uint16_t sent=0;
    while(sent<len){
        uint16_t chunk=len-sent; if(chunk>1024)chunk=1024;
        if(CH395_SocketSendData(s->sid,&data[sent],chunk)==CH395_ERR_SUCCESS) sent+=chunk;
        else break;
    }
    xSemaphoreGive(s->mtx);
    return (int)sent;
}

int TCP_Client_Recv(uint8_t idx, uint8_t *buf, uint16_t sz)
{
    if(idx>=TCP_MAX_SOCKETS||s_sk[idx].st!=TCP_SOCK_CONNECTED) return -1;
    Sock_t *s=&s_sk[idx];
    uint16_t a=rb_avail(s);
    if(a>0) return rb_read(s,buf,sz);
    uint16_t rl=CH395_GetSocketRecvLen(s->sid);
    if(rl>0){
        uint8_t tmp[512]; uint16_t rd=rl>sizeof(tmp)?sizeof(tmp):rl;
        CH395_SocketRecvData(s->sid,tmp,rd);
        rb_write(s,tmp,rd);
        return rb_read(s,buf,sz);
    }
    return 0;
}

void TCP_Client_Close(uint8_t idx)
{
    if(idx>=TCP_MAX_SOCKETS) return;
    CH395_CloseSocket(s_sk[idx].sid);
    s_sk[idx].st=TCP_SOCK_CLOSED; s_sk[idx].rxh=0; s_sk[idx].rxt=0;
}

TCP_Sock_State_e TCP_Client_GetState(uint8_t idx)
{
    return (idx<TCP_MAX_SOCKETS)?s_sk[idx].st:TCP_SOCK_ERROR;
}

void TCP_Client_Process(void)
{
    for(int i=0;i<TCP_MAX_SOCKETS;i++){
        Sock_t *s=&s_sk[i];
        if(s->st==TCP_SOCK_CONNECTED){
            if(CH395_GetSocketStatus(s->sid)!=CH395_SOCKET_ESTABLISHED){
                s->st=TCP_SOCK_CLOSED; continue;
            }
            uint16_t rl=CH395_GetSocketRecvLen(s->sid);
            if(rl>0){
                uint8_t tmp[512]; uint16_t rd=rl>sizeof(tmp)?sizeof(tmp):rl;
                CH395_SocketRecvData(s->sid,tmp,rd);
                rb_write(s,tmp,rd);
            }
        }
    }
}

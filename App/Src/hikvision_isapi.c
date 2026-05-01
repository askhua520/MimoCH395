/**
  ******************************************************************************
  * @file    : hikvision_isapi.c
  * @brief   : 海康ISAPI协议 - MD5摘要认证 + OSD叠加
  ******************************************************************************
  */
#include "hikvision_isapi.h"
#include "sensor_manager.h"
#include "log_uart.h"
#include "tcp_client.h"
#include "app_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ======================== MD5 (RFC 1321) ======================== */
typedef struct { uint32_t s[4]; uint32_t c[2]; uint8_t b[64]; } MD5CTX;

#define F(x,y,z) (((x)&(y))|((~(x))&(z)))
#define G(x,y,z) (((x)&(z))|((y)&(~(z))))
#define H(x,y,z) ((x)^(y)^(z))
#define I(x,y,z) ((y)^((x)|(~(z))))
#define RL(x,n) (((x)<<(n))|((x)>>(32-(n))))

static void md5t(uint32_t st[4], const uint8_t blk[64])
{
    uint32_t a=st[0],b=st[1],c=st[2],d=st[3],x[16];
    memcpy(x,blk,64);
    #define FF(a,b,c,d,k,s,t) {a+=F(b,c,d)+x[k]+t;a=RL(a,s);a+=b;}
    #define GG(a,b,c,d,k,s,t) {a+=G(b,c,d)+x[k]+t;a=RL(a,s);a+=b;}
    #define HH(a,b,c,d,k,s,t) {a+=H(b,c,d)+x[k]+t;a=RL(a,s);a+=b;}
    #define II(a,b,c,d,k,s,t) {a+=I(b,c,d)+x[k]+t;a=RL(a,s);a+=b;}
    FF(a,b,c,d,0,7,0xd76aa478) FF(d,a,b,c,1,12,0xe8c7b756)
    FF(c,d,a,b,2,17,0x242070db) FF(b,c,d,a,3,22,0xc1bdceee)
    FF(a,b,c,d,4,7,0xf57c0faf) FF(d,a,b,c,5,12,0x4787c62a)
    FF(c,d,a,b,6,17,0xa8304613) FF(b,c,d,a,7,22,0xfd469501)
    FF(a,b,c,d,8,7,0x698098d8) FF(d,a,b,c,9,12,0x8b44f7af)
    FF(c,d,a,b,10,17,0xffff5bb1) FF(b,c,d,a,11,22,0x895cd7be)
    FF(a,b,c,d,12,7,0x6b901122) FF(d,a,b,c,13,12,0xfd987193)
    FF(c,d,a,b,14,17,0xa679438e) FF(b,c,d,a,15,22,0x49b40821)
    GG(a,b,c,d,1,5,0xf61e2562) GG(d,a,b,c,6,9,0xc040b340)
    GG(c,d,a,b,11,14,0x265e5a51) GG(b,c,d,a,0,20,0xe9b6c7aa)
    GG(a,b,c,d,5,5,0xd62f105d) GG(d,a,b,c,10,9,0x02441453)
    GG(c,d,a,b,15,14,0xd8a1e681) GG(b,c,d,a,4,20,0xe7d3fbc8)
    GG(a,b,c,d,9,5,0x21e1cde6) GG(d,a,b,c,14,9,0xc33707d6)
    GG(c,d,a,b,3,14,0xf4d50d87) GG(b,c,d,a,8,20,0x455a14ed)
    GG(a,b,c,d,13,5,0xa9e3e905) GG(d,a,b,c,2,9,0xfcefa3f8)
    GG(c,d,a,b,7,14,0x676f02d9) GG(b,c,d,a,12,20,0x8d2a4c8a)
    HH(a,b,c,d,5,4,0xfffa3942) HH(d,a,b,c,8,11,0x8771f681)
    HH(c,d,a,b,11,16,0x6d9d6122) HH(b,c,d,a,14,23,0xfde5380c)
    HH(a,b,c,d,1,4,0xa4beea44) HH(d,a,b,c,4,11,0x4bdecfa9)
    HH(c,d,a,b,7,16,0xf6bb4b60) HH(b,c,d,a,10,23,0xbebfbc70)
    HH(a,b,c,d,13,4,0x289b7ec6) HH(d,a,b,c,0,11,0xeaa127fa)
    HH(c,d,a,b,3,16,0xd4ef3085) HH(b,c,d,a,6,23,0x04881d05)
    HH(a,b,c,d,9,4,0xd9d4d039) HH(d,a,b,c,12,11,0xe6db99e5)
    HH(c,d,a,b,15,16,0x1fa27cf8) HH(b,c,d,a,2,23,0xc4ac5665)
    II(a,b,c,d,0,6,0xf4292244) II(d,a,b,c,7,10,0x432aff97)
    II(c,d,a,b,14,15,0xab9423a7) II(b,c,d,a,5,21,0xfc93a039)
    II(a,b,c,d,12,6,0x655b59c3) II(d,a,b,c,3,10,0x8f0ccc92)
    II(c,d,a,b,10,15,0xffeff47d) II(b,c,d,a,1,21,0x85845dd1)
    II(a,b,c,d,8,6,0x6fa87e4f) II(d,a,b,c,15,10,0xfe2ce6e0)
    II(c,d,a,b,6,15,0xa3014314) II(b,c,d,a,13,21,0x4e0811a1)
    II(a,b,c,d,4,6,0xf7537e82) II(d,a,b,c,11,10,0xbd3af235)
    II(c,d,a,b,2,15,0x2ad7d2bb) II(b,c,d,a,9,21,0xeb86d391)
    #undef FF #undef GG #undef HH #undef II
    st[0]+=a;st[1]+=b;st[2]+=c;st[3]+=d;
}

static void md5i(MD5CTX *ctx) { ctx->c[0]=ctx->c[1]=0; ctx->s[0]=0x67452301; ctx->s[1]=0xefcdab89; ctx->s[2]=0x98badcfe; ctx->s[3]=0x10325476; }

static void md5u(MD5CTX *ctx, const uint8_t *in, uint32_t len)
{
    uint32_t i,idx,pl;
    idx=(ctx->c[0]>>3)&0x3F;
    ctx->c[0]+=len<<3;
    if(ctx->c[0]<(len<<3))ctx->c[1]++;
    ctx->c[1]+=len>>29;
    pl=64-idx;
    if(len>=pl){memcpy(&ctx->b[idx],in,pl);md5t(ctx->s,ctx->b);for(i=pl;i+63<len;i+=64)md5t(ctx->s,&in[i]);idx=0;}else{i=0;}
    memcpy(&ctx->b[idx],&in[i],len-i);
}

static void md5f(uint8_t dig[16], MD5CTX *ctx)
{
    uint8_t bits[8];uint32_t idx,pl;
    memcpy(bits,ctx->c,8);
    idx=(ctx->c[0]>>3)&0x3F;
    pl=(idx<56)?(56-idx):(120-idx);
    static const uint8_t pad[64]={0x80};
    md5u(ctx,pad,pl);md5u(ctx,bits,8);
    memcpy(dig,ctx->s,16);
}

static void md5hex(const char *in, char out[33])
{
    MD5CTX ctx;uint8_t d[16];
    md5i(&ctx);md5u(&ctx,(const uint8_t*)in,strlen(in));md5f(d,&ctx);
    for(int i=0;i<16;i++)sprintf(&out[i*2],"%02x",d[i]);
    out[32]=0;
}

/* ======================== 摄像机状态机 ======================== */
typedef enum { CAM_IDLE, CAM_CONN, CAM_AUTH1, CAM_AUTH2, CAM_OSD, CAM_DONE, CAM_ERR } CamSt_e;

typedef struct {
    char ip[16]; uint16_t port; CamSt_e st; uint8_t skt;
    uint8_t auth_ok, osd_ok, retry; uint32_t last_act;
    char realm[64],nonce[64],qop[32],opaque[64],cnonce[32];
    uint32_t nc;
} Cam_t;

static Cam_t s_cams[ISAPI_MAX_CAMERAS];
static const char *s_user="admin", *s_pass="anor0825";
static char s_rx[2048], s_tx[1024], s_xml[512], s_ab[256];

static int http_get(Cam_t *c, const char *uri, char *buf, int sz)
{
    return snprintf(buf,sz,"GET %s HTTP/1.1\r\nHost: %s:%d\r\nConnection: keep-alive\r\n\r\n",
                    uri,c->ip,c->port);
}

static int http_get_auth(Cam_t *c, const char *uri, char *buf, int sz)
{
    char h1[33],h2[33],r[33],i1[128],i2[128],i3[256];
    snprintf(i1,128,"%s:%s:%s",s_user,c->realm,s_pass); md5hex(i1,h1);
    snprintf(i2,128,"GET:%s",uri); md5hex(i2,h2);
    snprintf(i3,256,"%s:%s:%08x:%s:%s:%s",h1,c->nonce,c->nc,c->cnonce,c->qop,h2);
    md5hex(i3,r);
    snprintf(s_ab,256,"Digest username=\"%s\",realm=\"%s\",nonce=\"%s\",uri=\"%s\",qop=%s,nc=%08x,cnonce=\"%s\",response=\"%s\",opaque=\"%s\"",
             s_user,c->realm,c->nonce,uri,c->qop,c->nc,c->cnonce,r,c->opaque);
    return snprintf(buf,sz,"GET %s HTTP/1.1\r\nHost: %s:%d\r\nAuthorization: %s\r\nConnection: keep-alive\r\n\r\n",
                    uri,c->ip,c->port,s_ab);
}

static int build_osd(Cam_t *c, uint8_t si, char *buf, int sz)
{
    SensorInfo_t *inf = Sensor_Manager_GetInfo(si);
    if(!inf) return 0;
    char l1[32],l2[32],l3[32],l4[32],l5[32];
    snprintf(l1,32,"传感器%d",si+1);
    snprintf(l2,32,"烟感:%s",inf->smoke_alarm?"告警":"正常");
    snprintf(l3,32,"水浸:%s",inf->water_alarm?"告警":"正常");
    snprintf(l4,32,"温度:%.1f℃",inf->temperature/10.0f);
    snprintf(l5,32,"湿度:%.1f%%",inf->humidity/10.0f);
    int xl=snprintf(s_xml,512,
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
        "<VideoOverlay><TextOverlayList>"
        "<TextOverlay><id>1</id><enabled>true</enabled><positionX>40</positionX><positionY>200</positionY><displayText>%s</displayText></TextOverlay>"
        "<TextOverlay><id>2</id><enabled>true</enabled><positionX>40</positionX><positionY>160</positionY><displayText>%s</displayText></TextOverlay>"
        "<TextOverlay><id>3</id><enabled>true</enabled><positionX>40</positionX><positionY>120</positionY><displayText>%s</displayText></TextOverlay>"
        "<TextOverlay><id>4</id><enabled>true</enabled><positionX>40</positionX><positionY>80</positionY><displayText>%s</displayText></TextOverlay>"
        "<TextOverlay><id>5</id><enabled>true</enabled><positionX>40</positionX><positionY>40</positionY><displayText>%s</displayText></TextOverlay>"
        "</TextOverlayList></VideoOverlay>",l1,l2,l3,l4,l5);
    const char *uri="/ISAPI/System/Video/overlays/channels/1";
    char h1[33],h2[33],r[33],i1[128],i2[128],i3[256];
    snprintf(i1,128,"%s:%s:%s",s_user,c->realm,s_pass); md5hex(i1,h1);
    snprintf(i2,128,"PUT:%s",uri); md5hex(i2,h2);
    snprintf(i3,256,"%s:%s:%08x:%s:%s:%s",h1,c->nonce,c->nc,c->cnonce,c->qop,h2);
    md5hex(i3,r);
    return snprintf(buf,sz,
        "PUT %s HTTP/1.1\r\nHost: %s:%d\r\nContent-Type: application/xml\r\nContent-Length: %d\r\n"
        "Authorization: Digest username=\"%s\",realm=\"%s\",nonce=\"%s\",uri=\"%s\",qop=%s,nc=%08x,cnonce=\"%s\",response=\"%s\",opaque=\"%s\"\r\n"
        "Connection: keep-alive\r\n\r\n%s",
        uri,c->ip,c->port,xl,s_user,c->realm,c->nonce,uri,c->qop,c->nc,c->cnonce,r,c->opaque,s_xml);
}

static int parse_status(const char *r) { const char *p=strchr(r,' '); return p?atoi(p+1):-1; }

static int parse_auth(const char *r, Cam_t *c)
{
    const char *p=strstr(r,"WWW-Authenticate:");
    if(!p) return -1;
    #define EXTRACT(tag,fld) do{const char *q=strstr(p,tag"=\"");if(q){q+=strlen(tag)+2;const char *e=strchr(q,'"');if(e&&(size_t)(e-q)<sizeof(c->fld)){strncpy(c->fld,q,e-q);c->fld[e-q]=0;}}}while(0)
    EXTRACT("realm",realm); EXTRACT("nonce",nonce); EXTRACT("qop",qop); EXTRACT("opaque",opaque);
    #undef EXTRACT
    snprintf(c->cnonce,32,"%08lx",(unsigned long)xTaskGetTickCount());
    c->nc=1;
    return 0;
}

void Hikvision_ISAPI_Init(void)
{
    const char *ips[]={"192.168.100.200","192.168.100.201","192.168.100.202","192.168.100.203"};
    for(int i=0;i<ISAPI_MAX_CAMERAS;i++){
        strncpy(s_cams[i].ip,ips[i],15);s_cams[i].ip[15]=0;
        s_cams[i].port=CAMERA_PORT;s_cams[i].st=CAM_IDLE;s_cams[i].skt=i;
        s_cams[i].auth_ok=0;s_cams[i].osd_ok=0;s_cams[i].retry=0;s_cams[i].nc=0;
    }
    Log_Printf("[API] Init: %d cameras\r\n",ISAPI_MAX_CAMERAS);
}

void Hikvision_ISAPI_Process(void)
{
    for(int i=0;i<ISAPI_MAX_CAMERAS;i++){
        Cam_t *c=&s_cams[i];int rl=0,resp;
        switch(c->st){
        case CAM_IDLE:
            if((xTaskGetTickCount()-c->last_act)>=(TickType_t)pdMS_TO_TICKS(5000)){
                c->st=CAM_CONN;c->retry=0;c->auth_ok=0;
            }break;
        case CAM_CONN:
            if(TCP_Client_Connect(c->skt,c->ip,c->port)==0){
                c->st=CAM_AUTH1;Log_Printf("[API] Cam%d connected\r\n",i);
            }else c->st=CAM_ERR;
            break;
        case CAM_AUTH1:
            rl=http_get(c,"/ISAPI/System/Network/capabilities",s_tx,1024);
            TCP_Client_Send(c->skt,(uint8_t*)s_tx,rl);
            resp=TCP_Client_Recv(c->skt,(uint8_t*)s_rx,2048);
            if(resp>0){
                int st=parse_status(s_rx);
                if(st==401){if(parse_auth(s_rx,c)==0)c->st=CAM_AUTH2;else c->st=CAM_ERR;}
                else if(st==200){c->auth_ok=1;c->st=CAM_OSD;}
            }break;
        case CAM_AUTH2:
            rl=http_get_auth(c,"/ISAPI/System/Network/capabilities",s_tx,1024);
            TCP_Client_Send(c->skt,(uint8_t*)s_tx,rl);
            resp=TCP_Client_Recv(c->skt,(uint8_t*)s_rx,2048);
            if(resp>0){
                if(parse_status(s_rx)==200){c->auth_ok=1;c->st=CAM_OSD;Log_Printf("[API] Cam%d auth OK\r\n",i);}
                else{Log_Printf("[API] Cam%d auth fail\r\n",i);c->st=CAM_ERR;}
            }break;
        case CAM_OSD:
            rl=build_osd(c,i,s_tx,1024);
            if(rl>0){
                TCP_Client_Send(c->skt,(uint8_t*)s_tx,rl);
                resp=TCP_Client_Recv(c->skt,(uint8_t*)s_rx,2048);
                if(resp>0&&parse_status(s_rx)==200){c->osd_ok=1;Log_Printf("[API] Cam%d OSD OK\r\n",i);}
            }c->st=CAM_DONE;break;
        case CAM_DONE:
            c->last_act=xTaskGetTickCount();c->st=CAM_IDLE;break;
        case CAM_ERR:
            if(++c->retry<3){vTaskDelay(1000);c->st=CAM_CONN;}
            else{c->last_act=xTaskGetTickCount();c->st=CAM_IDLE;Log_Printf("[API] Cam%d FAILED\r\n",i);}
            break;
        default:c->st=CAM_IDLE;break;
        }
    }
}

void Hikvision_ISAPI_SetCameraIP(uint8_t idx,const char *ip)
{
    if(idx>=ISAPI_MAX_CAMERAS)return;
    strncpy(s_cams[idx].ip,ip,15);s_cams[idx].ip[15]=0;
    s_cams[idx].st=CAM_IDLE;s_cams[idx].auth_ok=0;s_cams[idx].osd_ok=0;
    Log_Printf("[API] Cam%d IP=%s\r\n",idx,ip);
}

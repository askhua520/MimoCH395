/**
  ******************************************************************************
  * @file    : hikvision_isapi.h
  * @brief   : 海康ISAPI协议 - OSD叠加
  ******************************************************************************
  */
#ifndef __HIKVISION_ISAPI_H
#define __HIKVISION_ISAPI_H

#include "main.h"
#include <stdint.h>

#define ISAPI_MAX_CAMERAS       4

void Hikvision_ISAPI_Init(void);
void Hikvision_ISAPI_Process(void);
void Hikvision_ISAPI_SetCameraIP(uint8_t index, const char *ip);

#endif

/**
  ******************************************************************************
  * @file    : app_config.h
  * @brief   : 全局配置参数
  ******************************************************************************
  */
#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

/* 网络 */
#define DEVICE_IP               {192, 168, 100, 86}
#define DEVICE_PORT             2000
#define CAMERA_PORT             80
#define CAMERA_COUNT            4

/* Modbus */
#define SENSOR_COUNT            4
#define SENSOR_ADDR_START       3
#define SLAVE_ADDR              1

/* 定时器 */
#define TIM2_PERIOD_MS          10
#define TIM3_PERIOD_MS          100

/* 版本 */
#define FW_VERSION              "1.0.0"
#define FW_DATE                 "2026-05-01"

#endif /* __APP_CONFIG_H */

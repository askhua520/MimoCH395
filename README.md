# MimoCH395 - CH395网络叠加器

STM32F103RGT6 + CH395 + 海康摄像机OSD叠加

## 快速开始

### 方法1: 用CubeMX生成 (推荐)
1. 打开 `MimoCH395.ioc`
2. 选择 MDK-ARM V5，点击 Generate Code
3. 将 `App/` 目录下的文件添加到Keil工程
4. 添加头文件路径: `App/Inc`, `App/ch395`
5. 编译烧录

### 方法2: 用setup.py自动组装
```bash
python setup.py
```
自动下载STM32CubeF1和FreeRTOS，组装完整工程。

## 源文件清单 (12个)

| 文件 | 行数 | 功能 |
|:---|:---|:---|
| Core/Src/main.c | 200 | 时钟+GPIO+UART+TIM初始化 |
| Core/Src/stm32f1xx_it.c | 40 | 中断处理 |
| App/Src/user.c | 90 | FreeRTOS任务创建 |
| App/Src/log_uart.c | 50 | 日志串口(注册制) |
| App/Src/modbus_master.c | 190 | Modbus主机采集 |
| App/Src/modbus_slave.c | 140 | Modbus从机汇总 |
| App/Src/sensor_manager.c | 70 | 传感器数据管理 |
| App/Src/hikvision_isapi.c | 350 | 海康ISAPI+MD5认证+OSD |
| App/ch395/ch395_if.c | 160 | CH395驱动接口 |
| App/ch395/tcp_client.c | 150 | TCP客户端4路Socket |

## FreeRTOS任务

| 任务 | 优先级 | 周期 | 功能 |
|:---|:---|:---|:---|
| Log | Low | 10ms | 日志输出 |
| MBM | AboveNormal | 100ms | Modbus主机采集 |
| MBS | AboveNormal | 10ms | Modbus从机响应 |
| Sen | Normal | 100ms | 传感器数据同步 |
| API | Normal | 100ms | 海康OSD叠加 |
| Mon | BelowNormal | 30s | 系统监控 |

## 硬件接口

| 接口 | UART | 引脚 | 用途 |
|:---|:---|:---|:---|
| CH395 | UART3 | PA10/PA11 | 以太网 |
| RS485传感器 | UART4 | PB10/PB11 | Modbus主机 |
| RS485输出 | UART1 | PA9/PA10 | Modbus从机 |
| RS485调试 | UART5 | PB12/PB13 | 日志 |

## 网络参数

- 设备IP: 192.168.100.86:2000
- 摄像机: 192.168.100.200~203:80
- 认证: admin/anor0825 (MD5摘要)

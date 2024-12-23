# **SPI**开发指南

发布版本：1.0

作者邮箱：<zyf@rock-chips.com>

日期：2019.07

文件密级：公开资料

---
**前言**

**概述**

**产品版本**

| **芯片名称**            | **RT Thread版本** |
| ----------------------- | :---------------- |
| 全部采用RT Thread的芯片 |                   |

**读者对象**

本文档（本指南）主要适用于以下工程师：
技术支持工程师
软件开发工程师

**修订记录**

| **日期**   | **版本** | **作者** | **修改说明** |
| ---------- | -------- | -------- | ------------ |
| 2019-07-13 | V1.0     | 赵仪峰   | 初始发布     |
|            |          |          |              |
|            |          |          |              |

---

[TOC]

---

## 1 Rockchip SPI 功能特点

SPI（Serial Peripheral Interface）

* 支持4种SPI模式
* 支持2个片选
* 支持8bits 和 16bits 传输
* 支持中断传输模式和DMA传输模式
* 32级FIFO深度（部分芯片是64级）
* 数据采样时钟RXD可配置

## 2 软件

### 2.1 代码路径

框架代码：

```c
components/drivers/include/drivers/spi.h
components/drivers/spi/spi_core.c
components/drivers/spi/spi_dev.c
components/drivers/spi/qspi_core.c
```

串口驱动适配层：

```c
bsp/rockchip-common/drivers/drv_spi.c
bsp/rockchip-common/drivers/drv_spi.h
```

串口测试命令，串口用户程序完全可以参照以下驱动：

```c
bsp/rockchip-common/tests/spi_test.c
```

### 2.2 配置

打开串口配置，同时会生成/dev/spi0..2设备。

```c
RT-Thread bsp drivers  --->
    RT-Thread rockchip "project" drivers  --->
        [*] Enable SPI
        [ ]   Enable SPI0 (SPI2APB)
        [*]   Enable SPI1
        [*]   Enable SPI2
```

### 2.3 SPI测试

使能SPI测试程序：

~~~c
RT-Thread bsp test case  --->
    [*] RT-Thread Common Test case  --->
    	[*] Enable BSP Common TEST
		[*]  Enable BSP Common SPI TEST
~~~

SPI测试命令：

~~~c
1. config spi_device: op_mode, spi_mode, bit_first, speed:
	op_mode: 0 -> master mode, 1 -> slave mode
	spi_mode: 0 - 3 -> RT_SPI_MODE_0 ~ RT_SPI_MODE_3
	bit_first: 0 -> LSB, 1 -> MSB
	speed: config spi clock, the units is Hz
	/* config spi1 cs0 master mode, spi mode 0, LSB, spi clock 1 MHz*/
	example: spi_test config spi1_0 0 0 0 1000000
2. write/read/loop spi_device: times, size like:
	/* write spi1 cs0 1024 bytes 1 time*/
	example: spi_test write spi1_0 1 1024
	/* read spi1 cs1 1024 bytes 10 time */
	example: spi_test read spi1_1 10 1024
	/* loop back mode test spi2 cs0 1024 bytes 10 times */
	example: spi_test loop spi2_0 10 1024
~~~

### 2.4  SPI 使用配置

SPI控制器作为MASTER时可以支持0-50MHz（个别平台可以到更高频率），作为SLAVE时可以支持0-20Mhz。

SPI 框架有提供配置函数rt_spi_configure(), 可以用于配置频率，模式和传输位宽。

SPI支持4种模式，具体使用哪种模式，参考设备手册.

4种模式定义如下：

~~~c
#define RT_SPI_MODE_0       (0 | 0)                        /* CPOL = 0, CPHA = 0 */
#define RT_SPI_MODE_1       (0 | RT_SPI_CPHA)              /* CPOL = 0, CPHA = 1 */
#define RT_SPI_MODE_2       (RT_SPI_CPOL | 0)              /* CPOL = 1, CPHA = 0 */
#define RT_SPI_MODE_3       (RT_SPI_CPOL | RT_SPI_CPHA)    /* CPOL = 1, CPHA = 1 */
~~~

配置代码例子：

~~~c
struct rt_spi_configuration cfg;

cfg.data_width = 8; /* 配置8bits传输模式 */
cfg.mode = RT_SPI_MASTER | RT_SPI_MSB | RT_SPI_MODE_0;
cfg.max_hz = 20 * 1000 * 1000; /* 配置频率 20Mhz */
rt_spi_configure(spi_device, &cfg); /* 配置 SPI*/
~~~
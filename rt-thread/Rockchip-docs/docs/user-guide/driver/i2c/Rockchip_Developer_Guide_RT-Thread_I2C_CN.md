# Rockchip RT-Thread I2C开发向导

发布版本：1.0.1

作者邮箱：<avid.wu@rock-chips.com>

日期：2020.03

文件密级：公开资料

---

**前言**

**概述**

**产品版本**

| **支持芯片**  | **RT-Thread 版本** |
| ------------- | ------------------ |
| RK2108/Pisces | 3.1.3              |

**读者对象**

本文档（本指南）主要适用于以下工程师：

软件开发工程师

**修订记录**

| **日期**   | **版本** | **作者** | **修改说明** |
| ---------- | -------- | -------- | ------------ |
| 2020-02-18 | V1.0.0   | 吴达超   | 初始版本     |
| 2020-03-17 | V1.0.1   | 陈谋春   | 修正链接     |

---

[TOC]

---

## 功能配置

I2C 的功能配置分成两个部分：驱动框架和 VENDOR 配置，具体需要的配置如下：

```shell
# I2C驱动框架
RT_USING_I2C [=y]
# VENDOR驱动，根据需求启用你需要的I2C总线，也可以全开，只是会浪费一点点的内存
RT_USING_I2C0 [=y]
RT_USING_I2C1 [=y]
RT_USING_I2C2 [=y]
```

## 板级配置

RK2108 和 Pisces 的 I2C 的 IO 都是复用的，所以需要确保 IOMUX 的配置中相关的 IO 切到 I2C 模式，具体参见[开发指南](../../../quick-start/Rockchip_Developer_Guide_RT-Thread/Rockchip_Developer_Guide_RT-Thread_CN.html#443-iomux)中的IOMUX章节。

## 开发指南

我司 I2C 驱动遵循 RTT 系统标准 I2C 驱动框架，因此可直接参考 RTT 官方[I2C开发指南](https://www.rt-thread.org/document/site/programming-manual/device/i2c/i2c/)。
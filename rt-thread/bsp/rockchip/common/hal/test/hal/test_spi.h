/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2020-2022 Rockchip Electronics Co., Ltd.
 */

#ifndef _UNITY_HAL_SPI_H
#define _UNITY_HAL_SPI_H

#include "hal_base.h"

/********* RK HAL Unity Test Config **************/
#include "test_conf.h"

/************** Unity Config *********************/
#define UNITY_INT_WIDTH      32
#define UNITY_OUTPUT_CHAR(a) putchar(a)

#define ROCKCHIP_SPI_SPEED_DEFAULT 1000000

#define SPI_DEVICE_MAX 4
#define SPI_MAX_CH     2

/* RK_SPI_CONFIG mode */
#define RK_SPI_CPHA (1<<0)                         /* bit[0]:CPHA, clock phase */
#define RK_SPI_CPOL (1<<1)                         /* bit[1]:CPOL, clock polarity */
/**
 * At CPOL=0 the base value of the clock is zero
 *  - For CPHA=0, data are captured on the clock's rising edge (low->high transition)
 *    and data are propagated on a falling edge (high->low clock transition).
 *  - For CPHA=1, data are captured on the clock's falling edge and data are
 *    propagated on a rising edge.
 * At CPOL=1 the base value of the clock is one (inversion of CPOL=0)
 *  - For CPHA=0, data are captured on clock's falling edge and data are propagated
 *    on a rising edge.
 *  - For CPHA=1, data are captured on clock's rising edge and data are propagated
 *    on a falling edge.
 */
#define RK_SPI_MODE_0    (0 | 0)                        /* CPOL = 0, CPHA = 0 */
#define RK_SPI_MODE_1    (0 | RK_SPI_CPHA)              /* CPOL = 0, CPHA = 1 */
#define RK_SPI_MODE_2    (RK_SPI_CPOL | 0)              /* CPOL = 1, CPHA = 0 */
#define RK_SPI_MODE_3    (RK_SPI_CPOL | RK_SPI_CPHA)    /* CPOL = 1, CPHA = 1 */
#define RK_SPI_MODE_MASK (RK_SPI_CPHA | RK_SPI_CPOL | RK_SPI_MSB)

#define RK_SPI_LSB (0<<2)                         /* bit[2]: 0-LSB */
#define RK_SPI_MSB (1<<2)                         /* bit[2]: 1-MSB */

#define RK_SPI_MASTER (0<<3)                         /* SPI master device */
#define RK_SPI_SLAVE  (1<<3)                         /* SPI slave device */

#define RK_SPI_CSM_SHIFT (4)
#define RK_SPI_CSM_MASK  (0x3 << 4)                     /* SPI master ss_n hold cycles for MOTO SPI master */

/* Rockchip SPI configuration */
struct RK_SPI_CONFIG {
    uint8_t mode;
    uint8_t dataWidth;
    uint8_t reserved;

    uint32_t maxHz;
};

/* Rockchip SPI xfer massage */
struct RK_SPI_MESSAGE {
    uint32_t ch;
    const void *sendBuf;
    void *recvBuf;
    uint32_t length;

    unsigned csTake    : 1;
    unsigned csRelease : 1;
};

HAL_Status SPI_Configure(uint8_t id, struct RK_SPI_CONFIG *config);
uint32_t SPI_Write(uint8_t id, uint8_t cs, const void *sendBuf, uint32_t length);
uint32_t SPI_Read(uint8_t id, uint8_t cs, void *recvBuf, uint32_t length);
HAL_Status SPI_SendThenSend(uint8_t id, uint8_t ch, const void *sendBuf0, uint32_t len0, const void *sendBuf1, uint32_t len1);
HAL_Status SPI_SendThenRecv(uint8_t id, uint8_t ch, const void *sendBuf, uint32_t len0, void *recvBuf, uint32_t len1);
HAL_Status SPI_Init(uint8_t id);

#endif

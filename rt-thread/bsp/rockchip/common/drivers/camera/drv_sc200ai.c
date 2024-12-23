/**
  * Copyright (c) 2022 Rockchip Electronic Co.,Ltd
  *
  * SPDX-License-Identifier: Apache-2.0
  ******************************************************************************
  * @file    drv_sc200ai.c
  * @version V0.0.1
  *
  * Change Logs:
  * 2022-07-07     ISP Team      first implementation
  *
  ******************************************************************************
  */

#include "camera.h"
#include <rthw.h>
#include <rtdevice.h>
#include <rtthread.h>
#include "hal_base.h"
#include <rtconfig.h>
#include "board.h"
#include "drv_clock.h"
#include "board.h"

#ifdef RT_USING_SC200AI
#define SC200AI_DEBUG_INFO      0

#if SC200AI_DEBUG_INFO
#include <stdio.h>
#define SC200AI_DEBUG(...)               rk_kprintf("[SC200AI]:");rk_kprintf(__VA_ARGS__)
#else
#define SC200AI_DEBUG(...)
#endif

#define SC200AI_INFO(dev,arg...)         rk_kprintf("[%s]:",dev->name);rk_kprintf(arg)

#define RT_USING_SC200AI_OPS 0

#define SC200AI_DEVICE_NAME          "sensor_0"
#define I2C_BUS_NAME                "i2c4"
#define SC200AI_7BIT_ADDRESS         (0x30)
#define SC200AI_REG_ID_H_ADDRESS     (0x3107)
#define SC200AI_REG_ID_L_ADDRESS     (0x3108)
#define CHIP_ID             0xcb1c
#define SC200AI_REG_EXPOSURE_H      0x3e00
#define SC200AI_REG_EXPOSURE_M      0x3e01
#define SC200AI_REG_EXPOSURE_L      0x3e02
#define SC200AI_REG_SEXPOSURE_H     0x3e22
#define SC200AI_REG_SEXPOSURE_M     0x3e04
#define SC200AI_REG_SEXPOSURE_L     0x3e05
#define SC200AI_REG_DIG_GAIN        0x3e06
#define SC200AI_REG_DIG_FINE_GAIN   0x3e07
#define SC200AI_REG_ANA_GAIN        0x3e08
#define SC200AI_REG_ANA_FINE_GAIN   0x3e09
#define SC200AI_REG_SDIG_GAIN       0x3e10
#define SC200AI_REG_SDIG_FINE_GAIN  0x3e11
#define SC200AI_REG_SANA_GAIN       0x3e12
#define SC200AI_REG_SANA_FINE_GAIN  0x3e13
#define SC200AI_GAIN_MIN        0x0040
#define SC200AI_GAIN_MAX        (54 * 32 * 64)       //53.975*31.75*64
#define SC200AI_GAIN_STEP       1
#define SC200AI_GAIN_DEFAULT        0x0800
#define SC200AI_LGAIN           0
#define SC200AI_SGAIN           1
#define SC200AI_FLIP_MIRROR_REG         0x3221
#define SC200AI_REG_VTS_H               0x320e
#define SC200AI_REG_VTS_L               0x320f

#define SC200AI_FETCH_EXP_H(VAL)        (((VAL) >> 12) & 0xF)
#define SC200AI_FETCH_EXP_M(VAL)        (((VAL) >> 4) & 0xFF)
#define SC200AI_FETCH_EXP_L(VAL)        (((VAL) & 0xF) << 4)

#define SC200AI_FETCH_AGAIN_H(VAL)      (((VAL) >> 8) & 0x03)
#define SC200AI_FETCH_AGAIN_L(VAL)      ((VAL) & 0xFF)

#define SC200AI_FETCH_MIRROR(VAL, ENABLE)   (ENABLE ? VAL | 0x06 : VAL & 0xf9)
#define SC200AI_FETCH_FLIP(VAL, ENABLE)     (ENABLE ? VAL | 0x60 : VAL & 0x9f)

#define SC200AI_REG_TEST_PATTERN    0x4501
#define SC200AI_TEST_PATTERN_BIT_MASK   BIT(3)

#define REG_END                     (0x0)
#define REG_DELAY                   (0xff)

#define SENSOR_ID(_msb, _lsb)       ((_msb) << 8 | (_lsb))
#define ARRAY_SIZE(x)               (sizeof(x) / sizeof((x)[0]))

#define SC200AI_PIN_CTRL_ENABLE          1
#define SC200AI_I2C_DEBUG_ENABLE         0
#define SC200AI_TESTPATTERN_ENABLE       0

/* Compiler Related Definitions */
#define rk_inline                               rt_inline

/* redefine macro function */
#define RK_ALIGN(size, align)                   RT_ALIGN(size, align)
#define rk_container_of(ptr, type, member)      rt_container_of(ptr, type, member)

/* redefine system variables */
typedef rt_mutex_t                              rk_mutex_t;
typedef rt_sem_t                                rk_semaphore_t;
typedef struct rt_device                        rk_device;
typedef struct rt_i2c_bus_device                rk_i2c_bus_device;
typedef struct clk_gate                         rk_clk_gate;

/* define private system variables */
typedef rt_list_t                               rk_list_node;
typedef rt_list_t                               rk_queue_list;

#define rk_list_first_entry(ptr, type, member)              rt_list_first_entry(ptr, type, member)
#define rk_list_for_each_entry(pos, head, member)           rt_list_for_each_entry(pos, head, member)
#define rk_kprintf                                          rt_kprintf

/* redefine the basic data type */
typedef rt_err_t                        ret_err_t;
typedef rt_size_t                       ret_size_t;
typedef rt_base_t                       dt_base_t;
typedef rt_ubase_t                      dt_ubase_t;
typedef rt_tick_t                       rk_tick_t;
typedef int                             dt_cmd_t;

#define RK_NULL                          RT_NULL
#define MACRO_ASSERT(EX)                 RT_ASSERT(EX)

/* redefine system err code */
#define RET_SYS_EOK                     (RT_EOK)
#define RET_SYS_ERROR                   (-RT_ERROR)
#define RET_SYS_ETIMEOUT                (-RT_ETIMEOUT)  /**< Timed out */
#define RET_SYS_EFULL                   (-RT_EFULL)     /**< The resource is full */
#define RET_SYS_EEMPTY                  (-RT_EEMPTY)    /**< The resource is empty */
#define RET_SYS_ENOMEM                  (-RT_ENOMEM)    /**< No memory */
#define RET_SYS_ENOSYS                  (-RT_ENOSYS)    /**< No system */
#define RET_SYS_EBUSY                   (-RT_EBUSY)     /**< Busy */
#define RET_SYS_EIO                     (-RT_EIO)       /**< IO error */
#define RET_SYS_EINTR                   (-RT_EINTR)     /**< Interrupted system call */
#define RET_SYS_EINVAL                  (-RT_EINVAL)    /**< Invalid argument */

#if 0
struct mclk
{
    rk_clk_gate *gate;
    eCLOCK_Name clk_name;
};
#endif

#define I2C_WRITE_CONTINUE

struct SC200AI_sensor_reg
{
    uint16_t reg_addr;
    uint8_t val;
};

struct sc200ai_mode
{
    uint32_t bus_fmt;
    uint32_t width;
    uint32_t height;
    struct v4l2_fract max_fps;
    uint32_t hts_def;
    uint32_t vts_def;
    uint32_t exp_def;
    /* const struct sensor_reg *reg_list; */
#ifdef I2C_WRITE_CONTINUE
    const uint8_t *reg_list;
#else
    const struct SC200AI_sensor_reg *reg_list;
#endif
    const int reg_list_size;
    uint32_t hdr_mode;
};

#ifdef I2C_WRITE_CONTINUE

static const uint8_t g_SC200AI_init_reg_table[] =
{
    0x3, 0x01, 0x03, 0x01,
    0x3, 0x01, 0x00, 0x00,
    0x3, 0x36, 0xe9, 0x80,
    0x3, 0x36, 0xf9, 0x80,
    0x3, 0x30, 0x1f, 0x4e,
    0xa, 0x32, 0x08, 0x03, 0xc0, 0x02, 0x1c, 0x04, 0x4c, 0x02, 0x32,
    0x3, 0x32, 0x11, 0x02,
    0x3, 0x32, 0x13, 0x02,
    0x3, 0x32, 0x15, 0x31,
    0x3, 0x32, 0x20, 0x17,
    0x3, 0x32, 0x43, 0x01,
    0x4, 0x32, 0x48, 0x02, 0x09,
    0x3, 0x32, 0x53, 0x08,
    0x3, 0x32, 0x71, 0x0a,
    0x6, 0x33, 0x01, 0x06, 0x0c, 0x08, 0x60,
    0x3, 0x33, 0x06, 0x30,
    0x4, 0x33, 0x08, 0x10, 0x70,
    0x3, 0x33, 0x0b, 0x80,
    0x6, 0x33, 0x0d, 0x16, 0x1c, 0x02, 0x02,
    0x3, 0x33, 0x1c, 0x04,
    0x5, 0x33, 0x1e, 0x51, 0x61, 0x07,
    0x3, 0x33, 0x33, 0x10,
    0x3, 0x33, 0x4c, 0x08,
    0x3, 0x33, 0x56, 0x09,
    0x3, 0x33, 0x64, 0x17,
    0xf, 0x33, 0x90, 0x08, 0x18, 0x38, 0x06, 0x06, 0x06, 0x08, 0x18, 0x38, 0x06, 0x0a, 0x10, 0x20,
    0x3, 0x33, 0xac, 0x08,
    0x4, 0x33, 0xae, 0x10, 0x19,
    0x4, 0x36, 0x21, 0xe8, 0x16,
    0x3, 0x36, 0x30, 0xa0,
    0x3, 0x36, 0x37, 0x36,
    0x5, 0x36, 0x3a, 0x1f, 0xc6, 0x0e,
    0x3, 0x36, 0x70, 0x0a,
    0x5, 0x36, 0x74, 0x82, 0x76, 0x78,
    0x4, 0x36, 0x7c, 0x48, 0x58,
    0x5, 0x36, 0x90, 0x34, 0x33, 0x44,
    0x4, 0x36, 0x9c, 0x40, 0x48,
    0x4, 0x36, 0xeb, 0x0c, 0x1c,
    0x3, 0x36, 0xfd, 0x14,
    0x3, 0x39, 0x01, 0x02,
    0x3, 0x39, 0x04, 0x04,
    0x3, 0x39, 0x08, 0x41,
    0x3, 0x39, 0x1f, 0x10,
    0x4, 0x3e, 0x01, 0x45, 0xc0,
    0x4, 0x3e, 0x16, 0x00, 0x80,
    0x3, 0x3f, 0x09, 0x48,
    0x3, 0x48, 0x19, 0x05,
    0x3, 0x48, 0x1b, 0x03,
    0x3, 0x48, 0x1d, 0x0a,
    0x3, 0x48, 0x1f, 0x02,
    0x3, 0x48, 0x21, 0x08,
    0x3, 0x48, 0x23, 0x03,
    0x3, 0x48, 0x25, 0x02,
    0x3, 0x48, 0x27, 0x03,
    0x3, 0x48, 0x29, 0x04,
    0x3, 0x50, 0x00, 0x46,
    0x4, 0x57, 0x87, 0x10, 0x06,
    0x4, 0x57, 0x8a, 0x10, 0x06,
    0x8, 0x57, 0x90, 0x10, 0x10, 0x00, 0x10, 0x10, 0x00,
    0x3, 0x57, 0x99, 0x00,
    0x4, 0x57, 0xc7, 0x10, 0x06,
    0x4, 0x57, 0xca, 0x10, 0x06,
    0x3, 0x57, 0xd1, 0x10,
    0x3, 0x57, 0xd4, 0x10,
    0x3, 0x57, 0xd9, 0x00,
    0x4, 0x59, 0x00, 0xf1, 0x04,
    0x12, 0x59, 0xe0, 0x60, 0x08, 0x3f, 0x18, 0x18, 0x3f, 0x06, 0x02, 0x38, 0x10, 0x0c, 0x10, 0x04, 0x02, 0xa0, 0x08,
    0xe, 0x59, 0xf4, 0x18, 0x10, 0x0c, 0x10, 0x06, 0x02, 0x18, 0x10, 0x0c, 0x10, 0x04, 0x02,
    0x3, 0x36, 0xe9, 0x20,
    0x3, 0x36, 0xf9, 0x24,
    0x46,

};

static const uint8_t g_SC200AI_linear_1080p_reg_table[] =
{
    0x3, 0x01, 0x03, 0x01,
    0x3, 0x01, 0x00, 0x00,
    0x3, 0x36, 0xe9, 0x80,
    0x3, 0x36, 0xf9, 0x80,
    0x3, 0x30, 0x1f, 0x03,
    0x6, 0x32, 0x0c, 0x04, 0x4c, 0x04, 0x65,
    0x3, 0x32, 0x43, 0x01,
    0x4, 0x32, 0x48, 0x02, 0x09,
    0x3, 0x32, 0x53, 0x08,
    0x3, 0x32, 0x71, 0x0a,
    0x3, 0x33, 0x01, 0x20,
    0x3, 0x33, 0x04, 0x40,
    0x3, 0x33, 0x06, 0x32,
    0x3, 0x33, 0x0b, 0x88,
    0x3, 0x33, 0x0f, 0x02,
    0x3, 0x33, 0x1e, 0x39,
    0x3, 0x33, 0x33, 0x10,
    0x4, 0x36, 0x21, 0xe8, 0x16,
    0x3, 0x36, 0x37, 0x1b,
    0x5, 0x36, 0x3a, 0x1f, 0xc6, 0x0e,
    0x3, 0x36, 0x70, 0x0a,
    0x5, 0x36, 0x74, 0x82, 0x76, 0x78,
    0x4, 0x36, 0x7c, 0x48, 0x58,
    0x5, 0x36, 0x90, 0x34, 0x33, 0x44,
    0x4, 0x36, 0x9c, 0x40, 0x48,
    0x3, 0x39, 0x01, 0x02,
    0x3, 0x39, 0x04, 0x04,
    0x3, 0x39, 0x08, 0x41,
    0x3, 0x39, 0x1d, 0x14,
    0x3, 0x39, 0x1f, 0x18,
    0x4, 0x3e, 0x01, 0x8c, 0x20,
    0x4, 0x3e, 0x16, 0x00, 0x80,
    0x3, 0x3f, 0x09, 0x48,
    0x4, 0x57, 0x87, 0x10, 0x06,
    0x4, 0x57, 0x8a, 0x10, 0x06,
    0x8, 0x57, 0x90, 0x10, 0x10, 0x00, 0x10, 0x10, 0x00,
    0x3, 0x57, 0x99, 0x00,
    0x4, 0x57, 0xc7, 0x10, 0x06,
    0x4, 0x57, 0xca, 0x10, 0x06,
    0x3, 0x57, 0xd1, 0x10,
    0x3, 0x57, 0xd4, 0x10,
    0x3, 0x57, 0xd9, 0x00,
    0x12, 0x59, 0xe0, 0x60, 0x08, 0x3f, 0x18, 0x18, 0x3f, 0x06, 0x02, 0x38, 0x10, 0x0c, 0x10, 0x04, 0x02, 0xa0, 0x08,
    0xe, 0x59, 0xf4, 0x18, 0x10, 0x0c, 0x10, 0x06, 0x02, 0x18, 0x10, 0x0c, 0x10, 0x04, 0x02,
    0x3, 0x36, 0xe9, 0x20,
    0x3, 0x36, 0xf9, 0x27,
    0x2e,

};

static const uint8_t g_SC200AI_hdr_1080p_reg_table[] =
{
    0x3, 0x01, 0x03, 0x01,
    0x3, 0x01, 0x00, 0x00,
    0x3, 0x36, 0xe9, 0x80,
    0x3, 0x36, 0xf9, 0x80,
    0x3, 0x30, 0x1f, 0x02,
    0x6, 0x32, 0x0c, 0x04, 0x4c, 0x08, 0xcc,
    0x3, 0x32, 0x20, 0x53,
    0x3, 0x32, 0x43, 0x01,
    0x4, 0x32, 0x48, 0x02, 0x09,
    0x3, 0x32, 0x50, 0x3f,
    0x3, 0x32, 0x53, 0x08,
    0x3, 0x32, 0x71, 0x0a,
    0x6, 0x33, 0x01, 0x06, 0x0c, 0x08, 0x60,
    0x3, 0x33, 0x06, 0x30,
    0x4, 0x33, 0x08, 0x10, 0x70,
    0x3, 0x33, 0x0b, 0x80,
    0x6, 0x33, 0x0d, 0x16, 0x1c, 0x02, 0x02,
    0x3, 0x33, 0x1c, 0x04,
    0x5, 0x33, 0x1e, 0x51, 0x61, 0x07,
    0x3, 0x33, 0x33, 0x10,
    0x3, 0x33, 0x47, 0x77,
    0x3, 0x33, 0x4c, 0x08,
    0x3, 0x33, 0x56, 0x09,
    0x3, 0x33, 0x64, 0x17,
    0x3, 0x33, 0x6c, 0xcc,
    0xf, 0x33, 0x90, 0x08, 0x18, 0x38, 0x06, 0x06, 0x06, 0x08, 0x18, 0x38, 0x06, 0x0a, 0x10, 0x20,
    0x3, 0x33, 0xac, 0x08,
    0x4, 0x33, 0xae, 0x10, 0x19,
    0x4, 0x36, 0x21, 0xe8, 0x16,
    0x3, 0x36, 0x30, 0xa0,
    0x3, 0x36, 0x37, 0x36,
    0x5, 0x36, 0x3a, 0x1f, 0xc6, 0x0e,
    0x3, 0x36, 0x70, 0x0a,
    0x5, 0x36, 0x74, 0x82, 0x76, 0x78,
    0x4, 0x36, 0x7c, 0x48, 0x58,
    0x5, 0x36, 0x90, 0x34, 0x33, 0x44,
    0x4, 0x36, 0x9c, 0x40, 0x48,
    0x4, 0x36, 0xeb, 0x0c, 0x0c,
    0x3, 0x36, 0xfd, 0x14,
    0x3, 0x39, 0x01, 0x02,
    0x3, 0x39, 0x04, 0x04,
    0x3, 0x39, 0x08, 0x41,
    0x3, 0x39, 0x1f, 0x10,
    0x5, 0x3e, 0x00, 0x01, 0x06, 0x00,
    0x8, 0x3e, 0x04, 0x10, 0x60, 0x00, 0x80, 0x03, 0x40,
    0x6, 0x3e, 0x10, 0x00, 0x80, 0x03, 0x40,
    0x4, 0x3e, 0x16, 0x00, 0x80,
    0x4, 0x3e, 0x23, 0x01, 0x9e,
    0x3, 0x3f, 0x09, 0x48,
    0x3, 0x48, 0x16, 0xb1,
    0x3, 0x48, 0x19, 0x09,
    0x3, 0x48, 0x1b, 0x05,
    0x3, 0x48, 0x1d, 0x14,
    0x3, 0x48, 0x1f, 0x04,
    0x3, 0x48, 0x21, 0x0a,
    0x3, 0x48, 0x23, 0x05,
    0x3, 0x48, 0x25, 0x04,
    0x3, 0x48, 0x27, 0x05,
    0x3, 0x48, 0x29, 0x08,
    0x4, 0x57, 0x87, 0x10, 0x06,
    0x4, 0x57, 0x8a, 0x10, 0x06,
    0x8, 0x57, 0x90, 0x10, 0x10, 0x00, 0x10, 0x10, 0x00,
    0x3, 0x57, 0x99, 0x00,
    0x4, 0x57, 0xc7, 0x10, 0x06,
    0x4, 0x57, 0xca, 0x10, 0x06,
    0x3, 0x57, 0xd1, 0x10,
    0x3, 0x57, 0xd4, 0x10,
    0x3, 0x57, 0xd9, 0x00,
    0x12, 0x59, 0xe0, 0x60, 0x08, 0x3f, 0x18, 0x18, 0x3f, 0x06, 0x02, 0x38, 0x10, 0x0c, 0x10, 0x04, 0x02, 0xa0, 0x08,
    0xe, 0x59, 0xf4, 0x18, 0x10, 0x0c, 0x10, 0x06, 0x02, 0x18, 0x10, 0x0c, 0x10, 0x04, 0x02,
    0x3, 0x36, 0xe9, 0x20,
    0x3, 0x36, 0xf9, 0x24,
    0x48,
};

#else
//27MHz
//371.25Mbps 2lane 960x540 120fps
static const struct SC200AI_sensor_reg g_SC200AI_init_reg_table[] =
{
    {0x0103, 0x01},
    {0x0100, 0x00},
    {0x36e9, 0x80},
    {0x36f9, 0x80},
    {0x301f, 0x4e},
    {0x3208, 0x03},
    {0x3209, 0xc0},
    {0x320a, 0x02},
    {0x320b, 0x1c},
    //HTS=1100*2=2200
    {0x320c, 0x04},
    {0x320d, 0x4c},
    //VTS=562
    {0x320e, 0x02},
    {0x320f, 0x32},
    {0x3211, 0x02},
    {0x3213, 0x02},
    {0x3215, 0x31},
    {0x3220, 0x17},
    {0x3243, 0x01},
    {0x3248, 0x02},
    {0x3249, 0x09},
    {0x3253, 0x08},
    {0x3271, 0x0a},
    {0x3301, 0x06},
    {0x3302, 0x0c},
    {0x3303, 0x08},
    {0x3304, 0x60},
    {0x3306, 0x30},
    {0x3308, 0x10},
    {0x3309, 0x70},
    {0x330b, 0x80},
    {0x330d, 0x16},
    {0x330e, 0x1c},
    {0x330f, 0x02},
    {0x3310, 0x02},
    {0x331c, 0x04},
    {0x331e, 0x51},
    {0x331f, 0x61},
    {0x3320, 0x07},
    {0x3333, 0x10},
    {0x334c, 0x08},
    {0x3356, 0x09},
    {0x3364, 0x17},
    {0x3390, 0x08},
    {0x3391, 0x18},
    {0x3392, 0x38},
    {0x3393, 0x06},
    {0x3394, 0x06},
    {0x3395, 0x06},
    {0x3396, 0x08},
    {0x3397, 0x18},
    {0x3398, 0x38},
    {0x3399, 0x06},
    {0x339a, 0x0a},
    {0x339b, 0x10},
    {0x339c, 0x20},
    {0x33ac, 0x08},
    {0x33ae, 0x10},
    {0x33af, 0x19},
    {0x3621, 0xe8},
    {0x3622, 0x16},
    {0x3630, 0xa0},
    {0x3637, 0x36},
    {0x363a, 0x1f},
    {0x363b, 0xc6},
    {0x363c, 0x0e},
    {0x3670, 0x0a},
    {0x3674, 0x82},
    {0x3675, 0x76},
    {0x3676, 0x78},
    {0x367c, 0x48},
    {0x367d, 0x58},
    {0x3690, 0x34},
    {0x3691, 0x33},
    {0x3692, 0x44},
    {0x369c, 0x40},
    {0x369d, 0x48},
    {0x36eb, 0x0c},
    {0x36ec, 0x1c},
    {0x36fd, 0x14},
    {0x3901, 0x02},
    {0x3904, 0x04},
    {0x3908, 0x41},
    {0x391f, 0x10},
    {0x3e01, 0x45},
    {0x3e02, 0xc0},
    {0x3e16, 0x00},
    {0x3e17, 0x80},
    {0x3f09, 0x48},
    {0x4819, 0x05},
    {0x481b, 0x03},
    {0x481d, 0x0a},
    {0x481f, 0x02},
    {0x4821, 0x08},
    {0x4823, 0x03},
    {0x4825, 0x02},
    {0x4827, 0x03},
    {0x4829, 0x04},
    {0x5000, 0x46},
    {0x5787, 0x10},
    {0x5788, 0x06},
    {0x578a, 0x10},
    {0x578b, 0x06},
    {0x5790, 0x10},
    {0x5791, 0x10},
    {0x5792, 0x00},
    {0x5793, 0x10},
    {0x5794, 0x10},
    {0x5795, 0x00},
    {0x5799, 0x00},
    {0x57c7, 0x10},
    {0x57c8, 0x06},
    {0x57ca, 0x10},
    {0x57cb, 0x06},
    {0x57d1, 0x10},
    {0x57d4, 0x10},
    {0x57d9, 0x00},
    {0x5900, 0xf1},
    {0x5901, 0x04},
    {0x59e0, 0x60},
    {0x59e1, 0x08},
    {0x59e2, 0x3f},
    {0x59e3, 0x18},
    {0x59e4, 0x18},
    {0x59e5, 0x3f},
    {0x59e6, 0x06},
    {0x59e7, 0x02},
    {0x59e8, 0x38},
    {0x59e9, 0x10},
    {0x59ea, 0x0c},
    {0x59eb, 0x10},
    {0x59ec, 0x04},
    {0x59ed, 0x02},
    {0x59ee, 0xa0},
    {0x59ef, 0x08},
    {0x59f4, 0x18},
    {0x59f5, 0x10},
    {0x59f6, 0x0c},
    {0x59f7, 0x10},
    {0x59f8, 0x06},
    {0x59f9, 0x02},
    {0x59fa, 0x18},
    {0x59fb, 0x10},
    {0x59fc, 0x0c},
    {0x59fd, 0x10},
    {0x59fe, 0x04},
    {0x59ff, 0x02},
    {0x36e9, 0x20},
    {0x36f9, 0x24},
    {REG_END, 0x00},
};

//27MHz
//371.25Mbps 2lane 1920x1080 30fps
static const struct SC200AI_sensor_reg g_SC200AI_linear_1080p_reg_table[] =
{
    {0x0103, 0x01},
    {0x0100, 0x00},
    {0x36e9, 0x80},
    {0x36f9, 0x80},
    {0x301f, 0x03},
    //HTS=1100*2=2200
    {0x320c, 0x04},
    {0x320d, 0x4c},
    //VTS =1125
    {0x320e, 0x04},
    {0x320f, 0x65},
    {0x3243, 0x01},
    {0x3248, 0x02},
    {0x3249, 0x09},
    {0x3253, 0x08},
    {0x3271, 0x0a},
    {0x3301, 0x20},
    {0x3304, 0x40},
    {0x3306, 0x32},
    {0x330b, 0x88},
    {0x330f, 0x02},
    {0x331e, 0x39},
    {0x3333, 0x10},
    {0x3621, 0xe8},
    {0x3622, 0x16},
    {0x3637, 0x1b},
    {0x363a, 0x1f},
    {0x363b, 0xc6},
    {0x363c, 0x0e},
    {0x3670, 0x0a},
    {0x3674, 0x82},
    {0x3675, 0x76},
    {0x3676, 0x78},
    {0x367c, 0x48},
    {0x367d, 0x58},
    {0x3690, 0x34},
    {0x3691, 0x33},
    {0x3692, 0x44},
    {0x369c, 0x40},
    {0x369d, 0x48},
    {0x3901, 0x02},
    {0x3904, 0x04},
    {0x3908, 0x41},
    {0x391d, 0x14},
    {0x391f, 0x18},
    {0x3e01, 0x8c},
    {0x3e02, 0x20},
    {0x3e16, 0x00},
    {0x3e17, 0x80},
    {0x3f09, 0x48},
    {0x5787, 0x10},
    {0x5788, 0x06},
    {0x578a, 0x10},
    {0x578b, 0x06},
    {0x5790, 0x10},
    {0x5791, 0x10},
    {0x5792, 0x00},
    {0x5793, 0x10},
    {0x5794, 0x10},
    {0x5795, 0x00},
    {0x5799, 0x00},
    {0x57c7, 0x10},
    {0x57c8, 0x06},
    {0x57ca, 0x10},
    {0x57cb, 0x06},
    {0x57d1, 0x10},
    {0x57d4, 0x10},
    {0x57d9, 0x00},
    {0x59e0, 0x60},
    {0x59e1, 0x08},
    {0x59e2, 0x3f},
    {0x59e3, 0x18},
    {0x59e4, 0x18},
    {0x59e5, 0x3f},
    {0x59e6, 0x06},
    {0x59e7, 0x02},
    {0x59e8, 0x38},
    {0x59e9, 0x10},
    {0x59ea, 0x0c},
    {0x59eb, 0x10},
    {0x59ec, 0x04},
    {0x59ed, 0x02},
    {0x59ee, 0xa0},
    {0x59ef, 0x08},
    {0x59f4, 0x18},
    {0x59f5, 0x10},
    {0x59f6, 0x0c},
    {0x59f7, 0x10},
    {0x59f8, 0x06},
    {0x59f9, 0x02},
    {0x59fa, 0x18},
    {0x59fb, 0x10},
    {0x59fc, 0x0c},
    {0x59fd, 0x10},
    {0x59fe, 0x04},
    {0x59ff, 0x02},
    {0x36e9, 0x20},
    {0x36f9, 0x27},
    {REG_END, 0x00},
};

//27MHz
//371.25Mbps 2lane 1920x1080 30fps
static const struct SC200AI_sensor_reg g_SC200AI_hdr_1080p_reg_table[] =
{
    {0x0103, 0x01},
    {0x0100, 0x00},
    {0x36e9, 0x80},
    {0x36f9, 0x80},
    {0x301f, 0x02},
    //HTS=1100*2=2200
    {0x320c, 0x04},
    {0x320d, 0x4c},
    //VTS =2252
    {0x320e, 0x08},
    {0x320f, 0xcc},
    {0x3220, 0x53},
    {0x3243, 0x01},
    {0x3248, 0x02},
    {0x3249, 0x09},
    {0x3250, 0x3f},
    {0x3253, 0x08},
    {0x3271, 0x0a},
    {0x3301, 0x06},
    {0x3302, 0x0c},
    {0x3303, 0x08},
    {0x3304, 0x60},
    {0x3306, 0x30},
    {0x3308, 0x10},
    {0x3309, 0x70},
    {0x330b, 0x80},
    {0x330d, 0x16},
    {0x330e, 0x1c},
    {0x330f, 0x02},
    {0x3310, 0x02},
    {0x331c, 0x04},
    {0x331e, 0x51},
    {0x331f, 0x61},
    {0x3320, 0x07},
    {0x3333, 0x10},
    {0x3347, 0x77},
    {0x334c, 0x08},
    {0x3356, 0x09},
    {0x3364, 0x17},
    {0x336c, 0xcc},
    {0x3390, 0x08},
    {0x3391, 0x18},
    {0x3392, 0x38},
    {0x3393, 0x06},
    {0x3394, 0x06},
    {0x3395, 0x06},
    {0x3396, 0x08},
    {0x3397, 0x18},
    {0x3398, 0x38},
    {0x3399, 0x06},
    {0x339a, 0x0a},
    {0x339b, 0x10},
    {0x339c, 0x20},
    {0x33ac, 0x08},
    {0x33ae, 0x10},
    {0x33af, 0x19},
    {0x3621, 0xe8},
    {0x3622, 0x16},
    {0x3630, 0xa0},
    {0x3637, 0x36},
    {0x363a, 0x1f},
    {0x363b, 0xc6},
    {0x363c, 0x0e},
    {0x3670, 0x0a},
    {0x3674, 0x82},
    {0x3675, 0x76},
    {0x3676, 0x78},
    {0x367c, 0x48},
    {0x367d, 0x58},
    {0x3690, 0x34},
    {0x3691, 0x33},
    {0x3692, 0x44},
    {0x369c, 0x40},
    {0x369d, 0x48},
    {0x36eb, 0x0c},
    {0x36ec, 0x0c},
    {0x36fd, 0x14},
    {0x3901, 0x02},
    {0x3904, 0x04},
    {0x3908, 0x41},
    {0x391f, 0x10},
    {0x3e00, 0x01},
    {0x3e01, 0x06},
    {0x3e02, 0x00},
    {0x3e04, 0x10},
    {0x3e05, 0x60},
    {0x3e06, 0x00},
    {0x3e07, 0x80},
    {0x3e08, 0x03},
    {0x3e09, 0x40},
    {0x3e10, 0x00},
    {0x3e11, 0x80},
    {0x3e12, 0x03},
    {0x3e13, 0x40},
    {0x3e16, 0x00},
    {0x3e17, 0x80},
    {0x3e23, 0x01},
    {0x3e24, 0x9e},
    {0x3f09, 0x48},
    {0x4816, 0xb1},
    {0x4819, 0x09},
    {0x481b, 0x05},
    {0x481d, 0x14},
    {0x481f, 0x04},
    {0x4821, 0x0a},
    {0x4823, 0x05},
    {0x4825, 0x04},
    {0x4827, 0x05},
    {0x4829, 0x08},
    {0x5787, 0x10},
    {0x5788, 0x06},
    {0x578a, 0x10},
    {0x578b, 0x06},
    {0x5790, 0x10},
    {0x5791, 0x10},
    {0x5792, 0x00},
    {0x5793, 0x10},
    {0x5794, 0x10},
    {0x5795, 0x00},
    {0x5799, 0x00},
    {0x57c7, 0x10},
    {0x57c8, 0x06},
    {0x57ca, 0x10},
    {0x57cb, 0x06},
    {0x57d1, 0x10},
    {0x57d4, 0x10},
    {0x57d9, 0x00},
    {0x59e0, 0x60},
    {0x59e1, 0x08},
    {0x59e2, 0x3f},
    {0x59e3, 0x18},
    {0x59e4, 0x18},
    {0x59e5, 0x3f},
    {0x59e6, 0x06},
    {0x59e7, 0x02},
    {0x59e8, 0x38},
    {0x59e9, 0x10},
    {0x59ea, 0x0c},
    {0x59eb, 0x10},
    {0x59ec, 0x04},
    {0x59ed, 0x02},
    {0x59ee, 0xa0},
    {0x59ef, 0x08},
    {0x59f4, 0x18},
    {0x59f5, 0x10},
    {0x59f6, 0x0c},
    {0x59f7, 0x10},
    {0x59f8, 0x06},
    {0x59f9, 0x02},
    {0x59fa, 0x18},
    {0x59fb, 0x10},
    {0x59fc, 0x0c},
    {0x59fd, 0x10},
    {0x59fe, 0x04},
    {0x59ff, 0x02},
    {0x36e9, 0x20},
    {0x36f9, 0x24},
    {REG_END, 0x00},
};

#endif

static struct sc200ai_mode supported_modes[] =
{
    {
        .bus_fmt = MEDIA_BUS_FMT_SBGGR10_1X10,
        .width = 960,
        .height = 540,
        .max_fps = {
            .numerator = 10000,
            .denominator = 1200000,
        },
        .exp_def = 0x46,
        .hts_def = 0x44C * 2,
        .vts_def = 0x0232,
        .reg_list = g_SC200AI_init_reg_table,
        .reg_list_size = ARRAY_SIZE(g_SC200AI_init_reg_table),
        .hdr_mode = NO_HDR,
    },
    {
        .bus_fmt = MEDIA_BUS_FMT_SBGGR10_1X10,
        .width = 1920,
        .height = 1080,
        .max_fps = {
            .numerator = 10000,
            .denominator = 300000,
        },
        .exp_def = 0x46,
        .hts_def = 0x44C * 2,
        .vts_def = 0x0465,
        .reg_list = g_SC200AI_linear_1080p_reg_table,
        .reg_list_size = ARRAY_SIZE(g_SC200AI_linear_1080p_reg_table),
        .hdr_mode = NO_HDR,
    },
    {
        .bus_fmt = MEDIA_BUS_FMT_SBGGR10_1X10,
        .width = 1920,
        .height = 1080,
        .max_fps = {
            .numerator = 10000,
            .denominator = 300000,
        },
        .exp_def = 0x46,
        .hts_def = 0x44C * 2,
        .vts_def = 0x8CC,
        .reg_list = g_SC200AI_hdr_1080p_reg_table,
        .reg_list_size = ARRAY_SIZE(g_SC200AI_hdr_1080p_reg_table),
        .hdr_mode = HDR_X2,
    }
};

struct SC200AI_dev
{
    struct rk_camera_device parent;
    rk_device dev;
    char name[RK_CAMERA_DEVICE_NAME_SIZE];

#if RT_USING_SC200AI_OPS
    struct SC200AI_ops *ops;
#endif

    int32_t pin_rst;
    int32_t pin_pwdn;
    int32_t pin_clkout;
    char i2c_name[RK_CAMERA_I2C_NAME_SIZE];
    rk_i2c_bus_device *i2c_bus;
    struct rt_mutex mutex_lock;
    struct rk_camera_exp_val init_exp;
    struct sc200ai_mode *cur_mode;
    struct sc200ai_mode *dst_mode;
    int flip;
    int hdr;
    bool has_init_exp;
    bool streaming;
};
typedef struct SC200AI_dev *rt_SC200AI_dev_t;

struct SC200AI_ops
{
    ret_err_t (*init)(struct SC200AI_dev *dev);
    ret_err_t (*open)(struct SC200AI_dev *dev, uint16_t oflag);
    ret_err_t (*close)(struct SC200AI_dev *dev);
    ret_err_t (*control)(struct SC200AI_dev *dev, int cmd, void *arg);
};

static struct SC200AI_dev g_SC200AI;

static ret_err_t SC200AI_read_reg(rk_i2c_bus_device *bus,
                                  uint16_t reg, uint8_t *data)
{
    struct rt_i2c_msg msg[2];
    uint8_t send_buf[2];
    uint8_t recv_buf[1];
    ret_err_t ret;

    MACRO_ASSERT(bus != RK_NULL);

    send_buf[0] = ((reg >> 8) & 0xff);
    send_buf[1] = ((reg >> 0) & 0xff);
    msg[0].addr = SC200AI_7BIT_ADDRESS;
    msg[0].flags = RT_I2C_WR;
    msg[0].len = 2;
    msg[0].buf = send_buf;

    msg[1].addr = SC200AI_7BIT_ADDRESS;
    msg[1].flags = RT_I2C_RD;
    msg[1].len = 1;
    msg[1].buf = recv_buf;

    ret = rt_i2c_transfer(bus, msg, 2);
    *data = recv_buf[0];
    if (ret == 2)
        ret = RET_SYS_EOK;
    else
        ret = RET_SYS_ERROR;

    return ret;
}

static ret_err_t SC200AI_write_reg(rk_i2c_bus_device *bus,
                                   uint16_t reg, uint8_t data)
{
    uint8_t send_buf[3];
    struct rt_i2c_msg msgs;
    int ret = 0;

    MACRO_ASSERT(bus != RK_NULL);

    send_buf[0] = ((reg >> 8) & 0xff);
    send_buf[1] = ((reg >> 0) & 0xff);
    send_buf[2] = data;

    msgs.addr = SC200AI_7BIT_ADDRESS;
    msgs.flags = RT_I2C_WR;
    msgs.buf = send_buf;
    msgs.len = 3;
    ret = rt_i2c_transfer(bus, &msgs, 1);

    if (ret == 1)
    {
        SC200AI_DEBUG("(%s):s0.0 i2c_bus ok\n");
        return RET_SYS_EOK;
    }
    else
    {
        SC200AI_DEBUG("(%s):s0.0 i2c_bus error\n");
        return RET_SYS_ERROR;
    }
}

#ifdef I2C_WRITE_CONTINUE

#define MAX_I2C_MSG 80
static int SC200AI_write_multiple_reg_continue(struct SC200AI_dev *dev,
        const uint8_t *i2c_data, int len)
{
    uint16_t i;
    rk_i2c_bus_device *i2c_bus;
    struct rt_i2c_msg msgs[MAX_I2C_MSG];
    int ret = 0;
    int offset = 0;

    MACRO_ASSERT(dev != RK_NULL);
    MACRO_ASSERT(i2c_data != RK_NULL);

    i2c_bus = dev->i2c_bus;
    MACRO_ASSERT(i2c_bus != RK_NULL);

    for (i = 0; i < i2c_data[len - 1]; i++)
    {
        msgs[i].addr = SC200AI_7BIT_ADDRESS;
        msgs[i].flags = RT_I2C_WR;
        msgs[i].buf = (uint8_t *)&i2c_data[offset + 1];
        msgs[i].len = i2c_data[offset];
        offset += (i2c_data[offset] + 1);
    }
#if 0
    ret = rt_i2c_transfer(i2c_bus, msgs, i2c_data[len - 1]);
#else
    for (i = 0; i < i2c_data[len - 1]; i++)
        ret |= rt_i2c_transfer(i2c_bus, &msgs[i], 1);
#endif
    if (ret == 1)
    {
        SC200AI_DEBUG("(%s):s0.0 i2c_bus ok\n");
        return RET_SYS_EOK;
    }
    else
    {
        SC200AI_DEBUG("(%s):s0.0 i2c_bus error\n");
        return RET_SYS_ERROR;
    }

}

#else
static ret_err_t SC200AI_write_reg_continue(rk_i2c_bus_device *bus,
        char *data, uint32_t len)
{
    struct rt_i2c_msg msgs;
    int ret = 0;

    MACRO_ASSERT(bus != RK_NULL);

    msgs.addr = SC200AI_7BIT_ADDRESS;
    msgs.flags = RT_I2C_WR;
    msgs.buf = data;
    msgs.len = len;
    ret = rt_i2c_transfer(bus, &msgs, 1);

    if (ret == 1)
    {
        SC200AI_DEBUG("(%s):s0.0 i2c_bus ok\n");
        return RET_SYS_EOK;
    }
    else
    {
        SC200AI_DEBUG("(%s):s0.0 i2c_bus error\n");
        return RET_SYS_ERROR;
    }
}

#define I2C_WRITE_DEBUG
static void SC200AI_write_multiple_reg(struct SC200AI_dev *dev,
                                       const struct SC200AI_sensor_reg *reg, int len)
{
    uint16_t i;
    rk_i2c_bus_device *i2c_bus;
    int k = 0;
    char *data = rt_malloc(len);
    uint16_t reg_addr;
#ifdef I2C_WRITE_DEBUG
    int j = 0;
    int cnt = 0;
#endif

    MACRO_ASSERT(dev != RK_NULL);
    MACRO_ASSERT(reg != RK_NULL);

    i2c_bus = dev->i2c_bus;
    MACRO_ASSERT(i2c_bus != RK_NULL);

    for (i = 0;; i++)
    {
        if (reg[i].reg_addr == REG_END)
        {
            if (k > 0)
            {
#ifdef I2C_WRITE_DEBUG
                cnt++;
                rt_kprintf("0x%x, ", k + 2);
                for (j = 0; j < k + 2; j++)
                    rt_kprintf("0x%02x, ", data[j]);
                rt_kprintf("\n");
#endif
                SC200AI_write_reg_continue(i2c_bus, data, k + 2);
                k = 0;
            }
            break;
        }

        if (reg[i].reg_addr == REG_DELAY)
        {
            if (k > 0)
            {
#ifdef I2C_WRITE_DEBUG
                cnt++;
                rt_kprintf("0x%x, ", k + 2);
                for (j = 0; j < k + 2; j++)
                    rt_kprintf("0x%02x, ", data[j]);
                rt_kprintf("\n");
#endif
                SC200AI_write_reg_continue(i2c_bus, data, k + 2);
                k = 0;
            }
            HAL_DelayUs(reg[i].val);
        }
        else
        {
            if (k == 0)
            {
                reg_addr = reg[i].reg_addr;
                data[0] = ((reg_addr >> 8) & 0xff);
                data[1] = ((reg_addr >> 0) & 0xff);
                data[2] = reg[i].val;
                k++;
            }
            else
            {
                if ((reg[i - 1].reg_addr + 1) == reg[i].reg_addr)
                {
                    data[k + 2] = reg[i].val;
                    k++;
                    //rt_kprintf(">>>k %d, addr %04x\n", k, reg[i].reg_addr);
                }
                else
                {
#ifdef I2C_WRITE_DEBUG
                    cnt++;
                    rt_kprintf("0x%x, ", k + 2);
                    for (j = 0; j < k + 2; j++)
                        rt_kprintf("0x%02x, ", data[j]);
                    rt_kprintf("\n");
#endif
                    SC200AI_write_reg_continue(i2c_bus, data, k + 2);
                    reg_addr = reg[i].reg_addr;
                    data[0] = ((reg_addr >> 8) & 0xff);
                    data[1] = ((reg_addr >> 0) & 0xff);
                    data[2] = reg[i].val;
                    k = 1;
                }

            }
        }
    }
#ifdef I2C_WRITE_DEBUG
    rt_kprintf("0x%x,\n", cnt);
#endif

}
#endif

/* mode: 0 = lgain  1 = sgain */
static int sc200ai_set_gain_reg(struct SC200AI_dev *dev, uint32_t gain, int mode)
{
    uint8_t Coarse_gain = 1, DIG_gain = 1;
    uint32_t Dcg_gainx100 = 1, ANA_Fine_gainx64 = 1;
    uint8_t Coarse_gain_reg = 0, DIG_gain_reg = 0;
    uint8_t ANA_Fine_gain_reg = 0x20, DIG_Fine_gain_reg = 0x80;
    int ret = 0;

    gain = gain * 16;
    if (gain <= 1024)
        gain = 1024;
    else if (gain > SC200AI_GAIN_MAX * 16)
        gain = SC200AI_GAIN_MAX * 16;

    if (gain < 2 * 1024)                 // start again
    {
        Dcg_gainx100 = 100;
        Coarse_gain = 1;
        DIG_gain = 1;
        Coarse_gain_reg = 0x03;
        DIG_gain_reg = 0x0;
        DIG_Fine_gain_reg = 0x80;
    }
    else if (gain <= 3456)
    {
        Dcg_gainx100 = 100;
        Coarse_gain = 2;
        DIG_gain = 1;
        Coarse_gain_reg = 0x07;
        DIG_gain_reg = 0x0;
        DIG_Fine_gain_reg = 0x80;
    }
    else if (gain <= 6908)
    {
        Dcg_gainx100 = 340;
        Coarse_gain = 1;
        DIG_gain = 1;
        Coarse_gain_reg = 0x23;
        DIG_gain_reg = 0x0;
        DIG_Fine_gain_reg = 0x80;
    }
    else if (gain <= 13817)
    {
        Dcg_gainx100 = 340;
        Coarse_gain = 2;
        DIG_gain = 1;
        Coarse_gain_reg = 0x27;
        DIG_gain_reg = 0x0;
        DIG_Fine_gain_reg = 0x80;
    }
    else if (gain <= 27635)
    {
        Dcg_gainx100 = 340;
        Coarse_gain = 4;
        DIG_gain = 1;
        Coarse_gain_reg = 0x2f;
        DIG_gain_reg = 0x0;
        DIG_Fine_gain_reg = 0x80;
    }
    else if (gain <= 55270)               // end again
    {
        Dcg_gainx100 = 340;
        Coarse_gain = 8;
        DIG_gain = 1;
        Coarse_gain_reg = 0x3f;
        DIG_gain_reg = 0x0;
        DIG_Fine_gain_reg = 0x80;
    }
    else if (gain < 55270 * 2)             // start dgain
    {
        Dcg_gainx100 = 340;
        Coarse_gain = 8;
        DIG_gain = 1;
        ANA_Fine_gainx64 = 127;
        Coarse_gain_reg = 0x3f;
        DIG_gain_reg = 0x0;
        ANA_Fine_gain_reg = 0x7f;
    }
    else if (gain < 55270 * 4)
    {
        Dcg_gainx100 = 340;
        Coarse_gain = 8;
        DIG_gain = 2;
        ANA_Fine_gainx64 = 127;
        Coarse_gain_reg = 0x3f;
        DIG_gain_reg = 0x1;
        ANA_Fine_gain_reg = 0x7f;
    }
    else if (gain < 55270 * 8)
    {
        Dcg_gainx100 = 340;
        Coarse_gain = 8;
        DIG_gain = 4;
        ANA_Fine_gainx64 = 127;
        Coarse_gain_reg = 0x3f;
        DIG_gain_reg = 0x3;
        ANA_Fine_gain_reg = 0x7f;
    }
    else if (gain < 55270 * 16)
    {
        Dcg_gainx100 = 340;
        Coarse_gain = 8;
        DIG_gain = 8;
        ANA_Fine_gainx64 = 127;
        Coarse_gain_reg = 0x3f;
        DIG_gain_reg = 0x7;
        ANA_Fine_gain_reg = 0x7f;
    }
    else if (gain <= 1754822)
    {
        Dcg_gainx100 = 340;
        Coarse_gain = 8;
        DIG_gain = 16;
        ANA_Fine_gainx64 = 127;
        Coarse_gain_reg = 0x3f;
        DIG_gain_reg = 0xF;
        ANA_Fine_gain_reg = 0x7f;
    }

    if (gain < 3456)
        ANA_Fine_gain_reg = abs(100 * gain / (Dcg_gainx100 * Coarse_gain) / 16);
    else if (gain == 3456)
        ANA_Fine_gain_reg = 0x6C;
    else if (gain < 55270)
        ANA_Fine_gain_reg = abs(100 * gain / (Dcg_gainx100 * Coarse_gain) / 16);
    else
        DIG_Fine_gain_reg = abs(800 * gain / (Dcg_gainx100 * Coarse_gain *
                                              DIG_gain) / ANA_Fine_gainx64);

    if (mode == SC200AI_LGAIN)
    {
        ret = SC200AI_write_reg(dev->i2c_bus,
                                SC200AI_REG_DIG_GAIN,
                                DIG_gain_reg & 0xF);
        ret |= SC200AI_write_reg(dev->i2c_bus,
                                 SC200AI_REG_DIG_FINE_GAIN,
                                 DIG_Fine_gain_reg);
        ret |= SC200AI_write_reg(dev->i2c_bus,
                                 SC200AI_REG_ANA_GAIN,
                                 Coarse_gain_reg);
        ret |= SC200AI_write_reg(dev->i2c_bus,
                                 SC200AI_REG_ANA_FINE_GAIN,
                                 ANA_Fine_gain_reg);
    }
    else
    {
        ret = SC200AI_write_reg(dev->i2c_bus,
                                SC200AI_REG_SDIG_GAIN,
                                DIG_gain_reg & 0xF);
        ret |= SC200AI_write_reg(dev->i2c_bus,
                                 SC200AI_REG_SDIG_FINE_GAIN,
                                 DIG_Fine_gain_reg);
        ret |= SC200AI_write_reg(dev->i2c_bus,
                                 SC200AI_REG_SANA_GAIN,
                                 Coarse_gain_reg);
        ret |= SC200AI_write_reg(dev->i2c_bus,
                                 SC200AI_REG_SANA_FINE_GAIN,
                                 ANA_Fine_gain_reg);
    }

    if (gain <= 20 * 1024)
        ret |= SC200AI_write_reg(dev->i2c_bus,
                                 0x5799,
                                 0x0);
    else if (gain >= 30 * 1024)
        ret |= SC200AI_write_reg(dev->i2c_bus,
                                 0x5799,
                                 0x07);

    return ret;
}

static rt_err_t rk_sc200ai_set_expval(struct SC200AI_dev *dev, struct rk_camera_exp_val *exp)
{
    rt_err_t ret = RET_SYS_EOK;
    uint32_t l_exp_time, s_exp_time;
    uint32_t l_a_gain, s_a_gain;

    SC200AI_DEBUG("(%s) enter \n", __FUNCTION__);

    MACRO_ASSERT(dev != RK_NULL);

    rt_mutex_take(&dev->mutex_lock, RT_WAITING_FOREVER);

    if (!dev->has_init_exp && !dev->streaming)
    {
        dev->init_exp = *exp;
        dev->has_init_exp = true;
        SC200AI_DEBUG("sc200ai don't stream, record exp for hdr!\n");

        rt_mutex_release(&dev->mutex_lock);
        return ret;
    }

    if (dev->cur_mode->hdr_mode == NO_HDR)
    {
        l_exp_time = exp->reg_time[0] * 2;
        l_a_gain = exp->reg_gain[0];
        s_exp_time = 0;
        s_a_gain = 0;
    }
    else if (dev->cur_mode->hdr_mode == HDR_X2)
    {
        l_exp_time = exp->reg_time[1] * 2;
        s_exp_time = exp->reg_time[0] * 2;
        l_a_gain = exp->reg_gain[1];
        s_a_gain = exp->reg_gain[0];
    }

    rk_kprintf("rev exp req: L_exp: 0x%x, 0x%x, S_exp: 0x%x, 0x%x\n",
               l_exp_time, l_a_gain, s_exp_time, s_a_gain);

    //set exposure
    if (l_exp_time > 4362)                  //(2250 - 64 - 5) * 2
        l_exp_time = 4362;
    if (s_exp_time > 404)                //((0x3e23<<8|0x3e24) - 5) * 2
        s_exp_time = 404;

    ret = SC200AI_write_reg(dev->i2c_bus,
                            SC200AI_REG_EXPOSURE_H,
                            SC200AI_FETCH_EXP_H(l_exp_time));
    ret |= SC200AI_write_reg(dev->i2c_bus,
                             SC200AI_REG_EXPOSURE_M,
                             SC200AI_FETCH_EXP_M(l_exp_time));
    ret |= SC200AI_write_reg(dev->i2c_bus,
                             SC200AI_REG_EXPOSURE_L,
                             SC200AI_FETCH_EXP_L(l_exp_time));
    if (dev->cur_mode->hdr_mode == HDR_X2)
    {
        ret |= SC200AI_write_reg(dev->i2c_bus,
                                 SC200AI_REG_SEXPOSURE_M,
                                 SC200AI_FETCH_EXP_M(s_exp_time));
        ret |= SC200AI_write_reg(dev->i2c_bus,
                                 SC200AI_REG_SEXPOSURE_L,
                                 SC200AI_FETCH_EXP_L(s_exp_time));
    }

    ret |= sc200ai_set_gain_reg(dev, l_a_gain, SC200AI_LGAIN);
    if (dev->cur_mode->hdr_mode == HDR_X2)
        ret |= sc200ai_set_gain_reg(dev, s_a_gain, SC200AI_SGAIN);

    rt_mutex_release(&dev->mutex_lock);

    SC200AI_DEBUG("(%s) exit \n", __FUNCTION__);

    return ret;
}

static rt_err_t rk_sc200ai_set_vts(struct SC200AI_dev *dev, uint32_t dst_vts)
{
    rt_err_t ret = RET_SYS_ENOSYS;

    SC200AI_DEBUG("(%s) enter \n", __FUNCTION__);

    MACRO_ASSERT(dev != RK_NULL);

    SC200AI_DEBUG("(%s) set vts: 0x%x \n", __FUNCTION__, dst_vts);

    ret = SC200AI_write_reg(dev->i2c_bus, SC200AI_REG_VTS_L,
                            (uint8_t)(dst_vts & 0xff));
    ret |= SC200AI_write_reg(dev->i2c_bus, SC200AI_REG_VTS_H,
                             (uint8_t)(dst_vts >> 8));

    SC200AI_DEBUG("(%s) exit \n", __FUNCTION__);

    return ret;
}

static rt_err_t rk_sc200ai_set_flip_mirror(struct SC200AI_dev *dev, uint32_t flip)
{
    rt_err_t ret = RET_SYS_ENOSYS;
    uint8_t val = 0;

    SC200AI_DEBUG("(%s) enter \n", __FUNCTION__);

    MACRO_ASSERT(dev != RK_NULL);

    ret = SC200AI_read_reg(dev->i2c_bus, SC200AI_FLIP_MIRROR_REG, &val);
    switch (flip)
    {
    case 0:
        val = SC200AI_FETCH_MIRROR(val, false);
        val = SC200AI_FETCH_FLIP(val, false);
        break;
    case 1:
        val = SC200AI_FETCH_MIRROR(val, true);
        val = SC200AI_FETCH_FLIP(val, false);
        break;
    case 2:
        val = SC200AI_FETCH_MIRROR(val, false);
        val = SC200AI_FETCH_FLIP(val, true);
        break;
    case 3:
        val = SC200AI_FETCH_MIRROR(val, true);
        val = SC200AI_FETCH_FLIP(val, true);
        break;
    default:
        val = SC200AI_FETCH_MIRROR(val, false);
        val = SC200AI_FETCH_FLIP(val, false);
        break;
    };
    SC200AI_DEBUG("(%s) flip 0x%x, reg val 0x%x\n", __FUNCTION__, flip, val);
    ret |= SC200AI_write_reg(dev->i2c_bus, SC200AI_FLIP_MIRROR_REG, val);
    SC200AI_DEBUG("(%s) exit\n", __FUNCTION__);

    return ret;
}

static void SC200AI_stream_on(struct SC200AI_dev *dev)
{
    SC200AI_DEBUG("(%s) enter \n", __FUNCTION__);
#ifndef I2C_WRITE_CONTINUE
    int i = 0;
#endif

    rk_kprintf("sc200ai on enter tick:%u\n", rt_tick_get());

    MACRO_ASSERT(dev != RK_NULL);

    rt_mutex_take(&dev->mutex_lock, RT_WAITING_FOREVER);

#ifdef I2C_WRITE_CONTINUE
    SC200AI_write_multiple_reg_continue((struct SC200AI_dev *)dev, dev->cur_mode->reg_list, dev->cur_mode->reg_list_size);
#else
    for (i = 0; i < ARRAY_SIZE(supported_modes); i++)
    {
        SC200AI_write_multiple_reg((struct SC200AI_dev *)dev, supported_modes[i].reg_list, supported_modes[i].reg_list_size);
    }

#endif

#ifndef RT_USING_CAM_STREAM_ON_LATE
    if (dev->has_init_exp)
    {
        rt_mutex_release(&dev->mutex_lock);
        rk_sc200ai_set_expval(dev, &dev->init_exp);
        rt_mutex_take(&dev->mutex_lock, RT_WAITING_FOREVER);
    }
    rk_sc200ai_set_flip_mirror(dev, dev->flip);

    SC200AI_write_reg(dev->i2c_bus, 0x0100, 0x01);
    dev->streaming = true;
#endif

    rt_mutex_release(&dev->mutex_lock);
    SC200AI_DEBUG("(%s) exit \n", __FUNCTION__);
    rk_kprintf("sc200ai on exit tick:%u\n", rt_tick_get());
}

static void SC200AI_stream_on_late(struct SC200AI_dev *dev)
{

    rk_kprintf("%s enter tick:%u\n", __FUNCTION__, rt_tick_get());

    MACRO_ASSERT(dev != RK_NULL);

    rt_mutex_take(&dev->mutex_lock, RT_WAITING_FOREVER);

    if (dev->has_init_exp)
    {
        rt_mutex_release(&dev->mutex_lock);
        rk_sc200ai_set_expval(dev, &dev->init_exp);
        rt_mutex_take(&dev->mutex_lock, RT_WAITING_FOREVER);
    }
    rk_sc200ai_set_flip_mirror(dev, dev->flip);

    SC200AI_write_reg(dev->i2c_bus, 0x0100, 0x01);
    rt_mutex_release(&dev->mutex_lock);
    dev->streaming = true;
    rk_kprintf("%s exit tick:%u\n", __FUNCTION__, rt_tick_get());
}

static void SC200AI_stream_off(struct SC200AI_dev *dev)
{
    rk_i2c_bus_device *i2c_bus;

    SC200AI_DEBUG("(%s) enter \n", __FUNCTION__);

    MACRO_ASSERT(dev != RK_NULL);


    i2c_bus = dev->i2c_bus;
    if (i2c_bus)
    {
        rt_mutex_take(&dev->mutex_lock, RT_WAITING_FOREVER);

        SC200AI_write_reg(dev->i2c_bus, 0x0100, 0x00);

        rt_mutex_release(&dev->mutex_lock);
    }
    else
    {
        SC200AI_INFO(dev, "Err: not find out i2c bus!\n");
    }
    dev->streaming = false;

    SC200AI_DEBUG("(%s) exit \n", __FUNCTION__);
}

ret_err_t rk_SC200AI_init(struct rk_camera_device *dev)
{
    SC200AI_DEBUG("(%s) enter \n", __FUNCTION__);

    ret_err_t ret = RET_SYS_EOK;
    struct SC200AI_dev *SC200AI;

    SC200AI = (struct SC200AI_dev *)dev;
    struct rk_camera_device *camera = (struct rk_camera_device *)&SC200AI->parent;
#if RT_USING_SC200AI_OPS
    if (SC200AI->ops->init)
    {
        return (SC200AI->ops->init(SC200AI));
    }
#else

    if (SC200AI)
    {
        camera->info.mbus_fmt.width = 960;
        camera->info.mbus_fmt.height = 540;
        camera->info.mbus_fmt.pixelcode = MEDIA_BUS_FMT_SBGGR10_1X10;//0x0c uyvy;0x08 vyuy;0x04 yvyu;0x00 yuyv
        camera->info.mbus_fmt.field = 0;
        camera->info.mbus_fmt.colorspace = 0;
        camera->info.mbus_config.linked_freq = 371000000;
        camera->info.mbus_config.mbus_type = CAMERA_MBUS_CSI2_DPHY;
        camera->info.mbus_config.flags = MEDIA_BUS_FLAGS_CSI2_LVDS_LANES_2 |
                                         MEDIA_BUS_FLAGS_CSI2_LVDS_CLOCK_MODE_CONTIN;
        camera->info.hdr_mode = 0;

        SC200AI->i2c_bus = (rk_i2c_bus_device *)rt_device_find(SC200AI->i2c_name);

        if (!SC200AI->i2c_bus)
        {
            SC200AI_DEBUG("Warning:not find i2c source 2:%s !!!\n",
                          SC200AI->i2c_name);
            return RET_SYS_ENOSYS;
        }
        else
        {
            SC200AI_DEBUG("(%s):s0 find i2c_bus:%s\n", SC200AI->i2c_name);
        }
    }
    else
    {
        ret = RET_SYS_ENOSYS;
    }
#endif

    SC200AI_DEBUG("(%s) exit \n", __FUNCTION__);
    return ret;

}

static ret_err_t rk_SC200AI_open(struct rk_camera_device *dev, rt_uint16_t oflag)
{
#if RT_USING_SC200AI_OPS
    struct SC200AI_dev *SC200AI;
#endif
    ret_err_t ret = RET_SYS_EOK;

    SC200AI_DEBUG("(%s) enter \n", __FUNCTION__);

    MACRO_ASSERT(dev != RK_NULL);

#if RT_USING_SC200AI_OPS
    SC200AI = (struct SC200AI_dev *)dev;
    if (SC200AI->ops->open)
    {
        return (SC200AI->ops->open(SC200AI, oflag));
    }
#endif

    SC200AI_DEBUG("(%s) exit \n", __FUNCTION__);

    return ret;
}

ret_err_t rk_SC200AI_close(struct rk_camera_device *dev)
{
#if RT_USING_SC200AI_OPS
    struct SC200AI_dev *SC200AI;
#endif
    uint8_t ret = RET_SYS_EOK;

    SC200AI_INFO(dev, "(%s) enter \n", __FUNCTION__);

    MACRO_ASSERT(dev != RK_NULL);

#if RT_USING_SC200AI_OPS
    SC200AI = (struct SC200AI_dev *)dev;
    if (SC200AI->ops->close)
    {
        return (SC200AI->ops->close(SC200AI));
    }
#endif

    SC200AI_INFO(dev, "(%s) exit \n", __FUNCTION__);
    return ret;

}

static rt_err_t rk_sc200ai_get_expinf(struct SC200AI_dev *dev, struct rk_camera_exp_info *exp)
{
    rt_err_t ret = RET_SYS_EOK;
    struct sc200ai_mode *mode;

    SC200AI_DEBUG("(%s) enter \n", __FUNCTION__);

    MACRO_ASSERT(dev != RK_NULL);

    mode = dev->cur_mode;
    exp->width = mode->width;
    exp->height = mode->height;
    exp->hts = mode->hts_def;
    exp->vts = mode->vts_def;
    exp->pix_clk = (uint64_t)exp->hts * (uint64_t)exp->vts * (uint64_t)mode->max_fps.denominator /
                   (uint64_t)mode->max_fps.numerator;
    exp->time_valid_delay = 2;
    exp->gain_valid_delay = 2;

    exp->dst_width = dev->dst_mode->width;
    exp->dst_height = dev->dst_mode->height;
    exp->dst_hts = dev->dst_mode->hts_def;
    exp->dst_vts = dev->dst_mode->vts_def;
    exp->dst_pix_clk = (uint64_t)exp->dst_hts * (uint64_t)exp->dst_vts *
                       (uint64_t)dev->dst_mode->max_fps.denominator /
                       (uint64_t)dev->dst_mode->max_fps.numerator;

    SC200AI_DEBUG("(%s) exit \n", __FUNCTION__);
    return ret;
}

static rt_err_t rk_sc200ai_get_intput_fmt(struct SC200AI_dev *dev, struct rk_camera_mbus_framefmt *mbus_fmt)
{
    rt_err_t ret = RET_SYS_EOK;
    struct sc200ai_mode *mode;

    SC200AI_DEBUG("(%s) enter \n", __FUNCTION__);

    MACRO_ASSERT(dev != RK_NULL);

    mode = dev->cur_mode;
    mbus_fmt->width = mode->width;
    mbus_fmt->height = mode->height;
    mbus_fmt->field = 0;
    mbus_fmt->pixelcode = MEDIA_BUS_FMT_SBGGR10_1X10;

    SC200AI_DEBUG("(%s) exit \n", __FUNCTION__);
    return ret;

}

static rt_err_t rk_sc200ai_set_intput_fmt(struct SC200AI_dev *dev, struct rk_camera_mbus_framefmt *mbus_fmt)
{
    rt_err_t ret = RET_SYS_EOK;
    struct sc200ai_mode *mode;
    int i = 0;
    bool is_find_fmt = false;
    struct rk_camera_device *camera = (struct rk_camera_device *)&dev->parent;

    SC200AI_DEBUG("(%s) enter \n", __FUNCTION__);

    MACRO_ASSERT(dev != RK_NULL);

    if (mbus_fmt->width == dev->dst_mode->width &&
            mbus_fmt->height == dev->dst_mode->height)
    {
        mode = dev->dst_mode;
        is_find_fmt = true;
    }
    else
    {
        for (i = 0; i < ARRAY_SIZE(supported_modes); i++)
        {
            mode = &supported_modes[i];
            if (mbus_fmt->width == mode->width &&
                    mbus_fmt->height == mode->height)
            {
                is_find_fmt = true;
                break;
            }
        }
    }
    if (is_find_fmt)
    {
        if (mode->width != dev->cur_mode->width)
        {
            dev->cur_mode = mode;
            rt_mutex_take(&dev->mutex_lock, RT_WAITING_FOREVER);

            rk_kprintf("switch to dst fmt, dst_width %d, dst_height %d, dst_fps %d, hdr: %d, dst_vts: 0x%x\n",
                       dev->cur_mode->width, dev->cur_mode->height,
                       dev->cur_mode->max_fps.denominator / dev->cur_mode->max_fps.numerator,
                       dev->cur_mode->hdr_mode, dev->cur_mode->vts_def);

#ifdef I2C_WRITE_CONTINUE
            SC200AI_write_multiple_reg_continue((struct SC200AI_dev *)dev, dev->cur_mode->reg_list, dev->cur_mode->reg_list_size);
#else
            SC200AI_write_multiple_reg((struct SC200AI_dev *)dev, dev->cur_mode->reg_list, dev->cur_mode->reg_list_size);
#endif
            ret = rk_sc200ai_set_flip_mirror(dev, dev->flip);
            rt_mutex_release(&dev->mutex_lock);
        }
        camera->info.hdr_mode = mode->hdr_mode;
    }

    SC200AI_DEBUG("(%s) exit \n", __FUNCTION__);

    return ret;
}

static rt_err_t rk_sc200ai_match_dst_config(struct SC200AI_dev *dev, struct rk_camera_dst_config *dst_config)
{
    rt_err_t ret = RET_SYS_ENOSYS;
    struct sc200ai_mode *mode;
    int i = 0;
    int cur_fps, dst_fps, cur_vts, dst_vts;

    SC200AI_DEBUG("(%s) enter \n", __FUNCTION__);

    MACRO_ASSERT(dev != RK_NULL);

    dst_fps = dst_config->cam_fps_denominator / dst_config->cam_fps_numerator;
    dev->flip = dst_config->cam_mirror_flip;
    dev->hdr = dst_config->cam_hdr;
    //match current resolution config
    for (i = 0; i < ARRAY_SIZE(supported_modes); i++)
    {
        mode = &supported_modes[i];

        if (dst_config->width == mode->width &&
                dst_config->height == mode->height && dst_config->cam_hdr == mode->hdr_mode)
        {
            dev->dst_mode = mode;
            ret = RET_SYS_EOK;
            SC200AI_INFO(dev, "find match fmt support_mode[%d]\n", i);
            break;
        }
        else
            SC200AI_INFO(dev, "support_modes[%d] is not match\n", i);
    }
    cur_fps = dev->dst_mode->max_fps.denominator / dev->dst_mode->max_fps.numerator;
    cur_vts = dev->dst_mode->vts_def;

    //match fps config
    if (cur_fps == dst_fps)
        return 0;

    if (dst_fps > cur_fps)
    {
        rk_kprintf("Err dst fps is larger than cur fps\n");
        return RET_SYS_EINVAL;
    }
    dst_vts = cur_fps * cur_vts / dst_fps;

    dev->dst_mode->max_fps.denominator = dst_config->cam_fps_denominator;
    dev->dst_mode->max_fps.numerator = dst_config->cam_fps_numerator;
    dev->dst_mode->vts_def = dst_vts;

    SC200AI_DEBUG("(%s) exit \n", __FUNCTION__);

    return ret;
}

ret_err_t rk_SC200AI_control(struct rk_camera_device *dev,
                             int cmd,
                             void *args)
{
    SC200AI_DEBUG("(%s) enter \n", __FUNCTION__);

    ret_err_t ret = RET_SYS_EOK;
    struct SC200AI_dev *SC200AI;

    MACRO_ASSERT(dev != RK_NULL);
    SC200AI = (struct SC200AI_dev *)dev;
#if RT_USING_SC200AI_OPS
    return (SC200AI->ops->control(SC200AI, cmd, args));
#else
    switch (cmd)
    {

    case RK_DEVICE_CTRL_DEVICE_INIT:
    {
        ret = rk_SC200AI_init(dev);
    }
    break;

    case RK_DEVICE_CTRL_CAMERA_STREAM_ON:
    {
        SC200AI_stream_on(SC200AI);
    }
    break;
    case RK_DEVICE_CTRL_CAMERA_STREAM_OFF:
    {
        SC200AI_stream_off(SC200AI);
    }
    break;
    case RK_DEVICE_CTRL_CAMERA_GET_EXP_INF:
    {
        ret = rk_sc200ai_get_expinf(SC200AI, (struct rk_camera_exp_info *)args);
    }
    break;
    case RK_DEVICE_CTRL_CAMERA_SET_EXP_VAL:
    {
        ret = rk_sc200ai_set_expval(SC200AI, (struct rk_camera_exp_val *)args);
    }
    break;
    case RK_DEVICE_CTRL_CAMERA_SET_VTS_VAL:
    {
        ret = rk_sc200ai_set_vts(SC200AI, *(uint32_t *)args);
    }
    break;
    case RK_DEVICE_CTRL_CAMERA_GET_FORMAT:
    {
        ret = rk_sc200ai_get_intput_fmt(SC200AI, (struct rk_camera_mbus_framefmt *)args);
    }
    break;
    case RK_DEVICE_CTRL_CAMERA_SET_FORMAT:
    {
        ret = rk_sc200ai_set_intput_fmt(SC200AI, (struct rk_camera_mbus_framefmt *)args);
    }
    break;
    case RK_DEVICE_CTRL_CID_MATCH_CAM_CONFIG:
    {
        ret = rk_sc200ai_match_dst_config(SC200AI, (struct rk_camera_dst_config *)args);
    }
    break;
    case RK_DEVICE_CTRL_CAMERA_SET_FLIPMIRROR:
    {
        ret = rk_sc200ai_set_flip_mirror(SC200AI, *(uint32_t *)args);
    }
    break;
    case RK_DEVICE_CTRL_CAMERA_STREAM_ON_LATE:
    {
        SC200AI_stream_on_late(SC200AI);
    }
    break;
    default:
        SC200AI_DEBUG("(%s) exit CMD %d\n", __FUNCTION__, cmd);
        break;
    }
#endif
    SC200AI_DEBUG("(%s) exit \n", __FUNCTION__);
    return ret;
}

struct rk_camera_ops rk_SC200AI_ops =
{
    rk_SC200AI_init,
    rk_SC200AI_open,
    NULL,
    rk_SC200AI_control
};


int rk_camera_SC200AI_init(void)
{
    ret_err_t ret = RET_SYS_EOK;
    struct SC200AI_dev *instance = &g_SC200AI;
    struct rk_camera_device *camera = &instance->parent;
    struct clk_gate *clkgate;

    camera->ops = &rk_SC200AI_ops;

    rt_strncpy(instance->name, SC200AI_DEVICE_NAME, rt_strlen(SC200AI_DEVICE_NAME));
    rt_strncpy(instance->i2c_name, I2C_BUS_NAME, rt_strlen(I2C_BUS_NAME));

#if SC200AI_I2C_DEBUG_ENABLE
    instance->i2c_bus = (rk_i2c_bus_device *)rt_device_find(instance->i2c_name);
    if (!instance->i2c_bus)
    {
        SC200AI_INFO(instance, "Warning:not find i2c source 3:%s !!!\n", instance->i2c_name);
        ret = RET_SYS_ENOSYS;
        goto ERR;
    }
    else
    {
        SC200AI_DEBUG("(%s):s0.0 find i2c_bus\n");
    }
#endif

    instance->cur_mode = &supported_modes[0];
    instance->dst_mode = &supported_modes[1];
    instance->flip = 0;
    instance->hdr = 0;

    HAL_GPIO_SetPinDirection(CAMERA_RST_GPIO_GROUP, CAMERA_RST_GPIO_PIN, GPIO_OUT);
    HAL_GPIO_SetPinLevel(CAMERA_RST_GPIO_GROUP, CAMERA_RST_GPIO_PIN, 1);
    clk_set_rate(CAMERA_CLK_REF, 27000000);
    clkgate = get_clk_gate_from_id(CAMERA_CLK_REF);
    clk_enable(clkgate);

    rt_mutex_init(&instance->mutex_lock, "SC200AI_mutex", RT_IPC_FLAG_FIFO);
    MACRO_ASSERT(rt_object_get_type(&instance->mutex_lock.parent.parent) == RT_Object_Class_Mutex);
    camera->i2c_bus = instance->i2c_bus;
    rt_strncpy(camera->name, instance->name, rt_strlen(SC200AI_DEVICE_NAME));
    rk_camera_register(camera, camera->name, instance);
    SC200AI_DEBUG("(%s) exit \n", __FUNCTION__);
    return ret;
}

void SC200AI_detect(void)
{
    struct SC200AI_dev *instance = &g_SC200AI;

    SC200AI_DEBUG("start to detect SC200AI for testing \n");
    SC200AI_DEBUG("dev name:%s\n", instance->name);
    SC200AI_DEBUG("dev i2c_bus:%s\n", instance->i2c_name);
    instance->i2c_bus = (rk_i2c_bus_device *)rt_device_find(instance->i2c_name);
    if (!instance->i2c_bus)
    {
        SC200AI_DEBUG("Warning:not find i2c source 1:%s !!!\n", instance->i2c_name);
        return;
    }
    else
    {
        SC200AI_DEBUG("(%s):s0 find i2c_bus:%s\n", instance->i2c_name);
    }

}
#if defined(__RT_THREAD__)
INIT_DEVICE_EXPORT(rk_camera_SC200AI_init);
#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(SC200AI_detect, check SC200AI is available or not);
#endif
#endif
#endif


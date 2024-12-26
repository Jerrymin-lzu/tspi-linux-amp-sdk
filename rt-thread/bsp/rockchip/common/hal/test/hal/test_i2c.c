/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2021 Rockchip Electronics Co., Ltd.
 */

#include "hal_bsp.h"
#include "hal_base.h"
#include "unity.h"
#include "unity_fixture.h"

#if defined(HAL_I2C_MODULE_ENABLED)
/*************************** I2C DRIVER ****************************/

/***************************** MACRO Definition ******************************/
#define ROCKCHIP_I2C_TIMEOUT (1 * 1000)

#define I2C_BUS_MAX 6

struct I2C_DEVICE_CLASS {
    /* error status */
    uint32_t error;

    /* Hal */
    struct I2C_HANDLE instance;
    const struct HAL_I2C_DEV *halDev;
    uint32_t state;
};

/***************************** Structure Definition **************************/

/***************************** Function Declare ******************************/

/********************* Private MACRO Definition ******************************/

#define DEFINE_ROCKCHIP_I2C(ID)          \
                                         \
                                         \
struct I2C_DEVICE_CLASS g_I2C##ID##Dev = \
{                                        \
    .halDev = &g_i2c##ID##Dev,           \
};                                       \

#define ROCKCHIP_I2C(ID) g_I2C##ID##Dev

/********************* Private Structure Definition **************************/

/********************* Private Variable Definition ***************************/
/* Define I2C resource */
#ifdef HAL_I2C0
DEFINE_ROCKCHIP_I2C(0)
#endif
#ifdef HAL_I2C1
DEFINE_ROCKCHIP_I2C(1)
#endif
#ifdef HAL_I2C2
DEFINE_ROCKCHIP_I2C(2)
#endif
#ifdef HAL_I2C3
DEFINE_ROCKCHIP_I2C(3)
#endif
#ifdef HAL_I2C4
DEFINE_ROCKCHIP_I2C(4)
#endif
#ifdef HAL_I2C5
DEFINE_ROCKCHIP_I2C(5)
#endif

/* Add I2C resource to group */
struct I2C_DEVICE_CLASS *gI2CDev[I2C_BUS_MAX] =
{
#ifdef HAL_I2C0
    &ROCKCHIP_I2C(0),
#else
    NULL,
#endif
#ifdef HAL_I2C1
    &ROCKCHIP_I2C(1),
#else
    NULL,
#endif
#ifdef HAL_I2C2
    &ROCKCHIP_I2C(2),
#else
    NULL,
#endif
#ifdef HAL_I2C3
    &ROCKCHIP_I2C(3),
#else
    NULL,
#endif
#ifdef HAL_I2C4
    &ROCKCHIP_I2C(4),
#else
    NULL,
#endif
#ifdef HAL_I2C5
    &ROCKCHIP_I2C(5),
#else
    NULL,
#endif
};

/********************* Private Function Definition ***************************/
/********************* Public Function Definition ****************************/

static HAL_Status I2C_Configure(struct I2C_DEVICE_CLASS *i2c, struct I2C_MSG *msgs,
                                int32_t num)
{
    uint32_t addr = (msgs[0].addr & 0x7f) << 1;
    struct I2C_HANDLE *pI2C = &i2c->instance;
    struct I2C_MSG *message;
    int32_t ret = 0;

    if (num >= 2 && msgs[0].len < 4 &&
        !(msgs[0].flags & HAL_I2C_M_RD) && (msgs[1].flags & HAL_I2C_M_RD)) {
        uint32_t regAddr = 0;
        int i;

        for (i = 0; i < msgs[0].len; ++i) {
            regAddr |= msgs[0].buf[i] << (i * 8);
            regAddr |= HAL_I2C_REG_MRXADDR_VALID(i);
        }

        addr |= HAL_I2C_REG_MRXADDR_VALID(0);
        HAL_I2C_ConfigureMode(pI2C, REG_CON_MOD_REGISTER_TX, addr, regAddr);

        message = &msgs[1];
        ret = 2;
    }
    else {
        if (msgs[0].flags & HAL_I2C_M_RD) {
            addr |= 1; /* set read bit */
            addr |= HAL_I2C_REG_MRXADDR_VALID(0);

            HAL_I2C_ConfigureMode(pI2C, REG_CON_MOD_REGISTER_TX, addr, 0);
        }
        else {
            HAL_I2C_ConfigureMode(pI2C, REG_CON_MOD_TX, 0, 0);
        }

        message = &msgs[0];
        ret = 1;
    }

    HAL_I2C_SetupMsg(pI2C, msgs[0].addr, message->buf, message->len, message->flags);

    return ret;
}

static HAL_Status I2C_Xfer(uint8_t id, struct I2C_MSG msgs[], uint32_t num)
{
    struct I2C_DEVICE_CLASS *i2c = (struct I2C_DEVICE_CLASS *)gI2CDev[id];
    struct I2C_HANDLE *pI2C = &i2c->instance;
    int32_t i, ret = 0;
    uint32_t start;

    for (i = 0; i < num; i += ret) {
        uint32_t tmo = ROCKCHIP_I2C_TIMEOUT;
        bool last = false;

        ret = I2C_Configure(i2c, msgs + i, num - i);
        if (ret <= 0) {
            HAL_DBG("i2c_setup() failed\n");
            break;
        }

        /* Is it the last message? */
        if (i + ret >= num)
            last = true;

        HAL_I2C_Transfer(pI2C, I2C_POLL, last);
        start = HAL_GetTick();
        do {
            i2c->error = HAL_I2C_IRQHandler(pI2C);
            if (i2c->error != HAL_BUSY)
                break;

            if ((HAL_GetTick() - start) > ROCKCHIP_I2C_TIMEOUT) {
                ret = HAL_TIMEOUT;
                break;
            }
        } while (true);

        /* timeout to force stop */
        if (ret == HAL_TIMEOUT) {
            HAL_I2C_ForceStop(pI2C);
            break;
        }

        if (i2c->error) {
            ret = i2c->error;
            break;
        }
    }

    HAL_I2C_Close(pI2C);

    return ret < 0 ? ret : num;
}

static HAL_Status I2C_Init(uint8_t id)
{
    struct I2C_DEVICE_CLASS *i2c;
    const struct HAL_I2C_DEV *i2cDev;
    uint32_t freq;

    HAL_ASSERT(id < I2C_BUS_MAX);

    i2c = gI2CDev[id];
    if (!i2c) {
        return HAL_ERROR;
    }

    i2cDev = i2c->halDev;
    /* Get clock rate here. */
    freq = HAL_CRU_ClkGetFreq(i2cDev->clkID);
    HAL_I2C_Init(&i2c->instance, i2cDev->pReg, freq, I2C_100K);

    return 0;
}

/*************************** I2C TEST ****************************/
#define I2C_MAX_DEVICES 16
static uint16_t i2c_devices[I2C_MAX_DEVICES];

TEST_GROUP(HAL_I2C);

TEST_SETUP(HAL_I2C){
}

TEST_TEAR_DOWN(HAL_I2C){
}

/* I2C test case 0 */
static int32_t I2C_WriteTest(uint8_t id, uint16_t addr,
                             uint8_t *data_buf,
                             uint16_t data_len)
{
    struct I2C_MSG msgs[1];
    int32_t ret;

    HAL_ASSERT(id < I2C_BUS_MAX);

    msgs[0].addr  = addr;
    msgs[0].flags = HAL_I2C_M_WR;
    msgs[0].buf   = data_buf;
    msgs[0].len   = data_len;

    ret = I2C_Xfer(id, msgs, 1);

    return ret;
}

/* I2C test case 1 */
static int32_t I2C_ReadTest(uint8_t id, uint16_t addr, uint8_t *cmd_buf,
                            uint16_t cmd_len, uint8_t *data_buf,
                            uint16_t data_len)
{
    struct I2C_MSG msgs[2];
    int32_t ret;

    HAL_ASSERT(id < I2C_BUS_MAX);

    msgs[0].addr  = addr;
    msgs[0].flags = HAL_I2C_M_WR;
    msgs[0].buf   = cmd_buf;
    msgs[0].len   = cmd_len;

    msgs[1].addr  = addr;
    msgs[1].flags = HAL_I2C_M_RD;
    msgs[1].buf   = data_buf;
    msgs[1].len   = data_len;

    ret = I2C_Xfer(id, msgs, 2);

    return ret;
}

/* I2C test case 2 */
static uint32_t I2C_ScanDevicesTest(uint8_t id)
{
    uint32_t i, j, ret, deviceID;
    int first = 0x03, last = 0x77;
    struct I2C_MSG msgs[1];
    uint8_t cmd = 0;

    HAL_ASSERT(id < I2C_BUS_MAX);

    deviceID = 0;
    memset(i2c_devices, 0, sizeof(i2c_devices));
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    for (i = 0; i < 128; i += 16) {
        printf("%02x: ", i);
        for(j = 0; j < 16; j++) {
            /* Skip unwanted addresses */
            if (i + j < first || i + j > last) {
                printf("   ");
                continue;
            }

            msgs[0].addr  = i + j;
            msgs[0].flags = HAL_I2C_M_RD;
            msgs[0].buf   = &cmd;
            msgs[0].len   = 0;
            ret = I2C_Xfer(id, msgs, 1);
            if (ret != 1) {
                printf("-- ");
            } else {
                printf("%02x ", i + j);
                i2c_devices[deviceID] = i + j;
                deviceID++;
            }
        }
        printf("\n");
    }

    return deviceID;
}

/* I2C test case 3 */
static uint32_t I2C_DumpDevicesTest(uint8_t id, uint16_t addr)
{
    uint32_t i, j, ret;
    uint8_t cmd, data;

    HAL_ASSERT(id < I2C_BUS_MAX);

    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    for (i = 0; i < 256; i+=16) {
        printf("%02x: ", i);
        for(j = 0; j < 16; j++) {
            cmd = i + j;
            ret = I2C_ReadTest(id, addr, &cmd, 1, &data, 1);
            if (ret == 2)
                printf("%02x ", data);
            else
                printf("XX ");
        }
        printf("\n");
    }

    return 0;
}

TEST_GROUP_RUNNER(HAL_I2C) {
    int32_t bus, id, i = 0, ret;

    HAL_DBG("\n");
    HAL_DBG("%s\n", __func__);
    HAL_DBG("Note:\n");
    HAL_DBG("    cpu polling transfer\n");
    HAL_DBG("    Default tests have no tx transfer\n");

    for (bus = 0; bus < I2C_BUS_MAX; bus++) {
        if(I2C_Init(bus))
            continue;

        id = I2C_ScanDevicesTest(bus);
        if (id) {
            for (i = id; i < id; i++) {
                printf("Device%d-%02x: \n", i, i2c_devices[i]);
                /* Option test */
                I2C_DumpDevicesTest(bus, i2c_devices[i]);

            }
	}
    }
}

#endif

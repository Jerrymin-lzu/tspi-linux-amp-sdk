/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2022 Rockchip Electronics Co., Ltd.
 */

#include "hal_bsp.h"
#include "hal_base.h"
#include "unity.h"
#include "unity_fixture.h"
#include "test_spi.h"

#if defined(HAL_SPI_MODULE_ENABLED) && defined(UNITY_HAL_SPI)
/*************************** SPI DRIVER ****************************/

/***************************** MACRO Definition ******************************/
#define SPI_MAX_SCLK_RATE 200000000

/***************************** Structure Definition **************************/

struct SPI_DEVICE_CLASS {
    /* status */
    uint32_t error;

    /* Hal */
    struct SPI_HANDLE instance;
    const struct HAL_SPI_DEV *halDev;
    uint32_t state;

#ifdef HAL_PL330_MODULE_ENABLED
    /* dma */
    uint32_t dmaBurstSize;
    struct PL330_CHAN *dmaRxChan;
    struct PL330_CHAN *dmaTxChan;
#endif
};

/***************************** Function Declare ******************************/

/********************* Private MACRO Definition ******************************/

#define DEFINE_ROCKCHIP_SPI(ID)       \
                                      \
                                      \
struct SPI_DEVICE_CLASS gSpiDev##ID = \
{                                     \
    .halDev = &g_spi##ID##Dev,          \
};                                    \

#define ROCKCHIP_SPI(ID) gSpiDev##ID

#define RXBUSY (1 << 0)
#define TXBUSY (1 << 1)

#define ROCKCHIP_SPI_TX_IDLE_TIMEOUT 20

/********************* Private Structure Definition **************************/

/********************* Private Variable Definition ***************************/
#ifdef HAL_PL330_MODULE_ENABLED
static struct HAL_PL330_DEV *s_pl330;
static uint8_t mcrTxBuf[PL330_CHAN_BUF_LEN];
static uint8_t mcrRxBuf[PL330_CHAN_BUF_LEN];
#endif

/* Define SPI resource */
#ifdef SPI0
DEFINE_ROCKCHIP_SPI(0)
#endif
#ifdef SPI1
DEFINE_ROCKCHIP_SPI(1)
#endif
#ifdef SPI2
DEFINE_ROCKCHIP_SPI(2)
#endif
#ifdef SPI3
DEFINE_ROCKCHIP_SPI(3)
#endif

/* Add SPI resource to group */
struct SPI_DEVICE_CLASS *gSpiDev[SPI_DEVICE_MAX] =
{
#ifdef SPI0
    &ROCKCHIP_SPI(0),
#else
    NULL,
#endif
#ifdef SPI1
    &ROCKCHIP_SPI(1),
#else
    NULL,
#endif
#ifdef SPI2
    &ROCKCHIP_SPI(2),
#else
    NULL,
#endif
#ifdef SPI3
    &ROCKCHIP_SPI(3),
#else
    NULL,
#endif
};

/********************* Private Function Definition ***************************/

/********************* Public Function Definition ****************************/

HAL_Status SPI_Configure(uint8_t id, struct RK_SPI_CONFIG *configuration)
{
    struct SPI_DEVICE_CLASS *spi = (struct SPI_DEVICE_CLASS *)gSpiDev[id];
    struct SPI_HANDLE *pSPI = &spi->instance;
    struct SPI_CONFIG *pSPIConfig = &pSPI->config;

    /* Data width */
    if (configuration->dataWidth <= 8) {
        pSPIConfig->nBytes = CR0_DATA_FRAME_SIZE_8BIT;
    } else if (configuration->dataWidth <= 16) {
        pSPIConfig->nBytes = CR0_DATA_FRAME_SIZE_16BIT;
    } else {
        return HAL_ERROR;
    }

    /* CPOL */
    if (configuration->mode & RK_SPI_CPOL) {
        pSPIConfig->clkPolarity = CR0_POLARITY_HIGH;
    } else {
        pSPIConfig->clkPolarity = CR0_POLARITY_LOW;
    }

    /* CPHA */
    if (configuration->mode & RK_SPI_CPHA) {
        pSPIConfig->clkPhase = CR0_PHASE_2EDGE;
    } else {
        pSPIConfig->clkPhase = CR0_PHASE_1EDGE;
    }

    /* MSB or LSB */
    if (configuration->mode & RK_SPI_MSB) {
        pSPIConfig->firstBit = CR0_FIRSTBIT_MSB;
    } else {
        pSPIConfig->firstBit = CR0_FIRSTBIT_LSB;
    }

    /* Master or Slave */
    if (configuration->mode & RK_SPI_SLAVE) {
        pSPIConfig->opMode = CR0_OPM_SLAVE;
    } else {
        pSPIConfig->opMode = CR0_OPM_MASTER;
    }

    /* CSM cycles */
    pSPIConfig->csm = CR0_CSM((configuration->mode & RK_SPI_CSM_MASK) >> RK_SPI_CSM_SHIFT);

#ifdef HAL_CRU_MODULE_ENABLED
    pSPI->maxFreq = HAL_CRU_ClkGetFreq(spi->halDev->clkId);
    pSPIConfig->speed = configuration->maxHz;

    if (pSPIConfig->opMode == CR0_OPM_MASTER) {
        if (pSPI->config.speed > HAL_SPI_MASTER_MAX_SCLK_OUT) {
            pSPI->config.speed = HAL_SPI_MASTER_MAX_SCLK_OUT;
        }

        /* the master minimum divisor is 2 */
        if (pSPI->maxFreq < 2 * pSPI->config.speed) {
            HAL_CRU_ClkSetFreq(spi->halDev->clkId, 2 * pSPI->config.speed);
            pSPI->maxFreq = HAL_CRU_ClkGetFreq(spi->halDev->clkId);
            HAL_DBG("SPI SCLK is in maxFreq %ldHz in master mode\n", pSPI->maxFreq);
        }
    } else {
        if (pSPI->config.speed > HAL_SPI_SLAVE_MAX_SCLK_OUT) {
            pSPI->config.speed = HAL_SPI_SLAVE_MAX_SCLK_OUT;
        }

        /* the slave minimum divisor is 6 */
        if (pSPI->maxFreq < 6 * pSPI->config.speed) {
            HAL_CRU_ClkSetFreq(spi->halDev->clkId, 6 * pSPI->config.speed);
            pSPI->maxFreq = HAL_CRU_ClkGetFreq(spi->halDev->clkId);
            HAL_DBG("SPI SCLK is in maxFreq %ldHz in slave mode\n", pSPI->maxFreq);
        }
    }
#else
    pSPI->maxFreq = SPI_MAX_SCLK_RATE;
    pSPIConfig->speed = configuration->maxHz;
#endif

    return HAL_OK;
}

#ifdef HAL_PL330_MODULE_ENABLED
static uint32_t SPI_CalcBurstSize(uint32_t data_len)
{
    uint32_t i;

    /* burst size: 1, 2, 4, 8 */
    for (i = 1; i < 8; i <<= 1) {
        if (data_len & i) {
            break;
        }
    }

    /* DW_DMA is not support burst 2 */
    if (i == 2) {
        i = 1;
    }

    return i;
}

static void SPI_DmaRxCb(void *data)
{
    struct SPI_DEVICE_CLASS *spi = data;

    spi->state &= ~RXBUSY;
}

static void SPI_DmaTxCb(void *data)
{
    struct SPI_DEVICE_CLASS *spi = data;

    spi->state &= ~TXBUSY;
}

static int SPI_DmaPrepare(struct SPI_DEVICE_CLASS *spi, struct RK_SPI_MESSAGE *message)
{
    struct SPI_HANDLE *pSPI = &spi->instance;
    int ret = 0;
    struct DMA_SLAVE_CONFIG sConfig;

    spi->state &= ~RXBUSY;
    spi->state &= ~TXBUSY;

    /* Configure rx firstly. */
    if (message->recvBuf) {
        spi->dmaRxChan = HAL_PL330_RequestChannel(s_pl330, spi->halDev->rxDma.channel);

        sConfig.direction = spi->halDev->rxDma.direction;
        sConfig.srcAddr = spi->halDev->rxDma.addr;
        sConfig.dstAddr = (uint32_t)message->recvBuf;
        sConfig.srcAddrWidth = pSPI->config.nBytes;
        sConfig.dstAddrWidth = pSPI->config.nBytes;
        sConfig.srcMaxBurst = spi->dmaBurstSize;
        sConfig.dstMaxBurst = spi->dmaBurstSize;
        HAL_PL330_Config(spi->dmaRxChan, &sConfig);
        HAL_PL330_PrepDmaSingle(spi->dmaRxChan, (uint32_t)message->recvBuf, message->length, spi->halDev->rxDma.direction,
                                SPI_DmaRxCb, spi);
        HAL_PL330_SetMcBuf(spi->dmaRxChan, &mcrRxBuf);
    }

    if (message->sendBuf) {
        spi->dmaTxChan = HAL_PL330_RequestChannel(s_pl330, spi->halDev->txDma.channel);

        sConfig.direction = spi->halDev->txDma.direction;
        sConfig.srcAddr = (uint32_t)message->sendBuf;
        sConfig.dstAddr = spi->halDev->txDma.addr;
        sConfig.srcAddrWidth = pSPI->config.nBytes;
        sConfig.dstAddrWidth = pSPI->config.nBytes;
        sConfig.srcMaxBurst = 8;
        sConfig.dstMaxBurst = 8;
        HAL_PL330_Config(spi->dmaTxChan, &sConfig);
        HAL_PL330_PrepDmaSingle(spi->dmaTxChan, (uint32_t)message->sendBuf, message->length, spi->halDev->txDma.direction,
                                SPI_DmaTxCb, spi);
        HAL_PL330_SetMcBuf(spi->dmaTxChan, &mcrTxBuf);
#ifdef HAL_DCACHE_MODULE_ENABLED
        HAL_DCACHE_CleanByRange((uint32_t)message->sendBuf, message->length);
#endif
    }

    if (message->recvBuf) {
        spi->state |= RXBUSY;

        ret = HAL_PL330_Start(spi->dmaRxChan);
    }

    if (message->sendBuf) {
        spi->state |= TXBUSY;

        ret = HAL_PL330_Start(spi->dmaTxChan);
    }

    return ret;
}
#endif

static uint32_t SPI_ReadAndWrite(uint8_t id, struct RK_SPI_MESSAGE *message)
{
    struct SPI_DEVICE_CLASS *spi = (struct SPI_DEVICE_CLASS *)gSpiDev[id];
    struct SPI_HANDLE *pSPI = &spi->instance;
    uint64_t timeout;
    HAL_Status ret = HAL_OK;
#ifdef HAL_PL330_MODULE_ENABLED
    uint32_t start, timeoutMs;
#endif

    HAL_ASSERT((message->sendBuf != NULL) || (message->recvBuf != NULL));

    /* Configure spi mode here. */
    HAL_SPI_Configure(pSPI, message->sendBuf, message->recvBuf, message->length);

    spi->error = 0;
    if (message->csTake) {
        HAL_SPI_SetCS(pSPI, message->ch, true);
    }

    /* Use poll mode for master while less fifo length. */
#ifdef HAL_PL330_MODULE_ENABLED
    if (HAL_SPI_CanDma(pSPI)) {
        spi->dmaBurstSize = 1;
        pSPI->dmaBurstSize = spi->dmaBurstSize;
        HAL_SPI_DmaTransfer(pSPI);
        SPI_DmaPrepare(spi, message);
        timeoutMs = HAL_SPI_CalculateTimeout(&spi->instance);
        if (message->sendBuf) {
            start = HAL_GetTick();
            do {
                if ((HAL_GetTick() - start) > timeoutMs) {
                    HAL_DBG_ERR("%s dma tx timeout\n", __func__);

                    ret = HAL_TIMEOUT;
                    break;
                }

                /* If Interrupt is disabled, poll dma status */
                if (HAL_PL330_GetPosition(spi->dmaTxChan) == message->length) {
                    spi->state &= ~TXBUSY;
                }
            } while (spi->state &= TXBUSY);
        }

        if (message->recvBuf) {
            start = HAL_GetTick();
            do {
                if ((HAL_GetTick() - start) > timeoutMs) {
                    HAL_DBG_ERR("%s dma rx timeout\n", __func__);

                    ret = HAL_TIMEOUT;
                    break;
                }

                /* If Interrupt is disabled, poll dma status */
                if (HAL_PL330_GetPosition(spi->dmaRxChan) == message->length) {
                    spi->state &= ~RXBUSY;
                }
            } while (spi->state &= RXBUSY);
        }

        if (message->sendBuf) {
            HAL_PL330_Stop(spi->dmaTxChan);
            HAL_PL330_ReleaseChannel(spi->dmaTxChan);
        }

        if (message->recvBuf) {
#ifdef HAL_DCACHE_MODULE_ENABLED
            HAL_DCACHE_CleanByRange((uint32_t)message->sendBuf, message->length);
#endif
            HAL_PL330_Stop(spi->dmaRxChan);
            HAL_PL330_ReleaseChannel(spi->dmaRxChan);
        }
    } else {
#else
    if (1) {
#endif
        HAL_SPI_PioTransfer(pSPI);
        /* If tx, wait until the FIFO data completely. */
        if (message->sendBuf) {
            timeout = HAL_GetTick() + ROCKCHIP_SPI_TX_IDLE_TIMEOUT; /* some tolerance */
            do {
                ret = HAL_SPI_QueryBusState(pSPI);
                if (ret == HAL_OK) {
                    break;
                }
            } while (timeout > HAL_GetTick());
        }
    }

    if (HAL_OK != ret) {
        spi->error = ret;
        HAL_DBG("%s error\n", __func__);
    }

    /* Disable SPI when finished. */
    HAL_SPI_Stop(pSPI);

    if (message->csRelease) {
        HAL_SPI_SetCS(pSPI, message->ch, false);
    }

    /* Successful to return message length and fail to return 0. */
    return spi->error ? 0 :  message->length;
}

static uint32_t SPI_Transfer(uint8_t id, uint8_t ch, const void *sendBuf, void *recvBuf, uint32_t length)
{
    uint32_t ret;
    struct RK_SPI_MESSAGE message;

    /* initial message */
    message.ch = ch;
    message.sendBuf = sendBuf;
    message.recvBuf = recvBuf;
    message.length = length;
    message.csTake = 1;
    message.csRelease = 1;

    ret = SPI_ReadAndWrite(id, &message);

    return ret;
}

uint32_t SPI_Read(uint8_t id, uint8_t ch, void *recvBuf, uint32_t length)
{
    HAL_ASSERT(id < SPI_DEVICE_MAX);

    return SPI_Transfer(id, ch, NULL, recvBuf, length);
}

uint32_t SPI_Write(uint8_t id, uint8_t ch, const void *sendBuf, uint32_t length)
{
    HAL_ASSERT(id < SPI_DEVICE_MAX);

    return SPI_Transfer(id, ch, sendBuf, NULL, length);
}

HAL_Status SPI_SendThenSend(uint8_t id, uint8_t ch, const void *sendBuf0, uint32_t len0, const void *sendBuf1, uint32_t len1)
{
    HAL_Status ret = HAL_OK;
    struct RK_SPI_MESSAGE message;

    HAL_ASSERT(id < SPI_DEVICE_MAX);

    /* initial first send message */
    message.ch = ch;
    message.sendBuf = sendBuf0;
    message.recvBuf = NULL;
    message.length = len0;
    message.csTake = 1;
    message.csRelease = 0;

    ret = SPI_ReadAndWrite(id, &message);
    if (ret != len0) {
        ret = HAL_ERROR;

        goto out;
    }

    /* initial second send message */
    message.ch = ch;
    message.sendBuf = sendBuf1;
    message.recvBuf = NULL;
    message.length = len1;
    message.csTake = 0;
    message.csRelease = 1;

    ret = SPI_ReadAndWrite(id, &message);
    if (ret != len1) {
        ret = HAL_ERROR;
    }

out:

    return ret;
}

HAL_Status SPI_SendThenRecv(uint8_t id, uint8_t ch, const void *sendBuf, uint32_t len0, void *recvBuf, uint32_t len1)
{
    HAL_Status ret = HAL_OK;
    struct RK_SPI_MESSAGE message;

    HAL_ASSERT(id < SPI_DEVICE_MAX);

    /* initial first send message */
    message.ch = ch;
    message.sendBuf = sendBuf;
    message.recvBuf = NULL;
    message.length = len0;
    message.csTake = 1;
    message.csRelease = 0;

    if (SPI_ReadAndWrite(id, &message) != len0) {
        ret = HAL_ERROR;

        goto out;
    }

    /* initial second send message */
    message.ch = ch;
    message.sendBuf = NULL;
    message.recvBuf = recvBuf;
    message.length = len1;
    message.csTake = 0;
    message.csRelease = 1;

    if (SPI_ReadAndWrite(id, &message) != len1) {
        ret = HAL_ERROR;
    }

out:

    return ret;
}

HAL_Status SPI_Init(uint8_t id)
{
    struct SPI_DEVICE_CLASS *spi;
    struct RK_SPI_CONFIG config;

    HAL_ASSERT(id < SPI_DEVICE_MAX);

    memset(&config, 0, sizeof(struct RK_SPI_CONFIG));

    spi = gSpiDev[id];
    if (!spi) {
        return HAL_ERROR;
    }

    HAL_SPI_Init(&spi->instance, (uint32_t)spi->halDev->base, spi->halDev->isSlave);

    /* Pre-config */
    config.mode |= RK_SPI_MASTER;
    config.dataWidth = 8;
    config.maxHz = ROCKCHIP_SPI_SPEED_DEFAULT;
    SPI_Configure(id, &config);

    return HAL_OK;
}

/*************************** SPI TEST ****************************/
/* Test-config */
#define SPI_TEST_ID 1

#define SPI_TEST_SIZE 4096
static uint8_t tx_buf[2 * SPI_TEST_SIZE];
static uint8_t rx_buf[2 * SPI_TEST_SIZE];
static uint8_t *tx;
static uint8_t *rx;

TEST_GROUP(HAL_SPI);

TEST_SETUP(HAL_SPI){
}

TEST_TEAR_DOWN(HAL_SPI){
}

/* SPI test case 0 */
static void SPI_LoopTest(uint16_t size)
{
    uint32_t i, ret;

    HAL_DBG("SPI%d loop test, size=%d\n", SPI_TEST_ID, size);

    for (i = 0; i < SPI_TEST_SIZE / 4; i++) {
        ((uint32_t *)tx)[i] = (size << 16) | i;
    }
    tx[0] = 0xa5;
    memset(rx, 0, SPI_TEST_SIZE);
    ret = SPI_Transfer(SPI_TEST_ID, 0, (const void *)tx, (void *)rx, size);
    TEST_ASSERT(ret == size);

    for (i = 0; i < size; i++) {
        if (tx[i] != rx[i]) {
            HAL_DBG_ERR("It's a spi loop test, so connecting mosi and miso in advance!\n");
            HAL_DBG_HEX("w:", tx, 4, size / 4);
            HAL_DBG_HEX("r:", rx, 4, size / 4);
            TEST_ASSERT(0);
            break;
        }
    }
}

/* SPI test case 1 */
static void SPI_WriteTest(uint16_t size)
{
    uint32_t i, ret;

    HAL_DBG("SPI%d write test, size=%d\n", SPI_TEST_ID, size);

    for (i = 0; i < SPI_TEST_SIZE / 4; i++) {
        ((uint32_t *)tx)[i] = (size << 16) | i;
    }
    ret = SPI_Write(SPI_TEST_ID, 0, (const void *)tx, size);
    TEST_ASSERT(ret == size);
}

/* SPI test case 2 */
static void SPI_ReadTest(uint16_t size)
{
    uint32_t ret;

    HAL_DBG("SPI%d read test, size=%d\n", SPI_TEST_ID, size);

    memset(rx, 0, SPI_TEST_SIZE);
    ret = SPI_Read(SPI_TEST_ID, 0, (void *)rx, size);

    HAL_DBG_HEX("r:", rx, 4, 4);
    TEST_ASSERT(ret == size);
}

TEST_GROUP_RUNNER(HAL_SPI){
#ifdef HAL_PL330_MODULE_ENABLED
    struct HAL_PL330_DEV *pl330 = &g_pl330Dev0;

    HAL_PL330_Init(pl330);
    s_pl330 = pl330;
#endif

    HAL_DBG("\n");
    HAL_DBG("%s\n", __func__);
    HAL_DBG("Note:\n");
    HAL_DBG("    dma transfer        : size > HAL_SPI_DMA_SIZE_MIN\n");
    HAL_DBG("    cpu polling transfer: size <= HAL_SPI_DMA_SIZE_MIN\n");
    HAL_DBG("    HAL_SPI_DMA_SIZE_MIN currently equals 512B\n");
    HAL_DBG("    Test SPI%d cs0!!!!\n", SPI_TEST_ID);

    tx = (uint8_t *)(((uint32_t)&tx_buf + 0x3f) & (~0x3f));
    rx = (uint8_t *)(((uint32_t)&rx_buf + 0x3f) & (~0x3f));

    SPI_Init(SPI_TEST_ID);

    HAL_DBG("If loop at \"[HAL INFO] SPI0 loop test, size=1\", check iomux and clock enable\n");
    SPI_LoopTest(1);
    SPI_LoopTest(31);
    SPI_LoopTest(32);
    SPI_LoopTest(128);
    SPI_LoopTest(4095);
    SPI_LoopTest(4096);
    SPI_WriteTest(32);
    SPI_WriteTest(1024);
    SPI_ReadTest(32);
    SPI_ReadTest(1024);
}

#endif

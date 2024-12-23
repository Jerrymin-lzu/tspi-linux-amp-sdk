/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2020 Rockchip Electronics Co., Ltd.
 */

#include "hal_bsp.h"
#include "hal_base.h"
#include "unity.h"
#include "unity_fixture.h"

#ifdef HAL_PL330_MODULE_ENABLED

#define TSIZE 64
#define TEST_CHANNEL 0

static struct HAL_PL330_DEV *s_pl330;
__ALIGNED(64) static uint8_t src[TSIZE];
__ALIGNED(64) static uint8_t dst[TSIZE];
__ALIGNED(64) static uint8_t buf[PL330_CHAN_BUF_LEN];

TEST_GROUP(HAL_PL330);

TEST_SETUP(HAL_PL330){
}

TEST_TEAR_DOWN(HAL_PL330){
}

#if defined(HAL_NVIC_MODULE_ENABLED)
static void HAL_PL330_Handler(void)
{
    HAL_PL330_IrqHandler(s_pl330);
    if (s_pl330->chans[TEST_CHANNEL].desc.callback)
        s_pl330->chans[TEST_CHANNEL].desc.callback(&s_pl330->chans[TEST_CHANNEL]);
}
#elif defined(HAL_GIC_MODULE_ENABLED)
static void HAL_PL330_Handler(uint32_t irq, void *args)
{
    struct HAL_PL330_DEV *pl330 = (struct HAL_PL330_DEV *)args;

    HAL_PL330_IrqHandler(pl330);
    if (pl330->chans[TEST_CHANNEL].desc.callback)
        pl330->chans[TEST_CHANNEL].desc.callback(&pl330->chans[TEST_CHANNEL]);
}
#endif

static void MEMCPY_Callback(void *cparam)
{
    struct PL330_CHAN *pchan = cparam;
    uint32_t ret;

    TEST_ASSERT_EQUAL_MEMORY(src, dst, TSIZE);

    ret = HAL_PL330_Stop(pchan);
    TEST_ASSERT(ret == HAL_OK);

    ret = HAL_PL330_ReleaseChannel(pchan);
    TEST_ASSERT(ret == HAL_OK);
}

TEST(HAL_PL330, MemcpyTest){
    uint32_t ret, i;
    struct PL330_CHAN *pchan;

    for (i = 0; i < TSIZE; i++) {
        src[i] = i;
    }

    pchan = HAL_PL330_RequestChannel(s_pl330, (DMA_REQ_Type)TEST_CHANNEL);
    TEST_ASSERT_NOT_NULL(pchan);

    HAL_PL330_SetMcBuf(pchan, buf);

    ret = HAL_PL330_PrepDmaMemcpy(pchan, (uint32_t)dst, (uint32_t)src,
                                  TSIZE, MEMCPY_Callback, pchan);
    TEST_ASSERT(ret == HAL_OK);

    ret = HAL_PL330_Start(pchan);
    TEST_ASSERT(ret == HAL_OK);
}

TEST_GROUP_RUNNER(HAL_PL330){
    uint32_t ret, timeout = 1000;

#ifdef DMA0_BASE
    struct HAL_PL330_DEV *pl330 = &g_pl330Dev0;
#else
    struct HAL_PL330_DEV *pl330 = &g_pl330Dev;
#endif
    ret = HAL_PL330_Init(pl330);
    TEST_ASSERT(ret == HAL_OK);

    s_pl330 = pl330;

#if defined(HAL_NVIC_MODULE_ENABLED)
    HAL_NVIC_SetIRQHandler(pl330->irq[0], (NVIC_IRQHandler) & HAL_PL330_Handler);
    HAL_NVIC_SetIRQHandler(pl330->irq[1], (NVIC_IRQHandler) & HAL_PL330_Handler);
#elif defined(HAL_GIC_MODULE_ENABLED)
    HAL_IRQ_HANDLER_SetIRQHandler(pl330->irq[0], HAL_PL330_Handler, pl330);
    HAL_IRQ_HANDLER_SetIRQHandler(pl330->irq[1], HAL_PL330_Handler, pl330);
    HAL_GIC_Enable(pl330->irq[0]);
    HAL_GIC_Enable(pl330->irq[1]);
#endif

    RUN_TEST_CASE(HAL_PL330, MemcpyTest);
    while (timeout--) {
        if (pl330->pReg->INTEN & (1 << TEST_CHANNEL) == 0)
            break;

        HAL_DelayUs(10);
    }

    TEST_ASSERT(timeout != 0);
    ret = HAL_PL330_DeInit(pl330);
    TEST_ASSERT(ret == HAL_OK);
}

#endif

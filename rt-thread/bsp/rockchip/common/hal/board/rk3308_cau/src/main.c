/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2022 Rockchip Electronics Co., Ltd.
 */

#include "hal_bsp.h"
#include "hal_base.h"
#include "hal_define.h"
#include "middleware_conf.h"
#include "board.h"

#include "rpmsg_app.h"

extern void _start(void);

static struct GIC_AMP_IRQ_INIT_CFG irqsConfig[] = {
    /* Config the irqs here. */
    // todo...

    GIC_AMP_IRQ_CFG_ROUTE(AMP0_IRQn, 0xd0, CPU_GET_AFFINITY(0, 0)),
    GIC_AMP_IRQ_CFG_ROUTE(AMP1_IRQn, 0xd0, CPU_GET_AFFINITY(1, 0)),
    GIC_AMP_IRQ_CFG_ROUTE(AMP2_IRQn, 0xd0, CPU_GET_AFFINITY(2, 0)),
    GIC_AMP_IRQ_CFG_ROUTE(AMP3_IRQn, 0xd0, CPU_GET_AFFINITY(3, 0)),

    GIC_AMP_IRQ_CFG_ROUTE(RPMSG_01_IRQn, 0xd0, CPU_GET_AFFINITY(1, 0)),
    GIC_AMP_IRQ_CFG_ROUTE(RPMSG_02_IRQn, 0xd0, CPU_GET_AFFINITY(2, 0)),
    GIC_AMP_IRQ_CFG_ROUTE(RPMSG_03_IRQn, 0xd0, CPU_GET_AFFINITY(3, 0)),

    GIC_AMP_IRQ_CFG_ROUTE(RPMSG_10_IRQn, 0xd0, CPU_GET_AFFINITY(0, 0)),
    GIC_AMP_IRQ_CFG_ROUTE(RPMSG_12_IRQn, 0xd0, CPU_GET_AFFINITY(2, 0)),
    GIC_AMP_IRQ_CFG_ROUTE(RPMSG_13_IRQn, 0xd0, CPU_GET_AFFINITY(3, 0)),

    GIC_AMP_IRQ_CFG_ROUTE(RPMSG_20_IRQn, 0xd0, CPU_GET_AFFINITY(0, 0)),
    GIC_AMP_IRQ_CFG_ROUTE(RPMSG_21_IRQn, 0xd0, CPU_GET_AFFINITY(1, 0)),
    GIC_AMP_IRQ_CFG_ROUTE(RPMSG_23_IRQn, 0xd0, CPU_GET_AFFINITY(3, 0)),

    GIC_AMP_IRQ_CFG_ROUTE(RPMSG_30_IRQn, 0xd0, CPU_GET_AFFINITY(0, 0)),
    GIC_AMP_IRQ_CFG_ROUTE(RPMSG_31_IRQn, 0xd0, CPU_GET_AFFINITY(1, 0)),
    GIC_AMP_IRQ_CFG_ROUTE(RPMSG_32_IRQn, 0xd0, CPU_GET_AFFINITY(2, 0)),

    GIC_AMP_IRQ_CFG_ROUTE(0, 0, CPU_GET_AFFINITY(1, 0)),   /* sentinel */
};

static struct GIC_IRQ_AMP_CTRL irqConfig = {
    .cpuAff = CPU_GET_AFFINITY(1, 0),
    .defPrio = 0xd0,
    .defRouteAff = CPU_GET_AFFINITY(1, 0),
    .irqsCfg = &irqsConfig[0],
};

static void SPINLOCK_Init(void)
{
#ifdef HAL_SPINLOCK_MODULE_ENABLED
    uint32_t ownerID = HAL_CPU_TOPOLOGY_GetCurrentCpuId() << 1 | 1;
    HAL_SPINLOCK_Init(ownerID);
#endif
}

int main(void)
{
    int32_t ret;
    bool idle_enable;

    /* HAL BASE Init */
    HAL_Init();

    /* Interrupt Init */
    HAL_GIC_Init(&irqConfig);

    /* SPINLOCK Init */
    SPINLOCK_Init();

    /* BSP Init */
    BSP_Init();

    /* Board Init */
    Board_Init();

    /* RPMSG Init */
    rpmsg_init();

    rk_printf("\n");
    rk_printf("****************************************\n");
    rk_printf("  Hello RK3308 Bare-metal using RK_HAL! \n");
    rk_printf("   Fuzhou Rockchip Electronics Co.Ltd   \n");
    rk_printf("              CPI_ID(%lu)               \n", HAL_CPU_TOPOLOGY_GetCurrentCpuId());
    rk_printf("****************************************\n");
    rk_printf(" CPU(%lu) Initial OK!\n", HAL_CPU_TOPOLOGY_GetCurrentCpuId());
    rk_printf("\n");

#ifdef HAL_USING_RPMSG_CMD
    rpmsg_cmd_init(NULL);
#endif

#ifdef HAL_USING_ECNR_APP
    rpmsg_app_init(EPT_M1R0_ECNR, REMOTE_ID_0, RPMSG_ECN);
#endif

    while (1) {

        idle_enable = true;

        /* TODO: Message loop */
#ifdef HAL_USING_RPMSG_CMD
        ret = rpmsg_cmd_process(NULL);
        HAL_ASSERT((ret == HAL_OK) || (ret == HAL_BUSY));
        if (ret == HAL_BUSY) {
            idle_enable = false;
        }
#endif
#ifdef HAL_USING_ECNR_APP
        ret = rpmsg_app_process(RPMSG_ECN);
        HAL_ASSERT((ret == HAL_OK) || (ret == HAL_BUSY));
        if (ret == HAL_BUSY) {
            idle_enable = false;
        }
#endif
        /* Enter cpu idle when no message */
        if (idle_enable == true) {
            HAL_CPU_EnterIdle();
        }
    }

    return 0;
}

void _start(void)
{
    main();
}

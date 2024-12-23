/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2020 Rockchip Electronics Co., Ltd.
 */

#include "hal_base.h"

#include "unity.h"
#include "unity_fixture.h"
#include "unity_runner.h"

static void RunAllTests(void)
{
#if defined(HAL_DEMO_MODULE_ENABLED) && defined(UNITY_HAL_DEMO)
    RUN_TEST_GROUP(HAL_DEMO);
#endif

#ifdef UNITY_HAL_LEAGACY
    RUN_TEST_GROUP(HAL_LEAGACY);
#endif
#ifdef HAL_TIMER_MODULE_ENABLED
    RUN_TEST_GROUP(HAL_TIMER);
#endif
#if defined(HAL_QPIPSRAM_MODULE_ENABLED) && defined(UNITY_HAL_QPIPSRAM)
    RUN_TEST_GROUP(HAL_QPIPSRAM);
#endif
#if defined(HAL_HYPERPSRAM_MODULE_ENABLED) && defined(UNITY_HAL_HYPERPSRAM)
    RUN_TEST_GROUP(HAL_HYPERPSRAM);
#endif
#if defined(HAL_SNOR_MODULE_ENABLED) && defined(UNITY_HAL_SPIFLASH)
    RUN_TEST_GROUP(HAL_SNOR);
#endif
#ifdef HAL_PL330_MODULE_ENABLED
    RUN_TEST_GROUP(HAL_PL330);
#endif
#if defined(HAL_PMU_MODULE_ENABLED) && (defined(RKMCU_PISCES) || defined(RKMCU_RK2108))
    RUN_TEST_GROUP(HAL_PD);
#endif
#if defined(HAL_CRU_MODULE_ENABLED) && (defined(RKMCU_PISCES) || defined(RKMCU_RK2108))
    RUN_TEST_GROUP(HAL_CRU);
#endif
#if defined(HAL_SPI_MODULE_ENABLED) && defined(UNITY_HAL_SPI)
    RUN_TEST_GROUP(HAL_SPI);
#endif
#if defined(UNITY_MID_SDHCI) && defined(DRIVERS_SDHCI)
    RUN_TEST_GROUP(MID_SDHCI);
#endif
#if defined(HAL_I2C_MODULE_ENABLED) && defined(UNITY_HAL_I2C)
    RUN_TEST_GROUP(HAL_I2C);
#endif
#ifdef HAL_SARADC_MODULE_ENABLED
    RUN_TEST_GROUP(HAL_SARADC);
#endif
#ifdef HAL_PCIE_MODULE_ENABLED
    RUN_TEST_GROUP(HAL_PCIE);
#endif
#if (defined(HAL_GMAC_MODULE_ENABLED) || defined(HAL_GMAC1000_MODULE_ENABLED)) && defined(UNITY_HAL_GMAC)
    RUN_TEST_GROUP(HAL_GMAC);
#endif
}

int test_main(void)
{
    char *argv[1];

	argv[0] = "hal unit test";
    return UnityMain(1, argv, RunAllTests);
}

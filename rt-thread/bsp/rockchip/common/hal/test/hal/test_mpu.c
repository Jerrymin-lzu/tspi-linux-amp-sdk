/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2020 Rockchip Electronics Co., Ltd.
 */
#include "hal_base.h"

#if defined(HAL_ICACHE_MODULE_ENABLED) && defined(HAL_DCACHE_MODULE_ENABLED) && defined(MPU)

#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(HAL_MPU);

TEST_SETUP(HAL_MPU){
}

TEST_TEAR_DOWN(HAL_MPU){
}

/*-----------------------------------------------------------------------------
 *      Test implementation
 * Note: it must be compiled with -O0, otherwise the variable will be instead
 * with constant.
 *----------------------------------------------------------------------------*/

static void ClearMpu(void)
{
    for (uint32_t i = 0U; i < 8U; ++i) {
        MPU->RNR = i;
        MPU->RBAR = 0U;
        MPU->RASR = 0U;
    }
}

/*-----------------------------------------------------------------------------
 *      Test cases
 *----------------------------------------------------------------------------*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MPU_SetClear
\details
- Check if ARM_MPU_Load correctly loads MPU table to registers.
*/
TEST(HAL_MPU, TC_MPU_SetClear){
    static const ARM_MPU_Region_t table[] = {
        { .RBAR = 0U, .RASR = 0U },
        { .RBAR = ARM_MPU_RBAR(2U, 0x30000000U),
          .RASR = ARM_MPU_RASR(1U, ARM_MPU_AP_FULL, 0U, 0U, 0U, 0U, 0U,
                               ARM_MPU_REGION_SIZE_128MB) },
        { .RBAR = 0x50000000U,
          .RASR = ARM_MPU_RASR(0U, ARM_MPU_AP_FULL, 0U, 0U, 0U, 0U, 0U,
                               ARM_MPU_REGION_SIZE_64MB) }
    };

#define ASSERT_MPU_REGION(rnr, region)              \
    MPU->RNR = rnr;                                 \
    TEST_ASSERT((MPU->RBAR & MPU_RBAR_ADDR_Msk) ==  \
                (region.RBAR & MPU_RBAR_ADDR_Msk)); \
    TEST_ASSERT(MPU->RASR == region.RASR)

    ClearMpu();

    ARM_MPU_SetRegion(table[1].RBAR, table[1].RASR);

    ASSERT_MPU_REGION(1U, table[0]);
    ASSERT_MPU_REGION(2U, table[1]);
    ASSERT_MPU_REGION(3U, table[0]);

    ARM_MPU_SetRegionEx(5U, table[2].RBAR, table[2].RASR);

    ASSERT_MPU_REGION(4U, table[0]);
    ASSERT_MPU_REGION(5U, table[2]);
    ASSERT_MPU_REGION(6U, table[0]);

    ARM_MPU_ClrRegion(5U);

    MPU->RNR = 5U;
    TEST_ASSERT((MPU->RASR & MPU_RASR_ENABLE_Msk) == 0U);

#undef ASSERT_MPU_REGION
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MPU_Load
\details
- Check if ARM_MPU_Load correctly loads MPU table to registers.
*/
TEST(HAL_MPU, TC_MPU_Load){
    static const ARM_MPU_Region_t table[] = {
        { .RBAR = ARM_MPU_RBAR(0U, 0x00000000U),
          .RASR = ARM_MPU_RASR(1U, ARM_MPU_AP_FULL, 0U, 0U, 0U, 0U, 0U,
                               ARM_MPU_REGION_SIZE_32MB) },
        { .RBAR = ARM_MPU_RBAR(1U, 0x20000000U),
          .RASR = ARM_MPU_RASR(1U, ARM_MPU_AP_FULL, 0U, 0U, 0U, 0U, 0U,
                               ARM_MPU_REGION_SIZE_64MB) },
        { .RBAR = ARM_MPU_RBAR(2U, 0x30000000U),
          .RASR = ARM_MPU_RASR(1U, ARM_MPU_AP_FULL, 0U, 0U, 0U, 0U, 0U,
                               ARM_MPU_REGION_SIZE_128MB) },
        { .RBAR = ARM_MPU_RBAR(3U, 0x40000000U),
          .RASR = ARM_MPU_RASR(1U, ARM_MPU_AP_FULL, 0U, 0U, 0U, 0U, 0U,
                               ARM_MPU_REGION_SIZE_256MB) },
        { .RBAR = ARM_MPU_RBAR(4U, 0x50000000U),
          .RASR = ARM_MPU_RASR(1U, ARM_MPU_AP_FULL, 0U, 0U, 0U, 0U, 0U,
                               ARM_MPU_REGION_SIZE_512MB) },
        { .RBAR = ARM_MPU_RBAR(5U, 0x60000000U),
          .RASR = ARM_MPU_RASR(1U, ARM_MPU_AP_FULL, 0U, 0U, 0U, 0U, 0U,
                               ARM_MPU_REGION_SIZE_16MB) },
        { .RBAR = ARM_MPU_RBAR(6U, 0x70000000U),
          .RASR = ARM_MPU_RASR(1U, ARM_MPU_AP_FULL, 0U, 0U, 0U, 0U, 0U,
                               ARM_MPU_REGION_SIZE_8MB) },
        { .RBAR = ARM_MPU_RBAR(7U, 0x80000000U),
          .RASR = ARM_MPU_RASR(1U, ARM_MPU_AP_FULL, 0U, 0U, 0U, 0U, 0U,
                               ARM_MPU_REGION_SIZE_4MB) }
    };

#define ASSERT_MPU_REGION(rnr, table)                   \
    MPU->RNR = rnr;                                     \
    TEST_ASSERT((MPU->RBAR & MPU_RBAR_ADDR_Msk) ==      \
                (table[rnr].RBAR & MPU_RBAR_ADDR_Msk)); \
    TEST_ASSERT(MPU->RASR == table[rnr].RASR)

    ClearMpu();

    ARM_MPU_Load(&(table[0]), 1U);

    ASSERT_MPU_REGION(0U, table);

    ARM_MPU_Load(&(table[1]), 5U);

    ASSERT_MPU_REGION(0U, table);
    ASSERT_MPU_REGION(1U, table);
    ASSERT_MPU_REGION(2U, table);
    ASSERT_MPU_REGION(3U, table);
    ASSERT_MPU_REGION(4U, table);
    ASSERT_MPU_REGION(5U, table);

    ARM_MPU_Load(&(table[6]), 2U);

    ASSERT_MPU_REGION(5U, table);
    ASSERT_MPU_REGION(6U, table);
    ASSERT_MPU_REGION(7U, table);

#undef ASSERT_MPU_REGION
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MPU_Cache
\details
- Check if correctly control cacheable.
*/
TEST(HAL_MPU, TC_MPU_Cache){
    static const ARM_MPU_Region_t table[] = {
        { .RBAR = ARM_MPU_RBAR(0U, 0x00000000U),
          .RASR = ARM_MPU_RASR(1U, ARM_MPU_AP_FULL, 0U, 0U, 0U, 0U, 0U,
                               ARM_MPU_REGION_SIZE_32MB) },
        { .RBAR = ARM_MPU_RBAR(1U, 0x20000000U),
          .RASR = ARM_MPU_RASR(1U, ARM_MPU_AP_FULL, 0U, 0U, 1U, 0U, 0U,
                               ARM_MPU_REGION_SIZE_512KB) },
        { .RBAR = ARM_MPU_RBAR(2U, 0x20080000U),
          .RASR = ARM_MPU_RASR(1U, ARM_MPU_AP_PRIV, 0U, 0U, 0U, 0U, 0U,
                               ARM_MPU_REGION_SIZE_512KB) },
        { .RBAR = ARM_MPU_RBAR(3U, 0x40000000U),
          .RASR = ARM_MPU_RASR(1U, ARM_MPU_AP_FULL, 0U, 0U, 0U, 0U, 0U,
                               ARM_MPU_REGION_SIZE_256MB) }
    };
    int *uncacheVar = (int *)0x20080000;
    int *cacheVar = (int *)(uncacheVar - 32);

#define ASSERT_MPU_REGION(rnr, table)                   \
    MPU->RNR = rnr;                                     \
    TEST_ASSERT((MPU->RBAR & MPU_RBAR_ADDR_Msk) ==      \
                (table[rnr].RBAR & MPU_RBAR_ADDR_Msk)); \
    TEST_ASSERT(MPU->RASR == table[rnr].RASR)

    ClearMpu();

    ARM_MPU_Load(&(table[0]), 4U);

    ASSERT_MPU_REGION(0U, table);
    ASSERT_MPU_REGION(1U, table);
    ASSERT_MPU_REGION(2U, table);
    ASSERT_MPU_REGION(3U, table);

    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);

    *cacheVar = 0x0815U;
    HAL_DCACHE_Enable();
    HAL_DCACHE_CleanInvalidate();

    /* test cached value gets lost */

    // WHEN a value is written
    *cacheVar = 0x4711U;

    // ... and the cache is invalidated
    HAL_DCACHE_Invalidate();

    // ... and the cache is disabled
    HAL_DCACHE_Disable();

    // THEN the new value has been lost
    TEST_ASSERT(*cacheVar == 0x0815U);

    /* tear down */
    __ISB();

    *uncacheVar = 0x0815U;
    HAL_DCACHE_Enable();
    HAL_DCACHE_CleanInvalidate();

    /* test cached value gets lost */

    // WHEN a value is written
    *uncacheVar = 0x4711U;

    // ... and the cache is invalidated
    HAL_DCACHE_Invalidate();

    // ... and the cache is disabled
    HAL_DCACHE_Disable();

    // THEN the new value has been lost
    TEST_ASSERT(*uncacheVar == 0x4711U);

    /* tear down */
    __ISB();
#undef ASSERT_MPU_REGION
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MPU_Priv
\details
- Check if correctly control privilege.
*/
TEST(HAL_MPU, TC_MPU_Priv){
    static const ARM_MPU_Region_t table[] = {
        { .RBAR = ARM_MPU_RBAR(0U, 0x00000000U),
          .RASR = ARM_MPU_RASR(1U, ARM_MPU_AP_FULL, 0U, 0U, 0U, 0U, 0U,
                               ARM_MPU_REGION_SIZE_32MB) },
        { .RBAR = ARM_MPU_RBAR(1U, 0x20000000U),
          .RASR = ARM_MPU_RASR(1U, ARM_MPU_AP_FULL, 0U, 0U, 0U, 0U, 0U,
                               ARM_MPU_REGION_SIZE_512KB) },
        { .RBAR = ARM_MPU_RBAR(2U, 0x20080000U),
          .RASR = ARM_MPU_RASR(1U, ARM_MPU_AP_PRIV, 0U, 0U, 0U, 0U, 0U,
                               ARM_MPU_REGION_SIZE_512KB) },
        { .RBAR = ARM_MPU_RBAR(3U, 0x40000000U),
          .RASR = ARM_MPU_RASR(1U, ARM_MPU_AP_FULL, 0U, 0U, 0U, 0U, 0U,
                               ARM_MPU_REGION_SIZE_256MB) }
    };
    int *var = (int *)0x20080000;
    uint32_t control;

#define ASSERT_MPU_REGION(rnr, table)                   \
    MPU->RNR = rnr;                                     \
    TEST_ASSERT((MPU->RBAR & MPU_RBAR_ADDR_Msk) ==      \
                (table[rnr].RBAR & MPU_RBAR_ADDR_Msk)); \
    TEST_ASSERT(MPU->RASR == table[rnr].RASR)

    ClearMpu();

    ARM_MPU_Load(&(table[0]), 4U);

    ASSERT_MPU_REGION(0U, table);
    ASSERT_MPU_REGION(1U, table);
    ASSERT_MPU_REGION(2U, table);
    ASSERT_MPU_REGION(3U, table);

    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);

    control = __get_CONTROL() & (~CONTROL_nPRIV_Msk);
    __set_CONTROL(control);
    *var = 0x55aa;
    HAL_ASSERT(*var == 0x55aa);

    control = __get_CONTROL() | CONTROL_nPRIV_Msk;
    __set_CONTROL(control);
    *var = 0x55aa;
    TEST_ASSERT(false);
#undef ASSERT_MPU_REGION
}

TEST_GROUP_RUNNER(HAL_MPU){
    RUN_TEST_CASE(HAL_MPU, TC_MPU_SetClear);
    RUN_TEST_CASE(HAL_MPU, TC_MPU_Load);
    RUN_TEST_CASE(HAL_MPU, TC_MPU_Cache);
    RUN_TEST_CASE(HAL_MPU, TC_MPU_Priv);
}

#endif

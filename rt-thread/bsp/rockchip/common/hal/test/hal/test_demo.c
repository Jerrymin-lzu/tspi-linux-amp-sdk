/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2021 Rockchip Electronics Co., Ltd.
 */

#include "hal_bsp.h"
#include "hal_base.h"
#include "unity.h"
#include "unity_fixture.h"

#if defined(HAL_DEMO_MODULE_ENABLED) && defined(UNITY_HAL_DEMO)
/*************************** DEMO DRIVER ****************************/

/*************************** DEMO TEST ****************************/
TEST_GROUP(HAL_DEMO);

TEST_SETUP(HAL_DEMO){
}

TEST_TEAR_DOWN(HAL_DEMO){
}

/* DEMO test case 0 */
TEST(HAL_DEMO, DemoSimpleTest){
    HAL_DBG("It's a demo simple test");
}

TEST_GROUP_RUNNER(HAL_DEMO){
    RUN_TEST_CASE(HAL_DEMO, DemoSimpleTest);
}

#endif

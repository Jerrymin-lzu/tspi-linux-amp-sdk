/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2021 Rockchip Electronics Co., Ltd.
 */

#include "hal_bsp.h"
#include "hal_base.h"
#include "unity.h"
#include "unity_fixture.h"

#if defined(UNITY_MID_SDHCI) && defined(HAL_PL330_MODULE_ENABLED) && defined(DRIVERS_SDHCI)
#include "mmc_api.h"

#define TestSector   8
#define maxTestSector (TestSector * 4)
static int pWriteBuf[maxTestSector * 128];
static int pReadBuf[maxTestSector * 128];
static int userCapSize;

static int SdhciInit(void)
{
    int ioctlParam[5] = {0, 0, 0, 0, 0};
    int ret;

    sdmmc_init((void *)0xFE310000);
    ret = sdmmc_ioctrl(SDM_IOCTRL_REGISTER_CARD, ioctlParam);
    if (ret) {
        printf("emmc init error!\n");
        return -1;
    }

    ret = sdmmc_ioctrl(SDM_IOCTR_GET_CAPABILITY, ioctlParam);
    if (ret) {
        printf("emmc get capability error!\n");
        return -1;
    }

    userCapSize = ioctlParam[1];
}

static int SdhciTest(void)
{
    int i, j, loop = 0;
    int testEndLBA;
    int testLBA = 0;
    int testSecCount = 1;
    int printFlag;

    testEndLBA = userCapSize / 32;

    for (i = 0; i < (maxTestSector * 128); i++)
        pWriteBuf[i] = i;

    for (loop = 0; loop < 2; loop ++) {
        HAL_DBG("---------Test loop = %d---------\n", loop);
        HAL_DBG("---------Test ftl write %s---------\n", "");
        testSecCount = 1;
        HAL_DBG("testEndLBA = %x\n", testEndLBA);
        HAL_DBG("testLBA = %x\n", testLBA);

        for (testLBA = 0x10000 + loop; (testLBA + testSecCount) < testEndLBA;) {
            sdmmc_write(testLBA, testSecCount, pWriteBuf);
            sdmmc_read(testLBA, testSecCount, pReadBuf);
            printFlag = testLBA & 0x1FF;

            if (printFlag < testSecCount)
                HAL_DBG("testLBA = %x\n", testLBA);

            for (j = 0; j < testSecCount * 128; j++) {
                if (pWriteBuf[j] != pReadBuf[j]) {
                    printf("write not match:row=%x, num=%x, write=%x, read=%x\n", testLBA, j, pWriteBuf[j], pReadBuf[j]);
                    while (1);
                }
            }

            testLBA += testSecCount;
            testSecCount++;

            if (testSecCount > maxTestSector)
                testSecCount = 1;
        }

        HAL_DBG("---------Test ftl check---------%s\n", "");
        testSecCount = 1;

        for (testLBA = 0x10000 + loop; (testLBA + testSecCount) < testEndLBA;) {
            sdmmc_read(testLBA, testSecCount, pReadBuf);
            printFlag = testLBA & 0x7FF;

            if (printFlag < testSecCount)
                HAL_DBG("testLBA = %x\n", testLBA);

                for (j = 0; j < testSecCount * 128; j++) {
                    if (pWriteBuf[j] != pReadBuf[j]) {
                        printf("check not match:row=%x, num=%x, write=%x, read=%x\n", testLBA, j, pWriteBuf[j], pReadBuf[j]);
                        while (1);
                    }
                }

                testLBA += testSecCount;
                testSecCount++;

                if (testSecCount > maxTestSector)
                    testSecCount = 1;
        }
    }

    HAL_DBG("---------Test end---%s------\n", "");

    return 0;
}

TEST_GROUP(MID_SDHCI);

TEST_SETUP(MID_SDHCI){
}

TEST_TEAR_DOWN(MID_SDHCI){
}

TEST(MID_SDHCI, SdhciStressTest){
    SdhciTest();
}

TEST_GROUP_RUNNER(MID_SDHCI){
    SdhciInit();
    RUN_TEST_CASE(MID_SDHCI, SdhciStressTest);
}

#endif

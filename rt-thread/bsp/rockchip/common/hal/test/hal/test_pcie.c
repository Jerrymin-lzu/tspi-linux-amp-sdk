/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2022 Rockchip Electronics Co., Ltd.
 */

#include "hal_bsp.h"
#include "hal_base.h"
#include "unity.h"
#include "unity_fixture.h"

#if defined(HAL_PCIE_MODULE_ENABLED)

static struct HAL_PCIE_HANDLE s_pcieHandle;

static int PCIE_WaitForLinkUp(struct HAL_PCIE_HANDLE *pcie)
{
    int retries;

    for (retries = 0; retries < 100000; retries++) {
        if (HAL_PCIE_LinkUp(pcie)) {
            /*
             * We may be here in case of L0 in Gen1. But if EP is capable
             * of Gen2 or Gen3, Gen switch may happen just in this time, but
             * we keep on accessing devices in unstable link status. Given
             * that LTSSM max timeout is 24ms per period, we can wait a bit
             * more for Gen switch.
             */
            HAL_DelayMs(50);
            printf("PCIe Link up, LTSSM is 0x%lx\n", HAL_PCIE_GetLTSSM(pcie));

            return 0;
        }
        printf("PCIe Linking... LTSSM is 0x%lx\n", HAL_PCIE_GetLTSSM(pcie));
        HAL_DelayMs(20);
    }
    printf("PCIe Link failed, LTSSM is 0x%lx\n", HAL_PCIE_GetLTSSM(pcie));
    HAL_DelayMs(20);

    return -1;
}

static int PCIE_WaitForDmaFinished(struct HAL_PCIE_HANDLE *pcie, struct DMA_TABLE *cur)
{
    int us = 0xFFFFFFFF;
    int ret;

    while (us--) {
        ret = HAL_PCIE_GetDmaStatus(pcie, cur->chn, cur->dir);
        if (ret) {
            return 0;
        }
        HAL_CPUDelayUs(1);
    }

    return -1;
}

static int PCIE_Init(struct HAL_PCIE_HANDLE *pcie, struct HAL_PCIE_DEV *dev)
{
    int ret;

    HAL_PCIE_Init(pcie, dev);

    ret = PCIE_WaitForLinkUp(pcie);
    if (ret) {
        return ret;
    }

    return 0;
}

static HAL_Status PCIE_INTxIsr(uint32_t irq, void *args)
{
    printf("%s trigger, please impletment the INTx int clear function\n", __func__);
    /* to-do */

    return HAL_OK;
}

/*************************** PCIe DRIVER ****************************/

#define __PCIE_RawWriteB(v, a) (*(volatile unsigned char *)(a) = (v))
#define __PCIE_RawWriteQ(v, a) (*(volatile unsigned long long *)(a) = (v))

#define __PCIE_RawReadB(a) (*(volatile unsigned char *)(a))
#define __PCIE_RawReadQ(a) (*(volatile unsigned long long *)(a))

static void PCIE_MemcpyFromIo(void *to, const volatile void *from, size_t count)
{
    while (count && !HAL_IS_ALIGNED((unsigned long)from, 8)) {
        *(uint8_t *)to = __PCIE_RawReadB(from);
        from++;
        to++;
        count--;
    }

#if defined(HAL_AP_CORE) && defined(HAL_DCACHE_MODULE_ENABLED)
    while (count >= 8) {
        *(uint64_t *)to = __PCIE_RawReadQ(from);
        from += 8;
        to += 8;
        count -= 8;
    }
#endif

    while (count) {
        *(uint8_t *)to = __PCIE_RawReadB(from);
        from++;
        to++;
        count--;
    }
}

static void PCIE_MemcpyToIo(volatile void *to, const void *from, size_t count)
{
    while (count && !HAL_IS_ALIGNED((unsigned long)to, 8)) {
        __PCIE_RawWriteB(*(uint8_t *)from, to);
        from++;
        to++;
        count--;
    }

#if defined(HAL_AP_CORE) && defined(HAL_DCACHE_MODULE_ENABLED)
    while (count >= 8) {
        __PCIE_RawWriteQ(*(uint64_t *)from, to);
        from += 8;
        to += 8;
        count -= 8;
    }
#endif

    while (count) {
        __PCIE_RawWriteB(*(uint8_t *)from, to);
        from++;
        to++;
        count--;
    }
}

static void PCIE_MemSetIo(volatile void *dst, int c, size_t count)
{
    uint64_t temp = (uint8_t)c;

    temp |= temp << 8;
    temp |= temp << 16;
    temp |= temp << 32;

    while (count && !HAL_IS_ALIGNED((unsigned long)dst, 8)) {
        __PCIE_RawWriteB(c, dst);
        dst++;
        count--;
    }

    while (count >= 8) {
        __PCIE_RawWriteQ(temp, dst);
        dst += 8;
        count -= 8;
    }

    while (count) {
        __PCIE_RawWriteB(c, dst);
        dst++;
        count--;
    }
}

#define memset_io(a, b, c)     PCIE_MemSetIo((a), (b), (c))
#define memcpy_fromio(a, b, c) PCIE_MemcpyFromIo((a), (b), (c))
#define memcpy_toio(a, b, c)   PCIE_MemcpyToIo((a), (b), (c))

/*************************** PCIe TEST ****************************/

#define TEST_PCIE_DMA_BUS_ADDR   0x3C000000
#define TEST_PCIE_DMA_LOCAL_ADDR 0x3C000000
#define TEST_PCIE_DMA_SIZE       0x1000
#define TEST_PCIE_DMA_CHAN       0

#define TEST_DEVICE              PCI_BDF(0x01, 0x00, 0x00)
#define TEST_DEVICE_BAR_CPU_ADDR 0xF0200000
#define TEST_DEVICE_TEST_SIZE    0x100

TEST_GROUP(HAL_PCIE);

TEST_SETUP(HAL_PCIE){
}

TEST_TEAR_DOWN(HAL_PCIE){
}

/* PCIe test case 0 */
TEST(HAL_PCIE, PCIeCPUSimpleTest){
    uint32_t cfg = g_pcieDev.cfgBase;
    void *bar = (void *)TEST_DEVICE_BAR_CPU_ADDR;
    uint32_t vid, did, class;
    uint32_t buf[TEST_DEVICE_TEST_SIZE];
    uint32_t len = TEST_DEVICE_TEST_SIZE;
    int index;

    printf("PCIe CPU test:");
    printf("check the bdf of TEST_DEVICE first, change it if need!\n");

    index = HAL_PCIE_OutboundConfigCFG0(&s_pcieHandle, TEST_DEVICE, 0x100000);
    printf("cfg address=%lx, atu_index=%d\n", cfg, index);
    HAL_PCI_ReadConfigWord(cfg, PCI_VENDOR_ID, &vid);
    HAL_PCI_ReadConfigWord(cfg, PCI_DEVICE_ID, &did);

    printf("bar address=%p\n", bar);
    printf(" - cfg read: VID=%lx\n", vid);
    printf(" - cfg read: DID=%lx\n", did);
    HAL_PCI_ReadConfigDWord(cfg, PCI_CLASS_REVISION, &class);
    printf(" - cfg read: Class=%lx\n", class);
    HAL_PCI_ReadConfigByte(cfg, PCI_CLASS_DEVICE, &class);
    printf(" - cfg read: Class device=%lx\n", class);
    HAL_PCI_ReadConfigByte(cfg, PCI_CLASS_CODE, &class);
    printf(" - cfg read: Class code=%lx\n", class);

    printf(" - bar memory operation test\n");
    memset(buf, 0, len);
    memcpy_fromio(buf, bar, len);
    HAL_DBG_HEX("memcpy_fromio:", buf, 4, 0x10);

    memset_io(bar, 0xa5, len);
    memset(buf, 0, len);
    memcpy_fromio(buf, bar, len);
    HAL_DBG_HEX("memset_io:", buf, 4, 0x10);

    memset(buf, 0x5a, len);
    memcpy_toio(bar, buf, len);
    memset(buf, 0, len);
    memcpy_fromio(buf, bar, len);
    HAL_DBG_HEX("memcpy_toio:", buf, 4, 0x10);
}

/* PCIe test case 1 */
TEST(HAL_PCIE, PCIeDMASimpleTest){
    struct DMA_TABLE table;
    int ret;

    printf("PCIe DMA Test\n");

    /* DMA write test */
    memset(&table, 0, sizeof(struct DMA_TABLE));
    HAL_DCACHE_CleanByRange(TEST_PCIE_DMA_LOCAL_ADDR, TEST_PCIE_DMA_SIZE);
    table.bufSize = TEST_PCIE_DMA_SIZE;
    table.bus = TEST_PCIE_DMA_BUS_ADDR;
    table.local = TEST_PCIE_DMA_LOCAL_ADDR;
    table.chn = TEST_PCIE_DMA_CHAN;
    table.dir = DMA_TO_BUS;

    HAL_PCIE_ConfigDma(&s_pcieHandle, &table);
    HAL_PCIE_StartDma(&s_pcieHandle, &table);
    ret = PCIE_WaitForDmaFinished(&s_pcieHandle, &table);
    TEST_ASSERT(ret == 0);
    printf("PCIe DMA wr success\n");

    /* DMA read test */
    memset(&table, 0, sizeof(struct DMA_TABLE));
    HAL_DCACHE_CleanByRange(TEST_PCIE_DMA_LOCAL_ADDR, TEST_PCIE_DMA_SIZE);
    table.bufSize = TEST_PCIE_DMA_SIZE;
    table.bus = TEST_PCIE_DMA_BUS_ADDR;
    table.local = TEST_PCIE_DMA_LOCAL_ADDR;
    table.chn = TEST_PCIE_DMA_CHAN;
    table.dir = DMA_FROM_BUS;

    HAL_PCIE_ConfigDma(&s_pcieHandle, &table);
    HAL_PCIE_StartDma(&s_pcieHandle, &table);
    ret = PCIE_WaitForDmaFinished(&s_pcieHandle, &table);
    HAL_DCACHE_InvalidateByRange(TEST_PCIE_DMA_LOCAL_ADDR, TEST_PCIE_DMA_SIZE);
    TEST_ASSERT(ret == 0);
    printf("PCIe DMA rd success\n");
}

TEST_GROUP_RUNNER(HAL_PCIE){
    int ret;

    printf("PCIe Test\n");

    ret = PCIE_Init(&s_pcieHandle, &g_pcieDev);
    TEST_ASSERT(ret == 0);

    HAL_IRQ_HANDLER_SetIRQHandler(g_pcieDev.legacyIrqNum, PCIE_INTxIsr, NULL);

    /* Waitting for device config well, or optimize the logic through other handshakes */
    printf("Adding 100ms delay for waiting device ready\n");
    HAL_DelayMs(100);

    RUN_TEST_CASE(HAL_PCIE, PCIeCPUSimpleTest);
    RUN_TEST_CASE(HAL_PCIE, PCIeDMASimpleTest);
}

#endif

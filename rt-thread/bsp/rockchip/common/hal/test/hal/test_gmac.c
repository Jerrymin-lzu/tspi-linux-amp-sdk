/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2021 Rockchip Electronics Co., Ltd.
 */

#include "hal_bsp.h"
#include "hal_base.h"
#include "hal_gmac.h"
#include "unity.h"
#include "unity_fixture.h"

#if (defined(HAL_GMAC_MODULE_ENABLED) || defined(HAL_GMAC1000_MODULE_ENABLED))

/*************************** GMAC DRIVER ****************************/

/***************************** MACRO Definition ******************************/
#define __is_print(ch) ((unsigned int)((ch) - ' ') < 127u - ' ')

#define CONFIG_SYS_NONCACHED_MEMORY (4 << 20) /* 4M */

/* 1MB granularity */
#define MMU_SECTION_SIZE (1 << 20)

/***************************** Structure Definition **************************/

/***************************** Function Declare ******************************/

/********************* Private MACRO Definition ******************************/
#define GMAC_BUS_MAX 2

#define ARCH_DMA_MINALIGN 64

#define GMAC_DESCRIPTOR_SIZE 16
#define GMAC_DESCRIPTORS_TX  8
#define GMAC_DESCRIPTORS_RX  8
#define GMAC_DESC_TX_SIZE    HAL_GMAC_ALIGN(GMAC_DESCRIPTORS_TX * GMAC_DESCRIPTOR_SIZE, ARCH_DMA_MINALIGN)
#define GMAC_DESC_RX_SIZE    HAL_GMAC_ALIGN(GMAC_DESCRIPTORS_RX * GMAC_DESCRIPTOR_SIZE, ARCH_DMA_MINALIGN)

#define GMAC_RX_BUFFER_SIZE (GMAC_DESCRIPTORS_RX * HAL_GMAC_MAX_PACKET_SIZE)
#define GMAC_TX_BUFFER_SIZE (GMAC_DESCRIPTORS_TX * HAL_GMAC_MAX_PACKET_SIZE)

/* Basic mode control register. */
#define BMCR_RESV      0x003f      /* Unused...                   */
#define BMCR_SPEED1000 0x0040     /* MSB of Speed (1000)         */
#define BMCR_CTST      0x0080     /* Collision test              */
#define BMCR_FULLDPLX  0x0100     /* Full duplex                 */
#define BMCR_ANRESTART 0x0200     /* Auto negotiation restart    */
#define BMCR_ISOLATE   0x0400     /* Isolate data paths from MII */
#define BMCR_PDOWN     0x0800     /* Enable low power state      */
#define BMCR_ANENABLE  0x1000     /* Enable auto negotiation     */
#define BMCR_SPEED100  0x2000     /* Select 100Mbps              */
#define BMCR_LOOPBACK  0x4000     /* TXD loopback bits           */
#define BMCR_RESET     0x8000     /* Reset to default state      */
#define BMCR_SPEED10   0x0000     /* Select 10Mbps               */

#define GMAC_TEST_TIMES 1600

/********************* Private Structure Definition **************************/
/* GMAC consumer config data. */
struct GMAC_ETH_CONFIG {
    struct GMAC_HANDLE instance;
    const struct HAL_GMAC_DEV *halDev;
    eGMAC_PHY_Interface mode;
    uint32_t speed;
    uint32_t maxSpeed;
    uint16_t phyAddr;

    bool extClk;

    /* phy reset gpio */
    struct GPIO_REG *resetGpioBank;
    ePINCTRL_GPIO_PINS resetGpioNum;
    uint32_t resetDelayMs[3];

    int32_t txDelay;
    int32_t rxDelay;

    struct GMAC_Desc *txDescs;
    struct GMAC_Desc *rxDescs;

    uint8_t *txBuff;
    uint8_t *rxBuff;

    /* MAC address info, hw address */
    uint8_t macAddr[6];
};

/********************* Private Variable Definition ***************************/
#ifdef NC_MEM_BASE
static const unsigned long os_no_cache_start = (unsigned long)NC_MEM_BASE;
#else
static const unsigned long os_no_cache_start = 0;
#endif

/*
 * Reserve one MMU section worth of address space below the malloc() area that
 * will be mapped uncached.
 */
static unsigned long noncached_start;
static unsigned long noncached_end;
static unsigned long noncached_next;

static unsigned int m_nocachemem_inited = 0;

static uint8_t dstAddr[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

#if defined(HAL_GMAC_MODULE_ENABLED) && defined(SOC_RK3568)
static struct GMAC_ETH_CONFIG ethConfigTable[] =
{
#ifdef HAL_GMAC0
    {
        .halDev = &g_gmac0Dev,
        .mode = PHY_INTERFACE_MODE_RGMII,
        .maxSpeed = 1000,
        .phyAddr = 1,

        .extClk = false,

        .resetGpioBank = GPIO2,
        .resetGpioNum = GPIO_PIN_D3,
        .resetDelayMs = { 0, 20, 100 },

        .txDelay = 0x3C,
        .rxDelay = 0x2f,
    },
#endif

#ifdef HAL_GMAC1
    {
        .halDev = &g_gmac1Dev,
        .mode = PHY_INTERFACE_MODE_RGMII,
        .maxSpeed = 1000,
        .phyAddr = 1,

        .extClk = false,

        .resetGpioBank = GPIO2,
        .resetGpioNum = GPIO_PIN_D1,
        .resetDelayMs = { 0, 20, 100 },

        .txDelay = 0x4f,
        .rxDelay = 0x26,
    },
#endif
};
#endif

#if defined(HAL_GMAC1000_MODULE_ENABLED) && defined(SOC_RK3358)
static struct GMAC_ETH_CONFIG ethConfigTable[] =
{
#ifdef HAL_GMAC0
    {
        .halDev = &g_gmacDev,
        .mode = PHY_INTERFACE_MODE_RMII,
        .maxSpeed = 100,
        .speed = 100,
        .phyAddr = 0,

        .extClk = false,

        .resetGpioBank = GPIO2,
        .resetGpioNum = GPIO_PIN_B5,
        .resetDelayMs = { 0, 50, 50 },
    },
#endif
};
#endif

#if defined(HAL_GMAC1000_MODULE_ENABLED) && defined(SOC_RK3308)
static struct GMAC_ETH_CONFIG ethConfigTable[] =
{
#ifdef HAL_GMAC0
    {
        .halDev = &g_gmac0Dev,
        .mode = PHY_INTERFACE_MODE_RMII,
        .maxSpeed = 100,
        .speed = 100,
        .phyAddr = 0,

        .extClk = true,

        .resetGpioBank = GPIO4,
        .resetGpioNum = GPIO_PIN_C0,
        .resetDelayMs = { 0, 50, 50 },
    },
#endif
};
#endif

/********************* Private Function Definition ***************************/

static void Dump_Hex(char *message, uint8_t *ptr, uint16_t buflen)
{
    unsigned char *buf = (unsigned char *)ptr;
    int i, j;

    printf("%s package at %p, len: %d: \n", message, ptr, buflen);
    for (i = 0; i < buflen; i += 16) {
        printf("%08X: ", i);

        for (j = 0; j < 16; j++) {
            if (i + j < buflen) {
                printf("%02X ", buf[i + j]);
            } else {
                printf("   ");
            }
        }
        printf(" ");

        for (j = 0; j < 16; j++) {
            if (i + j < buflen) {
                printf("%c", __is_print(buf[i + j]) ? buf[i + j] : '.');
            }
        }
        printf("\n");
    }
}

static void print_desc(struct GMAC_HANDLE *pGMAC)
{
    struct GMAC_Desc *desc;
    int nIndex;

    printf("\n");

    if (pGMAC->rxDescs) {
        desc = pGMAC->rxDescs;
        for (nIndex = 0; nIndex < pGMAC->rxSize; nIndex++) {
            desc = pGMAC->rxDescs + nIndex;
            printf("rx_desc[%d]@0x%08lx={0x%lx, 0x%lx, 0x%lx, 0x%lx};\n",
                   nIndex, (uint32_t)desc, desc->des0, desc->des1, desc->des2, desc->des3);
        }
    }

    if (pGMAC->txDescs) {
        desc = pGMAC->txDescs;
        for (nIndex = 0; nIndex < pGMAC->txSize; nIndex++) {
            desc = pGMAC->txDescs + nIndex;
            printf("tx_desc[%d]@0x%08lx={0x%lx, 0x%lx, 0x%lx, 0x%lx};\n",
                   nIndex, (uint32_t)desc, desc->des0, desc->des1, desc->des2, desc->des3);
        }
    }
}

static void Dump_Regs(struct GMAC_HANDLE *pGMAC)
{
    uint32_t *reg = (uint32_t *)pGMAC->pReg;
    int i;

    printf("\n");

    for (i = 0; i < (0x200 / 0x4); i++) {
        printf("offset_0x%08x: %08lx %08lx %08lx %08lx\n", i * 0x10,
               *((volatile const uint32_t *)(reg + 0x4 * i)), *(volatile const uint32_t *)((reg + 0x4 * i + 1)),
               *((volatile const uint32_t *)(reg + 0x4 * i + 2)), *((volatile const uint32_t *)(reg + 0x4 * i + 3)));
    }

    reg = reg + 0x1000 / 4;

    for (i = 0; i < (0x100 / 0x4); i++) {
        printf("offset_0x%08x: %08lx %08lx %08lx %08lx\n", i * 0x10 + 0x1000,
               *((volatile const uint32_t *)(reg + 0x4 * i)), *((volatile const uint32_t *)(reg + 0x4 * i + 1)),
               *((volatile const uint32_t *)(reg + 0x4 * i + 2)), *((volatile const uint32_t *)(reg + 0x4 * i + 3)));
    }
}

static void PHY_Read(struct GMAC_HANDLE *pGMAC, uint32_t phyReg)
{
    int data;

    data = HAL_GMAC_MDIORead(pGMAC, pGMAC->phyConfig.phyAddress, phyReg);
    if (data >= 0) {
        printf("PHY_Read: %02lX --> %08X\n", phyReg, data);
    } else {
        printf("PHY_Read: %02lX --> faild\n", phyReg);
    }
}

static void PHY_Write(uint32_t phyReg, uint32_t data)
{
    struct GMAC_HANDLE *pGMAC;
    int status;

    status = HAL_GMAC_MDIOWrite(pGMAC, pGMAC->phyConfig.phyAddress, phyReg, data);
    if (!status) {
        printf("PHY_Write: %02lX --> %08lX\n", phyReg, data);
    } else {
        printf("PHY_Write: %02lX --> faild\n", phyReg);
    }
}

static void PHY_Dump(struct GMAC_ETH_CONFIG *eth, struct GMAC_HANDLE *pGMAC)
{
    int data, i;

    for (i = 0; i < 32; i++) {
        data = HAL_GMAC_MDIORead(pGMAC, eth->phyAddr, i);
        if (data < 0) {
            printf("phy_dump: %d --> faild\n", i);
            break;
        }

        if (i % 8 == 7) {
            printf("%d --> %08X\n ", i, data);
        } else {
            printf("%d --> %08X\n\n", i, data);
        }
    }
    printf("\n");
}

static void GMAC_PHY_Reset(struct GMAC_ETH_CONFIG *config)
{
    if (config->resetGpioBank) {
        HAL_GPIO_SetPinDirection(config->resetGpioBank,
                                 config->resetGpioNum,
                                 GPIO_OUT);
        HAL_GPIO_SetPinLevel(config->resetGpioBank,
                             config->resetGpioNum,
                             GPIO_HIGH);

        HAL_DelayMs(config->resetDelayMs[0]);

        HAL_GPIO_SetPinLevel(config->resetGpioBank,
                             config->resetGpioNum,
                             GPIO_LOW);
        HAL_DelayMs(config->resetDelayMs[1]);

        HAL_GPIO_SetPinLevel(config->resetGpioBank,
                             config->resetGpioNum,
                             GPIO_HIGH);
        HAL_DelayMs(config->resetDelayMs[2]);
    }
}

static inline void NET_Random_ETHAddr(uint8_t *addr)
{
    unsigned int seed = HAL_TIMER_GetCount(SYS_TIMER) | 0xffffffff;
    uint8_t i;

    for (i = 0; i < 6; i++) {
        addr[i] = rand_r(&seed);
    }

    addr[0] &= 0xfe;    /* clear multicast bit */
    addr[0] |= 0x02;    /* set local assignment bit (IEEE802) */
}

static inline void NET_Random_Package(uint8_t *addr, uint16_t len)
{
    unsigned int seed = HAL_TIMER_GetCount(SYS_TIMER) | 0xffffffff;
    uint16_t i;

    for (i = 0; i < len; i++) {
        addr[i] = rand_r(&seed);
    }
}

static void PHY_Update_Links(struct GMAC_ETH_CONFIG *eth, struct GMAC_HANDLE *pGMAC,
                             uint8_t id)
{
    HAL_Status status;

    status = HAL_GMAC_PHYStartup(pGMAC);
    if (status) {
        printf("HAL_PHY_Startup() failed: %d\n", status);

        return;
    }
    HAL_DelayMs(7000);

    status = HAL_GMAC_PHYUpdateLink(pGMAC);
    if (status == HAL_OK) {
        if (pGMAC->phyStatus.link) {
            status = HAL_GMAC_PHYParseLink(pGMAC);
            if (PHY_SPEED_1000M == pGMAC->phyStatus.speed) {
                printf("Phy%d: 1000M link speed\n", id);
            } else if (PHY_SPEED_100M == pGMAC->phyStatus.speed) {
                printf("Phy%d: 100M link speed\n", id);
            } else {
                printf("Phy%d: 10M link speed\n", id);
            }

            if (PHY_DUPLEX_HALF == pGMAC->phyStatus.duplex) {
                printf("Phy%d: half dumplex\n", id);
            } else {
                printf("Phy%d: full dumplex\n", id);
            }

            if (pGMAC->phyStatus.pause) {
                printf("Phy%d: flow control rx/tx\n", id);
            } else {
                printf("Phy%d: flow control off\n", id);
            }

            status = HAL_GMAC_AdjustLink(pGMAC, eth->txDelay, eth->rxDelay);
            if (status != HAL_OK) {
                printf("HAL_GMAC_AdjustLink() failed: %d\n", status);
            }

            printf("Phy%d: link up.\n", id);
        } else {
            printf("Phy%d: link down.\n", id);
        }
    }
}

/* interrupt service routine polling */
static HAL_Status GMAC_ETH_IRQ(struct GMAC_HANDLE *pGMAC)
{
    HAL_Status status;

    /* enter interrupt */
    /* rt_interrupt_enter(); */
    status = HAL_GMAC_IRQHandler(pGMAC);

    if (status & DMA_HANLE_RX) {
        /* a frame has been received */
        return 1;
    }

    return 0;
    /* leave interrupt */
    /* rt_interrupt_leave(); */
}

static HAL_Status noncached_init(void)
{
    unsigned long start, end;
    size_t size;

    if (os_no_cache_start <= 0) {
	printf("Noncached_init failed, plase defined no cached memort\n");
	return -1;
    }

    start = HAL_GMAC_ALIGN(os_no_cache_start, 64);
    size = HAL_GMAC_ALIGN(CONFIG_SYS_NONCACHED_MEMORY, MMU_SECTION_SIZE);
    end = start + size;

    noncached_start = start;
    noncached_end = end;
    noncached_next = start;
    m_nocachemem_inited = 1;

    return 0;
}

static unsigned long noncached_alloc(size_t size, size_t align)
{
    if (!m_nocachemem_inited) {
        if (noncached_init())
	    return (unsigned long)NULL;
    }

    unsigned long next = HAL_GMAC_ALIGN(noncached_next, align);

    if (next >= noncached_end || (noncached_end - next) < size) {
        return 0;
    }

    noncached_next = next + size;

    return next;
}

static void *malloc_align(size_t size, size_t align)
{
    void *align_ptr;
    void *ptr;
    size_t align_size;

    /* align the alignment size to 4 byte */
    align = ((align + 0x03) & ~0x03);

    /* get total aligned size */
    align_size = ((size + 0x03) & ~0x03) + align;
    /* allocate memory block from heap */
    ptr = malloc(align_size);
    if (ptr != NULL) {
        /* the allocated memory block is aligned */
        if (((uint32_t)ptr & (align - 1)) == 0) {
            align_ptr = (void *)((uint32_t)ptr + align);
        } else {
            align_ptr = (void *)(((uint32_t)ptr + (align - 1)) & ~(align - 1));
        }

        /* set the pointer before alignment pointer to the real pointer */
        *((uint32_t *)((uint32_t)align_ptr - sizeof(void *))) = (uint32_t)ptr;

        ptr = align_ptr;
    }

    return ptr;
}

static void free_align(void *ptr)
{
    void *real_ptr;

    real_ptr = (void *)*(uint32_t *)((uint32_t)ptr - sizeof(void *));
    free(real_ptr);
}

/********************* Public Function Definition ****************************/

static HAL_Status GMAC_Send_Test(struct GMAC_ETH_CONFIG *eth, struct GMAC_HANDLE *pGMAC,
                                 uint16_t len)
{
    uint8_t *ptr = NULL;
    HAL_Status status;

    if (!pGMAC->phyStatus.link) {
        return -1;
    }

    /* Check the frame length. */
    if (len > HAL_GMAC_MAX_FRAME_SIZE) {
        return -1;
    }

    ptr = (uint8_t *)HAL_GMAC_GetTXBuffer(pGMAC);
    memcpy(ptr, dstAddr, 6);
    memcpy(ptr + 6, eth->macAddr, 6);
    ptr[12] = 0x40;
    ptr[13] = 0x06;
    ptr[14] = 0x00;
    ptr[15] = 0x01;
    NET_Random_Package(ptr + 16, len - 16);

    if (len > 60) {
        /* index */
        ptr[50] = pGMAC->txDescIdx;
    }

    /* dump packages */
    Dump_Hex("Tx", ptr, len);

    HAL_DCACHE_CleanByRange((uint32_t)ptr, len);
    status = HAL_GMAC_Send(pGMAC, ptr, len);
    if (status) {
        printf("GMAC send failed: %d\n", status);
    }

    return status;
}

static uint16_t GMAC_Recv_Test(struct GMAC_HANDLE *pGMAC)
{
    uint8_t *ptr = NULL;
    HAL_Status status;
    int32_t size;

    if (!pGMAC->phyStatus.link) {
        return -1;
    }

    status = GMAC_ETH_IRQ(pGMAC);
    ptr = HAL_GMAC_Recv(pGMAC, &size);
    while (status && ptr) {
        if (size > 0 && ptr) {
            /* dump packages */
            Dump_Hex("Rx", ptr, size);
            HAL_DCACHE_InvalidateByRange((uint32_t)ptr, size);
            HAL_GMAC_CleanRX(pGMAC);
        } else {
            printf("GMAC recv failed: %ld\n", size);

            return -1;
        }
        ptr = HAL_GMAC_Recv(pGMAC, &size);
    }

    return 0;
}

static HAL_Status GMAC_Memory_Init(struct GMAC_ETH_CONFIG *eth, struct GMAC_HANDLE *pGMAC)
{
    eth->rxDescs = (struct GMAC_Desc *)noncached_alloc(GMAC_DESC_RX_SIZE, ARCH_DMA_MINALIGN);
    eth->txDescs = (struct GMAC_Desc *)noncached_alloc(GMAC_DESC_TX_SIZE, ARCH_DMA_MINALIGN);

    if (!eth->rxDescs || !eth->txDescs)
        return -1;

    eth->rxBuff = malloc_align(GMAC_RX_BUFFER_SIZE, ARCH_DMA_MINALIGN);
    eth->txBuff = malloc_align(GMAC_TX_BUFFER_SIZE, ARCH_DMA_MINALIGN);

    if (!eth->rxBuff || !eth->txBuff)
        return -1;

    memset(eth->rxDescs, 0, GMAC_DESC_RX_SIZE);
    memset(eth->txDescs, 0, GMAC_DESC_TX_SIZE);

    memset(eth->rxBuff, 0, GMAC_RX_BUFFER_SIZE);
    HAL_DCACHE_InvalidateByRange((uint32_t)eth->rxBuff, GMAC_RX_BUFFER_SIZE);

    memset(eth->txBuff, 0, GMAC_TX_BUFFER_SIZE);
    HAL_DCACHE_CleanByRange((uint32_t)eth->txBuff, GMAC_TX_BUFFER_SIZE);

    HAL_GMAC_DMARxDescInit(pGMAC, eth->rxDescs, eth->rxBuff, GMAC_DESCRIPTORS_RX);
    HAL_GMAC_DMATxDescInit(pGMAC, eth->txDescs, eth->txBuff, GMAC_DESCRIPTORS_TX);

    print_desc(pGMAC);

    return 0;
}

static HAL_Status GMAC_Init(uint8_t id)
{
    struct GMAC_ETH_CONFIG *eth;
    struct GMAC_HANDLE *pGMAC;
    const struct HAL_GMAC_DEV *gmacDev;
    struct GMAC_PHY_Config config;
    eGMAC_PHY_Interface interface;
    HAL_Status status;
    uint32_t freq;

    HAL_ASSERT(id < GMAC_BUS_MAX);

    eth = &ethConfigTable[id];
    if (!eth) {
        return HAL_ERROR;
    }

    pGMAC = &eth->instance;
    gmacDev = eth->halDev;

    /* MAC init */
    interface = eth->mode;

    if (interface == PHY_INTERFACE_MODE_RGMII) {
        HAL_CRU_ClkSetFreq(gmacDev->clkID, 125000000);
    } else {
        HAL_CRU_ClkSetFreq(gmacDev->clkID, 50000000);
    }

    freq = HAL_CRU_ClkGetFreq(gmacDev->pclkID);
    HAL_GMAC_Init(pGMAC, gmacDev->pReg, freq, interface, eth->extClk);

    /* PHY Init */
    config.speed = eth->speed;
    config.maxSpeed = eth->maxSpeed;
    config.duplexMode = PHY_DUPLEX_FULL;
    config.neg = PHY_AUTONEG_ENABLE;
    config.interface = interface;
    config.phyAddress = eth->phyAddr;
    config.features = 0;
    status = HAL_GMAC_PHYInit(pGMAC, &config);
    if (status) {
        printf("HAL_PHY_Init() failed: %d\n", status);

        return status;
    }

    /* MAC Address */
    NET_Random_ETHAddr(eth->macAddr);
    HAL_GMAC_WriteHWAddr(&eth->instance, eth->macAddr);

    return 0;
}

/*************************** GMAC TEST ****************************/
#define GMAC_MAX_DEVICES 2

#ifdef SOC_RK3568
#ifdef HAL_GMAC0
/**
 * @brief  Config iomux for GMAC0
 */
static void GMAC0_Iomux_Config(void)
{
    /* GMAC0 iomux */
    HAL_PINCTRL_SetIOMUX(GPIO_BANK2,
                         GPIO_PIN_B6, /* gmac0_rxd0 */
                         PIN_CONFIG_MUX_FUNC1);

    HAL_PINCTRL_SetIOMUX(GPIO_BANK2,
                         GPIO_PIN_C3 | /* gmac0_mdc */
                         GPIO_PIN_C4 | /* gmac0_mdio */
                         GPIO_PIN_C0 | /* gmac0_rxdvcrs */
                         GPIO_PIN_B7 | /* gmac0_rxd1 */
                         GPIO_PIN_A3 | /* gmac0_rxd2 */
                         GPIO_PIN_A4 | /* gmac0_rxd3 */
                         GPIO_PIN_A6 | /* gmac0_txd2 */
                         GPIO_PIN_A7 | /* gmac0_txd3 */
                         GPIO_PIN_A5,  /* gmac0_rxclk */
                         PIN_CONFIG_MUX_FUNC2);

    HAL_PINCTRL_SetIOMUX(GPIO_BANK2,
                         GPIO_PIN_B3 | /* gmac0_txd0 */
                         GPIO_PIN_B4 | /* gmac0_txd1 */
                         GPIO_PIN_B5,  /* gmac0_txen */
                         PIN_CONFIG_MUX_FUNC1 | PIN_CONFIG_DRV_LEVEL2);

    HAL_PINCTRL_SetIOMUX(GPIO_BANK2,
                         GPIO_PIN_A7 | /* gmac0_txd3 */
                         GPIO_PIN_A6,  /* gmac0_txd2 */
                         PIN_CONFIG_MUX_FUNC2 | PIN_CONFIG_DRV_LEVEL2);
    HAL_PINCTRL_SetIOMUX(GPIO_BANK2,   /* gmac0_txclk */
                         PIO_PIN_B0,
                         PIN_CONFIG_MUX_FUNC2 | PIN_CONFIG_DRV_LEVEL1);

#if 0
    /* io-domian: 1.8v or 3.3v for vccio4 */
	WRITE_REG_MASK_WE(GRF->IO_VSEL0,
	                  GRF_IO_VSEL0_POC_VCCIO4_SEL18_MASK,
	                  (1 << GRF_IO_VSEL0_POC_VCCIO4_SEL18_SHIFT));

	WRITE_REG_MASK_WE(GRF->IO_VSEL1,
	                  GRF_IO_VSEL1_POC_VCCIO4_SEL33_MASK,
	                  (0 << GRF_IO_VSEL1_POC_VCCIO4_SEL33_SHIFT));
#endif
}
#endif

#ifdef HAL_GMAC1
/**
 * @brief  Config iomux for GMAC1
 */
static void GMAC1_M1_Iomux_Config(void)
{
    /* GMAC1 M1 iomux */
    HAL_PINCTRL_SetIOMUX(GPIO_BANK4,
                         GPIO_PIN_B6 | /* gmac1_mdcm1 */
                         GPIO_PIN_B7 | /* gmac1_mdiom1 */
                         GPIO_PIN_B1 | /* gmac1_rxdvcrsm1 */
                         GPIO_PIN_A7 | /* gmac1_rxd0m1 */
                         GPIO_PIN_B0 | /* gmac1_rxd1m1 */
                         GPIO_PIN_A1 | /* gmac1_rxd2m1 */
                         GPIO_PIN_A2 | /* gmac1_rxd3m1 */
                         GPIO_PIN_A6 | /* gmac1_txenm1 */
                         GPIO_PIN_A3,  /* gmac1_rxclkm1 */
                         PIN_CONFIG_MUX_FUNC3);

    HAL_PINCTRL_SetIOMUX(GPIO_BANK4,
                         GPIO_PIN_A0,  /* gmac1_txd1m1 */
                         PIN_CONFIG_MUX_FUNC3 | PIN_CONFIG_DRV_LEVEL3);

    HAL_PINCTRL_SetIOMUX(GPIO_BANK4,
                         GPIO_PIN_A4 | /* gmac1_txd0m1 */
                         GPIO_PIN_A5,  /* gmac1_txd1m1 */
                         PIN_CONFIG_MUX_FUNC3 | PIN_CONFIG_DRV_LEVEL2);

    /* GMAC1 M1 iomux */
    HAL_PINCTRL_SetIOMUX(GPIO_BANK3,
                         GPIO_PIN_D6 | /* gmac1_txd2m1 */
                         GPIO_PIN_D7,  /* gmac1_txd3m1 */
                         PIN_CONFIG_MUX_FUNC3 | PIN_CONFIG_DRV_LEVEL2);

    HAL_PINCTRL_IOFuncSelForGMAC1(IOFUNC_SEL_M1);

#if 0
    /* io-domian: 1.8v or 3.3v for vccio6 */
    WRITE_REG_MASK_WE(GRF->IO_VSEL0,
	              GRF_IO_VSEL0_POC_VCCIO6_SEL18_MASK,
	              (1 << GRF_IO_VSEL0_POC_VCCIO6_SEL18_SHIFT));

    WRITE_REG_MASK_WE(GRF->IO_VSEL1,
	              GRF_IO_VSEL1_POC_VCCIO6_SEL33_MASK,
	              (0 << GRF_IO_VSEL1_POC_VCCIO6_SEL33_SHIFT));
#endif
}
#endif

static void GMAC_Iomux_Config(uint8_t id)
{
    if (id == 1) {
        GMAC1_M1_Iomux_Config();
    } else if (id == 0) {
        GMAC0_Iomux_Config();
    }
}
#endif

#ifdef SOC_RK3358
#ifdef HAL_GMAC0
/**
 * @brief  Config iomux for GMAC0
 */
static void GMAC_Iomux_Config(uint8_t id)
{
    /* GMAC iomux */
    HAL_PINCTRL_SetIOMUX(GPIO_BANK2,
                         GPIO_PIN_B1 | /* gmac_mdc */
                         GPIO_PIN_A7 | /* gmac_mdio */
                         GPIO_PIN_A6 | /* gmac_rxdvcrs */
                         GPIO_PIN_A3 | /* gmac_rxd0 */
                         GPIO_PIN_A4 | /* gmac_rxd1 */
                         GPIO_PIN_A5,  /* gmac_rxer */
                         PIN_CONFIG_MUX_FUNC2 | PIN_CONFIG_PUL_DEFAULT);

    HAL_PINCTRL_SetIOMUX(GPIO_BANK2,
                         GPIO_PIN_A2 | /* gmac_txd0 */
                         GPIO_PIN_A1 | /* gmac_txd1 */
                         GPIO_PIN_A0,  /* gmac_txen */
                         PIN_CONFIG_MUX_FUNC2 | PIN_CONFIG_DRV_LEVEL3 | PIN_CONFIG_PUL_DEFAULT);

    HAL_PINCTRL_SetIOMUX(GPIO_BANK2,
                         GPIO_PIN_B2, /* gmac_mclkinout */
                         PIN_CONFIG_MUX_FUNC2 | PIN_CONFIG_DRV_LEVEL3 | PIN_CONFIG_PUL_DEFAULT);
}
#endif
#endif

#ifdef SOC_RK3308
#ifdef HAL_GMAC0
/**
 * @brief  Config iomux for GMAC0
 */
static void GMAC_Iomux_Config(uint8_t id)
{
    /* GMAC0 iomux */
    HAL_PINCTRL_SetIOMUX(GPIO_BANK4,
                         GPIO_PIN_A0 | /* gmac0_rxer_m1 */
                         GPIO_PIN_A1 | /* gmac0_rxdv_m1 */
                         GPIO_PIN_A2 | /* gmac0_rxd0_m1 */
                         GPIO_PIN_A3 | /* gmac0_rxd1_m1 */
                         GPIO_PIN_A4 | /* gmac0_txd0_m1 */
                         GPIO_PIN_A5 | /* gmac0_txd1_m1 */
                         GPIO_PIN_B4 | /* gmac0_clk_m1 */
                         GPIO_PIN_B5 | /* gmac0_mdc_m1 */
                         GPIO_PIN_B6 | /* gmac0_mdio_m1 */
                         GPIO_PIN_B7,  /* gmac0_txen_m1 */
                         PIN_CONFIG_MUX_FUNC2);

    /* set GMAC IOMUX selection to M1 */
    WRITE_REG_MASK_WE(GRF->SOC_CON5,
                      GRF_SOC_CON5_GRF_MAC_MULTI_IOFUNC_SRC_SEL_MASK,
                      (1 << GRF_SOC_CON5_GRF_MAC_MULTI_IOFUNC_SRC_SEL_SHIFT));
}
#endif
#endif

/*************************** GMAC TEST MAIN ****************************/

TEST_GROUP(HAL_GMAC);

TEST_SETUP(HAL_GMAC){
}

TEST_TEAR_DOWN(HAL_GMAC){
}

TEST_GROUP_RUNNER(HAL_GMAC) {
    struct GMAC_ETH_CONFIG *eth;
    struct GMAC_HANDLE *pGMAC;
    int32_t bus, num = 0, i;

    HAL_DBG("\n");
    HAL_DBG("%s\n", __func__);
    HAL_DBG("    GMAC Test:\n");

    num = sizeof(ethConfigTable) / sizeof(struct GMAC_ETH_CONFIG);
    HAL_DBG("    GMAC Num: %ld\n", num);
    for (bus = 0; bus < num; bus++) {
        eth = &ethConfigTable[bus];
        if (eth) {
            pGMAC = &eth->instance;
        } else {
            return;
        }

        /* ionmux */
        GMAC_Iomux_Config(bus);

        HAL_CRU_ClkEnable(eth->halDev->pclkGateID);
        HAL_CRU_ClkEnable(eth->halDev->clkGateID);

        /* Register irq */

        /* PHY reset */
        GMAC_PHY_Reset(eth);

        /* GMAC Init */
        GMAC_Init(bus);

        GMAC_Memory_Init(eth, pGMAC);

        /* Enable GMAC and DMA transmission and reception */
        HAL_GMAC_Start(pGMAC, eth->macAddr);

        /* Update links information */
        PHY_Update_Links(eth, pGMAC, bus);

        /* Dump MAC Regs */
        Dump_Regs(pGMAC);
        /* Dump PHY Regs */
        PHY_Dump(eth, pGMAC);

        for (i = 0; i < GMAC_TEST_TIMES; i++) {
            /* GMAC Send 64 bytes */
            GMAC_Send_Test(eth, pGMAC, 64);
            /* GMAC Send 1500 bytes */
            GMAC_Send_Test(eth, pGMAC, 1500);
            HAL_DelayMs(1000);

            /* GMAC Recv */
            GMAC_Recv_Test(pGMAC);
        }

        HAL_CRU_ClkDisable(eth->halDev->pclkGateID);
        HAL_CRU_ClkDisable(eth->halDev->clkGateID);

        free_align(eth->txBuff);
        free_align(eth->rxBuff);
    }
}
#endif

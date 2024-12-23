/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2022 Rockchip Electronics Co., Ltd.
 */

#include "hal_bsp.h"
#include "hal_base.h"
#include "ipc_conf.h"
#include "board_conf.h"
#include "board.h"

/* Console Uart Port config */
#if (HAL_CONSOLE == 2)
#define CONSOLE_PORT UART2
#define CONSOLE_DEV  g_uart2Dev
#else //default
#define CONSOLE_PORT UART4
#define CONSOLE_DEV  g_uart4Dev
#endif

static void HAL_IODOMAIN_Config(void)
{
    /* VCC IO 2 voltage select 1v8 */
    GRF->SOC_CON0 = (1 << GRF_SOC_CON0_IO_VSEL2_SHIFT) |
                    (GRF_SOC_CON0_IO_VSEL2_MASK << 16);
}

#ifdef PRIMARY_CPU
static void HAL_IOMUX_Uart2M1Config(void)
{
    /* UART2 M1 RX-4D2 TX-4D3 */
    HAL_PINCTRL_SetIOMUX(GPIO_BANK4,
                         GPIO_PIN_D2 |
                         GPIO_PIN_D3,
                         PIN_CONFIG_MUX_FUNC2);
}

static void HAL_IOMUX_Uart4M0Config(void)
{
    /* UART4 M0 RX-4B0 TX-4B1 */
    HAL_PINCTRL_SetIOMUX(GPIO_BANK4,
                         GPIO_PIN_B0 |
                         GPIO_PIN_B1,
                         PIN_CONFIG_MUX_FUNC1);
}

static void console_uart_init(void)
{
    const struct UART_REG *pUart = CONSOLE_PORT;
    const struct HAL_UART_DEV *p_uartDev = &CONSOLE_DEV;
    struct HAL_UART_CONFIG hal_uart_config = {
        .baudRate = UART_BR_1500000,
        .dataBit = UART_DATA_8B,
        .stopBit = UART_ONE_STOPBIT,
        .parity = UART_PARITY_DISABLE,
    };

    if (UART2 == pUart) {
        HAL_IOMUX_Uart2M1Config();
    } else if (UART4 == pUart) {
        HAL_IOMUX_Uart4M0Config();
    }

    HAL_UART_Init(p_uartDev, &hal_uart_config);
}
#endif

#ifdef __GNUC__
int _write(int fd, char *ptr, int len);
#else
int fputc(int ch, FILE *f);
#endif

#ifdef __GNUC__
int _write(int fd, char *ptr, int len)
{
    const struct UART_REG *pUart = CONSOLE_PORT;
    int i = 0;

    /*
     * write "len" of char from "ptr" to file id "fd"
     * Return number of char written.
     *
    * Only work for STDOUT, STDIN, and STDERR
     */
    if (fd > 2) {
        return -1;
    }

    while (*ptr && (i < len)) {
        if (*ptr == '\n') {
            HAL_UART_SerialOutChar(pUart, '\r');
        }
        HAL_UART_SerialOutChar(pUart, *ptr);

        i++;
        ptr++;
    }

    return i;
}
#else
int fputc(int ch, FILE *f)
{
    if (ch == '\n') {
        HAL_UART_SerialOutChar(pUart, '\r');
    }

    HAL_UART_SerialOutChar(pUart, (char)ch);

    return 0;
}
#endif

#ifdef HAL_USING_LOGBUFFER
#if defined(CPU0)
#define LOG_MEM_BASE ((uint32_t)&__share_log0_start__)
#define LOG_MEM_END  ((uint32_t)&__share_log0_end__)
#elif defined(PRIMARY_CPU)
#define LOG_MEM_BASE ((uint32_t)&__share_log1_start__)
#define LOG_MEM_END  ((uint32_t)&__share_log1_end__)
#elif defined(CPU2)
#define LOG_MEM_BASE ((uint32_t)&__share_log2_start__)
#define LOG_MEM_END  ((uint32_t)&__share_log2_end__)
#elif defined(CPU3)
#define LOG_MEM_BASE ((uint32_t)&__share_log3_start__)
#define LOG_MEM_END  ((uint32_t)&__share_log3_end__)
#else
#error "error: Undefined CPU id!"
#endif

static struct ringbuffer_t log_buffer, *plog_buf = NULL;
void log_buffer_init(void)
{
    ringbuffer_init(&log_buffer, (uint8_t *)LOG_MEM_BASE, (int16_t)(LOG_MEM_END - LOG_MEM_BASE));
    plog_buf = &log_buffer;
}

struct ringbuffer_t *get_log_ringbuffer(void)
{
    HAL_ASSERT(plog_buf != NULL);
    return plog_buf;
}
#endif

int rk_printf(const char *fmt, ...)
{
    va_list args;
    uint32_t length, outlen;
    static char hal_log_buf[32 + HAL_CONSOLEBUF_SIZE];
    char *p_log_str = &hal_log_buf[32];

    va_start(args, fmt);
    length = vsnprintf(p_log_str, HAL_CONSOLEBUF_SIZE - 1, fmt, args);
    va_end(args);

    if (length > HAL_CONSOLEBUF_SIZE - 1)
        length = HAL_CONSOLEBUF_SIZE - 1;

    p_log_str[HAL_CONSOLEBUF_SIZE - 1] = 0;


    outlen = 0;
    if (p_log_str[strlen(p_log_str) - 1] == '\n')
    {        
        uint64_t cnt64;
        uint32_t cpu_id, sec, ms, us;
        cpu_id = HAL_CPU_TOPOLOGY_GetCurrentCpuId();
        cnt64 = HAL_GetSysTimerCount();
        us = (uint32_t)((cnt64 / (PLL_INPUT_OSC_RATE / 1000000)) % 1000);
        ms = (uint32_t)((cnt64 / (PLL_INPUT_OSC_RATE / 1000)) % 1000);
        sec = (uint32_t)(cnt64 / PLL_INPUT_OSC_RATE);
        outlen = snprintf(hal_log_buf, 32 - 1 , "[(%d)%d.%03d.%03d] ", cpu_id, sec, ms, us);
    }
    outlen += snprintf(hal_log_buf + outlen, HAL_CONSOLEBUF_SIZE - 1, "%s", p_log_str);

    /* Save log to buffer */
#ifdef HAL_USING_LOGBUFFER
    ringbuffer_put_force(get_log_ringbuffer(), hal_log_buf, outlen);
#endif

    HAL_SPINLOCK_Lock(RK_PRINTF_SPINLOCK_ID);
    printf("%s", hal_log_buf);
    HAL_SPINLOCK_Unlock(RK_PRINTF_SPINLOCK_ID);

    return 0;
}

void Board_Init(void)
{
    HAL_IODOMAIN_Config();

#ifdef HAL_USING_LOGBUFFER
    log_buffer_init();
#endif

#ifdef PRIMARY_CPU
    console_uart_init();
#endif
}

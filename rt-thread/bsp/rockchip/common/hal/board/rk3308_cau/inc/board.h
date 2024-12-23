/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2022 Rockchip Electronics Co., Ltd.
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#include "ringbuffer.h"

extern uint32_t __share_log0_start__[];
extern uint32_t __share_log0_end__[];

extern uint32_t __share_log1_start__[];
extern uint32_t __share_log1_end__[];

extern uint32_t __share_log2_start__[];
extern uint32_t __share_log2_end__[];

extern uint32_t __share_log3_start__[];
extern uint32_t __share_log3_end__[];

/*
 * Get log ringbuffer pointer
 */
struct ringbuffer_t *get_log_ringbuffer(void);

/*
 * Printf with time stamp
 */
int rk_printf(const char *fmt, ...);

/*
 * Board initial
 */
void Board_Init(void);

#endif

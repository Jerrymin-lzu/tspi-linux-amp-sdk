/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2023 Rockchip Electronics Co., Ltd.
 */

#ifndef __ECNR_H__
#define __ECNR_H__

#include "hal_conf.h"
#include "hal_base.h"

#ifdef HAL_USING_ECNR_APP

enum
{
    ECNR_INIT = 0,
    ECNR_DEINIT,
    ECNR_PROCESS,
};

void ecnr_ept_cb(void *param);

#endif

#endif

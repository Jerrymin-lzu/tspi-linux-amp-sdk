/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2023 Rockchip Electronics Co., Ltd.
 */

#ifndef __RPMSG_APP_H__
#define __RPMSG_APP_H__

#include "ecnr.h"

#ifdef HAL_USING_ECNR_APP

#include "rpmsg_cmd.h"
#include "middleware_conf.h"

enum
{
    RPMSG_ECN = 0,
    RPMSG_MAX,
};

static struct rpmsg_cmd_table_t rpmsg_ept_table[RPMSG_MAX] =
{
    { RPMSG_CMD_GET_ECN_USAGE, ecnr_ept_cb },
};
static struct rpmsg_ept_handle_t rpmsg_ept_handle[RPMSG_MAX];

#endif

#endif

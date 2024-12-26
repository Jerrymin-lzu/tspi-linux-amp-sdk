/**
  * Copyright (c) 2023 Rockchip Electronics Co., Ltd
  *
  * SPDX-License-Identifier: Apache-2.0
  */

#include "hal_conf.h"

#ifdef HAL_USING_RPMSG_CMD

#include "hal_base.h"
#include "queue.h"
#include "middleware_conf.h"
#include "rpmsg_cmd.h"

#if defined(CPU0)
#define REMOTE_ID  REMOTE_ID_0
#define MASTER_EPT EPT_M1R0_SYSCMD
#elif defined(CPU2)
#define REMOTE_ID  REMOTE_ID_2
#define MASTER_EPT EPT_M1R2_SYSCMD
#elif defined(CPU3)
#define REMOTE_ID  REMOTE_ID_3
#define MASTER_EPT EPT_M1R3_SYSCMD
#else
#error "error: Undefined CPU id!"
#endif
#define REMOTE_EPT EPT_M2R_ADDR(MASTER_EPT)

static struct rpmsg_ept_handle_t rpmsg_cmd_handle;

static struct rpmsg_cmd_table_t rpmsg_cmd_table[] =
{
    { RPMSG_CMD_GET_CPU_USAGE, rpmsg_cmd_cpuusage_callback },
};

/*
 * Initial epts:
 */
int rpmsg_cmd_init(void *arg)
{
    /* Use rpmsg_cmd_ept_init() initial ept & creat thread */
    rpmsg_cmd_ept_init(&rpmsg_cmd_handle,
                       MASTER_ID, REMOTE_ID, REMOTE_EPT,
                       rpmsg_cmd_table,
                       sizeof(rpmsg_cmd_table) / sizeof(struct rpmsg_cmd_table_t),
                       rpmsg_cmd_ept_do_work, &rpmsg_cmd_handle);

    return HAL_OK;
}

int rpmsg_cmd_process(void *arg)
{
    int32_t ret = HAL_OK;
    struct rpmsg_ept_handle_t *handle = &rpmsg_cmd_handle;

    if (handle->work_fun) {
        ret = handle->work_fun(handle->arg);
    }

    return ret;
}

#endif

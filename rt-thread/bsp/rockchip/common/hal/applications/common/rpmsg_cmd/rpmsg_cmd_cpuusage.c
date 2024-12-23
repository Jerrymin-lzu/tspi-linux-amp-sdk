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

void rpmsg_cmd_cpuusage_callback(void *param)
{
    struct rpmsg_cmd_data_t *p_rpmsg_data = (struct rpmsg_cmd_data_t *)param;
    struct rpmsg_cmd_head_t *head = &p_rpmsg_data->head;

    *(uint32_t *)head->addr = HAL_GetCPUUsage();

    head->cmd = RPMSG_CMD2ACK(head->cmd);
    rpmsg_cmd_send(p_rpmsg_data, RL_BLOCK);
}

#endif

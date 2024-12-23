/**
  * Copyright (c) 2023 Rockchip Electronics Co., Ltd
  *
  * SPDX-License-Identifier: Apache-2.0
  */

#ifndef __RPMSG_CMD_H__
#define __RPMSG_CMD_H__

#include "hal_conf.h"

#ifdef HAL_USING_RPMSG_CMD

#include "queue.h"
#include "middleware_conf.h"

/* RPMsg cmd callback */
struct rpmsg_cmd_table_t
{
    uint32_t cmd;
    void (*callback)(void *param);
};

/* RPMsg cmd handle */
struct rpmsg_ept_handle_t
{
    uint32_t master_id;
    uint32_t remote_id;

    struct rpmsg_lite_instance *instance;
    struct rpmsg_lite_endpoint *ept;
    uint32_t endpoint;

    uint32_t cmd_counter;
    struct rpmsg_cmd_table_t *cmd_table;

    queue_t *mq;
    int (*work_fun)(void *arg);
    void *arg;
};

/* RPMSG cmd head format */
struct rpmsg_cmd_head_t
{
    uint32_t type;
    uint32_t cmd;
    void *priv;
    void *addr;
};

/* RPMSG cmd data format */
struct rpmsg_cmd_data_t
{
    struct rpmsg_cmd_head_t head;
    struct rpmsg_ept_handle_t *handle;
    uint32_t endpoint;
};

/* RPMsg mq max count */
#define RPMSG_MQ_MAX 32UL

/* RPMsg cmd type define */
#define RPMSG_TYPE_DIRECT 1UL
#define RPMSG_TYPE_URGENT 2UL
#define RPMSG_TYPE_NORMAL 3UL

/* RPMsg cmd define */
#define RPMSG_CMD2ACK(val) (val | 0x01UL)      /* CMD covert to ACK */
#define RPMSG_ACK2CMD(val) (val & (~0x01UL))   /* ACK covert to CMD */

#define RPMSG_CMD_GET_CPU_USAGE ((1UL << 1) + 0)
#define RPMSG_ACK_GET_CPU_USAGE ((1UL << 1) + 1)
#define RPMSG_CMD_GET_ECN_USAGE ((2UL << 1) + 0)
#define RPMSG_ACK_GET_ECN_USAGE ((2UL << 1) + 1)

/* RPMsg CMD API Functions */
void rpmsg_cmd_ept_init(struct rpmsg_ept_handle_t *handle,
                        uint32_t master_id, uint32_t remote_id, uint32_t endpoint,
                        struct rpmsg_cmd_table_t *cmd_table, uint32_t cmd_counter,
                        int (*work_fun)(void *arg), void *arg);
void rpmsg_cmd_ept_work_init(struct rpmsg_ept_handle_t *handle,
                             int (*work_fun)(void *arg), void *arg);
int rpmsg_cmd_ept_do_work(void *arg);
int rpmsg_cmd_send(struct rpmsg_cmd_data_t *p_rpmsg_data, uint32_t timeout);
void *rpmsg_cmd_table_callback_find(uint32_t cmd, struct rpmsg_cmd_table_t *table, uint32_t size);

/* RPMsg Command Callback Functions */
void rpmsg_cmd_cpuusage_callback(void *param);

#endif

#endif

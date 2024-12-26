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

void *rpmsg_cmd_table_callback_find(uint32_t cmd, struct rpmsg_cmd_table_t *table, uint32_t size)
{
    uint32_t i;

    for (i = 0; i < size; i++) {
        if (table[i].cmd == cmd) {
            return (void *)(table[i].callback);
        }
    }

    return NULL;
}

int rpmsg_cmd_send(struct rpmsg_cmd_data_t *p_rpmsg_data, uint32_t timeout)
{
    int32_t ret;

    HAL_ASSERT(p_rpmsg_data);
    HAL_ASSERT(p_rpmsg_data->handle);

    struct rpmsg_cmd_head_t *head = &p_rpmsg_data->head;
    struct rpmsg_ept_handle_t *handle = p_rpmsg_data->handle;

#ifdef PRIMARY_CPU
    ret = rpmsg_lite_send(handle->instance, handle->ept,
                          EPT_M2R_ADDR(handle->endpoint),
                          head, sizeof(struct rpmsg_cmd_head_t),
                          timeout);
#else
    ret = rpmsg_lite_send(handle->instance, handle->ept,
                          EPT_R2M_ADDR(handle->endpoint),
                          head, sizeof(struct rpmsg_cmd_head_t),
                          timeout);
#endif

    return ret;
}

static int32_t rpmsg_ept_cb(void *payload, uint32_t payload_len, uint32_t src, void *priv)
{
    struct rpmsg_ept_handle_t *handle = (struct rpmsg_ept_handle_t *)priv;
    struct rpmsg_cmd_head_t *head = (struct rpmsg_cmd_head_t *)payload;

    HAL_ASSERT(payload_len == sizeof(struct rpmsg_cmd_head_t));

    if (head->type == RPMSG_TYPE_DIRECT) {
        void (*callback)(void *param);
        callback = rpmsg_cmd_table_callback_find(head->cmd, handle->cmd_table, handle->cmd_counter);
        if (callback) {
            struct rpmsg_cmd_data_t rpmsg_data;

            memcpy(&rpmsg_data.head, payload, sizeof(struct rpmsg_cmd_data_t));
            rpmsg_data.handle = handle;
            rpmsg_data.endpoint = src;

            callback(&rpmsg_data);
        }
    } else if (head->type == RPMSG_TYPE_URGENT) {
        struct rpmsg_cmd_data_t *p_rpmsg_data = malloc(sizeof(struct rpmsg_cmd_data_t));
        HAL_ASSERT(p_rpmsg_data);
        memcpy(&p_rpmsg_data->head, payload, sizeof(struct rpmsg_cmd_data_t));
        p_rpmsg_data->handle = handle;
        p_rpmsg_data->endpoint = src;

        /* push queue */
        qdata_t qdata;
        qdata.data = p_rpmsg_data;
        qdata.size = sizeof(struct rpmsg_cmd_data_t);
        queue_push_urgent(handle->mq, &qdata);
    } else {
        struct rpmsg_cmd_data_t *p_rpmsg_data = malloc(sizeof(struct rpmsg_cmd_data_t));
        HAL_ASSERT(p_rpmsg_data);
        memcpy(&p_rpmsg_data->head, payload, sizeof(struct rpmsg_cmd_data_t));
        p_rpmsg_data->handle = handle;
        p_rpmsg_data->endpoint = src;

        /* push queue */
        qdata_t qdata;
        qdata.data = p_rpmsg_data;
        qdata.size = sizeof(struct rpmsg_cmd_data_t);
        queue_push(handle->mq, &qdata);        
    }

    return RL_RELEASE;
}

int rpmsg_cmd_ept_do_work(void *arg)
{
    struct rpmsg_ept_handle_t *handle = (struct rpmsg_ept_handle_t *)arg;

    if (!queue_empty(handle->mq)) {
        qdata_t *p_qdata = queue_front(handle->mq);
        struct rpmsg_cmd_data_t *p_rpmsg_data = (struct rpmsg_cmd_data_t *)p_qdata->data;
        struct rpmsg_cmd_head_t *head = &p_rpmsg_data->head;
        void (*callback)(void *param) = rpmsg_cmd_table_callback_find(head->cmd, handle->cmd_table, handle->cmd_counter);

        if (callback) {
            callback(p_rpmsg_data);
        }

        free(p_qdata->data);

        queue_pop(handle->mq);
    }

    if (!queue_empty(handle->mq)) {
        return HAL_BUSY;
    }

    return HAL_OK;
}

void rpmsg_cmd_ept_init(struct rpmsg_ept_handle_t *handle,
                        uint32_t master_id, uint32_t remote_id, uint32_t endpoint,
                        struct rpmsg_cmd_table_t *cmd_table, uint32_t cmd_counter,
                        int (*work_fun)(void *arg), void *arg)
{
    HAL_ASSERT(handle);

    handle->master_id = master_id;
    handle->remote_id = remote_id;
    handle->endpoint = endpoint;
    handle->cmd_table = cmd_table;
    handle->cmd_counter = cmd_counter;

#ifdef PRIMARY_CPU
    handle->instance = rpmsg_master_get_instance(handle->master_id, handle->remote_id);
#else
    handle->instance = rpmsg_remote_get_instance(handle->master_id, handle->remote_id);
#endif
    HAL_ASSERT(handle->instance);

    handle->ept = rpmsg_lite_create_ept(handle->instance, handle->endpoint, rpmsg_ept_cb, handle);
    HAL_ASSERT(handle->ept);

    handle->work_fun = work_fun;
    handle->arg = arg;

    handle->mq = (queue_t *)malloc(sizeof(queue_t));
    HAL_ASSERT(handle->mq);

    queue_init(handle->mq);
}

void rpmsg_cmd_ept_work_init(struct rpmsg_ept_handle_t *handle,
                             int (*work_fun)(void *arg), void *arg)
{
    HAL_ASSERT(handle);
    HAL_ASSERT(handle->master_id == MASTER_ID);
    HAL_ASSERT(handle->remote_id != MASTER_ID);
    HAL_ASSERT(handle->remote_id <= REMOTE_ID_3);

#ifdef PRIMARY_CPU
    handle->instance = rpmsg_master_get_instance(handle->master_id, handle->remote_id);
#else
    handle->instance = rpmsg_remote_get_instance(handle->master_id, handle->remote_id);
#endif
    HAL_ASSERT(handle->instance);

    handle->ept = rpmsg_lite_create_ept(handle->instance, handle->endpoint, rpmsg_ept_cb, handle);
    HAL_ASSERT(handle->ept);

    handle->work_fun = work_fun;
    handle->arg = arg;

    handle->mq = (queue_t *)malloc(sizeof(queue_t));
    HAL_ASSERT(handle->mq);

    queue_init(handle->mq);
}

#endif

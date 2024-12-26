/**
  * Copyright (c) 2023 Rockchip Electronics Co., Ltd
  *
  * SPDX-License-Identifier: Apache-2.0
  */

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "list.h"

/*
 * Queue list structure
 */

struct queue_data_t {
    uint32_t type;
    void *data;    // output
    uint32_t size;
};
typedef struct queue_data_t qdata_t;                  /**< Type for queue data. */

/*
 * Queue node structure
 */
struct queue_node_t {
    list_t list;
    qdata_t qdat;
};
typedef struct queue_node_t queue_t;                  /**< Type for queue node. */

/*
 * initialize a queue
 *
 * pq: queue to be initialized
 */
void queue_init(queue_t *pq);

/*
 * destory a queue
 *
 * pq: queue to be initialized
 */
void queue_destory(queue_t *pq);

/*
 * push a queue
 *
 * pq:   queue to be push
 * data: data of pq
 */
void queue_push(queue_t *pq, qdata_t *data);
/*
 * push a queue urgent
 *
 * pq:   queue to be push
 * data: data of pq
 */
void queue_push_urgent(queue_t *pq, qdata_t *data);

/*
 * pop a queue
 *
 * pq:   queue to be pop
 */
void queue_pop(queue_t *pq);

/*
 * get the head element of pq
 *
 * pq: queue pointer
 */
qdata_t *queue_front(queue_t *pq);

/*
 * get the tail element of pq
 *
 * pq: queue pointer
 */
qdata_t *queue_back(queue_t *pq);

/*
 * check queue is empty
 *
 * pq: queue pointer
 */
bool queue_empty(queue_t *pq);

/*
 * get the size of pq
 *
 * pq: queue pointer
 */
unsigned int queue_size(queue_t *pq);

#endif

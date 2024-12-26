/**
  * Copyright (c) 2023 Rockchip Electronics Co., Ltd
  *
  * SPDX-License-Identifier: Apache-2.0
  */

#ifndef _RINGBUFFER_H__
#define _RINGBUFFER_H__

/* ring buffer */
struct ringbuffer_t {
    uint8_t *buffer_ptr;
    uint16_t read_mirror : 1;
    uint16_t read_index : 15;
    uint16_t write_mirror : 1;
    uint16_t write_index : 15;
    int16_t buffer_size;
};

enum ringbuffer_state_t {
    RINGBUFFER_EMPTY,
    RINGBUFFER_FULL,
    RINGBUFFER_HALFFULL,
};

/**
 * RingBuffer for DeviceDriver
 *
 * Please note that the ring buffer implementation of RT-Thread
 * has no thread wait or resume feature.
 */
void ringbuffer_init(struct ringbuffer_t *rb, uint8_t *pool, int16_t size);
void ringbuffer_reset(struct ringbuffer_t *rb);
uint32_t ringbuffer_put(struct ringbuffer_t *rb, const uint8_t *ptr, uint16_t length);
uint32_t ringbuffer_put_force(struct ringbuffer_t *rb, const uint8_t *ptr, uint16_t length);
uint32_t ringbuffer_putchar(struct ringbuffer_t *rb, const uint8_t ch);
uint32_t ringbuffer_putchar_force(struct ringbuffer_t *rb, const uint8_t ch);
uint32_t ringbuffer_get(struct ringbuffer_t *rb, uint8_t *ptr, uint16_t length);
uint32_t ringbuffer_getchar(struct ringbuffer_t *rb, uint8_t *ch);
uint32_t ringbuffer_data_len(struct ringbuffer_t *rb);
struct ringbuffer_t *ringbuffer_create(uint16_t length);
void ringbuffer_destroy(struct ringbuffer_t *rb);

#endif

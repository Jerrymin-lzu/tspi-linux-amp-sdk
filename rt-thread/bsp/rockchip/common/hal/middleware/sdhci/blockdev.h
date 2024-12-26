/*
 * Copyright 2012 Google Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __SDHCI_BLOCKDEV_H__
#define __SDHCI_BLOCKDEV_H__

#include "hal_bsp.h"
#include "hal_base.h"

typedef unsigned long	u64;
typedef unsigned char	u8;
typedef signed char	s8;
typedef unsigned short	u16;
typedef signed short	s16;
typedef unsigned int	u32;
typedef signed int	s32;

#define PRINT_E 	printf
#define udelay		HAL_DelayUs
#define mdelay		HAL_DelayMs
#define MAX		HAL_MAX
#define MIN		HAL_MIN
#define FALSE		HAL_FALSE
#define TRUE		HAL_TRUE

typedef uint64_t lba_t;

typedef struct BlockDevOps {
	int card_id;
} BlockDevOps;

typedef struct BlockDev {
	BlockDevOps ops;

	const char *name;
	int external_gpt;
	unsigned int block_size;
	/* If external_gpt = 0, then stream_block_count may be 0, indicating
	 * that the block_count value applies for both read/write and streams */
	lba_t block_count;		/* size addressable by read/write */
	lba_t stream_block_count;	/* size addressible by new_stream */
} BlockDev;

typedef struct BlockDevCtrlrOps {
	int (*update)(struct BlockDevCtrlrOps *me);
	/*
	 * Check if a block device is owned by the ctrlr. 1 = success, 0 =
	 * failure
	 */
	int (*is_bdev_owned)(struct BlockDevCtrlrOps *me, BlockDev *bdev);
} BlockDevCtrlrOps;

typedef struct BlockDevCtrlr {
	BlockDevCtrlrOps ops;

	int need_update;
} BlockDevCtrlr;

typedef enum {
	BLOCKDEV_FIXED,
	BLOCKDEV_REMOVABLE,
} blockdev_type_t;

#endif
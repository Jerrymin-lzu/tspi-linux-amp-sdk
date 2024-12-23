#!/bin/sh

# SPDX-License-Identifier: BSD-3-Clause */

# Copyright (c) 2022 Rockchip Electronics Co., Ltd.

AUDIO_ALGORITHM_PATH=$(HAL_PATH)/applications/vehicle

INCLUDES += \
-I"$(AUDIO_ALGORITHM_PATH)" \
-I"$(AUDIO_ALGORITHM_PATH)/include" \
-I"$(AUDIO_ALGORITHM_PATH)/algorithm/ecnr"

SRC_DIRS += \
    $(AUDIO_ALGORITHM_PATH) \
    $(AUDIO_ALGORITHM_PATH)/algorithm/ecnr

LDFLAGS += -L$(AUDIO_ALGORITHM_PATH)/lib/ -laec_bf_process -lrkaudio_common

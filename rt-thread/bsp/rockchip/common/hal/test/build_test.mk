# SPDX-License-Identifier: BSD-3-Clause */

# Copyright (c) 2020 Rockchip Electronics Co., Ltd.

#-------------------------------------------------------------------------
# set HAL_TEST_PATH
#-------------------------------------------------------------------------
HAL_TEST_INC += \
-I"$(HAL_PATH)/test/unity/src" \
-I"$(HAL_PATH)/test/unity/extras/fixture/src" \
-I"$(HAL_PATH)/test" \
-I"$(HAL_PATH)/test/coremark" \
-I"$(HAL_PATH)/test/coremark/barebones" \

HAL_TEST_SRC += \
    $(HAL_PATH)/test/unity/extras/fixture/src \
    $(HAL_PATH)/test/unity/src \
    $(HAL_PATH)/test/hal \
    $(HAL_PATH)/test/hal/psram \
    $(HAL_PATH)/test \
    $(HAL_PATH)/test/coremark \
    $(HAL_PATH)/test/coremark/barebones \

SRC_DIRS += $(HAL_TEST_SRC)
INCLUDES += $(HAL_TEST_INC)

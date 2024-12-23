/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2023 Rockchip Electronics Co., Ltd.
 */

#include "hal_conf.h"
#include "hal_base.h"

#ifdef HAL_USING_ECNR_APP

#include "rpmsg_app.h"

/**
 * @brief  Init rpmsg algorithm applications.
 * @param  mept: master ept.
 * @param  rid: remote cpu id.
 * @param  aid: algorithm id.
 * @return NULL
 */
HAL_Status rpmsg_app_init(int mept, int rid, int aid)
{
    rpmsg_cmd_ept_init(&rpmsg_ept_handle[aid], MASTER_ID,
                       rid, EPT_M2R_ADDR(mept),
                       &rpmsg_ept_table[aid],
                       sizeof(rpmsg_ept_table) / sizeof(struct rpmsg_cmd_table_t),
                       rpmsg_cmd_ept_do_work, &rpmsg_ept_handle[aid]);

    return HAL_OK;
}

/**
 * @brief  rpmsg algorithm process.
 * @param  aid: algorithm id.
 * @return HAL_Status
 */
HAL_Status rpmsg_app_process(int aid)
{
    uint32_t ret = HAL_OK;

    struct rpmsg_ept_handle_t *handle = &rpmsg_ept_handle[aid];

    if (handle->work_fun)
    {
        ret = handle->work_fun(handle->arg);
    }

    return ret;
}

#endif


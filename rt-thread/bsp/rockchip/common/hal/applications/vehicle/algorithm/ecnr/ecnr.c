/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2022 Rockchip Electronics Co., Ltd.
 */

#include "skv.h"
#include "ecnr.h"
#include "hal_conf.h"

#ifdef HAL_USING_ECNR_APP

#include "rpmsg_cmd.h"

static RTSkvParam *g_ecnr_param;
static SkvFrameParam *g_ecnr_fparam;

static void ecnr_param_printf(void)
{
    printf("==== HAL SkvFrameParam Print ====\n");
    printf("fparam->samplebits:%d\n", g_ecnr_fparam->bits);
    printf("fparam->samplerate:%d\n", g_ecnr_fparam->rate);
    printf("fparam->period_size:%d\n", g_ecnr_fparam->period_size);
    printf("fparam->in_channels:%d\n", g_ecnr_fparam->in_channels);
    printf("fparam->out_channels:%d\n", g_ecnr_fparam->out_channels);
    printf("fparam->src_channels:%d\n", g_ecnr_fparam->src_channels);
    printf("fparam->ref_channels:%d\n", g_ecnr_fparam->ref_channels);

    printf("==== HAL RTSkvParam Print ====\n");
    printf("param->model:%x\n", g_ecnr_param->model);

    printf("==== HAL SkvAecParam Print ====\n");
    printf("param->aec->pos:%d\n", g_ecnr_param->aec->pos);
    printf("param->aec->drop_ref_channel:%d\n", g_ecnr_param->aec->drop_ref_channel);
    printf("param->aec->aec_mode_en:%d\n", g_ecnr_param->aec->aec_mode_en);
    printf("param->aec->delay_len:%d\n", g_ecnr_param->aec->delay_len);
    printf("param->aec->look_ahead:%d\n", g_ecnr_param->aec->look_ahead);

    printf("==== HAL SkvBeamFormParam Print ====\n");
    printf("param->bf->model_en:%d\n", g_ecnr_param->bf->model_en);
    printf("param->bf->ref_pos:%d\n", g_ecnr_param->bf->ref_pos);
    printf("param->bf->targ:%d\n", g_ecnr_param->bf->targ);
    printf("param->bf->num_ref_channel:%d\n", g_ecnr_param->bf->num_ref_channel);
    printf("param->bf->drop_ref_channel:%d\n", g_ecnr_param->bf->drop_ref_channel);

    printf("==== HAL SkvDereverbParam Print ====\n");
    printf("param->bf->dereverb->rlsLg:%d\n", g_ecnr_param->bf->dereverb->rlsLg);
    printf("param->bf->dereverb->curveLg:%d\n", g_ecnr_param->bf->dereverb->curveLg);
    printf("param->bf->dereverb->delay:%d\n", g_ecnr_param->bf->dereverb->delay);
    printf("param->bf->dereverb->forgetting:%f\n", g_ecnr_param->bf->dereverb->forgetting);
    printf("param->bf->dereverb->t60:%f\n", g_ecnr_param->bf->dereverb->t60);
    printf("param->bf->dereverb->coCoeff:%f\n", g_ecnr_param->bf->dereverb->coCoeff);

    printf("==== HAL SkvAesParam Print ====\n");
    printf("param->bf->aes->beta_up:%f\n", g_ecnr_param->bf->aes->beta_up);
    printf("param->bf->aes->beta_down:%f\n", g_ecnr_param->bf->aes->beta_down);

    printf("==== HAL SkvNlpParam Print ====\n");
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 8; j++)
            printf("param->bf->nlp->nlp16k[%d][%d]:%d\n", j, i, g_ecnr_param->bf->nlp->nlp16k[j][i]);

    printf("==== HAL SkvAnrParam Print ====\n");
    printf("param->bf->anr->noiseFactor:%f\n", g_ecnr_param->bf->anr->noiseFactor);
    printf("param->bf->anr->swU:%d\n", g_ecnr_param->bf->anr->swU);
    printf("param->bf->anr->psiMin:%f\n", g_ecnr_param->bf->anr->psiMin);
    printf("param->bf->anr->psiMax:%f\n", g_ecnr_param->bf->anr->psiMax);
    printf("param->bf->anr->fGmin:%f\n", g_ecnr_param->bf->anr->fGmin);

    printf("==== HAL SkvAgcParam Print ====\n");
    printf("param->bf->agc->attack_time:%f\n", g_ecnr_param->bf->agc->attack_time);
    printf("param->bf->agc->release_time:%f\n", g_ecnr_param->bf->agc->release_time);
    printf("param->bf->agc->max_gain:%f\n", g_ecnr_param->bf->agc->max_gain);
    printf("param->bf->agc->max_peak:%f\n", g_ecnr_param->bf->agc->max_peak);
    printf("param->bf->agc->fRth0:%f\n", g_ecnr_param->bf->agc->fRth0);
    printf("param->bf->agc->fRk0:%f\n", g_ecnr_param->bf->agc->fRk0);
    printf("param->bf->agc->fRth1:%f\n", g_ecnr_param->bf->agc->fRth1);
    printf("param->bf->agc->fs:%d\n", g_ecnr_param->bf->agc->fs);
    printf("param->bf->agc->frmlen:%f\n", g_ecnr_param->bf->agc->frmlen);
    printf("param->bf->agc->attenuate_time:%f\n", g_ecnr_param->bf->agc->attenuate_time);
    printf("param->bf->agc->fRth2:%f\n", g_ecnr_param->bf->agc->fRth2);
    printf("param->bf->agc->fRk1:%f\n", g_ecnr_param->bf->agc->fRk1);
    printf("param->bf->agc->fRk2:%f\n", g_ecnr_param->bf->agc->fRk2);
    printf("param->bf->agc->fLineGainDb:%f\n", g_ecnr_param->bf->agc->fLineGainDb);
    printf("param->bf->agc->swSmL0:%d\n", g_ecnr_param->bf->agc->swSmL0);
    printf("param->bf->agc->swSmL1:%d\n", g_ecnr_param->bf->agc->swSmL1);
    printf("param->bf->agc->swSmL2:%d\n", g_ecnr_param->bf->agc->swSmL2);

    printf("==== HAL SkvCngParam Print ====\n");
    printf("param->bf->cng->fGain:%f\n", g_ecnr_param->bf->cng->fGain);
    printf("param->bf->cng->fMpy:%f\n", g_ecnr_param->bf->cng->fMpy);
    printf("param->bf->cng->fSmoothAlpha:%f\n", g_ecnr_param->bf->cng->fSmoothAlpha);
    printf("param->bf->cng->fSpeechGain:%f\n", g_ecnr_param->bf->cng->fSpeechGain);

    printf("==== HAL SkvDtdParam Print ====\n");
    printf("param->bf->dtd->ksiThd_high:%f\n", g_ecnr_param->bf->dtd->ksiThd_high);
    printf("param->bf->dtd->ksiThd_low:%f\n", g_ecnr_param->bf->dtd->ksiThd_low);

    printf("==== HAL SkvEqParam Print ====\n");
    printf("param->bf->eq->shwParaLen:%d\n", g_ecnr_param->bf->eq->shwParaLen);
    for (int i = 0; i < 5; i++)
    {
        printf("param->bf->eq->pfCoeff:");
        for (int j = 0; j < 13; j++)
        {
            printf(" %d", g_ecnr_param->bf->eq->pfCoeff[i][j]);
        }
        printf("\n");
    }

    printf("==== HAL mic_chns_map Print ====\n");
    for (int i = 0; i < g_ecnr_fparam->src_channels; i++)
        printf("param->aec->mic_chns_map[%d];%d\n", i, g_ecnr_param->aec->mic_chns_map[i]);
}

/* ecnr algorithm handle */
void *g_ecnr_st_ptr;

/**
 * @brief  anc algorithm init.
 * @param  aparam: parameters address.
 * @return HAL_Status
 */
static HAL_Status ecnr_app_init(void *aparam)
{
    uint32_t *command = (uint32_t *)aparam;
    uint32_t offset = (uint32_t)command[1];

    g_ecnr_fparam = malloc(sizeof(SkvFrameParam));
    memcpy(g_ecnr_fparam, offset, sizeof(SkvFrameParam));
    offset = offset + sizeof(SkvFrameParam);

    g_ecnr_param = malloc(sizeof(RTSkvParam));
    memcpy(g_ecnr_param, offset, sizeof(RTSkvParam));
    offset = offset + sizeof(RTSkvParam);

    g_ecnr_param->aec = malloc(sizeof(SkvAecParam));
    memcpy(g_ecnr_param->aec, offset, sizeof(SkvAecParam));
    offset = offset + sizeof(SkvAecParam);

    g_ecnr_param->bf = malloc(sizeof(SkvBeamFormParam));
    memcpy(g_ecnr_param->bf, offset, sizeof(SkvBeamFormParam));
    offset = offset + sizeof(SkvBeamFormParam);

    g_ecnr_param->bf->dereverb = malloc(sizeof(SkvDereverbParam));
    memcpy(g_ecnr_param->bf->dereverb, offset, sizeof(SkvDereverbParam));
    offset = offset + sizeof(SkvDereverbParam);

    g_ecnr_param->bf->aes = malloc(sizeof(SkvAesParam));
    memcpy(g_ecnr_param->bf->aes, offset, sizeof(SkvAesParam));
    offset = offset + sizeof(SkvAesParam);

    g_ecnr_param->bf->nlp = malloc(sizeof(SkvNlpParam));
    memcpy(g_ecnr_param->bf->nlp, offset, sizeof(SkvNlpParam));
    offset = offset + sizeof(SkvNlpParam);

    g_ecnr_param->bf->anr = malloc(sizeof(SkvAnrParam));
    memcpy(g_ecnr_param->bf->anr, offset, sizeof(SkvAnrParam));
    offset = offset + sizeof(SkvAnrParam);

    g_ecnr_param->bf->agc = malloc(sizeof(SkvAgcParam));
    memcpy(g_ecnr_param->bf->agc, offset, sizeof(SkvAgcParam));
    offset = offset + sizeof(SkvAgcParam);

    g_ecnr_param->bf->cng = malloc(sizeof(SkvCngParam));
    memcpy(g_ecnr_param->bf->cng, offset, sizeof(SkvCngParam));
    offset = offset + sizeof(SkvCngParam);

    g_ecnr_param->bf->dtd = malloc(sizeof(SkvDtdParam));
    memcpy(g_ecnr_param->bf->dtd, offset, sizeof(SkvDtdParam));
    offset = offset + sizeof(SkvDtdParam);

    g_ecnr_param->bf->eq = malloc(sizeof(SkvEqParam));
    memcpy(g_ecnr_param->bf->eq, offset, sizeof(SkvEqParam));
    offset = offset + sizeof(SkvEqParam);

    g_ecnr_param->aec->mic_chns_map = malloc(g_ecnr_fparam->src_channels * sizeof(short));
    memcpy(g_ecnr_param->aec->mic_chns_map, offset, g_ecnr_fparam->src_channels * sizeof(short));

    //ecnr_param_printf();
    //rkaudio_param_printf(g_ecnr_fparam->src_channels, g_ecnr_fparam->ref_channels, g_ecnr_param);

    g_ecnr_st_ptr = rkaudio_preprocess_init(g_ecnr_fparam->rate, g_ecnr_fparam->bits, g_ecnr_fparam->src_channels,
                                            g_ecnr_fparam->ref_channels, g_ecnr_param);
    if (g_ecnr_st_ptr == NULL)
    {
        printf("Failed to create audio preprocess handle\n");
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
 * @brief  anc algorithm deinit.
 * @param  aparam: parameters address.
 * @return HAL_Status
 */
static HAL_Status ecnr_app_deinit(void *aparam)
{
    uint32_t *command = (uint32_t *)aparam;

    rkaudio_preprocess_destory(g_ecnr_st_ptr);

    free(g_ecnr_fparam);

    free(g_ecnr_param->aec->mic_chns_map);
    free(g_ecnr_param->aec);

    free(g_ecnr_param->bf->eq);
    free(g_ecnr_param->bf->aes);
    free(g_ecnr_param->bf->anr);
    free(g_ecnr_param->bf->agc);
    free(g_ecnr_param->bf->nlp);
    free(g_ecnr_param->bf->cng);
    free(g_ecnr_param->bf->dtd);
    free(g_ecnr_param->bf->dereverb);
    free(g_ecnr_param->bf);

    free(g_ecnr_param);

    return HAL_OK;
}

/**
 * @brief  anc algorithm process.
 * @param  aparam: parameters address.
 * @return HAL_Status
 */
static HAL_Status ecnr_app_process(void *aparam)
{
    char *send_buffer;
    char *recv_buffer;

    uint32_t *command = (uint32_t *)aparam;

    recv_buffer = (char *)command[1];
    send_buffer = (char *)command[2];

    int is_wakeup = 0;
    int in_size = g_ecnr_fparam->period_size * g_ecnr_fparam->in_channels * 2;

    rkaudio_preprocess_short(g_ecnr_st_ptr, (short *)recv_buffer, send_buffer, in_size / 2, &is_wakeup);

    return HAL_OK;
}

void ecnr_ept_cb(void *param)
{
    struct rpmsg_cmd_data_t *p_rpmsg_data = (struct rpmsg_cmd_data_t *)param;
    struct rpmsg_cmd_head_t *head = &p_rpmsg_data->head;

    int ret = HAL_OK;
    uint32_t *command = (uint32_t *)head->addr;

    switch (command[0])
    {
    case ECNR_INIT:
        ret = ecnr_app_init(head->addr);
        break;
    case ECNR_DEINIT:
        ret = ecnr_app_deinit(head->addr);
        break;
    case ECNR_PROCESS:
        ret = ecnr_app_process(head->addr);
        break;
    default:
        printf("Function not implemented\n");
        ret = HAL_ERROR;
        break;
    }

    if (ret == HAL_OK)
    {
        head->cmd = RPMSG_CMD2ACK(head->cmd);
        rpmsg_cmd_send(p_rpmsg_data, RL_BLOCK);
    }
}

#endif

/* Copyright 2012-17 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: AMD
 *
 */
#ifndef __DC_DWBC_DCN10_H__
#define __DC_DWBC_DCN10_H__

#if defined(CONFIG_DRM_AMD_DC_DCN)

/* DCN */
#define BASE_INNER(seg) \
	DCE_BASE__INST0_SEG ## seg

#define BASE(seg) \
	BASE_INNER(seg)

#define SR(reg_name)\
		.reg_name = BASE(mm ## reg_name ## _BASE_IDX) +  \
					mm ## reg_name

#define SRI(reg_name, block, id)\
	.reg_name = BASE(mm ## block ## id ## _ ## reg_name ## _BASE_IDX) + \
					mm ## block ## id ## _ ## reg_name


#define SRII(reg_name, block, id)\
	.reg_name[id] = BASE(mm ## block ## id ## _ ## reg_name ## _BASE_IDX) + \
					mm ## block ## id ## _ ## reg_name

#define SF(reg_name, field_name, post_fix)\
	.field_name = reg_name ## __ ## field_name ## post_fix


#define DWBC_COMMON_REG_LIST_DCN1_0(inst) \
	SRI(WB_ENABLE, CNV, inst),\
	SRI(WB_EC_CONFIG, CNV, inst),\
	SRI(CNV_MODE, CNV, inst),\
	SRI(WB_SOFT_RESET, CNV, inst),\
	SRI(MCIF_WB_BUFMGR_SW_CONTROL, MCIF_WB, inst),\
	SRI(MCIF_WB_BUF_PITCH, MCIF_WB, inst),\
	SRI(MCIF_WB_ARBITRATION_CONTROL, MCIF_WB, inst),\
	SRI(MCIF_WB_SCLK_CHANGE, MCIF_WB, inst),\
	SRI(MCIF_WB_BUF_1_ADDR_Y, MCIF_WB, inst),\
	SRI(MCIF_WB_BUF_1_ADDR_Y_OFFSET, MCIF_WB, inst),\
	SRI(MCIF_WB_BUF_1_ADDR_C, MCIF_WB, inst),\
	SRI(MCIF_WB_BUF_1_ADDR_C_OFFSET, MCIF_WB, inst),\
	SRI(MCIF_WB_BUF_2_ADDR_Y, MCIF_WB, inst),\
	SRI(MCIF_WB_BUF_2_ADDR_Y_OFFSET, MCIF_WB, inst),\
	SRI(MCIF_WB_BUF_2_ADDR_C, MCIF_WB, inst),\
	SRI(MCIF_WB_BUF_2_ADDR_C_OFFSET, MCIF_WB, inst),\
	SRI(MCIF_WB_BUF_3_ADDR_Y, MCIF_WB, inst),\
	SRI(MCIF_WB_BUF_3_ADDR_Y_OFFSET, MCIF_WB, inst),\
	SRI(MCIF_WB_BUF_3_ADDR_C, MCIF_WB, inst),\
	SRI(MCIF_WB_BUF_3_ADDR_C_OFFSET, MCIF_WB, inst),\
	SRI(MCIF_WB_BUF_4_ADDR_Y, MCIF_WB, inst),\
	SRI(MCIF_WB_BUF_4_ADDR_Y_OFFSET, MCIF_WB, inst),\
	SRI(MCIF_WB_BUF_4_ADDR_C, MCIF_WB, inst),\
	SRI(MCIF_WB_BUF_4_ADDR_C_OFFSET, MCIF_WB, inst),\
	SRI(MCIF_WB_BUFMGR_VCE_CONTROL, MCIF_WB, inst),\
	SRI(MCIF_WB_NB_PSTATE_LATENCY_WATERMARK, MCIF_WB, inst),\
	SRI(MCIF_WB_NB_PSTATE_CONTROL, MCIF_WB, inst),\
	SRI(MCIF_WB_WATERMARK, MCIF_WB, inst),\
	SRI(MCIF_WB_WARM_UP_CNTL, MCIF_WB, inst),\
	SRI(MCIF_WB_BUF_LUMA_SIZE, MCIF_WB, inst),\
	SRI(MCIF_WB_BUF_CHROMA_SIZE, MCIF_WB, inst)

#define DWBC_COMMON_MASK_SH_LIST_DCN1_0(mask_sh) \
	SF(CNV0_WB_ENABLE, WB_ENABLE, mask_sh),\
	SF(CNV0_WB_EC_CONFIG, DISPCLK_R_WB_GATE_DIS, mask_sh),\
	SF(CNV0_WB_EC_CONFIG, DISPCLK_G_WB_GATE_DIS, mask_sh),\
	SF(CNV0_WB_EC_CONFIG, DISPCLK_G_WBSCL_GATE_DIS, mask_sh),\
	SF(CNV0_WB_EC_CONFIG, WB_LB_LS_DIS, mask_sh),\
	SF(CNV0_WB_EC_CONFIG, WB_LUT_LS_DIS, mask_sh),\
	SF(CNV0_CNV_MODE, CNV_WINDOW_CROP_EN, mask_sh),\
	SF(CNV0_CNV_MODE, CNV_STEREO_TYPE, mask_sh),\
	SF(CNV0_CNV_MODE, CNV_INTERLACED_MODE, mask_sh),\
	SF(CNV0_CNV_MODE, CNV_EYE_SELECTION, mask_sh),\
	SF(CNV0_CNV_MODE, CNV_STEREO_POLARITY, mask_sh),\
	SF(CNV0_CNV_MODE, CNV_INTERLACED_FIELD_ORDER, mask_sh),\
	SF(CNV0_CNV_MODE, CNV_STEREO_SPLIT, mask_sh),\
	SF(CNV0_CNV_MODE, CNV_NEW_CONTENT, mask_sh),\
	SF(CNV0_CNV_MODE, CNV_FRAME_CAPTURE_EN, mask_sh),\
	SF(CNV0_WB_SOFT_RESET, WB_SOFT_RESET, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUFMGR_SW_CONTROL, MCIF_WB_BUFMGR_ENABLE, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUFMGR_SW_CONTROL, MCIF_WB_BUF_DUALSIZE_REQ, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUFMGR_SW_CONTROL, MCIF_WB_BUFMGR_SW_INT_EN, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUFMGR_SW_CONTROL, MCIF_WB_BUFMGR_SW_INT_ACK, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUFMGR_SW_CONTROL, MCIF_WB_BUFMGR_SW_SLICE_INT_EN, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUFMGR_SW_CONTROL, MCIF_WB_BUFMGR_SW_OVERRUN_INT_EN, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUFMGR_SW_CONTROL, MCIF_WB_BUFMGR_SW_LOCK, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUFMGR_SW_CONTROL, MCIF_WB_P_VMID, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUFMGR_SW_CONTROL, MCIF_WB_BUF_ADDR_FENCE_EN, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUF_PITCH, MCIF_WB_BUF_LUMA_PITCH, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUF_PITCH, MCIF_WB_BUF_CHROMA_PITCH, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_ARBITRATION_CONTROL, MCIF_WB_CLIENT_ARBITRATION_SLICE, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_ARBITRATION_CONTROL, MCIF_WB_TIME_PER_PIXEL, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_SCLK_CHANGE, WM_CHANGE_ACK_FORCE_ON, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_SCLK_CHANGE, MCIF_WB_CLI_WATERMARK_MASK, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUF_1_ADDR_Y, MCIF_WB_BUF_1_ADDR_Y, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUF_1_ADDR_Y_OFFSET, MCIF_WB_BUF_1_ADDR_Y_OFFSET, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUF_1_ADDR_C, MCIF_WB_BUF_1_ADDR_C, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUF_1_ADDR_C_OFFSET, MCIF_WB_BUF_1_ADDR_C_OFFSET, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUF_2_ADDR_Y, MCIF_WB_BUF_2_ADDR_Y, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUF_2_ADDR_Y_OFFSET, MCIF_WB_BUF_2_ADDR_Y_OFFSET, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUF_2_ADDR_C, MCIF_WB_BUF_2_ADDR_C, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUF_2_ADDR_C_OFFSET, MCIF_WB_BUF_2_ADDR_C_OFFSET, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUF_3_ADDR_Y, MCIF_WB_BUF_3_ADDR_Y, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUF_3_ADDR_Y_OFFSET, MCIF_WB_BUF_3_ADDR_Y_OFFSET, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUF_3_ADDR_C, MCIF_WB_BUF_3_ADDR_C, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUF_3_ADDR_C_OFFSET, MCIF_WB_BUF_3_ADDR_C_OFFSET, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUF_4_ADDR_Y, MCIF_WB_BUF_4_ADDR_Y, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUF_4_ADDR_Y_OFFSET, MCIF_WB_BUF_4_ADDR_Y_OFFSET, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUF_4_ADDR_C, MCIF_WB_BUF_4_ADDR_C, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUF_4_ADDR_C_OFFSET, MCIF_WB_BUF_4_ADDR_C_OFFSET, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUFMGR_VCE_CONTROL, MCIF_WB_BUFMGR_VCE_LOCK_IGNORE, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUFMGR_VCE_CONTROL, MCIF_WB_BUFMGR_VCE_INT_EN, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUFMGR_VCE_CONTROL, MCIF_WB_BUFMGR_VCE_INT_ACK, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUFMGR_VCE_CONTROL, MCIF_WB_BUFMGR_VCE_SLICE_INT_EN, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUFMGR_VCE_CONTROL, MCIF_WB_BUFMGR_VCE_LOCK, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUFMGR_VCE_CONTROL, MCIF_WB_BUFMGR_SLICE_SIZE, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_NB_PSTATE_LATENCY_WATERMARK, NB_PSTATE_CHANGE_REFRESH_WATERMARK, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_NB_PSTATE_CONTROL, NB_PSTATE_CHANGE_URGENT_DURING_REQUEST, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_NB_PSTATE_CONTROL, NB_PSTATE_CHANGE_FORCE_ON, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_NB_PSTATE_CONTROL, NB_PSTATE_ALLOW_FOR_URGENT, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_NB_PSTATE_CONTROL, NB_PSTATE_CHANGE_WATERMARK_MASK, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_WATERMARK, MCIF_WB_CLI_WATERMARK, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_WARM_UP_CNTL, MCIF_WB_PITCH_SIZE_WARMUP, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUF_LUMA_SIZE, MCIF_WB_BUF_LUMA_SIZE, mask_sh),\
	SF(MCIF_WB0_MCIF_WB_BUF_CHROMA_SIZE, MCIF_WB_BUF_CHROMA_SIZE, mask_sh)

#define DWBC_REG_FIELD_LIST(type) \
	type WB_ENABLE;\
	type DISPCLK_R_WB_GATE_DIS;\
	type DISPCLK_G_WB_GATE_DIS;\
	type DISPCLK_G_WBSCL_GATE_DIS;\
	type WB_LB_LS_DIS;\
	type WB_LB_SD_DIS;\
	type WB_LUT_LS_DIS;\
	type CNV_WINDOW_CROP_EN;\
	type CNV_STEREO_TYPE;\
	type CNV_INTERLACED_MODE;\
	type CNV_EYE_SELECTION;\
	type CNV_STEREO_POLARITY;\
	type CNV_INTERLACED_FIELD_ORDER;\
	type CNV_STEREO_SPLIT;\
	type CNV_NEW_CONTENT;\
	type CNV_FRAME_CAPTURE_EN;\
	type WB_SOFT_RESET;\
	type MCIF_WB_BUFMGR_ENABLE;\
	type MCIF_WB_BUF_DUALSIZE_REQ;\
	type MCIF_WB_BUFMGR_SW_INT_EN;\
	type MCIF_WB_BUFMGR_SW_INT_ACK;\
	type MCIF_WB_BUFMGR_SW_SLICE_INT_EN;\
	type MCIF_WB_BUFMGR_SW_OVERRUN_INT_EN;\
	type MCIF_WB_BUFMGR_SW_LOCK;\
	type MCIF_WB_P_VMID;\
	type MCIF_WB_BUF_ADDR_FENCE_EN;\
	type MCIF_WB_BUF_LUMA_PITCH;\
	type MCIF_WB_BUF_CHROMA_PITCH;\
	type MCIF_WB_CLIENT_ARBITRATION_SLICE;\
	type MCIF_WB_TIME_PER_PIXEL;\
	type WM_CHANGE_ACK_FORCE_ON;\
	type MCIF_WB_CLI_WATERMARK_MASK;\
	type MCIF_WB_BUF_1_ADDR_Y;\
	type MCIF_WB_BUF_1_ADDR_Y_OFFSET;\
	type MCIF_WB_BUF_1_ADDR_C;\
	type MCIF_WB_BUF_1_ADDR_C_OFFSET;\
	type MCIF_WB_BUF_2_ADDR_Y;\
	type MCIF_WB_BUF_2_ADDR_Y_OFFSET;\
	type MCIF_WB_BUF_2_ADDR_C;\
	type MCIF_WB_BUF_2_ADDR_C_OFFSET;\
	type MCIF_WB_BUF_3_ADDR_Y;\
	type MCIF_WB_BUF_3_ADDR_Y_OFFSET;\
	type MCIF_WB_BUF_3_ADDR_C;\
	type MCIF_WB_BUF_3_ADDR_C_OFFSET;\
	type MCIF_WB_BUF_4_ADDR_Y;\
	type MCIF_WB_BUF_4_ADDR_Y_OFFSET;\
	type MCIF_WB_BUF_4_ADDR_C;\
	type MCIF_WB_BUF_4_ADDR_C_OFFSET;\
	type MCIF_WB_BUFMGR_VCE_LOCK_IGNORE;\
	type MCIF_WB_BUFMGR_VCE_INT_EN;\
	type MCIF_WB_BUFMGR_VCE_INT_ACK;\
	type MCIF_WB_BUFMGR_VCE_SLICE_INT_EN;\
	type MCIF_WB_BUFMGR_VCE_LOCK;\
	type MCIF_WB_BUFMGR_SLICE_SIZE;\
	type NB_PSTATE_CHANGE_REFRESH_WATERMARK;\
	type NB_PSTATE_CHANGE_URGENT_DURING_REQUEST;\
	type NB_PSTATE_CHANGE_FORCE_ON;\
	type NB_PSTATE_ALLOW_FOR_URGENT;\
	type NB_PSTATE_CHANGE_WATERMARK_MASK;\
	type MCIF_WB_CLI_WATERMARK;\
	type MCIF_WB_CLI_CLOCK_GATER_OVERRIDE;\
	type MCIF_WB_PITCH_SIZE_WARMUP;\
	type MCIF_WB_BUF_LUMA_SIZE;\
	type MCIF_WB_BUF_CHROMA_SIZE;\

struct dcn10_dwbc_registers {
	uint32_t WB_ENABLE;
	uint32_t WB_EC_CONFIG;
	uint32_t CNV_MODE;
	uint32_t WB_SOFT_RESET;
	uint32_t MCIF_WB_BUFMGR_SW_CONTROL;
	uint32_t MCIF_WB_BUF_PITCH;
	uint32_t MCIF_WB_ARBITRATION_CONTROL;
	uint32_t MCIF_WB_SCLK_CHANGE;
	uint32_t MCIF_WB_BUF_1_ADDR_Y;
	uint32_t MCIF_WB_BUF_1_ADDR_Y_OFFSET;
	uint32_t MCIF_WB_BUF_1_ADDR_C;
	uint32_t MCIF_WB_BUF_1_ADDR_C_OFFSET;
	uint32_t MCIF_WB_BUF_2_ADDR_Y;
	uint32_t MCIF_WB_BUF_2_ADDR_Y_OFFSET;
	uint32_t MCIF_WB_BUF_2_ADDR_C;
	uint32_t MCIF_WB_BUF_2_ADDR_C_OFFSET;
	uint32_t MCIF_WB_BUF_3_ADDR_Y;
	uint32_t MCIF_WB_BUF_3_ADDR_Y_OFFSET;
	uint32_t MCIF_WB_BUF_3_ADDR_C;
	uint32_t MCIF_WB_BUF_3_ADDR_C_OFFSET;
	uint32_t MCIF_WB_BUF_4_ADDR_Y;
	uint32_t MCIF_WB_BUF_4_ADDR_Y_OFFSET;
	uint32_t MCIF_WB_BUF_4_ADDR_C;
	uint32_t MCIF_WB_BUF_4_ADDR_C_OFFSET;
	uint32_t MCIF_WB_BUFMGR_VCE_CONTROL;
	uint32_t MCIF_WB_NB_PSTATE_LATENCY_WATERMARK;
	uint32_t MCIF_WB_NB_PSTATE_CONTROL;
	uint32_t MCIF_WB_WATERMARK;
	uint32_t MCIF_WB_WARM_UP_CNTL;
	uint32_t MCIF_WB_BUF_LUMA_SIZE;
	uint32_t MCIF_WB_BUF_CHROMA_SIZE;
};
struct dcn10_dwbc_mask {
	DWBC_REG_FIELD_LIST(uint32_t)
};
struct dcn10_dwbc_shift {
	DWBC_REG_FIELD_LIST(uint8_t)
};
struct dcn10_dwbc {
	struct dwbc base;
	const struct dcn10_dwbc_registers *dwbc_regs;
	const struct dcn10_dwbc_shift *dwbc_shift;
	const struct dcn10_dwbc_mask *dwbc_mask;
};

void dcn10_dwbc_construct(struct dcn10_dwbc *dwbc10,
		struct dc_context *ctx,
		const struct dcn10_dwbc_registers *dwbc_regs,
		const struct dcn10_dwbc_shift *dwbc_shift,
		const struct dcn10_dwbc_mask *dwbc_mask,
		int inst);

#endif

#endif
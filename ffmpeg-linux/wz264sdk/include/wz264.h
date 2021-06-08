/*****************************************************************************
 * wz264.h: wz264 public header
 *****************************************************************************
 * Copyright (C) 2003-2018 wz264 project
 *
 * Authors: Laurent Aimar <fenrir@via.ecp.fr>
 *          Loren Merritt <lorenm@u.washington.edu>
 *          Fiona Glaser <fiona@wz264.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *
 * This program is also available under a commercial proprietary license.
 * For more information, contact us at licensing@wz264.com.
 *****************************************************************************/

#ifndef WZ264_WZ264_H
#define WZ264_WZ264_H

#define WZ_CAE 1
#if !defined(WZ_CAE_VMAF)
#define WZ_CAE_VMAF 0
#endif
#if !defined(WZ_CAE_ENANCE_INSIDE)
#define WZ_CAE_ENANCE_INSIDE 0
#endif
#define WZ_CAE_DEFAULT_PARAM 1
#define WZ_CAE_TEMPOLAYER_DEBUG 0
#define WZ_CAE_ADJCBR 0
#if WZ_CAE_ADJCBR
#define WZ_CAE_ADJCBR_DEBUG 1
#endif
#define WZ_CAE_FTWAITFIRST 1
#if WZ_CAE_FTWAITFIRST
#define WZ_CAE_COMBINE_NALS 1
#endif
#define WZ_2PASS_CRF_ADAPT 1
// wz: WILDFIRE is old logic by all MBs reencoding
#define WZ_SYNTAX_LOG 0  // To log H264 syntax element

#define WZ_CRF_ADAPT_FRAME_REENCODE 1

#ifndef ENABLE_WZ_LOG
#define ENABLE_WZ_LOG 1
#endif
#ifndef ENABLE_WZ_2PASS
#define ENABLE_WZ_2PASS 1
#endif

// --deadzone 11,21,21,11,21,21 --decimate-th 4,6,7,4,6,7 to restore to original values
#define WZ_DCT_DECIMATE 1
#define WZ_QUANT_DEADZONE 1

/*     WZ264 tuning types: Multiple tunings can be combined by "|"
       e.g., WZ264_TUNE_SSIM | WZ264_TUNE_SCREEN                */
#define WZ264_TUNE_FILM 0x0001
#define WZ264_TUNE_ANIMATION 0x0002
#define WZ264_TUNE_GRAIN 0x0004
#define WZ264_TUNE_STILLIMAGE 0x0010
#define WZ264_TUNE_PSNR 0x0020
#define WZ264_TUNE_SSIM 0x0040
#define WZ264_TUNE_FASTDECODE 0x0080
#define WZ264_TUNE_SCREEN 0x0100
#define WZ264_TUNE_ARM 0x0200
#define WZ264_TUNE_ZEROLANT 0x0400

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

#ifndef WZ264_CHROMA_FORMAT
#include "wz264_config.h"
#endif

/* Application developers planning to link against a shared library version of
 * libwz264 from a Microsoft Visual Studio or similar development environment
 * will need to define WZ264_API_IMPORTS before including this header.
 * This clause does not apply to MinGW, similar development environments, or non
 * Windows platforms. */
#ifdef WZ264_API_IMPORTS
#define WZ264_API __declspec(dllimport)
#else
#define WZ264_API
#endif

/* wz264_t:
 *      opaque handler for encoder */
typedef struct wz264_t wz264_t;

/****************************************************************************
 * NAL structure and functions
 ****************************************************************************/

#if !WZ264_SDK
enum nal_unit_type_e {
  NAL_UNKNOWN = 0,
  NAL_SLICE = 1,
  NAL_SLICE_DPA = 2,
  NAL_SLICE_DPB = 3,
  NAL_SLICE_DPC = 4,
  NAL_SLICE_IDR = 5, /* ref_idc != 0 */
  NAL_SEI = 6,       /* ref_idc == 0 */
  NAL_SPS = 7,
  NAL_PPS = 8,
  NAL_AUD = 9,
  NAL_FILLER = 12,
  /* ref_idc == 0 for 6,9,10,11,12 */
};
enum nal_priority_e {
  NAL_PRIORITY_DISPOSABLE = 0,
  NAL_PRIORITY_LOW = 1,
  NAL_PRIORITY_HIGH = 2,
  NAL_PRIORITY_HIGHEST = 3,
};

/* The data within the payload is already NAL-encapsulated; the ref_idc and type
 * are merely in the struct for easy access by the calling application.
 * All data returned in an wz264_nal_t, including the data in p_payload, is no longer
 * valid after the next call to wz264_encoder_encode.  Thus it must be used or copied
 * before calling wz264_encoder_encode or wz264_encoder_headers again. */
typedef struct wz264_nal_t {
  int i_ref_idc; /* nal_priority_e */
  int i_type;    /* nal_unit_type_e */
  int b_long_startcode;
  int i_first_mb; /* If this NAL is a slice, the index of the first MB in the slice. */
  int i_last_mb;  /* If this NAL is a slice, the index of the last MB in the slice. */

  /* Size of payload (including any padding) in bytes. */
  int i_payload;
  /* If param->b_annexb is set, Annex-B bytestream with startcode.
   * Otherwise, startcode is replaced with a 4-byte size.
   * This size is the size used in mp4/similar muxing; it is equal to i_payload-4 */
  uint8_t *p_payload;

  /* Size of padding in bytes. */
  int i_padding;
} wz264_nal_t;
#else
#include "wz264/wz264enc.h"
#endif  // WZ264_SDK

/****************************************************************************
 * Encoder parameters
 ****************************************************************************/
/* CPU flags */

/* x86 */
#define WZ264_CPU_MMX (1 << 0)
#define WZ264_CPU_MMX2 (1 << 1) /* MMX2 aka MMXEXT aka ISSE */
#define WZ264_CPU_MMXEXT WZ264_CPU_MMX2
#define WZ264_CPU_SSE (1 << 2)
#define WZ264_CPU_SSE2 (1 << 3)
#define WZ264_CPU_LZCNT (1 << 4)
#define WZ264_CPU_SSE3 (1 << 5)
#define WZ264_CPU_SSSE3 (1 << 6)
#define WZ264_CPU_SSE4 (1 << 7)  /* SSE4.1 */
#define WZ264_CPU_SSE42 (1 << 8) /* SSE4.2 */
#define WZ264_CPU_AVX (1 << 9)   /* Requires OS support even if YMM registers aren't used */
#define WZ264_CPU_XOP (1 << 10)  /* AMD XOP */
#define WZ264_CPU_FMA4 (1 << 11) /* AMD FMA4 */
#define WZ264_CPU_FMA3 (1 << 12)
#define WZ264_CPU_BMI1 (1 << 13)
#define WZ264_CPU_BMI2 (1 << 14)
#define WZ264_CPU_AVX2 (1 << 15)
#define WZ264_CPU_AVX512 (1 << 16) /* AVX-512 {F, CD, BW, DQ, VL}, requires OS support */
/* x86 modifiers */
#define WZ264_CPU_CACHELINE_32 \
  (1 << 17) /* avoid memory loads that span the border between two cachelines */
#define WZ264_CPU_CACHELINE_64 (1 << 18) /* 32/64 is the size of a cacheline in bytes */
#define WZ264_CPU_SSE2_IS_SLOW (1 << 19) /* avoid most SSE2 functions on Athlon64 */
#define WZ264_CPU_SSE2_IS_FAST (1 << 20) /* a few functions are only faster on Core2 and Phenom */
#define WZ264_CPU_SLOW_SHUFFLE \
  (1 << 21) /* The Conroe has a slow shuffle unit (relative to overall SSE performance) */
#define WZ264_CPU_STACK_MOD4 (1 << 22) /* if stack is only mod4 and not mod16 */
#define WZ264_CPU_SLOW_ATOM                                                                          \
  (1 << 23)                              /* The Atom is terrible: slow SSE unaligned loads, slow     \
                                          * SIMD multiplies, slow SIMD variable shifts, slow pshufb, \
                                          * cacheline split penalties -- gather everything here that \
                                          * isn't shared by other CPUs to avoid making half a dozen  \
                                          * new SLOW flags. */
#define WZ264_CPU_SLOW_PSHUFB (1 << 24)  /* such as on the Intel Atom */
#define WZ264_CPU_SLOW_PALIGNR (1 << 25) /* such as on the AMD Bobcat */

/* PowerPC */
#define WZ264_CPU_ALTIVEC 0x0000001

/* ARM and AArch64 */
#define WZ264_CPU_ARMV6 0x0000001
#define WZ264_CPU_NEON 0x0000002 /* ARM NEON */
#define WZ264_CPU_FAST_NEON_MRC \
  0x0000004 /* Transfer from NEON to ARM register is fast (Cortex-A9) */
#define WZ264_CPU_ARMV8 0x0000008

/* MIPS */
#define WZ264_CPU_MSA 0x0000001 /* MIPS MSA */

/* Analyse flags */
#define WZ264_ANALYSE_I4x4 0x0001      /* Analyse i4x4 */
#define WZ264_ANALYSE_I8x8 0x0002      /* Analyse i8x8 (requires 8x8 transform) */
#define WZ264_ANALYSE_PSUB16x16 0x0010 /* Analyse p16x8, p8x16 and p8x8 */
#define WZ264_ANALYSE_PSUB8x8 0x0020   /* Analyse p8x4, p4x8, p4x4 */
#define WZ264_ANALYSE_BSUB16x16 0x0100 /* Analyse b16x8, b8x16 and b8x8 */
#define WZ264_DIRECT_PRED_NONE 0
#define WZ264_DIRECT_PRED_SPATIAL 1
#define WZ264_DIRECT_PRED_TEMPORAL 2
#define WZ264_DIRECT_PRED_AUTO 3
#define WZ264_ME_DIA 0
#define WZ264_ME_HEX 1
#define WZ264_ME_UMH 2
#define WZ264_ME_ESA 3
#define WZ264_ME_TESA 4
#define WZ264_ME_NSTEP_DIA 5
#define WZ264_ME_NSTEP 6
#define WZ264_ME_NSTEP_8PT 7
#define WZ264_ME_CLAMPED_NSTEP_DIA 8
// Search maximum 8-points in the radius grid around center,
// up to 11 search stages. First stage consists of 8 search points
// and the rest with 6 search points each in hex shape.
#define WZ264_ME_HEX2 9
// Search maximum 8-points in the radius grid around center,
// up to 11 search stages. First stage consists of 4 search
// points and the rest with 8 search points each.
#define WZ264_ME_BIGDIA 10
// Search 8-points in the square grid around center, up to 11 search stages.
#define WZ264_ME_SQUARE 11
// HEX search with up to 2 stages.
#define WZ264_ME_FAST_HEX 12
// BIGDIA search with up to 2 stages.
#define WZ264_ME_FAST_DIAMOND 13
// BIGDIA search with up to 3 stages.
#define WZ264_ME_FAST_BIGDIA 14
#define WZ264_ME_LAST WZ264_ME_FAST_BIGDIA
#define NUM_SEARCH_METHODS = WZ264_ME_LAST + 1,
#define NUM_DISTINCT_SEARCH_METHODS (WZ264_ME_SQUARE + 1)

#define WZ264_ME_FURTHER_NONE 0
#define WZ264_ME_FURTHER_CROSS 1
#define WZ264_ME_FURTHER_CROSS_FME 2
#define WZ264_CQM_FLAT 0
#define WZ264_CQM_JVT 1
#if !WZ264_SDK
#define WZ264_RC_CQP 0
#define WZ264_RC_CRF 1
#define WZ264_RC_ABR 2
#endif
#define WZ264_QP_AUTO 0
#define WZ264_AQ_NONE 0
#define WZ264_AQ_VARIANCE 1
#define WZ264_AQ_AUTOVARIANCE 2
#define WZ264_AQ_AUTOVARIANCE_BIASED 3
#define WZ264_AQ_FINE_VARIANCE 4
#define WZ264_AQ_FINE_AUTOVARIANCE 5
#define WZ264_AQ_CDEF_VAR 6
#define WZ264_AQ_MAX_VALUE WZ264_AQ_CDEF_VAR
#define WZ264_B_ADAPT_NONE 0
#define WZ264_B_ADAPT_FAST 1
#define WZ264_B_ADAPT_TRELLIS 2
#define WZ264_WEIGHTP_NONE 0
#define WZ264_WEIGHTP_SIMPLE 1
#define WZ264_WEIGHTP_SMART 2
#define WZ264_B_PYRAMID_NONE 0
#define WZ264_B_PYRAMID_STRICT 1
#define WZ264_B_PYRAMID_NORMAL 2
#define WZ264_KEYINT_MIN_AUTO 0
#define WZ264_KEYINT_MAX_INFINITE (1 << 30)
#define WZ264_KEYINT_MAX_AUTO 0

static const char *const wz264_direct_pred_names[] = { "none", "spatial", "temporal", "auto", 0 };
static const char *const wz264_motion_est_names[] = { "dia",      "hex",       "umh",
                                                      "esa",      "tesa",      "nstep-dia",
                                                      "nstep",    "nstep-8pt", "nstep-dia-clamped",
                                                      "hex2",     "bigdia",    "square",
                                                      "fast-hex", "fast-dia",  "fast-bigdia",
                                                      0 };
static const char *const wz264_motion_further_names[] = { "none", "cross", 0 };
static const char *const wz264_b_pyramid_names[] = { "none", "strict", "normal", 0 };
static const char *const wz264_overscan_names[] = { "undef", "show", "crop", 0 };
static const char *const wz264_vidformat_names[] = { "component", "pal",   "ntsc", "secam",
                                                     "mac",       "undef", 0 };
static const char *const wz264_fullrange_names[] = { "off", "on", 0 };
static const char *const wz264_colorprim_names[] = {
  "",          "bt709", "undef",  "",         "bt470m",   "bt470bg",  "smpte170m",
  "smpte240m", "film",  "bt2020", "smpte428", "smpte431", "smpte432", 0
};
static const char *const wz264_transfer_names[] = {
  "",          "bt709",     "undef",     "",         "bt470m",       "bt470bg", "smpte170m",
  "smpte240m", "linear",    "log100",    "log316",   "iec61966-2-4", "bt1361e", "iec61966-2-1",
  "bt2020-10", "bt2020-12", "smpte2084", "smpte428", "arib-std-b67", 0
};
static const char *const wz264_colmatrix_names[] = {
  "GBR",       "bt709", "undef",    "",        "fcc",       "bt470bg",           "smpte170m",
  "smpte240m", "YCgCo", "bt2020nc", "bt2020c", "smpte2085", "chroma-derived-nc", "chroma-derived-c",
  "ICtCp",     0
};
static const char *const wz264_nal_hrd_names[] = { "none", "vbr", "cbr", 0 };

/* Colorspace type */
#define WZ264_CSP_MASK 0x00ff       /* */
#define WZ264_CSP_VFLIP 0x1000      /* the csp is vertically flipped */
#define WZ264_CSP_HIGH_DEPTH 0x2000 /* the csp has a depth of 16 bits per pixel component */

#if !WZ264_SDK
#define WZ264_CSP_NONE 0x0000 /* Invalid mode     */
#define WZ264_CSP_I420 0x0001 /* yuv 4:2:0 planar */
#define WZ264_CSP_YV12 0x0002 /* yvu 4:2:0 planar */
#define WZ264_CSP_NV12 0x0003 /* yuv 4:2:0, with one y plane and one packed u+v */
#define WZ264_CSP_NV21 0x0004 /* yuv 4:2:0, with one y plane and one packed v+u */
#define WZ264_CSP_I422 0x0005 /* yuv 4:2:2 planar */
#define WZ264_CSP_YV16 0x0006 /* yvu 4:2:2 planar */
#define WZ264_CSP_NV16 0x0007 /* yuv 4:2:2, with one y plane and one packed u+v */
#define WZ264_CSP_YUYV 0x0008 /* yuyv 4:2:2 packed */
#define WZ264_CSP_UYVY 0x0009 /* uyvy 4:2:2 packed */
#define WZ264_CSP_V210 0x000a /* 10-bit yuv 4:2:2 packed in 32 */
#define WZ264_CSP_I444 0x000b /* yuv 4:4:4 planar */
#define WZ264_CSP_YV24 0x000c /* yvu 4:4:4 planar */
#define WZ264_CSP_BGR 0x000d  /* packed bgr 24bits */
#define WZ264_CSP_BGRA 0x000e /* packed bgr 32bits */
#define WZ264_CSP_RGB 0x000f  /* packed rgb 24bits */
#define WZ264_CSP_MAX 0x0010  /* end of list */

/* Slice type */
#define WZ264_TYPE_AUTO 0x0000 /* Let wz264 choose the right type */
#define WZ264_TYPE_IDR 0x0001
#define WZ264_TYPE_I 0x0002
#define WZ264_TYPE_P 0x0003
#define WZ264_TYPE_BREF 0x0004 /* Non-disposable B-frame */
#define WZ264_TYPE_B 0x0005
#define WZ264_TYPE_KEYFRAME 0x0006 /* IDR or I depending on b_open_gop option */
#endif

#define IS_WZ264_TYPE_I(x) \
  ((x) == WZ264_TYPE_I || (x) == WZ264_TYPE_IDR || (x) == WZ264_TYPE_KEYFRAME)
#define IS_WZ264_TYPE_B(x) ((x) == WZ264_TYPE_B || (x) == WZ264_TYPE_BREF)

#if !WZ264_SDK
/* Log level */
#define WZ264_LOG_NONE (-1)
#define WZ264_LOG_ERROR 0
#define WZ264_LOG_WARNING 1
#define WZ264_LOG_INFO 2
#define WZ264_LOG_DEBUG 3
#endif

/* Threading */
#define WZ264_THREADS_AUTO 0 /* Automatically select optimal number of threads */
#define WZ264_SYNC_LOOKAHEAD_AUTO \
  (-1) /* Automatically select optimal lookahead thread buffer size */

/* HRD */
#define WZ264_NAL_HRD_NONE 0
#define WZ264_NAL_HRD_VBR 1
#define WZ264_NAL_HRD_CBR 2

typedef struct wz264_param_t {
  /* CPU flags */
  unsigned int cpu;
  int i_threads;           /* encode multiple frames in parallel */
  int i_lookahead_threads; /* multiple threads for lookahead analysis */
  int b_sliced_threads;    /* Whether to use slice-based threading. */
  int b_deterministic;     /* whether to allow non-deterministic optimizations when threaded */
  int b_cpu_independent; /* force canonical behavior rather than cpu-dependent optimal algorithms */
  int i_sync_lookahead;  /* threaded lookahead buffer */

  /* Video Properties */
  int i_width;
  int i_height;
  int i_csp; /* CSP of encoded bitstream */
  int i_bitdepth;
  int i_level_idc;
  int i_frame_total; /* number of frames to encode if known, else 0 */

  /* NAL HRD
   * Uses Buffering and Picture Timing SEIs to signal HRD
   * The HRD in H.264 was not designed with VFR in mind.
   * It is therefore not recommendeded to use NAL HRD with VFR.
   * Furthermore, reconfiguring the VBV (via wz264_encoder_reconfig)
   * will currently generate invalid HRD. */
  int i_nal_hrd;

  int i_tune;
  int i_preset;
#ifdef WZ264_SDK
  wz264_vui_t vui;
#else
  struct {
    /* they will be reduced to be 0 < x <= 65535 and prime */
    int i_sar_height;
    int i_sar_width;

    int i_overscan; /* 0=undef, 1=no overscan, 2=overscan */

    /* see h264 annex E for the values of the following */
    int i_vidformat;
    int b_fullrange;
    int i_colorprim;
    int i_transfer;
    int i_colmatrix;
    int i_chroma_loc; /* both top & bottom */
  } vui;
#endif

  /* Bitstream parameters */
  int i_frame_reference; /* Maximum number of reference frames */
  int i_dpb_size;   /* Force a DPB size larger than that implied by B-frames and reference frames.
                     * Useful in combination with interactive error resilience. */
  int i_keyint_max; /* Force an IDR keyframe at this interval */
  int i_keyint_min; /* Scenecuts closer together than this are coded as I, not IDR. */
  int i_scenecut_threshold; /* how aggressively to insert extra I frames */
  int i_intra_refresh; /* Whether or not to use periodic intra refresh instead of IDR frames. */

  int i_bframe; /* how many b-frame between 2 references pictures */
  int i_bframe_adaptive;
  int i_bframe_bias;
  int
    i_bframe_pyramid; /* Keep some B-frames as references: 0=off, 1=strict hierarchical, 2=normal */
  int b_open_gop;
  int b_bluray_compat;

  int b_deblocking_filter;
  int i_deblocking_filter_alphac0; /* [-6, 6] -6 light filter, 6 strong */
  int i_deblocking_filter_beta;    /* [-6, 6]  idem */

  int b_cabac;
  int i_cabac_init_idc;

  int b_constrained_intra;
  int b_interlaced;
  int b_tff;
  /* Fake Interlaced.
   *
   * Used only when b_interlaced=0. Setting this flag makes it possible to flag the stream as PAFF
   * interlaced yet encode all frames progessively. It is useful for encoding 25p and 30p Blu-Ray
   * streams.
   */
  int b_fake_interlaced;

  int i_cqm_preset;
  /* Log */
  void (*pf_log)(void *, int i_level, const char *psz, va_list);
  void *p_log_private;
  int i_log_level;
  int b_full_recon;   /* fully reconstruct frames, even when not necessary for encoding.  Implied by
                         psz_dump_yuv */
  char *psz_dump_yuv; /* filename (in UTF-8) for reconstructed frames */

  /* Encoder analyser parameters */
  struct {
    unsigned int intra; /* intra partitions */
    unsigned int inter; /* inter partitions */

    int b_transform_8x8;
    int i_weighted_pred;   /* weighting for P-frames */
    int b_weighted_bipred; /* implicit weighting for B-frames */
    int i_direct_mv_pred;  /* spatial vs temporal mv prediction */
    int i_chroma_qp_offset;

    int i_me_method;      /* motion estimation algorithm to use (WZ264_ME_*) */
    int i_me_range;       /* integer pixel motion estimation search range (from predicted mv) */
    int i_me_further;     /* further search (cross me etc.) */
    int i_rough_cross_me; /* rough cross me to use, based on preset */
    int i_mv_range;       /* maximum length of a mv (in pixels). -1 = auto, based on level */
    int
      i_mv_range_thread; /* minimum space between threads. -1 = auto, based on number of threads. */
    int i_subpel_refine; /* subpixel motion estimation quality */
    int b_chroma_me;     /* chroma ME for subpel and mode decision in P-frames */
    int b_mixed_references;     /* allow each mb partition to have its own reference number */
    int i_trellis;              /* trellis RD quantization */
    float f_fastskip_thresh_p;  // earlyskip for P
    float f_tryskip_thresh_p;   // fastskip for P, if tryskip >= 8.0, use prob
    float f_pskip_uvratio;      // skip for P thresh, ratio for uv
    float f_pskip_dcratio;
    float f_pskip_mid_ratio;  // protect middle QP range
    int b_fast_decide_pskip;
    int b_dct_decimate; /* transform coefficient thresholding on P-frames */
#if WZ_DCT_DECIMATE
    int i_decimate_th
      [6];  // Bframe_luma_8x8,Bframe_luma_16x16,Bframe_chrom,Pframe_luma_8x8,Pframe_luma_16x16,Pframe_chrom,
#endif
    int i_noise_reduction; /* adaptive pseudo-deadzone */
    float f_psy_rd;        /* Psy RD strength */
    float f_psy_rd_roi;
    float f_psy_trellis; /* Psy trellis strength */
    int b_psy;           /* Toggle all psy optimizations */
    int i_mbrd;

    /* the deadzone size that will be used in luma quantization */
#if !WZ_QUANT_DEADZONE
    int i_luma_deadzone[2]; /* {inter, intra} */
#else
    int i_deadzone[6]; /* {luma_I,Luma_P,Luma_B,Chroma_I,Chroma_P,Chroma_B} */
#endif

    int i_subme_reduce; /* Reduce subme iterations */

    int b_psnr; /* compute and print PSNR stats */
    int b_ssim; /* compute and print SSIM stats */
  } analyse;

  /* Rate control parameters */
  struct {
    int i_rc_method;    /* WZ264_RC_* */
    float f_short_load; /* short loading at start, indicate crf offset */

    int i_qp_constant;  /* 0=lossless */
    int i_qp_min;       /* min allowed QP value */
    int i_qp_max;       /* max allowed QP value */
    int i_qp_step;      /* max QP step between frames */
    int i_large_qp_nr;  // QP > 51 use NR

    int i_bitrate;
    float f_rf_constant;     /* 1pass VBR, nominal QP */
    float f_rf_constant_max; /* In CRF mode, maximum CRF as caused by VBV */
    float f_rate_tolerance;
    int i_vbv_max_bitrate;
    int i_vbv_buffer_size;
    float f_vbv_buffer_init; /* <=1: fraction of buffer_size. >1: kbit */
    float f_ip_factor;
    float f_pb_factor;
    /* VBV filler: force CBR VBV and use filler bytes to ensure hard-CBR.
     * Implied by NAL-HRD CBR. */
    int b_filler;

    int i_aq_mode; /* psy adaptive QP. (WZ264_AQ_*) */
    float f_aq_strength;

    float f_iboost;
    float f_ifirst_inc;   // only work for zerolatency first I
    float f_key_penalty;  // keyframe penalty for iboost
    float f_aq_smooth;    // aq strength used for smooth area
    float f_aq_balance;
    float f_aq_pow;
    float f_aq_weight;
    float f_rc_qcomp;
    int i_rc_rf_by_fps;
    float f_center_alpha;
    float f_center_beta;

    int b_fill_simple_cplx; /* fill simple cplx to base cplx in RC */
    int b_mb_tree;          /* Macroblock-tree ratecontrol. */
    int i_lookahead;
    int b_need_cost;
    float f_fdiff_pow;  // work with b_need_cost == 0
    /* 2pass */
    int b_stat_write;   /* Enable stat writing in psz_stat_out */
    char *psz_stat_out; /* output filename (in UTF-8) of the 2pass stats file */
    int b_stat_read;    /* Read stat from psz_stat_in and use it */
    char *psz_stat_in;  /* input filename (in UTF-8) of the 2pass stats file */

    /* 2pass params (same as ffmpeg ones) */
    float f_qcompress;       /* 0.0 => cbr, 1.0 => constant qp */
    float f_qblur;           /* temporally blur quants */
    float f_complexity_blur; /* temporally blur complexity */

    /* golden frame boost */
    float f_golden_frame_scale;
    int i_golden_frame_interval;

    /* cyclic refresh */
    float f_cyclic_refresh_scale;
    int i_cyclic_refresh_interval;
    int i_cyclic_refresh_period;
  } rc;

  /* Cropping Rectangle parameters: added to those implicitly defined by
     non-mod16 video resolutions. */
  struct {
    unsigned int i_left;
    unsigned int i_top;
    unsigned int i_right;
    unsigned int i_bottom;
  } crop_rect;

  /* frame packing arrangement flag */
  int i_frame_packing;

  /* alternative transfer SEI */
  int i_alternative_transfer;

  /* Muxing parameters */
  int b_aud;            /* generate access unit delimiters */
  int b_repeat_headers; /* put SPS/PPS before each keyframe */
  int b_annexb;         /* if set, place start codes (4 bytes) before NAL units,
                         * otherwise place size (4 bytes) before NAL units. */
  int i_sps_id;         /* SPS and PPS id number */
  int b_vfr_input;      /* VFR input.  If 1, use timebase and timestamps for ratecontrol purposes.
                         * If 0, use fps only. */
  int b_pulldown;       /* use explicity set timebase for CFR */
  uint32_t i_fps_num;
  uint32_t i_fps_den;
  uint32_t i_timebase_num; /* Timebase numerator */
  uint32_t i_timebase_den; /* Timebase denominator */

  /* Pulldown:
   * The correct pic_struct must be passed with each input frame.
   * The input timebase should be the timebase corresponding to the output framerate. This should be
   * constant. e.g. for 3:2 pulldown timebase should be 1001/30000 The PTS passed with each frame
   * must be the PTS of the frame after pulldown is applied. Frame doubling and tripling require
   * b_vfr_input set to zero (see H.264 Table D-1)
   *
   * Pulldown changes are not clearly defined in H.264. Therefore, it is the calling app's
   * responsibility to manage this.
   */

  int b_pic_struct;

  /* Don't optimize header parameters based on video content, e.g. ensure that splitting an input
   * video, compressing each part, and stitching them back together will result in identical
   * SPS/PPS. This is necessary for stitching with container formats that don't allow multiple
   * SPS/PPS. */
  int b_stitchable;

  /* Slicing parameters */
  int i_slice_max_size;       /* Max size per slice in bytes; includes estimated NAL overhead. */
  int i_slice_max_mbs;        /* Max number of MBs per slice; overrides i_slice_count. */
  int i_slice_min_mbs;        /* Min number of MBs per slice */
  int i_slice_count;          /* Number of slices per frame: forces rectangular slices. */
  int i_slice_count_max;      /* Absolute cap on slices per frame; stops applying slice-max-size
                               * and slice-max-mbs if this is reached. */
  float f_jnd_csf_strength;   // strength of qp adjustment based on CSF
  float f_jnd_dark_strength;  // strength of qp adjustment based on dark degree
  int i_cae_debug;            // cae debug log
  int i_cae_max_kbps;         // long term max bitrate limits for crf
  int b_show_mediainfo;
  int b_high_res;         // enable for firework, when resolution is > 1080p, crf = setting + 1.
  int i_enable_crf_adapt; /* enable crf adatp, 1: vmaf calc for cae and crf adapt by vmaf
                           * 2: crf adapt according to blockiness strength. */

  char wz_ai_filter_string[1024];
  int i_visual_opt;  // 0, 1, 2; 1: sal-diff etc.; 2: + inside enhance
  float f_roi_dqp_ratio;
  float f_roi_lambda_factor;
  float f_pyramid_lambda[3];
  int i_sal_diff_max;
  float f_saldiff_minratio;
  float f_saldiff_maxratio;
  float f_cae_quality_level;  //  higher level means higher quality, -1: auto
  float crf_adapt_range;
#if WZ_CRF_ADAPT_FRAME_REENCODE
  float f_crf_adapt_range_up;
  float f_crf_adapt_range_down;
  float f_local_motion_weight;
  float f_smooth_weight;
  float f_blockiness_tolerance;
  int b_quality_psnr;
  int i_ssd_threshold;
  float f_quality_psnr_up;
  float f_quality_psnr_down;
  int b_quality_vmaf;
  float f_var_threshold;
  float f_quality_vmaf_up;
  float f_quality_vmaf_down;
#endif
  int i_cae_sharptype;
  int i_cae_smoothtype;
  int i_cae_autoenh;
  int i_cae_ymaxdiff;
  float f_cae_yenh_level;
  float f_cae_uvenh_level;
  float f_cae_ydenoise_level;
  float f_cae_uvdenoise_level;
  float f_cae_ydetail_level;
  float f_cae_yenhmin_level;
  float f_cae_aenh_strength;
  int iblockpb;           /* block P or B frame encoding thread(s) while
                           * it's referenced I is encoding, used only when
                           * i_thread_frames > 1 */
  int i_tempo_scalable;   /* enable temporal scalability up to 4 layers video
                           * sequence, nal_ref_idc 3 to 0 means highest to lowest
                           * layer, combining with slice's nal_type can solely
                           * identify target frame, used with some constrains.
                           * 0 - bypass, 1: 2 layers, 2: 3 layers, 3: 4 layers */
  int i_nal_ref_idc;      /* keep record of nal_ref_idc of current NAL Header */
  int i_tempolayer_frame; /* keep record of layer pattern cycle */
#if WZ_CAE_TEMPOLAYER_DEBUG
  int i_tempolayer_drop_frame; /* for test purpose only
                                * 1 - drop all i_nal_ref_idc <= 0 frames
                                * 2 - drop all i_nal_ref_idc <= 1 frames
                                * 3 - drop all i_nal_ref_idc <= 2 frames */
#endif
#if WZ_CAE_ADJCBR
  int adjcbr; /* adjustable bitrate anytime. Should be used with CBR
               * settings(means --bitrate/--vbv-maxrate/--vbv-bufsize
               * are set to same value). Encoder will change these
               * values when necessary when adjcbr = 1. */
#endif
#if WZ_CAE_FTWAITFIRST
  int ftwaitfirst; /* For frame-base threads, ftwaitfirst = 1 means
                    * at begin of encoder waiting for 'thread_current' to be
                    * empty and use 'thread_current' for current frame.
                    */
#endif
#if WZ_SYNTAX_LOG
  FILE *syn_log_file;
#endif
  int i_content_type;           // all 0 - off
                                // bit 0 - 0/1 cross off/on
                                // bit 1 - 0/1 P8x8 FME off/on
                                // bit 2 - 0/1 P16x16 FME off/on
                                // bit 3 - 0/1 FME on original/reconstructed reference
  double f_cross_me_penal_hor;  // penalty on cross me horizontal
  double f_cross_me_penal_ver;  // penalty on cross me horizontal
#if WZ_2PASS_CRF_ADAPT
  // Bitrate oriented 2pass crf adaptation. Set >0 to enable.
  // Must be used with cae-maxrate, which provides the target bitrate for crf prediction.
  // The input frame should be down scaled to 1/8 of the original video by the caller.
  // The predicted crf will be printed to log.
  int i_crf_adapt_2pass;
#endif
  int i_log2_max_frame_num;  // [4, 16],specifies the value of the variable MaxFrameNum that is used
                             // in frame_num

  /* Optional callback for freeing this wz264_param_t when it is done being used.
   * Only used when the wz264_param_t sits in memory for an indefinite period of time,
   * i.e. when an wz264_param_t is passed to wz264_t in an wz264_picture_t or in zones.
   * Not used when wz264_encoder_reconfig is called directly. */
  void (*param_free)(void *);

  /* Optional low-level callback for low-latency encoding.  Called for each output NAL unit
   * immediately after the NAL unit is finished encoding.  This allows the calling application
   * to begin processing video data (e.g. by sending packets over a network) before the frame
   * is done encoding.
   *
   * This callback MUST do the following in order to work correctly:
   * 1) Have available an output buffer of at least size nal->i_payload*3/2 + 5 + 64.
   * 2) Call wz264_nal_encode( h, dst, nal ), where dst is the output buffer.
   * After these steps, the content of nal is valid and can be used in the same way as if
   * the NAL unit were output by wz264_encoder_encode.
   *
   * This does not need to be synchronous with the encoding process: the data pointed to
   * by nal (both before and after wz264_nal_encode) will remain valid until the next
   * wz264_encoder_encode call.  The callback must be re-entrant.
   *
   * This callback does not work with frame-based threads; threads must be disabled
   * or sliced-threads enabled.  This callback also does not work as one would expect
   * with HRD -- since the buffering period SEI cannot be calculated until the frame
   * is finished encoding, it will not be sent via this callback.
   *
   * Note also that the NALs are not necessarily returned in order when sliced threads is
   * enabled.  Accordingly, the variable i_first_mb and i_last_mb are available in
   * wz264_nal_t to help the calling application reorder the slices if necessary.
   *
   * When this callback is enabled, wz264_encoder_encode does not return valid NALs;
   * the calling application is expected to acquire all output NALs through the callback.
   *
   * It is generally sensible to combine this callback with a use of slice-max-mbs or
   * slice-max-size.
   *
   * The opaque pointer is the opaque pointer from the input frame associated with this
   * NAL unit. This helps distinguish between nalu_process calls from different sources,
   * e.g. if doing multiple encodes in one process.
   */
  void (*nalu_process)(wz264_t *h, wz264_nal_t *nal, void *opaque);
} wz264_param_t;

void wz264_nal_encode(wz264_t *h, uint8_t *dst, wz264_nal_t *nal);

/****************************************************************************
 * H.264 level restriction information
 ****************************************************************************/

typedef struct wz264_level_t {
  uint8_t level_idc;
  uint32_t mbps;       /* max macroblock processing rate (macroblocks/sec) */
  uint32_t frame_size; /* max frame size (macroblocks) */
  uint32_t dpb;        /* max decoded picture buffer (mbs) */
  uint32_t bitrate;    /* max bitrate (kbit/sec) */
  uint32_t cpb;        /* max vbv buffer (kbit) */
  uint16_t mv_range;   /* max vertical mv component range (pixels) */
  uint8_t mvs_per_2mb; /* max mvs per 2 consecutive mbs. */
  uint8_t slice_rate;  /* ?? */
  uint8_t mincr;       /* min compression ratio */
  uint8_t bipred8x8;   /* limit bipred to >=8x8 */
  uint8_t direct8x8;   /* limit b_direct to >=8x8 */
  uint8_t frame_only;  /* forbid interlacing */
} wz264_level_t;

/* all of the levels defined in the standard, terminated by .level_idc=0 */
WZ264_API extern const wz264_level_t wz264_levels[];

/****************************************************************************
 * Basic parameter handling functions
 ****************************************************************************/

/* wz264_param_default:
 *      fill wz264_param_t with default values and do CPU detection */
void wz264_param_default(wz264_param_t *);

/* wz264_param_parse:
 *  set one parameter by name.
 *  returns 0 on success, or returns one of the following errors.
 *  note: BAD_VALUE occurs only if it can't even parse the value,
 *  numerical range is not checked until wz264_encoder_open() or
 *  wz264_encoder_reconfig().
 *  value=NULL means "true" for boolean options, but is a BAD_VALUE for non-booleans. */
#if !WZ264_SDK
#define WZ264_PARAM_BAD_NAME (-1)
#define WZ264_PARAM_BAD_VALUE (-2)
#endif
int wz264_param_parse(wz264_param_t *, const char *name, const char *value);

/****************************************************************************
 * Advanced parameter handling functions
 ****************************************************************************/

/* These functions expose the full power of wz264's preset-tune-profile system for
 * easy adjustment of large numbers of internal parameters.
 *
 * In order to replicate wz264CLI's option handling, these functions MUST be called
 * in the following order:
 * 1) wz264_param_default_preset
 * 2) Custom user options (via param_parse or directly assigned variables)
 * 3) wz264_param_apply_fastfirstpass
 * 4) wz264_param_apply_profile
 *
 * Additionally, wz264CLI does not apply step 3 if the preset chosen is "placebo"
 * or --slow-firstpass is set. */

/* wz264_param_default_preset:
 *      The same as wz264_param_default, but also use the passed preset and tune
 *      to modify the default settings.
 *      (either can be NULL, which implies no preset or no tune, respectively)
 *
 *      Currently available presets are, ordered from fastest to slowest: */
#if !WZ264_SDK
static const char *const wz264_preset_names[] = { "ultrafast", "superfast", "veryfast", "faster",
                                                  "fast",      "medium",    "slow",     "slower",
                                                  "veryslow",  "placebo",   0 };
#endif

/*      The presets can also be indexed numerically, as in:
 *      wz264_param_default_preset( &param, "3", ... )
 *      with ultrafast mapping to "0" and placebo mapping to "9".  This mapping may
 *      of course change if new presets are added in between, but will always be
 *      ordered from fastest to slowest.
 *
 *      Warning: the speed of these presets scales dramatically.  Ultrafast is a full
 *      100 times faster than placebo!
 *
 *      Currently available tunings are: */
#if !WZ264_SDK
static const char *const wz264_tune_names[] = {
  "film", "animation",  "grain",  "stillimage",  "psnr",
  "ssim", "fastdecode", "screen", "zerolatency", 0
};
#endif

/*      Multiple tunings can be used if separated by a delimiter in ",./-+",
 *      however multiple psy tunings cannot be used.
 *      film, animation, grain, stillimage, psnr, and ssim are psy tunings.
 *
 *      returns 0 on success, negative on failure (e.g. invalid preset/tune name). */
int wz264_param_default_preset(wz264_param_t *, const char *preset, const char *tune);

/* wz264_param_apply_fastfirstpass:
 *      If first-pass mode is set (rc.b_stat_read == 0, rc.b_stat_write == 1),
 *      modify the encoder settings to disable options generally not useful on
 *      the first pass. */
void wz264_param_apply_fastfirstpass(wz264_param_t *);

/* wz264_param_apply_profile:
 *      Applies the restrictions of the given profile.
 *      Currently available profiles are, from most to least restrictive: */
#if !WZ264_SDK
static const char *const wz264_profile_names[] = { "baseline", "main",    "high", "high10",
                                                   "high422",  "high444", 0 };
#endif

/*      (can be NULL, in which case the function will do nothing)
 *
 *      Does NOT guarantee that the given profile will be used: if the restrictions
 *      of "High" are applied to settings that are already Baseline-compatible, the
 *      stream will remain baseline.  In short, it does not increase settings, only
 *      decrease them.
 *
 *      returns 0 on success, negative on failure (e.g. invalid profile name). */
int wz264_param_apply_profile(wz264_param_t *, const char *profile);

/****************************************************************************
 * Picture structures and functions
 ****************************************************************************/
enum pic_struct_e {
  PIC_STRUCT_AUTO = 0,         // automatically decide (default)
  PIC_STRUCT_PROGRESSIVE = 1,  // progressive frame
  // "TOP" and "BOTTOM" are not supported in wz264 (PAFF only)
  PIC_STRUCT_TOP_BOTTOM = 4,         // top field followed by bottom
  PIC_STRUCT_BOTTOM_TOP = 5,         // bottom field followed by top
  PIC_STRUCT_TOP_BOTTOM_TOP = 6,     // top field, bottom field, top field repeated
  PIC_STRUCT_BOTTOM_TOP_BOTTOM = 7,  // bottom field, top field, bottom field repeated
  PIC_STRUCT_DOUBLE = 8,             // double frame
  PIC_STRUCT_TRIPLE = 9,             // triple frame
};

typedef struct wz264_hrd_t {
  double cpb_initial_arrival_time;
  double cpb_final_arrival_time;
  double cpb_removal_time;

  double dpb_output_time;
} wz264_hrd_t;

/* Arbitrary user SEI:
 * Payload size is in bytes and the payload pointer must be valid.
 * Payload types and syntax can be found in Annex D of the H.264 Specification.
 * SEI payload alignment bits as described in Annex D must be included at the
 * end of the payload if needed.
 * The payload should not be NAL-encapsulated.
 * Payloads are written first in order of input, apart from in the case when HRD
 * is enabled where payloads are written after the Buffering Period SEI. */

typedef struct wz264_sei_payload_t {
  int payload_size;
  int payload_type;
  uint8_t *payload;
} wz264_sei_payload_t;

typedef struct wz264_sei_t {
  int num_payloads;
  wz264_sei_payload_t *payloads;
  /* In: optional callback to free each payload AND wz264_sei_payload_t when used. */
  void (*sei_free)(void *);
} wz264_sei_t;

#if !WZ264_SDK
typedef struct wz264_image_t {
  int i_csp;         /* Colorspace */
  int i_plane;       /* Number of image planes */
  int i_stride[4];   /* Strides for each plane */
  uint8_t *plane[4]; /* Pointers to each plane */
} wz264_image_t;

typedef struct wz264_image_properties_t {
  /* All arrays of data here are ordered as follows:
   * each array contains one offset per macroblock, in raster scan order.  In interlaced
   * mode, top-field MBs and bottom-field MBs are interleaved at the row level.
   * Macroblocks are 16x16 blocks of pixels (with respect to the luma plane).  For the
   * purposes of calculating the number of macroblocks, width and height are rounded up to
   * the nearest 16.  If in interlaced mode, height is rounded up to the nearest 32 instead. */

  /* In: an array of quantizer offsets to be applied to this image during encoding.
   *     These are added on top of the decisions made by wz264.
   *     Offsets can be fractional; they are added before QPs are rounded to integer.
   *     Adaptive quantization must be enabled to use this feature.  Behavior if quant
   *     offsets differ between encoding passes is undefined. */
  float *quant_offsets;
  /* In: optional callback to free quant_offsets when used.
   *     Useful if one wants to use a different quant_offset array for each frame. */
  void (*quant_offsets_free)(void *);

  /* In: optional array of flags for each macroblock.
   *     Allows specifying additional information for the encoder such as which macroblocks
   *     remain unchanged.  Usable flags are listed below.
   *     wz264_param_t.analyse.b_mb_info must be set to use this, since wz264 needs to track
   *     extra data internally to make full use of this information.
   *
   * Out: if b_mb_info_update is set, wz264 will update this array as a result of encoding.
   *
   *      For "MBINFO_CONSTANT", it will remove this flag on any macroblock whose decoded
   *      pixels have changed.  This can be useful for e.g. noting which areas of the
   *      frame need to actually be blitted. Note: this intentionally ignores the effects
   *      of deblocking for the current frame, which should be fine unless one needs exact
   *      pixel-perfect accuracy.
   *
   *      Results for MBINFO_CONSTANT are currently only set for P-frames, and are not
   *      guaranteed to enumerate all blocks which haven't changed.  (There may be false
   *      negatives, but no false positives.)
   */
  uint8_t *mb_info;
  /* In: optional callback to free mb_info when used. */
  void (*mb_info_free)(void *);

/* The macroblock is constant and remains unchanged from the previous frame. */
#define WZ264_MBINFO_CONSTANT (1 << 0)
  /* More flags may be added in the future. */

  /* Out: SSIM of the the frame luma (if wz264_param_t.b_ssim is set) */
  double f_ssim;
  /* Out: Average PSNR of the frame (if wz264_param_t.b_psnr is set) */
  double f_psnr_avg;
  /* Out: PSNR of Y, U, and V (if wz264_param_t.b_psnr is set) */
  double f_psnr[3];

  /* Out: Average effective CRF of the encoded frame */
  double f_crf_avg;
} wz264_image_properties_t;
#endif  // WZ_SDK

typedef struct wz264_picture_t {
  /* In: force picture type (if not auto)
   *     If wz264 encoding parameters are violated in the forcing of picture types,
   *     wz264 will correct the input picture type and log a warning.
   * Out: type of the picture encoded */
  int i_type;
  /* In: force quantizer for != WZ264_QP_AUTO */
  int i_qpplus1;
  /* In: pic_struct, for pulldown/doubling/etc...used only if b_pic_struct=1.
   *     use pic_struct_e for pic_struct inputs
   * Out: pic_struct element associated with frame */
  int i_pic_struct;
  /* Out: whether this frame is a keyframe.  Important when using modes that result in
   * SEI recovery points being used instead of IDR frames. */
  int b_keyframe;
  /* Out: when temporal scalable is enable, this is used to indicate tempolayer
     0 is the most important layer */
  int i_tempolayer;
  /* In: user pts, Out: pts of encoded picture (user)*/
  int64_t i_pts;
  /* Out: frame dts. When the pts of the first frame is close to zero,
   *      initial frames may have a negative dts which must be dealt with by any muxer */
  int64_t i_dts;
  /* In: raw image data */
  /* Out: reconstructed image data.  wz264 may skip part of the reconstruction process,
          e.g. deblocking, in frames where it isn't necessary.  To force complete
          reconstruction, at a small speed cost, set b_full_recon. */
  wz264_image_t img;
  /* In: optional information to modify encoder decisions for this frame
   * Out: information about the encoded frame */
  wz264_image_properties_t prop;
  /* Out: HRD timing information. Output only when i_nal_hrd is set. */
  wz264_hrd_t hrd_timing;
  /* In: arbitrary user SEI (e.g subtitles, AFDs) */
  wz264_sei_t extra_sei;
  /* private user data. copied from input to output frames. */
  void *opaque;
} wz264_picture_t;

/* wz264_picture_init:
 *  initialize an wz264_picture_t.  Needs to be done if the calling application
 *  allocates its own wz264_picture_t as opposed to using wz264_picture_alloc. */
void wz264_picture_init(wz264_picture_t *pic);

/* wz264_picture_alloc:
 *  alloc data for a picture. You must call wz264_picture_clean on it.
 *  returns 0 on success, or -1 on malloc failure or invalid colorspace. */
int wz264_picture_alloc(wz264_picture_t *pic, int i_csp, int i_width, int i_height);

/* wz264_picture_clean:
 *  free associated resource for a wz264_picture_t allocated with
 *  wz264_picture_alloc ONLY */
void wz264_picture_clean(wz264_picture_t *pic);

/****************************************************************************
 * Encoder functions
 ****************************************************************************/

/* wz264_encoder_open:
 *      create a new encoder handler, all parameters from wz264_param_t are copied */
wz264_t *wz264_encoder_open(wz264_param_t *);

/* wz264_encoder_reconfig:
 *      various parameters from wz264_param_t are copied.
 *      this takes effect immediately, on whichever frame is encoded next;
 *      due to delay, this may not be the next frame passed to encoder_encode.
 *      if the change should apply to some particular frame, use wz264_picture_t->param instead.
 *      returns 0 on success, negative on parameter validation error.
 *      not all parameters can be changed; see the actual function for a detailed breakdown.
 *
 *      since not all parameters can be changed, moving from preset to preset may not always
 *      fully copy all relevant parameters, but should still work usably in practice. however,
 *      more so than for other presets, many of the speed shortcuts used in ultrafast cannot be
 *      switched out of; using reconfig to switch between ultrafast and other presets is not
 *      recommended without a more fine-grained breakdown of parameters to take this into account.
 */
int wz264_encoder_reconfig(wz264_t *, wz264_param_t *);
/* wz264_encoder_parameters:
 *      copies the current internal set of parameters to the pointer provided
 *      by the caller.  useful when the calling application needs to know
 *      how wz264_encoder_open has changed the parameters, or the current state
 *      of the encoder after multiple wz264_encoder_reconfig calls.
 *      note that the data accessible through pointers in the returned param struct
 *      (e.g. filenames) should not be modified by the calling application. */
void wz264_encoder_parameters(wz264_t *, wz264_param_t *);
/* wz264_encoder_headers:
 *      return the SPS and PPS that will be used for the whole stream.
 *      *pi_nal is the number of NAL units outputted in pp_nal.
 *      returns the number of bytes in the returned NALs.
 *      returns negative on error.
 *      the payloads of all output NALs are guaranteed to be sequential in memory. */
int wz264_encoder_headers(wz264_t *, wz264_nal_t **pp_nal, int *pi_nal);
/* wz264_encoder_encode:
 *      encode one picture.
 *      *pi_nal is the number of NAL units outputted in pp_nal.
 *      returns the number of bytes in the returned NALs.
 *      returns negative on error and zero if no NAL units returned.
 *      the payloads of all output NALs are guaranteed to be sequential in memory. */
int wz264_encoder_encode(wz264_t *, wz264_nal_t **pp_nal, int *pi_nal, wz264_picture_t *pic_in,
                         wz264_picture_t *pic_out);
/* wz264_encoder_close:
 *      close an encoder handler */
void wz264_encoder_close(wz264_t *);
/* wz264_encoder_delayed_frames:
 *      return the number of currently delayed (buffered) frames
 *      this should be used at the end of the stream, to know when you have all the encoded frames.
 */
int wz264_encoder_delayed_frames(wz264_t *);
/* wz264_encoder_maximum_delayed_frames( wz264_t * ):
 *      return the maximum number of delayed (buffered) frames that can occur with the current
 *      parameters. */
int wz264_encoder_maximum_delayed_frames(wz264_t *);
/* wz264_encoder_intra_refresh:
 *      If an intra refresh is not in progress, begin one with the next P-frame.
 *      If an intra refresh is in progress, begin one as soon as the current one finishes.
 *      Requires that b_intra_refresh be set.
 *
 *      Useful for interactive streaming where the client can tell the server that packet loss has
 *      occurred.  In this case, keyint can be set to an extremely high value so that intra
 * refreshes only occur when calling wz264_encoder_intra_refresh.
 *
 *      In multi-pass encoding, if wz264_encoder_intra_refresh is called differently in each pass,
 *      behavior is undefined.
 *
 *      Should not be called during an wz264_encoder_encode. */
void wz264_encoder_intra_refresh(wz264_t *);
/* wz264_encoder_invalidate_reference:
 *      An interactive error resilience tool, designed for use in a low-latency
 * one-encoder-few-clients system.  When the client has packet loss or otherwise incorrectly decodes
 * a frame, the encoder can be told with this command to "forget" the frame and all frames that
 * depend on it, referencing only frames that occurred before the loss.  This will force a keyframe
 * if no frames are left to reference after the aforementioned "forgetting".
 *
 *      It is strongly recommended to use a large i_dpb_size in this case, which allows the encoder
 * to keep around extra, older frames to fall back on in case more recent frames are all
 * invalidated. Unlike increasing i_frame_reference, this does not increase the number of frames
 * used for motion estimation and thus has no speed impact.  It is also recommended to set a very
 * large keyframe interval, so that keyframes are not used except as necessary for error recovery.
 *
 *      wz264_encoder_invalidate_reference is not currently compatible with the use of B-frames or
 * intra refresh.
 *
 *      In multi-pass encoding, if wz264_encoder_invalidate_reference is called differently in each
 * pass, behavior is undefined.
 *
 *      Should not be called during an wz264_encoder_encode, but multiple calls can be made
 * simultaneously.
 *
 *      Returns 0 on success, negative on failure. */
int wz264_encoder_invalidate_reference(wz264_t *, int64_t pts);

#ifdef __cplusplus
}
#endif

#endif

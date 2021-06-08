/*
 * H.264 encoding using the wz264 library
 * Copyright (C) 2005  Mans Rullgard <mans@mansr.com>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libavutil/eval.h"
#include "libavutil/internal.h"
#include "libavutil/opt.h"
#include "libavutil/mem.h"
#include "libavutil/pixdesc.h"
#include "libavutil/stereo3d.h"
#include "libavutil/time.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/threadmessage.h"
#include "avcodec.h"
#include "internal.h"
#include "packet_internal.h"
#include <wz264.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct WZ264Context {
    AVClass        *class;
    wz264_param_t    params;
    wz264_t         *enc;
    wz264_picture_t  pic;
    uint8_t        *sei;
    int             sei_size;
    char *preset;
    char *tune;
    char *profile;
    char *level;
    int fastfirstpass;
    char *wpredp;
    char *wz264opts;
    float crf;
    float crf_max;
    int cqp;
    int aq_mode;
    float aq_strength;
    float aq_smooth;
    char *psy_rd;
    int psy;
    int rc_lookahead;
    int weightp;
    int weightb;
    int ssim;
    int intra_refresh;
    int bluray_compat;
    int b_bias;
    int b_pyramid;
    int mixed_refs;
    int dct8x8;
    int fast_pskip;
    int aud;
    int mbtree;
    char *deblock;
    float cplxblur;
    char *partitions;
    int direct_pred;
    int slice_max_size;
    char *stats;
    int nal_hrd;
    int avcintra_class;
    int motion_est;
    int forced_idr;
    int coder;
    int a53_cc;
    int b_frame_strategy;
    int chroma_offset;
    int scenechange_threshold;
    int noise_reduction;

    int has_output;
    int has_flushed;
    AVThreadMessageQueue* filter_frame_cache;
    char *wz264_params;
} WZ264Context;

static void WZ264_log(void *p, int level, const char *fmt, va_list args)
{
    static const int level_map[] = {
        [WZ264_LOG_ERROR]   = AV_LOG_ERROR,
        [WZ264_LOG_WARNING] = AV_LOG_WARNING,
        [WZ264_LOG_INFO]    = AV_LOG_INFO,
        [WZ264_LOG_DEBUG]   = AV_LOG_DEBUG
    };

    if (level < 0 || level > WZ264_LOG_DEBUG)
        return;

    av_vlog(p, level_map[level], fmt, args);
}


static int encode_nals(AVCodecContext *ctx, AVPacket *pkt,
                       const wz264_nal_t *nals, int nnal)
{
    WZ264Context *x4 = ctx->priv_data;
    uint8_t *p;
    int i, size = x4->sei_size, ret;

    if (!nnal)
        return 0;

    for (i = 0; i < nnal; i++)
        size += nals[i].i_payload;

    if ((ret = ff_alloc_packet2(ctx, pkt, size, 0)) < 0)
        return ret;

    p = pkt->data;

    /* Write the SEI as part of the first frame. */
    if (x4->sei_size > 0 && nnal > 0) {
        if (x4->sei_size > size) {
            av_log(ctx, AV_LOG_ERROR, "Error: nal buffer is too small\n");
            return -1;
        }
        memcpy(p, x4->sei, x4->sei_size);
        p += x4->sei_size;
        x4->sei_size = 0;
        av_freep(&x4->sei);
    }

    for (i = 0; i < nnal; i++){
        memcpy(p, nals[i].p_payload, nals[i].i_payload);
        p += nals[i].i_payload;
    }

    return 1;
}

static int avfmt2_num_planes(int avfmt)
{
    switch (avfmt) {
    case AV_PIX_FMT_YUV420P:
    case AV_PIX_FMT_YUVJ420P:
    case AV_PIX_FMT_YUV420P9:
    case AV_PIX_FMT_YUV420P10:
    case AV_PIX_FMT_YUV444P:
        return 3;

    case AV_PIX_FMT_BGR0:
    case AV_PIX_FMT_BGR24:
    case AV_PIX_FMT_RGB24:
    case AV_PIX_FMT_GRAY8:
    case AV_PIX_FMT_GRAY10:
        return 1;

    default:
        return 3;
    }
}

static void reconfig_encoder(AVCodecContext *ctx, const AVFrame *frame)
{
    WZ264Context *x4 = ctx->priv_data;
    AVFrameSideData *side_data;


  if (x4->avcintra_class < 0) {
    if (x4->params.b_interlaced && x4->params.b_tff != frame->top_field_first) {
        x4->params.b_tff = frame->top_field_first;
        wz264_encoder_reconfig(x4->enc, &x4->params);
    }
    if (x4->params.vui.i_sar_height*ctx->sample_aspect_ratio.num != ctx->sample_aspect_ratio.den * x4->params.vui.i_sar_width) {
        x4->params.vui.i_sar_height = ctx->sample_aspect_ratio.den;
        x4->params.vui.i_sar_width  = ctx->sample_aspect_ratio.num;
        wz264_encoder_reconfig(x4->enc, &x4->params);
    }

    if (x4->params.rc.i_vbv_buffer_size != ctx->rc_buffer_size / 1000 ||
        x4->params.rc.i_vbv_max_bitrate != ctx->rc_max_rate    / 1000) {
        x4->params.rc.i_vbv_buffer_size = ctx->rc_buffer_size / 1000;
        x4->params.rc.i_vbv_max_bitrate = ctx->rc_max_rate    / 1000;
        wz264_encoder_reconfig(x4->enc, &x4->params);
    }

    if (x4->params.rc.i_rc_method == WZ264_RC_ABR &&
        x4->params.rc.i_bitrate != ctx->bit_rate / 1000) {
        x4->params.rc.i_bitrate = ctx->bit_rate / 1000;
        wz264_encoder_reconfig(x4->enc, &x4->params);
    }

    if (x4->crf >= 0 &&
        x4->params.rc.i_rc_method == WZ264_RC_CRF &&
        x4->params.rc.f_rf_constant != x4->crf) {
        x4->params.rc.f_rf_constant = x4->crf;
        wz264_encoder_reconfig(x4->enc, &x4->params);
    }

    if (x4->params.rc.i_rc_method == WZ264_RC_CQP &&
        x4->cqp >= 0 &&
        x4->params.rc.i_qp_constant != x4->cqp) {
        x4->params.rc.i_qp_constant = x4->cqp;
        wz264_encoder_reconfig(x4->enc, &x4->params);
    }

    if (x4->crf_max >= 0 &&
        x4->params.rc.f_rf_constant_max != x4->crf_max) {
        x4->params.rc.f_rf_constant_max = x4->crf_max;
        wz264_encoder_reconfig(x4->enc, &x4->params);
    }
  }

    side_data = av_frame_get_side_data(frame, AV_FRAME_DATA_STEREO3D);
    if (side_data) {
        AVStereo3D *stereo = (AVStereo3D *)side_data->data;
        int fpa_type;

        switch (stereo->type) {
        case AV_STEREO3D_CHECKERBOARD:
            fpa_type = 0;
            break;
        case AV_STEREO3D_COLUMNS:
            fpa_type = 1;
            break;
        case AV_STEREO3D_LINES:
            fpa_type = 2;
            break;
        case AV_STEREO3D_SIDEBYSIDE:
            fpa_type = 3;
            break;
        case AV_STEREO3D_TOPBOTTOM:
            fpa_type = 4;
            break;
        case AV_STEREO3D_FRAMESEQUENCE:
            fpa_type = 5;
            break;
        case AV_STEREO3D_2D:
            fpa_type = 6;
            break;
        default:
            fpa_type = -1;
            break;
        }

        /* Inverted mode is not supported by wz264 */
        if (stereo->flags & AV_STEREO3D_FLAG_INVERT) {
            av_log(ctx, AV_LOG_WARNING,
                   "Ignoring unsupported inverted stereo value %d\n", fpa_type);
            fpa_type = -1;
        }

        if (fpa_type != x4->params.i_frame_packing) {
            x4->params.i_frame_packing = fpa_type;
            wz264_encoder_reconfig(x4->enc, &x4->params);
        }
    }
}

static int WZ264_frame(AVCodecContext *ctx, AVPacket *pkt, const AVFrame *frame,
                      int *got_packet)
{
    WZ264Context *x4 = ctx->priv_data;
    wz264_nal_t *nal;
    int nnal, i;
    int ret = 0;
    wz264_picture_t pic_out = {0};
    int pict_type;

    int out_size = 0;
    do {
        wz264_picture_init( &x4->pic );
        x4->pic.img.i_csp   = x4->params.i_csp;
        if (x4->params.i_bitdepth > 8)
            x4->pic.img.i_csp |= WZ264_CSP_HIGH_DEPTH;
        x4->pic.img.i_plane = avfmt2_num_planes(ctx->pix_fmt);

        if (frame) {
            for (i = 0; i < x4->pic.img.i_plane; i++) {
                x4->pic.img.plane[i]    = frame->data[i];
                x4->pic.img.i_stride[i] = frame->linesize[i];
            }

            x4->pic.i_pts  = frame->pts;

            switch (frame->pict_type) {
            case AV_PICTURE_TYPE_I:
                x4->pic.i_type = x4->forced_idr > 0 ? WZ264_TYPE_IDR
                                                    : WZ264_TYPE_KEYFRAME;
                break;
            case AV_PICTURE_TYPE_P:
                x4->pic.i_type = WZ264_TYPE_P;
                break;
            case AV_PICTURE_TYPE_B:
                x4->pic.i_type = WZ264_TYPE_B;
                break;
            default:
                x4->pic.i_type = WZ264_TYPE_AUTO;
                break;
            }

            reconfig_encoder(ctx, frame);

            if (x4->a53_cc) {
                void *sei_data;
                size_t sei_size;

                ret = ff_alloc_a53_sei(frame, 0, &sei_data, &sei_size);
                if (ret < 0) {
                    av_log(ctx, AV_LOG_ERROR, "Not enough memory for closed captions, skipping\n");
                } else if (sei_data) {
                    x4->pic.extra_sei.payloads = av_mallocz(sizeof(x4->pic.extra_sei.payloads[0]));
                    if (x4->pic.extra_sei.payloads == NULL) {
                        av_log(ctx, AV_LOG_ERROR, "Not enough memory for closed captions, skipping\n");
                        av_free(sei_data);
                    } else {
                        x4->pic.extra_sei.sei_free = av_free;

                        x4->pic.extra_sei.payloads[0].payload_size = sei_size;
                        x4->pic.extra_sei.payloads[0].payload = sei_data;
                        x4->pic.extra_sei.num_payloads = 1;
                        x4->pic.extra_sei.payloads[0].payload_type = 4;
                    }
                }
            }
        }

        do {
            if (wz264_encoder_encode(x4->enc, &nal, &nnal, frame? &x4->pic: NULL, &pic_out) < 0)
                return AVERROR_EXTERNAL;

            ret = encode_nals(ctx, pkt, nal, nnal);
            if (ret < 0)
                return ret;
            out_size += pkt->size;
        } while (!ret && !frame && wz264_encoder_delayed_frames(x4->enc));

        if (x4->has_flushed && !ret) {
            av_frame_free(&frame);
            if (x4->filter_frame_cache) {
                // if frame == NULL and size == 0, result frame no change remain NULL
                av_thread_message_queue_recv(x4->filter_frame_cache, &frame, AV_THREAD_MESSAGE_NONBLOCK);
            }

        }

    } while(x4->has_flushed && !ret && frame);


    pkt->pts = pic_out.i_pts;
    pkt->dts = pic_out.i_dts;

    switch (pic_out.i_type) {
    case WZ264_TYPE_IDR:
    case WZ264_TYPE_I:
        pict_type = AV_PICTURE_TYPE_I;
        break;
    case WZ264_TYPE_P:
        pict_type = AV_PICTURE_TYPE_P;
        break;
    case WZ264_TYPE_B:
    case WZ264_TYPE_BREF:
        pict_type = AV_PICTURE_TYPE_B;
        break;
    default:
        pict_type = AV_PICTURE_TYPE_NONE;
    }
#if FF_API_CODED_FRAME
FF_DISABLE_DEPRECATION_WARNINGS
    ctx->coded_frame->pict_type = pict_type;
FF_ENABLE_DEPRECATION_WARNINGS
#endif

    pkt->flags |= AV_PKT_FLAG_KEY*pic_out.b_keyframe;
    if (ret) {
        ff_side_data_set_encoder_stats(pkt, (pic_out.i_qpplus1 - 1) * FF_QP2LAMBDA, NULL, 0, pict_type);

#if FF_API_CODED_FRAME
FF_DISABLE_DEPRECATION_WARNINGS
        ctx->coded_frame->quality = (pic_out.i_qpplus1 - 1) * FF_QP2LAMBDA;
FF_ENABLE_DEPRECATION_WARNINGS
#endif
    }

    if (!x4->has_output && ret > 0) {
        x4->has_output = 1;
    }

    *got_packet = ret;

    return 0;
}

static av_cold int WZ264_close(AVCodecContext *avctx)
{
    WZ264Context *x4 = avctx->priv_data;

    av_freep(&avctx->extradata);
    av_freep(&x4->sei);

    if (x4->enc) {
        wz264_encoder_close(x4->enc);
        x4->enc = NULL;
    }

    av_thread_message_queue_free(&x4->filter_frame_cache);

    return 0;
}

#define OPT_STR(opt, param)                                                   \
    do {                                                                      \
        int ret;                                                              \
        if ((ret = wz264_param_parse(&x4->params, opt, param)) < 0) { \
            if(ret == WZ264_PARAM_BAD_NAME)                                    \
                av_log(avctx, AV_LOG_ERROR,                                   \
                        "bad option '%s': '%s'\n", opt, param);               \
            else                                                              \
                av_log(avctx, AV_LOG_ERROR,                                   \
                        "bad value for '%s': '%s'\n", opt, param);            \
            return -1;                                                        \
        }                                                                     \
    } while (0)

static int convert_pix_fmt(enum AVPixelFormat pix_fmt)
{
    switch (pix_fmt) {
    case AV_PIX_FMT_YUV420P:
    case AV_PIX_FMT_YUVJ420P:
    case AV_PIX_FMT_YUV420P9:
    case AV_PIX_FMT_YUV420P10: return WZ264_CSP_I420;
    case AV_PIX_FMT_YUV422P:
    case AV_PIX_FMT_YUVJ422P:
    case AV_PIX_FMT_YUV422P10: return WZ264_CSP_I422;
    case AV_PIX_FMT_YUV444P:
    case AV_PIX_FMT_YUVJ444P:
    case AV_PIX_FMT_YUV444P9:
    case AV_PIX_FMT_YUV444P10: return WZ264_CSP_I444;
#if CONFIG_LIBWZ264RGB_ENCODER
    case AV_PIX_FMT_BGR0:
        return WZ264_CSP_BGRA;
    case AV_PIX_FMT_BGR24:
        return WZ264_CSP_BGR;

    case AV_PIX_FMT_RGB24:
        return WZ264_CSP_RGB;
#endif
    case AV_PIX_FMT_NV12:      return WZ264_CSP_NV12;
    case AV_PIX_FMT_NV16:
    case AV_PIX_FMT_NV20:      return WZ264_CSP_NV16;
#ifdef WZ264_CSP_NV21
    case AV_PIX_FMT_NV21:      return WZ264_CSP_NV21;
#endif
#ifdef WZ264_CSP_I400
    case AV_PIX_FMT_GRAY8:
    case AV_PIX_FMT_GRAY10:    return WZ264_CSP_I400;
#endif
    };
    return 0;
}

#define PARSE_WZ264_OPT(name, var)\
    if (x4->var && wz264_param_parse(&x4->params, name, x4->var) < 0) {\
        av_log(avctx, AV_LOG_ERROR, "Error parsing option '%s' with value '%s'.\n", name, x4->var);\
        return AVERROR(EINVAL);\
    }

extern char wz_filter_configs_string[1024];
static av_cold int WZ264_init(AVCodecContext *avctx)
{
    WZ264Context *x4 = avctx->priv_data;
    AVCPBProperties *cpb_props;
    int sw,sh;

    if (avctx->global_quality > 0)
        av_log(avctx, AV_LOG_WARNING, "-qscale is ignored, -crf is recommended.\n");

#if CONFIG_LIBWZ262_ENCODER
    if (avctx->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
        x4->params.b_mpeg2 = 1;
        wz264_param_default_mpeg2(&x4->params);
    } else
#endif
    wz264_param_default(&x4->params);
    x4->params.b_deblocking_filter         = avctx->flags & AV_CODEC_FLAG_LOOP_FILTER;

    if (x4->preset || x4->tune)
        if (wz264_param_default_preset(&x4->params, x4->preset, x4->tune) < 0) {
            int i;
            av_log(avctx, AV_LOG_ERROR, "Error setting preset/tune %s/%s.\n", x4->preset, x4->tune);
            av_log(avctx, AV_LOG_INFO, "Possible presets:");
            for (i = 0; wz264_preset_names[i]; i++)
                av_log(avctx, AV_LOG_INFO, " %s", wz264_preset_names[i]);
            av_log(avctx, AV_LOG_INFO, "\n");
            av_log(avctx, AV_LOG_INFO, "Possible tunes:");
            for (i = 0; wz264_tune_names[i]; i++)
                av_log(avctx, AV_LOG_INFO, " %s", wz264_tune_names[i]);
            av_log(avctx, AV_LOG_INFO, "\n");
            return AVERROR(EINVAL);
        }

    if (avctx->level > 0)
        x4->params.i_level_idc = avctx->level;

    x4->params.pf_log               = WZ264_log;
    x4->params.p_log_private        = avctx;
    x4->params.i_log_level          = WZ264_LOG_DEBUG;
    x4->params.i_csp                = convert_pix_fmt(avctx->pix_fmt);
    x4->params.i_bitdepth           = av_pix_fmt_desc_get(avctx->pix_fmt)->comp[0].depth;

    PARSE_WZ264_OPT("weightp", wpredp);

    if (avctx->bit_rate) {
        x4->params.rc.i_bitrate   = avctx->bit_rate / 1000;
        x4->params.rc.i_rc_method = WZ264_RC_ABR;
    }
    x4->params.rc.i_vbv_buffer_size = avctx->rc_buffer_size / 1000;
    x4->params.rc.i_vbv_max_bitrate = avctx->rc_max_rate    / 1000;
    x4->params.rc.b_stat_write      = avctx->flags & AV_CODEC_FLAG_PASS1;
    if (avctx->flags & AV_CODEC_FLAG_PASS2) {
        x4->params.rc.b_stat_read = 1;
    } else {
        if (x4->crf >= 0) {
            x4->params.rc.i_rc_method   = WZ264_RC_CRF;
            x4->params.rc.f_rf_constant = x4->crf;
        } else if (x4->cqp >= 0) {
            x4->params.rc.i_rc_method   = WZ264_RC_CQP;
            x4->params.rc.i_qp_constant = x4->cqp;
        }

        if (x4->crf_max >= 0)
            x4->params.rc.f_rf_constant_max = x4->crf_max;
    }

    if (avctx->rc_buffer_size && avctx->rc_initial_buffer_occupancy > 0 &&
        (avctx->rc_initial_buffer_occupancy <= avctx->rc_buffer_size)) {
        x4->params.rc.f_vbv_buffer_init =
            (float)avctx->rc_initial_buffer_occupancy / avctx->rc_buffer_size;
    }

    PARSE_WZ264_OPT("level", level);

    if (avctx->i_quant_factor > 0)
        x4->params.rc.f_ip_factor         = 1 / fabs(avctx->i_quant_factor);
    if (avctx->b_quant_factor > 0)
        x4->params.rc.f_pb_factor         = avctx->b_quant_factor;

#if FF_API_PRIVATE_OPT
FF_DISABLE_DEPRECATION_WARNINGS
    if (avctx->chromaoffset >= 0)
        x4->chroma_offset = avctx->chromaoffset;
FF_ENABLE_DEPRECATION_WARNINGS
#endif
    if (x4->chroma_offset >= 0)
        x4->params.analyse.i_chroma_qp_offset = x4->chroma_offset;

    if (avctx->gop_size >= 0)
        x4->params.i_keyint_max         = avctx->gop_size;
    if (avctx->max_b_frames >= 0)
        x4->params.i_bframe             = avctx->max_b_frames;

#if FF_API_PRIVATE_OPT
FF_DISABLE_DEPRECATION_WARNINGS
    if (avctx->scenechange_threshold >= 0)
        x4->scenechange_threshold = avctx->scenechange_threshold;
FF_ENABLE_DEPRECATION_WARNINGS
#endif
    if (x4->scenechange_threshold >= 0)
        x4->params.i_scenecut_threshold = x4->scenechange_threshold;

    if (avctx->qmin >= 0)
        x4->params.rc.i_qp_min          = avctx->qmin;
    if (avctx->qmax >= 0)
        x4->params.rc.i_qp_max          = avctx->qmax;
    if (avctx->max_qdiff >= 0)
        x4->params.rc.i_qp_step         = avctx->max_qdiff;
    if (avctx->qblur >= 0)
        x4->params.rc.f_qblur           = avctx->qblur;     /* temporally blur quants */
    if (avctx->qcompress >= 0)
        x4->params.rc.f_qcompress       = avctx->qcompress; /* 0.0 => cbr, 1.0 => constant qp */
    if (avctx->refs >= 0)
        x4->params.i_frame_reference    = avctx->refs;
    else if (x4->level) {
        int i;
        int mbn = AV_CEIL_RSHIFT(avctx->width, 4) * AV_CEIL_RSHIFT(avctx->height, 4);
        int level_id = -1;
        char *tail;
        int scale = 1;

        if (!strcmp(x4->level, "1b")) {
            level_id = 9;
        } else if (atof(x4->level) < 7){
            level_id = (int)(10 * atof(x4->level) + 0.5);
        } else {
            level_id = atoi(x4->level);
        }
        if (level_id <= 0)
            av_log(avctx, AV_LOG_WARNING, "Failed to parse level\n");

        for (i = 0; i<wz264_levels[i].level_idc; i++)
            if (wz264_levels[i].level_idc == level_id)
                x4->params.i_frame_reference = av_clip(wz264_levels[i].dpb / mbn / scale, 1, x4->params.i_frame_reference);
    }

    if (avctx->trellis >= 0)
        x4->params.analyse.i_trellis    = avctx->trellis;
    if (avctx->me_range >= 0)
        x4->params.analyse.i_me_range   = avctx->me_range;
#if FF_API_PRIVATE_OPT
    FF_DISABLE_DEPRECATION_WARNINGS
    if (avctx->noise_reduction >= 0)
        x4->noise_reduction = avctx->noise_reduction;
    FF_ENABLE_DEPRECATION_WARNINGS
#endif
    if (x4->noise_reduction >= 0)
        x4->params.analyse.i_noise_reduction = x4->noise_reduction;
    if (avctx->me_subpel_quality >= 0)
        x4->params.analyse.i_subpel_refine   = avctx->me_subpel_quality;
#if FF_API_PRIVATE_OPT
FF_DISABLE_DEPRECATION_WARNINGS
    if (avctx->b_frame_strategy >= 0)
        x4->b_frame_strategy = avctx->b_frame_strategy;
FF_ENABLE_DEPRECATION_WARNINGS
#endif
    if (avctx->keyint_min >= 0)
        x4->params.i_keyint_min = avctx->keyint_min;
#if FF_API_CODER_TYPE
FF_DISABLE_DEPRECATION_WARNINGS
    if (avctx->coder_type >= 0)
        x4->coder = avctx->coder_type == FF_CODER_TYPE_AC;
FF_ENABLE_DEPRECATION_WARNINGS
#endif
    if (avctx->me_cmp >= 0)
        x4->params.analyse.b_chroma_me = avctx->me_cmp & FF_CMP_CHROMA;

    if (x4->aq_mode >= 0)
        x4->params.rc.i_aq_mode = x4->aq_mode;
    if (x4->aq_strength >= 0)
        x4->params.rc.f_aq_strength = x4->aq_strength;
    if (x4->aq_smooth >= 0)
        x4->params.rc.f_aq_smooth = x4->aq_smooth;
    PARSE_WZ264_OPT("psy-rd", psy_rd);
    PARSE_WZ264_OPT("deblock", deblock);
    PARSE_WZ264_OPT("partitions", partitions);
    PARSE_WZ264_OPT("stats", stats);
    if (x4->psy >= 0)
        x4->params.analyse.b_psy  = x4->psy;
    if (x4->rc_lookahead >= 0)
        x4->params.rc.i_lookahead = x4->rc_lookahead;
    if (x4->weightp >= 0)
        x4->params.analyse.i_weighted_pred = x4->weightp;
    if (x4->weightb >= 0)
        x4->params.analyse.b_weighted_bipred = x4->weightb;
    if (x4->cplxblur >= 0)
        x4->params.rc.f_complexity_blur = x4->cplxblur;

    if (x4->ssim >= 0)
        x4->params.analyse.b_ssim = x4->ssim;
    if (x4->intra_refresh >= 0)
        x4->params.i_intra_refresh = x4->intra_refresh;
    if (x4->bluray_compat >= 0) {
        x4->params.b_bluray_compat = x4->bluray_compat;
        x4->params.b_vfr_input = 0;
    }
    if (x4->b_bias != INT_MIN)
        x4->params.i_bframe_bias              = x4->b_bias;
    if (x4->b_pyramid >= 0)
        x4->params.i_bframe_pyramid = x4->b_pyramid;
    if (x4->mixed_refs >= 0)
        x4->params.analyse.b_mixed_references = x4->mixed_refs;
    if (x4->dct8x8 >= 0)
        x4->params.analyse.b_transform_8x8    = x4->dct8x8;
   // if (x4->fast_pskip >= 0)
   //     x4->params.analyse.b_fast_pskip       = x4->fast_pskip;
    if (x4->aud >= 0)
        x4->params.b_aud                      = x4->aud;
    if (x4->mbtree >= 0)
        x4->params.rc.b_mb_tree               = x4->mbtree;
    if (x4->direct_pred >= 0)
        x4->params.analyse.i_direct_mv_pred   = x4->direct_pred;

    if (x4->slice_max_size >= 0)
        x4->params.i_slice_max_size =  x4->slice_max_size;

    if (x4->fastfirstpass)
        wz264_param_apply_fastfirstpass(&x4->params);

    /* Allow specifying the wz264 profile through AVCodecContext. */
    if (!x4->profile)
        switch (avctx->profile) {
        case FF_PROFILE_H264_BASELINE:
            x4->profile = av_strdup("baseline");
            break;
        case FF_PROFILE_H264_HIGH:
            x4->profile = av_strdup("high");
            break;
        case FF_PROFILE_H264_HIGH_10:
            x4->profile = av_strdup("high10");
            break;
        case FF_PROFILE_H264_HIGH_422:
            x4->profile = av_strdup("high422");
            break;
        case FF_PROFILE_H264_HIGH_444:
            x4->profile = av_strdup("high444");
            break;
        case FF_PROFILE_H264_MAIN:
            x4->profile = av_strdup("main");
            break;
        default:
            break;
        }

    if (x4->nal_hrd >= 0)
        x4->params.i_nal_hrd = x4->nal_hrd;

    if (x4->motion_est >= 0)
        x4->params.analyse.i_me_method = x4->motion_est;

    if (x4->coder >= 0)
        x4->params.b_cabac = x4->coder;

    if (x4->b_frame_strategy >= 0)
        x4->params.i_bframe_adaptive = x4->b_frame_strategy;

    if (x4->profile)
        if (wz264_param_apply_profile(&x4->params, x4->profile) < 0) {
            int i;
            av_log(avctx, AV_LOG_ERROR, "Error setting profile %s.\n", x4->profile);
            av_log(avctx, AV_LOG_INFO, "Possible profiles:");
            for (i = 0; wz264_profile_names[i]; i++)
                av_log(avctx, AV_LOG_INFO, " %s", wz264_profile_names[i]);
            av_log(avctx, AV_LOG_INFO, "\n");
            return AVERROR(EINVAL);
        }

    x4->params.i_width          = avctx->width;
    x4->params.i_height         = avctx->height;
    av_reduce(&sw, &sh, avctx->sample_aspect_ratio.num, avctx->sample_aspect_ratio.den, 4096);
    x4->params.vui.i_sar_width  = sw;
    x4->params.vui.i_sar_height = sh;
    x4->params.i_timebase_den = avctx->time_base.den;
    x4->params.i_timebase_num = avctx->time_base.num;
    x4->params.i_fps_num = avctx->time_base.den;
    x4->params.i_fps_den = avctx->time_base.num * avctx->ticks_per_frame;

    x4->params.analyse.b_psnr = avctx->flags & AV_CODEC_FLAG_PSNR;

    x4->params.i_threads      = avctx->thread_count;
    if (avctx->thread_type)
        x4->params.b_sliced_threads = avctx->thread_type == FF_THREAD_SLICE;

    x4->params.b_open_gop     = !(avctx->flags & AV_CODEC_FLAG_CLOSED_GOP);

    x4->params.i_slice_count  = avctx->slices;

    x4->params.vui.b_fullrange = avctx->pix_fmt == AV_PIX_FMT_YUVJ420P ||
                                 avctx->pix_fmt == AV_PIX_FMT_YUVJ422P ||
                                 avctx->pix_fmt == AV_PIX_FMT_YUVJ444P ||
                                 avctx->color_range == AVCOL_RANGE_JPEG;

    x4->params.b_interlaced   = avctx->flags & AV_CODEC_FLAG_INTERLACED_DCT;
    if (avctx->colorspace != AVCOL_SPC_UNSPECIFIED)
        x4->params.vui.i_colmatrix = avctx->colorspace;
    if (avctx->color_primaries != AVCOL_PRI_UNSPECIFIED)
        x4->params.vui.i_colorprim = avctx->color_primaries;
    if (avctx->color_trc != AVCOL_TRC_UNSPECIFIED)
        x4->params.vui.i_transfer  = avctx->color_trc;

    if (avctx->flags & AV_CODEC_FLAG_GLOBAL_HEADER)
        x4->params.b_repeat_headers = 0;

    if(x4->wz264opts){
        const char *p= x4->wz264opts;
        while(p){
            char param[4096]={0}, val[4096]={0};
            if(sscanf(p, "%4095[^:=]=%4095[^:]", param, val) == 1){
                OPT_STR(param, "1");
            }else
                OPT_STR(param, val);
            p= strchr(p, ':');
            p+=!!p;
        }
    }

    if (x4->wz264_params) {
        AVDictionary *dict    = NULL;
        AVDictionaryEntry *en = NULL;

        if (!av_dict_parse_string(&dict, x4->wz264_params, "=", ":", 0)) {
            while ((en = av_dict_get(dict, "", en, AV_DICT_IGNORE_SUFFIX))) {
                if (wz264_param_parse(&x4->params, en->key, en->value) < 0)
                    av_log(avctx, AV_LOG_WARNING,
                           "Error parsing option '%s = %s'.\n",
                            en->key, en->value);
            }

            av_dict_free(&dict);
        }
    }

    // update AVCodecContext with wz264 parameters
    avctx->has_b_frames = x4->params.i_bframe ?
        x4->params.i_bframe_pyramid ? 2 : 1 : 0;
    if (avctx->max_b_frames < 0)
        avctx->max_b_frames = 0;

    avctx->bit_rate = x4->params.rc.i_bitrate*1000;

    x4->enc = wz264_encoder_open(&x4->params);
    if (!x4->enc)
        return AVERROR_EXTERNAL;

    if (avctx->flags & AV_CODEC_FLAG_GLOBAL_HEADER) {
        wz264_nal_t *nal;
        uint8_t *p;
        int nnal, s, i;

        s = wz264_encoder_headers(x4->enc, &nal, &nnal);
        avctx->extradata = p = av_mallocz(s + AV_INPUT_BUFFER_PADDING_SIZE);
        if (!p)
            return AVERROR(ENOMEM);

        for (i = 0; i < nnal; i++) {
            /* Don't put the SEI in extradata. */
            if (nal[i].i_type == NAL_SEI) {
                av_log(avctx, AV_LOG_INFO, "%s\n", nal[i].p_payload+25);
                x4->sei_size = nal[i].i_payload;
                x4->sei      = av_malloc(x4->sei_size);
                if (!x4->sei)
                    return AVERROR(ENOMEM);
                memcpy(x4->sei, nal[i].p_payload, nal[i].i_payload);
                continue;
            }
            memcpy(p, nal[i].p_payload, nal[i].i_payload);
            p += nal[i].i_payload;
        }
        avctx->extradata_size = p - avctx->extradata;
    }

    cpb_props = ff_add_cpb_side_data(avctx);
    if (!cpb_props)
        return AVERROR(ENOMEM);
    cpb_props->buffer_size = x4->params.rc.i_vbv_buffer_size * 1000;
    cpb_props->max_bitrate = x4->params.rc.i_vbv_max_bitrate * 1000;
    cpb_props->avg_bitrate = x4->params.rc.i_bitrate         * 1000;

    return 0;
}

static const enum AVPixelFormat pix_fmts_8bit[] = {
    AV_PIX_FMT_YUV420P,
    AV_PIX_FMT_YUVJ420P,
    AV_PIX_FMT_YUV422P,
    AV_PIX_FMT_YUVJ422P,
    AV_PIX_FMT_YUV444P,
    AV_PIX_FMT_YUVJ444P,
    AV_PIX_FMT_NV12,
    AV_PIX_FMT_NV16,
#ifdef WZ264_CSP_NV21
    AV_PIX_FMT_NV21,
#endif
    AV_PIX_FMT_NONE
};
static const enum AVPixelFormat pix_fmts_9bit[] = {
    AV_PIX_FMT_YUV420P9,
    AV_PIX_FMT_YUV444P9,
    AV_PIX_FMT_NONE
};
static const enum AVPixelFormat pix_fmts_10bit[] = {
    AV_PIX_FMT_YUV420P10,
    AV_PIX_FMT_YUV422P10,
    AV_PIX_FMT_YUV444P10,
    AV_PIX_FMT_NV20,
    AV_PIX_FMT_NONE
};
static const enum AVPixelFormat pix_fmts_all[] = {
    AV_PIX_FMT_YUV420P,
    AV_PIX_FMT_YUVJ420P,
    AV_PIX_FMT_YUV422P,
    AV_PIX_FMT_YUVJ422P,
    AV_PIX_FMT_YUV444P,
    AV_PIX_FMT_YUVJ444P,
    AV_PIX_FMT_NV12,
    AV_PIX_FMT_NV16,
#ifdef WZ264_CSP_NV21
    AV_PIX_FMT_NV21,
#endif
    AV_PIX_FMT_YUV420P10,
    AV_PIX_FMT_YUV422P10,
    AV_PIX_FMT_YUV444P10,
    AV_PIX_FMT_NV20,
#ifdef WZ264_CSP_I400
    AV_PIX_FMT_GRAY8,
    AV_PIX_FMT_GRAY10,
#endif
    AV_PIX_FMT_NONE
};
#if CONFIG_LIBWZ264RGB_ENCODER
static const enum AVPixelFormat pix_fmts_8bit_rgb[] = {
    AV_PIX_FMT_BGR0,
    AV_PIX_FMT_BGR24,
    AV_PIX_FMT_RGB24,
    AV_PIX_FMT_NONE
};
#endif

static const enum AVPixelFormat wz264_csp_eight[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_YUVJ420P,
                                                      AV_PIX_FMT_NONE };

static av_cold void wz264_init_static(AVCodec *codec)
{
    codec->pix_fmts = pix_fmts_all;
}

#define OFFSET(x) offsetof(WZ264Context, x)
#define VE AV_OPT_FLAG_VIDEO_PARAM | AV_OPT_FLAG_ENCODING_PARAM
static const AVOption options[] = {
    { "preset",        "Set the encoding preset (cf. wz264 --fullhelp)",   OFFSET(preset),        AV_OPT_TYPE_STRING, { .str = "medium" }, 0, 0, VE},
    { "tune",          "Tune the encoding params (cf. wz264 --fullhelp)",  OFFSET(tune),          AV_OPT_TYPE_STRING, { 0 }, 0, 0, VE},
    { "profile",       "Set profile restrictions (cf. wz264 --fullhelp) ", OFFSET(profile),       AV_OPT_TYPE_STRING, { 0 }, 0, 0, VE},
    { "fastfirstpass", "Use fast settings when encoding first pass",      OFFSET(fastfirstpass), AV_OPT_TYPE_BOOL, { .i64 = 1 }, 0, 1, VE},
    { "level", "Specify level (as defined by Annex A)", OFFSET(level), AV_OPT_TYPE_STRING, {.str=NULL}, 0, 0, VE},
    { "passlogfile", "Filename for 2 pass stats", OFFSET(stats), AV_OPT_TYPE_STRING, {.str=NULL}, 0, 0, VE},
    { "wpredp", "Weighted prediction for P-frames", OFFSET(wpredp), AV_OPT_TYPE_STRING, {.str=NULL}, 0, 0, VE},
    { "a53cc",          "Use A53 Closed Captions (if available)",          OFFSET(a53_cc),        AV_OPT_TYPE_BOOL,   {.i64 = 1}, 0, 1, VE},
    { "wz264opts", "wz264 options", OFFSET(wz264opts), AV_OPT_TYPE_STRING, {.str=NULL}, 0, 0, VE},
    { "crf",           "Visionular quality degree",                       OFFSET(crf),           AV_OPT_TYPE_FLOAT,  {.dbl = -1 }, -1, FLT_MAX, VE },
    { "crf_max",       "In CRF mode, prevents VBV from lowering quality beyond this point.",OFFSET(crf_max), AV_OPT_TYPE_FLOAT, {.dbl = -1 }, -1, FLT_MAX, VE },
    { "qp",            "Constant quantization parameter rate control method",OFFSET(cqp),        AV_OPT_TYPE_INT,    { .i64 = -1 }, -1, INT_MAX, VE },
    { "aq-mode",       "AQ method",                                       OFFSET(aq_mode),       AV_OPT_TYPE_INT,    { .i64 = -1 }, -1, INT_MAX, VE, "aq_mode"},
    { "none",          NULL,                              0, AV_OPT_TYPE_CONST, {.i64 = WZ264_AQ_NONE},         INT_MIN, INT_MAX, VE, "aq_mode" },
    { "variance",      "Variance AQ (complexity mask)",   0, AV_OPT_TYPE_CONST, {.i64 = WZ264_AQ_VARIANCE},     INT_MIN, INT_MAX, VE, "aq_mode" },
    { "autovariance",  "Auto-variance AQ",                0, AV_OPT_TYPE_CONST, {.i64 = WZ264_AQ_AUTOVARIANCE}, INT_MIN, INT_MAX, VE, "aq_mode" },
    { "autovariance-biased", "Auto-variance AQ with bias to dark scenes", 0, AV_OPT_TYPE_CONST, {.i64 = WZ264_AQ_AUTOVARIANCE_BIASED}, INT_MIN, INT_MAX, VE, "aq_mode" },
    { "aq-strength",   "AQ strength. Reduces blocking and blurring in flat and textured areas.", OFFSET(aq_strength), AV_OPT_TYPE_FLOAT, {.dbl = -1}, -1, FLT_MAX, VE},
    { "aq-smooth",     "AQ smooth. ", OFFSET(aq_smooth), AV_OPT_TYPE_FLOAT, {.dbl = -1}, -1, FLT_MAX, VE  },
    { "psy",           "Use psychovisual optimizations.",                 OFFSET(psy),           AV_OPT_TYPE_BOOL,   { .i64 = -1 }, -1, 1, VE },
    { "psy-rd",        "Strength of psychovisual optimization, in <psy-rd>:<psy-trellis> format.", OFFSET(psy_rd), AV_OPT_TYPE_STRING,  {0 }, 0, 0, VE},
    { "rc-lookahead",  "Number of frames to look ahead for frametype and ratecontrol", OFFSET(rc_lookahead), AV_OPT_TYPE_INT, { .i64 = -1 }, -1, INT_MAX, VE },
    { "weightb",       "Weighted prediction for B-frames.",               OFFSET(weightb),       AV_OPT_TYPE_BOOL,   { .i64 = -1 }, -1, 1, VE },
    { "weightp",       "Weighted prediction analysis method.",            OFFSET(weightp),       AV_OPT_TYPE_INT,    { .i64 = -1 }, -1, INT_MAX, VE, "weightp" },
    { "none",          NULL, 0, AV_OPT_TYPE_CONST, {.i64 = WZ264_WEIGHTP_NONE},   INT_MIN, INT_MAX, VE, "weightp" },
    { "simple",        NULL, 0, AV_OPT_TYPE_CONST, {.i64 = WZ264_WEIGHTP_SIMPLE}, INT_MIN, INT_MAX, VE, "weightp" },
    { "smart",         NULL, 0, AV_OPT_TYPE_CONST, {.i64 = WZ264_WEIGHTP_SMART},  INT_MIN, INT_MAX, VE, "weightp" },
    { "ssim",          "Calculate and print SSIM stats.",                 OFFSET(ssim),          AV_OPT_TYPE_BOOL,   { .i64 = -1 }, -1, 1, VE },
    { "intra-refresh", "Use Periodic Intra Refresh instead of IDR frames.",OFFSET(intra_refresh),AV_OPT_TYPE_BOOL,   { .i64 = -1 }, -1, 1, VE },
    { "bluray-compat", "Bluray compatibility workarounds.",               OFFSET(bluray_compat) ,AV_OPT_TYPE_BOOL,   { .i64 = -1 }, -1, 1, VE },
    { "b-bias",        "Influences how often B-frames are used",          OFFSET(b_bias),        AV_OPT_TYPE_INT,    { .i64 = INT_MIN}, INT_MIN, INT_MAX, VE },
    { "b-pyramid",     "Keep some B-frames as references.",               OFFSET(b_pyramid),     AV_OPT_TYPE_INT,    { .i64 = -1 }, -1, INT_MAX, VE, "b_pyramid" },
    { "none",          NULL,                                  0, AV_OPT_TYPE_CONST, {.i64 = WZ264_B_PYRAMID_NONE},   INT_MIN, INT_MAX, VE, "b_pyramid" },
    { "strict",        "Strictly hierarchical pyramid",       0, AV_OPT_TYPE_CONST, {.i64 = WZ264_B_PYRAMID_STRICT}, INT_MIN, INT_MAX, VE, "b_pyramid" },
    { "normal",        "Non-strict (not Blu-ray compatible)", 0, AV_OPT_TYPE_CONST, {.i64 = WZ264_B_PYRAMID_NORMAL}, INT_MIN, INT_MAX, VE, "b_pyramid" },
    { "mixed-refs",    "One reference per partition, as opposed to one reference per macroblock", OFFSET(mixed_refs), AV_OPT_TYPE_BOOL, { .i64 = -1}, -1, 1, VE },
    { "8x8dct",        "High profile 8x8 transform.",                     OFFSET(dct8x8),        AV_OPT_TYPE_BOOL,   { .i64 = -1 }, -1, 1, VE},
    { "fast-pskip",    NULL,                                              OFFSET(fast_pskip),    AV_OPT_TYPE_BOOL,   { .i64 = -1 }, -1, 1, VE},
    { "aud",           "Use access unit delimiters.",                     OFFSET(aud),           AV_OPT_TYPE_BOOL,   { .i64 = -1 }, -1, 1, VE},
    { "mbtree",        "Use macroblock tree ratecontrol.",                OFFSET(mbtree),        AV_OPT_TYPE_BOOL,   { .i64 = -1 }, -1, 1, VE},
    { "deblock",       "Loop filter parameters, in <alpha:beta> form.",   OFFSET(deblock),       AV_OPT_TYPE_STRING, { 0 },  0, 0, VE},
    { "cplxblur",      "Reduce fluctuations in QP (before curve compression)", OFFSET(cplxblur), AV_OPT_TYPE_FLOAT,  {.dbl = -1 }, -1, FLT_MAX, VE},
    { "partitions",    "A comma-separated list of partitions to consider. "
                       "Possible values: p8x8, p4x4, b8x8, i8x8, i4x4, none, all", OFFSET(partitions), AV_OPT_TYPE_STRING, { 0 }, 0, 0, VE},
    { "direct-pred",   "Direct MV prediction mode",                       OFFSET(direct_pred),   AV_OPT_TYPE_INT,    { .i64 = -1 }, -1, INT_MAX, VE, "direct-pred" },
    { "none",          NULL,      0,    AV_OPT_TYPE_CONST, { .i64 = WZ264_DIRECT_PRED_NONE },     0, 0, VE, "direct-pred" },
    { "spatial",       NULL,      0,    AV_OPT_TYPE_CONST, { .i64 = WZ264_DIRECT_PRED_SPATIAL },  0, 0, VE, "direct-pred" },
    { "temporal",      NULL,      0,    AV_OPT_TYPE_CONST, { .i64 = WZ264_DIRECT_PRED_TEMPORAL }, 0, 0, VE, "direct-pred" },
    { "auto",          NULL,      0,    AV_OPT_TYPE_CONST, { .i64 = WZ264_DIRECT_PRED_AUTO },     0, 0, VE, "direct-pred" },
    { "slice-max-size","Limit the size of each slice in bytes",           OFFSET(slice_max_size),AV_OPT_TYPE_INT,    { .i64 = -1 }, -1, INT_MAX, VE },
    { "stats",         "Filename for 2 pass stats",                       OFFSET(stats),         AV_OPT_TYPE_STRING, { 0 },  0,       0, VE },
    { "nal-hrd",       "Signal HRD information (requires vbv-bufsize; "
                       "cbr not allowed in .mp4)",                        OFFSET(nal_hrd),       AV_OPT_TYPE_INT,    { .i64 = -1 }, -1, INT_MAX, VE, "nal-hrd" },
    { "none",          NULL, 0, AV_OPT_TYPE_CONST, {.i64 = WZ264_NAL_HRD_NONE}, INT_MIN, INT_MAX, VE, "nal-hrd" },
    { "vbr",           NULL, 0, AV_OPT_TYPE_CONST, {.i64 = WZ264_NAL_HRD_VBR},  INT_MIN, INT_MAX, VE, "nal-hrd" },
    { "cbr",           NULL, 0, AV_OPT_TYPE_CONST, {.i64 = WZ264_NAL_HRD_CBR},  INT_MIN, INT_MAX, VE, "nal-hrd" },
    { "avcintra-class","AVC-Intra class 50/100/200",                      OFFSET(avcintra_class),AV_OPT_TYPE_INT,     { .i64 = -1 }, -1, 200   , VE},
    { "me_method",    "Set motion estimation method",                     OFFSET(motion_est),    AV_OPT_TYPE_INT,    { .i64 = -1 }, -1, WZ264_ME_TESA, VE, "motion-est"},
    { "motion-est",   "Set motion estimation method",                     OFFSET(motion_est),    AV_OPT_TYPE_INT,    { .i64 = -1 }, -1, WZ264_ME_TESA, VE, "motion-est"},
    { "dia",           NULL, 0, AV_OPT_TYPE_CONST, { .i64 = WZ264_ME_DIA },  INT_MIN, INT_MAX, VE, "motion-est" },
    { "hex",           NULL, 0, AV_OPT_TYPE_CONST, { .i64 = WZ264_ME_HEX },  INT_MIN, INT_MAX, VE, "motion-est" },
    { "umh",           NULL, 0, AV_OPT_TYPE_CONST, { .i64 = WZ264_ME_UMH },  INT_MIN, INT_MAX, VE, "motion-est" },
    { "esa",           NULL, 0, AV_OPT_TYPE_CONST, { .i64 = WZ264_ME_ESA },  INT_MIN, INT_MAX, VE, "motion-est" },
    { "tesa",          NULL, 0, AV_OPT_TYPE_CONST, { .i64 = WZ264_ME_TESA }, INT_MIN, INT_MAX, VE, "motion-est" },
    { "forced-idr",   "If forcing keyframes, force them as IDR frames.",      OFFSET(forced_idr),  AV_OPT_TYPE_BOOL,   { .i64 = 0 }, -1, 1, VE },
    { "coder",        "Coder type",                                           OFFSET(coder), AV_OPT_TYPE_INT, { .i64 = -1 }, -1, 1, VE, "coder" },
    { "default",          NULL, 0, AV_OPT_TYPE_CONST, { .i64 = -1 }, INT_MIN, INT_MAX, VE, "coder" },
    { "cavlc",            NULL, 0, AV_OPT_TYPE_CONST, { .i64 = 0 },  INT_MIN, INT_MAX, VE, "coder" },
    { "cabac",            NULL, 0, AV_OPT_TYPE_CONST, { .i64 = 1 },  INT_MIN, INT_MAX, VE, "coder" },
    { "vlc",              NULL, 0, AV_OPT_TYPE_CONST, { .i64 = 0 },  INT_MIN, INT_MAX, VE, "coder" },
    { "ac",               NULL, 0, AV_OPT_TYPE_CONST, { .i64 = 1 },  INT_MIN, INT_MAX, VE, "coder" },
    { "b_strategy",      "Strategy to choose between I/P/B-frames",       OFFSET(b_frame_strategy), AV_OPT_TYPE_INT, { .i64 = -1 }, -1, 2, VE },
    { "chromaoffset",    "QP difference between chroma and luma",         OFFSET(chroma_offset), AV_OPT_TYPE_INT, { .i64 = -1 }, INT_MIN, INT_MAX, VE },
    { "sc_threshold",    "Scene change threshold",                        OFFSET(scenechange_threshold), AV_OPT_TYPE_INT, { .i64 = -1 }, INT_MIN, INT_MAX, VE },
    { "noise_reduction", "Noise reduction",                               OFFSET(noise_reduction), AV_OPT_TYPE_INT, { .i64 = -1 }, INT_MIN, INT_MAX, VE },

    { "wz264-params",  "Override the wz264 configuration using a :-separated list of key=value parameters", OFFSET(wz264_params), AV_OPT_TYPE_STRING, { 0 }, 0, 0, VE },
    { NULL },
};

static const AVCodecDefault wz264_defaults[] = {
    { "b",                "0" },
    { "bf",               "-1" },
    { "flags2",           "0" },
    { "g",                "-1" },
    { "i_qfactor",        "-1" },
    { "b_qfactor",        "-1" },
    { "qmin",             "-1" },
    { "qmax",             "-1" },
    { "qdiff",            "-1" },
    { "qblur",            "-1" },
    { "qcomp",            "-1" },
//     { "rc_lookahead",     "-1" },
    { "refs",             "-1" },
#if FF_API_PRIVATE_OPT
    { "sc_threshold",     "-1" },
#endif
    { "trellis",          "-1" },
#if FF_API_PRIVATE_OPT
    { "nr",               "-1" },
#endif
    { "me_range",         "-1" },
    { "subq",             "-1" },
#if FF_API_PRIVATE_OPT
    { "b_strategy",       "-1" },
#endif
    { "keyint_min",       "-1" },
#if FF_API_CODER_TYPE
    { "coder",            "-1" },
#endif
    { "cmp",              "-1" },
    { "threads",          AV_STRINGIFY(WZ264_THREADS_AUTO) },
    { "thread_type",      "0" },
    { "flags",            "+cgop" },
    { "rc_init_occupancy","-1" },
    { NULL },
};

static const AVClass wz264_class = {
    .class_name = "libwz264",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_libwz264_encoder = {
    .name             = "libwz264",
    .long_name        = NULL_IF_CONFIG_SMALL("libwz264 H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10"),
    .type             = AVMEDIA_TYPE_VIDEO,
    .id               = AV_CODEC_ID_H264,
    .priv_data_size   = sizeof(WZ264Context),
    .init             = WZ264_init,
    .encode2          = WZ264_frame,
    .close            = WZ264_close,
    .capabilities     = AV_CODEC_CAP_DELAY | AV_CODEC_CAP_AUTO_THREADS,
    .priv_class       = &wz264_class,
    .defaults         = wz264_defaults,
    .init_static_data = wz264_init_static,
    .caps_internal    = FF_CODEC_CAP_INIT_CLEANUP,
    .wrapper_name     = "libwz264",
};


#if CONFIG_LIBWZ264RGB_ENCODER
static const AVClass rgbclass = {
    .class_name = "libwz264rgb",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_libwz264rgb_encoder = {
    .name           = "libwz264rgb",
    .long_name      = NULL_IF_CONFIG_SMALL("libwz264 H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10 RGB"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_H264,
    .priv_data_size = sizeof(WZ264Context),
    .init           = WZ264_init,
    .encode2        = WZ264_frame,
    .close          = WZ264_close,
    .capabilities   = AV_CODEC_CAP_DELAY | AV_CODEC_CAP_AUTO_THREADS,
    .priv_class     = &rgbclass,
    .defaults       = wz264_defaults,
    .pix_fmts       = pix_fmts_8bit_rgb,
    .wrapper_name   = "libwz264",
};
#endif

#if CONFIG_LIBWZ262_ENCODER
static const AVClass WZ262_class = {
    .class_name = "libwz262",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_libwz262_encoder = {
    .name             = "libwz262",
    .long_name        = NULL_IF_CONFIG_SMALL("libwz262 MPEG2VIDEO"),
    .type             = AVMEDIA_TYPE_VIDEO,
    .id               = AV_CODEC_ID_MPEG2VIDEO,
    .priv_data_size   = sizeof(WZ264Context),
    .init             = WZ264_init,
    .encode2          = WZ264_frame,
    .close            = WZ264_close,
    .capabilities     = AV_CODEC_CAP_DELAY | AV_CODEC_CAP_AUTO_THREADS,
    .priv_class       = &WZ262_class,
    .defaults         = wz264_defaults,
    .pix_fmts         = pix_fmts_8bit,
    .caps_internal    = FF_CODEC_CAP_INIT_CLEANUP,
    .wrapper_name     = "libwz264",
};
#endif

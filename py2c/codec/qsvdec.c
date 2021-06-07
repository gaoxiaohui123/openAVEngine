/*
 * Copyright (c) 2015 Anton Khirnov
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @file
 * Intel QSV-accelerated H.264 decoding example.
 *
 * @example qsvdec.c
 * This example shows how to do QSV-accelerated H.264 decoding with output
 * frames in the GPU video surfaces.
 */

#include "config.h"

#include <stdio.h>

#if 0
#include "libavformat/avformat.h"
#include "libavformat/avio.h"

#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/buffer.h"
#include "libavutil/error.h"
#include "libavutil/hwcontext.h"
#include "libavutil/hwcontext_qsv.h"
#include "libavutil/mem.h"
#else
#include "hcsvc.h"
#include "inc.h"
#endif
//typedef struct DecodeContext {
//    AVBufferRef *hw_device_ref;
//} DecodeContext;

static int get_format(AVCodecContext *avctx, const enum AVPixelFormat *pix_fmts)
{
    while (*pix_fmts != AV_PIX_FMT_NONE) {
        if (*pix_fmts == AV_PIX_FMT_QSV) {
            //DecodeContext *decode = avctx->opaque;
            AVBufferRef *hw_device_ctx = avctx->opaque;
            AVHWFramesContext  *frames_ctx;
            AVQSVFramesContext *frames_hwctx;
            int ret;

            /* create a pool of surfaces to be used by the decoder */
            avctx->hw_frames_ctx = av_hwframe_ctx_alloc(hw_device_ctx);
            if (!avctx->hw_frames_ctx)
                return AV_PIX_FMT_NONE;
            frames_ctx   = (AVHWFramesContext*)avctx->hw_frames_ctx->data;
            frames_hwctx = frames_ctx->hwctx;

            frames_ctx->format            = AV_PIX_FMT_QSV;
            frames_ctx->sw_format         = avctx->sw_pix_fmt;
            frames_ctx->width             = FFALIGN(avctx->coded_width,  32);
            frames_ctx->height            = FFALIGN(avctx->coded_height, 32);
            frames_ctx->initial_pool_size = 32;

            printf("get_format: frames_ctx->width= %d \n", frames_ctx->width);
            printf("get_format: frames_ctx->height= %d \n", frames_ctx->height);

            frames_hwctx->frame_type = MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET;

            ret = av_hwframe_ctx_init(avctx->hw_frames_ctx);
            if (ret < 0)
                return AV_PIX_FMT_NONE;

            return AV_PIX_FMT_QSV;
        }

        pix_fmts++;
    }

    fprintf(stderr, "The QSV pixel format not offered in get_format()\n");

    return AV_PIX_FMT_NONE;
}

static int decode_packet(struct SwsContext **scale,
                         AVCodecContext *decoder_ctx,
                         AVFrame *hw_frame, AVFrame *sw_frame,
                         AVPacket *pkt, AVIOContext *output_ctx,
                         uint8_t *outbuf, int *p_width, int *p_height)
{
    int ret = 0;

    ret = avcodec_send_packet(decoder_ctx, pkt);
    if (ret < 0) {
        fprintf(stderr, "decode_packet: Error during decoding\n");
        return ret;
    }

    while (ret >= 0) {
        int i, j;

        ret = avcodec_receive_frame(decoder_ctx, hw_frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            break;
        }
        else if (ret < 0) {
            fprintf(stderr, "Error during decoding\n");
            return ret;
        }
        //printf("decode_packet: hw_frame->linesize[0]= %d \n", hw_frame->linesize[0]);
        //printf("decode_packet: hw_frame->linesize[1]= %d \n", hw_frame->linesize[1]);
        //printf("decode_packet: hw_frame->linesize[2]= %d \n", hw_frame->linesize[2]);
        /* A real program would do something useful with the decoded frame here.
         * We just retrieve the raw data and write it to a file, which is rather
         * useless but pedagogic. */
        ret = av_hwframe_transfer_data(sw_frame, hw_frame, 0);// AV_PIX_FMT_QSV到 AV_PIX_FMT_NV12
        if (ret < 0) {
            fprintf(stderr, "Error transferring the data to system memory\n");
            goto fail;
        }
        if(output_ctx)
        {
            for (i = 0; i < FF_ARRAY_ELEMS(sw_frame->data) && sw_frame->data[i]; i++)
                for (j = 0; j < (sw_frame->height >> (i > 0)); j++)
                    avio_write(output_ctx, sw_frame->data[i] + j * sw_frame->linesize[i], sw_frame->width);
        }
        int w = sw_frame->width;
        int h = sw_frame->height;
        if(p_width)
        {
            p_width[0] = w;
        }
        if(p_height)
        {
            p_height[0] = h;
        }

        if(outbuf)
        {
#if 1
            int size = w * h;
            memcpy(outbuf, sw_frame->data[0], size);
            uint8_t *u_data = (uint8_t *)&outbuf[size];
            uint8_t *v_data = &u_data[(size >> 2)];
            uint16_t *p = (uint16_t *)sw_frame->data[1];
            int offset = 0;
            for(int i = 0; i < (size >> 1); i += 2)
            {
                //for(int j = 0; j < width; j+=2)
                {
                    uint16_t v = p[offset];
                    u_data[offset] = v & 0xFF;
                    v_data[offset] = (v >> 8) & 0xFF;
                    offset++;
                }
            }
#else
            if(scale)
            {
                int size = w * h;
                if(!*scale)
                {
                    *scale = sws_getContext(  sw_frame->width,
	                                          sw_frame->height,
	                                          sw_frame->format,
		                                      w,
		                                      h,
		                                      AV_PIX_FMT_YUV420P,
		                                      SWS_FAST_BILINEAR,
		                                      NULL, NULL, NULL);
                }
                if(*scale)
                {
                    AVFrame tmp_frame = {};//注意：初始化至关重要，否则会导致异常崩溃
                    tmp_frame.data[0] = outbuf;
                    tmp_frame.data[1] = &outbuf[size];
                    tmp_frame.data[2] = &outbuf[size + (size >> 2)];
                    tmp_frame.linesize[0] = w;
                    tmp_frame.linesize[1] = w >> 1;
                    tmp_frame.linesize[2] = w >> 1;
                    tmp_frame.width = w;
		            tmp_frame.height = h;
                    tmp_frame.format = AV_PIX_FMT_YUV420P;
                    sws_scale(  *scale,
		                        sw_frame->data,
		        		        sw_frame->linesize,
		        		        0,
		        		        h,
		        		        tmp_frame.data,
		        		        tmp_frame.linesize);
                }
            }
#endif
        }
        //printf("decode_packet: sw_frame->linesize[0]= %d \n", sw_frame->linesize[0]);
        //printf("decode_packet: sw_frame->linesize[1]= %d \n", sw_frame->linesize[1]);
        //printf("decode_packet: sw_frame->linesize[2]= %d \n", sw_frame->linesize[2]);
        //printf("decode_packet: sw_frame->width= %d \n", sw_frame->width);
        //printf("decode_packet: sw_frame->height= %d \n", sw_frame->height);

fail:
        av_frame_unref(sw_frame);
        av_frame_unref(hw_frame);

        if (ret < 0)
            return ret;
    }

    return 0;
}
#define HCSVC_API __attribute__ ((__visibility__("default")))

extern AVCodec *find_codec_by_name(char *codec_name, int is_encoder);
int decode_init(AVCodec *decoder,
                AVFormatContext **p_input_ctx,
                AVStream **p_video_st,
                AVCodecContext **p_decoder_ctx,
                AVFrame **p_hw_frame,
                AVFrame **p_sw_frame,
                AVIOContext **p_output_ctx,
                AVBufferRef **p_hw_device_ctx,
                char *codec_name,
                char *infile, char *outfile)
{
    int ret = 0;
    AVBufferRef *hw_device_ctx = NULL;
    AVFormatContext *input_ctx = NULL;
    AVStream *video_st = NULL;
    AVCodecContext *decoder_ctx = NULL;
    AVFrame *hw_frame = NULL, *sw_frame = NULL;
    //DecodeContext decode = { NULL };
    AVIOContext *output_ctx = NULL;

    //int codec_id = AV_CODEC_ID_H265;
    unsigned int codec_id = AV_CODEC_ID_H264;
    //hevc_cuvid
    if( !strcmp(codec_name, "hevc_qsv") ||
        !strcmp(codec_name, "hevc_vaapi") ||
        !strcmp(codec_name, "hevc_cuvid"))
    {
        //argv[1] = "/home/gxh/works/out.265";
        codec_id = AV_CODEC_ID_H265;
    }

    printf("decode_init: codec_name=%s \n", codec_name);

    if(infile)
    {
        /* open the input file */
        ret = avformat_open_input(&input_ctx, infile, NULL, NULL);
        if (ret < 0) {
            printf("decode_init: avformat_open_input: ret=%d \n", ret);
            return ret;
        }

        /* find the first H.264 video stream */
        for (int i = 0; i < input_ctx->nb_streams; i++) {
            AVStream *st = input_ctx->streams[i];

            if (st->codecpar->codec_id == codec_id && !video_st)
                video_st = st;
            else
                st->discard = AVDISCARD_ALL;
        }
        if (!video_st) {
            printf("decode_init: video_st=%x \n", video_st);
            return ret;
        }
    }


    /* open the hardware device */
    ret = av_hwdevice_ctx_create(&hw_device_ctx, AV_HWDEVICE_TYPE_QSV,
                                 "auto", NULL, 0);
    if (ret < 0) {
        printf("decode_init: av_hwdevice_ctx_create: ret=%d \n", ret);
        return ret;
    }
    printf("decode_init: hw_device_ctx=%x \n", hw_device_ctx);

    decoder_ctx = avcodec_alloc_context3(decoder);
    if (!decoder_ctx) {
        printf("decode_init: avcodec_alloc_context3: decoder_ctx=%x \n", decoder_ctx);
        return ret;
    }
    decoder_ctx->codec_id = codec_id;

    printf("decode_init: codec_id=%d \n", codec_id);

    if(video_st)
    {
        if (video_st->codecpar->extradata_size) {
            decoder_ctx->extradata = av_mallocz(video_st->codecpar->extradata_size +
                                                AV_INPUT_BUFFER_PADDING_SIZE);
            if (!decoder_ctx->extradata) {
                ret = AVERROR(ENOMEM);
                printf("decode_init: av_mallocz: ret=%d \n", ret);
            }
            memcpy(decoder_ctx->extradata, video_st->codecpar->extradata,
                   video_st->codecpar->extradata_size);
            decoder_ctx->extradata_size = video_st->codecpar->extradata_size;
        }
    }


    decoder_ctx->opaque      = hw_device_ctx;
    decoder_ctx->get_format  = get_format;

    ret = avcodec_open2(decoder_ctx, NULL, NULL);
    if (ret < 0) {
        printf("decode_init: avcodec_open2: ret=%d \n", ret);
        return ret;
    }
    printf("decode_init: ret=%d \n", ret);
    if(outfile)
    {
        /* open the output stream */
        ret = avio_open(&output_ctx, outfile, AVIO_FLAG_WRITE);
        if (ret < 0) {
            printf("decode_init: avio_open: ret=%d \n", ret);
            return ret;
        }
    }
    hw_frame    = av_frame_alloc();
    sw_frame = av_frame_alloc();
    if (!hw_frame || !sw_frame) {
        ret = AVERROR(ENOMEM);
        printf("decode_init: av_frame_alloc: ret=%d \n", ret);
        return ret;
    }
    *p_input_ctx = input_ctx;
    *p_video_st = video_st;
    *p_decoder_ctx = decoder_ctx;
    *p_hw_frame = hw_frame;
    *p_sw_frame = sw_frame;
    *p_output_ctx = output_ctx;
    *p_hw_device_ctx = hw_device_ctx;
    return ret;
}
int hw_decode_one_frame(struct SwsContext **scale, AVCodecContext *c, AVPacket *pkt, AVFrame *hw_frame, AVFrame *sw_frame, uint8_t *outbuf)
{
    int width = 0, height = 0;
    int ret = decode_packet(scale, c, hw_frame, sw_frame, pkt, NULL, outbuf, &width, &height);
    if(!ret)
    {
        ret = (width * height * 3) >> 1;
        if(!ret)
        {
            printf("hw_decode_one_frame: ret=%d \n", ret);
        }
    }

    return ret;
}
int hw_decode_close(AVFormatContext *input_ctx,
                    AVStream *video_st,
                    AVCodecContext *decoder_ctx,
                    AVFrame *hw_frame,
                    AVFrame *sw_frame,
                    AVIOContext *output_ctx,
                    AVBufferRef *hw_device_ctx)
{
    int ret = 0;
    AVPacket pkt = { 0 };
    /* flush the decoder */
    pkt.data = NULL;
    pkt.size = 0;
    if(output_ctx && decoder_ctx)
        ret = decode_packet(NULL, decoder_ctx, hw_frame, sw_frame, &pkt, output_ctx, NULL, NULL, NULL);

    if (ret < 0) {
        char buf[1024];
        av_strerror(ret, buf, sizeof(buf));
        fprintf(stderr, "%s\n", buf);
    }
    if(input_ctx)
    {
        avformat_close_input(&input_ctx);
    }
    if(hw_frame)
    {
        av_frame_free(&hw_frame);
    }
    if(sw_frame)
    {
        av_frame_free(&sw_frame);
    }
    if(decoder_ctx)
    {
        avcodec_free_context(&decoder_ctx);
    }
    if(hw_device_ctx)
    {
        av_buffer_unref(&hw_device_ctx);
    }
    if(output_ctx)
    {
        avio_close(output_ctx);
    }
    return ret;
}
HCSVC_API
int api_vaapi_decode_test(char *infile, char *outfile, char *codec_name, int loopn)
{
    AVBufferRef *hw_device_ctx = NULL;
    AVFormatContext *input_ctx = NULL;
    AVStream *video_st = NULL;
    AVCodecContext *decoder_ctx = NULL;
    const AVCodec *decoder;
    AVFrame *hw_frame = NULL;
    AVFrame *sw_frame = NULL;
    AVIOContext *output_ctx = NULL;
    int width = 0, height = 0;

    AVPacket pkt = { 0 };
    //DecodeContext decode = { NULL };
    int ret, i;

    decoder = find_codec_by_name(codec_name, 0);
    if(!decoder)
    {
        fprintf(stderr, "The QSV decoder is not present in libavcodec\n");
        return ret;
    }

    ret = decode_init(  decoder,
                        &input_ctx,
                        &video_st,
                        &decoder_ctx,
                        &hw_frame,
                        &sw_frame,
                        &output_ctx,
                        &hw_device_ctx,
                        codec_name,
                        infile, outfile);
    if(ret < 0)
    {
        fprintf(stderr, "decode_init \n");
        return ret;
    }

    int frame_num = 0;
    /* actual decoding */
    while (ret >= 0) {
        ret = av_read_frame(input_ctx, &pkt);
        if (ret < 0)
            break;

        if (pkt.stream_index == video_st->index)
            ret = decode_packet(NULL, decoder_ctx, hw_frame, sw_frame, &pkt, output_ctx, NULL, &width, &height);

        av_packet_unref(&pkt);
        frame_num++;
        printf("api_vaapi_decode_test: frame_num=%d \n", frame_num);
    }

    hw_decode_close(input_ctx,
                    video_st,
                    decoder_ctx,
                    hw_frame,
                    sw_frame,
                    output_ctx,
                    hw_device_ctx);

    return frame_num;
}

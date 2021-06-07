/*
 * Video Acceleration API (video encoding) encode sample
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
 * Intel VAAPI-accelerated encoding example.
 *
 * @example vaapi_encode.c
 * This example shows how to do VAAPI-accelerated encoding. now only support NV12
 * raw file, usage like: vaapi_encode 1920 1080 input.yuv output.h264
 *
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#if 0
#include "libavcodec/avcodec.h"
#include "libavutil/pixdesc.h"
#include "libavutil/hwcontext.h"

#include "libavutil/avutil.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "libavutil/time.h"
#else
#include "hcsvc.h"
#include "inc.h"
#endif

//static int width, height;
//static AVBufferRef *hw_device_ctx = NULL;

static int set_hwframe_ctx(AVCodecContext *ctx, AVBufferRef *hw_device_ctx, int width, int height)
{
    AVBufferRef *hw_frames_ref;
    AVHWFramesContext *frames_ctx = NULL;
    int err = 0;

    if (!(hw_frames_ref = av_hwframe_ctx_alloc(hw_device_ctx))) {
        fprintf(stderr, "Failed to create VAAPI frame context.\n");
        return -1;
    }
    frames_ctx = (AVHWFramesContext *)(hw_frames_ref->data);
    //frames_ctx->format    = AV_PIX_FMT_VAAPI;
    frames_ctx->format    = ctx->pix_fmt;
    frames_ctx->sw_format = AV_PIX_FMT_NV12;
    frames_ctx->width     = width;
    frames_ctx->height    = height;
    frames_ctx->initial_pool_size = 20;
    if ((err = av_hwframe_ctx_init(hw_frames_ref)) < 0) {
        fprintf(stderr, "Failed to initialize VAAPI frame context."
                "Error code: %s\n",av_err2str(err));
        av_buffer_unref(&hw_frames_ref);
        return err;
    }
    ctx->hw_frames_ctx = av_buffer_ref(hw_frames_ref);
    if (!ctx->hw_frames_ctx)
        err = AVERROR(ENOMEM);

    av_buffer_unref(&hw_frames_ref);
    return err;
}

static int encode_write(AVCodecContext *avctx, AVFrame *frame, FILE *fout, uint8_t *outbuf, int *psize)
{
    int ret = 0;
    int offset = 0;
    AVPacket enc_pkt;

    av_init_packet(&enc_pkt);
    enc_pkt.data = NULL;
    enc_pkt.size = 0;

    if ((ret = avcodec_send_frame(avctx, frame)) < 0) {
        fprintf(stderr, "encode_write: Error code: %s\n", av_err2str(ret));
        goto end;
    }
    while (1) {
        ret = avcodec_receive_packet(avctx, &enc_pkt);

        if (ret)
        {
            if(!offset)
            {
                printf("encode_write: ret=%d \n", ret);
            }

            break;
        }
        int ret2 = 0;
        enc_pkt.stream_index = 0;
        if(fout)
        {
            ret2 = fwrite(enc_pkt.data, enc_pkt.size, 1, fout);
        }
        if(outbuf)
        {
            ret2 = enc_pkt.size;
            memcpy(&outbuf[offset], enc_pkt.data, ret2);
        }
        offset += ret2;
        //printf("encode_write: enc_pkt.size=%d \n", enc_pkt.size);
        av_packet_unref(&enc_pkt);
    }

end:
    ret = ((ret == AVERROR(EAGAIN)) ? 0 : -1);
    //printf("encode_write: offset=%d \n", offset);
    if(psize)
    {
        psize[0] = offset;
    }

    return ret;
}
#define HCSVC_API __attribute__ ((__visibility__("default")))

AVCodec *find_codec_by_name(char *codec_name, int is_encoder)
{
    AVCodec *codec = NULL;
    if(is_encoder)
    {
        codec = avcodec_find_encoder_by_name(codec_name);
    }
    else{
        codec = avcodec_find_decoder_by_name(codec_name);
    }

    return codec;
}
int hw_encode_init(AVCodecContext *params, AVCodec *codec, AVBufferRef **p_hw_device_ctx, AVCodecContext **p_c,
                char *enc_name, int width, int height)
{
    int ret = 0;
    AVBufferRef *hw_device_ctx = NULL;
    AVCodecContext *c = NULL;
    int HW_TYPE = AV_HWDEVICE_TYPE_QSV;
    printf("hw_encode_init: start \n");
    printf("hw_encode_init: enc_name=%x \n", enc_name);
    printf("hw_encode_init: enc_name=%s \n", enc_name);

    if(!strcmp(enc_name, "h264_vaapi") || !strcmp(enc_name, "hevc_vaapi"))
    {
        HW_TYPE   = AV_HWDEVICE_TYPE_VAAPI;
    }
    else if(!strcmp(enc_name, "h264_qsv") || !strcmp(enc_name, "hevc_qsv"))
    {
        HW_TYPE   = AV_HWDEVICE_TYPE_QSV;
    }
    else if(!strcmp(enc_name, "h264_nvenc") || !strcmp(enc_name, "hevc_nvenc"))
    {
        HW_TYPE   = AV_HWDEVICE_TYPE_CUDA;//注意:N卡使用AV_PIX_FMT_CUDA
    }
    printf("hw_encode_init: start av_hwdevice_ctx_create \n");
    ret = av_hwdevice_ctx_create(&hw_device_ctx, HW_TYPE, "auto", NULL, 0);
    printf("hw_encode_init: start av_hwdevice_ctx_create: ret=%d \n", ret);
    if(ret < 0)
    {
        fprintf(stderr, "codec_init: av_hwdevice_ctx_create: Error code: %s\n", av_err2str(ret));
        return ret;
    }
    if (!(c = avcodec_alloc_context3(codec))) {
        printf("codec_init: avcodec_alloc_context3: c=%x \n", c);
        return -1;
    }
    if(!strcmp(enc_name, "h264_vaapi") || !strcmp(enc_name, "hevc_vaapi"))
    {
        c->pix_fmt   = AV_PIX_FMT_VAAPI;
    }
    else if(!strcmp(enc_name, "h264_qsv") || !strcmp(enc_name, "hevc_qsv"))
    {
        c->pix_fmt   = AV_PIX_FMT_QSV;
    }
    else if(!strcmp(enc_name, "h264_nvenc") || !strcmp(enc_name, "hevc_nvenc"))
    {
        c->pix_fmt   = AV_PIX_FMT_CUDA;//注意:N卡使用AV_PIX_FMT_CUDA
    }
    //printf("api_vaapi_encode_test: width=%d, height=%d \n", width, height);
    //c->b_repeat_headers = 1;
    //c->flags |= AV_CODEC_FLAG2_LOCAL_HEADER;
    if(params)
    {
        c->width     = params->width;
        c->height    = params->height;
        c->time_base = params->time_base;//(AVRational){1, 25};
        c->framerate = params->framerate;//(AVRational){25, 1};
        c->sample_aspect_ratio = params->sample_aspect_ratio;//(AVRational){1, 1};
        //c->pix_fmt   = AV_PIX_FMT_VAAPI;
        //c->pix_fmt   = params->pix_fmt;//AV_PIX_FMT_QSV;//注意:N卡使用AV_PIX_FMT_CUDA
        //c->pix_fmt   = AV_PIX_FMT_YUV420P;
        //
        c->bit_rate = params->bit_rate;//1024000;
        printf("hw_encode_init: params->bit_rate=%d \n", params->bit_rate);
        printf("hw_encode_init: params->width=%d \n", params->width);
        printf("hw_encode_init: params->height=%d \n", params->height);
        if(!strcmp(enc_name, "h264_vaapi") || !strcmp(enc_name, "h264_qsv"))
        {
            printf("hw_encode_init: enc_name=%s \n", enc_name);
            av_opt_set_int(c->priv_data, "async_depth", 1, 0);
            av_opt_set_int(c->priv_data, "idr_interval", c->gop_size, 0);
            av_opt_set_int(c->priv_data, "bitrate_limit", c->bit_rate, 0);
            av_opt_set_int(c->priv_data, "int_ref_qp_delta", 0, 0);
            av_opt_set(c->priv_data, "profile", "main", 0);
            av_opt_set(c->priv_data, "preset", "veryfast", 0);//veryslow//veryfast//medium
            av_opt_set(c->priv_data, "tune", "zerolatency", 0);
            //注意:早期的elecard不支持high profile,可用264visa分析流;
            //或者控制profile为baseline或main profile
            int slice_max_size = 1100;//MTU_SIZE
            av_opt_get_int(params->priv_data, "max_slice_size", 0, &slice_max_size);
            av_opt_set_int(c->priv_data, "max_slice_size", slice_max_size, 0);//max_slice_size
            printf("hw_encode_init: slice_max_size=%d \n", slice_max_size);

            c->profile = params->profile;//FF_PROFILE_H264_MAIN;//FF_PROFILE_H264_BASELINE;//
            c->gop_size = params->gop_size;//50;
            c->keyint_min = c->gop_size;
            c->max_b_frames = params->max_b_frames;//1;
            char *cpreset = NULL;
            av_opt_get(params->priv_data, "preset", 0, &cpreset);
            if(cpreset)
            {
                //av_opt_set(c->priv_data, "preset", cpreset, 0);
                //av_free(cpreset);//???
                printf("hw_encode_init: cpreset=%s \n", cpreset);
            }
            else{
                //av_opt_set(c->priv_data, "preset", "veryfast", 0);//veryslow//veryfast//medium
            }
            char *ctune = NULL;
            av_opt_get(params->priv_data, "tune", 0, &ctune);
            if(ctune)
            {
                av_opt_set(c->priv_data, "preset", ctune, 0);
                //av_free(ctune);//???
                printf("hw_encode_init: ctune=%s \n", ctune);
            }
            else{
                av_opt_set(c->priv_data, "tune", "zerolatency", 0);
            }
            c->coder_type = params->coder_type;//FF_CODER_TYPE_AC;//FF_CODER_TYPE_VLC;//FF_CODER_TYPE_AC;// : FF_CODER_TYPE_VLC;
            c->qmax = params->qmax;//40;
	        c->qmin = params->qmin;//20;

        }
    }
    else{
        c->width     = width;
        c->height    = height;
        c->time_base = (AVRational){1, 25};
        c->framerate = (AVRational){25, 1};
        c->sample_aspect_ratio = (AVRational){1, 1};
        //
        //c->pix_fmt   = AV_PIX_FMT_YUV420P;
        //
        c->bit_rate = 3300 * 1000;//2048000;//1024000;
        if(!strcmp(enc_name, "h264_vaapi") || !strcmp(enc_name, "h264_qsv"))
        {
            printf("hw_encode_init: enc_name=%s \n", enc_name);
            //注意:早期的elecard不支持high profile,可用264visa分析流;
            //或者控制profile为baseline或main profile
            int slice_max_size = 1100;//MTU_SIZE
            av_opt_set_int(c->priv_data, "max_slice_size", slice_max_size, 0);//max_slice_size

            c->profile = FF_PROFILE_H264_MAIN;//FF_PROFILE_H264_BASELINE;//
            c->gop_size = 50;
            c->keyint_min = c->gop_size;
            c->max_b_frames = 1;
            av_opt_set_int(c->priv_data, "async_depth", 1, 0);
            av_opt_set_int(c->priv_data, "idr_interval", c->gop_size, 0);
            av_opt_set_int(c->priv_data, "bitrate_limit", c->bit_rate, 0);
            av_opt_set_int(c->priv_data, "int_ref_qp_delta", 0, 0);
            av_opt_set(c->priv_data, "profile", "main", 0);
            av_opt_set(c->priv_data, "preset", "veryfast", 0);//veryslow//veryfast//medium
            av_opt_set(c->priv_data, "tune", "zerolatency", 0);
            c->coder_type = FF_CODER_TYPE_AC;//FF_CODER_TYPE_VLC;//FF_CODER_TYPE_AC;// : FF_CODER_TYPE_VLC;
            c->qmax = 40;
	        c->qmin = 20;
	        //c->max_qdiff = 4;
	        //av_opt_set_int(c->priv_data, "int_ref_qp_delta", 4, 0);
	        //
	        //av_opt_set_int(c->priv_data, "mbbrc", 1, 0);
	        //c->slice_count = 8;//test
        }
    }

    //
    /* set hw_frames_ctx for encoder's AVCodecContext */
    printf("hw_encode_init: start set_hwframe_ctx \n");
    if ((ret = set_hwframe_ctx(c, hw_device_ctx, width, height)) < 0) {
        fprintf(stderr, "codec_init: set_hwframe_ctx: Error code: %s\n", av_err2str(ret));
        return ret;
    }
    printf("hw_encode_init: start set_hwframe_ctx: ret=%d \n", ret);
    if ((ret = avcodec_open2(c, codec, NULL)) < 0) {
        fprintf(stderr, "codec_init: avcodec_open2: Error code: %s\n", av_err2str(ret));
        return ret;
    }
    *p_c = c;
    *p_hw_device_ctx = hw_device_ctx;
    printf("hw_encode_init: ok \n");
    return ret;
}
int hw_encode_one_frame(AVCodecContext *c, AVFrame *sw_frame, AVFrame *hw_frame, FILE *fout, uint8_t *outbuf)
{
    int ret = 0;
    int frame_bytes = 0;
    //printf("hw_encode_one_frame: sw_frame->width=%d \n", sw_frame->width);
    //printf("hw_encode_one_frame: sw_frame->height=%d \n", sw_frame->height);
    //printf("hw_encode_one_frame: c->hw_frames_ctx=%x \n", c->hw_frames_ctx);
    if ((ret = av_hwframe_get_buffer(c->hw_frames_ctx, hw_frame, 0)) < 0) {
        fprintf(stderr, "encode_one_frame: Error code: %s.\n", av_err2str(ret));
        return ret;
    }
    if (!hw_frame->hw_frames_ctx) {
        ret = AVERROR(ENOMEM);
        return ret;
    }
    if ((ret = av_hwframe_transfer_data(hw_frame, sw_frame, 0)) < 0) {
        fprintf(stderr, "encode_one_frame: Error while transferring frame data to surface."
                "Error code: %s.\n", av_err2str(ret));
        return ret;
    }

    if ((ret = (encode_write(c, hw_frame, fout, outbuf, &frame_bytes))) < 0) {
        fprintf(stderr, "encode_one_frame: Failed to encode.\n");
        return ret;
    }
    ret = frame_bytes;
    return ret;
}

int hw_encode_close(   AVBufferRef *hw_device_ctx, AVCodecContext *c,
                    AVFrame *sw_frame, AVFrame *hw_frame,
                    FILE *fin, FILE *fout, uint8_t *uv_data, uint8_t *outbuf)
{
    int ret = 0;
    /* flush encoder */
    if(fout && outbuf)
        ret = encode_write(c, NULL, fout, outbuf, NULL);
    if (ret == AVERROR_EOF)
        ret = 0;
    if (fin)
        fclose(fin);
    if (fout)
        fclose(fout);
    if(sw_frame)
        av_frame_free(&sw_frame);
    if(hw_frame)
        av_frame_free(&hw_frame);
    if(c)
        avcodec_free_context(&c);
    if(hw_device_ctx)
        av_buffer_unref(&hw_device_ctx);
    if(uv_data)
    {
        free(uv_data);
    }
    return ret;
}
HCSVC_API
int api_vaapi_encode_test(char *infile, char *outfile, int width, int height, char *enc_name, int loopn)
//int api_vaapi_encode_test()
{
    int err;
    AVBufferRef *hw_device_ctx = NULL;
    FILE *fin = NULL, *fout = NULL;
    AVFrame *sw_frame = NULL, *hw_frame = NULL;
    AVCodecContext *c = NULL;
    AVCodec *codec = NULL;
    int frame_num = 0;
    uint8_t *uv_data = NULL;
    //const char *enc_name = "h264_vaapi";
    //const char *enc_name = "hevc_vaapi";//hevc_qsv
    //enc_name = "h264_vaapi";

    av_register_all(); // 注册支持的文件格式及对应的codec
    avformat_network_init();
    //size   = width * height;
    uint8_t *file_buf = NULL;
    int offset = 0;
    int file_size = 0;
    int frame_size = width * height;
    int frame_size2 = (frame_size * 3) >> 1;
#if 1
    int64_t time_0 = get_sys_time();//api_get_time_stamp_ll();
    frame_num = api_read_i420(infile, frame_size, &file_buf, &file_size, 1);
    if(frame_num <= 0)
    {
        return -1;
    }
    int64_t time_1 = get_sys_time();//api_get_time_stamp_ll();
    float avg_read_file_time = (float)(time_1 - time_0) / (float)frame_num;
    printf("api_vaapi_encode_test: frame_num=%d\n", frame_num);
    uint8_t *data_y = &file_buf[offset % file_size];
    AVFrame tmp_frame = {};
    //AVFrame tmp_frame;//注意：初始化至关重要，否则会导致异常崩溃
    tmp_frame.data[0] = data_y;
    tmp_frame.data[1] = &data_y[frame_size];
    //tmp_frame.data[2] = &data_y[frame_size + (frame_size >> 2)];
    tmp_frame.linesize[0] = width;
    tmp_frame.linesize[1] = width;
    //tmp_frame.linesize[2] = width >> 1;
    tmp_frame.width = width;
    tmp_frame.height = height;
    tmp_frame.format = AV_PIX_FMT_NV12;
    sw_frame = &tmp_frame;
#else
    if (!(fin = fopen(infile, "rb"))) //fopen(infile, "r")在windows下会导致文件读取失败
    {
        fprintf(stderr, "api_vaapi_encode_test: Fail to open input file : %s\n", strerror(errno));
        return -1;
    }
    if (!(sw_frame = av_frame_alloc())) {
        err = AVERROR(ENOMEM);
        printf("api_vaapi_encode_test:1:  err=%d \n", err);
        return err;
    }
    sw_frame->width  = width;
    sw_frame->height = height;
    sw_frame->format = AV_PIX_FMT_NV12;
    if ((err = av_frame_get_buffer(sw_frame, 0)) < 0)
    {
        printf("api_vaapi_encode_test: 2: err=%d \n", err);
        return err;
    }
    uv_data = malloc((frame_size >> 1) * sizeof(uint8_t));
#endif
    printf("api_vaapi_encode_test: infile=%s \n", infile);
    printf("api_vaapi_encode_test: outfile=%s \n", outfile);
    printf("api_vaapi_encode_test: enc_name=%s \n", enc_name);
    printf("api_vaapi_encode_test: width=%d, height=%d \n", width, height);
    //infile = "C:\Users\86139\winshare\InToTree_1920x1080.yuv";
    //if (argc < 5) {
    //    fprintf(stderr, "Usage: %s <width> <height> <input file> <output file>\n", argv[0]);
    //    return -1;
    //}
    int oflag = outfile && strcmp(outfile, "");
    printf("api_vaapi_encode_test: oflag=%d \n", oflag);
    if(oflag)
    {
        if (!(fout = fopen(outfile, "w+b"))) {
            fprintf(stderr, "Fail to open output file : %s\n", strerror(errno));
            err = -1;
            return err;
        }
    }

    uint8_t *outbuf = malloc((frame_size >> 1) * sizeof(uint8_t));
    //====
    codec = find_codec_by_name(enc_name, 1);//hevc_nvenc
    if(!codec)
    {
        fprintf(stderr, "api_vaapi_encode_test: Could not find encoder.\n");
        err = -1;
        return err;
    }
    printf("api_vaapi_encode_test: codec=%x \n", codec);

    err = hw_encode_init(NULL, codec, &hw_device_ctx, &c, enc_name, width, height);
    if(err < 0)
    {
        printf("api_vaapi_encode_test: encode_init: err=%d \n", err);
        return err;
    }
    //====
    time_0 = get_sys_time();
    frame_num = 0;
    while (1) {
#if 0
        if (!(sw_frame = av_frame_alloc())) {
            err = AVERROR(ENOMEM);
            printf("api_vaapi_encode_test:1:  err=%d \n", err);
            return err;
        }
        /* read data into software frame, and transfer them into hw frame */
        sw_frame->width  = width;
        sw_frame->height = height;
        sw_frame->format = AV_PIX_FMT_NV12;
        if ((err = av_frame_get_buffer(sw_frame, 0)) < 0)
        {
            printf("api_vaapi_encode_test: 2: err=%d \n", err);
            return err;
        }
#endif

#if 0
        if ((err = fread((uint8_t*)(sw_frame->data[0]), frame_size, 1, fin)) <= 0)
        {
            if(frame_num < loopn)
            {
                fseek( fin, 0, SEEK_SET );
                if ((err = fread((uint8_t*)(sw_frame->data[0]), frame_size, 1, fin)) <= 0)
                {
                    printf("api_vaapi_encode_test: 3: err=%d \n", err);
                    break;
                }
            }
            else{
                printf("api_vaapi_encode_test: 3: err=%d \n", err);
                break;
            }
        }
#if 0
        if ((err = fread((uint8_t*)(sw_frame->data[1]), frame_size/2, 1, fin)) <= 0)
            break;
#else
        if ((err = fread((uint8_t*)(uv_data), frame_size/2, 1, fin)) <= 0)
        {
            printf("api_vaapi_encode_test: 4: err=%d \n", err);
            break;
        }
        //
        uint8_t *u_data = uv_data;
        uint8_t *v_data = &uv_data[(frame_size >> 2)];
        uint16_t *p = (uint16_t *)sw_frame->data[1];
        int offset = 0;
        for(int i = 0; i < (frame_size >> 1); i += 2)
        {
            //for(int j = 0; j < width; j+=2)
            {
                uint16_t v0 = u_data[offset];
                uint16_t v1 = v_data[offset];
                p[offset] = v0 | (v1 << 8);
                offset++;
            }
        }
#endif
#else
        if(frame_num >= loopn)
        {
            break;
        }
        data_y = &file_buf[offset % file_size];
        sw_frame->data[0] = data_y;
        sw_frame->data[1] = &data_y[frame_size];
#endif

        if (!(hw_frame = av_frame_alloc())) {
            err = AVERROR(ENOMEM);
            return err;
        }
        //
        int64_t time0 = api_get_time_stamp_ll();
        int ret = hw_encode_one_frame(c, sw_frame, hw_frame, fout, outbuf);
        int64_t time1 = api_get_time_stamp_ll();
        int difftime = (int)(time1 - time0);
        printf("api_vaapi_encode_test: ret=%d, difftime=%d (ms) \n", ret, difftime);
        //
        av_frame_free(&hw_frame);
#if 0
        av_frame_free(&sw_frame);
#endif
        if(ret > 0)
            frame_num++;
        offset += frame_size2;
        if(offset >= file_size && file_size)
        {
            offset = offset % file_size;
        }
        printf("api_vaapi_encode_test: frame_num=%d \n", frame_num);
    }
    time_1 = get_sys_time();

    hw_encode_close(    hw_device_ctx, c,
                        NULL, hw_frame,
                        fin, fout, uv_data, outbuf);
    float avg_encode_time = (float)(time_1 - time_0) / (float)loopn;
    printf("api_vaapi_encode_test: loopn=%d, avg_encode_time=%2.2f (ms)\n", loopn, avg_encode_time);
    printf("api_vaapi_encode_test: frame_num=%d, avg_read_file_time=%2.2f (ms)\n", frame_num, avg_read_file_time);
    return frame_num;
}

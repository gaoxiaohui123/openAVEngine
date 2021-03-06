/*
 * Copyright (c) 2001 Fabrice Bellard
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
 * libavcodec API use example.
 *
 * @example decoding_encoding.c
 * Note that libavcodec only handles codecs (mpeg, mpeg4, etc...),
 * not file formats (avi, vob, mp4, mov, mkv, mxf, flv, mpegts, mpegps, etc...). See library 'libavformat' for the
 * format handling
 */

#include "inc.h"
#include "wels/codec_api.h"
//#include "libavutil/log2_tab.c" //ff_log2_tab变量定义在log2_tab.c文件中，libavformat模块有引用到

extern int rate_control(void *hnd);
extern int rate_control16(void *hnd);
extern void get_time(int64_t time, char *ctime, int offset);
extern int64_t get_time_stamp(void);

extern int GetvalueInt(cJSON *json, char *key);
extern char* GetvalueStr(cJSON *json, char *key, char *result);
extern AVCodec *find_codec_by_name(char *codec_name, int is_encoder);
extern int hw_encode_init(AVCodecContext *params, AVCodec *codec, AVBufferRef **p_hw_device_ctx, AVCodecContext **p_c,
                char *enc_name, int width, int height);
extern int hw_encode_one_frame(AVCodecContext *c, AVFrame *sw_frame, AVFrame *hw_frame, FILE *fout, uint8_t *outbuf);
extern int hw_encode_close(   AVBufferRef *hw_device_ctx, AVCodecContext *c,
                    AVFrame *sw_frame, AVFrame *hw_frame,
                    FILE *fin, FILE *fout, uint8_t *uv_data, uint8_t *outbuf);
extern int decode_init(AVCodec *decoder,
                AVFormatContext **p_input_ctx,
                AVStream **p_video_st,
                AVCodecContext **p_decoder_ctx,
                AVFrame **p_hw_frame,
                AVFrame **p_sw_frame,
                AVIOContext **p_output_ctx,
                AVBufferRef **p_hw_device_ctx,
                char *codec_name,
                char *infile, char *outfile);
extern int hw_decode_close(AVFormatContext *input_ctx,
                    AVStream *video_st,
                    AVCodecContext *decoder_ctx,
                    AVFrame *hw_frame,
                    AVFrame *sw_frame,
                    AVIOContext *output_ctx,
                    AVBufferRef *hw_device_ctx);
extern int hw_decode_one_frame(struct SwsContext **scale, AVCodecContext *c, AVPacket *pkt, AVFrame *hw_frame, AVFrame *sw_frame, uint8_t *outbuf);
extern int WiteFrame2(AVStreamObj *stream, AVPacket *pkt, int avtype);
extern void release_brnode(BitRateNode *head);
//pthread_mutex_t * glob_lock = NULL;


/* check that a given sample format is supported by the encoder */
static int check_sample_fmt(AVCodec *codec, enum AVSampleFormat sample_fmt)
{
    const enum AVSampleFormat *p = codec->sample_fmts;

    while (*p != AV_SAMPLE_FMT_NONE) {
        if (*p == sample_fmt)
            return 1;
        p++;
    }
    return 0;
}

/* just pick the highest supported samplerate */
static int select_sample_rate(AVCodec *codec)
{
    const int *p;
    int best_samplerate = 0;

    if (!codec->supported_samplerates)
        return 44100;

    p = codec->supported_samplerates;
    while (*p) {
        best_samplerate = FFMAX(*p, best_samplerate);
        p++;
    }
    return best_samplerate;
}

/* select layout with the highest channel count */
static int select_channel_layout(AVCodec *codec)
{
    const uint64_t *p;
    uint64_t best_ch_layout = 0;
    int best_nb_channels   = 0;

    if (!codec->channel_layouts)
        return AV_CH_LAYOUT_STEREO;

    p = codec->channel_layouts;
    while (*p) {
        int nb_channels = av_get_channel_layout_nb_channels(*p);

        if (nb_channels > best_nb_channels) {
            best_ch_layout    = *p;
            best_nb_channels = nb_channels;
        }
        p++;
    }
    return best_ch_layout;
}

/*
 * Audio encoding example
 */
static void audio_encode_example(const char *filename)
{
    AVCodec *codec;

    AVCodecContext *c= NULL;
    AVFrame *frame;
    AVPacket pkt;
    int i, j, k, ret, got_output;
    int buffer_size;
    FILE *f;
    uint16_t *samples;
    float t, tincr;

    printf("Encode audio file %s\n", filename);

    /* find the MP2 encoder */
    codec = avcodec_find_encoder(AV_CODEC_ID_MP2);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate audio codec context\n");
        exit(1);
    }

    /* put sample parameters */
    c->bit_rate = 64000;

    /* check that the encoder supports s16 pcm input */
    c->sample_fmt = AV_SAMPLE_FMT_S16;
    if (!check_sample_fmt(codec, c->sample_fmt)) {
        fprintf(stderr, "Encoder does not support sample format %s",
                av_get_sample_fmt_name(c->sample_fmt));
        exit(1);
    }

    /* select other audio parameters supported by the encoder */
    c->sample_rate    = select_sample_rate(codec);
    c->channel_layout = select_channel_layout(codec);
    c->channels       = av_get_channel_layout_nb_channels(c->channel_layout);

    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }

    /* frame containing input raw audio */
    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate audio frame\n");
        exit(1);
    }

    frame->nb_samples     = c->frame_size;
    frame->format         = c->sample_fmt;
    frame->channel_layout = c->channel_layout;

    /* the codec gives us the frame size, in samples,
     * we calculate the size of the samples buffer in bytes */
    buffer_size = av_samples_get_buffer_size(NULL, c->channels, c->frame_size,
                                             c->sample_fmt, 0);
    if (buffer_size < 0) {
        fprintf(stderr, "Could not get sample buffer size\n");
        exit(1);
    }
    samples = av_malloc(buffer_size);
    if (!samples) {
        fprintf(stderr, "Could not allocate %d bytes for samples buffer\n",
                buffer_size);
        exit(1);
    }
    /* setup the data pointers in the AVFrame */
    ret = avcodec_fill_audio_frame(frame, c->channels, c->sample_fmt,
                                   (const uint8_t*)samples, buffer_size, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not setup audio frame\n");
        exit(1);
    }

    /* encode a single tone sound */
    t = 0;
    tincr = 2 * M_PI * 440.0 / c->sample_rate;
    for (i = 0; i < 200; i++) {
        av_init_packet(&pkt);
        pkt.data = NULL; // packet data will be allocated by the encoder
        pkt.size = 0;

        for (j = 0; j < c->frame_size; j++) {
            samples[2*j] = (int)(sin(t) * 10000);

            for (k = 1; k < c->channels; k++)
                samples[2*j + k] = samples[2*j];
            t += tincr;
        }
        /* encode the samples */
        ret = avcodec_encode_audio2(c, &pkt, frame, &got_output);
        if (ret < 0) {
            fprintf(stderr, "Error encoding audio frame\n");
            exit(1);
        }
        if (got_output) {
            fwrite(pkt.data, 1, pkt.size, f);
            av_free_packet(&pkt);
        }
    }

    /* get the delayed frames */
    for (got_output = 1; got_output; i++) {
        ret = avcodec_encode_audio2(c, &pkt, NULL, &got_output);
        if (ret < 0) {
            fprintf(stderr, "Error encoding frame\n");
            exit(1);
        }

        if (got_output) {
            fwrite(pkt.data, 1, pkt.size, f);
            av_free_packet(&pkt);
        }
    }

    fclose(f);
    av_freep(&samples);
    av_frame_free(&frame);
    avcodec_close(c);//!!!!!
    av_free(c);

}

/*
 * Audio decoding.
 */
static void audio_decode_example(const char *outfilename, const char *filename)
{
    AVCodec *codec;
    AVCodecContext *c= NULL;
    int len;
    FILE *f, *outfile;
    uint8_t inbuf[AUDIO_INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    AVPacket pkt;
    AVFrame *decoded_frame = NULL;

    av_init_packet(&pkt);

    printf("Decode audio file %s to %s\n", filename, outfilename);

    /* find the mpeg audio decoder */
    codec = avcodec_find_decoder(AV_CODEC_ID_MP2);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate audio codec context\n");
        exit(1);
    }

    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }
    outfile = fopen(outfilename, "wb");
    if (!outfile) {
        av_free(c);
        exit(1);
    }

    /* decode until eof */
    pkt.data = inbuf;
    pkt.size = fread(inbuf, 1, AUDIO_INBUF_SIZE, f);

    while (pkt.size > 0) {
        int i, ch;
        int got_frame = 0;

        if (!decoded_frame) {
            if (!(decoded_frame = av_frame_alloc())) {
                fprintf(stderr, "Could not allocate audio frame\n");
                exit(1);
            }
        }

        len = avcodec_decode_audio4(c, decoded_frame, &got_frame, &pkt);
        if (len < 0) {
            fprintf(stderr, "Error while decoding\n");
            exit(1);
        }
        if (got_frame) {
            /* if a frame has been decoded, output it */
            int data_size = av_get_bytes_per_sample(c->sample_fmt);
            if (data_size < 0) {
                /* This should not occur, checking just for paranoia */
                fprintf(stderr, "Failed to calculate data size\n");
                exit(1);
            }
            for (i=0; i<decoded_frame->nb_samples; i++)
                for (ch=0; ch<c->channels; ch++)
                    fwrite(decoded_frame->data[ch] + data_size*i, 1, data_size, outfile);
        }
        pkt.size -= len;
        pkt.data += len;
        pkt.dts =
        pkt.pts = AV_NOPTS_VALUE;
        if (pkt.size < AUDIO_REFILL_THRESH) {
            /* Refill the input buffer, to avoid trying to decode
             * incomplete frames. Instead of this, one could also use
             * a parser, or use a proper container format through
             * libavformat. */
            memmove(inbuf, pkt.data, pkt.size);
            pkt.data = inbuf;
            len = fread(pkt.data + pkt.size, 1,
                        AUDIO_INBUF_SIZE - pkt.size, f);
            if (len > 0)
                pkt.size += len;
        }
    }

    fclose(outfile);
    fclose(f);

    avcodec_close(c);
    av_free(c);
    av_frame_free(&decoded_frame);
}

/*
 * Video decoding example
 */

static void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize,
                     char *filename)
{
    FILE *f;
    int i;

    f = fopen(filename,"a");
    //fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);
    for (i = 0; i < ysize; i++)
        fwrite(buf + i * wrap, 1, xsize, f);
    fclose(f);
}
static int decode_write_frame(const char *outfilename, AVCodecContext *avctx,
                              AVFrame *frame, int *frame_count, AVPacket *pkt, int last)
{
    int len, got_frame = 0;
    char buf[1024];
    printf("decode_write_frame: start decode \n");
    len = avcodec_decode_video2(avctx, frame, &got_frame, pkt);
    if (len < 0) {
        fprintf(stderr, "decode_write_frame: Error while decoding frame %d\n", *frame_count);
        return len;
    }
    else{
        //printf("decode_write_frame: len= %d\n", len);
    }
    if (got_frame) {
        printf("decode_write_frame: Saving %sframe %3d\n", last ? "last " : "", *frame_count);
        fflush(stdout);

        /* the picture is allocated by the decoder, no need to free it */
        snprintf(buf, sizeof(buf), outfilename, *frame_count);
        //
        //printf("frame->linesize[0] %d \n", frame->linesize[0]);
        //printf("frame->linesize[1] %d \n", frame->linesize[1]);
        //printf("frame->linesize[2] %d \n", frame->linesize[2]);
        //printf("frame->width %d \n", frame->width);
        //printf("frame->height %d \n", frame->height);
        //
        pgm_save(frame->data[0], frame->linesize[0],
                 frame->width, frame->height, buf);
        pgm_save(frame->data[1], frame->linesize[1],
                 frame->width >> 1, frame->height >> 1, buf);
        pgm_save(frame->data[2], frame->linesize[2],
                 frame->width >> 1, frame->height >> 1, buf);
        (*frame_count)++;
    }
    if (pkt->data) {
        pkt->size -= len;
        pkt->data += len;
    }
    return got_frame;
}
/*
 * Video encoding example
 */
void i420_2nv12(uint8_t *src, uint8_t *dst, int frame_size)
{
    uint8_t *u_data = src;
    uint8_t *v_data = &src[(frame_size >> 2)];
    uint16_t *p = (uint16_t *)dst;
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
}
HCSVC_API
int api_read_i420(char *infile, int frame_size, uint8_t **outbuf, int *file_size, int is_nv12)
{
    FILE *fin = NULL;
    int frame_size2 = (frame_size * 3) >> 1;
    int64_t time_0 = get_sys_time();
    if (!(fin = fopen(infile, "rb"))) //fopen(infile, "r")在windows下会导致文件读取失败
    {
        fprintf(stderr, "api_read_i420: Fail to open input file : %s\n", strerror(errno));
        return -1;
    }
    printf("api_read_i420: fin=%x\n", fin);
    int ret = fseek(fin, 0, SEEK_END);
    printf("api_read_i420: fin=%x\n", fin);
    int len = ftell(fin);
    len = frame_size2 * 100;//test
    printf("api_read_i420: len=%d\n", len);
    float flen = len / (float)(1 << 30);//K,M,G
    printf("api_read_i420: flen=%f(GB) \n", flen);
    ret = fseek( fin, 0, SEEK_SET );
    printf("api_read_i420: fin=%x\n", fin);
    if(ret != 0)
    {
        printf("api_read_i420: fseek: ret=%d\n", ret);
    }
    int64_t time_1 = get_sys_time();//api_get_time_stamp_ll();
    float avg_seek_file_time = (float)(time_1 - time_0) / (float)(len / frame_size2);
    printf("api_read_i420: avg_seek_file_time=%2.2f (ms)\n", avg_seek_file_time);
    //fclose(fin);
    //if (!(fin = fopen(infile, "rb"))) //fopen(infile, "r")在windows下会导致文件读取失败
    //{
    //    fprintf(stderr, "api_read_i420: Fail to open input file : %s\n", strerror(errno));
    //    return -1;
    //}
    time_0 = get_sys_time();
    uint8_t *p = *outbuf;
    uint8_t *p2 = NULL;
    if(!p){
        p = av_malloc((len + frame_size2) * sizeof(uint8_t));
        p2 = av_malloc((len + frame_size2) * sizeof(uint8_t));
        if(!p || !p2)
        {
            printf("api_read_i420: p=%x \n", p);
            return -1;
        }
        *outbuf = p;
    }
    int offset = 0;
    int i = 0;
    do{
        ret = fread(&p[offset], 1, frame_size2, fin);
        if(ret != frame_size2)
        {
            printf("api_read_i420: i=%d, ret=%d \n", i, ret);
            break;
        }
        offset += ret;
        i++;
    }while(offset < len);
    if(file_size)
    {
        file_size[0] = offset;
    }
    fclose(fin);
    time_1 = get_sys_time();//api_get_time_stamp_ll();
    float avg_read_file_time = (float)(time_1 - time_0) / (float)(len / frame_size2);
    printf("api_read_i420: avg_read_file_time=%2.2f (ms)\n", avg_read_file_time);
    if(p2)
    {
        time_0 = get_sys_time();
        int offset = 0;
        for(int j = 0; j < i; j++)
        {
            if(is_nv12)
            {
                memcpy(&p2[offset], &p[offset], frame_size);//y
                i420_2nv12(&p[offset + frame_size], &p2[offset + frame_size], frame_size);//uv
            }
            else{
                memcpy(&p2[offset], &p[offset], frame_size2);
            }
            //
            offset += frame_size2;
        }
        time_1 = get_sys_time();//api_get_time_stamp_ll();
        float avg_memcpy_time = (float)(time_1 - time_0) / (float)i;
        printf("api_read_i420: avg_memcpy_time=%2.2f (ms)\n", avg_memcpy_time);
        if(is_nv12)
        {
            av_free(p);
            *outbuf = p2;
        }
        else{
            av_free(p2);
        }

    }
    return i;
}
static int video_encode_example(char *infile, char *outfile, int width, int height, int codec_id, int loopn)
{
    AVCodec *codec;
    AVCodecContext *c= NULL;
    int i, ret, x, y, got_output = 0;
    FILE *fin = NULL, *fout = NULL;
    AVPacket pkt;
    AVFrame *frame = NULL;
    //uint8_t endcode[] = { 0, 0, 1, 0xb7 };
    printf("Encode video infile=%s \n", infile);
    printf("Encode video outfile=%s \n", outfile);
    uint8_t *file_buf = NULL;
    int offset = 0;
    int file_size = 0;
    int frame_size = width * height;
    int frame_size2 = (frame_size * 3) >> 1;
#if 1
    int64_t time_0 = get_sys_time();//api_get_time_stamp_ll();
    int frame_num = api_read_i420(infile, frame_size, &file_buf, &file_size, 0);
    if(frame_num <= 0)
    {
        return -1;
    }
    int64_t time_1 = get_sys_time();//api_get_time_stamp_ll();
    float avg_read_file_time = (float)(time_1 - time_0) / (float)frame_num;
    printf("video_encode_example: frame_num=%d\n", frame_num);
    uint8_t *data_y = &file_buf[offset % file_size];
    AVFrame tmp_frame = {};
    //AVFrame tmp_frame;//注意：初始化至关重要，否则会导致异常崩溃
    tmp_frame.data[0] = data_y;
    tmp_frame.data[1] = &data_y[frame_size];
    tmp_frame.data[2] = &data_y[frame_size + (frame_size >> 2)];
    tmp_frame.linesize[0] = width;
    tmp_frame.linesize[1] = width >> 1;
    tmp_frame.linesize[2] = width >> 1;
    tmp_frame.width = width;
    tmp_frame.height = height;
    tmp_frame.format = AV_PIX_FMT_YUV420P;
    frame = &tmp_frame;
#else
    if (!(fin = fopen(infile, "rb"))) //fopen(infile, "r")在windows下会导致文件读取失败
    {
        fprintf(stderr, "video_encode_example: Fail to open input file : %s\n", strerror(errno));
        return -1;
    }
    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        return -1;
    }
    frame->format = AV_PIX_FMT_YUV420P;
    frame->width  = width;
    frame->height = height;
    ret = av_image_alloc(frame->data, frame->linesize, width, height, AV_PIX_FMT_YUV420P, 16);
#endif

    int oflag = outfile && strcmp(outfile, "");
    printf("video_encode_example: oflag=%d \n", oflag);
    fout = fopen(outfile, "wb");
    if (!fout && oflag) {
        fprintf(stderr, "Could not open %s\n", outfile);
        return -1;
    }
    printf("video_encode_example: fout=%x \n", fout);

    /* find the mpeg1 video encoder */

    if(codec_id < 0)
    {
        printf("video_encode_example: libwz264 \n");
        codec = avcodec_find_encoder_by_name("libwz264");
        if (!codec) {
            fprintf(stderr, "video_encode_example: Codec not found\n");
            return -1;
        }
    }
    else{
        printf("video_encode_example: libx264 \n");
        codec = avcodec_find_encoder(codec_id);
        if (!codec) {
            codec = avcodec_find_encoder_by_name("libwz264");
            if (!codec) {
                fprintf(stderr, "video_encode_example: Codec not found\n");
                return -1;
            }
        }
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        return -1;
    }

    /* put sample parameters */
    c->bit_rate = 2048 * 1000;
    /* resolution must be a multiple of two */
    c->width = width;
    c->height = height;
    /* frames per second */
    c->time_base = (AVRational){1,25};
    /* emit one intra frame every ten frames
     * check frame pict_type before passing frame
     * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
     * then gop_size is ignored and the output of encoder
     * will always be I frame irrespective to gop_size
     */
    c->refs = 1;
    c->gop_size = 50; //10;
    c->b_frame_strategy = false;
    c->max_b_frames = 0;//1;
    c->pix_fmt = AV_PIX_FMT_YUV420P;
    //c->thread_type = FF_THREAD_SLICE; //FF_THREAD_FRAME;//gxh
    //c->thread_type = FF_THREAD_FRAME;
#if 1
    if (codec_id == AV_CODEC_ID_H264 || codec_id < 0)
    {
        c->coder_type = FF_CODER_TYPE_AC;
        //存在异常：理论上placebo压缩率最高
        av_opt_set(c->priv_data, "preset", "superfast", 0);//medium//ultrafast//veryfast//superfast//placebo
        av_opt_set(c->priv_data, "tune", "zerolatency", 0);
        c->qmax = 40;
	    c->qmin = 20;// 26;

        int mtu_size = 1100;
        if(mtu_size > 0 && true)
        {
            c->thread_type = FF_THREAD_FRAME;
            c->thread_count = 1;
            //c->thread_type = FF_THREAD_SLICE;
            //c->thread_count = 4;
            char x264opts[1024] = "";
            char ctmp[64] = "";
            sprintf(ctmp, "%d", mtu_size);
            strcat(x264opts, "slice-max-size=");
            strcat(x264opts, ctmp);

            //
            int qp = 30;//26;
	        sprintf(ctmp, "%d", qp);
            if(strlen(x264opts))
	        {
	            strcat(x264opts, ":");
	        }
            strcat(x264opts, "qp=");
            strcat(x264opts, ctmp);
            if(codec_id < 0)
            {
                av_opt_set(c->priv_data, "wz264opts",x264opts,0);
            }
            else{
                av_opt_set(c->priv_data, "x264opts",x264opts,0);
            }

            printf("video_encode_example: x264opts=%s \n", x264opts);
        }
    }
    else //if(codec_id == AV_CODEC_ID_WZ264 && false)
#endif
    {
        c->thread_type = FF_THREAD_FRAME;
        c->thread_count = 1;
        c->coder_type = FF_CODER_TYPE_AC;
        //存在异常：理论上placebo压缩率最高
        av_opt_set(c->priv_data, "preset", "superfast", 0);//medium//ultrafast//veryfast//superfast//placebo
        av_opt_set(c->priv_data, "tune", "zerolatency", 0);
        //c->qmax = 40;
	    //c->qmin = 20;// 26;

	    //c->qmax = 30;
	    //c->qmin = 30;// 26;
	    //c->bit_rate = 4800 * 1000;
	    //c->bit_rate = 315 * 1000;

	    //c->qmax = 25;
	    //c->qmin = 25;// 26;
	    c->bit_rate = 710 * 1000;
    }

    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "video_encode_example: Could not open codec\n");
        return -1;
    }

    /* the image can be allocated by any means and av_image_alloc() is
     * just the most convenient way if av_malloc() is to be used */
    //ret = av_image_alloc(frame->data, frame->linesize, c->width, c->height, c->pix_fmt, 16);

    printf("frame->linesize[0] %d \n", frame->linesize[0]);
    printf("frame->linesize[1] %d \n", frame->linesize[1]);
    printf("frame->linesize[2] %d \n", frame->linesize[2]);
    /* encode 1 second of video */
    time_0 = get_sys_time();
    for (i = 0; i < loopn; i++)
    {
        av_init_packet(&pkt);
        pkt.data = NULL;    // packet data will be allocated by the encoder
        pkt.size = 0;
        //fflush(stdout);
#if 0
        /* prepare a dummy image */
        /* Y */
        for (y = 0; y < c->height; y++) {
            for (x = 0; x < c->width; x++) {
                frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
            }
        }

        /* Cb and Cr */
        for (y = 0; y < c->height/2; y++) {
            for (x = 0; x < c->width/2; x++) {
                frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
                frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
            }
        }
#elif(0)
        if ((ret = fread((uint8_t*)(frame->data[0]), frame_size, 1, fin)) <= 0)
        {
            if(i < loopn)
            {
                fseek( fin, 0, SEEK_SET );
                if ((ret = fread((uint8_t*)(frame->data[0]), frame_size, 1, fin)) <= 0)
                {
                    printf("video_encode_example: ret=%d \n", ret);
                    break;
                }
            }
            else{
                printf("video_encode_example: ret=%d \n", ret);
                break;
            }
        }
        if ((ret = fread((uint8_t*)(frame->data[1]), (frame_size >> 2), 1, fin)) <= 0)
        {
            printf("video_encode_example: ret=%d \n", ret);
            break;
        }
        if ((ret = fread((uint8_t*)(frame->data[2]), (frame_size >> 2), 1, fin)) <= 0)
        {
            printf("video_encode_example: ret=%d \n", ret);
            break;
        }
#else
        //if(i >= 690)
        //{
        //    int offset2 = offset % file_size;
        //   printf("video_encode_example: offset2=%d \n", offset2);
        //}
        //printf("video_encode_example: file_size=%d \n", file_size);
        data_y = &file_buf[offset % file_size];
        frame->data[0] = data_y;
        frame->data[1] = &data_y[frame_size];
        frame->data[2] = &data_y[frame_size + (frame_size >> 2)];

        //printf("video_encode_example: offset=%d \n", offset);
#endif
        frame->pts = i;
#if 0
        int j = i % 50;
        if(j == 0)
        {
            frame->pict_type = AV_PICTURE_TYPE_I;
        }
        else{
            j = i % 2;
            if(!j)
            {
                frame->pict_type = AV_PICTURE_TYPE_P;
            }
            else
            {
                frame->pict_type = AV_PICTURE_TYPE_B;
            }
        }
#endif
        int64_t time0 = api_get_time_stamp_ll();
        //printf("video_encode_example: frame->data[0]=%x \n", frame->data[0]);
        /* encode the image */
        ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
        int64_t time1 = api_get_time_stamp_ll();
        int difftime = (int)(time1 - time0);
        printf("video_encode_example: i=%d, pkt.size=%d, difftime=%d (ms) \n", i, pkt.size, difftime);
        if (ret < 0) {
            fprintf(stderr, "Error encoding frame\n");
            return -1;
        }
        if (got_output && fout) {
            //printf("Write frame %3d (size=%5d)\n", i, pkt.size);
            fwrite(pkt.data, 1, pkt.size, fout);
            av_free_packet(&pkt);
        }
        offset += frame_size2;
        if(offset >= file_size && file_size)
        {
            offset = offset % file_size;
        }
    }
    time_1 = get_sys_time();

    /* get the delayed frames */
    if(fout)
    {
        for (got_output = 1; got_output; i++) {
            //fflush(stdout);
            ret = avcodec_encode_video2(c, &pkt, NULL, &got_output);
            if (ret < 0) {
                fprintf(stderr, "Error encoding frame\n");
                return -1;
            }
            if (got_output && fout) {
                printf("Write frame %3d (size=%5d)\n", i, pkt.size);
                fwrite(pkt.data, 1, pkt.size, fout);
                av_free_packet(&pkt);
            }
        }
    }

    printf("video_encode_example: end \n");
    /* add sequence end code to have a real mpeg file */
    if(fout)
    {
        uint8_t endcode[] = { 0, 0, 1, 0xb7 };
        fwrite(endcode, 1, sizeof(endcode), fout);
        fclose(fout);
    }

    if(fin)
    {
        fclose(fin);
    }
    avcodec_close(c);
    av_free(c);
    //av_freep(&frame->data[0]);
    //av_frame_free(&frame);
    if(file_buf)
    {
        av_free(file_buf);
    }
    float avg_encode_time = (float)(time_1 - time_0) / (float)loopn;
    printf("video_encode_example: loopn=%d, avg_encode_time=%2.2f (ms)\n", loopn, avg_encode_time);
    printf("video_encode_example: frame_num=%d, avg_read_file_time=%2.2f (ms)\n", frame_num, avg_read_file_time);
    return 0;
}
static void video_decode_example(const char *outfilename, const char *filename)
{
    AVCodec *codec;
    AVCodecContext *c= NULL;
    int frame_count;
    FILE *f;
    AVFrame *frame;
    uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    AVPacket pkt;

    av_init_packet(&pkt);

    /* set end of buffer to 0 (this ensures that no overreading happens for damaged mpeg streams) */
    ///memset(inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);

    printf("Decode video file %s to %s\n", filename, outfilename);

    /* find the mpeg1 video decoder */
    //codec = avcodec_find_decoder(AV_CODEC_ID_MPEG1VIDEO);
    codec = avcodec_find_decoder(AV_CODEC_ID_H264);

    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    if (codec->capabilities & AV_CODEC_CAP_TRUNCATED)
        c->flags |= AV_CODEC_FLAG_TRUNCATED; // we do not send complete frames
    //c->thread_type = FF_THREAD_SLICE;
    //printf("c->flags= %d\n", c->flags);
    //c->flags |= AV_CODEC_FLAG_TRUNCATED;
    //printf("c->flags= %d\n", c->flags);
    /* For some codecs, such as msmpeg4 and mpeg4, width and height
       MUST be initialized there because this information is not
       available in the bitstream. */

    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }

    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }

    frame_count = 0;
    int flag = 0;
    int read_size = 0;
#if 0
    for (;;) {
        pkt.size = fread(inbuf, 1, INBUF_SIZE, f);
        if (pkt.size == 0)
        {
            printf("pkt.size is zero \n");
            break;
        }
        read_size += pkt.size;

        /* NOTE1: some codecs are stream based (mpegvideo, mpegaudio)
           and this is the only method to use them because you cannot
           know the compressed data size before analysing it.

           BUT some other codecs (msmpeg4, mpeg4) are inherently frame
           based, so you must call them with all the data for one
           frame exactly. You must also initialize 'width' and
           'height' before initializing them. */

        /* NOTE2: some codecs allow the raw parameters (frame size,
           sample rate) to be changed at any frame. We handle this, so
           you should also take care of it */

        /* here, we use a stream based decoder (mpeg1video), so we
           feed decoder and see if it could decode a frame */
        pkt.data = inbuf;
        while (pkt.size > 0)
        {
            int ret = decode_write_frame(outfilename, c, frame, &frame_count, &pkt, 0);
            if (ret < 0)
            {
                printf("decode error \n");
                //exit(1);
                flag = 1;
                break;
            }
            else{
                if(ret == 1)
                {
                    printf("read_size= %d \n", read_size);
                    read_size = 0;
                }
            }
        }
        if (flag)
        {
            break;
        }
    }
#else
    AVCodecParserContext *pCodecParserCtx = av_parser_init( AV_CODEC_ID_H264 );
    while(1)
    {
        //cur_size = fread(in_buffer, 1, in_buffer_size, fp_in);
        int cur_size = fread(inbuf, 1, INBUF_SIZE, f);
        if (cur_size == 0)
            break;
        uint8_t *cur_ptr = inbuf;
        while (cur_size > 0)
        {
            int len = av_parser_parse2(
                pCodecParserCtx, c,
                &pkt.data, &pkt.size,
                cur_ptr, cur_size,
                AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);
            cur_ptr += len;
            cur_size -= len;
            if (pkt.size == 0)
                continue;
            int ret = decode_write_frame(outfilename, c, frame, &frame_count, &pkt, 0);
        }
    }
#endif

    /* some codecs, such as MPEG, transmit the I and P frame with a
       latency of one frame. You must do the following to have a
       chance to get the last frame of the video */
    pkt.data = NULL;
    pkt.size = 0;
    if (!flag)
    {
        decode_write_frame(outfilename, c, frame, &frame_count, &pkt, 1);
    }


    fclose(f);

    avcodec_close(c);
    av_free(c);
    av_frame_free(&frame);
    printf("decode over \n");
}
//
HCSVC_API
int api_sw_encode_test(char *infile, char *outfile, int width, int height, int codec_id0, int loopn)
{
    int codec_id = codec_id0;
    /* register all the codecs */
    avcodec_register_all();
    if (codec_id0 < 0)
    {
        if(codec_id0 != -100)
        {
            codec_id = AV_CODEC_ID_H264;
        }

        printf("api_sw_encode_test: 1: codec_id=%d \n", codec_id);
#if 0
        if(codec_id0 == -100)
        {
            codec_id = AV_CODEC_ID_WZ264;
            printf("api_sw_encode_test: AV_CODEC_ID_H264= %d \n", AV_CODEC_ID_H264);
            printf("api_sw_encode_test: AV_CODEC_ID_WZ264= %d \n", AV_CODEC_ID_WZ264);
        }
#endif
        printf("api_sw_encode_test: 2: codec_id= %d \n", codec_id);
        //codec_id = AV_CODEC_ID_MPEG1VIDEO;
    }

    video_encode_example(infile, outfile, width, height, codec_id, loopn);
    return 0;
}
int APIDecodecTest(const char *outfilename, const char *filename)
{
    printf("you input %s and outfile %s\n", filename, outfilename);
    if (remove(outfilename) == 0)
    {
        printf("Deleted successfully: %s \n", outfilename);
    }

    /* register all the codecs */
    avcodec_register_all();
    video_decode_example(outfilename, filename);
    return 0;
}
//================================================================
HCSVC_API
void api_register_test()
{
    printf("api_register_test: 0 \n");
    av_log_set_level(AV_LOG_DEBUG);//( AV_LOG_DEBUG );
    av_register_all();
    avcodec_register_all();
    avformat_network_init();
    avdevice_register_all();
    printf("api_register_test: over \n");
}
HCSVC_API
int api_create_audio_codec_handle(char *handle)
{
    long long ret = 0;
    if(handle)
    {
        AudioCodecObj *obj = (AudioCodecObj *)calloc(1, sizeof(AudioCodecObj));

        ret = (long long)obj;
        //handle = (void *)obj;
        int handle_size = sizeof(long long);
        printf("api_create_audio_codec_handle: handle_size= %d \n", handle_size);
        printf("api_create_audio_codec_handle: obj= %x \n", obj);
        printf("api_create_audio_codec_handle: obj->rtpObj= %x \n", obj->rtpObj);
        printf("api_create_audio_codec_handle: obj->resortObj= %x \n", obj->resortObj);
        memcpy(handle, &ret, handle_size);
        long long *testp = (long long *)handle;
        printf("api_create_audio_codec_handle: testp[0]= %x \n", testp[0]);
        int id = (int)obj;//long long ???
        obj->Obj_id = id;
        ret = id;
    }
    return (int)(ret & 0x7FFFFFFF);
}

HCSVC_API
int api_audio_codec_close(char *handle)
{
    long long ret = 0;
    if(handle)
    {
        long long *testp = (long long *)handle;
        AudioCodecObj *obj = (AudioCodecObj *)testp[0];
        //
        if(obj->audio_convert_ctx)
        {
            swr_free(&obj->audio_convert_ctx);
        }
        if(obj->frame)
        {
            av_frame_free(&obj->frame);
        }
        if(obj->c)
        {
            avcodec_close(obj->c);
            av_free(obj->c);
        }
        if(obj->rtpObj)
        {
            free(obj->rtpObj);
            obj->rtpObj = NULL;
        }
        if(obj->resortObj)
        {
            free(obj->resortObj);
            obj->resortObj = NULL;
        }
        if(obj->pktManObj)
        {
            free(obj->pktManObj);
            obj->pktManObj = NULL;
        }
        if(obj->json)
        {
            api_json_free(obj->json);
            obj->json = NULL;
        }
        //
        free(obj);
	    testp[0] = 0;
        printf("api_audio_capture_close: ok \n");
    }
    return (int)ret;
}

HCSVC_API
int api_audio_codec_init(char *handle, char *param)
{
    int ret = 0;

    ret = api_create_audio_codec_handle(handle);
    long long *testp = (long long *)handle;
    AudioCodecObj *obj = (AudioCodecObj *)testp[0];
    printf("api_audio_codec_init: param= %s \n", param);
    obj->json = (cJSON *)api_str2json(param);
    printf("api_audio_codec_init: obj->json= %x \n", obj->json);
    obj->param = param;
    obj->print = GetvalueInt(obj->json, "print");


    obj->codec_type = GetvalueInt(obj->json, "codec_type");
    obj->out_nb_samples = GetvalueInt(obj->json, "out_nb_samples");
    obj->out_channels = GetvalueInt(obj->json, "out_channels");
    obj->out_sample_rate = GetvalueInt(obj->json, "out_sample_rate");


    char text[64] = "";
    GetvalueStr(obj->json, "codec_id", text);
	if (!strcmp(text, "AV_CODEC_ID_AAC"))
	{
       obj->codec_id = (enum AVCodecID)(AV_CODEC_ID_AAC);
	}

	GetvalueStr(obj->json, "out_channel_layout", text);
	if (!strcmp(text, "AV_CH_LAYOUT_STEREO"))
	{
       obj->out_channel_layout = AV_CH_LAYOUT_STEREO;
	}
	else if (!strcmp(text, "AV_CH_LAYOUT_MONO"))
	{
       obj->out_channel_layout = AV_CH_LAYOUT_MONO;
	}

    obj->bit_rate = GetvalueInt(obj->json, "bit_rate");

    GetvalueStr(obj->json, "sample_fmt", text);
	if (!strcmp(text, "AV_SAMPLE_FMT_S16"))
	{
       obj->sample_fmt = AV_SAMPLE_FMT_S16;
	}
	else if(!strcmp(text, "AV_SAMPLE_FMT_FLTP"))
	{
	    obj->sample_fmt = AV_SAMPLE_FMT_FLTP;//float, planar
	}
	obj->out_size = av_samples_get_buffer_size( NULL,
                                                obj->out_channels,
                                                obj->out_nb_samples,
                                                obj->sample_fmt, 1);
	//av_register_all();
    //
    if(!obj->codec_type)
    {
        /* find the MP2 encoder */
        obj->codec = avcodec_find_encoder(obj->codec_id);
        if (!obj->codec) {
            fprintf(stderr, "api_audio_codec_init: 1: Codec not found\n");
            return -1;
        }

        obj->c = avcodec_alloc_context3(obj->codec);
        if (!obj->c) {
            fprintf(stderr, "api_audio_codec_init:Could not allocate audio codec context\n");
            return -1;
        }

        /* put sample parameters */
        obj->c->bit_rate = obj->bit_rate;//64000;

        /* check that the encoder supports s16 pcm input */
        obj->c->sample_fmt = obj->sample_fmt;//AV_SAMPLE_FMT_S16;
        printf("api_create_audio_codec_handle: AV_SAMPLE_FMT_S16= %d \n", AV_SAMPLE_FMT_S16);
        printf("api_create_audio_codec_handle: obj->sample_fmt= %d \n", obj->sample_fmt);
        //obj->c->sample_fmt = AV_SAMPLE_FMT_FLTP;
        if (!check_sample_fmt(obj->codec, obj->c->sample_fmt)) {
            fprintf(stderr, "api_audio_codec_init: Encoder does not support sample format %s \n",
                    av_get_sample_fmt_name(obj->c->sample_fmt));
            return -1;
        }

        /* select other audio parameters supported by the encoder */
        obj->c->sample_rate = select_sample_rate(obj->codec);
        obj->c->sample_rate = obj->out_sample_rate;
        //obj->c->sample_rate = 48000;//24000;
        //
        //obj->c->sample_fmt  = obj->codec->sample_fmts ? obj->codec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
        printf("api_create_audio_codec_handle: obj->codec_id= %d \n", obj->codec_id);
        //printf("api_create_audio_codec_handle: obj->codec->sample_fmts= %x \n", obj->codec->sample_fmts);
        printf("api_create_audio_codec_handle: obj->codec->sample_fmts[0]= %d \n", obj->codec->sample_fmts[0]);
        //printf("api_create_audio_codec_handle: obj->codec->supported_samplerates= %x \n", obj->codec->supported_samplerates);
        if (obj->codec->supported_samplerates && false)
        {
			obj->c->sample_rate = obj->codec->supported_samplerates[0];
			for (int i = 0; obj->codec->supported_samplerates[i]; i++)
			{
			    printf("api_create_audio_codec_handle: obj->codec->supported_samplerates[i]= %d \n", obj->codec->supported_samplerates[i]);
				if (obj->codec->supported_samplerates[i] == 48000)//44100)
				{
				    obj->c->sample_rate = 48000;//44100;
				}
			}
		}

        obj->c->channel_layout = select_channel_layout(obj->codec);
        //printf("api_create_audio_codec_handle: obj->codec->channel_layouts= %x \n", obj->codec->channel_layouts);
        if (obj->codec->channel_layouts && false)
        {
			obj->c->channel_layout = obj->codec->channel_layouts[0];
			for (int i = 0; obj->codec->channel_layouts[i]; i++)
			{
				if (obj->codec->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
				{
				    obj->c->channel_layout = AV_CH_LAYOUT_STEREO;
				}
			}
		}
        obj->c->channels   = av_get_channel_layout_nb_channels(obj->c->channel_layout);
        obj->c->frame_size = obj->out_nb_samples;
        obj->c->sample_rate = obj->out_sample_rate;
        obj->c->channels = obj->out_channels;
        obj->c->channel_layout = obj->out_channel_layout;

        /* open it */
        if (avcodec_open2(obj->c, obj->codec, NULL) < 0) {
            fprintf(stderr, "Could not open codec\n");
            return -1;
        }

        /* frame containing input raw audio */
        obj->frame = av_frame_alloc();
        if (!obj->frame) {
            fprintf(stderr, "Could not allocate audio frame\n");
            return -1;
        }
        obj->c->frame_size = obj->out_nb_samples;
        obj->frame->nb_samples     = obj->c->frame_size;
        obj->frame->format         = obj->c->sample_fmt;
        obj->frame->channel_layout = obj->c->channel_layout;
        obj->frame->channels = obj->c->channels;
        obj->frame->sample_rate = obj->c->sample_rate;


        printf("api_create_audio_codec_handle: obj->c->sample_rate= %d \n", obj->c->sample_rate);
        printf("api_create_audio_codec_handle: obj->c->frame_size= %d \n", obj->c->frame_size);
        printf("api_create_audio_codec_handle: obj->c->sample_fmt= %d \n", obj->c->sample_fmt);
        printf("api_create_audio_codec_handle: obj->c->channel_layout= %d \n", obj->c->channel_layout);
        printf("api_create_audio_codec_handle: obj->c->channels= %d \n", obj->c->channels);
        printf("api_create_audio_codec_handle: obj->out_channel_layout= %d \n", obj->out_channel_layout);
        printf("api_create_audio_codec_handle: obj->frame->sample_rate=%d \n", obj->frame->sample_rate);

#if 0
        /* the codec gives us the frame size, in samples,
         * we calculate the size of the samples buffer in bytes */
        buffer_size = av_samples_get_buffer_size(NULL, obj->c->channels, obj->c->frame_size,
                                             obj->c->sample_fmt, 0);
        if (buffer_size < 0) {
            fprintf(stderr, "Could not get sample buffer size\n");
            return -1;
        }
        samples = av_malloc(buffer_size);
        if (!samples) {
            fprintf(stderr, "Could not allocate %d bytes for samples buffer\n",
                    buffer_size);
            return -1;
        }
        /* setup the data pointers in the AVFrame */
        ret = avcodec_fill_audio_frame(obj->frame, obj->c->channels, obj->c->sample_fmt,
                                   (const uint8_t*)samples, buffer_size, 0);
        if (ret < 0) {
            fprintf(stderr, "Could not setup audio frame\n");
            return -1;
        }
#endif
    }
    else
    {
        char filename[256] = "";
        GetvalueStr(obj->json, "filename", filename);
        if (strcmp(filename, ""))
        {
            obj->fp = fopen(filename, "wb");
        }
        printf("api_audio_codec_init: filename=%s \n", filename);

        /* find the mpeg audio decoder */
        obj->codec = avcodec_find_decoder(obj->codec_id);
        if (!obj->codec) {
            fprintf(stderr, "api_audio_codec_init: 2: Codec not found\n");
            return -1;
        }

        obj->c = avcodec_alloc_context3(obj->codec);
        if (!obj->c) {
            fprintf(stderr, "Could not allocate audio codec context\n");
            return -1;
        }

        /* open it */
        if (avcodec_open2(obj->c, obj->codec, NULL) < 0) {
            fprintf(stderr, "Could not open codec\n");
            return -1;
        }
        obj->frame = av_frame_alloc();
        if (!obj->frame) {
            fprintf(stderr, "Could not allocate audio frame\n");
            return -1;
        }
        //obj->c->channel_layout = obj->out_channel_layout;
        //obj->c->sample_fmt = obj->sample_fmt;
        //obj->c->frame_size = obj->out_nb_samples;
        //obj->frame->nb_samples     = obj->c->frame_size;
        //obj->frame->format         = obj->c->sample_fmt;
        //obj->frame->channel_layout = obj->c->channel_layout;
        printf("api_create_audio_codec_handle:obj->frame->sample_rate=%d \n", obj->frame->sample_rate);
        printf("api_create_audio_codec_handle: obj->c->frame_size= %d \n", obj->c->frame_size);
        printf("api_create_audio_codec_handle: obj->c->sample_fmt= %d \n", obj->c->sample_fmt);
        printf("api_create_audio_codec_handle: obj->c->channel_layout= %d \n", obj->c->channel_layout);
        printf("api_create_audio_codec_handle: obj->c->channels= %d \n", obj->c->channels);
        printf("api_create_audio_codec_handle: obj->out_channel_layout= %d \n", obj->out_channel_layout);
        obj->audio_convert_ctx = NULL;
        av_init_packet(&obj->pkt);
    }
    //obj->c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;//test
    return ret;
}

#if 1
int resample_init(AudioCodecObj *obj)
{
    int ret = 0;
    if(!obj->audio_convert_ctx)
    {

        obj->audio_convert_ctx = swr_alloc();
        if (obj->audio_convert_ctx == NULL)
        {
            printf("Could not allocate SwrContext\n");
            return -1;
        }
        printf("resample_init:obj->frame->sample_rate=%d \n", obj->frame->sample_rate);
        printf("resample_init:obj->frame->channels=%d \n", obj->frame->channels);
        printf("resample_init:obj->frame->format=%d \n", obj->frame->format);
        printf("resample_init:obj->out_channels=%d \n", obj->out_channels);
        printf("resample_init:obj->out_sample_rate=%d \n", obj->out_sample_rate);
        printf("resample_init:obj->sample_fmt=%d \n", obj->sample_fmt);
        /* set options */
        av_opt_set_int       (obj->audio_convert_ctx, "in_channel_count",   obj->frame->channels,       0);
        av_opt_set_int       (obj->audio_convert_ctx, "in_sample_rate",     obj->frame->sample_rate,    0);
        av_opt_set_sample_fmt(obj->audio_convert_ctx, "in_sample_fmt",      obj->frame->format, 0);
        av_opt_set_int       (obj->audio_convert_ctx, "out_channel_count",  obj->out_channels,       0);
        av_opt_set_int       (obj->audio_convert_ctx, "out_sample_rate",    obj->out_sample_rate,    0);
        av_opt_set_sample_fmt(obj->audio_convert_ctx, "out_sample_fmt",     obj->sample_fmt,     0);

        /* initialize the resampling context */
        if ((ret = swr_init(obj->audio_convert_ctx)) < 0) {
            fprintf(stderr, "resample_init: Failed to initialize the resampling context\n");
            return -1;
        }
        ret = 1;
    }
    return ret;
}
HCSVC_API
int api_audio_codec_one_frame(char *handle, char *data, char *param, char *outbuf, char *outparam[])
{
    int ret = 0;
    const char *tag = "string come from c code: api_audio_codec_one_frame";
    cJSON *json;

    long long *testp = (long long *)handle;
    AudioCodecObj *obj = (AudioCodecObj *)testp[0];

    if (obj->c != NULL)
    {
        json = (cJSON *)api_str2json(param);
        obj->param = (void *)json;
        int insize = GetvalueInt(json, "insize");
        int get_pkt = GetvalueInt(json, "get_pkt");
        //printf("api_audio_codec_one_frame: insize= %d \n", insize);
        int testflag = 0;
        if(!obj->codec_type)
        {
            int got_output = 0;
            //AVPacket pkt;
            av_init_packet(&obj->pkt);
            obj->pkt.data = NULL; // packet data will be allocated by the encoder
            obj->pkt.size = 0;

            //printf("api_audio_codec_one_frame: obj->codec_type= %d \n", obj->codec_type);

            //obj->frame->data[0] = data;
            //obj->frame->linesize[0] = insize;
            ret = avcodec_fill_audio_frame( obj->frame,
                                            obj->c->channels,
                                            obj->c->sample_fmt,
                                            (const uint8_t*)data,
                                            insize, 0);
            ret = avcodec_encode_audio2(obj->c, &obj->pkt, obj->frame, &got_output);
            if (ret < 0)
            {
                fprintf(stderr, "api_audio_codec_one_frame: Error encoding audio frame\n");
                return -1;
            }
            if(obj->print)
            {
                printf("api_audio_codec_one_frame:obj->frame->sample_rate=%d \n", obj->frame->sample_rate);
                printf("api_audio_codec_one_frame:obj->frame->nb_samples=%d \n", obj->frame->nb_samples);
                printf("api_audio_codec_one_frame:obj->frame->format=%d \n", obj->frame->format);
                printf("api_audio_codec_one_frame:obj->frame->channel_layout=%d \n", obj->frame->channel_layout);
                printf("api_audio_codec_one_frame:obj->frame->channels=%d \n", obj->frame->channels);

                printf("api_audio_codec_one_frame:obj->frame->linesize[0]=%d \n", obj->frame->linesize[0]);
                printf("api_audio_codec_one_frame:obj->frame->linesize[1]=%d \n", obj->frame->linesize[1]);
                printf("api_audio_codec_one_frame:obj->frame->linesize[2]=%d \n", obj->frame->linesize[2]);
                printf("api_audio_codec_one_frame:obj->frame->data[0]=%x \n", obj->frame->data[0]);
                printf("api_audio_codec_one_frame:obj->frame->data[1]=%x \n", obj->frame->data[1]);
                printf("api_audio_codec_one_frame:obj->frame->data[2]=%x \n", obj->frame->data[2]);
            }

            //printf("api_audio_codec_one_frame: ret= %d \n", ret);
            //printf("api_audio_codec_one_frame: got_output= %d \n", got_output);
            //printf("api_audio_codec_one_frame: pkt.size= %d \n", pkt.size);
            if (got_output)
            {
                if(obj->stream)
                {
                    WiteFrame2(obj->stream, &obj->pkt, kIsAudio);
                }
                //fwrite(pkt.data, 1, pkt.size, f);
                memcpy(outbuf, obj->pkt.data, obj->pkt.size);
                //printf("api_audio_codec_one_frame: obj->c->extradata_size= %d \n", obj->c->extradata_size);
                ret = obj->pkt.size;
                if(!get_pkt)
                {
                    av_free_packet(&obj->pkt);
                }
            }
        }
        else
        {
            //AVPacket pkt;
            //av_init_packet(&pkt);
            /* decode until eof */
            obj->pkt.data = data;
            obj->pkt.size = insize;
            int offset = 0;
            while (obj->pkt.size > 0)
            {
                int i, ch;
                int got_frame = 0;

                //if (!decoded_frame) {
                //    if (!(decoded_frame = av_frame_alloc())) {
                //        fprintf(stderr, "Could not allocate audio frame\n");
                //        exit(1);
                //    }
                //}

                int len = avcodec_decode_audio4(obj->c, obj->frame, &got_frame, &obj->pkt);
                //printf("api_audio_codec_one_frame:avcodec_decode_audio4: len=%d \n", len);
                //printf("api_audio_codec_one_frame: obj->stream=%x \n", obj->stream);
                if(obj->stream)
                {
                    //printf("video_decode_frame: obj->c->sample_rate= %d \n", obj->c->sample_rate);
                    WiteFrame2(obj->stream, &obj->pkt, kIsAudio);
                }
                if (len < 0) {
                    fprintf(stderr, "api_audio_codec_one_frame: Error while decoding\n");
                    return -1;
                }
                //printf("api_audio_codec_one_frame: len= %d \n", len);
                //printf("api_audio_codec_one_frame: got_frame= %d \n", got_frame);
                if (got_frame)
                {
                    /* if a frame has been decoded, output it */
                    int data_size = av_get_bytes_per_sample(obj->c->sample_fmt);
                    if (data_size < 0)
                    {
                        /* This should not occur, checking just for paranoia */
                        fprintf(stderr, "Failed to calculate data size\n");
                        return -1;
                    }
                    //for (i = 0; i < obj->frame->nb_samples; i++)
                    //    for (ch=0; ch<c->channels; ch++)
                    //        fwrite(decoded_frame->data[ch] + data_size*i, 1, data_size, outfile);
                    if(obj->print)
                    {
                        printf("api_audio_codec_one_frame:obj->frame->sample_rate=%d \n", obj->frame->sample_rate);
                        printf("api_audio_codec_one_frame:obj->frame->nb_samples=%d \n", obj->frame->nb_samples);
                        printf("api_audio_codec_one_frame:obj->frame->format=%d \n", obj->frame->format);
                        printf("api_audio_codec_one_frame:obj->frame->channel_layout=%d \n", obj->frame->channel_layout);
                        printf("api_audio_codec_one_frame:obj->frame->channels=%d \n", obj->frame->channels);

                        printf("api_audio_codec_one_frame:obj->frame->linesize[0]=%d \n", obj->frame->linesize[0]);
                        printf("api_audio_codec_one_frame:obj->frame->linesize[1]=%d \n", obj->frame->linesize[1]);
                        printf("api_audio_codec_one_frame:obj->frame->linesize[2]=%d \n", obj->frame->linesize[2]);
                        printf("api_audio_codec_one_frame:obj->frame->data[0]=%x \n", obj->frame->data[0]);
                        printf("api_audio_codec_one_frame:obj->frame->data[1]=%x \n", obj->frame->data[1]);
                        printf("api_audio_codec_one_frame:obj->frame->data[2]=%x \n", obj->frame->data[2]);
                        printf("api_audio_codec_one_frame:data_size=%d \n", data_size);
                    }

                    int ret2 = resample_init(obj);
                    if(ret2 >= 0)
                    {
                        if(ret2 == 1)
                        {
                            testflag = 1;
                            printf("api_audio_codec_one_frame: 0 \n");
                            printf("api_audio_codec_one_frame: insize=%d \n", insize);
                            printf("api_audio_codec_one_frame: obj->out_nb_samples=%d \n", obj->out_nb_samples);
                            printf("api_audio_codec_one_frame: obj->frame->nb_samples=%d \n", obj->frame->nb_samples);
                            //printf("api_audio_codec_one_frame: obj->frame->data[0]=%x \n", obj->frame->data[0]);
                            printf("api_audio_codec_one_frame: obj->frame->linesize[0]=%d \n", obj->frame->linesize[0]);
                            printf("api_audio_codec_one_frame: obj->audio_convert_ctx=%x \n", obj->audio_convert_ctx);
                        }
                        uint8_t *dst = (uint8_t *)&outbuf[offset];
                        int ret3 = swr_convert( obj->audio_convert_ctx,
                                            &dst,
                                            obj->out_nb_samples,
                                            (const uint8_t **)obj->frame->data,
                                            //obj->out_nb_samples);
                                            obj->frame->nb_samples);

                        int out_nb_samples = ret3;
                        if(ret2 == 1)
                        {
                            printf("api_audio_codec_one_frame: ret3=%d \n", ret3);
                        }
                        obj->out_size = av_samples_get_buffer_size( NULL,
                                                                    obj->out_channels,
                                                                    out_nb_samples,
                                                                    obj->sample_fmt, 1);
                        if(obj->fp)
                        {
                            //printf("api_audio_codec_one_frame:obj->out_size=%d \n", obj->out_size);
                            fwrite(dst, 1, obj->out_size, obj->fp);
                        }
                        offset += obj->out_size;
                        ret = offset;
                        if(ret2 == 1)
                        {
                            printf("api_audio_codec_one_frame: obj->out_size=%d \n", obj->out_size);
                        }
                    }
                    //ret = data_size;
                    //memcpy(&outbuf[offset], obj->frame->data[0], data_size);
                    //offset += data_size;
                }
                obj->pkt.size -= len;
                obj->pkt.data += len;
                obj->pkt.dts =
                obj->pkt.pts = AV_NOPTS_VALUE;
                if(obj->pkt.size > 0)
                {
                    printf("error: api_audio_codec_one_frame: obj->pkt.size=%d \n", obj->pkt.size);
                    //暂不支持多帧
                    break;
                }
            }
        }
        api_json_free(obj->param);
        obj->param = NULL;
        if(testflag)
        {
            printf("api_audio_codec_one_frame: ok \n");
        }
    }
    return ret;
}
//==================================added by gxh=================================

HCSVC_API
int api_create_codec_handle(char *handle)
{
    long long ret = 0;
    if(handle)
    {
        CodecObj *obj = (CodecObj *)calloc(1, sizeof(CodecObj));

        ret = (long long)obj;
        //handle = (void *)obj;
        int handle_size = sizeof(long long);
        printf("api_create_codec_handle: handle_size= %d \n", handle_size);
        printf("api_create_codec_handle: obj= %x \n", obj);
        printf("api_create_codec_handle: ret= %x \n", ret);
        memcpy(handle, &ret, handle_size);
        long long *testp = (long long *)handle;
        printf("api_create_codec_handle: testp[0]= %x \n", testp[0]);
        int id = (int)obj;//long long ???
        obj->Obj_id = id;
        ret = id;
    }
    return (int)ret;
}
HCSVC_API
int api_free_codec_handle(char *handle)
{
    long long ret = 0;
    if(handle)
    {
        long long *testp = (long long *)handle;
        CodecObj *obj = (CodecObj *)testp[0];
        ret = obj->Obj_id;
        if(obj->resortObj)
        {
            if(obj->resortObj->recv_buf)
            {
                for(int i = 0; i < obj->resortObj->buf_size; i++)
                {
                    free(obj->resortObj->recv_buf[i]);
                }
                free(obj->resortObj->recv_buf);
            }
            free(obj->resortObj);
            obj->resortObj = NULL;
        }
        if(obj->rtpObj)
        {
            free(obj->rtpObj);
            obj->rtpObj = NULL;
        }
        if(obj->fecEncObj)
        {
            free(obj->fecEncObj);
            obj->fecEncObj = NULL;
        }
        if(obj->fecDecObj)
        {
            free(obj->fecDecObj);
            obj->fecDecObj = NULL;
        }
        if(obj->pktManObj)
        {
            free(obj->pktManObj);
            obj->pktManObj = NULL;
        }
        if(obj->osd_buf)
        {
            free(obj->osd_buf);
            obj->osd_buf = NULL;
        }
        free(obj);
        int handle_size = sizeof(long long);
        memset(handle, 0, handle_size);
    }
    return (int)ret;
}

void ResetObj(CodecObj *obj)
{
    obj->Obj_id = -1;
    obj->resortObj = NULL;
    obj->rtpObj = NULL;
    obj->fecEncObj = NULL;
    obj->fecDecObj = NULL;
    obj->pktManObj = NULL;
    obj->ppObj = NULL;
    obj->logfp = NULL;
    obj->codec_id = AV_CODEC_ID_H264;
    obj->frame_idx = 0;
    obj->c = NULL;
    obj->param = NULL;
    obj->frame = av_frame_alloc();
    obj->hw_frame = av_frame_alloc();
    obj->brctrl.qp = 0;
    obj->brctrl.base_qp = 0;
    obj->brctrl.bytes_last = 0;
    obj->brctrl.bytes_target_last = 0;
    obj->brctrl.bytes_sum = 0;
    obj->brctrl.bytes_target_sum = 0;
    for(int i = 0; i < 4; i++)
    {
        memset(obj->outparam[i], 0, sizeof(char) * MAX_OUTCHAR_SIZE);
    }
    obj->f = NULL;
    obj->f2 = NULL;
}

int resetparam(void *hnd)
{
    CodecObj *obj = (CodecObj *)hnd;
    cJSON *json = (cJSON *)obj->param;
    int ret = 0;
    char x264opts[1024] = "";

    /* put sample parameters */
    obj->c->bit_rate = GetvalueInt(json, "bit_rate");//400000;
    /* resolution must be a multiple of two */
    obj->c->width = GetvalueInt(json, "width");//352;
    obj->c->height = GetvalueInt(json, "height");//288;
    /* frames per second */
    obj->c->time_base = (AVRational){1,GetvalueInt(json, "frame_rate")};//25
    /* emit one intra frame every ten frames
     * check frame pict_type before passing frame
     * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
     * then gop_size is ignored and the output of encoder
     * will always be I frame irrespective to gop_size
     */
    //obj->c->b_fast_pskip = 1;//test
    //obj->c->flags |= AV_CODEC_FLAG2_FAST;//AV_CODEC_FLAG2_FASTPSKIP;
    obj->c->frame_skip_threshold = 20;//4000;
    obj->c->frame_skip_cmp = 1;
#if 1
    obj->c->gop_size = MAX_GOP_SIZE;//10000;
#else
    obj->c->gop_size = GetvalueInt(json, "gop_size");//10;
#endif
    obj->c->keyint_min = obj->c->gop_size;
    obj->c->max_b_frames = GetvalueInt(json, "max_b_frames");//1;

    obj->c->coder_type = FF_CODER_TYPE_VLC;
    int coder_type = GetvalueInt(json, "coder_type");
    if(coder_type > 0)
    {
        if(coder_type == 1)
        {
            obj->c->coder_type = FF_CODER_TYPE_VLC;
        }
        else if(coder_type == 2)
        {
            obj->c->coder_type = FF_CODER_TYPE_AC;
        }
        else{
            obj->c->coder_type = obj->c->max_b_frames ? FF_CODER_TYPE_AC : FF_CODER_TYPE_VLC;
        }
    }
    int refs = GetvalueInt(json, "refs");
    if(refs > 0)
    {
        obj->c->refs = refs;
    }
	obj->c->b_frame_strategy = false;
    obj->c->pix_fmt = AV_PIX_FMT_YUV420P;

    //if (codec_id == AV_CODEC_ID_H264)
    //    av_opt_set(obj->c->priv_data, "preset", "slow", 0);
#if 0
    if(obj->brctrl.qp > 0)
    {
        obj->c->qmax = obj->brctrl.qp;
	    obj->c->qmin = obj->brctrl.qp;
    }
    else
    {
        obj->c->qmax = GetvalueInt(json, "qmax");//40;
	    obj->c->qmin = GetvalueInt(json, "qmin");//20;// 26;
	    if(obj->c->qmax == obj->c->qmin)
	    {
	        //c->cqp = c->qmax;
	        //obj->c->bit_rate = 0;
	    }
    }
#endif

#if 1
    int qp = GetvalueInt(json, "qp");

    if(obj->brctrl.qp > 0)
    {
        //char text[128] = "qp=";//"crf=27"
        char ctmp[32] = "";
	    sprintf(ctmp, "%d", obj->brctrl.qp);
        if(strlen(x264opts))
	    {
	        strcat(x264opts, ":");
	    }
        strcat(x264opts, "qp=");
        strcat(x264opts, ctmp);
        //av_opt_set(obj->c->priv_data, "x264opts",text,0);
        {
            //char text2[128] = ":qpstep=";//"crf=27"
            char ctmp[32] = "";
            int qpstep = 10;
            strcat(x264opts, ":qpstep=");
	        sprintf(ctmp, "%d", qpstep);
            strcat(x264opts, ctmp);
            //strcat(text, text2);
            //av_opt_set(obj->c->priv_data, "x264opts",x264opts,0);
            if(obj->codec_mode == 2)
            {
                av_opt_set(obj->c->priv_data, "wz264opts",x264opts,0);
            }
            else{
                av_opt_set(obj->c->priv_data, "x264opts",x264opts,0);
            }
            //printf("resetparam: text= %s \n", text);
        }
    }
    else if(qp >= 0)
    {
        //char text[128] = "qp=";//"crf=27"
        char ctmp[32] = "";
        if(strlen(x264opts))
	    {
	        strcat(x264opts, ":");
	    }
        strcat(x264opts, "qp=");
	    sprintf(ctmp, "%d", qp);
        strcat(x264opts, ctmp);
        //av_opt_set(obj->c->priv_data, "x264opts",text,0);
        //printf("resetparam: text= %s \n", text);
        {
            //char text2[128] = ":qpstep=";//"crf=27"
            char ctmp[32] = "";
            int qpstep = 10;
            strcat(x264opts, ":qpstep=");
	        sprintf(ctmp, "%d", qpstep);
            strcat(x264opts, ctmp);
            //strcat(text, text2);
            //av_opt_set(obj->c->priv_data, "x264opts",x264opts,0);
            if(obj->codec_mode == 2)
            {
                av_opt_set(obj->c->priv_data, "wz264opts",x264opts,0);
            }
            else{
                av_opt_set(obj->c->priv_data, "x264opts",x264opts,0);
            }
        }
    }
#endif
    //obj->c->thread_type = FF_THREAD_SLICE; //无B帧时，存在bug，卡顿

    int thread_type = GetvalueInt(json, "thread_type");
    int thread_count = GetvalueInt(json, "thread_count");
    if (thread_type > 0)
    {
        //FF_THREAD_FRAME: 1
        //FF_THREAD_SLICE: 2
        obj->c->thread_type = thread_type == 2 ? FF_THREAD_SLICE : FF_THREAD_FRAME;
        if (thread_count > 0)
        {
            obj->c->thread_count = thread_count;
            //printf("obj->c->thread_type %d \n", obj->c->thread_type);
            //printf("obj->c->thread_count %d \n", obj->c->thread_count);
        }
    }
    char ctmp[64] = "";
    GetvalueStr(json, "preset", ctmp);
    av_opt_set(obj->c->priv_data, "preset", ctmp, 0);//"superfast"
    memset(ctmp, 0, 64 * sizeof(char));
    GetvalueStr(json, "tune", ctmp);
    av_opt_set(obj->c->priv_data, "tune", ctmp, 0);//"zerolatency"
    //char ctmp[32] = "1400";
	//char text[128] = "slice-max-size=";
	//int mtu_size = 1400;
	//itoa(mtu_size, ctmp, 10);
	//sprintf(ctmp, 0, "%d", mtu_size);
	//strcat(text, ctmp);
    ///obj->c->rtp_payload_size = 1400;
    memset(ctmp, 0, 64 * sizeof(char));
	GetvalueStr(json, "slice-max-size", ctmp);
	if (strcmp(ctmp, ""))
	{
	    //char text[128] = "slice-max-size=";
        if(strlen(x264opts))
	    {
	        strcat(x264opts, ":");
	    }
	    strcat(x264opts, "slice-max-size=");
        strcat(x264opts, ctmp);
	    //printf("text %s \n", text);
	    //av_opt_set(obj->c->priv_data, "x264opts",x264opts,0);
	}
	if (1)
	{
	    //char text[128] = "slice-max-size=";
        if(strlen(x264opts))
	    {
	        strcat(x264opts, ":");
	    }
	    //strcat(x264opts, "no-scenecut=1");
	    strcat(x264opts, "scenecut=0");
	    //strcat(x264opts, "profile=baseline:level=3.0");
        //
	    //av_opt_set(obj->c->priv_data, "x264opts",x264opts,0);
	    //av_opt_set(obj->c->priv_data, "profile", "main", 0);
	}
	if(strlen(x264opts))
	{
	    //av_opt_set(obj->c->priv_data, "x264opts",x264opts,0);
	    if(obj->codec_mode == 2)
        {
            av_opt_set(obj->c->priv_data, "wz264opts",x264opts,0);
        }
        else{
            av_opt_set(obj->c->priv_data, "x264opts",x264opts,0);
        }
	}
	int ref_idx = GetvalueInt(json, "ref_idx");
	if(ref_idx > 0)
	{
	    obj->c->ref_idx = ref_idx;
	}
    int skip_frame = GetvalueInt(json, "skip_frame");
    //printf("resetparam: skip_frame=%d \n", skip_frame);
    obj->c->skip_frame = 0;
    if(skip_frame > 0)
	{
	    obj->c->skip_frame = skip_frame;
	}
	//printf("resetparam: obj->c->skip_frame=%d \n", obj->c->skip_frame);
    return ret;
}

int enc_resize(CodecObj *obj)
{
    int ret = -1;
    //printf("enc_resize: \n");
    AVCodecContext *c = obj->c;
    //AVFrame *frame = obj->frame;
    if(obj->frame)
    {
        av_frame_free(&obj->frame);
        obj->frame = NULL;
        obj->frame = av_frame_alloc();
    }
    //printf("enc_resize: c->width=%d \n", c->width);
    int frame_size = ((c->width * c->height * 3) >> 1);
    if(obj->osd_buf)
    {
        free(obj->osd_buf);
        obj->osd_buf = calloc(1, frame_size);
    }
    if(obj->is_hw)
    {
        obj->frame->width  = c->width;
        obj->frame->height = c->height;
        obj->frame->format = AV_PIX_FMT_NV12;
        if ((ret = av_frame_get_buffer(obj->frame, 0)) < 0)
            return ret;
        return ret;
    }

    obj->frame->format = c->pix_fmt;
    obj->frame->width  = c->width;
    obj->frame->height = c->height;

    /* the image can be allocated by any means and av_image_alloc() is
     * just the most convenient way if av_malloc() is to be used */
    //ret = av_image_alloc(frame->data, frame->linesize, c->width, c->height, c->pix_fmt, 32);
    int aline_size = 16;
    if((c->width % 128) == 0)
    {
        aline_size = 64;
    }
    else if((c->width % 64) == 0)
    {
        aline_size = 32;
    }
    else if((c->width % 32) == 0)
    {
        aline_size = 16;
    }
    else if((c->width % 16) == 0)
    {
        aline_size = 8;
    }
    else if((c->width % 8) == 0)
    {
        aline_size = 4;
    }
    else if((c->width % 4) == 0)
    {
        aline_size = 2;
    }
    else if((c->width % 2) == 0)
    {
        aline_size = 1;
    }
    ret = av_image_alloc(obj->frame->data, obj->frame->linesize, c->width, c->height, c->pix_fmt, aline_size);
    if (ret < 0) {
        printf("enc_resize: c->width=%d \n", c->width);
        printf("enc_resize: c->height=%d \n", c->height);
        fprintf(stderr, "enc_resize: Could not allocate raw picture buffer\n");
    }
    return ret;
}
AVCodecContext *encode_open(void *hnd)
{
    CodecObj *obj = (CodecObj *)hnd;
    int codec_id = obj->codec_id;
    AVFrame *frame = obj->frame;
    AVCodec *codec = NULL;
    AVCodecContext *c= NULL;
    int i, ret, x, y, got_output;
    FILE *f;
    cJSON *json = (cJSON *)obj->param;
    //AVFrame *frame;
    //AVPacket pkt;
    uint8_t endcode[] = { 0, 0, 1, 0xb7 };
    char x264opts[1024] = "";
    char *h264_names[3] = {"h264_qsv", "h264_vaapi", "h264_nvenc"};//"libx264"
    char *h265_names[3] = {"hevc_qsv", "hevc_vaapi", "hevc_nvenc"};

    int width = GetvalueInt(json, "width");
    int height = GetvalueInt(json, "height");
    obj->target_framerate = GetvalueInt(json, "frame_rate");
    obj->new_framerate = obj->target_framerate;
    printf("encode_open:  obj->codec_mode=%d \n", obj->codec_mode);
    if(obj->codec_mode > 0)
    {
        char *enc_name;
        if(obj->codec_mode == 1)//hw264
        {
            for(i = 0; i < 3; i++)
            {
                codec = find_codec_by_name(h264_names[i], 1);
                if(codec)
                {
                    enc_name = h264_names[i];
                    printf("encode_open: h264_names[i]=%s \n", h264_names[i]);
                    break;
                }
            }
        }
        else if(obj->codec_mode == 3)//hw265
        {
            for(i = 0; i < 3; i++)
            {
                codec = find_codec_by_name(h265_names[i], 1);
                if(codec)
                {
                    enc_name = h264_names[i];
                    printf("encode_open: h264_names[i]=%s \n", h264_names[i]);
                    break;
                }
            }
        }
        if(!codec)
        {
        }
        else{
            AVCodecContext params = {};
            params.width     = width;
            params.height    = height;
            params.time_base = (AVRational){1, obj->target_framerate};
            params.framerate = (AVRational){obj->target_framerate, 1};
            params.sample_aspect_ratio = (AVRational){1, 1};
            //params.pix_fmt   = AV_PIX_FMT_VAAPI;
            params.pix_fmt   = AV_PIX_FMT_QSV;//注意:N卡使用AV_PIX_FMT_CUDA
            //params.pix_fmt   = AV_PIX_FMT_YUV420P;
            //
            params.bit_rate = GetvalueInt(json, "bit_rate");
            if(!strcmp(enc_name, "h264_vaapi") || !strcmp(enc_name, "h264_qsv"))
            {
                printf("hw_encode_init: enc_name=%s \n", enc_name);
                //注意:早期的elecard不支持high profile,可用264visa分析流;
                //或者控制profile为baseline或main profile
                char ctmp[64] = "";
                GetvalueStr(json, "slice-max-size", ctmp);
                int slice_max_size = atoi(ctmp);//1100;//MTU_SIZE
                av_opt_set_int(params.priv_data, "max_slice_size", slice_max_size, 0);//max_slice_size

                params.profile = FF_PROFILE_H264_MAIN;//FF_PROFILE_H264_BASELINE;//
                params.gop_size = GetvalueInt(json, "gop_size");
                params.keyint_min = params.gop_size;
                params.max_b_frames = GetvalueInt(json, "max_b_frames");
                //av_opt_set(params.priv_data, "preset", "veryfast", 0);//veryslow//veryfast//medium
                //av_opt_set(params.priv_data, "tune", "zerolatency", 0);
                //av_opt_set(params.priv_data, "preset", GetvalueStr(json, "preset"), 0);//"superfast"
                av_opt_set(params.priv_data, "preset", "veryfast", 0);
                GetvalueStr(json, "tune", ctmp);
                av_opt_set(params.priv_data, "tune", ctmp, 0);//"zerolatency"
                int coder_type = GetvalueInt(json, "coder_type");
                if(coder_type > 0)
                {
                    if(coder_type == 1)
                    {
                        params.coder_type = FF_CODER_TYPE_VLC;
                    }
                    else if(coder_type == 2)
                    {
                        params.coder_type = FF_CODER_TYPE_AC;
                    }
                    else{
                        params.coder_type = c->max_b_frames ? FF_CODER_TYPE_AC : FF_CODER_TYPE_VLC;
                    }
                }
                params.qmax = GetvalueInt(json, "qmax");
	            params.qmin = GetvalueInt(json, "qmin");
            }
            ret = hw_encode_init(&params, codec, &obj->hw_device_ctx, &obj->c, enc_name, width, height);
            //ret = hw_encode_init(NULL, codec, &obj->hw_device_ctx, &obj->c, enc_name, width, height);
            if(ret < 0)
            {
                printf("error: encode_open: ret=%d, codec=%x \n", ret, codec);
            }
            else{
                obj->is_hw = 1;
                enc_resize(obj);
                return obj->c;
            }

        }
    }
    if(obj->codec_mode == 2)
    {
        printf("encode_open: libwz264 \n");
        codec = avcodec_find_encoder_by_name("libwz264");
        if (!codec) {
            fprintf(stderr, "encode_open: Codec not found\n");

        }
    }
    if(!codec)
    {
        obj->codec_mode = 0;
        /* find the mpeg1 video encoder */
        codec = avcodec_find_encoder(codec_id);
        if (!codec) {
            fprintf(stderr, "encode_open: Codec not found\n");
            return NULL;
        }
    }
    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        return c;
    }
#if 0
    //cbr
    c->bit_rate = br;
    c->rc_min_rate =br;
    c->rc_max_rate = br;
    c->bit_rate_tolerance = br;
    c->rc_buffer_size=br;
    c->rc_initial_buffer_occupancy = c->rc_buffer_size*3/4;
    c->rc_buffer_aggressivity= (float)1.0;
    c->rc_initial_cplx= 0.5;
    //vbr
    c->flags |= CODEC_FLAG_QSCALE;
    c->rc_min_rate =min;
    c->rc_max_rate = max;
    c->bit_rate = br;
#endif
    /* put sample parameters */
    c->bit_rate = GetvalueInt(json, "bit_rate");//400000;
    /* resolution must be a multiple of two */
    c->width = GetvalueInt(json, "width");//352;
    c->height = GetvalueInt(json, "height");//288;
    /* frames per second */
    c->time_base = (AVRational){1,obj->target_framerate};//25
    /* emit one intra frame every ten frames
     * check frame pict_type before passing frame
     * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
     * then gop_size is ignored and the output of encoder
     * will always be I frame irrespective to gop_size
     */
#if 1
    c->gop_size = MAX_GOP_SIZE;//10000;
#else
    c->gop_size = GetvalueInt(json, "gop_size");//10;
#endif
    c->keyint_min = c->gop_size;
    c->max_b_frames = GetvalueInt(json, "max_b_frames");//1;

    c->coder_type = FF_CODER_TYPE_VLC;
    int coder_type = GetvalueInt(json, "coder_type");
    if(coder_type > 0)
    {
        if(coder_type == 1)
        {
            c->coder_type = FF_CODER_TYPE_VLC;
        }
        else if(coder_type == 2)
        {
            c->coder_type = FF_CODER_TYPE_AC;
        }
        else{
            c->coder_type = c->max_b_frames ? FF_CODER_TYPE_AC : FF_CODER_TYPE_VLC;
        }
    }
    int refs = GetvalueInt(json, "refs");
    if(refs > 0)
    {
        c->refs = refs;
    }
	c->b_frame_strategy = false;
    c->pix_fmt = AV_PIX_FMT_YUV420P;

    if (codec_id == AV_CODEC_ID_H264)
        av_opt_set(c->priv_data, "preset", "slow", 0);
    //added by gxh
    //c->gop_size = 50;
#if 0
    if(obj->brctrl.qp > 0)
    {
        c->qmax = obj->brctrl.qp;
	    c->qmin = obj->brctrl.qp;
    }
    else
    {
        c->qmax = GetvalueInt(json, "qmax");//40;
	    c->qmin = GetvalueInt(json, "qmin");//20;// 26;
	    if(c->qmax == c->qmin)
	    {
	        //c->qp = c->qmax;
	        //obj->c->bit_rate = 0;
	    }
    }
#endif

    if(obj->codec_mode != 2)
    {
        //无skipframe下，保持码率平稳，则关闭一下代码
        c->qmax = 50;
	    c->qmin = 20;
	    c->qmax = GetvalueInt(json, "qmax");//40;
	    c->qmin = GetvalueInt(json, "qmin");//20;// 26;
        printf("c->qmax %d \n", c->qmax);
        printf("c->qmin %d \n", c->qmin);
    }

#if 1//20210410
    //打开则会导致宏块qp变化失效
    //if(c->max_b_frames == 0)
    //if(refs > 1)
    if(obj->codec_mode != 2)
    {
        //帧内QP不变
        int qp = GetvalueInt(json, "qp");
        if(obj->brctrl.qp > 0)
        {
            //char text[128] = "qp=";//"crf=27"
            char ctmp[32] = "";
    	    sprintf(ctmp, "%d", obj->brctrl.qp);
	        if(strlen(x264opts))
	        {
	            strcat(x264opts, ":");
	        }
	        strcat(x264opts, "qp=");
            strcat(x264opts, ctmp);
            //av_opt_set(c->priv_data, "x264opts",text,0);
            {
                //char text2[128] = ":qpstep=";//"crf=27"
                char ctmp[32] = "";
                int qpstep = 10;
                strcat(x264opts, ":qpstep=");
	            sprintf(ctmp, "%d", qpstep);
                strcat(x264opts, ctmp);
                //strcat(text, text2);
                if(obj->codec_mode == 2)
                {
                    av_opt_set(c->priv_data, "wz264opts",x264opts,0);
                }
                else{
                    av_opt_set(c->priv_data, "x264opts",x264opts,0);
                }

            }
        }
        else if(qp >= 0)
        {
            //char text[128] = "qp=";//"crf=27"
            char ctmp[32] = "";
            if(strlen(x264opts))
	        {
	            strcat(x264opts, ":");
	        }
	        strcat(x264opts, "qp=");
	        sprintf(ctmp, "%d", qp);
            strcat(x264opts, ctmp);
            //av_opt_set(c->priv_data, "x264opts",text,0);
            {
                //char text2[128] = ":qpstep=";//"crf=27"
                char ctmp[32] = "";
                int qpstep = 10;
                strcat(x264opts, ":qpstep=");
	            sprintf(ctmp, "%d", qpstep);
                strcat(x264opts, ctmp);
                //strcat(text, text2);
                if(obj->codec_mode == 2)
                {
                    av_opt_set(c->priv_data, "wz264opts",x264opts,0);
                }
                else{
                    av_opt_set(c->priv_data, "x264opts",x264opts,0);
                }
            }
        }
    }
#endif

    //c->thread_type = FF_THREAD_SLICE; //无B帧时，存在bug，卡顿

    int thread_type = GetvalueInt(json, "thread_type");
    int thread_count = GetvalueInt(json, "thread_count");
    if (thread_type > 0)
    {
        //FF_THREAD_FRAME: 1
        //FF_THREAD_SLICE: 2
        c->thread_type = thread_type == 2 ? FF_THREAD_SLICE : FF_THREAD_FRAME;
        if (thread_count > 0)
        {
            c->thread_count = thread_count;
            printf("c->thread_type %d \n", c->thread_type);
            printf("c->thread_count %d \n", c->thread_count);
        }
    }
    //c->frame_num = 8;
    char ctmp[64] = "";
    GetvalueStr(json, "preset", ctmp);
    av_opt_set(c->priv_data, "preset", ctmp, 0);//"superfast"
    memset(ctmp, 0, 64 * sizeof(char));
    GetvalueStr(json, "tune", ctmp);
    av_opt_set(c->priv_data, "tune", ctmp, 0);//"zerolatency"
    //char ctmp[32] = "1400";
	//char text[128] = "slice-max-size=";
	//int mtu_size = 1400;
	//itoa(mtu_size, ctmp, 10);
	//sprintf(ctmp, 0, "%d", mtu_size);
	//strcat(text, ctmp);
    ///c->rtp_payload_size = 1400;
    memset(ctmp, 0, 64 * sizeof(char));
	GetvalueStr(json, "slice-max-size", ctmp);
	if (strcmp(ctmp, ""))
	{
	    //char text[128] = "slice-max-size=";
	    if(strlen(x264opts))
	    {
	        strcat(x264opts, ":");
	    }
	    strcat(x264opts, "slice-max-size=");
	    strcat(x264opts, ctmp);
	    //printf("text %s \n", text);
	    //av_opt_set(c->priv_data, "x264opts",x264opts,0);
	}
	if (1)
	{
	    //char text[128] = "slice-max-size=";
        if(strlen(x264opts))
	    {
	        strcat(x264opts, ":");
	    }
	    //strcat(x264opts, "no-scenecut=1");
	    strcat(x264opts, "scenecut=0");
	    //strcat(x264opts, "profile=baseline:level=3.0");

	    //av_opt_set(c->priv_data, "x264opts",x264opts,0);
	    //av_opt_set(c->priv_data, "profile", "main", 0);
	}
	if(strlen(x264opts))
	{
	    if(obj->codec_mode == 2)
        {
            av_opt_set(c->priv_data, "wz264opts",x264opts,0);
        }
        else{
            av_opt_set(c->priv_data, "x264opts",x264opts,0);
        }
	}
	int ref_idx = GetvalueInt(json, "ref_idx");
	if(ref_idx > 0)
	{
	    c->ref_idx = ref_idx;
	}


#if 0
    int tff = GetvalueInt(json, "tff");
    if (tff > 0)
    {
        char text[128] = "tff=1";
        av_opt_set(c->priv_data, "x264opts",text,0);

        //av_opt_set(c->priv_data, "x264opts","interlaced=1",0);
         /* 将视频流标记为交错(隔行)，哪怕并非为交错式编码。可用于编码蓝光兼容的25p和30p视频。默认是未开启 */
        //int b_fake_interlaced;
        // "TOP" and "BOTTOM" are not supported in x264 (PAFF only)
    }
    int bff = GetvalueInt(json, "bff");
    if (bff > 0)
    {
        char text[128] = "bff=1";
        av_opt_set(c->priv_data, "x264opts",text,0);
    }
#endif
    int sdp = GetvalueInt(json, "sdp");
    if(sdp > 0)
    {
        c->flags|= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "encode_open: Could not open codec\n");
        return c;
    }

    //frame = av_frame_alloc();
    //if (!frame) {
    //    fprintf(stderr, "Could not allocate video frame\n");
    //    return c;
    //}
    printf("encode_open: 1 \n");
    obj->c = c;
    enc_resize(obj);
    printf("encode_open: 2 \n");
    //av_init_packet(&obj->pkt);
    //printf("frame->linesize[0] %d \n", frame->linesize[0]);
    //printf("frame->linesize[1] %d \n", frame->linesize[1]);
    //printf("frame->linesize[2] %d \n", frame->linesize[2]);
    //printf("obj->frame->width %5d (obj->frame->height=%5d)\n", obj->frame->width, obj->frame->height);
    //printf("frame->width %5d (frame->height=%5d)\n", frame->width, frame->height);
    return c;
}
int encode_open2(char *handle, char *param)
{
    long long ret = 0;
    cJSON *json;
    //if ((handle == NULL))
    {
        ret = (long long)api_create_codec_handle(handle);
        long long *testp = (long long *)handle;
        CodecObj *obj = (CodecObj *)testp[0];
        int id = obj->Obj_id;
        ResetObj(obj);
        obj->Obj_id = id;
#if 0
        if(!glob_lock)
        {
            glob_lock = calloc(1, sizeof(pthread_mutex_t));
            pthread_mutex_init(glob_lock, NULL);
        }
        pthread_mutex_lock(glob_lock);
        avcodec_register_all();
        av_log_set_level(AV_LOG_QUIET);
        pthread_mutex_unlock(glob_lock);

        CodecObj *obj = (CodecObj *)calloc(1, sizeof(CodecObj));

        ret = (long long)obj;
        //handle = (void *)obj;
        int handle_size = sizeof(long long);
        printf("encode_open2: handle_size= %d \n", handle_size);
        printf("encode_open2: obj= %x \n", obj);
        printf("encode_open2: ret= %x \n", ret);
        memcpy(handle, &ret, handle_size);
        long long *testp = (long long *)handle;
        printf("encode_open2: testp[0]= %x \n", testp[0]);
        avcodec_register_all();
        av_log_set_level(AV_LOG_QUIET);
        int id = (int)obj;//long long ???
        ret = id;
        ResetObj(obj);
#endif
        //if (obj->Obj_id < 0)
        {
            obj->json = (cJSON *)api_str2json(param);
            json = obj->json;
            obj->param = (void *)json;
            //
            obj->codec_mode = GetvalueInt(json, "codec_mode");
            obj->osd_enable = GetvalueInt(json, "osd");
            printf("encode_open2: obj->osd_enable=%d \n", obj->osd_enable);
            if(obj->osd_enable)
            {
                obj->orgX = GetvalueInt(obj->json, "orgX");
                obj->orgY = GetvalueInt(obj->json, "orgY");
                obj->scale = GetvalueInt(obj->json, "scale");
                obj->color = GetvalueInt(obj->json, "color");
                //char *src_pix_fmt = GetvalueStr(obj->json, "src_pix_fmt");
	            //if (strcmp(src_pix_fmt, ""))
	            //{
                //    if (!strcmp(src_pix_fmt, "AV_PIX_FMT_YUV420P"))
                //    {
                //        //obj->src_pix_fmt = AV_PIX_FMT_YUV420P;
                //    }
                //}
            }
            if(1)
            {
                obj->ppObj = calloc(1, sizeof(PreProcObj));
                int w = GetvalueInt(json, "width");
                int h = GetvalueInt(json, "height");
                int mb_w = (w >> 4) + ((w % 16) ? 1 : 0);
                int mb_h = (h >> 4) + ((h % 16) ? 1 : 0);
                int bmb_w = (mb_w >> 3) + ((mb_w % 8) ? 1 : 0);
                int bmb_h = mb_h;
                obj->ppObj->bmb_size = bmb_w * bmb_h;
                memset(obj->ppObj->skip_mb, 0, obj->ppObj->bmb_size);
                obj->ppObj->skip_frame_idx = 0;
                for(int i = 0; i < MAX_SKIP_FRAME_NUM; i++)
                {
                    obj->ppObj->last_frame[i] = NULL;
                }
            }
            //
            cJSON *item;
            item = cJSON_GetObjectItem(json, "preset");
            if(cJSON_IsString(item))
            {
                printf("Item: valuestring=%s\n", item->valuestring);
            }
            else if(cJSON_IsNumber(item))
            {
                printf("Item: valueint=%d\n", item->valueint);
            }
            printf("Item: type=%d, key=%s, valueint=%d, valuestring=%s\n", item->type, item->string, item->valueint, item->valuestring);
            item = cJSON_GetObjectItem(json, "bit_rate");
            if(cJSON_IsString(item))
            {
                printf("Item: valuestring=%s\n", item->valuestring);
            }
            else if(cJSON_IsNumber(item))
            {
                printf("Item: valueint=%d\n", item->valueint);
            }
            obj->Obj_id = id;
            int print = GetvalueInt(json, "print");
            printf("encode_open2: print=%d \n", print);
            obj->c = encode_open((void *)obj);//(obj->codec_id, (AVFrame *)&obj->frame);
            printf("obj->frame->width %5d (obj->frame->height=%5d)\n", obj->frame->width, obj->frame->height);
            //
            if(print)
            {
                const char *filename = "test.h264";
                obj->f = fopen(filename, "wb");
                //obj->f2 = fopen("/data/home/gxh/works/datashare/for_ENC/352X288/foreman_cif.yuv", "rb");
            }
        }
    }
    return (int)ret;
}
int video_encode_frame(void *hnd, AVPacket *pkt)
{
    CodecObj *obj = (CodecObj *)hnd;
    AVCodecContext *c = obj->c;
    AVFrame *frame = obj->frame;
    int ret = 0;
    int got_output = 0;
    //av_init_packet(pkt);
    pkt->data = NULL;    // packet data will be allocated by the encoder
    pkt->size = 0;
    //frame->pts = i;
    if(!obj->is_hw)
    {
        /* encode the image */
        ret = avcodec_encode_video2(c, pkt, frame, &got_output);
    }
    else{
        //obj->frame->width  = c->width;
        //obj->frame->height = c->height;
        //obj->frame->format = AV_PIX_FMT_NV12;
        //if (!(obj->frame = av_frame_alloc())) {
        //    ret = AVERROR(ENOMEM);
        //    return ret;
        //}

        AVFrame *hw_frame = NULL;
        if (!(hw_frame = av_frame_alloc())) {
            ret = AVERROR(ENOMEM);
            return ret;
        }
        int frame_size = ((c->width * c->height * 3) >> 1);
        av_new_packet(pkt, frame_size);
        //pkt->data = av_malloc(frame_size);
        //pkt->buf = pkt->data;//方便释放内存
        ret = hw_encode_one_frame(c, frame, hw_frame, NULL, pkt->data);
        av_frame_free(&hw_frame);
        //av_frame_free(&obj->frame);
        //printf("video_encode_frame: ret= %d \n", ret);
        if(ret > 0)
        {
            pkt->size = ret;
            got_output = 1;
        }

    }
    if (ret < 0) {
        fprintf(stderr, "video_encode_frame: Error encoding frame: ret=% d \n", ret);
        return -1;
    }
    if (got_output) {
        //printf("video_encode_frame: frame->pict_type=%d \n", frame->pict_type);
        //printf("Write frame %3d (size=%5d)\n", frame->pts, pkt->size);
        //fwrite(pkt->data, 1, pkt->size, f);
        //av_free_packet(&pkt);
    }
    else{
        //av_free_packet(pkt);
    }

    return got_output;
}
void encode_close(void *hnd)
{
    CodecObj *obj = (CodecObj *)hnd;
    AVCodecContext *c = obj->c;
    FILE *f = obj->f;
    if(obj->is_hw)
    {
        hw_encode_close(    obj->hw_device_ctx, c,
                            obj->frame, NULL,
                            NULL, NULL, NULL, NULL);
        obj->hw_device_ctx = NULL;
        obj->frame = NULL;
        obj->c = NULL;
        c = NULL;
    }
    if(obj->img_convert_ctx)
    {
        sws_freeContext(obj->img_convert_ctx);
        obj->img_convert_ctx = NULL;
    }

    if(c)
    {
        avcodec_close(c);
        av_free(c);
    }
    if(obj->frame)
    {
        av_frame_free(&obj->frame);
    }
    if(obj->hw_frame)
    {
        av_frame_free(&obj->hw_frame);
    }

    if(obj->json)
    {
        api_json_free(obj->json);
        obj->json = NULL;
    }
    if(obj->osd_handle)
    {
        api_simple_osd_close(obj->osd_handle);
        free(obj->osd_handle);
        obj->osd_handle = NULL;
    }
    if (f != NULL)
    {
        fclose(f);
    }
    if(obj->logfp)
    {
        fclose(obj->logfp);
        obj->logfp = NULL;
    }
    //cJSON_Delete(obj->param);
}
HCSVC_API
int api_video_encode_open(char *handle, char *param)
{
    return encode_open2(handle, param);
}
int osd_process(char *handle, char *outbuf, int codectype)
{
    int ret = 0;
    long long *testp = (long long *)handle;
    CodecObj *obj = (CodecObj *)testp[0];
	int step = 25;//50;
	//printf("osd_process: start \n");
	int skip_frame = 0;
	if(!obj->start_time)
    {
        obj->start_time = api_get_time_stamp_ll();
        obj->last_frame_num = obj->frame_num;
        obj->interval = 1500;//1s
    }
    int64_t now = api_get_time_stamp_ll();
    int difftime = (int)(now - obj->start_time);
    //printf("osd_process: 0 \n");
    if(difftime > obj->interval)
    {
        float sum_frame_num = (float)(obj->frame_num - obj->last_frame_num);
        obj->frame_rate = (int)((sum_frame_num * 1000.0) / (float)difftime + 0.5);
        obj->bits_rate = (int)((obj->sum_bits * 1000.0) / difftime) >> 10;

        obj->start_time = now;
        obj->sum_bits = 0;
        obj->last_frame_num = obj->frame_num;
    }
	if(obj->osd_enable)
	{
	    if(!obj->osd_handle)
	    {
	        obj->handle_size = 8;
	        obj->osd_handle = (char *)calloc(1, obj->handle_size * sizeof(char));

	        //printf("osd_process: 3 \n");
	        //obj->pCodecCtx->width//obj->pCodecCtx->height
            obj->json = api_renew_json_int(obj->json, "width", obj->width);
            obj->json = api_renew_json_int(obj->json, "height", obj->height);
            obj->json = api_renew_json_int(obj->json, "orgX", obj->orgX);
            obj->json = api_renew_json_int(obj->json, "orgY", obj->orgY);
            obj->json = api_renew_json_int(obj->json, "scale", obj->scale);
            obj->json = api_renew_json_int(obj->json, "color", obj->color);
            //obj->json = api_renew_json_str(obj->json, "src_pix_fmt", "AV_PIX_FMT_YUV420P");
            //printf("osd_process: 4 \n");
            char* jsonstr2 = api_json2str(obj->json);
            int ret2 = api_simple_osd_init(obj->osd_handle, jsonstr2);
            api_json2str_free(jsonstr2);
            printf("osd_process: 5 \n");
	    }
	    cJSON *json = (cJSON *)obj->param;
	    skip_frame = GetvalueInt(json, "skip_frame");
	    int framerate = obj->frame_rate;
        if(framerate > 0)
        {
            //printf("osd_process: 0 \n");
            //printf("osd_process: obj->width= %d \n", obj->width);
            //printf("osd_process: obj->height= %d \n", obj->height);
            int orgX = 0;
            int orgY = obj->height - ((obj->height * codectype) / 16);
            //printf("osd_process: orgY= %d \n", orgY);
            //orgY = (obj->height / 16);
            char context[255] = "";
            json = api_renew_json_int(json, "width", obj->width);
            json = api_renew_json_int(json, "height", obj->height);
            json = api_renew_json_int(json, "orgX", orgX);
            json = api_renew_json_int(json, "orgY", orgY);
            sprintf(&context[strlen(context)], " %d", obj->width);
            sprintf(&context[strlen(context)], "x%d", obj->height);
            sprintf(&context[strlen(context)], " %2d", framerate);
            if(codectype == 2)
            {
                int up_bitrate = GetvalueInt(json, "up_bitrate");
                //printf("osd_process: up_bitrate= %d \n", up_bitrate);
                if(up_bitrate > 0)
                {
                    //sprintf(&context[strlen(context)], " %5d", up_bitrate);
                    float fup_bitrate = (float)up_bitrate / 1024.0;
                    sprintf(&context[strlen(context)], " %1.2f", fup_bitrate);
                }
                else{
                    //sprintf(&context[strlen(context)], " %5d", obj->bits_rate);
                }
                int loop_loss_rate = GetvalueInt(json, "loop_loss_rate");
                //printf("osd_process: loop_loss_rate= %d \n", loop_loss_rate);
                if(loop_loss_rate > 0)
                {
                    loop_loss_rate -= 1;
                    sprintf(&context[strlen(context)], " %2d", loop_loss_rate);
                }
            }
            else{
                int down_bitrate = GetvalueInt(json, "down_bitrate");
                //printf("osd_process: down_bitrate= %d \n", down_bitrate);
                if(down_bitrate > 0)
                {
                    //sprintf(&context[strlen(context)], " %5d", down_bitrate);
                    float fdown_bitrate = (float)down_bitrate / 1024.0;
                    sprintf(&context[strlen(context)], " %1.2f", fdown_bitrate);
                }
                else{
                    //sprintf(&context[strlen(context)], " %5d", obj->bits_rate);
                    float fdown_bitrate = (float)obj->bits_rate / 1024.0;
                    sprintf(&context[strlen(context)], " %1.2f", fdown_bitrate);
                }
                int loop_loss_rate = GetvalueInt(json, "loss_rate");
                //printf("osd_process: loop_loss_rate= %d \n", loop_loss_rate);
                if(loop_loss_rate > 0)
                {
                    loop_loss_rate -= 1;
                    sprintf(&context[strlen(context)], " %2d", loop_loss_rate);
                }
                //printf("osd_process: context= %s \n", context);
            }

            json = api_renew_json_str(json, "context", context);
            //printf("osd_process: context= %s \n", context);
            //printf("osd_process: json= %x \n", json);
            //printf("osd_process: obj->osd_handle= %x \n", obj->osd_handle);
            char* jsonstr2 = api_json2str(json);
            //printf("osd_process: jsonstr2= %s \n", jsonstr2);
            ret = api_simple_osd_process(   obj->osd_handle,
                                            (char *)outbuf,
                                            jsonstr2);
            api_json2str_free(jsonstr2);
            //printf("osd_process: ret= %d \n", ret);
      }
	}
    //printf("osd_process: end \n");
	obj->frame_num += !skip_frame;
	return ret;
}
static int adapt_by_cpu(void *json)
{
    int ret = 0;
    int ref_idx = GetvalueInt(json, "ref_idx");
    int refs = GetvalueInt(json, "refs");
    int pict_type = GetvalueInt(json, "pict_type");
    int icpurate = 0;
    int memrate = 0;
    int devmemrate = 0;
    api_get_cpu_info2(&icpurate, &memrate, &devmemrate);
    if(icpurate > 95)
    {
        printf("adapt_by_cpu: icpurate= %d \n", icpurate);
        return 1;
#if 0
        if(refs > 1)
        {
            if(refs > 2)
            {
                if (((ref_idx & 1) == 0) || (ref_idx == (refs - 1)) || (ref_idx == 1))
                {
                    ret = 1;
                }
                else if ((ref_idx & 1) == 1)
                {
                    ret = 1;
                }
            }
            else{
                if ((ref_idx & 1) == 1)
                {
                    ret = 1;
                }
            }
        }
        else{
            if((pict_type == 3))//B
            {
                ret = 1;
            }
            else if((pict_type == 2))//P
            {
            }
        }
#endif
    }
#if 0
    else if(icpurate > 90)
    {
        if(refs > 1)
        {
            if(refs > 2)
            {
                if (((ref_idx & 1) == 0) || (ref_idx == (refs - 1)) || (ref_idx == 1))
                {
                    ret = 1;
                }
                else if ((ref_idx & 1) == 1)
                {
                }
            }
            else{
                if ((ref_idx & 1) == 1)
                {
                    ret = 1;
                }
            }
        }
        else{
            if((pict_type == 3))//B
            {
                ret = 1;
            }
            else if((pict_type == 2))//P
            {
            }
        }
    }
#endif
    return ret;
}
HCSVC_API
int api_video_set_skip_mb(char *handle, uint8_t *data, int size)
{
    int ret = 0;
    long long *testp = (long long *)handle;
    CodecObj *obj = (CodecObj *)testp[0];
    if(obj)
    {
        obj->c->bmb_size = size;
        memcpy(obj->c->skip_mb, data, size);
    }
    return ret;
}

HCSVC_API
int api_video_encode_one_frame(char *handle, char *data, char *param, char *outbuf, char *outparam[])
{
    int ret = 0;
    //char* ret = NULL;
    const char *tag = "string come from c code: api_video_encode_one_frame";
    cJSON *json;
    //printf("param %s \n", param);

    //if (id < MAX_OBJ_NUM)
    {
        long long *testp = (long long *)handle;
        CodecObj *obj = (CodecObj *)testp[0];

        //printf("obj->Obj_id %5d (obj->c=%5d)\n", obj->Obj_id, obj->c);
        //if (obj->Obj_id == id && obj->c != NULL)
        if (obj->c != NULL)
        {
            //AVPacket pkt;
            int got_output;
            json = (cJSON *)api_str2json(param);
            obj->param = (void *)json;
            int width = GetvalueInt(json, "width");
            int height = GetvalueInt(json, "height");
            int get_pkt = GetvalueInt(json, "get_pkt");
            if(width != obj->frame->width || height != obj->frame->height)
            {
                enc_resize(obj);
            }
            int size = obj->frame->width * obj->frame->height;
            char cTime[64] = "";
		    get_time(0, cTime, 0);
		    //printf("api_video_encode_one_frame: now= %s \n", cTime);
		    get_time_stamp();//test
            //注意：此处不能用obj->json = (cJSON *)api_str2json(param);否则，会出现编码异常，具体原因待查（可能与rtp等冲突）

            int adapt_cpu = GetvalueInt(json, "adapt_cpu");
            if(adapt_cpu)
            {
                int skip_frame = adapt_by_cpu(json);
                if(skip_frame)
                {
                    return ret;
                }
            }
            int frame_idx = GetvalueInt(json, "frame_idx");
            int bit_rate = GetvalueInt(json, "bit_rate");
            int refs = GetvalueInt(json, "refs");
            //int skip_frame = GetvalueInt(json, "skip_frame");
            //printf("api_video_encode_one_frame: bit_rate= %d \n", bit_rate);
            if(bit_rate > 0 && !obj->is_hw && obj->codec_mode != 2)//if(obj->is_hw)
            {
                int qp = rate_control(obj);
                if(qp >= 0)
                {
                    resetparam(obj);
                }
            }
            obj->width = obj->frame->width;
            obj->height = obj->frame->height;
            //printf("api_video_encode_one_frame: obj->width=%d, obj->height=%d \n", obj->width, obj->height);
            if(obj->osd_enable)
            {
                int frame_size = ((size * 3) >> 1);
                if(!obj->osd_buf)
                {
                    obj->osd_buf = calloc(1, frame_size);
                }
                memcpy(obj->osd_buf, data, frame_size);
                data = obj->osd_buf;
                osd_process(handle, data, 2);
            }
            //
            av_init_packet(&obj->pkt);
            obj->frame->pts = frame_idx;//obj->frame_idx;
            //printf("start memcpy \n");
            //fflush(stdout);

            if (obj->f2 != NULL)
            {
                fread(obj->frame->data[0], 1, size, obj->f2);
                fread(obj->frame->data[1], 1, (size >> 2), obj->f2);
                fread(obj->frame->data[2], 1, (size >> 2), obj->f2);
            }
            else{
                if(obj->is_hw)
                {
#if 1
                    memcpy(obj->frame->data[0], data, size);
                    uint8_t *u_data = (uint8_t *)&data[size];
                    uint8_t *v_data = &u_data[(size >> 2)];
                    uint16_t *p = (uint16_t *)obj->frame->data[1];
                    int offset = 0;
                    for(int i = 0; i < (size >> 1); i += 2)
                    {
                        //for(int j = 0; j < width; j+=2)
                        {
                            uint16_t v0 = u_data[offset];
                            uint16_t v1 = v_data[offset];
                            p[offset] = v0 | (v1 << 8);
                            offset++;
                        }
                    }
#else
                    if(!obj->img_convert_ctx)
                    {
                        obj->img_convert_ctx = sws_getContext(  obj->c->width,
	                                                            obj->c->height,
	                                                            AV_PIX_FMT_YUV420P,
		                                                        width,
		                                                        height,
		                                                        obj->frame->format,
		                                                        SWS_FAST_BILINEAR,
		                                                        NULL, NULL, NULL);
                    }
                    if(obj->img_convert_ctx)
                    {
                        AVFrame tmp_frame = {};//注意：初始化至关重要，否则会导致异常崩溃
                        tmp_frame.data[0] = data;
                        tmp_frame.data[1] = &data[size];
                        tmp_frame.data[2] = &data[size + (size >> 2)];
                        tmp_frame.linesize[0] = width;
                        tmp_frame.linesize[1] = width >> 1;
                        tmp_frame.linesize[2] = width >> 1;
                        tmp_frame.width = width;
		                tmp_frame.height = height;
                        tmp_frame.format = AV_PIX_FMT_YUV420P;

                        sws_scale(  obj->img_convert_ctx,
		                            tmp_frame.data,
		            		        tmp_frame.linesize, 0,
		            		        obj->c->height,
		            		        obj->frame->data,
		            		        obj->frame->linesize);

                    }
#endif
                }
                else{
                    memcpy(obj->frame->data[0], data, size);
                    memcpy(obj->frame->data[1], &data[size], (size >> 2));
                    memcpy(obj->frame->data[2], &data[size + (size >> 2)], (size >> 2));
                }
            }

            //
            AVCodecContext *c = obj->c;
            if(!obj->is_hw)
            {
                int pict_type = GetvalueInt(json, "pict_type");
                if(!pict_type)
                {
                    obj->frame->pict_type = AV_PICTURE_TYPE_NONE;
                }
                else{
                    obj->frame->pict_type = AV_PICTURE_TYPE_NONE;
                    if(pict_type == 1)
                    {
                        obj->frame->pict_type = AV_PICTURE_TYPE_I;
                    }
                    else if(pict_type == 2)
                    {
                        obj->frame->pict_type = AV_PICTURE_TYPE_P;
                    }
                    else if(pict_type == 3)
                    {
                        obj->frame->pict_type = AV_PICTURE_TYPE_B;
                    }
                }
            }

#if 0
            memset(c->nal_size, 0, MAX_PKT_NUM * sizeof(int));
            //memset(c->nal_size, 0, 1024 * sizeof(int));
#endif

            got_output = video_encode_frame((void *)obj, (AVPacket *)&obj->pkt);//(obj->c, (AVPacket *)&pkt, (AVFrame *)&obj->frame);
            //printf("got_output %d \n", got_output);
            if (got_output > 0)
            {
                if(obj->stream)
                {
                    WiteFrame2(obj->stream, &obj->pkt, kIsVideo);
                }
                int sdp = GetvalueInt(json, "sdp");
                //memset(obj->outparam[0], 0, sizeof(char) * MAX_OUTCHAR_SIZE);
                char cNalu = obj->pkt.data[4];
                char type = (cNalu & 0x1F);
                //printf("api_video_encode_one_frame: type=%u \n", type);
                if(obj->pkt.flags & AV_PKT_FLAG_KEY)
		        {
			        //int frame_type = PIC_TYPE_KEYFRAME;
			        outparam[0] = "PIC_TYPE_KEYFRAME";
			        //sprintf(p[i],"re%d",i);
		        }
		        else
		        {
		            if(obj->frame->pict_type == AV_PICTURE_TYPE_B)
		            {
		                outparam[0] = "B_frame";
		            }
		            else
		            {
		                outparam[0] = "P_frame";
		            }

		        }
                if ( obj->f != NULL)
                {
                    fwrite(obj->pkt.data, 1, obj->pkt.size, obj->f);
                }
                if(sdp && (obj->pkt.flags & AV_PKT_FLAG_KEY))
		        {
			        //int extraRet = SetExtraData(mst, video, c, kVideoChannel);
			        memcpy(outbuf, obj->c->extradata, obj->c->extradata_size);
			        memcpy(&outbuf[obj->c->extradata_size], obj->pkt.data, obj->pkt.size);
			        ret = obj->pkt.size + obj->c->extradata_size;
			        printf("api_video_encode_one_frame: obj->c->extradata_size= %d \n", obj->c->extradata_size);
		        }
		        else{
		            //memcpy(obj->buf, pkt.data, pkt.size);
                    memcpy(outbuf, obj->pkt.data, obj->pkt.size);
                    ret = obj->pkt.size;
                    //printf("api_video_encode_one_frame: ret=%d \n", ret);
		        }
                obj->sum_bits += (ret << 3);
                obj->brctrl.bytes_sum += ret;
                obj->brctrl.bytes_last = ret;
                //ret = (char *)obj->buf;

            }
            else{
                ret = 0;
            }
            if(!get_pkt)
            {
                if(obj->pkt.data && false)
                {
                    av_free(obj->pkt.data);
                    obj->pkt.data = NULL;
                }
                av_packet_unref(&obj->pkt);
                ///av_free_packet(&obj->pkt);
                //printf("api_video_encode_one_frame: obj->pkt.data= %x \n", obj->pkt.data);
            }
            obj->frame_idx += 1;
            api_json_free(obj->param);
            obj->param = NULL;
        }
    }
    //ret = (char *)tag;//test
    return ret;
}
HCSVC_API
int api_video_encode_close(char *handle)
{
    long long ret = 0;
    if(handle)
    {
        long long *testp = (long long *)handle;
        CodecObj *obj = (CodecObj *)testp[0];

        if(obj == NULL)//(obj->Obj_id != id)
        {
            return -1;
        }
        if(obj->rtpObj)
        {
            free(obj->rtpObj);
            obj->rtpObj = NULL;
        }
        if(obj->fecEncObj)
        {
            free(obj->fecEncObj);
            obj->fecEncObj = NULL;
        }
        if(obj->fecDecObj)
        {
            free(obj->fecDecObj);
            obj->fecDecObj = NULL;
        }
        if(obj->ppObj)
        {
            for(int i = 0; i < MAX_SKIP_FRAME_NUM; i++)
            {
                if(obj->ppObj->last_frame[i])
                {
                    int flag = !i;
                    flag |= (i && (obj->ppObj->last_frame[i] != obj->ppObj->last_frame[i - 1]));
                    if(flag)
                    {
                        av_free(obj->ppObj->last_frame[i]);
                        obj->ppObj->last_frame[i] = NULL;
                    }
                }
            }
            if(obj->ppObj->static_frame)
            {
            }
            if(obj->ppObj->img_convert_ctx)
            {
                sws_freeContext(obj->ppObj->img_convert_ctx);
                obj->ppObj->img_convert_ctx = NULL;
            }
            if(obj->ppObj->brhead)
            {
                release_brnode(obj->ppObj->brhead);
                obj->ppObj->brhead = NULL;
            }
            if(obj->ppObj->sadHnd)
            {
                I2SADHndClose(obj->ppObj->sadHnd);
            }

            free(obj->ppObj);
            obj->ppObj = NULL;
        }
        encode_close(obj);
        ResetObj(obj);
        ret = (long long)api_free_codec_handle(handle);
    }
    printf("api_video_encode_close: over \n");
    return (int)ret;
}
//======================================================================================

//===============================================
AVCodecContext *decode_open(void *hnd)
{
    CodecObj *obj = (CodecObj *)hnd;
    int codec_id = obj->codec_id;
    AVCodec *codec;
    AVCodecContext *c= NULL;

    //cJSON *json = (cJSON *)obj->param;
    char *h264_names[3] = {"h264_qsv", "h264_vaapi", "h264_cuvid"};//"libx264"
    char *h265_names[3] = {"hevc_qsv", "hevc_vaapi", "hevc_cuvid"};
    obj->codec_mode = GetvalueInt(obj->json, "codec_mode");
    if(obj->codec_mode > 0)
    {
        char *codec_name;
        if(obj->codec_mode == 1)//hw264
        {
            for(int i = 0; i < 3; i++)
            {
                codec = find_codec_by_name(h264_names[i], 0);
                if(codec)
                {
                    codec_name = h264_names[i];
                    printf("decode_open: h264_names[i]=%s \n", h264_names[i]);
                    break;
                }
            }
        }
        else if(obj->codec_mode == 3)//hw265
        {
            for(int i = 0; i < 3; i++)
            {
                codec = find_codec_by_name(h265_names[i], 0);
                if(codec)
                {
                    codec_name = h264_names[i];
                    printf("decode_open: h264_names[i]=%s \n", h264_names[i]);
                    break;
                }
            }
        }
        if(!codec)
        {
        }
        else{
            //AVBufferRef *hw_device_ctx = NULL;
            AVFormatContext *input_ctx = NULL;//no used
            AVStream *video_st = NULL;//no used
            //AVCodecContext *decoder_ctx = NULL;
            AVIOContext *output_ctx = NULL;//no used

            AVCodecContext params;
            if(!strcmp(codec_name, "h264_vaapi") || !strcmp(codec_name, "h264_qsv"))
            {

            }
            int ret = decode_init(  codec, &input_ctx, &video_st, &obj->c, &obj->hw_frame, &obj->frame,
                                    &output_ctx, &obj->hw_device_ctx, codec_name, NULL, NULL);
            if(ret < 0)
            {
                printf("error: decode_open: ret=%d \n", ret);
            }
            else{
                obj->is_hw = 1;
                printf("decode_open: ok \n");
                return obj->c;
            }

        }
    }
    /* find the mpeg1 video decoder */
    //codec = avcodec_find_decoder(AV_CODEC_ID_MPEG1VIDEO);
    codec = avcodec_find_decoder(codec_id);

    if (!codec) {
        fprintf(stderr, "decode_open: Codec not found\n");
        //exit(1);
        return c;
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        //exit(1);
        return c;
    }

    if (codec->capabilities & AV_CODEC_CAP_TRUNCATED)
        c->flags |= AV_CODEC_FLAG_TRUNCATED; // we do not send complete frames
    //c->thread_type = FF_THREAD_SLICE;
    //printf("c->flags= %d\n", c->flags);
    //c->flags |= AV_CODEC_FLAG_TRUNCATED;
    //printf("c->flags= %d\n", c->flags);
    /* For some codecs, such as msmpeg4 and mpeg4, width and height
       MUST be initialized there because this information is not
       available in the bitstream. */

    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        //exit(1);
        return c;
    }

    //obj->frame = av_frame_alloc();
    //if (!obj->frame) {
    //    fprintf(stderr, "Could not allocate video frame\n");
    //    //exit(1);
    //}
    obj->parser = av_parser_init( codec_id );
    obj->codec = codec;
    return c;
}
static int adapt_by_cpu2(int nal_ref_idc)
{
    int ret = 0;
    int icpurate = 0;
    int memrate = 0;
    int devmemrate = 0;
    api_get_cpu_info2(&icpurate, &memrate, &devmemrate);
    if(icpurate > 95)
    {
        if(nal_ref_idc < 3)
        {
            return 1;
        }
    }
    else if(icpurate > 90)
    {
        if(nal_ref_idc < 2)
        {
            return 1;
        }
    }
    return ret;
}
int get_nal_ref_idc(unsigned char *indata)
{
    int ret = -1;
    unsigned char *b = indata;
    if( b[0] == b[1] &&
        b[1] == b[2] &&
        b[0] == 0 &&
        b[3] == 1)
    {
        ret = ((b[4] & 0x7F) >> 5);
    }
    else if(b[0] == b[1] &&
            b[0] == 0 &&
            b[2] == 1)
    {
        ret = ((b[3] & 0x7F) >> 5);
    }
    return ret;
}
int video_decode_frame(void *hnd, unsigned char *indata, int insize, int oneframe, char *outBuf, AVPacket *pkt)
{
    CodecObj *obj = (CodecObj *)hnd;
    AVCodecContext *c = obj->c;
    AVFrame *frame = obj->frame;
    AVCodecParserContext *parser = obj->parser;
    uint8_t *inbuf = obj->inbuf;
    //uint8_t *buffer = (uint8_t *)av_malloc((insize + AV_INPUT_BUFFER_PADDING_SIZE)*sizeof(uint8_t));
    int ret = 0;
    //int frame_count;
    int offset = 0;
    int got_picture = 0;
    //AVFrame *frame;
    //uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    //AVPacket pkt;
    //av_init_packet(&pkt);

    /* set end of buffer to 0 (this ensures that no overreading happens for damaged mpeg streams) */
    //memset(inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);
    //memset(buffer + insize, 0, AV_INPUT_BUFFER_PADDING_SIZE);
    //memcpy(buffer, indata, insize);
    int nal_ref_idc = get_nal_ref_idc(indata);
    int adapt_cpu = GetvalueInt(obj->param, "adapt_cpu");
    if(adapt_cpu)
    {
        int skip_frame = adapt_by_cpu2(nal_ref_idc);
        if(skip_frame)
        {
            return ret;
        }
    }
    //printf("video_decode_frame: nal_ref_idc= %d \n", nal_ref_idc);
    while((insize - offset) > 0)
    {
        uint8_t *cur_ptr = inbuf;
        int cur_size = insize;
        if (oneframe > 0)
        {
            cur_ptr = indata;
            cur_size = insize;
            offset = insize;
        }
        else{
            cur_size = (insize - offset) >= INBUF_SIZE ? INBUF_SIZE : (insize - offset);
            if(cur_size > 0)
            {
                memcpy(inbuf, &indata[offset], cur_size);
            }
            offset += cur_size;
        }
        if (cur_size == 0)
            break;

        while (cur_size > 0)
        {
            if (oneframe > 0)
            {
                pkt->data = indata;
                //pkt->data = buffer;
                pkt->size = insize;
                cur_ptr += insize;
                cur_size -= insize;
            }
            else{
                int len = av_parser_parse2(
                    parser, c,
                    &pkt->data, &pkt->size,
                    cur_ptr, cur_size,
                    AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);
                cur_ptr += len;
                cur_size -= len;
            }

            //printf("video_decode_frame: insize= %d \n", insize);
            //printf("video_decode_frame: pkt->size= %d \n", pkt->size);
            if (pkt->size == 0)
                continue;
            //int ret = decode_write_frame(outfilename, c, frame, &frame_count, &pkt, 0);
            ret = avcodec_decode_video2(c, frame, &got_picture, pkt);
            if(obj->stream)
            {
                //printf("video_decode_frame: pkt->flags= %d \n", pkt->flags);//obj->frame->key_frame
                //printf("video_decode_frame: frame->key_frame= %d \n", frame->key_frame);
                //printf("video_decode_frame: frame->pict_type= %d \n", frame->pict_type);
                //c->width = frame->width;
                //c->height = frame->height;
                if(frame->key_frame)
                {
                    pkt->flags = frame->key_frame;
                }
                WiteFrame2(obj->stream, pkt, kIsVideo);
            }
            ///printf("video_decode_frame: frame->linesize[0] %d \n", frame->linesize[0]);
            //printf("frame->linesize[1] %d \n", frame->linesize[1]);
            //printf("frame->linesize[2] %d \n", frame->linesize[2]);
            //printf("video_decode_frame: frame->width %d \n", frame->width);
            //printf("video_decode_frame: frame->height %d \n", frame->height);

            if (ret < 0)
            {
                printf("video_decode_frame: Decode Error.\n");
                //av_free(buffer);
                return ret;
            }
            if (pkt->data) {
                pkt->size -= ret;
                pkt->data += ret;
            }
            if(got_picture)
            {
                //Y
                int w = frame->width;
                int h = frame->height;
                char *y = &outBuf[0];
                char *u = &outBuf[w * h];
                char *v = &outBuf[w * h + ((w * h) >> 2)];
				for (int i = 0; i < frame->height; i++)
				{
					memcpy(&y[i * frame->width], &frame->data[0][i * frame->linesize[0]], frame->width);
				}
				//U
				for (int i = 0; i < frame->height; i++)
				{
					if (!(i & 1))
					{
						memcpy(&u[(i >> 1) * (frame->width >> 1)], &frame->data[1][(i >> 1) * frame->linesize[1]], frame->width >> 1);
					}
				}
				//V
				for (int i = 0; i < frame->height; i++)
				{
					if (!(i & 1))
					{
						memcpy(&v[(i >> 1) * (frame->width >> 1)], &frame->data[2][(i >> 1) * frame->linesize[2]], frame->width >> 1);
					}
				}
				//printf("ret %d \n", ret);
				ret = (frame->width * frame->height * 3) >> 1;
				//printf("ret %d \n", ret);
            }
            else{
                ret = 0;
            }
        }
    }
    //av_free(buffer);
    return ret;
}
void decode_close(void *hnd)
{
    CodecObj *obj = (CodecObj *)hnd;
    AVCodecContext *c = obj->c;
    AVFrame *frame = obj->frame;
    AVCodecParserContext *parser = obj->parser;
    FILE *f = obj->f;
    printf("decode_close: start close \n");
    if(obj->is_hw)
    {
        hw_decode_close(NULL,
                        NULL,
                        obj->c,
                        obj->hw_frame,
                        obj->frame,
                        NULL,
                        obj->hw_device_ctx);
        obj->hw_device_ctx = NULL;
        obj->hw_frame = NULL;
        obj->frame = NULL;
        obj->c = NULL;
        c = NULL;
    }
    if(obj->img_convert_ctx)
    {
        sws_freeContext(obj->img_convert_ctx);
        obj->img_convert_ctx = NULL;
    }
    if(c)
    {
        avcodec_close(c);
        av_free(c);
    }

    printf("video decode_close: 1 \n");
    ///av_freep(&frame->data[0]);
    if(obj->frame)
    {
        av_frame_free(&obj->frame);
    }
    if(obj->hw_frame)
    {
        av_frame_free(&obj->hw_frame);
    }

    //av_free(frame);
    printf("video decode_close: 2 \n");
    if(obj->inbuf != NULL)
    {
        av_free(obj->inbuf);
    }
    printf("video decode_close: obj->inbuf \n");
    //parser close !!!
    //av_free_packet(&pkt);
    if(obj->json)
    {
        api_json_free(obj->json);
        obj->json = NULL;
    }
    if(obj->osd_handle)
    {
        api_simple_osd_close(obj->osd_handle);
        free(obj->osd_handle);
        obj->osd_handle = NULL;
    }
    if (f != NULL)
    {
        fclose(f);
    }
    if(obj->logfp)
    {
        fclose(obj->logfp);
        obj->logfp = NULL;
    }
    //cJSON_Delete(obj->param);
    printf("video decode_close: over \n");
}
int decode_open2(char *handle, char *param)
{
    long long ret = 0;
    //cJSON *json;
    //if ((handle == NULL))
    {
        ret = (long long)api_create_codec_handle(handle);
        long long *testp = (long long *)handle;
        CodecObj *obj = (CodecObj *)testp[0];
        int id = obj->Obj_id;
        ResetObj(obj);
        obj->Obj_id = id;
#if 0
        if(!glob_lock)
        {
            glob_lock = calloc(1, sizeof(pthread_mutex_t));
            pthread_mutex_init(glob_lock, NULL);
        }
        pthread_mutex_lock(glob_lock);
        avcodec_register_all();
        av_log_set_level(AV_LOG_QUIET);
        pthread_mutex_unlock(glob_lock);

        CodecObj *obj = (CodecObj *)calloc(1, sizeof(CodecObj));
        ret = (long long)obj;
        //handle = (void *)obj;
        int handle_size = sizeof(long long);
        printf("decode_open2: handle_size= %d \n", handle_size);
        printf("decode_open2: obj= %x \n", obj);
        printf("decode_open2: ret= %x \n", ret);
        memcpy(handle, &ret, handle_size);
        long long *testp = (long long *)handle;
        printf("decode_open2: testp[0]= %x \n", testp[0]);
        avcodec_register_all();
        av_log_set_level(AV_LOG_QUIET);
        int id = (int)obj;//long long ???
        ret = id;
        ResetObj(obj);
#endif

        //if (obj->Obj_id < 0)
        {
            obj->json = (cJSON *)api_str2json(param);
            //obj->param = (void *)json;
            //cJSON *item;
            //item = cJSON_GetObjectItem(json, "insize");
            //if(cJSON_IsString(item))
            //{
            //    printf("Item: valuestring=%s\n", item->valuestring);
            //}
            //else if(cJSON_IsNumber(item))
            //{
            //    printf("Item: valueint=%d\n", item->valueint);
            //}
            //printf("Item: type=%d, key=%s, valueint=%d, valuestring=%s\n", item->type, item->string, item->valueint, item->valuestring);
            //
            obj->osd_enable = GetvalueInt(obj->json, "osd");
            printf("decode_open2: obj->osd_enable=%d \n", obj->osd_enable);
            if(obj->osd_enable)
            {
                obj->orgX = GetvalueInt(obj->json, "orgX");
                obj->orgY = GetvalueInt(obj->json, "orgY");
                obj->scale = GetvalueInt(obj->json, "scale");
                obj->color = GetvalueInt(obj->json, "color");
                //char *src_pix_fmt = GetvalueStr(obj->json, "src_pix_fmt");
	            //if (strcmp(src_pix_fmt, ""))
	            //{
                //    if (!strcmp(src_pix_fmt, "AV_PIX_FMT_YUV420P"))
                //    {
                //        //obj->src_pix_fmt = AV_PIX_FMT_YUV420P;
                //    }
                //}
            }
            //
            obj->Obj_id = id;
            printf("decode_open2: id=%d \n", id);
            obj->c = decode_open((void *)obj);
            av_init_packet(&obj->pkt);
            obj->inbuf = (uint8_t *)av_malloc((INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE)*sizeof(uint8_t));
            memset(obj->inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);
            //
            if(!obj->logfp && false)
            {
                char filename[256] = "/home/gxh/works/decode_gxh_";
                char ctmp[32] = "";
                int fileidx = id;//random() % 10000;
                sprintf(ctmp, "%d", fileidx);
                strcat(filename, ctmp);
                strcat(filename, ".txt");
                obj->logfp = fopen(filename, "w");
            }
        }
    }
    return (int)ret;
}
HCSVC_API
int api_video_decode_open(char *handle, char *param)
{
    return decode_open2(handle, param);
}
//unsigned char *indata, int insize, int oneframe, char *outBuf
HCSVC_API
int api_video_decode_one_frame(char *handle, char *data, char *param, char *outbuf, char *outparam[])
{
    int ret = 0;
    //char* ret = NULL;
    const char *tag = "string come from c code: api_video_decode_one_frame";
    cJSON *json;

    //if (id < MAX_OBJ_NUM)
    {
        long long *testp = (long long *)handle;
        CodecObj *obj = (CodecObj *)testp[0];

        //decode_open2(id, param);
        //printf("api_video_decode_one_frame: obj= %x \n", obj);
        //printf("obj->Obj_id %5d (obj->c=%5d)\n", obj->Obj_id, obj->c);
        //if (obj->Obj_id == id && obj->c != NULL)
        if(obj->c != NULL)
        {
            json = (cJSON *)api_str2json(param);
            obj->param = (void *)json;
            //cJSON *json = (cJSON *)obj->param;
            int insize = GetvalueInt(json, "insize");
            int oneframe = GetvalueInt(json, "oneframe");
            //AVPacket pkt;
            //av_init_packet(&pkt);
            //printf("oneframe %d \n", oneframe);
            //printf("api_video_decode_one_frame: insize %d \n", insize);
            obj->sum_bits += (insize << 3);
            if(!obj->is_hw)
            {
                ret = video_decode_frame((void *)obj, data, insize, oneframe, outbuf, (AVPacket *)&obj->pkt);
                obj->width = obj->frame->width;
                obj->height = obj->frame->height;
                //printf("api_video_decode_one_frame: obj->width=%d \n", obj->width);
                //printf("api_video_decode_one_frame: obj->height=%d \n", obj->height);
            }
            else{
                obj->pkt.data = data;
                obj->pkt.size = insize;
                //printf("api_video_decode_one_frame: insize=%d \n", insize);
                ret = hw_decode_one_frame(&obj->img_convert_ctx, obj->c, &obj->pkt, obj->hw_frame, obj->frame, outbuf);
                //printf("api_video_decode_one_frame: ret=%d \n", ret);
                obj->width = obj->c->width;
                obj->height = obj->c->height;
                //printf("api_video_decode_one_frame: obj->width=%d \n", obj->width);
                //printf("api_video_decode_one_frame: obj->height=%d \n", obj->height);
                //printf("api_video_decode_one_frame: obj->c->width=%d \n", obj->c->width);
                //printf("api_video_decode_one_frame: obj->c->height=%d \n", obj->c->height);
            }
            //printf("api_video_decode_one_frame: obj->stream=%x \n", obj->stream);
            if(ret <= 0)
            {
                if (obj->logfp && true) {
                    fprintf(obj->logfp, "api_video_decode_one_frame: ret= %d \n", ret);
                    fprintf(obj->logfp, "api_video_decode_one_frame: insize= %d \n", insize);
                    fprintf(obj->logfp, "api_video_decode_one_frame: obj->frame_idx= %d \n", obj->frame_idx);
                    fflush(obj->logfp);
                }
                api_json_free(obj->param);
                obj->param = NULL;
                return ret;
            }

            //printf("api_video_decode_one_frame: obj->width=%d, obj->height=%d \n", obj->width, obj->height);
            //printf("api_video_decode_one_frame: obj->osd_enable=%d \n", obj->osd_enable);
            if(obj->osd_enable && ret > 0)
            {
                osd_process(handle, outbuf, 1);
            }
            outparam[0] = obj->outparam[0];
            outparam[1] = obj->outparam[1];
            outparam[2] = obj->outparam[2];
            //obj->sum_bits += (insize << 3);
            if ((obj->pkt.flags & AV_PKT_FLAG_KEY) || (obj->frame->pict_type == AV_PICTURE_TYPE_I) || (obj->frame->key_frame & AV_PKT_FLAG_KEY))
			{
				strcpy(outparam[0], "PIC_TYPE_KEYFRAME");
			}
			else{
			    strcpy(outparam[0], "P_frame");
			}
			if ((obj->frame->key_frame & AV_PKT_FLAG_CORRUPT))
			{
				printf("error %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% \n");
				strcpy(outparam[0], "ERR_frame");
			}
			//printf("api_video_decode_one_frame: outparam[0]=%s \n", outparam[0]);
			//outparam[1] = obj->frame->width;
			//outparam[2] = obj->frame->height;
			//printf("api_video_decode_one_frame: start write outparam ");
			//char text0[32] = "";
			//char text1[32] = "";
			//char *text0 = obj->outparam[1];
			//char *text1 = obj->outparam[2];
			sprintf(outparam[1], "%d", obj->width);
			sprintf(outparam[2], "%d", obj->height);
			//outparam[1] = text0;
			//outparam[2] = text1;
			//av_free_packet(&pkt);
            obj->frame_idx += 1;
            api_json_free(obj->param);
            obj->param = NULL;
            //printf("api_video_decode_one_frame: obj->frame_idx=%d \n", obj->frame_idx);
        }
    }
    return ret;
}
HCSVC_API
int api_video_decode_close(char *handle)
{
    long long ret = 0;
    if(handle)
    {
        long long *testp = (long long *)handle;
        CodecObj *obj = (CodecObj *)testp[0];

        if(obj == NULL)//(obj->Obj_id != id)
        {
            return -1;
        }
        if(obj->resortObj)
        {
            if(obj->resortObj->recv_buf)
            {
                for(int i = 0; i < obj->resortObj->buf_size; i++)
                {
                    free(obj->resortObj->recv_buf[i]);
                }
                free(obj->resortObj->recv_buf);
            }
            if(obj->resortObj->pRet)
            {
                for(int i = 0; i < MAX_RESORT_FRAME_NUM; i++)
                {
                    free(obj->resortObj->pRet->complete[i]);
                    obj->resortObj->pRet->complete[i] = NULL;
                }
                free(obj->resortObj->pRet);
                obj->resortObj->pRet = NULL;
            }
            free(obj->resortObj);
            obj->resortObj = NULL;
        }
        if(obj->rtpObj)
        {
            free(obj->rtpObj);
            obj->rtpObj = NULL;
        }
        if(obj->fecEncObj)
        {
            free(obj->fecEncObj);
            obj->fecEncObj = NULL;
        }
        if(obj->fecDecObj)
        {
            free(obj->fecDecObj);
            obj->fecDecObj = NULL;
        }
        if(obj->pktManObj)
        {
            free(obj->pktManObj);
            obj->pktManObj = NULL;
        }
        decode_close(obj);
        ResetObj(obj);
        ret = (long long)api_free_codec_handle(handle);
    }
    return (int)ret;
}
#endif
//===============================================

#if 0
int main(int argc, char **argv)
{
    const char *output_type;

    /* register all the codecs */
    avcodec_register_all();

    if (argc < 2) {
        printf("usage: %s output_type\n"
               "API example program to decode/encode a media stream with libavcodec.\n"
               "This program generates a synthetic stream and encodes it to a file\n"
               "named test.h264, test.mp2 or test.mpg depending on output_type.\n"
               "The encoded stream is then decoded and written to a raw data output.\n"
               "output_type must be chosen between 'h264', 'mp2', 'mpg'.\n",
               argv[0]);
        return 1;
    }
    output_type = argv[1];

    if (!strcmp(output_type, "h264")) {
        video_encode_example("test.h264", AV_CODEC_ID_H264);
    } else if (!strcmp(output_type, "mp2")) {
        audio_encode_example("test.mp2");
        audio_decode_example("test.pcm", "test.mp2");
    } else if (!strcmp(output_type, "mpg")) {
        video_encode_example("test.mpg", AV_CODEC_ID_MPEG1VIDEO);
        video_decode_example("test%02d.pgm", "test.mpg");
    } else {
        fprintf(stderr, "Invalid output type '%s', choose between 'h264', 'mp2', or 'mpg'\n",
                output_type);
        return 1;
    }

    return 0;
}
#endif
//



/*
 * Copyright (c) 2020
 * Created by gxh_20200602
 */

/*
#define SWS_FAST_BILINEAR     1

#define SWS_BILINEAR          2

#define SWS_BICUBIC           4

#define SWS_X                 8

#define SWS_POINT          0x10

#define SWS_AREA           0x20

#define SWS_BICUBLIN       0x40

#define SWS_GAUSS          0x80

#define SWS_SINC          0x100

#define SWS_LANCZOS       0x200

#define SWS_SPLINE        0x400
*/


#include "inc.h"

extern cJSON* mystr2json(char *text);
extern int GetvalueInt(cJSON *json, char *key);
extern char* GetvalueStr(cJSON *json, char *key);
extern cJSON* deleteJson(cJSON *json);

static void fill_yuv_image(uint8_t *data[4], int linesize[4],
                           int width, int height, int frame_index)
{
    int x, y;

    /* Y */
    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++)
            data[0][y * linesize[0] + x] = x + y + frame_index * 3;

    /* Cb and Cr */
    for (y = 0; y < height / 2; y++) {
        for (x = 0; x < width / 2; x++) {
            data[1][y * linesize[1] + x] = 128 + y + frame_index * 2;
            data[2][y * linesize[2] + x] = 64 + x + frame_index * 5;
        }
    }
}
int FFScale(ScaleObj *scale)
{
    int ret = 0;
    //uint8_t *src_data = scale->src_data;
    //uint8_t *dst_data  = scale->dst_data;
    enum AVPixelFormat src_pix_fmt = scale->src_pix_fmt;
    enum AVPixelFormat dst_pix_fmt = scale->dst_pix_fmt;
    struct SwsContext *sws_ctx = scale->sws_ctx;
    int src_w = scale->src_w;// = 320,
    int src_h = scale->src_h;// = 240,
    //int dst_w = scale->dst_w;
    //int dst_h = scale->dst_h;
    const char *dst_size = scale->dst_size;// = NULL;
    //int src_linesize = scale->src_linesize;
    //int dst_linesize = scale->dst_linesize;
    int scale_mode = scale->scale_mode;
#if 0
    if (av_parse_video_size(&scale->dst_w, &scale->dst_h, dst_size) < 0) {
        fprintf(stderr,
                "Invalid size '%s', must be in the form WxH or a valid size abbreviation\n",
                dst_size);
        //exit(1);
        return ret;
    }
#endif
    //printf("scale->dst_w= %d ,scale->dst_h= %d\n", scale->dst_w, scale->dst_h);
    if(scale->dst_pix_fmt == AV_PIX_FMT_YUV420P)
    {
        scale->dst_linesize[0] = scale->dst_w;
        scale->dst_linesize[1] = scale->dst_w >> 1;
        scale->dst_linesize[2] = scale->dst_w >> 1;
        int w = scale->dst_w;
        int h = scale->dst_h;
        scale->dst_data[0] = scale->outbuf;
        scale->dst_data[1] = &scale->outbuf[w *h];
        scale->dst_data[2] = &scale->outbuf[w * h + ((w * h) >> 2)];
        ret = (w * h * 3) >> 1;
    }
    /* create scaling context */
    if(sws_ctx == NULL)
    {
        sws_ctx = sws_getContext(src_w, src_h, src_pix_fmt,
                                scale->dst_w, scale->dst_h, dst_pix_fmt,
                                scale_mode, NULL, NULL, NULL);//SWS_BILINEAR
    }


    /* convert to destination format */
    sws_scale(sws_ctx, (const uint8_t * const*)scale->src_data,
              scale->src_linesize, 0, src_h, scale->dst_data, scale->dst_linesize);
    int flag = 0;//1;
    if(flag)
    {
        FILE *dst_file = fopen("scale_out.yuv", "wb");
        int dst_bufsize = scale->dst_w * scale->dst_h;
        fwrite(scale->dst_data[0], 1, dst_bufsize, dst_file);
        fwrite(scale->dst_data[1], 1, dst_bufsize >> 2, dst_file);
        fwrite(scale->dst_data[2], 1, dst_bufsize >> 2, dst_file);
        fclose(dst_file);
    }

    /* write scaled image to file */
    //fwrite(dst_data[0], 1, dst_bufsize, dst_file);
    sws_freeContext(scale->sws_ctx);
    scale->sws_ctx = NULL;

    return ret;
}
HCSVC_API
int api_create_scale_handle(char *handle)
{
    long long ret = 0;
    if(handle)
    {
        ScaleObj *obj = (ScaleObj *)calloc(1, sizeof(ScaleObj));
        //CodecObj *obj = (CodecObj *)&global_codec_objs[id];
        ret = (long long)obj;
        //handle = (void *)obj;
        int handle_size = sizeof(long long);
        printf("api_create_scale_handle: handle_size= %d \n", handle_size);
        printf("api_create_scale_handle: obj= %x \n", obj);
        printf("api_create_scale_handle: ret= %x \n", ret);
        memcpy(handle, &ret, handle_size);
        long long *testp = (long long *)handle;
        printf("api_create_scale_handle: testp[0]= %x \n", testp[0]);
        int id = (int)obj;//long long ???
        obj->Obj_id = id;
        ret = id;
    }
    return (int)ret;
}
HCSVC_API
int api_scale_init(char *handle, char *param)
{
    int ret = 0;
    ret = api_create_scale_handle(handle);
    long long *testp = (long long *)handle;
    ScaleObj *obj = (ScaleObj *)testp[0];
    obj->json = mystr2json(param);
    obj->param = param;
    //obj->Obj_id = id;
    //obj->outbuf = outbuf;
    obj->src_linesize[0] = 0;
    obj->src_linesize[1] = 0;
    obj->src_linesize[2] = 0;
    obj->src_linesize[3] = 0;
    obj->dst_linesize[0] = 0;
    obj->dst_linesize[1] = 0;
    obj->dst_linesize[2] = 0;
    obj->src_linesize[3] = 0;
    obj->src_w = GetvalueInt(obj->json, "src_w");
    obj->src_h = GetvalueInt(obj->json, "src_h");
    char *dst_size = GetvalueStr(obj->json, "dst_size");
	if (strcmp(dst_size, ""))
	{
       obj->dst_size = dst_size;
	}
#if 1
    obj->src_pix_fmt = GetvalueInt(obj->json, "src_pix_fmt");
    obj->dst_pix_fmt = GetvalueInt(obj->json, "dst_pix_fmt");
    obj->scale_mode = GetvalueInt(obj->json, "scale_mode");
#else
    char *src_pix_fmt = GetvalueStr(json, "src_pix_fmt");
	if (strcmp(src_pix_fmt, ""))
	{
        if (!strcmp(src_pix_fmt, "AV_PIX_FMT_YUV420P"))
        {
            obj->src_pix_fmt = AV_PIX_FMT_YUV420P;
            obj->src_linesize[0] = obj->src_w;
            obj->src_linesize[1] = obj->src_w >> 1;
            obj->src_linesize[2] = obj->src_w >> 1;
            int w = obj->src_w;
            int h = obj->src_h;
            obj->src_data[0] = data;
            obj->src_data[1] = &data[w *h];
            obj->src_data[2] = &data[w * h + ((w * h) >> 2)];
        }
        else if (!strcmp(src_pix_fmt, "AV_PIX_FMT_RGB24"))
        {
            obj->src_pix_fmt = AV_PIX_FMT_RGB24;
        }
	}
	char *dst_pix_fmt = GetvalueStr(json, "dst_pix_fmt");
	if (strcmp(dst_pix_fmt, ""))
	{
        if (!strcmp(dst_pix_fmt, "AV_PIX_FMT_YUV420P"))
        {
            obj->dst_pix_fmt = AV_PIX_FMT_YUV420P;
        }
        else if (!strcmp(dst_pix_fmt, "AV_PIX_FMT_RGB24"))
        {
            obj->dst_pix_fmt = AV_PIX_FMT_RGB24;
        }
	}
	char *scale_mode = GetvalueStr(json, "scale_mode");
	if (strcmp(scale_mode, ""))
	{
        if (!strcmp(scale_mode, "SWS_FAST_BILINEAR"))
        {
            obj->scale_mode = SWS_FAST_BILINEAR;
        }
        else if (!strcmp(scale_mode, "SWS_BILINEAR"))
        {
            obj->scale_mode = SWS_BILINEAR;
        }
        else if (!strcmp(scale_mode, "SWS_BICUBIC"))
        {
            obj->scale_mode = SWS_BICUBIC;
        }
        else if (!strcmp(scale_mode, "SWS_X"))
        {
            obj->scale_mode = SWS_X;
        }
        else if (!strcmp(scale_mode, "SWS_POINT"))
        {
            obj->scale_mode = SWS_POINT;
        }
        else if (!strcmp(scale_mode, "SWS_AREA"))
        {
            obj->scale_mode = SWS_AREA;
        }
        else if (!strcmp(scale_mode, "SWS_BICUBLIN"))
        {
            obj->scale_mode = SWS_BICUBLIN;
        }
        else if (!strcmp(scale_mode, "SWS_GAUSS"))
        {
            obj->scale_mode = SWS_GAUSS;
        }
        else if (!strcmp(scale_mode, "SWS_SINC"))
        {
            obj->scale_mode = SWS_SINC;
        }
        else if (!strcmp(scale_mode, "SWS_LANCZOS"))
        {
            obj->scale_mode = SWS_LANCZOS;
        }
        else if (!strcmp(scale_mode, "SWS_SPLINE"))
        {
            obj->scale_mode = SWS_SPLINE;
        }
	}
#endif
    return ret;
}
HCSVC_API
int api_ff_scale(int id, char *data, char *param, char *outbuf, char *outparam[])
{
    int ret = 0;
    ScaleObj scale;
    cJSON *json = mystr2json(param);
    //
    scale.Obj_id = id;
    scale.outbuf = outbuf;
    scale.src_linesize[0] = 0;
    scale.src_linesize[1] = 0;
    scale.src_linesize[2] = 0;
    scale.src_linesize[3] = 0;
    scale.dst_linesize[0] = 0;
    scale.dst_linesize[1] = 0;
    scale.dst_linesize[2] = 0;
    scale.src_linesize[3] = 0;
    scale.src_w = GetvalueInt(json, "src_w");
    scale.src_h = GetvalueInt(json, "src_h");
    char *dst_size = GetvalueStr(json, "dst_size");
	if (strcmp(dst_size, ""))
	{
       scale.dst_size = dst_size;
	}
    char *src_pix_fmt = GetvalueStr(json, "src_pix_fmt");
	if (strcmp(src_pix_fmt, ""))
	{
        if (!strcmp(src_pix_fmt, "AV_PIX_FMT_YUV420P"))
        {
            scale.src_pix_fmt = AV_PIX_FMT_YUV420P;
            scale.src_linesize[0] = scale.src_w;
            scale.src_linesize[1] = scale.src_w >> 1;
            scale.src_linesize[2] = scale.src_w >> 1;
            int w = scale.src_w;
            int h = scale.src_h;
            scale.src_data[0] = data;
            scale.src_data[1] = &data[w *h];
            scale.src_data[2] = &data[w * h + ((w * h) >> 2)];
        }
        else if (!strcmp(src_pix_fmt, "AV_PIX_FMT_RGB24"))
        {
            scale.src_pix_fmt = AV_PIX_FMT_RGB24;
        }
	}
	char *dst_pix_fmt = GetvalueStr(json, "dst_pix_fmt");
	if (strcmp(dst_pix_fmt, ""))
	{
        if (!strcmp(dst_pix_fmt, "AV_PIX_FMT_YUV420P"))
        {
            scale.dst_pix_fmt = AV_PIX_FMT_YUV420P;
        }
        else if (!strcmp(dst_pix_fmt, "AV_PIX_FMT_RGB24"))
        {
            scale.dst_pix_fmt = AV_PIX_FMT_RGB24;
        }
	}
	char *scale_mode = GetvalueStr(json, "scale_mode");
	if (strcmp(scale_mode, ""))
	{
        if (!strcmp(scale_mode, "SWS_FAST_BILINEAR"))
        {
            scale.scale_mode = SWS_FAST_BILINEAR;
        }
        else if (!strcmp(scale_mode, "SWS_BILINEAR"))
        {
            scale.scale_mode = SWS_BILINEAR;
        }
        else if (!strcmp(scale_mode, "SWS_BICUBIC"))
        {
            scale.scale_mode = SWS_BICUBIC;
        }
        else if (!strcmp(scale_mode, "SWS_X"))
        {
            scale.scale_mode = SWS_X;
        }
        else if (!strcmp(scale_mode, "SWS_POINT"))
        {
            scale.scale_mode = SWS_POINT;
        }
        else if (!strcmp(scale_mode, "SWS_AREA"))
        {
            scale.scale_mode = SWS_AREA;
        }
        else if (!strcmp(scale_mode, "SWS_BICUBLIN"))
        {
            scale.scale_mode = SWS_BICUBLIN;
        }
        else if (!strcmp(scale_mode, "SWS_GAUSS"))
        {
            scale.scale_mode = SWS_GAUSS;
        }
        else if (!strcmp(scale_mode, "SWS_SINC"))
        {
            scale.scale_mode = SWS_SINC;
        }
        else if (!strcmp(scale_mode, "SWS_LANCZOS"))
        {
            scale.scale_mode = SWS_LANCZOS;
        }
        else if (!strcmp(scale_mode, "SWS_SPLINE"))
        {
            scale.scale_mode = SWS_SPLINE;
        }
	}

    //int src_w = GetvalueInt(json, "src_w");
    scale.sws_ctx = NULL;
    ret = FFScale(&scale);
    deleteJson(json);
    return ret;
}
int main(int argc, char **argv)
{
    uint8_t *src_data[4], *dst_data[4];
    int src_linesize[4], dst_linesize[4];
    int src_w = 320, src_h = 240, dst_w, dst_h;
    enum AVPixelFormat src_pix_fmt = AV_PIX_FMT_YUV420P, dst_pix_fmt = AV_PIX_FMT_RGB24;
    const char *dst_size = NULL;
    const char *dst_filename = NULL;
    FILE *dst_file;
    int dst_bufsize;
    struct SwsContext *sws_ctx;
    int i, ret;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s output_file output_size\n"
                "API example program to show how to scale an image with libswscale.\n"
                "This program generates a series of pictures, rescales them to the given "
                "output_size and saves them to an output file named output_file\n."
                "\n", argv[0]);
        exit(1);
    }
    dst_filename = argv[1];
    dst_size     = argv[2];

    if (av_parse_video_size(&dst_w, &dst_h, dst_size) < 0) {
        fprintf(stderr,
                "Invalid size '%s', must be in the form WxH or a valid size abbreviation\n",
                dst_size);
        exit(1);
    }

    dst_file = fopen(dst_filename, "wb");
    if (!dst_file) {
        fprintf(stderr, "Could not open destination file %s\n", dst_filename);
        exit(1);
    }

    /* create scaling context */
    sws_ctx = sws_getContext(src_w, src_h, src_pix_fmt,
                             dst_w, dst_h, dst_pix_fmt,
                             SWS_BILINEAR, NULL, NULL, NULL);
    if (!sws_ctx) {
        fprintf(stderr,
                "Impossible to create scale context for the conversion "
                "fmt:%s s:%dx%d -> fmt:%s s:%dx%d\n",
                av_get_pix_fmt_name(src_pix_fmt), src_w, src_h,
                av_get_pix_fmt_name(dst_pix_fmt), dst_w, dst_h);
        ret = AVERROR(EINVAL);
        goto end;
    }

    /* allocate source and destination image buffers */
    if ((ret = av_image_alloc(src_data, src_linesize,
                              src_w, src_h, src_pix_fmt, 16)) < 0) {
        fprintf(stderr, "Could not allocate source image\n");
        goto end;
    }

    /* buffer is going to be written to rawvideo file, no alignment */
    if ((ret = av_image_alloc(dst_data, dst_linesize,
                              dst_w, dst_h, dst_pix_fmt, 1)) < 0) {
        fprintf(stderr, "Could not allocate destination image\n");
        goto end;
    }
    dst_bufsize = ret;

    for (i = 0; i < 100; i++) {
        /* generate synthetic video */
        fill_yuv_image(src_data, src_linesize, src_w, src_h, i);

        /* convert to destination format */
        sws_scale(sws_ctx, (const uint8_t * const*)src_data,
                  src_linesize, 0, src_h, dst_data, dst_linesize);

        /* write scaled image to file */
        fwrite(dst_data[0], 1, dst_bufsize, dst_file);
    }

    fprintf(stderr, "Scaling succeeded. Play the output file with the command:\n"
           "ffplay -f rawvideo -pix_fmt %s -video_size %dx%d %s\n",
           av_get_pix_fmt_name(dst_pix_fmt), dst_w, dst_h, dst_filename);

end:
    fclose(dst_file);
    av_freep(&src_data[0]);
    av_freep(&dst_data[0]);
    sws_freeContext(sws_ctx);
    return ret < 0;
}

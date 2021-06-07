#include "inc.h"


extern cJSON* mystr2json(char *text);
extern int GetvalueInt(cJSON *json, char *key);
extern cJSON* renewJson(cJSON *json, char *key, int ivalue, char *cvalue, cJSON *subJson);
extern cJSON* renewJsonInt(cJSON *json, char *key, int ivalue);
extern cJSON* renewJsonStr(cJSON *json, char *key, char *cvalue);
extern cJSON* deleteJson(cJSON *json);
//extern cJSON* renewJsonArray2(cJSON *json, char *key, short *value);
//extern inline int GetRtpInfo(uint8_t* dataPtr, int insize, RtpInfo *info);
extern int64_t get_sys_time();

typedef struct
{
    int Obj_id;
    cJSON *json;
    char *param;
    char *img_hnd;
    AVFrame *frame;
    int print;
    FILE *fp;
}ImageDenoiseObj;

HCSVC_API
int api_create_video_denoise_handle(char *handle);
HCSVC_API
int api_video_denoise_close(char *handle);
HCSVC_API
int api_video_denoise(char *handle, unsigned char *data[3], int linesize[3], int width, int height);


HCSVC_API
int api_create_image_denoise_handle(char *handle)
{
    long long ret = 0;
    if(handle)
    {
        ImageDenoiseObj *obj = (ImageDenoiseObj *)calloc(1, sizeof(ImageDenoiseObj));
        ret = (long long)obj;
        int handle_size = sizeof(long long);
        printf("api_create_image_denoise_handle: handle_size= %d \n", handle_size);
        printf("api_create_image_denoise_handle: obj= %x \n", obj);
        memcpy(handle, &ret, handle_size);
        long long *testp = (long long *)handle;
        printf("api_create_image_denoise_handle: testp[0]= %x \n", testp[0]);
        int id = (int)obj;//long long ???
        obj->Obj_id = id;
        ret = id;
    }
    return (int)(ret & 0x7FFFFFFF);
}
HCSVC_API
int api_image_denoise_init(char *handle, char *param)
{
    int ret = 0;

    ret = api_create_audio_codec_handle(handle);
    long long *testp = (long long *)handle;
    ImageDenoiseObj *obj = (ImageDenoiseObj *)testp[0];
    printf("api_image_denoise_init: param= %s \n", param);
    obj->json = mystr2json(param);
    printf("api_image_denoise_init: obj->json= %x \n", obj->json);
    obj->param = param;
    obj->print = GetvalueInt(obj->json, "print");

    obj->img_hnd = (char *)calloc(1, 8 * sizeof(char));

    ret = api_create_video_denoise_handle(obj->img_hnd);

    obj->frame = av_frame_alloc();
    if (!obj->frame) {
        fprintf(stderr, "api_image_denoise_init: Could not allocate audio frame\n");
        return -1;
    }

    return ret;
}
HCSVC_API
int api_image_denoise_close(char *handle)
{
    long long ret = 0;
    if(handle)
    {
        long long *testp = (long long *)handle;
        ImageDenoiseObj *obj = (ImageDenoiseObj *)testp[0];
        //
        if(obj->img_hnd)
        {
            api_video_denoise_close(obj->img_hnd);
            free(obj->img_hnd);
        }
        av_frame_free(&obj->frame);
        avcodec_close(obj->c);
        av_free(obj->c);
        if(obj->json)
        {
            api_json_free(obj->json);
            obj->json = NULL;
        }
        //
        free(obj);
	    testp[0] = 0;
        printf("api_image_denoise_close: ok \n");
    }
    return (int)ret;
}

HCSVC_API
int api_image_denoise(char *handle, char *data, char *param, char *outbuf, char *outparam[])
{
    int ret = 0;
    const char *tag = "string come from c code: api_image_denoise";
    cJSON *json;

    long long *testp = (long long *)handle;
    AudioCodecObj *obj = (AudioCodecObj *)testp[0];

    if (obj->c != NULL)
    {
        json = mystr2json(param);
        obj->param = (void *)json;
        int insize = GetvalueInt(json, "insize");

        ret = api_video_denoise(obj->img_hnd, obj->frame->data, obj->frame->linesize, width, height);
        deleteJson(json);
        obj->param = NULL;
    }
    return ret;
}
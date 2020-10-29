#include <stdio.h>
#include <dlfcn.h>

//#include "hcsvc.h"

#if 1

#ifdef __cplusplus
#define EXTERNC extern "C"
EXTERNC {
#else
#define EXTERNC
#endif

#ifdef _WIN32
#define HCSVC_API EXTERNC __declspec(dllexport)
#else
#define HCSVC_API __attribute__ ((__visibility__("default")))
#endif


HCSVC_API
void api_video_encode_open(int id, char *param);
HCSVC_API
int api_video_encode_one_frame(int id, char *data, char *param, char *outbuf, char *outparam[]);
HCSVC_API
int api_video_encode_close(int id);
HCSVC_API
void* api_renew_json_int(void *json, char *key, int ivalue);
HCSVC_API
void* api_renew_json_str(void *json, char *key, char *cvalue);
HCSVC_API
char* api_json2str(void *json);
HCSVC_API
void api_json2str_free(char *jsonstr);
HCSVC_API
int* api_get_array_by_str(char *text, char tok, int *num);
HCSVC_API
void api_get_array_free(int *pktSize);
HCSVC_API
long long api_get_time_stamp_ll(void);

#ifdef __cplusplus
}
#endif


#endif

static void *pHcsvclib = NULL;                     //指向so文件的指针

typedef int (*Fun_video_encode_open)(int id, char *param);
typedef int (*Fun_video_encode_one_frame)(int id, char *data, char *param, char *outbuf, char *outparam[]);
typedef int (*Fun_video_encode_close)(int id);
typedef void* (*Fun_renew_json_int)(void *json, char *key, int ivalue);
typedef void* (*Fun_renew_json_str)(void *json, char *key, char *cvalue);
typedef char* (*Fun_json2str)(void *json);
typedef void (*Fun_json2str_free)(char *jsonstr);
typedef int* (*Fun_get_array_by_str)(char *text, char tok, int *num);
typedef void (*Fun_get_array_free)(int *pktSize);
typedef long long (*Fun_get_time_stamp_ll)(void);

//函数指针
Fun_video_encode_open fun_video_encode_open = NULL;
Fun_video_encode_one_frame fun_video_encode_one_frame = NULL;
Fun_video_encode_close fun_video_encode_close = NULL;
Fun_renew_json_int fun_renew_json_int = NULL;
Fun_renew_json_str fun_renew_json_str = NULL;
Fun_json2str fun_json2str = NULL;
Fun_json2str_free fun_json2str_free = NULL;
Fun_get_array_by_str fun_get_array_by_str = NULL;
Fun_get_array_free fun_get_array_free = NULL;
Fun_get_time_stamp_ll fun_get_time_stamp_ll = NULL;

HCSVC_API
int hcsvc_dll_close()
{
    if(pHcsvclib)
    {
        //关闭so文件
        dlclose(pHcsvclib);
        pHcsvclib = NULL;
        //
        fun_video_encode_open = NULL;
        fun_video_encode_one_frame = NULL;
        fun_video_encode_close = NULL;
        fun_renew_json_int = NULL;
        fun_renew_json_str = NULL;
        fun_json2str = NULL;
        fun_json2str_free = NULL;
        fun_get_array_by_str = NULL;
        fun_get_array_free = NULL;
        fun_get_time_stamp_ll = NULL;
        return 0;
    }
    return -1;
}
HCSVC_API
int hcsvc_dll_init(char *filename)
{
    printf("hcsvc_dll_init start \n");
    //打开so文件
    //为了方便演示，我将库文件和可执行文件放在同一个目录下
    if(filename != NULL && strcmp(filename, ""))
    {
    }
    else{
        filename = "./libhcsvcapi.so";
    }
    printf("hcsvc_dll_init: filename= %s \n", filename);

    pHcsvclib = dlopen(filename, RTLD_NOW | RTLD_GLOBAL);
    if( NULL == pHcsvclib )
    {
        printf("Can't open the libhcsvcapi.so \n");
        return 0;
    }

    fun_video_encode_open = dlsym(pHcsvclib, "api_video_encode_open");
    if( NULL == fun_video_encode_open )
    {
        printf("Can't load function 'fun_video_encode_open' \n");
        hcsvc_dll_close();
        return 0;
    }

    fun_video_encode_one_frame = dlsym(pHcsvclib, "api_video_encode_one_frame");
    if( NULL == fun_video_encode_one_frame )
    {
        printf("Can't load function 'fun_video_encode_one_frame' \n");
        hcsvc_dll_close();
        return 0;
    }

    fun_video_encode_close = dlsym(pHcsvclib, "api_video_encode_close");
    if( NULL == fun_video_encode_close )
    {
        printf("Can't load function 'fun_video_encode_close' \n");
        hcsvc_dll_close();
        return 0;
    }

    fun_renew_json_int = dlsym(pHcsvclib, "api_renew_json_int");
    if( NULL == fun_renew_json_int )
    {
        printf("Can't load function 'fun_renew_json_int' \n");
        hcsvc_dll_close();
        return 0;
    }

    fun_renew_json_str = dlsym(pHcsvclib, "api_renew_json_str");
    if( NULL == fun_renew_json_str )
    {
        printf("Can't load function 'fun_renew_json_str' \n");
        hcsvc_dll_close();
        return 0;
    }

    fun_json2str = dlsym(pHcsvclib, "api_json2str");
    if( NULL == fun_json2str )
    {
        printf("Can't load function 'fun_json2str' \n");
        hcsvc_dll_close();
        return 0;
    }

    fun_json2str_free = dlsym(pHcsvclib, "api_json2str_free");
    if( NULL == fun_json2str_free )
    {
        printf("Can't load function 'fun_json2str_free' \n");
        hcsvc_dll_close();
        return 0;
    }

    fun_get_array_by_str = dlsym(pHcsvclib, "api_get_array_by_str");
    if( NULL == fun_get_array_by_str )
    {
        printf("Can't load function 'fun_get_array_by_str' \n");
        hcsvc_dll_close();
        return 0;
    }

    fun_get_array_free = dlsym(pHcsvclib, "api_get_array_free");
    if( NULL == fun_get_array_free )
    {
        printf("Can't load function 'fun_get_array_free' \n");
        hcsvc_dll_close();
        return 0;
    }

    fun_get_time_stamp_ll = dlsym(pHcsvclib, "api_get_time_stamp_ll");
    if( NULL == fun_get_time_stamp_ll )
    {
        printf("Can't load function 'fun_get_time_stamp_ll' \n");
        hcsvc_dll_close();
        return 0;
    }
    return (int)pHcsvclib;
}

int test_main() {
    //调用成功加载的函数
    //if(funAdd)
    //{
    //    printf("5 + 8 = %d\n", funAdd(5, 8));
    //}


	return 0;
}
HCSVC_API
void i2_video_encode_open(int id, char *param)
{
    fun_video_encode_open(id, param);
}
HCSVC_API
int i2_video_encode_one_frame(int id, char *data, char *param, char *outbuf, char *outparam[])
{
    if(fun_video_encode_one_frame)
    {
        return fun_video_encode_one_frame(id, data, param, outbuf, outparam);
    }
    else{
        return -1;
    }
}
HCSVC_API
int i2_video_encode_close(int id)
{
    if(fun_video_encode_close)
    {
        return fun_video_encode_close(id);
    }
    else{
        return -1;
    }
}
HCSVC_API
void* i2_renew_json_int(void *json, char *key, int ivalue)
{
    if(fun_renew_json_int)
    {
        return fun_renew_json_int(json, key, ivalue);
    }
    else{
        return NULL;
    }
}
HCSVC_API
void* i2_renew_json_str(void *json, char *key, char *cvalue)
{
    if(fun_renew_json_str)
    {
        return fun_renew_json_str(json, key, cvalue);
    }
    else{
        return NULL;
    }
}
HCSVC_API
char* i2_json2str(void *json)
{
    if(fun_json2str)
    {
        return fun_json2str(json);
    }
    else{
        return NULL;
    }
}
HCSVC_API
void i2_json2str_free(char *jsonstr)
{
    if(fun_json2str_free)
    {
        fun_json2str_free(jsonstr);
    }
}
HCSVC_API
int* i2_get_array_by_str(char *text, char tok, int *num)
{
    if(fun_get_array_by_str)
    {
        return fun_get_array_by_str(text, tok, num);
    }
    else{
        return NULL;
    }
}
HCSVC_API
void i2_get_array_free(int *pktSize)
{
    if(fun_get_array_free)
    {
        fun_get_array_free(pktSize);
    }
}
HCSVC_API
long long i2_get_time_stamp_ll(void)
{
    if(fun_get_time_stamp_ll)
    {
        return fun_get_time_stamp_ll();
    }
    return 0;
}

#if 0
void i1_video_encode_open(int id, char *param)
{
    api_video_encode_open(id, param);
}
int i1_video_encode_one_frame(int id, char *data, char *param, char *outbuf, char *outparam[])
{
    return api_video_encode_one_frame(id, data, param, outbuf, outparam);

}
int i1_video_encode_close(int id)
{
    return api_video_encode_close(id);

}
void* i1_renew_json_int(void *json, char *key, int ivalue)
{
    return api_renew_json_int(json, key, ivalue);
}
void* i1_renew_json_str(void *json, char *key, char *cvalue)
{
    return api_renew_json_str(json, key, cvalue);
}
char* i1_json2str(void *json)
{
    return api_json2str(json);
}
void i1_json2str_free(char *jsonstr)
{
    api_json2str_free(jsonstr);
}

int* i1_get_array_by_str(char *text, char tok, int *num)
{
    return api_get_array_by_str(text, tok, num);
}
void i1_get_array_free(int *pktSize)
{
    api_get_array_free(pktSize);
}

long long i1_get_time_stamp_ll(void)
{
    return api_get_time_stamp_ll();
}
#endif
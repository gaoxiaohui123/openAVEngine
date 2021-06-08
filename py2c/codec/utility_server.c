/*
 * Copyright (c) 2020
 * Created by gxh_20200602
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#ifndef _WIN32
#include <unistd.h>
#else
#include <io.h>
#endif

//#include <memory>
//#include <thread> // std::thread, std::this_thread::sleep_for
//#include <chrono> // std::chrono::seconds

#ifdef _WIN32
#include <direct.h>
#endif

#include <math.h>
#include <inttypes.h>
//#include <map>

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
//#include <vld.h>
#include <Mmsystem.h>
#else
#include <sys/time.h>
#endif


#include "cJSON.h"
//#include "inc.h"


#include "utility_server.h"

#ifdef _WIN32
#define HCSVC_API __declspec(dllexport)
//#define HCSVC_API __declspec(dllimport)
#else
#define HCSVC_API __attribute__ ((__visibility__("default")))
#endif


//#define LOSS_BUF_SIZE 128//1024
#define MAX_SSRC_NUM (1 << 16)
typedef struct
{
    unsigned int ssrc;
    int64_t time_stamp;
    //float buf0[LOSS_BUF_SIZE];
    //float buf1[LOSS_BUF_SIZE];
    int idx;
    float sum_loss_rate;
    int zero_cnt;
    int none_zero_cnt;
}LossManager;
LossManager globLossManagers[MAX_SSRC_NUM];
static int status0 = 0;
static int glob_time_offset = 0;

typedef struct
{
    unsigned int ssrc;
    //int idx;
    PacketManager *obj;
}LossHandle;
typedef struct
{
    int count;
    int size;
    LossHandle *hndList;
}AddrManager;
AddrManager globAddrManager[MAX_SSRC_NUM];//video
AddrManager globAddrManager2[MAX_SSRC_NUM];//audio
static int addr_status = 0;
static int addr_status2 = 0;

#define PKT_LOSS_INTERVAL 500
#define MAX_PKT_DELAY 500
int loglevel = 0;//1;//GetvalueInt(json, "loglevel");



//==================for json=============================

//修改对象的值
//cJSON_ReplaceItemInObject(item,"word",cJSON_CreateString("nihaoxiaobai"));
//cJSON_AddItemToObject(body, "Info", filter_root);
cJSON* renewJson(cJSON *json, char *key, int ivalue, char *cvalue, cJSON *subJson)
{
    cJSON *ret  = json;
    if(NULL == json)
    {
        ret = cJSON_CreateObject(); //创建JSON对象
        //printf("renewJson: cJSON_CreateObject: ret=%x \n", ret);
        if(NULL == ret)
        {
            //error happend here
            return NULL;
        }
    }
    if(NULL != key)
    {
        if(NULL != subJson)
        {
            cJSON* ret2 = cJSON_AddItemToObject(ret, key, subJson);
            if(ret2)
            {
                //cJSON_Delete(ret2);
            }
        }
        else if(NULL != cvalue)
        {
            cJSON* ret2 = cJSON_AddStringToObject(ret, key, cvalue);
            if(ret2)
            {
                //cJSON_Delete(ret2);
            }
        }
        else
        {
            cJSON* ret2 = cJSON_AddNumberToObject(ret, key, ivalue);
            //printf("renewJson: cJSON_CreateObject: ret2=%x \n", ret2);
            if(ret2)
            {
                //cJSON_Delete(ret2);
            }
        }
    }
    else{
        cJSON_Delete(ret); //释放json对象
    }
    return ret;
}

//static cJSON* deleteJson(cJSON *json)
//{
//    return renewJson(json, NULL, 0, NULL, NULL);
//}
cJSON* deleteJson(cJSON *json)
{
    if(json)
    {
        cJSON_Delete(json); //释放json对象
    }
    return 0;
    //return renewJson(json, NULL, 0, NULL, NULL);
}
HCSVC_API
void api_json_free(void *json)
{
    deleteJson((cJSON *)json);
}
cJSON* mystr2json(char *text)
{
    char *out;
    cJSON *json = NULL;
    if (text == NULL || !strcmp(text, ""))
    {
        return json;
    }

    json = cJSON_Parse(text);
    //if (!json) {
    //    printf("Error before: [%s]\n",cJSON_GetErrorPtr());
    //} else {
        //将传入的JSON结构转化为字符串
        //out=cJSON_Print(json);
        //cJSON_free(out);
    //}
    return json;
}
HCSVC_API
void *api_str2json(char *parmstr)
{
    cJSON *json = mystr2json(parmstr);
    return (void *)json;
}
float GetvalueFloat(cJSON *json, char *key)
{
    cJSON *item = cJSON_GetObjectItem(json, key);
    if(cJSON_IsNull(item))
    {
    }
    else if(cJSON_IsNumber(item))
    {
        return item->valuedouble;
    }
    return 0;
}
int GetvalueInt(cJSON *json, char *key)
{
    cJSON *item = cJSON_GetObjectItem(json, key);
    if(cJSON_IsNull(item))
    {
    }
    else if(cJSON_IsNumber(item))
    {
        return item->valueint;
    }
    return 0;
}
//cjson 遍历
static void foreach()
{
    char *parmstr = "{\"video\":{\"mjpeg\":\"1x2\", \"raw\":\"3x4\"}}";
    cJSON *json = (cJSON *)api_str2json(parmstr);
    cJSON *item = json;
    do{
        if(cJSON_IsObject(item))
        {

        }
        if(!cJSON_IsNull(item))
        {
            int type = item->type;
            printf("api_get_dev_info: type=%d \n", type);
            char *key = item->string;

            if(key)
            {
                printf("api_get_dev_info: key=%s \n", key);
                cJSON *next = item;
                do{
                    next = next->next;
                    //printf("api_get_dev_info: next=%x \n", next);
                    if(!cJSON_IsNull(next) && !cJSON_IsInvalid(next) && next)
                    {
                        int type = next->type;
                        printf("api_get_dev_info: type=%d \n", type);
                        char *key = next->string;
                        printf("api_get_dev_info: key=%s \n", key);
                    }
                }while(!cJSON_IsNull(next) && !cJSON_IsInvalid(next) && next);
            }
        }
        item = item->child;

    }while(!cJSON_IsNull(item) && item);
}
HCSVC_API
void* api_renew_json_int(void *json, char *key, int ivalue)
{
    json = api_delete_item(json, key);
    return (void *)renewJson((cJSON *)json, key, ivalue, NULL, NULL);
}
HCSVC_API
void* api_renew_json_str(void *json, char *key, char *cvalue)
{
    json = api_delete_item(json, key);
    return (void *)renewJson((cJSON *)json, key, 0, cvalue, NULL);
}
HCSVC_API
char* api_json2str(void *json)
{
    if(json)
    {
        return cJSON_Print((cJSON *)json);
    }
    return NULL;
}
HCSVC_API
void api_json2str_free(char *jsonStr)
{
    if(jsonStr)
    {
        cJSON_free(jsonStr);
    }
}
//=======================================================
#ifdef _WIN32
#if 0
int64_t get_sys_time()
{
	LARGE_INTEGER m_nFreq;
	LARGE_INTEGER m_nTime;
	QueryPerformanceFrequency(&m_nFreq); // ��ȡʱ������
	QueryPerformanceCounter(&m_nTime);//��ȡ��ǰʱ��
	//long long time = (m_nTime.QuadPart * 1000000 / m_nFreq.QuadPart);//΢��
	int64_t time = (m_nTime.QuadPart * 1000 / m_nFreq.QuadPart);//����
	return time;
}
#elif(0)
#include <time.h>
#include <windows.h>
int gettimeofday(struct timeval *tp, void *tzp)
{
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;

   GetLocalTime(&wtm);
   tm.tm_year    = wtm.wYear - 1900;
   tm.tm_mon    = wtm.wMonth - 1;
   tm.tm_mday    = wtm.wDay;
   tm.tm_hour    = wtm.wHour;
   tm.tm_min    = wtm.wMinute;
   tm.tm_sec    = wtm.wSecond;
   tm. tm_isdst    = -1;
   clock = mktime(&tm);
   tp->tv_sec = clock;
   tp->tv_usec = wtm.wMilliseconds * 1000;
   //printf("gettimeofday: tp->tv_usec=%lld \n", tp->tv_usec);
   printf("gettimeofday: tp->tv_sec=%lld \n", tp->tv_sec);
   return (0);
}

int64_t get_sys_time()
{
	struct timeval tBegin;
	//struct timeval tEnd;
	gettimeofday(&tBegin, NULL);
	//...process
	//gettimeofday(&tEnd, NULL);
	//long deltaTime = 1000000L * (tEnd.tv_sec - tBegin.tv_sec) + (tEnd.tv_usec - tBegin.tv_usec);
	int64_t time = (1000000L * (int64_t)tBegin.tv_sec + tBegin.tv_usec) / 1000;//us-->ms
	//printf("get_sys_time: time=%lld \n", time);
	time -= glob_time_offset;
    return time;
}

#else
#include <time.h>
#include <windows.h>
int64_t get_sys_time()
{
	struct timeval tBegin;
	//struct timeval tEnd;
	gettimeofday(&tBegin, NULL);
	//...process
	//gettimeofday(&tEnd, NULL);
	//long deltaTime = 1000000L * (tEnd.tv_sec - tBegin.tv_sec) + (tEnd.tv_usec - tBegin.tv_usec);
	int64_t time = (1000000L * (int64_t)tBegin.tv_sec + tBegin.tv_usec) / 1000;//us-->ms
	time -= glob_time_offset;
    return time;
}
int64_t get_sys_time2()
{
	struct timespec tBegin;
	//struct timeval tEnd;
	gettimeofday(&tBegin, NULL);
	//...process
	//gettimeofday(&tEnd, NULL);
	//long deltaTime = 1000000L * (tEnd.tv_sec - tBegin.tv_sec) + (tEnd.tv_usec - tBegin.tv_usec);
	//int64_t time = (1000000L * (int64_t)tBegin.tv_sec + tBegin.tv_usec);//us
	int64_t time = (1000000000L * (int64_t)tBegin.tv_sec + tBegin.tv_nsec);//ns
    return time;
}
#endif

#else
#include <sys/time.h>
int64_t get_sys_time()
{
	struct timeval tBegin;
	//struct timeval tEnd;
	gettimeofday(&tBegin, NULL);
	//...process
	//gettimeofday(&tEnd, NULL);
	//long deltaTime = 1000000L * (tEnd.tv_sec - tBegin.tv_sec) + (tEnd.tv_usec - tBegin.tv_usec);
	int64_t time = (1000000L * tBegin.tv_sec + tBegin.tv_usec) / 1000;//us-->ms
	time -= glob_time_offset;
    return time;
}
int64_t get_sys_time2()
{
	struct timespec tBegin;
	//struct timeval tEnd;
	gettimeofday(&tBegin, NULL);
	//...process
	//gettimeofday(&tEnd, NULL);
	//long deltaTime = 1000000L * (tEnd.tv_sec - tBegin.tv_sec) + (tEnd.tv_usec - tBegin.tv_usec);
	//int64_t time = (1000000L * (int64_t)tBegin.tv_sec + tBegin.tv_usec);//us
	int64_t time = (1000000000L * (int64_t)tBegin.tv_sec + tBegin.tv_nsec);//ns
    return time;
}
#endif

#if 0
void get_time(int64_t time, char *ctime, int offset)
{
#ifdef _WIN32
	if (time == 0)
	{
		time = (int64_t)av_gettime() + offset;
	}
	time += 11644473600000000;
	time *= 10;

	FILETIME ft;
	FILETIME ft2;
	ft.dwHighDateTime = time >> 32;
	ft.dwLowDateTime = time & 0xFFFFFFFF;
	FileTimeToLocalFileTime(&ft, &ft2);

	SYSTEMTIME st;
	FileTimeToSystemTime(&ft2, &st);
	sprintf(ctime, "%d-%02d-%02d %02d:%02d:%02d.%03d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
#else
    struct timeval tv;
    time_t nowtime;
    struct tm *nowtm;
    //char tmbuf[64], buf[64];
    gettimeofday(&tv, NULL);
    int64_t tvalue = (long long)tv.tv_sec * 1000000 + tv.tv_usec + offset;//us
#if defined(__ANDROID__)
    //tvalue += 116444736000000000;
    //if (logfp) { fprintf(logfp, "FF_GET_TIME2:tvalue= %lld \n", tvalue); fflush(logfp); }
#endif
    tv.tv_sec = (long)(tvalue / 1000000);
    tv.tv_usec = (long)(tvalue % 1000000);
    nowtime = tv.tv_sec; //nowtime
    nowtm = localtime(&nowtime);

    strftime(ctime, 64 * sizeof(char), "%Y-%m-%d %H:%M:%S", nowtm);
    sprintf(&ctime[strlen(ctime)], ".%03d", (int)(tv.tv_usec / 1000));
    //snprintf(buf, sizeof buf, "%s.%06d", tmbuf, tv.tv_usec);
    //printf("FF_GET_TIME2: ctime= %s \n", ctime);
#endif
    //printf("ctime = %s \n", ctime);
}
#endif

#ifdef _WIN32

#include <time.h>
#include <windows.h>
int64_t get_sys_time0()
{
	struct timeval tBegin;
	//struct timeval tEnd;
	gettimeofday(&tBegin, NULL);
	//...process
	//gettimeofday(&tEnd, NULL);
	//long deltaTime = 1000000L * (tEnd.tv_sec - tBegin.tv_sec) + (tEnd.tv_usec - tBegin.tv_usec);
	int64_t time = (1000000L * (int64_t)tBegin.tv_sec + tBegin.tv_usec) / 1000;//us-->ms
    return time;
}
#else
#include <sys/time.h>
int64_t get_sys_time0()
{
	struct timeval tBegin;
	//struct timeval tEnd;
	gettimeofday(&tBegin, NULL);
	//...process
	//gettimeofday(&tEnd, NULL);
	//long deltaTime = 1000000L * (tEnd.tv_sec - tBegin.tv_sec) + (tEnd.tv_usec - tBegin.tv_usec);
	int64_t time = (1000000L * tBegin.tv_sec + tBegin.tv_usec) / 1000;//us-->ms
    return time;
}
#endif

#if 0
void get_time(int64_t time, char *ctime, int offset)
{
#ifdef _WIN32
	if (time == 0)
	{
		time = (int64_t)av_gettime() + offset;
	}
	time += 11644473600000000;
	time *= 10;

	FILETIME ft;
	FILETIME ft2;
	ft.dwHighDateTime = time >> 32;
	ft.dwLowDateTime = time & 0xFFFFFFFF;
	FileTimeToLocalFileTime(&ft, &ft2);

	SYSTEMTIME st;
	FileTimeToSystemTime(&ft2, &st);
	sprintf(ctime, "%d-%02d-%02d %02d:%02d:%02d.%03d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
#else
    struct timeval tv;
    time_t nowtime;
    struct tm *nowtm;
    //char tmbuf[64], buf[64];
    gettimeofday(&tv, NULL);
    int64_t tvalue = (long long)tv.tv_sec * 1000000 + tv.tv_usec + offset;//us
#if defined(__ANDROID__)
    //tvalue += 116444736000000000;
    //if (logfp) { fprintf(logfp, "FF_GET_TIME2:tvalue= %lld \n", tvalue); fflush(logfp); }
#endif
    tv.tv_sec = (long)(tvalue / 1000000);
    tv.tv_usec = (long)(tvalue % 1000000);
    nowtime = tv.tv_sec; //nowtime
    nowtm = localtime(&nowtime);

    strftime(ctime, 64 * sizeof(char), "%Y-%m-%d %H:%M:%S", nowtm);
    sprintf(&ctime[strlen(ctime)], ".%03d", (int)(tv.tv_usec / 1000));
    //snprintf(buf, sizeof buf, "%s.%06d", tmbuf, tv.tv_usec);
    //printf("FF_GET_TIME2: ctime= %s \n", ctime);
#endif
    //printf("ctime = %s \n", ctime);
}
#endif

HCSVC_API
void api_set_time_offset(int time_offset)
{
    glob_time_offset = time_offset;
#if 0
    {
        char filename[256] = "time_offset_gxh_";
        char ctmp[32] = "";
        int fileidx = rand() % 10000;
        sprintf(ctmp, "%d", fileidx);
        strcat(filename, ctmp);
        strcat(filename, ".txt");
        FILE *logfp = fopen(filename, "w");
        if(logfp)
        {
            fprintf(logfp, "api_set_time_offset: time_offset=%d \n", time_offset);
            fflush(logfp);
            fclose(logfp);
        }
    }
#endif
}

static int64_t get_time_stamp(void)
{
    int64_t time_stamp = get_sys_time();
    //printf("get_time_stamp: time_stamp= %lld \n", time_stamp);
    return time_stamp;
}


static char *get_extern_info(char *data, int insize)
{
    char *ret = NULL;
    RTP_FIXED_HEADER  *rtp_hdr = NULL;
    EXTEND_HEADER *rtp_ext = NULL;
    rtp_hdr = (RTP_FIXED_HEADER *)data;
    int isrtp = (rtp_hdr->version == 2 &&  //版本号，此版本固定为2								V
            rtp_hdr->padding	== 0 &&  //														P
            rtp_hdr->csrc_len == 0 && //												X
            rtp_hdr->payload == H264_PLT //负载类型号
            );
    if(isrtp && rtp_hdr->extension)
    {
        rtp_ext = (EXTEND_HEADER *)&data[sizeof(RTP_FIXED_HEADER)];
        int rtp_extend_length = rtp_ext->rtp_extend_length;
        rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
        rtp_extend_length = (rtp_extend_length + 1) << 2;
        int rtp_header_size = sizeof(RTP_FIXED_HEADER) + rtp_extend_length;
        int refs = rtp_ext->refs;
		int ref_idx = rtp_ext->ref_idx;
		int ref_idc = rtp_ext->ref_idc;
		int res_num = rtp_ext->res_num;
		int res_idx = rtp_ext->res_idx;
		int qua_num = rtp_ext->qua_num;
		int qua_idx = rtp_ext->qua_idx;
		int enable_fec = rtp_ext->enable_fec;
		int refresh_idr = rtp_ext->refresh_idr;
		int is_lost_packet = rtp_ext->nack.nack0.is_lost_packet;
		int loss_rate = rtp_ext->nack.nack0.loss_rate;
		//int time_status = rtp_ext->nack.nack0.time_status;
		int time_offset = rtp_ext->nack.nack0.time_offset;
		int seqnum = rtp_hdr->seq_no;

		unsigned int timestamp = rtp_hdr->timestamp;
		uint64_t time_stamp0 = rtp_ext->nack.nack0.time_info.time_stamp0;
	    uint64_t time_stamp1 = rtp_ext->nack.nack0.time_info.time_stamp1;
	    int64_t packet_time_stamp = time_stamp0 | (time_stamp1 << 32);
		//int64_t packet_time_stamp = rtp_ext->nack.nack0.time_info.time_stamp;
		//printf("get_extern_info: refs= %d, ref_idx= %d \n", refs, ref_idx);
		cJSON *pJsonRoot  = NULL;
		char *key = "refs";
		int ivalue = refs;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);
		key = "ref_idx";
		ivalue = ref_idx;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);
		key = "ref_idc";
		ivalue = ref_idc;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);
		key = "res_num";
		ivalue = res_num;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);
		key = "res_idx";
		ivalue = res_idx;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);
		key = "qua_num";
		ivalue = qua_num;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);

        key = "is_header";
		ivalue = (rtp_header_size == insize);
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);

		key = "qua_idx";
		ivalue = qua_idx;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);
		key = "is_lost_packet";
		ivalue = is_lost_packet;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);
		key = "loss_rate";
		ivalue = loss_rate;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);
		//key = "time_status";
		//ivalue = time_status;
		//pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);
		key = "time_offset";
		ivalue = time_offset;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);
		key = "seqnum";
		ivalue = seqnum;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);
		key = "enable_fec";
		ivalue = enable_fec;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);
		key = "refresh_idr";
		ivalue = refresh_idr;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);
		key = "timestamp";
		ivalue = timestamp;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);
		key = "packet_time_stamp";
		char ctmp[32] = "";
	    sprintf(ctmp, "%ld", packet_time_stamp);
		pJsonRoot = api_renew_json_str(pJsonRoot, key, ctmp);

		ret = api_json2str(pJsonRoot); //free(ret);//返回json字符串，注意外面用完要记得释放空间

		api_json_free(pJsonRoot);

    }
    return ret;
}
static char *get_audio_extern_info(char *data)
{
    char *ret = NULL;
    RTP_FIXED_HEADER  *rtp_hdr = NULL;
    AUDIO_EXTEND_HEADER *rtp_ext = NULL;
    rtp_hdr = (RTP_FIXED_HEADER *)data;
    int isrtp = (rtp_hdr->version == 2 &&  //版本号，此版本固定为2								V
            rtp_hdr->padding	== 0 &&  //														P
            rtp_hdr->csrc_len == 0 && //												X
            rtp_hdr->payload == AAC_PLT //负载类型号
            );
    if(isrtp && rtp_hdr->extension)
    {
        rtp_ext = (AUDIO_EXTEND_HEADER *)&data[sizeof(RTP_FIXED_HEADER)];
		int is_lost_packet = rtp_ext->nack.nack0.is_lost_packet;
		int loss_rate = rtp_ext->nack.nack0.loss_rate;
		//int time_status = rtp_ext->nack.nack0.time_status;
		int time_offset = rtp_ext->nack.nack0.time_offset;
		int seqnum = rtp_hdr->seq_no;

		unsigned int timestamp = rtp_hdr->timestamp;
		uint64_t time_stamp0 = rtp_ext->nack.nack0.time_info.time_stamp0;
	    uint64_t time_stamp1 = rtp_ext->nack.nack0.time_info.time_stamp1;
	    int64_t packet_time_stamp = time_stamp0 | (time_stamp1 << 32);
		//int64_t packet_time_stamp = rtp_ext->nack.nack0.time_info.time_stamp;
		//printf("get_extern_info: refs= %d, ref_idx= %d \n", refs, ref_idx);
		cJSON *pJsonRoot  = NULL;

		char *key = "is_lost_packet";
		int ivalue = is_lost_packet;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);
		key = "loss_rate";
		ivalue = loss_rate;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);
		//key = "time_status";
		//ivalue = time_status;
		//pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);
		key = "time_offset";
		ivalue = time_offset;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);
		key = "seqnum";
		ivalue = seqnum;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);
		key = "timestamp";
		ivalue = timestamp;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);
		key = "packet_time_stamp";
		char ctmp[32] = "";
	    sprintf(ctmp, "%ld", packet_time_stamp);
		pJsonRoot = api_renew_json_str(pJsonRoot, key, ctmp);

		ret = api_json2str(pJsonRoot); //free(ret);//返回json字符串，注意外面用完要记得释放空间

		api_json_free(pJsonRoot);

    }
    return ret;
}
HCSVC_API
long long api_get_time_stamp_ll(void)
{
    int64_t time_stamp = get_sys_time();
    //printf("get_time_stamp: time_stamp= %lld \n", time_stamp);
    return time_stamp;
}

HCSVC_API
int api_get_time(char *outparam[])
{
    int64_t time_stamp = get_sys_time();
    char *text = outparam[0];
    memset(text, 0, sizeof(char) * TIME_OUTCHAR_SIZE);
    //sprintf(text, "%ld", time_stamp);
    sprintf(text, "%lld", time_stamp);
    if(!time_stamp || !strcmp(text, ""))
    {
        printf("error: api_get_time: time_stamp=%lld \n", time_stamp);
        printf("error: api_get_time: text=%x \n", text);
        printf("error: api_get_time: text=%s \n", text);
    }
    return (int)time_stamp;
}
HCSVC_API
int api_get_time2(char *handle, char *outparam[])
{
    int64_t time_stamp = get_sys_time();
    if(handle)
    {
        int handle_size = sizeof(long long);
        char *text = handle;
        sprintf(text, "%lld", time_stamp);
        outparam[0] = handle;
        //memcpy(handle, &time_stamp, handle_size);
    }
    return (int)time_stamp;
}
HCSVC_API
int api_memcpy(char *data, int size, char *outparam[])
{
    char *dst = outparam[0];
    memcpy(dst, data, size);
    return size;
}
HCSVC_API
int api_renew_time_stamp(char *data)
{
    int64_t time_stamp = get_sys_time();
    //sprintf(text, "%ld", time_stamp);
    //outparam[0] = text;
    if(data != NULL)
    {
        RTP_FIXED_HEADER  *rtp_hdr = NULL;
        EXTEND_HEADER *rtp_ext = NULL;
        rtp_hdr = (RTP_FIXED_HEADER *)data;
        if(rtp_hdr->extension)
        {
            rtp_ext = (EXTEND_HEADER *)&data[sizeof(RTP_FIXED_HEADER)];
            //int time_status = rtp_ext->nack.nack0.time_status;
		    //if(time_status == 0)
		    {
		        rtp_ext->nack.nack0.time_info.time_stamp0 = time_stamp & 0xFFFFFFFF;
			    rtp_ext->nack.nack0.time_info.time_stamp1 = (time_stamp >> 32) & 0xFFFFFFFF;
		        //rtp_ext->nack.nack0.time_info.time_stamp = time_stamp;
		    }
        }
    }
    return (int)time_stamp;
}

HCSVC_API
int api_get_extern_info(char *data, int insize, char *outparam[])
{
    int ret = 0;
    char *extend_info = get_extern_info(data, insize);
    if(NULL != extend_info)
    {
        memcpy(outparam[0], extend_info, strlen(extend_info));
        ret = strlen(extend_info);
        //free(extend_info);
        api_json2str_free(extend_info);
    }
    return ret;
}
HCSVC_API
int api_get_audio_extern_info(char *data, char *outparam[])
{
    int ret = 0;
    char *extend_info = get_audio_extern_info(data);
    if(NULL != extend_info)
    {
        memcpy(outparam[0], extend_info, strlen(extend_info));
        ret = strlen(extend_info);
        //free(extend_info);
        api_json2str_free(extend_info);
    }
    return ret;
}
//HCSVC_API
//void api_get_info_test(char *outparam[])
//{
//    strcpy(outparam[0], "api_get_info_test");
//}
HCSVC_API
int api_isrtp(char *dataPtr, int size, int raw_offset, int type)
{
    int ret = 0;
    if(size < sizeof(RTP_FIXED_HEADER))
    {
        return -1;
    }
    RTP_FIXED_HEADER* rtp_hdr = (RTP_FIXED_HEADER*)&dataPtr[raw_offset];
    int isrtp = (rtp_hdr->version == 2 &&  //版本号，此版本固定为2								V
                rtp_hdr->padding	== 0 &&  //														P
                rtp_hdr->csrc_len == 0 && //												X
                (rtp_hdr->payload == type) //负载类型号
                );
    if(!isrtp)
    {
        return -2;
    }
    if(size < (sizeof(RTP_FIXED_HEADER) + sizeof(EXTEND_HEADER)))
    {
        return -3;
    }
    if(rtp_hdr->payload == AAC_PLT)
    {
        AUDIO_EXTEND_HEADER *rtp_ext = (AUDIO_EXTEND_HEADER *)&dataPtr[raw_offset + sizeof(RTP_FIXED_HEADER)];
        if(rtp_hdr->extension)
        {
            int rtp_extend_profile = rtp_ext->rtp_extend_profile;
            if(rtp_extend_profile != EXTEND_PROFILE_ID)
            {
                return -4;
            }
            int extend_size = sizeof(AUDIO_EXTEND_HEADER);
            int rtp_extend_length = rtp_ext->rtp_extend_length;
            rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
            rtp_extend_length = (rtp_extend_length + 1) << 2;
            if(rtp_extend_length != extend_size)
            {
                return -5;
            }
            ret = 1;
        }
    }
    else if(rtp_hdr->payload == H264_PLT)
    {
        EXTEND_HEADER *rtp_ext = (EXTEND_HEADER *)&dataPtr[raw_offset + sizeof(RTP_FIXED_HEADER)];
        if(rtp_hdr->extension)
        {
            int rtp_extend_profile = rtp_ext->rtp_extend_profile;
            if(rtp_extend_profile != EXTEND_PROFILE_ID)
            {
                return -4;
            }
            int extend_size = sizeof(EXTEND_HEADER);
            int rtp_extend_length = rtp_ext->rtp_extend_length;
            rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
            rtp_extend_length = (rtp_extend_length + 1) << 2;
            int rtp_header_size = sizeof(RTP_FIXED_HEADER) + rtp_extend_length + raw_offset;
            int symbol_size = 0;
            unsigned int rtp_pkt_size = (unsigned int)rtp_ext->rtp_pkt_size + raw_offset;
            if(rtp_ext->enable_fec)
            {
                FEC_HEADER *fec_ext = (FEC_HEADER *)&dataPtr[rtp_header_size - sizeof(FEC_HEADER)];
                extend_size += sizeof(FEC_HEADER);
                symbol_size = fec_ext->symbol_size << 2;
                //rtp_pkt_size = symbol_size + rtp_header_size;
            }
            if(rtp_extend_length != extend_size)
            {
                return -5;
            }

            if(rtp_pkt_size != size && rtp_header_size != size)
            {
                printf("api_isrtp: rtp_pkt_size=%d,raw_offset=%d \n", rtp_pkt_size, raw_offset);
                printf("api_isrtp: rtp_header_size=%d,size=%d \n", rtp_header_size, size);
                return -6;
            }

            ret = 1;
        }
    }
    return ret;
}
/*webrtc rtx packet
0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                         RTP Header                            |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |            OSN                |                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               |
   |                  Original RTP Packet Payload                  |
   |                                                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
HCSVC_API
int GetRtpInfo(uint8_t* dataPtr, int insize, RtpInfo *info, int type)
{
    int ret = 0;
    int isrtp = 0;
    if(insize < sizeof(RTP_FIXED_HEADER))
    {
        return -50;
    }
    if(info->raw_offset < 0)
    {
        info->raw_offset = 0;
    }
    else{
        info->raw_offset = RAW_OFFSET;//2;
    }


    RTP_FIXED_HEADER* rtp_hdr = (RTP_FIXED_HEADER*)&dataPtr[info->raw_offset];
    isrtp = (rtp_hdr->version == 2 &&  //版本号，此版本固定为2								V
            rtp_hdr->padding	== 0 &&  //														P
            rtp_hdr->csrc_len == 0 && //												X
            (rtp_hdr->payload == type) //负载类型号
            );
    if(!isrtp)
    {
        rtp_hdr = (RTP_FIXED_HEADER*)&dataPtr[0];
        isrtp = (rtp_hdr->version == 2 &&  //版本号，此版本固定为2								V
            rtp_hdr->padding	== 0 &&  //														P
            rtp_hdr->csrc_len == 0 && //												X
            (rtp_hdr->payload == type)  //负载类型号
            );
        if(isrtp)
        {
            info->raw_offset = 0;
        }
    }
    if(isrtp)
    {
        int extend_size = sizeof(EXTEND_HEADER);
        if(insize < (sizeof(RTP_FIXED_HEADER) + sizeof(EXTEND_HEADER)))
        {
            return -51;
        }
        int offset = sizeof(RTP_FIXED_HEADER);//1;
        info->seqnum = rtp_hdr->seq_no;
        info->ssrc = rtp_hdr->ssrc;
        info->timestamp = rtp_hdr->timestamp;
        if(rtp_hdr->payload == AAC_PLT)
        {
            AUDIO_EXTEND_HEADER *rtp_ext = NULL;
            if(rtp_hdr->extension)
            {
                extend_size = sizeof(AUDIO_EXTEND_HEADER);
                rtp_ext = (AUDIO_EXTEND_HEADER *)&dataPtr[sizeof(RTP_FIXED_HEADER)];
                int rtp_extend_profile = rtp_ext->rtp_extend_profile;
                if(rtp_extend_profile != EXTEND_PROFILE_ID)
                {
                    return -22;
                }
                int rtp_extend_length = rtp_ext->rtp_extend_length;
                rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
                //printf("IsRtpData: rtp_extend_length= %d \n", rtp_extend_length);
                rtp_extend_length = (rtp_extend_length + 1) << 2;
                if(rtp_extend_length != extend_size)
                {
                    return -23;
                }
                offset = sizeof(RTP_FIXED_HEADER) + rtp_extend_length;
                info->rtp_header_size = offset + info->raw_offset;
            }
            else{
                //暂不支持无扩展rtp
                return -24;
            }
            ret = 1;
            //printf("GetRtpInfo: rtp_ext->seq_no= %d \n", rtp_ext->seq_no);
            return ret;
        }
        EXTEND_HEADER *rtp_ext = NULL;
        if(rtp_hdr->extension)
        {
            rtp_ext = (EXTEND_HEADER *)&dataPtr[info->raw_offset + sizeof(RTP_FIXED_HEADER)];
            int rtp_extend_profile = rtp_ext->rtp_extend_profile;
            if(rtp_extend_profile != EXTEND_PROFILE_ID)
            {
                printf("GetRtpInfo: info->raw_offset=%d \n", info->raw_offset);
                return -3;
            }
            int rtp_extend_length = rtp_ext->rtp_extend_length;
            rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
            //printf("IsRtpData: rtp_extend_length= %d \n", rtp_extend_length);
            rtp_extend_length = (rtp_extend_length + 1) << 2;
            if(rtp_ext->enable_fec)
            {
                extend_size += sizeof(FEC_HEADER);
            }
            if(rtp_extend_length != extend_size)
            {
                printf("GetRtpInfo: info->raw_offset=%d \n", info->raw_offset);
                return -4;
            }
            offset = sizeof(RTP_FIXED_HEADER) + rtp_extend_length;
            info->rtp_header_size = offset + info->raw_offset;

            unsigned int rtp_pkt_size = rtp_ext->rtp_pkt_size;
            info->rtp_pkt_num = rtp_ext->rtp_pkt_num;
            info->max_pkt_num = rtp_ext->rtp_pkt_num;
            info->is_rtx = rtp_ext->is_rtx;
            info->start_seqnum = rtp_ext->start_seqnum;
            info->is_header = info->rtp_header_size == insize;
            if(info->rtp_header_size != insize)
            {
                if(rtp_extend_length > (sizeof(EXTEND_HEADER) + sizeof(FEC_HEADER)))
                {
                    //printf("GetRtpInfo: rtp_extend_length= %d \n", rtp_extend_length);
                    return -5;
                }
                //if(rtp_pkt_size > 1500 || rtp_pkt_size > insize)
                if((rtp_pkt_size > 1500) || ((rtp_pkt_size + info->raw_offset) != insize))
                {
                    if((insize - (rtp_pkt_size + info->raw_offset)) == 2)
                    {
                        //printf("GetRtpInfo: insize=%d, rtp_pkt_size=%d, info->raw_offset=%d \n", insize, rtp_pkt_size, info->raw_offset);
                        //printf("GetRtpInfo: info->seqnum=%d \n", info->seqnum);
                        //uint16_t *rtx_seqnum = (uint16_t *)&dataPtr[info->rtp_header_size];
                        //printf("GetRtpInfo: rtx_seqnum[0]=%d \n", rtx_seqnum[0]);
                        //info->rtx_seqnum = rtx_seqnum[0];
                    }
                    else{
                        return -6;
                    }
                }
                //else
                //{
                //    if(!rtp_ext->enable_fec && (rtp_pkt_size + info->raw_offset) != insize)
                //    {
                //        ///printf("GetRtpInfo: rtp_pkt_size= %d, insize=%d, info->rtp_header_size=%d \n", rtp_pkt_size, insize, info->rtp_header_size);
                //        return -7;
                //    }
                //}
            }
            //printf("IsRtpData: 2: rtp_extend_length= %d \n", rtp_extend_length);
            uint64_t time_stamp0 = rtp_ext->nack.nack0.time_info.time_stamp0;
	        uint64_t time_stamp1 = rtp_ext->nack.nack0.time_info.time_stamp1;
	        info->nack_time = time_stamp0 | (time_stamp1 << 32);
            //info->nack_time = rtp_ext->nack.nack0.time_info.time_stamp;
            //info->ref_idc = rtp_ext->ref_idc + 1;
            info->ref_idc = rtp_ext->ref_idc;
            info->spatial_num = rtp_ext->res_num;
            info->refs = rtp_ext->refs;
            info->ref_idx = rtp_ext->ref_idx;
            if(rtp_ext->res_num)
            {
                info->spatial_idx = rtp_ext->res_idx;
            }
            info->main_spatial_idx = rtp_ext->main_spatial_idx;
            info->enable_fec = rtp_ext->enable_fec;
            if(rtp_ext->enable_fec)
            {
                FEC_HEADER *fec_ext = (FEC_HEADER *)&dataPtr[info->raw_offset + offset - sizeof(FEC_HEADER)];
                int k = fec_ext->k;
                int n = fec_ext->n;
                int fec_seq_no = fec_ext->fec_seq_no;
                int symbol_size = fec_ext->symbol_size << 2;
                info->fec_k = k;
                info->fec_n = n;
                info->max_pkt_num = n;
                if((k > n || fec_seq_no > n) && (info->rtp_header_size != insize))
                {
                    //if(info->rtp_header_size != insize)
                    {
                        ///printf("GetRtpInfo: k=%d, n=%d, symbol_size=%d, insize=%d \n", k, n, symbol_size, insize);
                        //if((symbol_size + info->rtp_header_size) != insize)
                        //{
                        //    return -symbol_size - 200;
                        //    //return -8;
                        //}
                        //else
                        if(k > n)
                        {
                            return -9;
                        }
                        else if(fec_seq_no > n)
                        {
                            return -10;
                        }

                    }
                }
                //info->fec_seq_no = fec_seq_no;
		        //printf("IsRtpData: k= %d \n", k);
		        //printf("IsRtpData: n= %d \n", n);
		        //printf("IsRtpData: fec_seq_no= %d \n", fec_seq_no);
                info->is_fec = fec_seq_no >= k;
                if(fec_seq_no == (n - 1) || rtp_hdr->marker)
                {
                    info->nal_marker = 1;
                }
                else if(fec_seq_no == (k - 1))
                {
                    info->nal_marker = 2;
                }
            }
            else{
                info->nal_marker = rtp_hdr->marker;
            }
            //if(rtp_ext->first_slice && !info->is_fec)
            if(!info->is_fec)
            {
                if(info->nal_type == 5)
                {
                }
                else if(info->nal_type == 7)
                {
                    info->is_first_slice = 1;
                }
                else{
                    if(rtp_ext->first_slice)
                    {
                        info->is_first_slice = 1;
                    }
                }
            }
        }
        else{
            //暂不支持无扩展rtp
            printf("GetRtpInfo: info->raw_offset=%d \n", info->raw_offset);
            return -19;
            if(info->nal_type == 28)
            {
                FU_HEADER* fu_hdr = (FU_HEADER*)&dataPtr[info->raw_offset + offset + 1];
                ret = (fu_hdr->S == 1) ? 1 : 2;
                info->nal_marker = fu_hdr->E == 1;
                info->is_first_slice = (fu_hdr->S == 1);
            }
            else {
                if(info->nal_type == 24)
                {
                    ret = 1;
                    info->is_first_slice = 1;
                }
                else if(info->nal_type <= 23)
                {
                    if(info->nal_type == 5)
                    {
                    }
                    else if(info->nal_type == 7)
                    {
                        info->is_first_slice = 1;
                    }
                    else{
                        if(rtp_ext->first_slice)
                        {
                            info->is_first_slice = 1;
                        }
                    }
                    info->nal_marker = rtp_hdr->marker;
                }
                else {
                    ret = 2;
                }
            }
        }
        if(info->rtp_header_size != insize)
        {
            NALU_HEADER* nalu_hdr = (NALU_HEADER*)&dataPtr[info->raw_offset + offset + 0];
            if(!info->is_fec)
            {
                info->nal_type = nalu_hdr->TYPE;
                if(rtp_ext)
                {
                    info->nal_type = rtp_ext->nal_type;
                    if(rtp_ext->nal_type != nalu_hdr->TYPE)
                    {

                        NALU_HEADER* nalu_hdr2 = (NALU_HEADER*)&dataPtr[info->raw_offset + offset + 2];
                        if(rtp_ext->nal_type == nalu_hdr2->TYPE)
                        {
                            uint16_t *rtx_seqnum = (uint16_t *)&dataPtr[info->raw_offset + offset];
                            info->rtx_seqnum = rtx_seqnum[0];
                            printf("GetRtpInfo: rtx_seqnum[0]=%d \n", rtx_seqnum[0]);
                            printf("error: GetRtpInfo: info->seqnum=%d \n", info->seqnum);
                        }
                        else{
                            printf("error: GetRtpInfo: rtp_ext->nal_type= %d \n", rtp_ext->nal_type);
                            printf("error: GetRtpInfo: nalu_hdr->TYPE= %d \n", nalu_hdr->TYPE);
                            printf("error: GetRtpInfo: nalu_hdr2->TYPE= %d \n", nalu_hdr2->TYPE);
                        }
                    }
                }
            }
        }
        else{
            if(rtp_ext)
            {
                info->nal_type = rtp_ext->nal_type;
            }
        }
        ret = info->nal_marker ? 2 : 1;
        //printf("IsRtpData: nalu_hdr->TYPE= %d \n", nalu_hdr->TYPE);
    }
    else{
        printf("error: GetRtpInfo: ret=%d \n", ret);
    }
    return ret;
}
//if return 1 ,then discast it
//丢包率是动态变化的；
//主动丢包后，丢包率是持续改善的；
//需要有记忆，视丢包的改善情况来决定是否要加大主动丢包；
//需要甄别，主动丢包也无法改善的丢包

static void GetLossRateInit()
{
    if(!status0)
    {
        LossManager manager = {0, 0, 0, 0, 0, 0};

        for(int i = 0; i < MAX_SSRC_NUM; i++)
        {
            globLossManagers[i] = manager;
        }
        int size = sizeof(LossManager);
        //memset((void *)&globLossManagers[0], 0, MAX_SSRC_NUM * size);
        status0 = 1;
    }
}
static void *GetManager(unsigned int ssrc)
{
    unsigned short hight_value = ssrc >> 16;
    unsigned short low_value = ssrc &0xFFFF;
    unsigned short idx = hight_value + low_value;
    LossManager *ret = &globLossManagers[idx];
    if(ret->ssrc == 0)
    {
        ret->ssrc = ssrc;
        ret->time_stamp = get_sys_time();
        ret->idx += 1;
    }
    else if(ret->ssrc == ssrc)
    {
        //int64_t time_stamp = get_sys_time();
        //int diff_time = (int)(ret->time_stamp - time_stamp);
        //if(diff_time < 60 * 1000)
        //{
        //}
        ret->time_stamp = get_sys_time();
        ret->idx += 1;
    }
    else{
        //位置冲突
        printf("warning: GetManager: ssrc= %u, ret->ssrc= %u \n", ssrc, ret->ssrc);
        ret = NULL;
    }
    return (void *)ret;
}
static int GetLossRate(LossManager *manager, float lossRate)
{
    int ret = lossRate;
    int idx = manager->idx - 1;
    if(idx > 0)
    {
        ret = manager->sum_loss_rate;
        if(lossRate > 0)
        {
            if(manager->none_zero_cnt <= 3)
            {
                //继续增大主动丢包
                ret = manager->sum_loss_rate + lossRate;
                manager->sum_loss_rate = ret;
            }
            else{
                //无法通过主动丢包改善丢包率
                ret = manager->sum_loss_rate;
                if(lossRate > (manager->sum_loss_rate / 2))
                {
                    printf("warning: GetLossRate: manager->ssrc= %u \n", manager->ssrc);
                    printf("warning: GetLossRate: lossRate= %2.2f \n", lossRate);
                    printf("warning: GetLossRate: manager->sum_loss_rate= %2.2f \n", manager->sum_loss_rate);
                    printf("warning: GetLossRate: manager->none_zero_cnt= %d \n", manager->none_zero_cnt);
                }
            }
            manager->zero_cnt = 0;
            manager->none_zero_cnt += 1;
        }
        else{
            //记录稳定的次数
            manager->zero_cnt += 1;
            ret = manager->sum_loss_rate;
            if(manager->zero_cnt > 3)
            {
                //稳定后，按一定变化率，尝试减少主动丢包率
                float k = 0.8;
                manager->sum_loss_rate = manager->sum_loss_rate * k;
            }
        }

    }
    else{
        //初始值
        manager->sum_loss_rate = lossRate;
        manager->zero_cnt = 0;
        manager->none_zero_cnt = 1;
    }
    return ret;
}
#if 0
static int CheckPacket(uint8_t* dataPtr, int insize, unsigned int ssrc, float lossRate0)
{
    int ret = 0;
    RtpInfo info = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int is_hcsvcrtp = GetRtpInfo(dataPtr, insize, &info, H264_PLT);
    if((is_hcsvcrtp > 0) && (ssrc == info.ssrc))
    {
        LossManager *manager = (LossManager *)GetManager(ssrc);
        if(!manager)
        {
            return ret;
        }
        float lossRate = GetLossRate(manager, lossRate0);
        int refs = info.refs;
        if(refs > 1)
        {
            int ref_idx = info.ref_idx;
            //int gop_size = 50;
            //int fps = 25;
            //int bit_rate = 512 * 1024;
            //float k = (float)gop_size / (float)fps;
            //int bytes_target_per_gop = (int)((bit_rate >> 3) * k);

            if(refs == 16)
            {
                float k1 = 0.1;
                //int bytes_target_I = (int)(bytes_target_per_gop * k1);
                float k2 = 0.1;
                //int bytes_target_P = (int)(bytes_target_per_gop * k2);
                float k3 = (1.0 - (k1 + k2));
                //int bytes_target_FP = (int)(bytes_target_per_gop * k3);
                float k4 = 4.0;
                //int bytes_target_FP0 = (int)((float)bytes_target_FP * 3.0 / k4);
                //int bytes_target_FP1 = (int)((float)bytes_target_FP * 1.0 / k4);
                //int p_num = gop_size / refs;
                //int fp0_num = (gop_size / refs) * 6;//此系數會影響碼率的控制精度；
                //int fp1_num = gop_size - (1 + p_num + fp0_num);
                if (ref_idx == 0)
                {
                    //I: k1: 0.1
                    float loss_threshold0 = 1.0 - k1;//0.9
                    if(lossRate > loss_threshold0)
                    {
                        ret = 1;
                        //降分辨率
                    }
                }
                else if(ref_idx == refs)
                {
                    //P: k2: 0.1
                    float loss_threshold0 = 1.0 - k1;//0.9
                    float loss_threshold1 = 1.0 - (k1 + k2);//0.8
                    if(lossRate > loss_threshold0)
                    {
                        ret = 1;
                    }
                    else if(lossRate > loss_threshold1)
                    {
                    }
                }
                else if (((ref_idx & 1) == 0) || (ref_idx == (refs - 1)) || (ref_idx == 1))
                {
                    //FP1: k3/4: 0.2
                    //[1, 2,4,6,8,10,12,14,15]
                    float loss_threshold = (1.0 - (k1 + k2)) / 4;//0.2
                    if(lossRate > loss_threshold)
                    {
                        ret = 1;
                    }
                    else{
                        if(lossRate > loss_threshold / 2)
                        {
                            ret = 1;
                        }
                    }
                }
                else if ((ref_idx & 1) == 1)
                {
                    //FP0: 3*k3/4: 0.6
                    //[3,5,7,9,11,13]
                    float loss_threshold0 = 1.0 - (k1 + k2);//0.8
                    float loss_threshold1 = (1.0 - (k1 + k2)) / 4;//0.2
                    if(lossRate > loss_threshold0)
                    {
                        ret = 1;
                    }
                    else if(lossRate > loss_threshold1)
                    {
                        if(lossRate > loss_threshold1 * 3)
                        {
                            ret = 1;
                        }
                        else if(lossRate > loss_threshold1 * 2)
                        {
                            if(ref_idx == 5 || ref_idx == 9 || ref_idx == 13)
                            {
                                ret = 1;
                            }
                        }
                    }
                }
            }
        }
	    else{

	    }
    }
    return ret;
}
#endif

static void AddrManagerInit(int is_video)
{
    if(is_video)
    {
        if(!addr_status)
        {
            for(int i = 0; i < MAX_SSRC_NUM; i++)
            {
                int size = 16;
                globAddrManager[i].count = 0;
                globAddrManager[i].size = size;
                globAddrManager[i].hndList = calloc(1, size * sizeof(LossHandle));
            }
            addr_status = 1;
        }
    }
    else{
        if(!addr_status2)
        {
            for(int i = 0; i < MAX_SSRC_NUM; i++)
            {
                int size = 16;
                globAddrManager2[i].count = 0;
                globAddrManager2[i].size = size;
                globAddrManager2[i].hndList = calloc(1, size * sizeof(LossHandle));
            }
            addr_status2 = 1;
        }
    }

}

HCSVC_API
int api_get_rtp_info(uint8_t* dataPtr, int insize, RtpInfo *info, int type)
{
    return GetRtpInfo(dataPtr, insize, info, type);
}
#if 0
HCSVC_API
int api_check_packet(uint8_t* dataPtr, int insize, unsigned int ssrc, int lossRate)
{
    GetLossRateInit();
    return CheckPacket(dataPtr, insize, ssrc, lossRate);
}

HCSVC_API
int api_count_loss_rate(uint8_t* dataPtr, int insize, int freq)
{
    //printf("api_count_loss_rate \n");
    CountLossRateInit();
    return CountLossRate(dataPtr, insize, freq);
}
#endif

HCSVC_API
int api_get_pkt_delay(char *dataPtr, int insize)
{
    int ret = -((int)1 << 30);
    if(insize < sizeof(RTP_FIXED_HEADER))
    {
        return ret;
    }
    int raw_offset = RAW_OFFSET;//2;
    RTP_FIXED_HEADER* rtp_hdr = (RTP_FIXED_HEADER*)&dataPtr[raw_offset];
    int isrtp = (   rtp_hdr->version == 2 &&  //版本号，此版本固定为2								V
                    rtp_hdr->padding	== 0 &&  //														P
                    rtp_hdr->csrc_len == 0 && //												X
                    (rtp_hdr->payload == H264_PLT || rtp_hdr->payload == AAC_PLT)   //负载类型号
                );
    if(!isrtp)
    {
        rtp_hdr = (RTP_FIXED_HEADER*)&dataPtr[0];
        isrtp = (   rtp_hdr->version == 2 &&  //版本号，此版本固定为2								V
                    rtp_hdr->padding	== 0 &&  //														P
                    rtp_hdr->csrc_len == 0 && //												X
                    (rtp_hdr->payload == H264_PLT || rtp_hdr->payload == AAC_PLT)   //负载类型号
                    );
        if(isrtp)
        {
            raw_offset = 0;
        }
    }
    if(isrtp)
    {
        if(insize < (sizeof(RTP_FIXED_HEADER) + sizeof(EXTEND_HEADER)))
        {
            return ret;
        }
        if(rtp_hdr->payload == AAC_PLT)
        {
            AUDIO_EXTEND_HEADER *rtp_ext = (AUDIO_EXTEND_HEADER *)&dataPtr[sizeof(RTP_FIXED_HEADER)];
            uint64_t time_stamp0 = rtp_ext->nack.nack0.time_info.time_stamp0;
	        uint64_t time_stamp1 = rtp_ext->nack.nack0.time_info.time_stamp1;
	        int64_t time0 = time_stamp0 | (time_stamp1 << 32);
            //int64_t time0 = rtp_ext->nack.nack0.time_info.time_stamp;
            int64_t time1 = get_sys_time();
            int difftime = (int)(time1 - time0);
            if(time0 > time1)
            {
                difftime = (int)(time0 - time1);
            }
            ret = difftime;
            return ret;
        }
        EXTEND_HEADER *rtp_ext = NULL;
        if(rtp_hdr->extension)
        {
            rtp_ext = (EXTEND_HEADER *)&dataPtr[raw_offset + sizeof(RTP_FIXED_HEADER)];
            int rtp_extend_length = rtp_ext->rtp_extend_length;
            rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
            rtp_extend_length = (rtp_extend_length + 1) << 2;
            int offset = sizeof(RTP_FIXED_HEADER) + rtp_extend_length;
            int rtp_header_size = offset + raw_offset;

            if(rtp_header_size != insize)
            {
                unsigned int rtp_pkt_size = rtp_ext->rtp_pkt_size;
                if(rtp_extend_length > (sizeof(EXTEND_HEADER) + sizeof(FEC_HEADER)))
                {
                    //printf("GetRtpInfo: rtp_extend_length= %d \n", rtp_extend_length);
                    return 0;
                }
                if(rtp_pkt_size > 1500 || rtp_pkt_size > insize)
                {
                    printf("api_get_pkt_delay: rtp_pkt_size= %d \n", rtp_pkt_size);
                    return 0;
                }
                else
                {
                    if(!rtp_ext->enable_fec && (rtp_pkt_size + raw_offset) != insize)
                    {
                        printf("api_get_pkt_delay: rtp_pkt_size= %d, insize=%d \n", rtp_pkt_size, insize);
                        return 0;
                    }
                }
            }
            uint64_t time_stamp0 = rtp_ext->nack.nack0.time_info.time_stamp0;
	        uint64_t time_stamp1 = rtp_ext->nack.nack0.time_info.time_stamp1;
	        int64_t time0 = time_stamp0 | (time_stamp1 << 32);
            //int64_t time0 = rtp_ext->nack.nack0.time_info.time_stamp;
            int64_t time1 = get_sys_time();
            int difftime = (int)(time1 - time0);
            if(time0 > time1)
            {
                difftime = (int)(time0 - time1);
            }
            ret = difftime;
        }
    }
    return ret;
}
HCSVC_API
int api_get_pkt_delay2(char *dataPtr, int insize, int raw_offset, int time_offset, int type)
{
    int ret = -1;
    RTP_FIXED_HEADER* rtp_hdr = (RTP_FIXED_HEADER*)&dataPtr[raw_offset];
    if(rtp_hdr->extension)
    {
        int64_t time0 = 0;
        int time_offset0 = 0;
        if(type == AAC_PLT)
        {
            AUDIO_EXTEND_HEADER *rtp_ext = (AUDIO_EXTEND_HEADER *)&dataPtr[raw_offset + sizeof(RTP_FIXED_HEADER)];
            uint64_t time_stamp0 = rtp_ext->nack.nack0.time_info.time_stamp0;
	        uint64_t time_stamp1 = rtp_ext->nack.nack0.time_info.time_stamp1;
	        time0 = time_stamp0 | (time_stamp1 << 32);
            //time0 = rtp_ext->nack.nack0.time_info.time_stamp;
            time_offset0 = rtp_ext->nack.nack0.time_offset;
            time_offset0 <<= (32 - 13);
			time_offset0 >>= (32 - 13);
        }
        else{
            EXTEND_HEADER *rtp_ext = (EXTEND_HEADER *)&dataPtr[raw_offset + sizeof(RTP_FIXED_HEADER)];
            uint64_t time_stamp0 = rtp_ext->nack.nack0.time_info.time_stamp0;
	        uint64_t time_stamp1 = rtp_ext->nack.nack0.time_info.time_stamp1;
	        time0 = time_stamp0 | (time_stamp1 << 32);
            //time0 = rtp_ext->nack.nack0.time_info.time_stamp;
            time_offset0 = rtp_ext->nack.nack0.time_offset;
            time_offset0 <<= (32 - 13);
		    time_offset0 >>= (32 - 13);
        }
        int64_t time1 = get_sys_time();
        //int difftime0 = (int)(time1 - time0);
        //printf("api_get_pkt_delay2: difftime0=%d \n", difftime0);
        time0 = time0 - time_offset0;//server time
        time1 = time1 - time_offset;//server time
        int difftime = (int)(time1 - time0);
        //if(time0 > time1)
        //{
        //    difftime = (int)(time0 - time1);
        //}
        if(difftime > 500)
        {
            printf("api_get_pkt_delay2: time_offset=%d,time_offset0=%d, type=%d \n", time_offset, time_offset0, type);
        }
        ret = difftime;
        return ret;
    }
    return ret;
}
HCSVC_API
int api_get_ref_idc(char *dataPtr, int insize, int raw_offset)
{
    int ret = -1;
    RTP_FIXED_HEADER* rtp_hdr = (RTP_FIXED_HEADER*)&dataPtr[raw_offset];
    if(rtp_hdr->extension)
    {
        EXTEND_HEADER *rtp_ext = (EXTEND_HEADER *)&dataPtr[raw_offset + sizeof(RTP_FIXED_HEADER)];
        ret = rtp_ext->ref_idc;
    }
    return ret;
}
#if 1
void netinfo_init(LossRateInfo *loss_rate_info)
{
    loss_rate_info[0].loss_rate = -1;
    loss_rate_info[0].max_loss_rate = 0;
    loss_rate_info[0].last_max_loss_rate = 0;
    loss_rate_info[0].sum_loss_rate = 0;
    loss_rate_info[0].cnt_loss_rate = 0;
    loss_rate_info[0].cnt_max = 1;
    loss_rate_info[0].now_time = 0;
    loss_rate_info[0].time_len = 40;
    loss_rate_info[1].loss_rate = -1;
    loss_rate_info[1].max_loss_rate = 0;
    loss_rate_info[1].last_max_loss_rate = 0;
    loss_rate_info[1].sum_loss_rate = 0;
    loss_rate_info[1].cnt_loss_rate = 0;
    loss_rate_info[1].cnt_max = 10;
    loss_rate_info[1].now_time = 0;
    loss_rate_info[1].time_len = 1000;
    loss_rate_info[2].loss_rate = -1;
    loss_rate_info[2].max_loss_rate = 0;
    loss_rate_info[2].last_max_loss_rate = 0;
    loss_rate_info[2].sum_loss_rate = 0;
    loss_rate_info[2].cnt_loss_rate = 0;
    loss_rate_info[2].cnt_max = 100;//1000;//100;//统计最大丢包率发生的周期// > 40*100 = 4s
    loss_rate_info[2].now_time = 0;
    loss_rate_info[2].time_len = 4000;
    //
    loss_rate_info[3].loss_rate = -1;
    loss_rate_info[3].max_loss_rate = 0;
    loss_rate_info[3].last_max_loss_rate = 0;
    loss_rate_info[3].sum_loss_rate = 0;
    loss_rate_info[3].cnt_loss_rate = 0;
    loss_rate_info[3].cnt_max = 10000;//1000;//100;//统计最大丢包率发生的周期// > 40*100 = 4s
    loss_rate_info[3].now_time = 0;
    loss_rate_info[3].time_len = 30 * 1000;
}
void PacketManagerInit(PacketManager *obj, unsigned int ssrc, RtpInfo *info, int freq)
{
    unsigned short hight_value = ssrc >> 16;
    unsigned short low_value = ssrc & 0xFFFF;
    unsigned short idx = hight_value + low_value;
    //printf("GetPacketManager: idx=%d \n", idx);
    if(obj)
    {
        obj->logfp = NULL;
        if(!obj->logfp && loglevel)
        {
            char filename[256] = "loss_rate_gxh_";
            char ctmp[32] = "";
            int fileidx = ssrc;//random() % 10000;
            sprintf(ctmp, "%d", freq);
            strcat(filename, ctmp);
            strcat(filename, "_");
            sprintf(ctmp, "%d", fileidx);
            strcat(filename, ctmp);
            strcat(filename, ".txt");
            obj->logfp = fopen(filename, "w");
        }
        for(int i = 0; i < MAX_PKT_BUF_SIZE; i++)
        {
            obj->info[i].status = 0;
            obj->info[i].seqnum = -1;
            obj->info[i].timestamp = -1;
        }
        obj->ssrc = ssrc;
        obj->layerId = info->spatial_idx;
        obj->time_stamp = get_sys_time();
        obj->min_seqnum = info->seqnum + LEFT_SHIFT16;
        obj->max_seqnum = info->seqnum + LEFT_SHIFT16;
        obj->old_seqnum = -1;
        obj->status = 0;
        obj->max_loss_series_num = 0;
        obj->max_fec_n = 0;
        //
        int I0 = obj->min_seqnum % MAX_PKT_BUF_SIZE;
        obj->info[I0].status = 1;
        obj->info[I0].seqnum = info->seqnum + LEFT_SHIFT16;
        obj->info[I0].timestamp = info->timestamp + LEFT_SHIFT32;
#if 0
        obj->loss_rate_info[0].loss_rate = -1;
        obj->loss_rate_info[0].max_loss_rate = 0;
        obj->loss_rate_info[0].last_max_loss_rate = 0;
        obj->loss_rate_info[0].sum_loss_rate = 0;
        obj->loss_rate_info[0].cnt_loss_rate = 0;
        obj->loss_rate_info[0].cnt_max = 1;
        obj->loss_rate_info[1].loss_rate = -1;
        obj->loss_rate_info[1].max_loss_rate = 0;
        obj->loss_rate_info[1].last_max_loss_rate = 0;
        obj->loss_rate_info[1].sum_loss_rate = 0;
        obj->loss_rate_info[1].cnt_loss_rate = 0;
        obj->loss_rate_info[1].cnt_max = 10;
        obj->loss_rate_info[2].loss_rate = -1;
        obj->loss_rate_info[2].max_loss_rate = 0;
        obj->loss_rate_info[2].last_max_loss_rate = 0;
        obj->loss_rate_info[2].sum_loss_rate = 0;
        obj->loss_rate_info[2].cnt_loss_rate = 0;
        obj->loss_rate_info[2].cnt_max = 100;
#else
        netinfo_init(obj->loss_rate_info);
#endif
        //obj->idx = 1;
    }
    return;
}
//当缓存足够大时（大于(1<<16)），包序比较需要结合时间戳进行；
//
//时间戳跨度足够大
//注意：时间戳取绝对时间时，存在从一开始就处于临界值得情况
//本代码支持时间戳以90khz，从0开始递增的情况，也支持一开始就处于临界值的情况
//多流切换，会出现“起始处于临界值的情况”
//解决“起始处于临界值的情况”的方法：给初始值获得一个跨度单位
int64_t correct_time_stamp(PacketManager *obj, int64_t time_stamp0, int64_t time_stamp)
{
    //此时的time_stamp未经校正
    int64_t ret = time_stamp;
    //int n = (time_stamp0 / LEFT_SHIFT32) + 1;
    int64_t n = (time_stamp0 >> 32);
    int64_t time_stamp1 = (time_stamp0 % LEFT_SHIFT32);//获得原始时间戳

    if(time_stamp1 > HALF_QUART_UINT && time_stamp < QUART_UINT)
    {
        //向最小差距进行补偿
        ret = time_stamp + (n + 1) * LEFT_SHIFT32;
        //printf("correct_time_stamp: 0 \n");
    }
    else if(time_stamp > HALF_QUART_UINT && time_stamp1 < QUART_UINT)
    {
        ret = time_stamp + (n - 1) * LEFT_SHIFT32;
        //printf("correct_time_stamp: 1 \n");
        if(ret < 0)
        {
            printf("error: correct_time_stamp: time_stamp0=%lld \n", time_stamp0);
            printf("error: correct_time_stamp: time_stamp=%lld \n", time_stamp);

            if (obj->logfp && loglevel)
            {
                fprintf(obj->logfp, "correct_time_stamp: time_stamp0=%lld \n", time_stamp0);
                fprintf(obj->logfp, "correct_time_stamp: time_stamp=%lld \n", time_stamp);
                fprintf(obj->logfp, "correct_time_stamp: time_stamp1=%lld \n", time_stamp1);
                fprintf(obj->logfp, "correct_time_stamp: ret=%lld \n", ret);
                fflush(obj->logfp);
            }
        }
    }
    else{
        ret = time_stamp + n * LEFT_SHIFT32;
        //printf("correct_time_stamp: 2 \n");
    }
    return ret;
}
//由于包序跨度不够大(乱序度可能会超过包序跨度)，通过时间戳比较，辅助校正
//当前包序与最大包序比较，减少跨度溢出的几率
//乱序度超过多个包序跨度的情况需另行考虑
//假定平均每帧1024个包，帧率为25fps，则（1<<16）的包序最大容忍乱序度为64*40ms=2.56s
//本代码支持包序从0开始递增的情况，也支持起始处于临界值的情况
//多流切换，会出现“起始处于临界值的情况”
int64_t correct_seq_num(PacketManager *obj, int64_t seq_num0, int64_t seq_num, int64_t time_stamp0, int64_t time_stamp, int freq)
{
    //此时的time_stamp已经校正
    int64_t ret = seq_num;
    //int n = (seq_num0 / LEFT_SHIFT16) + 1;
    int64_t n = (seq_num0 >> 16);
    int64_t seq_num1 = (seq_num0 % LEFT_SHIFT16);
    //
    int diff_time = (int)((time_stamp - time_stamp0) / freq);
    if(time_stamp < time_stamp0)
    {
        diff_time = (int)((time_stamp0 - time_stamp) / freq);
    }
    if(diff_time > 2560)
    {
        printf("warnning: correct_seq_num: seq_num0=%lld \n", seq_num0);
        printf("warnning: correct_seq_num: seq_num=%lld \n", seq_num);
        printf("warnning: correct_seq_num: time_stamp0=%lld \n", time_stamp0);
        printf("warnning: correct_seq_num: time_stamp=%lld \n", time_stamp);
        printf("warnning: correct_seq_num: diff_time=%d \n", diff_time);
        //ret = -11;//test
        //return ret;//test

        if (obj->logfp && loglevel)
        {
            fprintf(obj->logfp, "correct_seq_num: seq_num0=%lld \n", seq_num0);
            fprintf(obj->logfp, "correct_seq_num: seq_num=%lld \n", seq_num);
            fprintf(obj->logfp, "correct_seq_num: time_stamp0=%lld \n", time_stamp0);
            fprintf(obj->logfp, "correct_seq_num: time_stamp=%lld \n", time_stamp);
            fprintf(obj->logfp, "correct_seq_num: diff_time=%d \n", diff_time);
            fflush(obj->logfp);
        }
    }
    //
    if(seq_num1 > HALF_QUART_USHORT && seq_num < QUART_USHORT)
    {
        //向最小差距进行补偿
        ret = seq_num + (n + 1) * LEFT_SHIFT16;//包序增大，时间戳应该也是增大的time_stamp > time_stamp0
        if(time_stamp0 > time_stamp)
        {
            //异常值
            printf("error: correct_seq_num: seq_num0=%lld \n", seq_num0);
            printf("error: correct_seq_num: seq_num=%lld \n", seq_num);
            printf("error: correct_seq_num: time_stamp0=%lld \n", time_stamp0);
            printf("error: correct_seq_num: time_stamp=%lld \n", time_stamp);
            ret = -2;
            if (obj->logfp && loglevel)
            {
                fprintf(obj->logfp, "correct_seq_num: seq_num0=%lld \n", seq_num0);
                fprintf(obj->logfp, "correct_seq_num: seq_num=%lld \n", seq_num);
                fprintf(obj->logfp, "correct_seq_num: time_stamp0=%lld \n", time_stamp0);
                fprintf(obj->logfp, "correct_seq_num: time_stamp=%lld \n", time_stamp);
                fprintf(obj->logfp, "correct_seq_num: ret=%d \n", ret);
                fflush(obj->logfp);
            }
        }
        //printf("correct_seq_num: 0 \n");
    }
    else if(seq_num > HALF_QUART_USHORT && seq_num1 < QUART_USHORT)
    {
        ret = seq_num + (n - 1) * LEFT_SHIFT16;//包序减小time_stamp < time_stamp0
        if(time_stamp0 < time_stamp)
        {
            //异常值
            printf("error: correct_seq_num: seq_num0=%lld \n", seq_num0);
            printf("error: correct_seq_num: seq_num=%lld \n", seq_num);
            printf("error: correct_seq_num: time_stamp0=%lld \n", time_stamp0);
            printf("error: correct_seq_num: time_stamp=%lld \n", time_stamp);
            ret = -3;
            if (obj->logfp && loglevel)
            {
                fprintf(obj->logfp, "correct_seq_num: seq_num0=%lld \n", seq_num0);
                fprintf(obj->logfp, "correct_seq_num: seq_num=%lld \n", seq_num);
                fprintf(obj->logfp, "correct_seq_num: time_stamp0=%lld \n", time_stamp0);
                fprintf(obj->logfp, "correct_seq_num: time_stamp=%lld \n", time_stamp);
                fprintf(obj->logfp, "correct_seq_num: ret=%d \n", ret);
                fflush(obj->logfp);
            }
        }
        //printf("correct_seq_num: 1 \n");
    }
    else{
        ret = seq_num + n * LEFT_SHIFT16;
        //printf("correct_seq_num: 2 \n");
    }
    return ret;
}


int CountLossRate2(PacketManager *manager, uint8_t* dataPtr, int insize, int freq)
{
    int ret = -1;
    //printf("CountLossRate: insize=%d \n", insize);
    RtpInfo rtpInfo = {0};
    rtpInfo.raw_offset = -1;
    int type = freq == 90 ? H264_PLT : AAC_PLT;
    int is_hcsvcrtp = GetRtpInfo(dataPtr, insize, &rtpInfo, type);
    if(is_hcsvcrtp > 0)
    {
        unsigned int ssrc = rtpInfo.ssrc;
        int layerId = rtpInfo.spatial_idx;

        //if(rtpInfo.is_fec)
        //{
        //    printf("CountLossRate2: %d,%d,%d,%d, seqnum=%d \n", rtpInfo.fec_k, rtpInfo.fec_n, rtpInfo.is_fec, layerId, rtpInfo.seqnum);
        //}

        //注意：多流需要共用一个ssrc
        if(ssrc != manager->ssrc)
        {
            if (manager->logfp && loglevel)
            {
                fprintf(manager->logfp, "CountLossRate2: ssrc=%lld \n", ssrc);
                fprintf(manager->logfp, "CountLossRate2: manager->ssrc=%lld \n", manager->ssrc);
                fprintf(manager->logfp, "CountLossRate2: layerId=%d \n", layerId);
                fprintf(manager->logfp, "CountLossRate2: manager->layerId=%d \n", manager->layerId);
                fprintf(manager->logfp, "CountLossRate2: rtpInfo.rtp_header_size=%d \n", rtpInfo.rtp_header_size);
                fprintf(manager->logfp, "CountLossRate2: rtpInfo.raw_offset=%d \n", rtpInfo.raw_offset);

                fprintf(manager->logfp, "CountLossRate2: insize=%d \n", insize);
                fprintf(manager->logfp, "CountLossRate2: freq=%d \n", freq);
                fflush(manager->logfp);
            }
#if 1
            PacketManagerInit(manager, ssrc, &rtpInfo, freq);
#endif
            printf("CountLossRate2: manager->ssrc=%lld \n", manager->ssrc);
            printf("CountLossRate2: ssrc=%lld \n", ssrc);
            return -200;
        }
#if 1
        if(layerId != manager->layerId)
        {
            for(int64_t i = manager->min_seqnum; i < manager->max_seqnum; i++)
            {
                RtpPacketInfo *info = &manager->info[i % MAX_PKT_BUF_SIZE];
                info->status = 0;
                info->timestamp = -1;
                info->seqnum = -1;
            }
            manager->layerId = layerId;
            manager->time_stamp = get_sys_time();
            manager->min_seqnum = rtpInfo.seqnum + LEFT_SHIFT16;
            manager->max_seqnum = rtpInfo.seqnum + LEFT_SHIFT16;
            manager->old_seqnum = -1;
            manager->status = 0;
            int I0 = rtpInfo.seqnum % MAX_PKT_BUF_SIZE;
            manager->info[I0].status = 1;
            manager->info[I0].seqnum = rtpInfo.seqnum + LEFT_SHIFT16;
            manager->info[I0].timestamp = rtpInfo.timestamp + LEFT_SHIFT32;
            return -2;
        }
#endif
        int64_t seqnum = rtpInfo.seqnum;
        int64_t timestamp = rtpInfo.timestamp;
        if(timestamp > HALF_QUART_UINT)
        {
            if (manager->logfp && loglevel)
            {
                fprintf(manager->logfp, "CountLossRate2: ssrc=%lld \n", ssrc);
                fprintf(manager->logfp, "CountLossRate2: seqnum=%lld \n", seqnum);
                fprintf(manager->logfp, "CountLossRate2: timestamp=%lld \n", timestamp);
                fprintf(manager->logfp, "CountLossRate2: insize=%d \n", insize);
                fflush(manager->logfp);
            }
        }
        int idx = seqnum % MAX_PKT_BUF_SIZE;
        //
        int I1 = manager->max_seqnum % MAX_PKT_BUF_SIZE;
        RtpPacketInfo *info1 = &manager->info[I1];
        if(!info1->status)
        {
            printf("CountLossRate: manager->max_seqnum=%lld \n", manager->max_seqnum);
            return -3;
        }
        if(rtpInfo.fec_n > manager->max_fec_n)
        {
            manager->max_fec_n = rtpInfo.fec_n;
        }
        timestamp = correct_time_stamp(manager, info1->timestamp, timestamp);
        seqnum = correct_seq_num(manager, info1->seqnum, seqnum, info1->timestamp, timestamp, freq);
        //return ret;
        //
        int is_old_packet = 0;
        int64_t old_seqnum = manager->old_seqnum;
        if(old_seqnum >= 0)
        {
            //is_old_packet =  (seqnum <= old_seqnum) && ((old_seqnum - seqnum) < HALF_USHORT);
            //is_old_packet |= (seqnum > old_seqnum) && ((seqnum - old_seqnum) > HALF_USHORT);
            is_old_packet = (seqnum <= old_seqnum);
        }
        if(seqnum < 0 && manager->status)
        {
            if (manager->logfp && loglevel)
            {
                fprintf(manager->logfp, "CountLossRate2: manager->status=%d \n", manager->status);
                fprintf(manager->logfp, "CountLossRate2: seqnum=%d \n", seqnum);
                fprintf(manager->logfp, "CountLossRate2: old_seqnum=%d \n", old_seqnum);
                fprintf(manager->logfp, "CountLossRate2: ssrc=%u \n", ssrc);
                fprintf(manager->logfp, "CountLossRate2: manager->ssrc=%u \n", manager->ssrc);
                fprintf(manager->logfp, "CountLossRate2: insize=%d \n", insize);
                fflush(manager->logfp);
            }

        }
        if((old_seqnum < 0 || !is_old_packet) && seqnum >= 0)
        {
            int64_t min_seqnum = manager->min_seqnum;
            int64_t max_seqnum = manager->max_seqnum;
            //===
            if((min_seqnum < 0) && (max_seqnum < 0))
            {
                manager->min_seqnum = seqnum;
                manager->max_seqnum = seqnum;
            }
            else if(min_seqnum == max_seqnum)
            {
                int I0 = min_seqnum % MAX_PKT_BUF_SIZE;
                RtpPacketInfo *info0 = &manager->info[I0];
                if(!info0->status)
                {
                    manager->min_seqnum = seqnum;
                    manager->max_seqnum = seqnum;
                }
                else{
                    if(seqnum < min_seqnum)
                    {
                        manager->min_seqnum = seqnum;
                    }
                    else{
                        manager->max_seqnum = seqnum;
                    }
                }
            }
            else{
                if(seqnum < min_seqnum)
                {
                    manager->min_seqnum = seqnum;
                }
                else if(seqnum > max_seqnum)
                {
                    manager->max_seqnum = seqnum;
                }
            }
            int packet_distance = manager->max_seqnum - manager->min_seqnum;
            if(packet_distance > 10240)
            {
            }
            int idx2 = seqnum % MAX_PKT_BUF_SIZE;
            if(idx != idx2)
            {
                printf("CountLossRate: idx=%d, idx2=%d \n", idx, idx2);
                if (manager->logfp && loglevel)
                {
                    fprintf(manager->logfp, "CountLossRate2: idx=%d \n", idx);
                    fprintf(manager->logfp, "CountLossRate2: idx2=%d \n", idx2);
                    fflush(manager->logfp);
                }
            }
            manager->info[idx].status = 1;
            manager->info[idx].seqnum = seqnum;
            manager->info[idx].timestamp = timestamp;
        }
        //
        int64_t time_stamp = get_sys_time();
        int diff_time = (int)(time_stamp - manager->time_stamp);
        //printf("CountLossRate2: diff_time=%d \n", diff_time);
        if(diff_time > PKT_LOSS_INTERVAL)
        {
            //printf("CountLossRate: diff_time=%d \n", diff_time);
            int I0 = manager->min_seqnum % MAX_PKT_BUF_SIZE;
            int I1 = manager->max_seqnum % MAX_PKT_BUF_SIZE;
            RtpPacketInfo *info0 = &manager->info[I0];
            RtpPacketInfo *info1 = &manager->info[I1];
            int64_t timestamp0 = info0->timestamp;
            int64_t timestamp1 = info1->timestamp;
            int delay = (int)(timestamp1 - timestamp0) / freq;//90;//90000Hz
            //int delay = (int)((timestamp1 - timestamp0) / freq);

            //if(timestamp0 > timestamp1)
            //{
            //    delay = (int)((timestamp1 + LEFT_SHIFT32 - timestamp0) / freq);//90);
            //}
            int delay0 = delay;
            //printf("CountLossRate2: manager->min_seqnum=%d \n", manager->min_seqnum);
            //printf("CountLossRate2: manager->max_seqnum=%d \n", manager->max_seqnum);
            //printf("CountLossRate2: timestamp0=%lld \n", timestamp0);
            //printf("CountLossRate2: timestamp1=%lld \n", timestamp1);
            //printf("CountLossRate2: delay=%d \n", delay);
            if(delay > (PKT_LOSS_INTERVAL + MAX_PKT_DELAY))
            {
                //printf("CountLossRate: delay=%d \n", delay);
                int start = manager->min_seqnum;
                int end = manager->max_seqnum;
                //if(start > end)
                //{
                //    end += LEFT_SHIFT16;//MAX_USHORT;
                //}
                int64_t last_seq_num = -1;
                int loss_num = 0;
                int pkt_num = 0;
                int64_t i = 0;
                int j = 0;
                //int max_loss_series_num = 0;
                int loss_series_num = 0;
                for(i = start; i <= end; i++)
                {
                    RtpPacketInfo *info = &manager->info[i % MAX_PKT_BUF_SIZE];
                    int64_t this_timestamp = info->timestamp;
                    int64_t this_seqnum = info->seqnum;
                    //if(this_timestamp >= 0 && this_seqnum >= 0)
                    if(info->status)
                    {
                        ret = 0;
                        delay = (int)((this_timestamp - timestamp0) / freq);//90;//90000Hz
                        //if(timestamp0 > timestamp1)//???
                        //if(timestamp0 > this_timestamp)
                        //{
                        //    delay = (int)((this_timestamp + LEFT_SHIFT32 - timestamp0) / freq);//90);
                        //}
                        //if(delay >= PKT_LOSS_INTERVAL)
                        if((delay >= (delay0 >> 1)))
                        {
                            if(!pkt_num)
                            {
                                pkt_num = (int)(i - start);
                                printf("error: CountLossRate2: j= %d, i= %lld \n", j, i);
                                printf("error: CountLossRate2: loss_num= %d \n", loss_num);
                                printf("error: CountLossRate2: start= %d, end= %d \n", start, end);
                                ret = 101;
                            }
                            else{
                                ret = (int)(100 * ((float)loss_num / pkt_num));
                            }

                            if(loss_num != j)
                            {
                                //printf("warning: CountLossRate2: j= %d, loss_num= %d \n", j, loss_num);
                                if(j > loss_num)
                                {
                                    pkt_num += (j - loss_num);
                                    ret = (int)(100 * ((float)j / pkt_num));
                                }
                            }
                            manager->time_stamp = get_sys_time();
                            break;
                        }
                        if(last_seq_num >= 0)
                        {
                            int diff_seqnum = (int)(this_seqnum - last_seq_num);
                            //if(this_seqnum < last_seq_num)
                            //{
                            //    diff_seqnum = (this_seqnum + LEFT_SHIFT16) - last_seq_num;
                            //}
                            loss_num += (diff_seqnum - 1);
                        }
                        loss_series_num = 0;
                        pkt_num = i - start;
                        last_seq_num = this_seqnum;
                    }
                    else{
                        j++;
                        loss_series_num++;
                        if(loss_series_num > manager->max_loss_series_num)
                        {
                            manager->max_loss_series_num = loss_series_num;
                        }
                    }
                }
                if(ret >= 0)
                {
                    //clear
                    int64_t stop_seqnum = i;
                    //if(i < 0)
                    //{
                    //    if (manager->logfp && loglevel)
                    //    {
                    //        fprintf(manager->logfp, "CountLossRate2: i=%d \n", i);
                    //        fflush(manager->logfp);
                    //    }
                    //}
                    for(int64_t i = start; i < stop_seqnum; i++)
                    {
                        RtpPacketInfo *info = &manager->info[i % MAX_PKT_BUF_SIZE];
                        info->status = 0;
                        info->timestamp = -1;
                        info->seqnum = -1;
                    }
                    manager->min_seqnum = stop_seqnum;// % LEFT_SHIFT16;
                    manager->old_seqnum = manager->min_seqnum;
                    manager->status = 1;
                    old_seqnum = manager->old_seqnum;
                    //if (manager->logfp && loglevel)
                    //{
                    //    fprintf(manager->logfp, "CountLossRate2: manager->min_seqnum=%lld \n", manager->min_seqnum);
                    //    fprintf(manager->logfp, "CountLossRate2: manager->status=%d \n", manager->status);
                    //    fflush(manager->logfp);
                    //}

                    //printf("CountLossRate2: start=%d \n", start);
                    //printf("CountLossRate2: stop_seqnum=%d \n", stop_seqnum);
                }
                if(delay0 > 2000 || diff_time > 2000 || delay < PKT_LOSS_INTERVAL)
                {
                    printf("CountLossRate2: diff_time= %d \n", diff_time);
                    printf("CountLossRate2: delay= %d \n", delay);
                    printf("CountLossRate2: delay0= %d \n", delay0);
                    printf("CountLossRate2: ret=%d \n", ret);
                }
            }
            else{
                if(diff_time > 2000 || delay < 0)
                {
                    printf("CountLossRate2: manager->min_seqnum=%lld \n", manager->min_seqnum);
                    printf("CountLossRate2: manager->max_seqnum=%lld \n", manager->max_seqnum);
                    printf("CountLossRate2: timestamp0=%lld \n", timestamp0);
                    printf("CountLossRate2: timestamp1=%lld \n", timestamp1);
                    printf("CountLossRate2: delay=%d \n", delay);
                    printf("CountLossRate2: diff_time=%d \n", diff_time);

                    if (manager->logfp && loglevel)
                    {
                        fprintf(manager->logfp, "CountLossRate2: manager->min_seqnum=%lld \n", manager->min_seqnum);
                        fprintf(manager->logfp, "CountLossRate2: manager->max_seqnum=%lld \n", manager->max_seqnum);
                        fprintf(manager->logfp, "CountLossRate2: timestamp0=%lld \n", timestamp0);
                        fprintf(manager->logfp, "CountLossRate2: timestamp1=%lld \n", timestamp1);
                        fprintf(manager->logfp, "CountLossRate2: seqnum=%lld \n", seqnum);
                        fprintf(manager->logfp, "CountLossRate2: old_seqnum=%d \n", old_seqnum);
                        fprintf(manager->logfp, "CountLossRate2: delay=%d \n", delay);
                        fflush(manager->logfp);
                    }
                }
            }
        }
        //if (manager->logfp && loglevel)
        //{
        //    fprintf(manager->logfp, "CountLossRate2: manager->status=%d \n", manager->status);
        //    fprintf(manager->logfp, "CountLossRate2: old_seqnum=%d \n", old_seqnum);
        //    fflush(manager->logfp);
        //}
        if(old_seqnum < 0 && manager->status)
        {
            if (manager->logfp && loglevel)
            {
                fprintf(manager->logfp, "CountLossRate2: manager->status=%d \n", manager->status);
                fprintf(manager->logfp, "CountLossRate2: manager->min_seqnum=%lld \n", manager->min_seqnum);
                fprintf(manager->logfp, "CountLossRate2: manager->max_seqnum=%lld \n", manager->max_seqnum);
                fprintf(manager->logfp, "CountLossRate2: seqnum=%lld \n", seqnum);
                fprintf(manager->logfp, "CountLossRate2: old_seqnum=%d \n", old_seqnum);
                fprintf(manager->logfp, "CountLossRate2: insize=%d \n", insize);
                fprintf(manager->logfp, "CountLossRate2: freq=%d \n", freq);
                fflush(manager->logfp);
            }
        }

    }
    else{
        printf("CountLossRate2: is_hcsvcrtp=%d \n", is_hcsvcrtp);
    }
    return ret;
}
#endif
HCSVC_API
int api_count_loss_rate3(char *handle, uint8_t* dataPtr, int insize, int freq)
{
    int ret = -2;
    RtpInfo info = {0};
    info.raw_offset = -1;
    int type = freq == 90 ? H264_PLT : AAC_PLT;
    int is_hcsvcrtp = GetRtpInfo(dataPtr, insize, &info, type);
    if(is_hcsvcrtp > 0)
    {
        unsigned int ssrc = info.ssrc;
        PacketManager *obj = NULL;
        //
        int is_video = freq == 90;
        AddrManagerInit(is_video);
        int addrIdx = (ssrc & 0xFFFF);// + (ssrc >> 16);
        AddrManager *addrObj = &globAddrManager[addrIdx];
        if(!is_video)
        {
            addrObj = &globAddrManager2[addrIdx];
        }
        //printf("api_count_loss_rate3: addrIdx=%d \n", addrIdx);
        //printf("api_count_loss_rate3: addrObj->size=%d \n", addrObj->size);
        for(int i = 0; i < addrObj->count; i++)
        {
            LossHandle *lossObj = &addrObj->hndList[i];
            if(lossObj->ssrc == ssrc)
            {
                obj = lossObj->obj;
                break;
            }
        }
        if(!obj)
        {
            obj = (PacketManager *)calloc(1, sizeof(PacketManager));
            if(addrObj->count >= addrObj->size)
            {
                printf("api_count_loss_rate3: addrObj->count=%d \n", addrObj->count);
                printf("api_count_loss_rate3: addrObj->size=%d \n", addrObj->size);
                printf("api_count_loss_rate3: is_video=%d \n", is_video);
                printf("api_count_loss_rate3: ssrc=%u \n", ssrc);
                printf("api_count_loss_rate3: addrIdx=%d \n", addrIdx);
                //recalloc
            }
            else{
                addrObj->hndList[addrObj->count].ssrc = ssrc;
                addrObj->hndList[addrObj->count].obj = obj;
                addrObj->count++;
            }
            RtpInfo info = {0};
            info.raw_offset = -1;
            int is_hcsvcrtp = GetRtpInfo(dataPtr, insize, &info, type);
            if(is_hcsvcrtp > 0)
            {
                unsigned int ssrc = info.ssrc;
                PacketManagerInit(obj, ssrc, &info, freq);
                //return -1;
            }
            else{
                return -100;
            }
        }
        int layerId = info.spatial_idx;
        if(layerId != obj->layerId)
        {
            for(int64_t i = obj->min_seqnum; i < obj->max_seqnum; i++)
            {
                RtpPacketInfo *info = &obj->info[i % MAX_PKT_BUF_SIZE];
                info->status = 0;
                info->timestamp = -1;
                info->seqnum = -1;
            }
            obj->layerId = layerId;
            obj->time_stamp = get_sys_time();
            obj->min_seqnum = info.seqnum + LEFT_SHIFT16;
            obj->max_seqnum = info.seqnum + LEFT_SHIFT16;
            obj->old_seqnum = -1;
            obj->status = 0;
            int I0 = info.seqnum % MAX_PKT_BUF_SIZE;
            obj->info[I0].status = 1;
            obj->info[I0].seqnum = info.seqnum + LEFT_SHIFT16;
            obj->info[I0].timestamp = info.timestamp + LEFT_SHIFT32;
            return ret;
        }
        ret = CountLossRate2(obj, dataPtr, insize, freq);
        if(ret >= 0)
        {
            //printf("api_count_loss_rate3: ret=%d \n", ret);
            ret += 1;
            //
            ///ret = add_loss_rate(obj->loss_rate_info, ret);
        }

    }
    return ret;
}
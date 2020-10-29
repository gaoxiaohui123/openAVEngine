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
#ifdef _WIN32
#ifdef  GVEngine_EXPORTS
#define HCSVC_API __declspec(dllexport)
#else
#define HCSVC_API __declspec(dllimport)
#endif
#else
#define HCSVC_API __attribute__ ((__visibility__("default")))
#endif

#define H264_PLT 127
#define AAC_PLT  126
#define MAX_PKT_NUM 1024
#define MAX_OBJ_NUM  20//100
#define MAX_OUTCHAR_SIZE (5 * MAX_PKT_NUM)
#define TIME_OUTCHAR_SIZE 16
#if 1
typedef struct
{
    /**//* byte 0 */
    unsigned char csrc_len:4;        /**//* expect 0 */
    unsigned char extension:1;        /**//* expect 1, see RTP_OP below */
    unsigned char padding:1;        /**//* expect 0 */
    unsigned char version:2;        /**//* expect 2 */
    /**//* byte 1 */
    unsigned char payload:7;        /**//* RTP_PAYLOAD_RTSP */
    unsigned char marker:1;        /**//* expect 1 */
    /**//* bytes 2, 3 */
    unsigned short seq_no;
    /**//* bytes 4-7 */
    unsigned  int timestamp;
    /**//* bytes 8-11 */
    unsigned int ssrc;            /**//* stream number is used here. */
} RTP_FIXED_HEADER;


typedef struct {
    //byte 0
	unsigned char TYPE:5;
    unsigned char NRI:2;
	unsigned char F:1;
} NALU_HEADER; /**//* 1 BYTES */

typedef struct {
    //byte 0
    unsigned char TYPE:5;
	unsigned char NRI:2;
	unsigned char F:1;
} FU_INDICATOR; /**//* 1 BYTES */

typedef struct {
    //byte 0
    unsigned char TYPE:5;
	unsigned char R:1;  //Reserved
	unsigned char E:1;  //End
	unsigned char S:1;  //Start
} FU_HEADER; /**//* 1 BYTES */

typedef struct
{
  	int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
  	unsigned int len;             //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
  	unsigned int max_size;        //! Nal Unit Buffer size
  	int forbidden_bit;            //! should be always FALSE
  	int nal_reference_idc;        //! NALU_PRIORITY_xxxx
  	int nal_unit_type;            //! NALU_TYPE_xxxx
  	char *buf;                    //! contains the first byte followed by the EBSP
  	unsigned short lost_packets;  //! true, if packet loss is detected
} NALU_t;

typedef struct {
	unsigned int codec_id : 3;//4; //3	// identifies the code/codec being used. In practice, the "FEC encoding ID" that identifies the FEC Scheme should
					 // be used instead (see [RFC5052]). In our example, we are not compliant with the RFCs anyway, so keep it simple.
	unsigned int k : 9;//8; //9 : 512
	unsigned int n : 10;
	unsigned int symbol_size : 10;//symbol_size >> 2
	unsigned short fec_seq_no : 10;
	unsigned short resv;
}FEC_HEADER;
typedef struct {
    unsigned short st0; //client send packet time
    unsigned short rt0; //server receive packet time
    unsigned short st1; //server send packet time
    unsigned short rt1; //client receive packet time
}RTT_HEADER;
typedef union{
    int64_t time_stamp;
    RTT_HEADER rtt_list;
}TIME_HEADER;
typedef struct {
    unsigned char lost_packet_rate : 7; //a% * 100
    unsigned char is_lost_packet : 1;   //
    unsigned char time_status : 1;      //0:time_stamp; 1:rtt_list
    unsigned char rsv : 7;
    short time_offset;
    TIME_HEADER time_info;
}NACK_HEADER;
typedef struct {
	short rtp_extend_profile;       //profile used
	short rtp_extend_length;        //扩展字段的长度，为4的整数倍；1表示4个字节
    //4
	unsigned short rtp_pkt_size;	//rtp包数据大小（包含包头、扩展包）
	unsigned short seq_no;			//注意：此包序独立于整个rtp包序，是单一域的包序
	//8
	unsigned short refs : 5;		//时域信息：	时域层数（0为非SVC； 最大为1<<4）
	unsigned short ref_idx : 5;		//时域信息:	当前帧伸缩序号
	unsigned short ref_idc : 2;		//时域信息：	当前帧重要程度（0为最重要）
	unsigned short res_num : 2;		//空域信息：空域层数
	unsigned short res_idx : 2;		//空域信息：当前分辨率序号（0为最大分辨率）
	//
	unsigned short qua_num : 2;	    //质量域信息：质量域层数（此为多码流数）
	unsigned short qua_idx : 2;		//质量域信息：质量域序号（0为高码率）
	unsigned short first_slice : 1;
	unsigned short enable_fec : 1;
	unsigned short type : 5; //nal_unit_type
	unsigned short refresh_idr : 1;//有更新置1,下一個IDR恢復到0
	unsigned short raw_offset : 1; //视频裸流偏移//0：0; 1: 2字节
	unsigned short main_spatial_idx : 2; //空域信息：主分辨率序号（0为最大分辨率）
	unsigned short mult_spatial : 1;
	unsigned short rsv : 2;
    //12
	NACK_HEADER nack;
} EXTEND_HEADER;

#endif

typedef struct {
    int info2;
    int info3;
    int nal_mem_num;
    int nal_num;
    int size;
    unsigned  int timestamp;
    uint16_t refresh_idr;
    uint16_t enable_fec;
    char *buf;
    NALU_t *nal;//[MAX_RTP_NUM];
}SVCNalu;

static int global_rtp_obj_status = 0;
//static RtpObj global_rtp_objs[MAX_OBJ_NUM];
static char global_info_outparam[MAX_OBJ_NUM][MAX_OUTCHAR_SIZE];
static char global_time_outparam[MAX_OBJ_NUM][MAX_OUTCHAR_SIZE];

//extern cJSON* mystr2json(char *text);
//extern int GetvalueInt(cJSON *json, char *key);
//extern char* GetvalueStr(cJSON *json, char *key);
//extern cJSON* renewJson(cJSON *json, char *key, int ivalue, char *cvalue, cJSON *subJson);
//extern cJSON* renewJsonInt(cJSON *json, char *key, int ivalue);
//extern cJSON* renewJsonStr(cJSON *json, char *key, char *cvalue);
//extern cJSON* deleteJson(cJSON *json);
//extern cJSON* renewJsonArray2(cJSON *json, char *key, short *value);

//extern int api_fec_encode(int id, char *data, char *param, char *outbuf, char *outparam[]);
//extern int api_fec_decode(int id, char *data, char *param, char *outbuf, char *outparam[]);

//int FindStartCode2 (unsigned char *Buf);//查找开始字符0x000001
//int FindStartCode3 (unsigned char *Buf);//查找开始字符0x00000001

unsigned char test_data[100 * 1024];
static int test_flag0 = 0;
static int test_flag1 = 0;


//==================for json=============================

//修改对象的值
//cJSON_ReplaceItemInObject(item,"word",cJSON_CreateString("nihaoxiaobai"));
//cJSON_AddItemToObject(body, "Info", filter_root);
static cJSON* renewJson(cJSON *json, char *key, int ivalue, char *cvalue, cJSON *subJson)
{
    cJSON *ret  = json;
    if(NULL == json)
    {
        ret = cJSON_CreateObject(); //创建JSON对象

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
            cJSON *item = cJSON_GetObjectItem(ret, key);
            if(cJSON_IsArray(item) || cJSON_IsObject(item))
            {
                //cJSON_ReplaceItemInObject(ret, key, cJSON_CreateArray(subJson));
                //cJSON_ReplaceItemInObject(ret, key, cJSON_CreateObject(subJson));
                cJSON_ReplaceItemInObject(ret, key, subJson);
            }
            else{
                cJSON_AddItemToObject(ret, key, subJson);
            }
        }
        else if(NULL != cvalue)
        {
            cJSON *item = cJSON_GetObjectItem(ret, key);
            if(cJSON_IsString(item))
            {
                cJSON_ReplaceItemInObject(ret, key, cJSON_CreateString(cvalue));
                //cJSON_ReplaceItemInObject(ret, key, cvalue);
            }
            else{
                cJSON_AddStringToObject(ret, key, cvalue);
            }
        }
        else
        {
            cJSON *item = cJSON_GetObjectItem(ret, key);
            if(cJSON_IsNumber(item))
            {
                cJSON_ReplaceItemInObject(ret, key, cJSON_CreateNumber(ivalue));
                //cJSON_ReplaceItemInObject(ret, key, ivalue);
            }
            else{
                cJSON_AddNumberToObject(ret, key, ivalue);
            }
        }
    }
    else{
        cJSON_Delete(ret); //释放json对象
    }
    return ret;
}
static cJSON* renewJsonArray(cJSON *json, char *key, int *value, int len)
{
    cJSON *array = cJSON_CreateArray();
    cJSON_AddItemToObject(json, key, array);
    for(int i = 0; i < len; i++)
    {
        cJSON_AddItemToArray(array, cJSON_CreateNumber(value[i]));
    }
    return json;
}
static cJSON* renewJsonArray2(cJSON *json, char *key, short *value)
{
    cJSON *array = cJSON_CreateArray();
    cJSON_AddItemToObject(json, key, array);
    int i = 0;
    while(value[i] > 0)
    {
        cJSON_AddItemToArray(array, cJSON_CreateNumber(value[i]));
        i++;
    }
    return json;
}
static cJSON* renewJsonInt(cJSON *json, char *key, int ivalue)
{
    return renewJson(json, key, ivalue, NULL, NULL);
}
static cJSON* renewJsonStr(cJSON *json, char *key, char *cvalue)
{
    return renewJson(json, key, 0, cvalue, NULL);
}
static cJSON* deleteJson(cJSON *json)
{
    return renewJson(json, NULL, 0, NULL, NULL);
}
static cJSON* mystr2json(char *text)
{
    char *out;
    cJSON *json;
    if (text == NULL || !strcmp(text, ""))
    {
        //char *text = "{\"size\" : 1024, \"data\" : \"this is string\"}";
        //char* text = "{\"name\":\"Messi\",\"age\":\"29\"}";
        char data[128] = "";
        for (int i = 0; i < 128; i++)
        {
            data[i] = (char)i;
        }
        //text = "{\"size\" : 1024, \"data\" : data}";
        text = "{\"size\" : 1024, \"data\" : \"this is default string for test cjosn\"}";
    }

    json = cJSON_Parse(text);
    if (!json) {
        printf("Error before: [%s]\n",cJSON_GetErrorPtr());
    } else {
        //将传入的JSON结构转化为字符串
        out=cJSON_Print(json);
        //cJSON_Delete(json);
        //printf("%s\n",out);
        free(out);
    }
    return json;
}
static float GetvalueFloat(cJSON *json, char *key)
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
static int GetvalueInt(cJSON *json, char *key)
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
static char* GetvalueStr(cJSON *json, char *key)
{
    cJSON *item = cJSON_GetObjectItem(json, key);
    if(cJSON_IsNull(item))
    {
    }
    else if(cJSON_IsString(item))
    {
        return item->valuestring;
    }
    return "";
}
//cjson 遍历
static void foreach()
{
    char *parmstr = "{\"video\":{\"mjpeg\":\"1x2\", \"raw\":\"3x4\"}}";
    cJSON *json = mystr2json(parmstr);
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
    return (void *)renewJson((cJSON *)json, key, ivalue, NULL, NULL);
}
HCSVC_API
void* api_renew_json_str(void *json, char *key, char *cvalue)
{
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
void api_json2str_free(char *jsonstr)
{
    if(jsonstr)
    {
        free(jsonstr);
    }
}
//=======================================================
#ifdef _WIN32
static int64_t get_sys_time()
{
	LARGE_INTEGER m_nFreq;
	LARGE_INTEGER m_nTime;
	QueryPerformanceFrequency(&m_nFreq); // ��ȡʱ������
	QueryPerformanceCounter(&m_nTime);//��ȡ��ǰʱ��
	//long long time = (m_nTime.QuadPart * 1000000 / m_nFreq.QuadPart);//΢��
	int64_t time = (m_nTime.QuadPart * 1000 / m_nFreq.QuadPart);//����
	return time;
}
#else
#include <sys/time.h>
static int64_t get_sys_time()
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

static void get_time(int64_t time, char *ctime, int offset)
{
#ifdef _WIN32
	if (time == 0)
	{
		time = av_gettime() + offset;
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
static int64_t get_time_stamp(void)
{
    int64_t time_stamp = get_sys_time();
    //printf("get_time_stamp: time_stamp= %lld \n", time_stamp);
    return time_stamp;
}


static char *get_extern_info(char *data)
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
        int refs = rtp_ext->refs;
		int ref_idx = rtp_ext->ref_idx;
		int ref_idc = rtp_ext->ref_idc;
		int res_num = rtp_ext->res_num;
		int res_idx = rtp_ext->res_idx;
		int qua_num = rtp_ext->qua_num;
		int qua_idx = rtp_ext->qua_idx;
		int enable_fec = rtp_ext->enable_fec;
		int refresh_idr = rtp_ext->refresh_idr;
		int is_lost_packet = rtp_ext->nack.is_lost_packet;
		int lost_packet_rate = rtp_ext->nack.lost_packet_rate;
		int time_status = rtp_ext->nack.time_status;
		int time_offset = rtp_ext->nack.time_offset;
		int seqnum = rtp_hdr->seq_no;

		unsigned int timestamp = rtp_hdr->timestamp;
		int64_t packet_time_stamp = rtp_ext->nack.time_info.time_stamp;
		//printf("get_extern_info: refs= %d, ref_idx= %d \n", refs, ref_idx);
		cJSON *pJsonRoot  = NULL;
		char *key = "refs";
		int ivalue = refs;
		pJsonRoot = renewJsonInt(pJsonRoot, key, ivalue);
		key = "ref_idx";
		ivalue = ref_idx;
		pJsonRoot = renewJsonInt(pJsonRoot, key, ivalue);
		key = "ref_idc";
		ivalue = ref_idc;
		pJsonRoot = renewJsonInt(pJsonRoot, key, ivalue);
		key = "res_num";
		ivalue = res_num;
		pJsonRoot = renewJsonInt(pJsonRoot, key, ivalue);
		key = "res_idx";
		ivalue = res_idx;
		pJsonRoot = renewJsonInt(pJsonRoot, key, ivalue);
		key = "qua_num";
		ivalue = qua_num;
		pJsonRoot = renewJsonInt(pJsonRoot, key, ivalue);

		key = "qua_idx";
		ivalue = qua_idx;
		pJsonRoot = renewJsonInt(pJsonRoot, key, ivalue);
		key = "is_lost_packet";
		ivalue = is_lost_packet;
		pJsonRoot = renewJsonInt(pJsonRoot, key, ivalue);
		key = "lost_packet_rate";
		ivalue = lost_packet_rate;
		pJsonRoot = renewJsonInt(pJsonRoot, key, ivalue);
		key = "time_status";
		ivalue = time_status;
		pJsonRoot = renewJsonInt(pJsonRoot, key, ivalue);
		key = "time_offset";
		ivalue = time_offset;
		pJsonRoot = renewJsonInt(pJsonRoot, key, ivalue);
		key = "seqnum";
		ivalue = seqnum;
		pJsonRoot = renewJsonInt(pJsonRoot, key, ivalue);
		key = "enable_fec";
		ivalue = enable_fec;
		pJsonRoot = renewJsonInt(pJsonRoot, key, ivalue);
		key = "refresh_idr";
		ivalue = refresh_idr;
		pJsonRoot = renewJsonInt(pJsonRoot, key, ivalue);
		key = "timestamp";
		ivalue = timestamp;
		pJsonRoot = renewJsonInt(pJsonRoot, key, ivalue);
		key = "packet_time_stamp";
		char ctmp[32] = "";
	    sprintf(ctmp, "%ld", packet_time_stamp);
		pJsonRoot = renewJsonStr(pJsonRoot, key, ctmp);

		ret = cJSON_Print(pJsonRoot); //free(ret);//返回json字符串，注意外面用完要记得释放空间

		deleteJson(pJsonRoot);

    }
    return ret;
}
static char *get_audio_extern_info(char *data)
{
    char *ret = NULL;
    RTP_FIXED_HEADER  *rtp_hdr = NULL;
    EXTEND_HEADER *rtp_ext = NULL;
    rtp_hdr = (RTP_FIXED_HEADER *)data;
    int isrtp = (rtp_hdr->version == 2 &&  //版本号，此版本固定为2								V
            rtp_hdr->padding	== 0 &&  //														P
            rtp_hdr->csrc_len == 0 && //												X
            rtp_hdr->payload == AAC_PLT //负载类型号
            );
    if(isrtp && rtp_hdr->extension)
    {
        rtp_ext = (EXTEND_HEADER *)&data[sizeof(RTP_FIXED_HEADER)];
		int is_lost_packet = rtp_ext->nack.is_lost_packet;
		int lost_packet_rate = rtp_ext->nack.lost_packet_rate;
		int time_status = rtp_ext->nack.time_status;
		int time_offset = rtp_ext->nack.time_offset;
		int seqnum = rtp_hdr->seq_no;

		unsigned int timestamp = rtp_hdr->timestamp;
		int64_t packet_time_stamp = rtp_ext->nack.time_info.time_stamp;
		//printf("get_extern_info: refs= %d, ref_idx= %d \n", refs, ref_idx);
		cJSON *pJsonRoot  = NULL;

		char *key = "is_lost_packet";
		int ivalue = is_lost_packet;
		pJsonRoot = renewJsonInt(pJsonRoot, key, ivalue);
		key = "lost_packet_rate";
		ivalue = lost_packet_rate;
		pJsonRoot = renewJsonInt(pJsonRoot, key, ivalue);
		key = "time_status";
		ivalue = time_status;
		pJsonRoot = renewJsonInt(pJsonRoot, key, ivalue);
		key = "time_offset";
		ivalue = time_offset;
		pJsonRoot = renewJsonInt(pJsonRoot, key, ivalue);
		key = "seqnum";
		ivalue = seqnum;
		pJsonRoot = renewJsonInt(pJsonRoot, key, ivalue);
		key = "timestamp";
		ivalue = timestamp;
		pJsonRoot = renewJsonInt(pJsonRoot, key, ivalue);
		key = "packet_time_stamp";
		char ctmp[32] = "";
	    sprintf(ctmp, "%ld", packet_time_stamp);
		pJsonRoot = renewJsonStr(pJsonRoot, key, ctmp);

		ret = cJSON_Print(pJsonRoot); //free(ret);//返回json字符串，注意外面用完要记得释放空间

		deleteJson(pJsonRoot);

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
//HCSVC_API
//int api_get_time_stamp(int id, char *outparam[])
//{
//    int64_t time_stamp = get_sys_time();
//    char *text = global_time_outparam[id];
//    memset(global_time_outparam[id], 0, sizeof(char) * MAX_OUTCHAR_SIZE);
//    sprintf(text, "%ld", time_stamp);
//    outparam[0] = text;
//    return (int)time_stamp;
//}
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
//int api_renew_time_stamp(int id, char *data, char *outparam[])
int api_renew_time_stamp(char *data)
{

    //char *text = global_time_outparam[id];
    //memset(global_time_outparam[id], 0, sizeof(char) * MAX_OUTCHAR_SIZE);
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
            int time_status = rtp_ext->nack.time_status;
		    if(time_status == 0)
		    {
		        rtp_ext->nack.time_info.time_stamp = time_stamp;
		    }
        }
    }
    return (int)time_stamp;
}
#if 0
HCSVC_API
int api_get_time_stamp2(int id, char *data, char *outparam[])
{

    char *text = global_time_outparam[id];
    memset(global_time_outparam[id], 0, sizeof(char) * MAX_OUTCHAR_SIZE);
    int64_t time_stamp = get_sys_time();
    sprintf(text, "%ld", time_stamp);
    outparam[0] = text;
    if(data != NULL)
    {
        RTP_FIXED_HEADER  *rtp_hdr = NULL;
        EXTEND_HEADER *rtp_ext = NULL;
        rtp_hdr = (RTP_FIXED_HEADER *)data;
        if(rtp_hdr->extension)
        {
            rtp_ext = (EXTEND_HEADER *)&data[sizeof(RTP_FIXED_HEADER)];
            int time_status = rtp_ext->nack.time_status;
		    if(time_status == 0)
		    {
		        int64_t packet_time_stamp = rtp_ext->nack.time_info.time_stamp;
		        //text = global_outparam[id + 1];
		        //memset(global_outparam[id + 1], 0, sizeof(char) * MAX_OUTCHAR_SIZE);

		        text = global_time_outparam[id];
		        memset(global_time_outparam[id], 0, sizeof(char) * MAX_OUTCHAR_SIZE);

		        sprintf(text, "%ld", packet_time_stamp);
                outparam[1] = text;
		    }
        }
    }
    return (int)time_stamp;
}
#endif
HCSVC_API
int api_get_extern_info(int id, char *data, char *outparam[])
{
    int ret = 0;
    char *extend_info = get_extern_info(data);
    if(NULL != extend_info)
    {
        char *text = global_info_outparam[id];
        memset(global_info_outparam[id], 0, sizeof(char) * MAX_OUTCHAR_SIZE);
        outparam[0] = text;
        memcpy(text, extend_info, strlen(extend_info));
        ret = strlen(extend_info);
        free(extend_info);
    }
    return ret;
}
HCSVC_API
int api_get_audio_extern_info(int id, char *data, char *outparam[])
{
    int ret = 0;
    char *extend_info = get_audio_extern_info(data);
    if(NULL != extend_info)
    {
        char *text = global_info_outparam[id];
        memset(global_info_outparam[id], 0, sizeof(char) * MAX_OUTCHAR_SIZE);
        outparam[0] = text;
        memcpy(text, extend_info, strlen(extend_info));
        ret = strlen(extend_info);
        free(extend_info);
    }
    return ret;
}
typedef struct {
    int spatial_idx;        //空域第几层
    int spatial_num;        //空域总共层数
    int is_fec;             //是否是fec冗余块
    int enable_fec;         //是否开启了fec
    int nal_type;           // NAL类型
    int rtp_header_size;    //rtp头包括扩展头的大小
    int nal_marker;         //是否是当前层的最后块
    int raw_offset;
    int main_spatial_idx;   //主分辨率序号
    int mult_spatial;
    int fec_k;
    int fec_n;
    int ref_idc;
    int seqnum;
    int is_first_slice;
    //int fec_seq_no;
    int refs;		//时域信息：	时域层数（0为非SVC； 最大为1<<4）
	int ref_idx;		//时域信息:	当前帧伸缩序号
	unsigned int ssrc;
	unsigned int timestamp;
}RtpInfo;


static int GetRtpInfo(uint8_t* dataPtr, int insize, RtpInfo *info)
{
    int ret = 0;
    int isrtp = 0;
    if(insize < sizeof(RTP_FIXED_HEADER))
    {
        return ret;
    }
    info->raw_offset = 2;
    RTP_FIXED_HEADER* rtp_hdr = (RTP_FIXED_HEADER*)&dataPtr[info->raw_offset];
    isrtp = (rtp_hdr->version == 2 &&  //版本号，此版本固定为2								V
            rtp_hdr->padding	== 0 &&  //														P
            rtp_hdr->csrc_len == 0 && //												X
            (rtp_hdr->payload == H264_PLT || rtp_hdr->payload == AAC_PLT) //负载类型号
            );
    if(!isrtp)
    {
        rtp_hdr = (RTP_FIXED_HEADER*)&dataPtr[0];
        isrtp = (rtp_hdr->version == 2 &&  //版本号，此版本固定为2								V
            rtp_hdr->padding	== 0 &&  //														P
            rtp_hdr->csrc_len == 0 && //												X
            (rtp_hdr->payload == H264_PLT || rtp_hdr->payload == AAC_PLT)  //负载类型号
            );
        if(isrtp)
        {
            info->raw_offset = 0;
        }
    }
    if(isrtp)
    {

        int offset = sizeof(RTP_FIXED_HEADER);//1;
        info->seqnum = rtp_hdr->seq_no;
        info->ssrc = rtp_hdr->ssrc;
        info->timestamp = rtp_hdr->timestamp;
        if(rtp_hdr->payload == AAC_PLT)
        {
           ret = 1;
           return ret;
        }
        if(insize < (sizeof(RTP_FIXED_HEADER) + sizeof(EXTEND_HEADER)))
        {
            return ret;
        }
        EXTEND_HEADER *rtp_ext = NULL;
        if(rtp_hdr->extension)
        {
            rtp_ext = (EXTEND_HEADER *)&dataPtr[info->raw_offset + sizeof(RTP_FIXED_HEADER)];
            int rtp_extend_length = rtp_ext->rtp_extend_length;
            rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
            //printf("IsRtpData: rtp_extend_length= %d \n", rtp_extend_length);
            rtp_extend_length = (rtp_extend_length + 1) << 2;
            if(rtp_extend_length > (sizeof(EXTEND_HEADER) + sizeof(FEC_HEADER)))
            {
                printf("GetRtpInfo: rtp_extend_length= %d \n", rtp_extend_length);
                return 0;
            }
            int rtp_pkt_size = rtp_ext->rtp_pkt_size;
            if(rtp_pkt_size > 1500 || rtp_pkt_size > insize)
            {
                printf("GetRtpInfo: rtp_pkt_size= %d \n", rtp_pkt_size);
                return 0;
            }
            else
            {
                if(!rtp_ext->enable_fec && (rtp_pkt_size + info->raw_offset) != insize)
                {
                    printf("GetRtpInfo: rtp_pkt_size= %d, insize=%d \n", rtp_pkt_size, insize);
                    return 0;
                }
            }
            //printf("IsRtpData: 2: rtp_extend_length= %d \n", rtp_extend_length);
            offset = sizeof(RTP_FIXED_HEADER) + rtp_extend_length;
            //if(rtp_ext->raw_offset)
            //{
            //    info->raw_offset = 2;
            //}
            //else{
            //    info->raw_offset = 0;
            //}

            info->rtp_header_size = offset + info->raw_offset;
            info->ref_idc = rtp_ext->ref_idc + 1;
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
                if((symbol_size + info->rtp_header_size) != insize || k > n || fec_seq_no > n)
                {
                    printf("GetRtpInfo: k=%d, n=%d, symbol_size=%d, insize=%d \n", k, n, symbol_size, insize);
                    return 0;
                }
                //info->fec_seq_no = fec_seq_no;
		        //printf("IsRtpData: k= %d \n", k);
		        //printf("IsRtpData: n= %d \n", n);
		        //printf("IsRtpData: fec_seq_no= %d \n", fec_seq_no);
                info->is_fec = fec_seq_no >= k;
                if(fec_seq_no == (n - 1))
                {
                    info->nal_marker = 1;
                }
                else if(fec_seq_no == (k - 1))
                {
                    info->nal_marker = 2;
                }
            }
            if(rtp_ext->first_slice && !info->is_fec)
            {
                info->is_first_slice = 1;
            }

        }
        NALU_HEADER* nalu_hdr = (NALU_HEADER*)&dataPtr[info->raw_offset + offset + 0];
        info->nal_type = nalu_hdr->TYPE;
        if(rtp_ext)
        {
            info->nal_type = rtp_ext->type;
        }
        //if(rtp_ext && (nalu_hdr->TYPE != rtp_ext->type))
        //{
        //    printf("GetRtpInfo: nalu_hdr->TYPE= %d, rtp_ext->type= %d \n", nalu_hdr->TYPE, rtp_ext->type);
        //}


        if(info->nal_type == 28)
        {
            FU_HEADER* fu_hdr = (FU_HEADER*)&dataPtr[info->raw_offset + offset + 1];
            ret = (fu_hdr->S == 1) ? 1 : 2;
            if(!info->enable_fec)
            {
                info->nal_marker = fu_hdr->E == 1;
            }
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
                //独立片或pps/sps/sei
                ret = (info->nal_type == 7) || (info->nal_type == 1) || (info->nal_type == 5) ? 1 : 2;
                if(!info->enable_fec)
                {
                    info->nal_marker = (nalu_hdr->TYPE == 1) || (nalu_hdr->TYPE == 5);
                    //(nalu_hdr->TYPE != 6) && (nalu_hdr->TYPE != 7) && (nalu_hdr->TYPE != 8);
                }
                info->is_first_slice = (info->nal_type == 7) || (info->nal_type == 1) || (info->nal_type == 5);
            }
            else {
                ret = 2;
            }
        }

        //printf("IsRtpData: nalu_hdr->TYPE= %d \n", nalu_hdr->TYPE);
    }
    return ret;
}
//if return 1 ,then discast it
//丢包率是动态变化的；
//主动丢包后，丢包率是持续改善的；
//需要有记忆，视丢包的改善情况来决定是否要加大主动丢包；
//需要甄别，主动丢包也无法改善的丢包
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
static int CheckPacket(uint8_t* dataPtr, int insize, unsigned int ssrc, float lossRate0)
{
    int ret = 0;
    RtpInfo info = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int is_hcsvcrtp = GetRtpInfo(dataPtr, insize, &info);
    if(is_hcsvcrtp && (ssrc == info.ssrc))
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
#define MAX_PKT_BUF_SIZE (1 << 13)
#define PKT_LOSS_INTERVAL 1000
#define MAX_PKT_DELAY 500
#define LEFT_SHIFT32 ((long long)1 << 32)
#define LEFT_SHIFT16 ((int)1 << 16)
#define HALF_USHORT (1 << 15)
#define QUART_USHORT (1 << 14)
#define HALF_QUART_USHORT (HALF_USHORT + QUART_USHORT)
typedef struct
{
    int seqnum;
    int64_t timestamp;
}RtpPacketInfo;
typedef struct
{
    int idx;
    unsigned int ssrc;
    int64_t time_stamp;
    int min_seqnum;
    int max_seqnum;
    int old_seqnum;
    RtpPacketInfo info[MAX_PKT_BUF_SIZE];
}PacketManager;
//static PacketManager globPktManager = {0, 0, -1, -1, -1, -1};
//static PacketManager *globPacketManagers = NULL;//[MAX_SSRC_NUM];
static long long globPktManagerHnd[MAX_SSRC_NUM];
static int status1 = 0;
static unsigned int test_ssrc = 0;

static void CountLossRateInit()
{
    if(!status1)
    {
        //globPacketManagers = (PacketManager *)calloc(1, MAX_SSRC_NUM * sizeof(PacketManager));
        //PacketManager manager = {0, 0, -1, -1, -1, -1};

        //RtpPacketInfo *info = &manager.info;
        //for(int i = 0; i < MAX_PKT_BUF_SIZE; i++)
        //{
        //    globPktManager.info[i].seqnum = -1;
        //    globPktManager.info[i].timestamp = -1;
        //}
        //for(int i = 0; i < MAX_SSRC_NUM; i++)
        //{
        //    globPacketManagers[i] = manager;
        //}
        //int size = sizeof(PacketManager);
        memset((void *)globPktManagerHnd, 0, MAX_SSRC_NUM * sizeof(long long));
        status1 = 1;
    }
}
static void *GetPacketManager(unsigned int ssrc, RtpInfo *info)
{
    unsigned short hight_value = ssrc >> 16;
    unsigned short low_value = ssrc & 0xFFFF;
    unsigned short idx = hight_value + low_value;
    //printf("GetPacketManager: idx=%d \n", idx);

    PacketManager *ret = (PacketManager *)globPktManagerHnd[idx];//&globPacketManagers[idx];
    if(!ret)
    {
        //if(test_ssrc)
        //{
        //    printf("GetPacketManager: ssrc=%x \n", ssrc);
        //    return ret;
        //}
        printf("GetPacketManager: ssrc=%x \n", ssrc);
        printf("GetPacketManager: idx=%d \n", idx);
        ret = (PacketManager *)calloc(1, sizeof(PacketManager));
        for(int i = 0; i < MAX_PKT_BUF_SIZE; i++)
        {
            ret->info[i].seqnum = -1;
            ret->info[i].timestamp = -1;
        }
        //printf("GetPacketManager: 1 \n");
        //ret[0] = globPktManager;
        //printf("GetPacketManager: 2 \n");
        ret->ssrc = ssrc;
        ret->time_stamp = get_sys_time();
        ret->min_seqnum = info->seqnum;
        ret->max_seqnum = info->seqnum;
        ret->old_seqnum = -1;
        ret->idx = 1;
        globPktManagerHnd[idx] = (long long)ret;
        //printf("GetPacketManager: &globPktManagerHnd[0]=%x \n", &globPktManagerHnd[0]);
        //printf("GetPacketManager: &globPktManagerHnd[idx]=%x \n", &globPktManagerHnd[idx]);
        //int handle_size = sizeof(long long);
        //memcpy(&globPktManagerHnd[idx], ret, handle_size);
        //printf("GetPacketManager: 3 \n");
    }
    else if(ret->ssrc == ssrc)
    {
        ret->idx += 1;
    }
    else{
        //位置冲突
        printf("warning: GetPacketManager: ssrc= %u, ret->ssrc= %u \n", ssrc, ret->ssrc);
        ret = NULL;
    }
    return (void *)ret;
}
int CountLossRate(uint8_t* dataPtr, int insize)
{
    int ret = -1;
    //printf("CountLossRate: insize=%d \n", insize);
    RtpInfo info = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int is_hcsvcrtp = GetRtpInfo(dataPtr, insize, &info);
    if(is_hcsvcrtp)
    {
        unsigned int ssrc = info.ssrc;
        PacketManager *manager = (PacketManager *)GetPacketManager(ssrc, &info);
        if(!manager)
        {
            printf("CountLossRate: insize=%d \n", insize);
            return ret;
        }
        if(test_ssrc != ssrc)
        {
            //printf("CountLossRate: manager=%x \n", manager);
            //printf("CountLossRate: test_ssrc=%x \n", test_ssrc);
            //printf("CountLossRate: ssrc=%x \n", ssrc);
        }
        //
        int seqnum = info.seqnum;
        unsigned int timestamp = info.timestamp;
        int idx = seqnum % MAX_PKT_BUF_SIZE;
        //
        int is_old_packet = 0;
        int old_seqnum = manager->old_seqnum;
        if(old_seqnum >= 0)
        {
            if(old_seqnum > HALF_QUART_USHORT && seqnum < QUART_USHORT)
            {
            }
            else if(old_seqnum < QUART_USHORT && seqnum > HALF_QUART_USHORT)
            {
                is_old_packet  = 1;
            }
            else if(seqnum <= old_seqnum)
            {
                is_old_packet  = 1;
            }
        }
        if(old_seqnum < 0 || !is_old_packet)
        {
            int min_seqnum = manager->min_seqnum;
            int max_seqnum = manager->max_seqnum;
            //===
            if((min_seqnum < 0) && (max_seqnum < 0))
            {
                manager->min_seqnum = seqnum;
                manager->max_seqnum = seqnum;
            }
            else if(min_seqnum == max_seqnum)
            {
                if(1)
                {
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
                if(min_seqnum > HALF_QUART_USHORT && seqnum < QUART_USHORT)
                {
                }
                else if(seqnum < min_seqnum)
                {
                    manager->min_seqnum = seqnum;
                }
                //else if(obj->min_packet < HALF_USHORT && (seqnum > HALF_USHORT))
                else if(min_seqnum < QUART_USHORT && (seqnum > HALF_QUART_USHORT))
                {
                    manager->min_seqnum = seqnum;
                }

                if(max_seqnum < QUART_USHORT && seqnum > HALF_QUART_USHORT)
                {
                }
                else if(seqnum > max_seqnum)
                {
                    manager->max_seqnum = seqnum;
                }
                //else if(obj->max_packet > HALF_USHORT && (seqnum < HALF_USHORT))
                else if(max_seqnum > HALF_QUART_USHORT && (seqnum < QUART_USHORT))
                {
                    manager->max_seqnum = seqnum;
                }
            }
            //===
            if(test_ssrc != ssrc)
            {
                //printf("CountLossRate: 1 \n");
            }
            manager->info[idx].seqnum = seqnum;
            manager->info[idx].timestamp = timestamp;
        }
        if(test_ssrc != ssrc)
        {
            //printf("CountLossRate: 2 \n");
        }
        //
        int64_t time_stamp = get_sys_time();
        int diff_time = (int)(time_stamp - manager->time_stamp);
        //if(test_ssrc != ssrc)
        //{
        //    printf("CountLossRate: manager->time_stamp=%lld \n", manager->time_stamp);
        //    printf("CountLossRate: diff_time=%d \n", diff_time);
        //}
        if(diff_time > PKT_LOSS_INTERVAL)
        {
            //printf("CountLossRate: diff_time=%d \n", diff_time);
            int I0 = manager->min_seqnum % MAX_PKT_BUF_SIZE;
            int I1 = manager->max_seqnum % MAX_PKT_BUF_SIZE;
            RtpPacketInfo *info0 = &manager->info[I0];
            RtpPacketInfo *info1 = &manager->info[I1];
            int64_t timestamp0 = info0->timestamp;
            int64_t timestamp1 = info1->timestamp;
            int delay = (int)(timestamp1 - timestamp0) / 90;//90000Hz
            if(timestamp0 > timestamp1)
            {
                delay = (int)((timestamp1 + LEFT_SHIFT32 - timestamp0) / 90);
            }
            //printf("CountLossRate: delay=%d \n", delay);
            //if(test_ssrc != ssrc)
            //{
            //    printf("CountLossRate: delay=%d \n", delay);
            //}
            if(delay > (PKT_LOSS_INTERVAL + MAX_PKT_DELAY))
            {
                //printf("CountLossRate: delay=%d \n", delay);
                int start = manager->min_seqnum;
                int end = manager->max_seqnum;
                if(start > end)
                {
                    end += LEFT_SHIFT16;//MAX_USHORT;
                }
                int last_seq_num = -1;
                int loss_num = 0;
                int pkt_num = 0;
                int i = 0;
                int j = 0;
                for(i = start; i < end; i++)
                {
                    RtpPacketInfo *info = &manager->info[i % MAX_PKT_BUF_SIZE];
                    int64_t this_timestamp = info->timestamp;
                    int this_seqnum = info->seqnum;
                    if(this_timestamp >= 0 && this_seqnum >= 0)
                    {
                        delay = (int)(this_timestamp - timestamp0) / 90;//90000Hz
                        if(timestamp0 > timestamp1)
                        {
                            delay = (int)((this_timestamp + LEFT_SHIFT32 - timestamp0) / 90);
                        }
                        if(delay >= PKT_LOSS_INTERVAL)
                        {
                            if(!pkt_num)
                            {
                                printf("error: CountLossRate: start= %d, end= %d \n", start, end);
                            }
                            ret = (int)(100 * ((float)loss_num / pkt_num));
                            if(loss_num != j)
                            {
                                //printf("warning: CountLossRate: j= %d, loss_num= %d \n", j, loss_num);
                            }
                            manager->time_stamp = get_sys_time();
                            break;
                        }
                        if(last_seq_num >= 0)
                        {
                            int diff_seqnum = this_seqnum - last_seq_num;
                            if(this_seqnum < last_seq_num)
                            {
                                diff_seqnum = (this_seqnum + LEFT_SHIFT16) - last_seq_num;
                            }
                            loss_num += (diff_seqnum - 1);
                        }
                        pkt_num = i - start;
                        last_seq_num = this_seqnum;
                    }
                    else{
                        j++;
                    }
                }
                if(ret >= 0)
                {
                    //clear
                    int stop_seqnum = i;
                    for(int i = start; i < stop_seqnum; i++)
                    {
                        RtpPacketInfo *info = &manager->info[i % MAX_PKT_BUF_SIZE];
                        info->timestamp = -1;
                        info->seqnum = -1;
                    }
                    manager->min_seqnum = stop_seqnum % LEFT_SHIFT16;
                    manager->old_seqnum = manager->min_seqnum;
                }
                //printf("CountLossRate: ret=%d \n", ret);
            }
        }
        //if(test_ssrc != ssrc)
        //{
        //    printf("CountLossRate: ssrc=%x \n", ssrc);
        //}
        test_ssrc = ssrc;
    }
    else{
    }
    return ret;
}

HCSVC_API
int api_get_rtp_info(uint8_t* dataPtr, int insize, RtpInfo *info)
{
    return GetRtpInfo(dataPtr, insize, info);
}
HCSVC_API
int api_check_packet(uint8_t* dataPtr, int insize, unsigned int ssrc, int lossRate)
{
    GetLossRateInit();
    return CheckPacket(dataPtr, insize, ssrc, lossRate);
}
HCSVC_API
int api_count_loss_rate(uint8_t* dataPtr, int insize)
{
    //printf("api_count_loss_rate \n");
    CountLossRateInit();
    return CountLossRate(dataPtr, insize);
}
HCSVC_API
int api_get_pkt_delay(char *dataPtr, int insize)
{
    int ret = -((int)1 << 31);
    if(insize < sizeof(RTP_FIXED_HEADER))
    {
        return ret;
    }
    int raw_offset = 2;
    RTP_FIXED_HEADER* rtp_hdr = (RTP_FIXED_HEADER*)&dataPtr[raw_offset];
    int isrtp = (   rtp_hdr->version == 2 &&  //版本号，此版本固定为2								V
                    rtp_hdr->padding	== 0 &&  //														P
                    rtp_hdr->csrc_len == 0 && //												X
                    rtp_hdr->payload == H264_PLT   //负载类型号
                );
    if(!isrtp)
    {
        rtp_hdr = (RTP_FIXED_HEADER*)&dataPtr[0];
        isrtp = (   rtp_hdr->version == 2 &&  //版本号，此版本固定为2								V
                    rtp_hdr->padding	== 0 &&  //														P
                    rtp_hdr->csrc_len == 0 && //												X
                    rtp_hdr->payload == H264_PLT   //负载类型号
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
        EXTEND_HEADER *rtp_ext = NULL;
        if(rtp_hdr->extension)
        {
            rtp_ext = (EXTEND_HEADER *)&dataPtr[raw_offset + sizeof(RTP_FIXED_HEADER)];
            int rtp_extend_length = rtp_ext->rtp_extend_length;
            rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
            rtp_extend_length = (rtp_extend_length + 1) << 2;
            if(rtp_extend_length > (sizeof(EXTEND_HEADER) + sizeof(FEC_HEADER)))
            {
                //printf("GetRtpInfo: rtp_extend_length= %d \n", rtp_extend_length);
                return 0;
            }
            int rtp_pkt_size = rtp_ext->rtp_pkt_size;
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
            int64_t time0 = rtp_ext->nack.time_info.time_stamp;
            int64_t time1 = get_sys_time();
            int difftime = (int)(time1 - time0);
            ret = difftime;
        }
    }
    return ret;
}
//==============================================================================


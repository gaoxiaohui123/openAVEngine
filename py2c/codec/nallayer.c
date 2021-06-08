/*
 * Copyright (c) 2020
 * Created by gxh_20200602
 */

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

#include "inc.h"


//static int global_rtp_obj_status = 0;
//static RtpObj global_rtp_objs[MAX_OBJ_NUM];
//static char global_info_outparam[MAX_OBJ_NUM][MAX_OUTCHAR_SIZE];
//static char global_time_outparam[MAX_OBJ_NUM][TIME_OUTCHAR_SIZE];

extern int GetvalueInt(cJSON *json, char *key);
extern int *GetArrayValueInt(cJSON *json, char *key, int *arraySize);
extern short *GetArrayValueShort(cJSON *json, char *key, int *arraySize);
extern cJSON* renewJsonArray1(cJSON *json, char *key, short *value, int len);
extern cJSON* renewJsonArray4(cJSON *json, char *key, int *value, int len);
//extern cJSON* renewJsonArray2(cJSON *json, char *key, short *value);
extern cJSON* renewJsonArray3(cJSON **json, cJSON **array, char *key, cJSON *item);
extern long long *GetArrayObj(cJSON *json, char *key, int *arraySize);
//extern inline int GetRtpInfo(uint8_t* dataPtr, int insize, RtpInfo *info);
extern inline int GetNetInfo(uint8_t* dataPtr, int insize, NetInfo *info, int times);
extern inline int GetRtpHeaderSize(uint8_t* dataPtr, int dataSize);
extern int64_t get_sys_time();
extern void netinfo_init(LossRateInfo *loss_rate_info);

//extern int api_fec_encode(int id, char *data, char *param, char *outbuf, char *outparam[]);
//extern int api_fec_decode(int id, char *data, char *param, char *outbuf, char *outparam[]);

int FindStartCode2 (unsigned char *Buf);//查找开始字符0x000001
int FindStartCode3 (unsigned char *Buf);//查找开始字符0x00000001

unsigned char test_data[100 * 1024];
static int test_flag0 = 0;
static int test_flag1 = 0;

#if 1
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
int64_t get_time_stamp(void)
{
    int64_t time_stamp = api_get_time_stamp_ll();
    //printf("get_time_stamp: time_stamp= %lld \n", time_stamp);
    return time_stamp;
}

static void ResetObj(RtpObj *obj)
{
    obj->Obj_id = -1;
    obj->logfp = NULL;
    obj->seq_num = 0;
    obj->refresh_idr = 1;
    obj->enable_fec = 0;
    obj->ssrc = 0;
    obj->time_stamp = 0;
    obj->last_frame_time_stamp = 0;
    obj->start_time_stamp = 0;
    obj->start_frame_time = 0;
    obj->time0 = 0;
    obj->old_seqnum = -1;
    obj->json = NULL;
    obj->param = NULL;
    obj->rtpSize = NULL;
    obj->recv_buf = NULL;
    //obj->send_buf = NULL;
    obj->min_packet = MAX_USHORT;//-1;
    obj->max_packet = -1;
    obj->buf_size = RECV_BUF_NUM;
    for(int i = 0; i < 4; i++)
    {
        memset(obj->outparam[i], 0, sizeof(char) * MAX_OUTCHAR_SIZE);
    }
    memset(obj->sps, 0, sizeof(char) * MAX_SPS_SIZE);
    memset(obj->pps, 0, sizeof(char) * MAX_SPS_SIZE);
    //
    obj->delay_time = 0;
    obj->offset_time = 1;
    obj->delay_info[0].delay = -1;
    obj->delay_info[0].max_delay = 0;
    obj->delay_info[0].last_max_delay = 0;
    obj->delay_info[0].sum_delay = 0;
    obj->delay_info[0].cnt_delay = 0;
    obj->delay_info[0].cnt_max = 1;
    obj->delay_info[0].now_time = 0;
    obj->delay_info[0].time_len = 40;
    obj->delay_info[1].delay = -1;
    obj->delay_info[1].max_delay = 0;
    obj->delay_info[1].last_max_delay = 0;
    obj->delay_info[1].sum_delay = 0;
    obj->delay_info[1].cnt_delay = 0;
    obj->delay_info[1].cnt_max = 10;
    obj->delay_info[1].now_time = 0;
    obj->delay_info[1].time_len = 1000;
    obj->delay_info[2].delay = -1;
    obj->delay_info[2].max_delay = 0;
    obj->delay_info[2].last_max_delay = 0;
    obj->delay_info[2].sum_delay = 0;
    obj->delay_info[2].cnt_delay = 0;
    obj->delay_info[2].cnt_max = 100;
    obj->delay_info[2].now_time = 0;
    obj->delay_info[2].time_len = 5000;
    //
    obj->delay_info[3].delay = -1;
    obj->delay_info[3].max_delay = 0;
    obj->delay_info[3].last_max_delay = 0;
    obj->delay_info[3].sum_delay = 0;
    obj->delay_info[3].cnt_delay = 0;
    obj->delay_info[3].cnt_max = 10000;
    obj->delay_info[3].now_time = 0;
    obj->delay_info[3].time_len = 30 * 1000;

#if 0
    obj->loss_rate_info[0].loss_rate = -1;
    obj->loss_rate_info[0].max_loss_rate = 0;
    obj->loss_rate_info[0].last_max_loss_rate = 0;
    obj->loss_rate_info[0].sum_loss_rate = 0;
    obj->loss_rate_info[0].cnt_loss_rate = 0;
    obj->loss_rate_info[0].cnt_max = 1;
    obj->loss_rate_info[0].now_time = 0;
    obj->loss_rate_info[0].time_len = 40;
    obj->loss_rate_info[1].loss_rate = -1;
    obj->loss_rate_info[1].max_loss_rate = 0;
    obj->loss_rate_info[1].last_max_loss_rate = 0;
    obj->loss_rate_info[1].sum_loss_rate = 0;
    obj->loss_rate_info[1].cnt_loss_rate = 0;
    obj->loss_rate_info[1].cnt_max = 10;
    obj->loss_rate_info[1].now_time = 0;
    obj->loss_rate_info[1].time_len = 1000;
    obj->loss_rate_info[2].loss_rate = -1;
    obj->loss_rate_info[2].max_loss_rate = 0;
    obj->loss_rate_info[2].last_max_loss_rate = 0;
    obj->loss_rate_info[2].sum_loss_rate = 0;
    obj->loss_rate_info[2].cnt_loss_rate = 0;
    obj->loss_rate_info[2].cnt_max = 100;//1000;//100;//统计最大丢包率发生的周期// > 40*100 = 4s
    obj->loss_rate_info[2].now_time = 0;
    obj->loss_rate_info[2].time_len = 4000;
    //
    obj->loss_rate_info[3].loss_rate = -1;
    obj->loss_rate_info[3].max_loss_rate = 0;
    obj->loss_rate_info[3].last_max_loss_rate = 0;
    obj->loss_rate_info[3].sum_loss_rate = 0;
    obj->loss_rate_info[3].cnt_loss_rate = 0;
    obj->loss_rate_info[3].cnt_max = 10000;//1000;//100;//统计最大丢包率发生的周期// > 40*100 = 4s
    obj->loss_rate_info[3].now_time = 0;
    obj->loss_rate_info[3].time_len = 30 * 1000;
#else
    netinfo_init(obj->loss_rate_info);
#endif
}

int FindStartCode2 (unsigned char *Buf)
{

 	if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=1)
		return 0; //判断是否为0x000001,如果是返回1
 	else
		return 1;
}
int FindStartCode3 (unsigned char *Buf)
{
 	if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=0 || Buf[3] !=1)
		return 0;//判断是否为0x00000001,如果是返回1
 	else
		return 1;
}

//输出NALU长度和TYPE
void dump(NALU_t *n)
{
	if (!n)
		return;
	printf(" len: %d  ", n->len);
	printf("nal_unit_type: %x\n", n->nal_unit_type);
}

//static int GetAnnexbNALU (NALU_t *nalu,void *inbuf,int len)
int GetAnnexbNALU(SVCNalu *svc_nalu, int i, int offset)
{
    int pos = 0;
    int StartCodeFound, rewind;
    NALU_t *nalu = &svc_nalu->nal[i];
    unsigned char *Buf = (unsigned char *)&svc_nalu->buf[offset];
    int len = svc_nalu->size - offset;

  /*if ((Buf = (unsigned char*)calloc (nalu->max_size , sizeof(char))) == NULL)
	  printf ("GetAnnexbNALU: Could not allocate Buf memory\n");*/

    nalu->startcodeprefix_len = 3;//初始化码流序列的开始字符为3个字节

  // if (3 != fread (Buf, 1, 3, bits))//从码流中读3个字节
	 //  {
		//free(Buf);
		//return 0;
	 //  }

    svc_nalu->info2 = FindStartCode2 (Buf);//判断是否为0x000001
    if(svc_nalu->info2 != 1)
    {
	    //如果不是，再读一个字节
        //  if(1 != fread(Buf+3, 1, 1, bits))//读一个字节
		//{
		// free(Buf);
		// return 0;
		//}
        svc_nalu->info3 = FindStartCode3 (Buf);//判断是否为0x00000001
        if (svc_nalu->info3 != 1)//如果不是，返回-1
		{
		    //free(Buf);
		    return -1;
		}
        else
		{
		    //如果是0x00000001,得到开始前缀为4个字节
		    pos = 4;
		    nalu->startcodeprefix_len = 4;
		}
    }
    else
    {
	    //如果是0x000001,得到开始前缀为3个字节
		nalu->startcodeprefix_len = 3;
		pos = 3;
	}
   //查找下一个开始字符的标志位
   StartCodeFound = 0;
   svc_nalu->info2 = 0;
   svc_nalu->info3 = 0;

  while (!StartCodeFound)// && (pos < len))
  {
    //if (feof (bits))//判断是否到了文件尾
    if(pos >= len)
    {
      //nalu->len = (pos-1)-nalu->startcodeprefix_len;
	  nalu->len = (pos)-nalu->startcodeprefix_len;//gxh_201025
      memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);
      nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit
	  nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit
	  nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit
      //free(Buf);
      //return pos-1;
	  return pos;
    }
    //Buf[pos++] = fgetc (bits);//读一个字节到BUF中
	pos++;
    svc_nalu->info3 = FindStartCode3(&Buf[pos-4]);//判断是否为0x00000001
    if(svc_nalu->info3 != 1)
      svc_nalu->info2 = FindStartCode2(&Buf[pos-3]);//判断是否为0x000001
    StartCodeFound = (svc_nalu->info2 == 1 || svc_nalu->info3 == 1);
  }
  // Here, we have found another start code (and read length of startcode bytes more than we should
  // have.  Hence, go back in the file
  rewind = (svc_nalu->info3 == 1)? -4 : -3;

  //if (0 != fseek (bits, rewind, SEEK_CUR))//把文件指针指向前一个NALU的末尾
  if((pos-rewind) != len)
  {
    //free(Buf);
	//printf("GetAnnexbNALU: Cannot fseek in the bit stream file");
  }
  // Here the Start code, the complete NALU, and the next start code is in the Buf.
  // The size of Buf is pos, pos+rewind are the number of bytes excluding the next
  // start code, and (pos+rewind)-startcodeprefix_len is the size of the NALU excluding the start code

  nalu->len = (pos+rewind)-nalu->startcodeprefix_len;
  memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);//拷贝一个完整NALU，不拷贝起始前缀0x000001或0x00000001
  nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit
  nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit
  nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit
  //free(Buf);
  return (pos+rewind);//返回两个开始字符之间间隔的字节数，即包含有前缀的NALU的长度
}
int SplitNal(SVCNalu *svc_nalu, int nal_mem_num)
{
	int i = 0, offset = 0;
	while(offset < svc_nalu->size)
	{
		//printf("SplitNal: offset= %d \n", offset);
		offset += GetAnnexbNALU(svc_nalu, i, offset);
		i++;
		if(i >= nal_mem_num)
		{
		    printf("SplitNal: i= %d \n", i);
		    printf("SplitNal: offset= %d \n", offset);
		    printf("SplitNal: svc_nalu->size= %d \n", svc_nalu->size);
		}
	}
	return i;
}

int data2nalus(void *hnd, char *inBuf)
{
    RtpObj *obj = (RtpObj *)hnd;
    cJSON *json = obj->json;
    SVCNalu *svc_nalu = &obj->svc_nalu;
    int size = GetvalueInt(json, "insize");
    int mtu_size = GetvalueInt(json, "mtu_size");
    mtu_size = mtu_size ? mtu_size : FIX_MTU_SIZE;
#if 0
    if(!test_flag0)
    {
        memcpy(test_data, inBuf, size);
        test_flag0 = 1;
    }
#endif
    //printf("data2nalus: start:size= %d \n", size);
    int k = (int)(size / mtu_size);

    k = k < 4 ? 4 : k;//test

    int nal_mem_num = ((k ? k : 1) << 1) + 3;//3:sps/pps/sei
    nal_mem_num <<= 1;//预设冗余
    //printf("data2nalus: start 0 \n");
    obj->rtpSize = calloc(1, sizeof(short) * nal_mem_num);
    svc_nalu->nal_mem_num = nal_mem_num;
    svc_nalu->info2 = 0;
    svc_nalu->info3 = 0;
    svc_nalu->timestamp = obj->time_stamp;
    svc_nalu->size = size;
    svc_nalu->buf = inBuf;
    svc_nalu->nal = calloc(1, sizeof(NALU_t) * nal_mem_num);
    //printf("data2nalus: svc_nalu->timestamp= %d  \n", svc_nalu->timestamp);
    //printf("data2nalus: nal_mem_num= %d  \n", nal_mem_num);
    for(int i = 0; i < nal_mem_num; i++)
	{
		//nal[I].buf = new char [mtu_size+100];
		svc_nalu->nal[i].buf = calloc(1, sizeof(char) * size);
	}
	//printf("data2nalus: start 2 \n");
    int ret = SplitNal(svc_nalu, nal_mem_num);
    //printf("data2nalus: ret= %d  \n", ret);
    svc_nalu->nal_num = ret;
    if(ret > nal_mem_num)
    {
        printf("error: data2nalus: ret= %d, nal_mem_num=%d \n", ret, nal_mem_num);
    }
    //printf("data2nalus: ret= %d \n", ret);

    return ret;
}
int Rtp2stapA(uint8_t* dataPtr, short *rtpSize, int rtpNum, unsigned char *dst)
{
    int ret = 0;
    int offset = 0;
    int offset2 = 0;
    //printf("Rtp2stapA: rtpNum= %d \n", rtpNum);
    for(int i = 0; i < rtpNum; i++)
    {
        //printf("Rtp2stapA: i= %d \n", i);
        //printf("Rtp2stapA: rtpSize[i]= %d \n", rtpSize[i]);
        RTP_FIXED_HEADER* rtp_hdr = (RTP_FIXED_HEADER*)&dataPtr[offset];
        if(rtp_hdr->version == 2)
        {
            int heardSize = sizeof(RTP_FIXED_HEADER);//1;
            if(rtp_hdr->extension)
            {
                EXTEND_HEADER *rtp_ext = (EXTEND_HEADER *)&dataPtr[offset + heardSize];
                //memset(rtp_ext, 0, sizeof(EXTEND_HEADER));
                rtp_ext->rtp_extend_profile = EXTEND_PROFILE_ID;
                int rtp_extend_length = rtp_ext->rtp_extend_length;
                rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
                //printf("IsRtpData: rtp_extend_length= %d \n", rtp_extend_length);
                rtp_extend_length = (rtp_extend_length + 1) << 2;
                //printf("IsRtpData: 2: rtp_extend_length= %d \n", rtp_extend_length);
                heardSize = sizeof(RTP_FIXED_HEADER) + rtp_extend_length;
            }
            NALU_HEADER* nalu_hdr = (NALU_HEADER*)&dataPtr[offset + heardSize + 0];
            //printf("Rtp2stapA: nalu_hdr->TYPE= %d \n", nalu_hdr->TYPE);
            if((nalu_hdr->TYPE == 7) || (nalu_hdr->TYPE == 8))// || (nalu_hdr->TYPE == 6))
            {
                if(!offset2)
                {
                    //多覆盖了3个字节
                    memmove(&dst[offset2], &dataPtr[offset], heardSize);
                    offset2 += heardSize;
                    dst[offset2] = 24;//NAL HDR
                    offset2 += 1;
                }
                unsigned short *nal_size = (unsigned short *)&dst[offset2];//NALU Size = NALU HDR + NALU DATA，是头字节长度加负载数据长度
                int rawSize = rtpSize[i] - heardSize;
                //printf("Rtp2stapA: rawSize= %d \n", rawSize);
                nal_size[0] = (rawSize >> 8) | ((rawSize & 0xFF) << 8);
                memmove(&dst[offset2 + 2], &dataPtr[offset + heardSize], rawSize);//NALU DATA
                offset2 += 2 + rawSize;
            }
        }
        offset += rtpSize[i];
    }
    ret = offset2;
    if(ret > 0)
    {
        RTP_FIXED_HEADER* rtp_hdr = (RTP_FIXED_HEADER*)&dst[0];
        int heardSize = sizeof(RTP_FIXED_HEADER);
        EXTEND_HEADER *rtp_ext = (EXTEND_HEADER *)&dst[heardSize];
        rtp_ext->rtp_pkt_size = ret;
    }
    return ret;
}
int get_important_packet(RtpObj *obj, char *outBuf, short *rtpSize, uint16_t start_seqnum, uint16_t *idx)
{
    //int ret = 0;
    SVCNalu *svc_nalu = &obj->svc_nalu;
    cJSON *json = obj->json;
    uint16_t *seq_num = &obj->seq_num;
    RTP_FIXED_HEADER        *rtp_hdr    = NULL;
	NALU_HEADER		        *nalu_hdr   = NULL;
	FU_INDICATOR	        *fu_ind     = NULL;
	FU_HEADER		        *fu_hdr     = NULL;
	EXTEND_HEADER	        *rtp_ext    = NULL;
	char *sendptr = NULL;
	char* nalu_payload = NULL;
	int rtp_extend_length = (sizeof(EXTEND_HEADER) >> 2) - 1;//rtp_extend_profile,rtp_extend_length之外1个字（4个字节）
    int mtu_size = GetvalueInt(json, "mtu_size");
    int ref_idx = GetvalueInt(json, "ref_idx");
    int refs = GetvalueInt(json, "refs");
    int res_idx = GetvalueInt(json, "res_idx");
    int res_num = GetvalueInt(json, "res_num");
    int main_spatial_idx = GetvalueInt(json, "main_spatial_idx");
    uint32_t ssrc = (uint32_t)svc_nalu->ssrc;//GetvalueInt(json, "ssrc");
    int time_offset = GetvalueInt(json, "time_offset");
    int selfChanId = GetvalueInt(json, "selfChanId");
    int chanId = GetvalueInt(json, "chanId");
    int loss_rate = GetvalueInt(json, "loss_rate");
    int st0 = GetvalueInt(json, "st0");
    int rt0 = GetvalueInt(json, "rt0");
    int st1 = GetvalueInt(json, "st1");
    int rt1 = GetvalueInt(json, "rt1");
    int nal_num = svc_nalu->nal_num;
    //int slice_type = 0;
    int offset = 0;
    int i = 0;
    //int idx = 0;
    int extlen = 0;
    int bytes = 0;
    mtu_size = mtu_size ? mtu_size : FIX_MTU_SIZE;

    //printf("0: %d \n",sizeof(FEC_HEADER));
#if 0
    //printf("0: %d \n",sizeof(NACK_TEST));
    int64_t test_time = (int64_t)get_time_stamp();
    uint64_t test_t0 = test_time & 0xFFFFFFFF;
    uint64_t test_t1 = (test_time >> 32) & 0xFFFFFFFF;
    int64_t test_time2 = test_t0 | (test_t1 << 32);
    if(test_time != test_time2)
    {
        //printf("test_time=%lld \n",test_time);
        //printf("test_time2=%lld \n",test_time2);
        printf("test_time=%x \n",test_time);
        printf("test_time2=%x \n",test_time2);
        printf("test_t1=%x \n",test_t1);
    }
#endif
    while(i < nal_num)
    {
        NALU_t *n = &svc_nalu->nal[i];
		if ( (n->nal_unit_type == 7) || (n->nal_unit_type == 8) || (n->nal_unit_type == 6))
		{
			if (n->nal_unit_type == 7)
			{
				//printf("enc IDR = %d \n", IDRCnt);
				//LogOut(format1, (void *)&IDRCnt);
				//printf("get_important_packet: n->len = %d \n", n->len);
			}
            //is_key_frame = true;
			sendptr = &outBuf[offset];
			//rtp固定包头，为12字节,该句将sendbuf[0]的地址赋给rtp_hdr，以后对rtp_hdr的写入操作将直接写入sendbuf。
			rtp_hdr =(RTP_FIXED_HEADER*)&sendptr[0];
			//设置RTP HEADER￡
			rtp_hdr->version     = 2;  //版本号，此版本固定为2								V
			rtp_hdr->padding	 = 0;//														P
			rtp_hdr->csrc_len	 = 0;//														CC
			rtp_hdr->extension	 = 1;//														X
			rtp_hdr->payload     = H264_PLT;  //负载类型号
			rtp_hdr->marker		 = 0;//(size >= len);   //标志位，由具体协议规定其值。		M
			rtp_hdr->ssrc        = ssrc;//(unsigned int)svc_nalu;//htonl(10);    //随机指定为10，并且在本RTP会话中全局唯一	SSRC

			rtp_hdr->seq_no		 = (*seq_num);///htons(seq_num ++); //序列号，每发送一个RTP包增1	SQUENCE NUMBER
			//(*seq_num)不存在(*seq_num) > MAX_USHORT,除非改成int類型
			if ((*seq_num) >= MAX_USHORT)
			{
				*seq_num = 0;
			}
			else{
			    (*seq_num)++;
			}
			//printf("sps pps seqno=%d \n",rtp_hdr->seq_no);
			rtp_hdr->timestamp = svc_nalu->timestamp;//htonl(ts_current);
			//printf("get_important_packet: rtp_hdr->timestamp= %d  \n", rtp_hdr->timestamp);
			if(rtp_hdr->extension)
			{
				rtp_ext = (EXTEND_HEADER *)&sendptr[sizeof(RTP_FIXED_HEADER)];
				memset(rtp_ext, 0, sizeof(EXTEND_HEADER));
				rtp_ext->rtp_extend_profile = EXTEND_PROFILE_ID;//0;
				extlen = (rtp_extend_length + 1) << 2;
				rtp_ext->rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
				//
                rtp_ext->rtp_pkt_num = nal_num;
                rtp_ext->start_seqnum = start_seqnum;
                rtp_ext->nack_type = 0;
				rtp_ext->refs = refs & 0x1F;
				rtp_ext->ref_idx = ref_idx & 0x1F;
				if(refs == 1)
				{
				    rtp_ext->ref_idx = obj->frame_idx & 0x1F;
				}
				int ref_idc = ref_idx == 0 ? 0 : ref_idx == refs ? 1 : ref_idx == 1 ? 3 : 2;

				rtp_ext->ref_idc = ref_idc & 0x3;
				rtp_ext->res_num = res_num;//1;
				rtp_ext->res_idx = res_idx;//0;
				rtp_ext->main_spatial_idx = main_spatial_idx;
				rtp_ext->mult_spatial = 0;//res_num > 0;//默认只有一层给解码器
				rtp_ext->qua_num = 0;
				rtp_ext->qua_idx = 0;
				rtp_ext->nal_type = n->nal_unit_type;
				rtp_ext->enable_fec = 0;//svc_nalu->enable_fec;
				rtp_ext->refresh_idr = svc_nalu->refresh_idr;
				//
				int64_t now_time = (int64_t)get_time_stamp();
				rtp_ext->nack.nack0.time_info.time_stamp0 = now_time & 0xFFFFFFFF;
			    rtp_ext->nack.nack0.time_info.time_stamp1 = (now_time >> 32) & 0xFFFFFFFF;
				//rtp_ext->nack.nack0.time_info.time_stamp = now_time;
				rtp_ext->nack.nack0.is_lost_packet = 0;
				rtp_ext->nack.nack0.loss_rate = 0;
				//rtp_ext->nack.nack0.pkt_num = nal_num;
				//if(!time_offset)
				{
				    rtp_ext->nack.nack0.enable_nack = 0;
				    rtp_ext->nack.nack0.time_offset = time_offset;
                    if(selfChanId > 0)
                    {
                        rtp_ext->nack.nack0.enable_nack = 1;
                        rtp_ext->nack.nack0.time_info.rtt_list.rtt0.st0 = now_time & 0xFFFF;
				        rtp_ext->nack.nack0.time_info.rtt_list.rtt0.rt0 = 0;
				        rtp_ext->nack.nack0.time_info.rtt_list.rtt0.st1 = 0;
				        rtp_ext->nack.nack0.time_info.rtt_list.rtt0.rt1 = 0;
				        rtp_ext->nack.nack0.chanId = selfChanId;
				        rtp_ext->nack.nack0.loss_rate = 0;
				        rtp_ext->nack.nack0.info_status = 0;
                    }

				}
				//
				//int picType = n->nal_reference_idc >> 5;
				//rtp_ext->is_b_slice = picType == 3 || picType == 2 || is_key_frame ? false : true;
				//rtp_ext->pic_type = n->nal_reference_idc >> 5;

			}
			extlen = rtp_hdr->extension ? extlen : 0;
			//
			nalu_hdr =(NALU_HEADER*)&sendptr[sizeof(RTP_FIXED_HEADER) + extlen];//sendbuf[RTPHEADSIZE + offset]; //将sendbuf[12]的地址赋给nalu_hdr，之后对nalu_hdr的写入就将写入sendbuf中；
#if 1
			//n->forbidden_bit = 0;//20210113
			nalu_hdr->F = n->forbidden_bit;
			//if (n->forbidden_bit) printf("n->forbidden_bit \n", n->forbidden_bit);
			nalu_hdr->NRI = n->nal_reference_idc>>5;//有效数据在n->nal_reference_idc的第6，7位，需要右移5位才能将其值赋给nalu_hdr->NRI。
			nalu_hdr->TYPE = n->nal_unit_type;

			nalu_payload = &sendptr[sizeof(RTP_FIXED_HEADER) + extlen + 1];//sendbuf[13 + offset];//同理将sendbuf[13]赋给nalu_payload
			memcpy(nalu_payload,n->buf + 1,n->len-1);//去掉nalu头的nalu剩余内容写入sendbuf[13]开始的字符串。
#else
			memcpy(nalu_hdr, n->buf, n->len);
#endif
			bytes = n->len + sizeof(RTP_FIXED_HEADER) + extlen;						//获得sendbuf的长度,为nalu的长度（包含NALU头但除去起始前缀）加上rtp_header的固定长度12字节
			offset += bytes;
			rtpSize[(*idx)++] = bytes;
            if(rtp_hdr->extension)
            {
                rtp_ext->rtp_pkt_size = bytes;
            }
		}
		else
		{
			//slice_type = n->nal_reference_idc >> 5;
			//slice_type = slice_type == 3 ? AV_PICTURE_TYPE_I : slice_type == 2 ? AV_PICTURE_TYPE_P : AV_PICTURE_TYPE_B;
		}
		i++;
    }
    return offset;
}
int get_slice_packet(RtpObj *obj, char *outBuf, short *rtpSize, uint16_t start_seqnum, uint16_t *idx, int offset)
{
    int ret = 0;
    SVCNalu *svc_nalu = &obj->svc_nalu;
    cJSON *json = obj->json;
    uint16_t *seq_num = &obj->seq_num;
    RTP_FIXED_HEADER        *rtp_hdr    = NULL;
	NALU_HEADER		        *nalu_hdr   = NULL;
	FU_INDICATOR	        *fu_ind     = NULL;
	FU_HEADER		        *fu_hdr     = NULL;
	EXTEND_HEADER	        *rtp_ext    = NULL;
	char *sendptr = NULL;
	char* nalu_payload = NULL;
	int rtp_extend_length = (sizeof(EXTEND_HEADER) >> 2) - 1;//rtp_extend_profile,rtp_extend_length之外1个字（4个字节）
	//printf("get_slice_packet: sizeof(EXTEND_HEADER)= %d \n", sizeof(EXTEND_HEADER));
    int mtu_size = GetvalueInt(json, "mtu_size");
    int ref_idx = GetvalueInt(json, "ref_idx");
    int refs = GetvalueInt(json, "refs");
    int res_idx = GetvalueInt(json, "res_idx");
    int res_num = GetvalueInt(json, "res_num");
    int main_spatial_idx = GetvalueInt(json, "main_spatial_idx");
    uint32_t ssrc = svc_nalu->ssrc;//GetvalueInt(json, "ssrc");
    //printf("get_slice_packet: ssrc = %u \n", ssrc);
    //if(!ssrc)
    //{
    //    ssrc = (unsigned int)svc_nalu;
    //}
    int time_offset = GetvalueInt(json, "time_offset");
    int selfChanId = GetvalueInt(json, "selfChanId");
    int chanId = GetvalueInt(json, "chanId");//已增1
    //int loss_rate = GetvalueInt(json, "loss_rate");
    long long *objList = NULL;
    long long *objList1 = NULL;
    long long *objList2 = NULL;
    int arraySize = 0;
    int arraySize1 = 0;
    int arraySize2 = 0;
    int netIdx = 0;
    int netIdx1 = 0;
    int netIdx2 = 0;
    int maxNetNum = 3;
    int netStep = 1;
    int nal_num = svc_nalu->nal_num;
    //int slice_type = 0;
    //int offset = 0;
    int i = 0, I = 0;
    //int idx = 0;
    int extlen = 0;
    int bytes = 0;
    int firstSlice = 0;
	int nextFirstSlice = 0;

    if(selfChanId > 0)
    {
        //控制每一帧的net信息个数,注意调节大丢包率下的平衡
        if(nal_num < maxNetNum)
        {
            netStep = 1;
        }
        else
        {
            netStep = nal_num / maxNetNum;
        }
        //printf("get_slice_packet: netStep = %d \n", netStep);
        objList = GetArrayObj(json, "netInfo", &arraySize);
        objList1 = GetArrayObj(json, "rtxInfo", &arraySize1);
        objList2 = GetArrayObj(json, "rttInfo", &arraySize2);
        //printf("get_slice_packet: arraySize= %d\n",arraySize);
        //printf("get_slice_packet: objList1= %x \n",objList1);
        //printf("get_slice_packet: arraySize2= %d\n",arraySize2);
        //maxNetNum = arraySize ? arraySize > arraySize2 : arraySize2;
        maxNetNum = (arraySize + arraySize1 + arraySize2);
        maxNetNum += 1;
    }
	mtu_size = mtu_size ? mtu_size : FIX_MTU_SIZE;
    while(i < nal_num)
    {
        NALU_t *n = &svc_nalu->nal[i];
        //注意兼顾与sps/pps在一起和不在一起的情况；

		if(n->startcodeprefix_len == 4)
		{
			firstSlice = 1;
		}
		if(svc_nalu->nal[i + 1].startcodeprefix_len == 4 || ((i + 1) == nal_num))
		{
			nextFirstSlice = 1;
		}
		//printf("get_slice_packet: n->nal_unit_type=%d, i=%d \n",n->nal_unit_type, i);
		if ( (n->nal_unit_type != 7) && (n->nal_unit_type != 8) && (n->nal_unit_type != 6))
		{
            rtp_hdr =(RTP_FIXED_HEADER*)&outBuf[offset];
			rtp_hdr->payload     = H264_PLT;  //负载类型号，									PT
			rtp_hdr->version     = 2;  //版本号，此版本固定为2								V
			rtp_hdr->padding	 = 0;//														P
			rtp_hdr->csrc_len	 = 0;//														CC
			rtp_hdr->marker		 = 0;//(size >= len);   //标志位，由具体协议规定其值。		M
			rtp_hdr->ssrc        = ssrc;//(unsigned int)svc_nalu;;//htonl(10);    //随机指定为10，并且在本RTP会话中全局唯一	SSRC
			rtp_hdr->extension	 = 1;//														X
			rtp_hdr->timestamp = svc_nalu->timestamp;///htonl(ts_current);
			sendptr = &outBuf[sizeof(RTP_FIXED_HEADER) + offset];
			rtp_hdr->seq_no = (*seq_num);///htons(seq_num ++); //序列号，每发送一个RTP包增1
			if ((*seq_num) >= MAX_USHORT)
			{
				*seq_num = 0;
			}
			else{
			    (*seq_num)++;
			}
			if (rtp_hdr->extension)
			{
				rtp_ext = (EXTEND_HEADER *)&sendptr[0];
				memset(rtp_ext, 0, sizeof(EXTEND_HEADER));
				rtp_ext->rtp_extend_profile = EXTEND_PROFILE_ID;//0;
				extlen = (rtp_extend_length + 1) << 2;
				//printf("get_slice_packet: extlen= %d \n", extlen);
				rtp_ext->rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
				//
				rtp_ext->rtp_pkt_num = nal_num;
				rtp_ext->start_seqnum = start_seqnum;
				rtp_ext->nack_type = 0;
				rtp_ext->refs = refs & 0x1F;
				rtp_ext->ref_idx = ref_idx & 0x1F;
				if(refs == 1)
				{
				    rtp_ext->ref_idx = obj->frame_idx & 0x1F;
				}
				int ref_idc = ref_idx == 0 ? 0 : ref_idx == refs ? 1 : ref_idx == 1 ? 3 : 2;
#if 1//test_gxh_20210113
				int picType = n->nal_reference_idc >> 5;
				if(refs == 1)
    			{
	    		    int picType = n->nal_reference_idc >> 5;
		    	    if(picType == 3)//I
			        {
				        ref_idc = 0;
				    }
    			    else if(picType == 2)//P
	    		    {
		    	        ref_idc = 1;
			        }
				    else if(picType == 3)//B
    			    {
	    		        ref_idc = 2;
		    	    }
			    }
				else if(refs == 2)
    			{
	    		    if(ref_idc == 0)
		    	    {
			            n->nal_reference_idc = 3 << 5;
				    }
    			    else if(ref_idc == 1)
	    		    {
		    	        n->nal_reference_idc = 2 << 5;
			        }
				    else{
				        n->nal_reference_idc = 1 << 5;
    			    }
	    		}
		    	else
			    {
				    //if(ref_idc == 0)
				    if(ref_idc == 0 || ref_idc == 1)
				    {
    			        n->nal_reference_idc = 3 << 5;
	    		    }
		    	    //else if(ref_idc == 1 || ref_idc == 2)
		    	    else if(ref_idc == 2)
			        {
				        n->nal_reference_idc = 2 << 5;
				    }
    			    else if(ref_idc == 3)
	    		    {
		    	        n->nal_reference_idc = 1 << 5;
			        }
			    }
#endif
                //if((n->nal_reference_idc >> 5) == 2)
                //{
                //    n->nal_reference_idc = (1 << 5);
                //    //n->nal_reference_idc = 0;//导致解码错误
                //}
                //n->nal_reference_idc = 0;
				//printf("get_slice_packet: picType= %d \n", picType);

				rtp_ext->ref_idc = ref_idc & 0x3;
				rtp_ext->res_num = res_num;//1;
				rtp_ext->res_idx = res_idx;//0;
				rtp_ext->main_spatial_idx = main_spatial_idx;
				rtp_ext->mult_spatial = 0;//res_num > 0;//默认只有一层给解码器
				rtp_ext->qua_num = 0;
				rtp_ext->qua_idx = 0;
				rtp_ext->first_slice = firstSlice;
				rtp_ext->nal_type = n->nal_unit_type;
				rtp_ext->enable_fec = 0;//svc_nalu->enable_fec;
				rtp_ext->refresh_idr = svc_nalu->refresh_idr;

				//
				rtp_ext->nack.nack0.enable_nack = 0;
				rtp_ext->nack.nack0.is_lost_packet = 0;
				rtp_ext->nack.nack0.loss_rate = 0;
				rtp_ext->nack.nack0.time_offset = time_offset;
				int64_t now_time = (int64_t)get_time_stamp();
				rtp_ext->nack.nack0.time_info.time_stamp0 = now_time & 0xFFFFFFFF;
			    rtp_ext->nack.nack0.time_info.time_stamp1 = (now_time >> 32) & 0xFFFFFFFF;
				//rtp_ext->nack.nack0.time_info.time_stamp = now_time;
				RTT_HEADER *rtt_list = (RTT_HEADER *)&rtp_ext->nack.nack0.time_info.rtt_list;
				//rtp_ext->nack.nack0.pkt_num = nal_num;
				//printf("get_slice_packet: selfChanId=%d, I=%d, maxNetNum=%d \n",selfChanId, I, maxNetNum);
				//printf("get_slice_packet: netIdx=%d, arraySize=%d \n",netIdx, arraySize);
				//printf("get_slice_packet: netIdx2=%d, arraySize2=%d \n",netIdx2, arraySize2);
				if(selfChanId > 0 && I < maxNetNum)
				{
				    //if((i % netStep) == 0)
				    if(!I)
    				{
    				    if(ref_idc < 2)
    				    {
    				        //信息初建，原创
    				        rtp_ext->nack_type = 1;
	    			        rtp_ext->nack.nack0.enable_nack = 1;
		    		        rtt_list->rtt0.st0 = now_time & 0xFFFF;
			    	        rtt_list->rtt0.rt0 = 0;
				            rtt_list->rtt0.st1 = 0;
				            rtt_list->rtt0.rt1 = 0;
    				        rtp_ext->nack.nack0.chanId = selfChanId;//原始信息创建者（本端编码器）
    				        rtp_ext->nack.nack0.res_idx = res_idx;
	    			        rtp_ext->nack.nack0.loss_rate = 0;
		    		        rtp_ext->nack.nack0.info_status = 0;
		    		        //printf("get_slice_packet: rtp_ext->nack.nack0.enable_nack= %d \n", rtp_ext->nack.nack0.enable_nack);
#if 0
		    		        //节省bit数，进行重用
		    		        if(objList2 && (netIdx2 < arraySize2))
    				        {
    				            cJSON *thisjson = (cJSON *)objList2[netIdx2++];
                                if(thisjson)
                                {
                                    int decodeId = GetvalueInt(thisjson, "decodeId");
                                    int rtt = GetvalueInt(thisjson, "rtt");
                                    rtt_list->rtt1.rtt = (rtt + 7) >> 3;
		    		                rtt_list->rtt1.decodeId = decodeId;
                                }
    				        }
#endif
    				    }

			    	}
			    	else if(objList1 && I <= arraySize1)
			    	{
			    	    cJSON *thisjson = (cJSON *)objList1[netIdx1++];
			    	    if(thisjson)
                        {
                            MYPRINT("get_slice_packet: arraySize1=%d \n",arraySize1);
                            rtp_ext->nack_type = 2;
                            rtp_ext->nack.nack1.send_time = (uint32_t)GetvalueInt(thisjson, "send_time");
                            rtp_ext->nack.nack1.start_seqnum = GetvalueInt(thisjson, "start_seqnum");
                            rtp_ext->nack.nack1.chanId = GetvalueInt(thisjson, "chanId");
                            MYPRINT("get_slice_packet: rtp_ext->nack.nack1.start_seqnum=%d \n",rtp_ext->nack.nack1.start_seqnum);
                            MYPRINT("get_slice_packet: rtp_ext->nack.nack1.chanId=%d \n",rtp_ext->nack.nack1.chanId);
                            rtp_ext->nack.nack1.loss_type = (uint8_t)GetvalueInt(thisjson, "loss_type");
                            rtp_ext->nack.nack1.loss0 = (uint8_t)GetvalueInt(thisjson, "loss0");
                            rtp_ext->nack.nack1.loss1 = (uint32_t)GetvalueInt(thisjson, "loss1");
                            rtp_ext->nack.nack1.loss2 = (uint32_t)GetvalueInt(thisjson, "loss2");
                            rtp_ext->nack.nack1.loss3 = (uint32_t)GetvalueInt(thisjson, "loss3");
                            rtp_ext->nack.nack1.loss4 = (uint32_t)GetvalueInt(thisjson, "loss4");
                        }
			    	}
				    else{
				        //接收端收到了对端的时间信息
    				    if(objList && netIdx < arraySize)
	    			    {
	    			        //信息复述，转发
	    			        //printf("get_slice_packet: arraySize= %d \n", arraySize);
		    		        cJSON *thisjson = (cJSON *)objList[netIdx++];
                            if(thisjson)
                            {
                                int chanId = GetvalueInt(thisjson, "chanId");//已增1，产生信息者，且是反馈的终极接收者
                                int loss_rate = GetvalueInt(thisjson, "loss_rate");
                                int st0 = GetvalueInt(thisjson, "st0");
                                int rt0 = GetvalueInt(thisjson, "rt0");
                                int layerId = GetvalueInt(thisjson, "res_idx");
                                //int info_status = GetvalueInt(thisjson, "info_status");
                                //int st1 = GetvalueInt(thisjson, "st1");
                                //int rt1 = GetvalueInt(thisjson, "rt1");

                                //printf("get_slice_packet: rt0= %d\n",rt0);
                                //printf("%d, %d, %d, %d, %d, %d \n", chanId, loss_rate, st0, rt0, st1, rt1);
                                rtp_ext->nack_type = 1;
				                rtp_ext->nack.nack0.enable_nack = 1;
				                rtp_ext->nack.nack0.loss_rate = loss_rate;//loss_rate in [1,101]//此刻写入的是谁的loss_rate?
				                rtt_list->rtt0.st0 = st0;
    				            rtt_list->rtt0.rt0 = rt0;
	    			            rtt_list->rtt0.st1 = now_time & 0xFFFF;
	    			            rtt_list->rtt0.rt1 = 0;//复用为(rtt,decodeId)
	    			            //printf("get_slice_packet: chanId= %d\n",chanId);
		    		            rtp_ext->nack.nack0.chanId = chanId;//原始信息创建者（异端编码器），且是未终结的信息
		    		            rtp_ext->nack.nack0.res_idx = layerId;
			    	            rtp_ext->nack.nack0.info_status = 1;//info_status == 1
				                //printf("api_test_write_str: rtp_ext->nack.nack0.info_status= %d\n",rtp_ext->nack.nack0.info_status);
				                //节省bit数，进行重用
				                if(objList2 && (netIdx2 < arraySize2))
    				            {
    				                cJSON *thisjson = (cJSON *)objList2[netIdx2++];
                                    if(thisjson)
                                    {
                                        //此信息与rtt0(chanId创建）的信息是不相关的
                                        //decodeId可能与selfchanId及chanId均不同
                                        int decodeId = GetvalueInt(thisjson, "decodeId");//终极信息，创建id与接收id相同，且为对端
                                        int rtt = GetvalueInt(thisjson, "rtt");
                                        rtt_list->rtt1.rtt = (rtt + 7) >> 3;
		    		                    rtt_list->rtt1.decodeId = decodeId;
		    		                    //printf("get_slice_packet: decodeId= %d\n",decodeId);
		    		                    //printf("get_slice_packet: rtp_ext->nack.nack0.info_status= %d\n",rtp_ext->nack.nack0.info_status);
                                    }
    				            }
				            }
				        }
				    }
				}
			}
			//n->forbidden_bit = 0;//20210113
			//if(firstSlice && nextFirstSlice)
			if(true)
			{
				extlen = rtp_hdr->extension ? extlen : 0;
				//rtp_ext->rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
				nalu_hdr =(NALU_HEADER*)&sendptr[extlen];//&sendbuf[RTPHEADSIZE + offset]; //将sendbuf[12]的地址赋给nalu_hdr，之后对nalu_hdr的写入就将写入sendbuf中；
#if 1
				nalu_hdr->F = n->forbidden_bit;
				//if (n->forbidden_bit) printf("n->forbidden_bit \n", n->forbidden_bit);
				nalu_hdr->NRI = n->nal_reference_idc>>5;//有效数据在n->nal_reference_idc的第6，7位，需要右移5位才能将其值赋给nalu_hdr->NRI。
				nalu_hdr->TYPE = n->nal_unit_type;
				nalu_payload=&sendptr[extlen + 1];//sendbuf[13 + offset];//同理将sendbuf[13]赋给nalu_payload
				memcpy(nalu_payload,n->buf+1,n->len-1);//去掉nalu头的nalu剩余内容写入sendbuf[13]开始的字符串。
#else
                memcpy(nalu_hdr, n->buf, n->len);
#endif
                //printf("get_slice_packet: nalu_hdr->TYPE = %d \n", nalu_hdr->TYPE);

				///ts_current=ts_current+timestamp_increse;
				rtp_hdr->timestamp = svc_nalu->timestamp;//htonl(ts_current);										TIMESTAMP
				bytes = n->len + sizeof(RTP_FIXED_HEADER) + extlen;						//获得sendbuf的长度,为nalu的长度（包含NALU头但除去起始前缀）加上rtp_header的固定长度12字节

				offset += bytes;//send( socket1, sendbuf, bytes, 0 );//发送rtp包
				if(rtp_hdr->extension)
				{
				    rtp_ext->rtp_pkt_size = bytes;
				}

				firstSlice = 0;
				nextFirstSlice = 0;
			}
			else if(firstSlice && !nextFirstSlice)//发送一个需要分片的NALU的第一个分片，置FU HEADER的S位
			{
				extlen = rtp_hdr->extension ? extlen : 0;
				//rtp_ext->rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));

				fu_ind =(FU_INDICATOR*)&sendptr[extlen];//sendbuf[RTPHEADSIZE + VIDIORASHEADSIZE + offset]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
				fu_ind->F = n->forbidden_bit;
				//if (n->forbidden_bit) printf("n->forbidden_bit \n", n->forbidden_bit);
				fu_ind->NRI = n->nal_reference_idc>>5;
				fu_ind->TYPE = 28;
                if (rtp_hdr->extension)
			    {
			        rtp_ext->nal_type = 28;
			    }
				//设置FU HEADER,并将这个HEADER填入sendbuf[13]
				fu_hdr = (FU_HEADER*)&sendptr[extlen + 1];//sendbuf[13  + VIDIORASHEADSIZE + offset];
				fu_hdr->E = 0;//1;/// 0;
				fu_hdr->R = 0;
				fu_hdr->S = 1;
				fu_hdr->TYPE=n->nal_unit_type;

				nalu_payload=&sendptr[extlen + 2];//sendbuf[14 + VIDIORASHEADSIZE + offset];//同理将sendbuf[14]赋给nalu_payload
				memcpy(nalu_payload,n->buf + 1,n->len-1);//去掉NALU头

				bytes=n->len + sizeof(RTP_FIXED_HEADER) + extlen + 1;//获得sendbuf的长度,为nalu的长度（除去起始前缀和NALU头）加上rtp_header，fu_ind，fu_hdr的固定长度14字节

				offset += bytes;
				if(rtp_hdr->extension)
					rtp_ext->rtp_pkt_size = bytes;
				//send( socket1, sendbuf, bytes, 0 );//发送rtp包
				firstSlice = 0;

			}
			else if(nextFirstSlice && !firstSlice)//发送的是最后一个分片，注意最后一个分片的长度可能超过1400字节（当l>1386时）。
			{
				extlen = rtp_hdr->extension ? extlen : 0;
				//rtp_ext->rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
				fu_ind =(FU_INDICATOR*)&sendptr[extlen];//&sendbuf[RTPHEADSIZE + offset]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
				fu_ind->F=n->forbidden_bit;
				//if (n->forbidden_bit) printf("n->forbidden_bit \n", n->forbidden_bit);
				fu_ind->NRI=n->nal_reference_idc>>5;
				fu_ind->TYPE=28;
                if (rtp_hdr->extension)
			    {
			        rtp_ext->nal_type = 28;
			    }
				//设置FU HEADER,并将这个HEADER填入sendbuf[13]
				fu_hdr =(FU_HEADER*)&sendptr[extlen + 1];//sendbuf[13 + offset];
				fu_hdr->R = 0;
				fu_hdr->S = 0;//1;/// 0;
				fu_hdr->TYPE=n->nal_unit_type;
				fu_hdr->E = 1;

				nalu_payload=&sendptr[2 + extlen];//sendbuf[14 + offset];//同理将sendbuf[14]的地址赋给nalu_payload
				memcpy(nalu_payload,n->buf + 1,n->len-1);//将nalu最后剩余的l-1(去掉了一个字节的NALU头)字节内容写入sendbuf[14]开始的字符串。
				bytes=n->len + sizeof(RTP_FIXED_HEADER) + extlen + 1;		//获得sendbuf的长度,为剩余nalu的长度l-1加上rtp_header，FU_INDICATOR,FU_HEADER三个包头共14字节

				offset += bytes;
				if(rtp_hdr->extension)
					rtp_ext->rtp_pkt_size = bytes;
				//send( socket1, sendbuf, bytes, 0 );//发送rtp包
				//nextFirstSlice = 0;
			}
			else
			{
				extlen = rtp_hdr->extension ? extlen : 0;
				//rtp_ext->rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
				fu_ind =(FU_INDICATOR*)&sendptr[extlen];//&sendbuf[RTPHEADSIZE + offset]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
				fu_ind->F=n->forbidden_bit;
				//if (n->forbidden_bit) printf("n->forbidden_bit \n", n->forbidden_bit);
				fu_ind->NRI=n->nal_reference_idc>>5;
				fu_ind->TYPE=28;
                if (rtp_hdr->extension)
			    {
			        rtp_ext->nal_type = 28;
			    }
				//设置FU HEADER,并将这个HEADER填入sendbuf[13]
				fu_hdr =(FU_HEADER*)&sendptr[extlen + 1];//sendbuf[13 + offset];
				//fu_hdr->E=0;
				fu_hdr->R = 0;
				fu_hdr->S = 0;//1;/// 0;
				fu_hdr->E = 0;//1;/// 0;
				fu_hdr->TYPE=n->nal_unit_type;

				nalu_payload=&sendptr[2 + extlen];//sendbuf[14 + offset];//同理将sendbuf[14]的地址赋给nalu_payload
				memcpy(nalu_payload,n->buf + 1,n->len-1);//去掉起始前缀的nalu剩余内容写入sendbuf[14]开始的字符串。
				bytes=n->len + sizeof(RTP_FIXED_HEADER) + extlen + 1;			//获得sendbuf的长度,为nalu的长度（除去原NALU头）加上rtp_header，fu_ind，fu_hdr的固定长度14字节

				offset += bytes;
				if(rtp_hdr->extension)
					rtp_ext->rtp_pkt_size = bytes;
				//send( socket1, sendbuf, bytes, 0 );//发送rtp包
			}
			rtpSize[(*idx)++] = bytes;
			I++;
		}
		nextFirstSlice = 0;
		i++;
    }
    if(*idx)
	{
		rtp_hdr->marker = 1;
	}
	ret = *idx;
	//ret = offset;
	if(objList)
	{
	    free(objList);
	}
	if(objList1)
	{
	    free(objList1);
	}
	if(objList2)
	{
	    free(objList2);
	}
    return ret;
}
int raw2rtp_packet(void *hnd, char *outBuf)
{
    int ret = 0;
    RtpObj *obj = (RtpObj *)hnd;
    cJSON *json = obj->json;
    SVCNalu *svc_nalu = &obj->svc_nalu;
    int insize = GetvalueInt(json, "insize");
    //int mtu_size = GetvalueInt(json, "mtu_size");
    int offset = 0;
    uint16_t idx = 0;
    short *rtpSize = obj->rtpSize;
    obj->seq_num = GetvalueInt(json, "seqnum");
    obj->refresh_idr = GetvalueInt(json, "refresh_idr");
    obj->enable_fec = GetvalueInt(json, "enable_fec");
    svc_nalu->refresh_idr = obj->refresh_idr;
    svc_nalu->enable_fec = obj->enable_fec;
    svc_nalu->ssrc = obj->ssrc;
    unsigned short seq_num = obj->seq_num;
    uint16_t start_seqnum = seq_num;
    offset = get_important_packet(obj, outBuf, rtpSize, start_seqnum, &idx);
#if 0//gxh_20210111
    if(offset > 0)
    {
        //printf("raw2rtp_packet: offset= %d \n", offset);
        unsigned char tmpBuf[256] = "";
        offset = Rtp2stapA(outBuf, rtpSize, idx, tmpBuf);
        idx = 1;
        rtpSize[0] = offset;
        if ((seq_num) >= MAX_USHORT)
        {
		    seq_num = 0;
	    }
	    else{
	        seq_num++;
	    }
	    obj->seq_num = seq_num;
	    if(offset > 0)
	    {
	        memcpy(outBuf, tmpBuf, offset);
	    }

	    //printf("raw2rtp_packet: 2: offset= %d \n", offset);
    }
#endif
    ret = get_slice_packet(obj, outBuf, rtpSize, start_seqnum, &idx, offset);
    //
    obj->frame_idx++;
    if(obj->frame_idx >= 16)
    {
        obj->frame_idx = 0;
    }
    return ret;
}
uint32_t create_time_stamp(RtpObj *obj)
{
    uint32_t ret = 0;
    if(obj)
    {
        int64_t now = api_get_time_stamp_ll();
        if(!obj->time0)
        {
            obj->time0 = now;
        }
        int64_t difftime = now - obj->time0;
        int64_t time_stamp = difftime * 90;//90000 / 1000; //90hz

        ret = (uint32_t)(time_stamp % LEFT_SHIFT32);
        ret = ret ? ret : 1;
    }

    return ret;
}
HCSVC_API
int api_raw2rtp_packet(char *handle, char *data, char *param, char *outbuf, char *outparam[])
{
    int ret = 0;
    //printf("api_raw2rtp_packet: param= %s \n", param);
    //initobj(id);
    //if (id < MAX_OBJ_NUM)
    {
        long long *testp = (long long *)handle;
        CodecObj *codecObj = (CodecObj *)testp[0];
        RtpObj *obj = codecObj->rtpObj;//(RtpObj *)&global_rtp_objs[id];
        int id = codecObj->Obj_id;
        if (!obj)
        {
            obj = (RtpObj *)calloc(1, sizeof(RtpObj));
            codecObj->rtpObj = obj;
            ResetObj(obj);
            obj->Obj_id = id;
            printf("api_raw2rtp_packet: param= %s \n", param);
            //
            if(!obj->logfp)
            {
                char filename[256] = "rtp_enc_gxh_";
                char ctmp[32] = "";
                int fileidx = id;//random() % 10000;
                sprintf(ctmp, "%d", fileidx);
                strcat(filename, ctmp);
                strcat(filename, ".txt");
                //obj->logfp = fopen(filename, "w");
            }

        }

        obj->json = (cJSON *)api_str2json(param);

        unsigned int timestamp = (unsigned int)GetvalueInt(obj->json, "timestamp");
        if(timestamp)
        {
            obj->time_stamp = timestamp;//test
            if(!timestamp)
            {
                printf("error: api_raw2rtp_packet: timestamp= %u \n", timestamp);
            }
        }
        else{
            //obj->time_stamp += 40;//test
            obj->time_stamp = create_time_stamp(obj);
        }

        //printf("api_raw2rtp_packet: id= %d \n", id);
        //printf("api_raw2rtp_packet: obj->time_stamp= %u \n", obj->time_stamp);
        int insize = GetvalueInt(obj->json, "insize");
        uint32_t ssrc = GetvalueInt(obj->json, "ssrc");
        if(!ssrc)
        {
            if(!obj->ssrc)
            {
                unsigned int range = MAX_UINT;
                obj->ssrc = (unsigned int)api_create_id(range);
            }
        }
        else{
            obj->ssrc = ssrc;
        }
        //printf("api_raw2rtp_packet: insize= %d \n", insize);
        //printf("api_raw2rtp_packet: mtu_size= %d \n", mtu_size);

        ret = data2nalus(obj, data);

        SVCNalu *svc_nalu = &obj->svc_nalu;
        int nal_num = svc_nalu->nal_num;
        //for(int i = 0; i < nal_num; i++)
	    //{
	    //    int len = svc_nalu->nal[i].len;
	    //    //obj->rtpSize[i] = len;//test
	    //    printf("api_raw2rtp_packet: i= %d, len= %d \n", i, len);
	    //}

	    ret = raw2rtp_packet(obj, outbuf);

        //printf("api_raw2rtp_packet: raw2rtp_packet: ret= %d \n", ret);

        outparam[0] = obj->outparam[0];
        outparam[1] = obj->outparam[1];
        sprintf(obj->outparam[1], "%d", obj->seq_num);

	    //char text[2048] = "";//2048/4=512//512*1400=700kB

	    int sum = 0;
        for (int i = 0; i < ret; i++)
	    {
	        int size = obj->rtpSize[i];
            sum += size;
	    }

	    cJSON *json2 = NULL;
        json2 = renewJsonArray1(json2, "rtpSize", obj->rtpSize, ret);
        char *jsonStr = api_json2str(json2);//比较耗时
        api_json_free(json2);
        strcpy(obj->outparam[0], jsonStr);
        api_json2str_free(jsonStr);
        //printf("api_raw2rtp_packet: obj->outparam[0]= %s \n", obj->outparam[0]);
        ret = sum;

	    //free
        int nal_mem_num = svc_nalu->nal_mem_num;
        if(!nal_mem_num)
        {
            printf("error: api_raw2rtp_packet: nal_mem_num= %d \n", nal_mem_num);
        }
        for(int i = 0; i < nal_mem_num; i++)
	    {
		    free(svc_nalu->nal[i].buf);
	    }
	    if(svc_nalu->nal)
	    {
	        free(svc_nalu->nal);
	        svc_nalu->nal = NULL;
	    }
	    if(obj->rtpSize)
	    {
	        free(obj->rtpSize);
	        obj->rtpSize = NULL;
	    }

        if(obj->json)
        {
            api_json_free(obj->json);
	        obj->json = NULL;
        }

	    //
	    //outparam[0] = "api_raw2rtp_packet";
	}
    return ret;
}

//===================unpacket===================================================
int rtp_unpacket(SVCNalu *svc_nalu, char *inBuf, short *rtpSize, int count, int rtpLen, int mtu_size, char *outBuf, int *oSize, unsigned short *last_seq_num)
{
	char *receivebuf = inBuf;
	//NALU_t nal[MAX_RTP_NUM];
	RTP_FIXED_HEADER  *rtp_hdr = NULL;
	NALU_HEADER		*nalu_hdr = NULL;
	FU_INDICATOR	*fu_ind = NULL;
	FU_HEADER		*fu_hdr = NULL;
	EXTEND_HEADER *rtp_ext = NULL;
	char* nalu_payload;
	int offset = 0;
	int idx = 0;
	int i = 0;
	//
	int starthead = 4;
	unsigned int itype = 0;
	int pos = 0;
	short extprofile = -1;
	short extlen = 0;
	unsigned short seqnum = 0;
	int ssrc = 0;
	int marker = 0;
	//short slice_type = 0;
	unsigned  long timestamp = 0;
	int last_timestamp = 0;
	int rptHeadSize = sizeof(RTP_FIXED_HEADER);
	int rtp_extend_length = 0;
	int redundancy_code = 0;//rtPacket->redundancy;//VP8REDUNDANCYSIZE;//0;//12;//RTPHEADSIZE;//vp8为了区分连续不同的MTU而插入了12字节的特征码
	static int frmnum = 0;
	int discard = 0;
	int sps_pps_flag = 0;
	unsigned int rtp_pkt_size = 0;
	int flag = 0;
	int ret = 0;
	offset = 0;

	flag = count ? (i < count) : (offset < rtpLen);
	//while(i < (rtPacket->count + cn) && offset < rtPacket->rtpLen)
	while(flag)//(i < count || offset < count)
	{
		NALU_t *n = &svc_nalu->nal[i++];

		n->buf = &outBuf[pos];
		rtp_hdr =(RTP_FIXED_HEADER*)&receivebuf[offset];
		nalu_hdr =(NALU_HEADER*)&receivebuf[offset + rptHeadSize];
		itype =
		n->nal_unit_type = nalu_hdr->TYPE;
		n->nal_reference_idc = nalu_hdr->NRI << 5;
		n->forbidden_bit = nalu_hdr->F;
		//if (n->forbidden_bit) printf("n->forbidden_bit \n", n->forbidden_bit);
		n->lost_packets = 0;
		//
		ssrc = rtp_hdr->ssrc;
		unsigned short seqnum = rtp_hdr->seq_no;
		///seqnum = ((rtp_hdr->seq_no & 0xFF) << 8) | ((rtp_hdr->seq_no >> 8));
		int diff = seqnum - *last_seq_num;
		if(abs(diff) > 1 && diff != 256 && diff > -256)
		{
			//printf("lost packet: diff=%d \tseqnum=%d \t test_last_seqnum=%d \n", diff, seqnum, *last_seq_num);
			//ret = -1;
		}
		*last_seq_num = seqnum;

		timestamp = rtp_hdr->timestamp;
		marker = rtp_hdr->marker;
		//printf("seqnum=%d \n", seqnum);
		extprofile = -1;
		extlen = 0;
		if(rtp_hdr->extension)
		{
			rtp_ext = (EXTEND_HEADER *)&receivebuf[offset + rptHeadSize];

			extprofile = rtp_ext->rtp_extend_profile;// & 7;
			rtp_extend_length = rtp_ext->rtp_extend_length;
			rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
			extlen = (rtp_extend_length + 1) << 2;
			rtp_pkt_size = rtp_ext->rtp_pkt_size;
			//printf("rtp_unpacket: rtpSize[idx]=%d \trtp_pkt_size=%d \n", rtpSize[idx], rtp_pkt_size);
            int rtpsize0 = rtpSize[idx];
			if(rtpsize0 != rtp_pkt_size)
			{
			    printf("error: rtp_unpacket: rtp_pkt_size=%d \n",rtp_pkt_size);
                printf("error: rtp_unpacket: rtpsize0=%d \n",rtpsize0);
			}
			if(rtp_pkt_size <=0 || rtp_pkt_size > mtu_size)
			{

				if(rtp_pkt_size < 1500 && rtp_pkt_size > mtu_size)
				{

				}
				else
				{
                    //LogOut(" rtp_pkt_size = %d \n",(void *)&rtp_pkt_size);
                    printf("error: rtp_unpacket: mtu_size=%d \n",mtu_size);
                    printf("error: rtp_unpacket: rtp_pkt_size=%d \n",rtp_pkt_size);
                    printf("error: rtp_unpacket: i=%d \n",i);
                    for(int l = 0; l < svc_nalu->nal_mem_num; l++)
                    {
                        rtpsize0 = rtpSize[l];
                        printf("error: rtp_unpacket: l=%d, rtpsize0=%d \n",l, rtpsize0);
                    }
					return -1;
				}
			}
            if (extlen < 0 || extlen > mtu_size) {
                printf("error: rtp_unpacket: extlen=%d \n",extlen);
                return -1;
            }
            rtpSize[idx] = rtp_pkt_size;
            //get extend info
            {
                int refs = rtp_ext->refs;
		        int ref_idx = rtp_ext->ref_idx;
		        int ref_idc = rtp_ext->ref_idc;
		        int res_num = rtp_ext->res_num;
		        int res_idx = rtp_ext->res_idx;
		        int qua_num = rtp_ext->qua_num;
		        int qua_idx = rtp_ext->qua_idx;
		        int is_lost_packet = rtp_ext->nack.nack0.is_lost_packet;
		        int loss_rate = rtp_ext->nack.nack0.loss_rate;
		        //int time_status = rtp_ext->nack.nack0.time_status;
		        int time_offset = rtp_ext->nack.nack0.time_offset;
		        uint64_t time_stamp0 = rtp_ext->nack.nack0.time_info.time_stamp0;
	            uint64_t time_stamp1 = rtp_ext->nack.nack0.time_info.time_stamp1;
	            int64_t packet_time_stamp = time_stamp0 | (time_stamp1 << 32);
		        //int64_t packet_time_stamp = rtp_ext->nack.nack0.time_info.time_stamp;
            }
            //printf("rtp_unpacket: rtp_pkt_size=%d \n", rtp_pkt_size);
            //printf("rtp_unpacket: extlen=%d \n", extlen);
			fu_ind =(FU_INDICATOR*)&receivebuf[offset + rptHeadSize + extlen];
			fu_hdr =(FU_HEADER*)&receivebuf[offset + rptHeadSize + extlen + 1];

			itype = fu_ind->TYPE;

			n->nal_unit_type = fu_ind->TYPE == 28 ? fu_hdr->TYPE : itype;
			n->nal_reference_idc = fu_ind->NRI << 5;
			n->forbidden_bit = fu_ind->F;
			//if (n->forbidden_bit) printf("n->forbidden_bit \n", n->forbidden_bit);
		}
		else
		{
			fu_ind =(FU_INDICATOR*)&receivebuf[offset + rptHeadSize + extlen];
			fu_hdr =(FU_HEADER*)&receivebuf[offset + rptHeadSize + extlen + 1];

			itype = fu_ind->TYPE;
			n->nal_unit_type = fu_ind->TYPE == 28 ? fu_hdr->TYPE : itype;
			n->nal_reference_idc = fu_ind->NRI << 5;
			n->forbidden_bit = fu_ind->F;
			//if (n->forbidden_bit) printf("n->forbidden_bit \n", n->forbidden_bit);
		}
        //printf("rtp_unpacket: 0: itype= %d \n", itype);
		switch (itype)
		{
			case 1://非IDR图像的片
				starthead =
				n->startcodeprefix_len = VIDIORASHEADSIZE;
				n->len = rtpSize[idx] - (rptHeadSize + extlen);
				nalu_payload=&receivebuf[offset + rptHeadSize + extlen + 1];
				memcpy(&n->buf[starthead + 1],nalu_payload,n->len-1);
				n->buf[0] = 0;
				n->buf[1] = 0;
				n->buf[2] = 0;
				n->buf[3] = 1;
				n->buf[4] = n->forbidden_bit | n->nal_reference_idc |  n->nal_unit_type;
				//& 0x80; //& 0x60;& 0x1f;//
				//1 bit  // 2 bit	// 5 bit
				//slice_type = n->nal_reference_idc >> 5;
				//slice_type == 3 ? AV_PICTURE_TYPE_I : slice_type == 2 ? AV_PICTURE_TYPE_P : slice_type == 0 ? AV_PICTURE_TYPE_B : -1;
				n->len += starthead;
				offset += rtpSize[idx++];
				pos += n->len;
				break;
			case 5://IDR图像的片
				starthead =
				n->startcodeprefix_len = VIDIORASHEADSIZE;
				n->len = rtpSize[idx] - (rptHeadSize + extlen);
				nalu_payload=&receivebuf[offset + rptHeadSize + extlen + 1];
				memcpy(&n->buf[starthead + 1],nalu_payload,n->len-1);
				n->buf[0] = 0;
				n->buf[1] = 0;
				n->buf[2] = 0;
				n->buf[3] = 1;
				n->buf[4] = n->forbidden_bit | n->nal_reference_idc |  n->nal_unit_type;
				//slice_type = AV_PICTURE_TYPE_I;
				sps_pps_flag |= 2;
				//printf("dec IDRFrame = %d \n", IDRCnt);
				//char format1[128] = "dec IDR = %d \n";
                //LogTime(NULL);
				//LogOut(format1, (void *)&IDRCnt);
				//& 0x80; //& 0x60;& 0x1f;//
				//1 bit  // 2 bit	// 5 bit
				n->len += starthead;
				offset += rtpSize[idx++];
				pos += n->len;
				break;
			case 6://SEI
				starthead =
				n->startcodeprefix_len = VIDIORASHEADSIZE - 1;
				n->len = rtpSize[idx] - (rptHeadSize + extlen);
				nalu_payload=&receivebuf[offset + rptHeadSize + extlen + 1];
				memcpy(&n->buf[starthead + 1],nalu_payload,n->len-1);
				n->buf[0] = 0;
				n->buf[1] = 0;
				n->buf[2] = 1;
				n->buf[3] = n->forbidden_bit | n->nal_reference_idc |  n->nal_unit_type;
				//& 0x80; //& 0x60;& 0x1f;//
				//1 bit  // 2 bit	// 5 bit
				n->len += starthead;
				offset += rtpSize[idx++];
				pos += n->len;
				break;

			case 7:	//序列参数集
				starthead =
				n->startcodeprefix_len = VIDIORASHEADSIZE;
				n->len = rtpSize[idx] - (rptHeadSize + extlen);
				nalu_payload=&receivebuf[offset + rptHeadSize + extlen + 1];
				memcpy(&n->buf[starthead + 1],nalu_payload,n->len-1);
				n->buf[0] = 0;
				n->buf[1] = 0;
				n->buf[2] = 0;
				n->buf[3] = 1;
				n->buf[4] = n->forbidden_bit | n->nal_reference_idc |  n->nal_unit_type;
				//& 0x80; //& 0x60;& 0x1f;//
				//1 bit  // 2 bit	// 5 bit
				sps_pps_flag |= 1;
				//printf("dec IDR = %d \n", IDRCnt);
				//char format1[128] = "dec IDR = %d \n";
                //LogTime(NULL);
				//LogOut(format1, (void *)&IDRCnt);
				n->len += starthead;
				offset += rtpSize[idx++];
				pos += n->len;
				break;
			case 8://图像参数集
				starthead =
				n->startcodeprefix_len = VIDIORASHEADSIZE;
				n->len = rtpSize[idx] - (rptHeadSize + extlen);
				nalu_payload=&receivebuf[offset + rptHeadSize + extlen + 1];
				memcpy(&n->buf[starthead + 1],nalu_payload,n->len-1);
				n->buf[0] = 0;
				n->buf[1] = 0;
				n->buf[2] = 0;
				n->buf[3] = 1;
				n->buf[4] = n->forbidden_bit | n->nal_reference_idc |  n->nal_unit_type;
				//& 0x80; //& 0x60;& 0x1f;//
				//1 bit  // 2 bit	// 5 bit
				n->len += starthead;
				offset += rtpSize[idx++];
				pos += n->len;
				//printf("dec IDR = %d \n", IDRCnt);
				//char format1[128] = "dec IDR = %d \n";
				//LogOut(format1, (void *)&IDRCnt);
				break;
			case 24://单时间聚合包STAP-A
			{
			    int offset2 = 0;
			    int rtplen = rtpSize[idx];
			    char *dataPtr = &receivebuf[offset + rptHeadSize + extlen + 1];
			    //printf("rtp_unpacket: offset= %d \n", offset);
			    do{
			        if(!offset2)
			        {
                        offset2 += rptHeadSize + extlen + 1;
                        n->len = 0;
			        }
			        else
			        {
			            //n = &svc_nalu->nal[i++];
			            n->buf = &outBuf[pos];
			        }

			        unsigned short *nal_size = (unsigned short *)dataPtr;
			        int rawSize = nal_size[0];
			        rawSize = (rawSize >> 8) | ((rawSize & 0xFF) << 8);
			        //printf("rtp_unpacket: rawSize= %d \n", rawSize);
			        //
			        fu_ind = (FU_INDICATOR*)&dataPtr[2];
			        itype = fu_ind->TYPE;
			        //printf("rtp_unpacket: itype= %d \n", itype);
			        n->nal_unit_type = fu_ind->TYPE;
			        n->nal_reference_idc = fu_ind->NRI << 5;
			        n->forbidden_bit = fu_ind->F;
			        if(itype == 6)//SEI
			        {
			            starthead = VIDIORASHEADSIZE - 1;
			            n->buf[0] = 0;
				        n->buf[1] = 0;
				        n->buf[2] = 1;
				        n->buf[3] = n->forbidden_bit | n->nal_reference_idc |  n->nal_unit_type;
			        }
			        else{
			            starthead = VIDIORASHEADSIZE;
			            n->buf[0] = 0;
				        n->buf[1] = 0;
				        n->buf[2] = 0;
				        n->buf[3] = 1;
				        n->buf[4] = n->forbidden_bit | n->nal_reference_idc |  n->nal_unit_type;
			        }
			        nalu_payload = &dataPtr[3];
			        memcpy(&n->buf[starthead + 1], nalu_payload, (rawSize - 1));
			        n->len +=  rawSize + starthead;
			        n->startcodeprefix_len = starthead;
			        //
			        dataPtr += 2 + rawSize;
			        offset2 += 2 + rawSize;
			        pos += rawSize + starthead;
			    }while(offset2 < rtplen);
			    offset += rtpSize[idx++];
			    //pos += n->len;
				break;
			}
			case 28://片分组方式FU-A
				//slice_type = n->nal_reference_idc >> 5;
				//slice_type == 3 ? AV_PICTURE_TYPE_I : slice_type == 2 ? AV_PICTURE_TYPE_P : slice_type == 0 ? AV_PICTURE_TYPE_B : -1;
				//设置FU HEADER,并将这个HEADER填入sendbuf[13]
				///if(fu_hdr->S)
				if(true)
				{
					starthead =
					n->startcodeprefix_len = VIDIORASHEADSIZE;
					n->len = rtpSize[idx] - (rptHeadSize + extlen + 1);
					if (n->len > mtu_size)
					{
					    printf("error: rtp_unpacket: n->len=%d \n",n->len);
						return -1;
					}
					nalu_payload=&receivebuf[offset + rptHeadSize + extlen + 2];
					memcpy(&n->buf[starthead + 1],nalu_payload,n->len-1);

					n->buf[0] = 0;
					n->buf[1] = 0;
					n->buf[2] = 0;
					n->buf[3] = 1;
					//n->buf[4] = (fu_ind & 0xE0) | (fu_hdr & 0x1F);//(fuIndicader & 0xE0) | (fuHeader & 0x1F);
					//gxh_20130117	n->forbidden_bit = 0;//test
					n->buf[4] = n->forbidden_bit | n->nal_reference_idc |  n->nal_unit_type;
					//& 0x80; //& 0x60;& 0x1f;//
					//1 bit  // 2 bit	// 5 bit
					n->len += starthead;
					offset += rtpSize[idx++];
				}
				else
				{
				    //forbidden_bit: 如果有语法冲突，则为 1。当网络识别此单元存在比特错误时，可将其设为 1，以便接收方丢掉该单元。
				    //被复用为多SLICE与单SLICE的区分
					if(!n->forbidden_bit)
					{
						starthead =
						n->startcodeprefix_len = VIDIORASHEADSIZE - 1;
						n->len = rtpSize[idx] - (rptHeadSize + extlen + 1);
						if (n->len > mtu_size)
						{
						    printf("error: rtp_unpacket: n->len=%d \n",n->len);
							return -1;
						}
						nalu_payload=&receivebuf[offset + rptHeadSize + extlen + 2];
						memcpy(&n->buf[starthead + 1],nalu_payload,n->len-1);

						n->buf[0] = 0;
						n->buf[1] = 0;
						n->buf[2] = 1;
						n->buf[3] = n->forbidden_bit | n->nal_reference_idc |  n->nal_unit_type;
					}
					else
					{
						starthead =
						n->startcodeprefix_len = 0;//VIDIORASHEADSIZE - 1;
						n->len = rtpSize[idx] - (rptHeadSize + extlen + 2);
						if (n->len > mtu_size)
						{
						    printf("error: rtp_unpacket: n->len=%d \n",n->len);
							return -1;
						}
						nalu_payload=&receivebuf[offset + rptHeadSize + extlen + 2];
						memcpy(&n->buf[starthead],nalu_payload,n->len);
					}
					n->len += starthead;
					offset += rtpSize[idx++];
				}
                pos += n->len;
				break;
			default:
			    printf("error: rtp_unpacket: itype=%d \n",itype);
			    for(int l = 0; l < svc_nalu->nal_mem_num; l++)
                {
                    int rtpsize0 = rtpSize[l];
                    //printf("error: rtp_unpacket: l=%d, rtpsize0=%d \n",l, rtpsize0);
                }
				//n->len += starthead;
				offset += rtpSize[idx++];
				discard++;
				break;
		}
		//pos += n->len;
		offset += redundancy_code;
		//if((i - cn) > 1)
		//{
		//	if(timestamp != last_timestamp)
		//	{
		//		offset = rtPacket->rtpLen;//error
		//		return 0;
		//	}
		//}
		//last_seqnum = video_seqnum;
		last_timestamp = timestamp;
		flag = count ? (i < count) : (offset < rtpLen);
	}
	//printf("rtp_unpacket: i= %d \n", i);
	//printf("rtp_unpacket: pos= %d \n", pos);
	//rtPacket->video_seqnum = last_seqnum;
	//*num = i - discard;
	*oSize = pos;
	frmnum++;
	if (ret < 0)
	{
		return ret;
	}
	svc_nalu->nal_mem_num = i;
	return pos;
}
int rtp_packet2raw(void *hnd, char *inBuf, char *outBuf)
{
    int ret = 0;
    RtpObj *obj = (RtpObj *)hnd;
    cJSON *json = obj->json;
    SVCNalu *svc_nalu = &obj->svc_nalu;
    int mtu_size = GetvalueInt(json, "mtu_size");
    int nal_mem_num = 0;//((int)(size / mtu_size) << 1) + 3;//3:sps/pps/sei
    //printf("rtp_packet2raw: start 0 \n");
    int count = 0;//no used; use rtpLen
	int rtpLen = 0;
	int oSize = 0;

    obj->rtpSize = GetArrayValueShort(json, "rtpSize", &nal_mem_num);
    for( int i = 0 ; i < nal_mem_num ; i ++ ){
        rtpLen += obj->rtpSize[i];
    }

    //printf("rtp_packet2raw: nal_mem_num= %d \n", nal_mem_num);
    svc_nalu->nal_mem_num = nal_mem_num;
    svc_nalu->buf = inBuf;
    svc_nalu->nal = calloc(1, sizeof(NALU_t) * (nal_mem_num << 1) );
    short *rtpSize = obj->rtpSize;
	ret = rtp_unpacket(svc_nalu, inBuf, rtpSize, count, rtpLen, mtu_size, outBuf, &oSize, &obj->seq_num);
    if(ret < 0)
    {
        //lost packet
        return ret;
    }
    //printf("rtp_packet2raw: ret= %d \n", ret);
    ret = oSize;
    return ret;
}
HCSVC_API
int api_rtp_packet2raw(char *handle, char *data, char *param, char *outbuf, char *outparam[])
{
    int ret = 0;

    //initobj(id);
    {
        long long *testp = (long long *)handle;
        CodecObj *codecObj = (CodecObj *)testp[0];
        RtpObj *obj = codecObj->rtpObj;//(RtpObj *)&global_rtp_objs[id];
        int id = codecObj->Obj_id;
        if (!obj)
        {
            obj = (RtpObj *)calloc(1, sizeof(RtpObj));
            codecObj->rtpObj = obj;
            ResetObj(obj);
            obj->Obj_id = id;
            //
            if(!obj->logfp)
            {
                char filename[256] = "rtp_dec_gxh_";
                char ctmp[32] = "";
                int fileidx = id;//random() % 10000;
                sprintf(ctmp, "%d", fileidx);
                strcat(filename, ctmp);
                strcat(filename, ".txt");
                //obj->logfp = fopen(filename, "w");
            }
        }
        obj->json = (cJSON *)api_str2json(param);

        //printf("api_rtp_packet2raw: start \n");

        ret = rtp_packet2raw((void *)obj, data, outbuf);

        //printf("api_rtp_packet2raw: ret= %d \n", ret);
        if(ret > 0)
        {
            //free
            SVCNalu *svc_nalu = &obj->svc_nalu;
            int nal_mem_num = svc_nalu->nal_mem_num;
	        int sum = 0;

            short *rtpSize = calloc(1, nal_mem_num * sizeof(short));
            outparam[0] = obj->outparam[0];
            for(int i = 0; i < nal_mem_num; i++)
    	    {
	            int size = svc_nalu->nal[i].len;
	            rtpSize[i] = size;
                sum += size;
    	    }

	        cJSON *json2 = NULL;
            json2 = renewJsonArray1(json2, "rtpSize", rtpSize, nal_mem_num);
            char *jsonStr = api_json2str(json2);//比较耗时
            api_json_free(json2);
            strcpy(obj->outparam[0], jsonStr);
            api_json2str_free(jsonStr);
            free(rtpSize);

            if(sum != ret)
	        {
	            printf("error: api_rtp_packet2raw: sum= %d \n", sum);
	            printf("error:api_rtp_packet2raw: ret= %d \n", ret);
    	    }
        }
        if(obj->svc_nalu.nal)
        {
            free(obj->svc_nalu.nal);
            obj->svc_nalu.nal = NULL;
        }
	    if(obj->rtpSize)
	    {
	        free(obj->rtpSize);
	        obj->rtpSize = NULL;
	    }
	    if(obj->json)
	    {
	        api_json_free(obj->json);
	        obj->json = NULL;
	    }
	}
    return ret;
}
//
#if 0
int get_net_info(NetInfo *info, char *outBuf)
{
    int ret = 0;
    if(true)
    {
        int len0 = strlen(outBuf);
        if(len0 > 0)
        {
            strcat(outBuf,";");
        }
        strcat(outBuf,"{");

        strcat(outBuf, "\"");
        strcat(outBuf,"info_status");
        strcat(outBuf, "\":");
        sprintf(&outBuf[strlen(outBuf)], "%d", info->info_status);
        strcat(outBuf, ",");

        strcat(outBuf, "\"");
        strcat(outBuf,"chanId");
        strcat(outBuf, "\":");
        sprintf(&outBuf[strlen(outBuf)], "%d", info->chanId);
        strcat(outBuf, ",");

        strcat(outBuf, "\"");
        strcat(outBuf,"loss_rate");
        strcat(outBuf, "\":");
        sprintf(&outBuf[strlen(outBuf)], "%d", info->loss_rate);
        strcat(outBuf, ",");

        strcat(outBuf, "\"");
        strcat(outBuf,"st0");
        strcat(outBuf, "\":");
        sprintf(&outBuf[strlen(outBuf)], "%d", info->st0);
        strcat(outBuf, ",");

        strcat(outBuf, "\"");
        strcat(outBuf,"rt0");
        strcat(outBuf, "\":");
        sprintf(&outBuf[strlen(outBuf)], "%d", info->rt0);
        strcat(outBuf, ",");

        strcat(outBuf, "\"");
        strcat(outBuf,"st1");
        strcat(outBuf, "\":");
        sprintf(&outBuf[strlen(outBuf)], "%d", info->st1);
        strcat(outBuf, ",");

        strcat(outBuf, "\"");
        strcat(outBuf,"rt1");
        strcat(outBuf, "\":");
        sprintf(&outBuf[strlen(outBuf)], "%d", info->rt1);
        strcat(outBuf, "}");
        int len1 = strlen(outBuf);
        ret = len1 - len0;
    }
    return ret;
}
#endif
cJSON *get_net_info2(NetInfo *info, cJSON *pJsonRoot)
{
    char *ret = NULL;
    if(true)
    {
        //cJSON *pJsonRoot  = NULL;
		char *key = "info_status";
		int ivalue = info->info_status;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);

		key = "chanId";
		ivalue = info->chanId;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);//8

		key = "res_idx";
		ivalue = info->res_idx;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);//8

		key = "decodeId";
		ivalue = info->decodeId;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);//8

		key = "loss_rate";
		ivalue = info->loss_rate;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);//12

		key = "st0";
		ivalue = info->st0;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);//11

		key = "rt0";
		ivalue = info->rt0;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);//11

		key = "st1";
		ivalue = info->st1;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);//11

		key = "rt1";
		ivalue = info->rt1;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);//11

		key = "ssrc";
		ivalue = info->ssrc;
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);//11

		key = "tstmp0";
		ivalue = (int)((uint64_t)info->time_stamp & 0xFFFFFFFF);
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);//11

		key = "tstmp1";
		ivalue = (int)((uint64_t)info->time_stamp >> 32);
		pJsonRoot = api_renew_json_int(pJsonRoot, key, ivalue);//11

		//ret = api_json2str(pJsonRoot); //free(ret);//返回json字符串，注意外面用完要记得释放空间

		//api_json_free(pJsonRoot);
    }
    return pJsonRoot;
}
char *get_extern_info(char *data)
{
    char *ret = NULL;
    RTP_FIXED_HEADER  *rtp_hdr = NULL;
    EXTEND_HEADER *rtp_ext = NULL;
    rtp_hdr = (RTP_FIXED_HEADER *)data;
    if(rtp_hdr->extension)
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
	    sprintf(ctmp, "%lld", packet_time_stamp);
		pJsonRoot = api_renew_json_str(pJsonRoot, key, ctmp);

		ret = api_json2str(pJsonRoot); //free(ret);//返回json字符串，注意外面用完要记得释放空间

		api_json_free(pJsonRoot);

    }
    return ret;
}
static int adapt_by_cpu(int ref_idx, int refs, int pict_type)
{
    int ret = 0;
    int icpurate = 0;
    int memrate = 0;
    int devmemrate = 0;
    api_get_cpu_info2(&icpurate, &memrate, &devmemrate);
    if(icpurate > 95)
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
    }
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
    return ret;
}
//
#if 0
#define PKT_LOSS_INTERVAL 100
#define MAX_PKT_DELAY 100
static int count_loss_rate(void *hnd)
{
    int ret = 0;
    RtpObj *obj = (RtpObj *)hnd;
    RTP_FIXED_HEADER  *rtp_hdr = NULL;
    if(true)
    {
        int64_t time_stamp = api_get_time_stamp_ll();
        if(!obj->net_time_stamp)
        {
            obj->net_time_stamp = time_stamp;
            return ret;
        }
        int diff_time = (int)(time_stamp - obj->net_time_stamp);

        if(diff_time > PKT_LOSS_INTERVAL)
        {
            //printf("count_loss_rate: diff_time=%d \n", diff_time);
            int I0 = obj->min_packet % obj->buf_size;
            int I1 = obj->max_packet % obj->buf_size;
            //printf("get_frame: obj->min_packet= %u \n", obj->min_packet);
            //printf("get_frame: obj->max_packet= %u \n", obj->max_packet);
            uint8_t *buf0 = (uint8_t *)&obj->recv_buf[I0][sizeof(int)];
            uint8_t *buf1 = (uint8_t *)&obj->recv_buf[I1][sizeof(int)];
            RTP_FIXED_HEADER *rtp_hdr0 = (RTP_FIXED_HEADER *)buf0;
            RTP_FIXED_HEADER *rtp_hdr1 = (RTP_FIXED_HEADER *)buf1;
            int *p0 = (int *)&obj->recv_buf[I0][0];
            int size0 = p0[0];
            int *p1 = (int *)&obj->recv_buf[I1][0];
            int size1 = p1[0];
            //printf("get_frame: buf0= %lld \n", (long long)buf0);
            //printf("get_frame: I0= %d \n", I0);
            //printf("get_frame: I1= %d \n", I1);
            int64_t timestamp0 = rtp_hdr0->timestamp;
            int64_t timestamp1 = rtp_hdr1->timestamp;
            int delay = (int)(timestamp1 - timestamp0) / 90;//90000Hz
            if(timestamp0 > timestamp1)
            {
                delay = (int)((timestamp1 + LEFT_SHIFT32 - timestamp0) / 90);
            }
            //printf("count_loss_rate: delay=%d \n", delay);
            if(delay > (PKT_LOSS_INTERVAL + MAX_PKT_DELAY))
            {
                printf("count_loss_rate: delay=%d \n", delay);
                int start = obj->min_packet;
                int end = obj->max_packet;
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
                    int this_seq_num = i % LEFT_SHIFT16;//i > MAX_USHORT ? (i - MAX_USHORT) : i;
                    int I = this_seq_num % obj->buf_size;
                    uint8_t *buf = (uint8_t *)&obj->recv_buf[I][sizeof(int)];
                    int *p = (int *)&obj->recv_buf[I][0];
                    int size = p[0];

                    if(size > 0)
                    {
                        rtp_hdr = (RTP_FIXED_HEADER *)buf;
                        int64_t this_timestamp = rtp_hdr->timestamp;
                        int this_seqnum = rtp_hdr->seq_no;

                        if(this_timestamp >= 0 && this_seqnum >= 0)
                        {
                            delay = (int)(this_timestamp - timestamp0) / 90;//90000Hz
                            if(timestamp0 > timestamp1)
                            {
                                delay = (int)((this_timestamp + LEFT_SHIFT32 - timestamp0) / 90);
                            }
                            //printf("count_loss_rate: delay=%d \n", delay);
                            if(delay >= PKT_LOSS_INTERVAL)
                            {
                                if(!pkt_num)
                                {
                                    printf("error: count_loss_rate: start= %d, end= %d \n", start, end);
                                }
                                ret = (int)(100 * ((float)loss_num / pkt_num));
                                if(loss_num != j)
                                {
                                    //printf("warning: CountLossRate: j= %d, loss_num= %d \n", j, loss_num);
                                }
                                obj->net_time_stamp = api_get_time_stamp_ll();
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
                }
            }
        }
    }

    return ret;
}
#endif
int get_frame(void *hnd, char *outbuf, short *rtpSize, char *complete, void **net_info, long long *frame_timestamp)
{
    int ret = 0;
    RtpObj *obj = (RtpObj *)hnd;
    cJSON *json = obj->json;
    cJSON *jsonInfo = NULL;
    cJSON *jsonArray = NULL;
    //int insize = GetvalueInt(json, "insize");
    int delay_time = GetvalueInt(json, "delay_time");
    int qos_level = GetvalueInt(json, "qos_level");
    int adapt_cpu = GetvalueInt(json, "adapt_cpu");
    int counted_loss = 0;
    int counted_loss_rate = 0;
    RTP_FIXED_HEADER  *rtp_hdr = NULL;
	EXTEND_HEADER *rtp_ext = NULL;
	NALU_HEADER *nalu_hdr   = NULL;
	FU_INDICATOR *fu_ind     = NULL;
	FU_HEADER *fu_hdr     = NULL;
	uint32_t last_frame_time_stamp = obj->last_frame_time_stamp;
	int packet_distance = obj->max_packet - obj->min_packet;
	long long now_ms = api_get_time_stamp_ll();
    if(obj->delay_time > 0)
    {
        delay_time = obj->delay_time;
        if(delay_time > 200)
        {
            if(obj->loglevel)
            {
                printf("video: get_frame: delay_time= %d \n", delay_time);
            }
            //delay_time = 100;//test
            if(delay_time > MAX_DELAY_TIME)
            {
                delay_time = MAX_DELAY_TIME;
            }
        }
        //delay_time = 200;
    }
	//printf("get_frame: delay_time= %d \n", delay_time);
    //if(obj->max_packet >= 0 && obj->min_packet >= 0)
    {
        //if(packet_distance > (obj->buf_size >> 1))
        //{
        //    printf("warning: get_frame: packet_distance= %d \n", packet_distance);
        //}
        //
        {
            //先取後存
            int I0 = obj->min_packet % obj->buf_size;
            int I1 = obj->max_packet % obj->buf_size;
            //printf("get_frame: obj->min_packet= %u \n", obj->min_packet);
            //printf("get_frame: obj->max_packet= %u \n", obj->max_packet);
            uint8_t *buf0 = (uint8_t *)&obj->recv_buf[I0][sizeof(int)];
            uint8_t *buf1 = (uint8_t *)&obj->recv_buf[I1][sizeof(int)];
            RTP_FIXED_HEADER *rtp_hdr0 = (RTP_FIXED_HEADER *)buf0;
            RTP_FIXED_HEADER *rtp_hdr1 = (RTP_FIXED_HEADER *)buf1;
            int *p0 = (int *)&obj->recv_buf[I0][0];
            int size0 = p0[0];
            int *p1 = (int *)&obj->recv_buf[I1][0];
            int size1 = p1[0];
            //printf("get_frame: buf0= %lld \n", (long long)buf0);
            //printf("get_frame: I0= %d \n", I0);
            //printf("get_frame: I1= %d \n", I1);
            int64_t timestamp0 = rtp_hdr0->timestamp;
            int64_t timestamp1 = rtp_hdr1->timestamp;
            int delay = (int)(timestamp1 - timestamp0) / 90;//90000Hz
            if(timestamp0 > timestamp1)
            {
                delay = (int)((timestamp1 + LEFT_SHIFT32 - timestamp0) / 90);
            }
            if(delay > 10000)
            {
                //int64_t last_ms = obj->last_frame_time;
                //int delay2 = (int)(now_ms - last_ms);
                printf("warning: video: get_frame: delay= %u \n", delay);
                //printf("warning: get_frame: delay2= %u \n", delay2);
                //printf("warning: video: get_frame: obj->Obj_id= %u \n", obj->Obj_id);
                printf("warning: video: get_frame: timestamp0= %u \n", timestamp0);
                printf("warning: video: get_frame: timestamp1= %u \n", timestamp1);
                printf("warning: video: get_frame: I0= %u \n", I0);
                printf("warning: video: get_frame: I1= %u \n", I1);
                printf("warning: video: get_frame: obj->min_packet= %u \n", obj->min_packet);
                printf("warning: video: get_frame: obj->max_packet= %u \n", obj->max_packet);
                printf("warning: video: get_frame: obj->old_seqnum= %u \n", obj->old_seqnum);
#if 0
                for(int t = obj->min_packet; t <= obj->max_packet; t++)
                {
                    int I = t % obj->buf_size;
                    uint8_t *buf = (uint8_t *)&obj->recv_buf[I][sizeof(int)];
                    int *p = (int *)&obj->recv_buf[I][0];
                    int size = p[0];
                    if(!size)
                        printf("warning: video: get_frame: t= %d, t.size=%d \n", t, size);
                }
#endif
                //int I = obj->min_packet % obj->buf_size;
                //uint8_t *buf = (uint8_t *)&obj->recv_buf[I][sizeof(int)];
                //int *p = (int *)&obj->recv_buf[I][0];
                //int size = p[0];
                //printf("warning: get_frame: min_packet.size=%d \n", size);
                //I = obj->max_packet % obj->buf_size;
                //buf = (uint8_t *)&obj->recv_buf[I][sizeof(int)];
                //p = (int *)&obj->recv_buf[I][0];
                //size = p[0];
                //printf("warning: get_frame: max_packet.size=%d \n", size);
            }
            int offset = 0;
            //printf("get_frame: timestamp0= %d \n", timestamp0);
            //printf("get_frame: timestamp1= %d \n", timestamp1);
            //if(diff >= delay_time)
            {
                int ref_idx = 0;
                int refs = 0;
                int pict_type = 0;
                int marker = 0;
                int sps_flag = 0;
                int pps_flag = 0;
                int idr_flag = 0;
                int first_slice = 0;
                int64_t nack_time = 0;
                unsigned  int timestamp = 0;
                unsigned  int start_timestamp = 0;
                unsigned  int last_timestamp = 0;
                int j = 0;
                //int complete_flag = 0;
                int fec_k = 0;
                int fec_n = 0;
                int ref_idc = 0;
                int loss_num = 0;
                int last_seq_num = -1;
                int test_end_packet = 0;
                int this_seqnum = -1;
                int is_header = 0;
                uint16_t start_seq_num = obj->min_packet;
                uint16_t stop_seq_num = obj->min_packet;
                int start = obj->min_packet;
                int end = obj->max_packet;
                if(start > end)
                {
                    end += LEFT_SHIFT16;//MAX_USHORT;
                }
                packet_distance = end - start;
                if((end - start) > (obj->buf_size >> 1))
                {
                    int this_seq_num = start % LEFT_SHIFT16;
                    int I = this_seq_num % obj->buf_size;
                    uint8_t *buf = (uint8_t *)&obj->recv_buf[I][sizeof(int)];
                    int *p = (int *)&obj->recv_buf[I][0];
                    int size = p[0];

                    printf("warning: video: get_frame: obj->Obj_id= %u \n", obj->Obj_id);
                    printf("warning: video: get_frame: packet_distance= %d \n", packet_distance);
                    printf("warning: video: get_frame: delay= %d \n", delay);
                    printf("warning: video: get_frame: min_packet size=%d \n", size);
                    printf("warning: video: get_frame: obj->min_packet= %d \n", obj->min_packet);
                    printf("warning: video: get_frame: obj->max_packet= %d \n", obj->max_packet);
                }
                //for(int i = obj->min_packet; i < obj->max_packet; i++)
                //for(int i = start; i < end; i++)
                for(int i = start; i <= end; i++)
                {
                    //if(i == LEFT_SHIFT16)
                    //{
                    //    continue;
                    //}
                    //printf("get_frame: i= %d \n", i);
                    int this_seq_num = i % LEFT_SHIFT16;//i > MAX_USHORT ? (i - MAX_USHORT) : i;
                    int I = this_seq_num % obj->buf_size;
                    //memcpy(&obj->recv_buf[I][sizeof(int)], data, insize);
                    if(!obj->recv_buf[I])
                    {
                        printf("error: video: get_frame: obj->Obj_id= %x \n", obj->Obj_id);
                        printf("error: video: get_frame: I=%d \n", I);
                        printf("error: video: get_frame: obj->recv_buf[I]=%x \n", obj->recv_buf[I]);
                    }

                    uint8_t *buf = (uint8_t *)&obj->recv_buf[I][sizeof(int)];
                    int *p = (int *)&obj->recv_buf[I][0];
                    //p[0] = 0;
                    int size = p[0];
                    if(size > MAX_PACKET_SIZE)
                    {
                        printf("error: video: get_frame: size=%d \n\n\n\n\n\n\n\n\n\n", size);
                    }
                    if(size > 0)
                    {
                        rtp_hdr = (RTP_FIXED_HEADER *)buf;
                        int nal_unit_type = -1;
                        int extlen = 0;
                        this_seqnum = rtp_hdr->seq_no;
                        RtpInfo info = {0};
                        info.raw_offset = -1;

                        int is_hcsvc_rtp = GetRtpInfo((uint8_t*)buf, size, &info, H264_PLT);
                        if(!is_hcsvc_rtp > 0)
                        {
                            printf("error: video: get_frame: is_hcsvc_rtp= %d \n", is_hcsvc_rtp);
                            continue;
                        }
                        if(this_seq_num != this_seqnum)
                        {
                            printf("error: video: get_frame: this_seq_num= %d \n", this_seq_num);
                            printf("error: video: get_frame: this_seqnum= %d \n", this_seqnum);
                            printf("error: video: get_frame: obj->min_packet= %d \n", obj->min_packet);
                            printf("error: video: get_frame: obj->max_packet= %d \n", obj->max_packet);
                        }
                        //last_frame_time_stamp
                        timestamp = rtp_hdr->timestamp;

                        if(!start_timestamp)
                        {
                            start_timestamp = timestamp;
                            //obj->min_packet = this_seqnum;
                            start_seq_num = this_seqnum;
                            stop_seq_num = this_seqnum;
                            fec_k = info.fec_k;
                            fec_n = info.fec_n;
                            ref_idc = info.ref_idc;
                            nack_time = info.nack_time;
                            ref_idx = info.ref_idx;
                            refs = info.refs;
                            is_header = info.is_header;
                            //pict_type = 0;
                        }
                        if(abs(timestamp - start_timestamp) > delay_time)
                        {
                            //
                        }
                        //printf("get_frame: last_timestamp= %d \n", last_timestamp);
                        //printf("get_frame: timestamp= %d \n", timestamp);
                        //保证缓存里一直有数据
                        if(last_timestamp && (last_timestamp != timestamp))
                        {
                            //next frame
                            if(last_seq_num >= 0)
                            {
                                int diff_seqnum = this_seqnum - last_seq_num;
                                if(this_seqnum < last_seq_num)
                                {
                                    diff_seqnum = (this_seqnum + LEFT_SHIFT16) - last_seq_num;
                                }
                                loss_num += (diff_seqnum - 1);

                                //complete_flag += (diff_seqnum - 1);
                            }
                            //obj->last_frame_time_stamp = last_timestamp;
                            //ret = offset;
                            //obj->min_packet = this_seqnum;
                            ret = j;
                            frame_timestamp[0] = nack_time;//timestamp;
                            //printf("get_frame: j= %d \n", j);
                            test_end_packet = 1;
                            break;
                        }
                        else{
                            stop_seq_num = this_seqnum;
                            memcpy(&outbuf[offset], buf, size);
                            offset += size;
                            rtpSize[j] = size;
                            j += 1;
                            ///p[0] = 0;
                            //if(obj->cut_flag)
                            //{
                            //    if(this_seqnum < QUART_USHORT)
                            //    {
                            //        this_seqnum += MAX_USHORT;
                            //    }
                            //}
                            //obj->min_packet = this_seqnum;//
                        }
                        //stop_seq_num = this_seqnum;
#if 0
                        if(rtp_hdr->extension)
			            {
				            rtp_ext = (EXTEND_HEADER *)&buf[sizeof(RTP_FIXED_HEADER)];

				            int rtp_extend_length = rtp_ext->rtp_extend_length;
			                rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
			                extlen = (rtp_extend_length + 1) << 2;

			                if(rtp_ext->first_slice && !info.is_fec)
			                {

			                    first_slice += 1;
			                    if(first_slice > 1)
			                    {
			                        printf("get_frame: first_slice= %d \n", first_slice);
			                        printf("get_frame: info.is_fec= %d \n", info.is_fec);
			                        //printf("get_frame: info.fec_seq_no= %d \n", info.fec_seq_no);
			                    }

			                }
			                nal_unit_type = rtp_ext->nal_type;
			            }
#else
                        if(info.is_first_slice)
                        {
                            first_slice = info.is_first_slice;
                        }
			            nal_unit_type = info.nal_type;
#endif
			            //將此重要信息，寫入RTP，否則，FEC下不可讀
			            //nalu_hdr =(NALU_HEADER*)&buf[sizeof(RTP_FIXED_HEADER) + extlen];
			            //if(nal_unit_type != nalu_hdr->TYPE)
			            //{
			            //    printf("error: resort_packet: nal_unit_type= %d \n", nal_unit_type);
                        //    printf("error: resort_packet: nalu_hdr->TYPE= %d \n", nalu_hdr->TYPE);
                        //    for(int l = 0; l < 20; l++)
                        //    {
                        //        printf("##################\n");
                        //    }
			            //}
			            //nal_unit_type = nalu_hdr->TYPE;
                        if(last_seq_num >= 0)
                        {
                            int diff_seqnum = this_seqnum - last_seq_num;
                            if(this_seqnum < last_seq_num)
                            {
                                diff_seqnum = (this_seqnum + LEFT_SHIFT16) - last_seq_num;
                            }
                            loss_num += (diff_seqnum - 1);

                            //complete_flag += (diff_seqnum - 1);
                        }
			            if(nal_unit_type == 5)
			            {
			                idr_flag += 1;
			            }
			            else if(nal_unit_type == 24)
			            {
			                sps_flag += 1;
			                pps_flag += 1;
			                memcpy(obj->sps, buf, size);//can be used when sps loss and refresh_idr zero
			                printf("get_frame: 0: size= %d \n", size);
			                if (obj->logfp && false)
			                {
                                fprintf(obj->logfp, "get_frame: sps: size= %d \n", size);
                                fflush(obj->logfp);
                            }
			            }
			            else if(nal_unit_type == 7)
			            {
			                sps_flag += 1;
			                //判断是否是fec,并进行解析
			                memcpy(obj->sps, buf, size);//can be used when sps loss and refresh_idr zero
			                //printf("get_frame: 1: size= %d \n", size);
			                //printf("get_frame: 1: nal_unit_type= %d \n", nal_unit_type);
			                if (obj->logfp && false)
			                {
                                fprintf(obj->logfp, "get_frame: sps: size= %d \n", size);
                                fflush(obj->logfp);
                            }
			            }
			            else if(nal_unit_type == 8)
			            {
			                pps_flag += 1;
			                memcpy(obj->pps, buf, size);//can be used when pps loss and refresh_idr zero
			                //printf("get_frame: 2: size= %d \n", size);
			                //printf("get_frame: 2: nal_unit_type= %d \n", nal_unit_type);
			                if (obj->logfp && false)
			                {
                                fprintf(obj->logfp, "get_frame: pps: size= %d \n", size);
                                fflush(obj->logfp);
                            }
			            }
			            else{

			            }
                        //if(rtp_hdr->marker)
                        if(info.nal_marker)
                        {
                            marker += 1;
                        }
                        if(last_timestamp == 0)
                        {
                            last_timestamp = timestamp;
                        }
                        last_seq_num = this_seqnum;
                    }//size
                }//i
                if(fec_k > 0 && j >= fec_k && fec_n <= 255 && true)
                //if(fec_k > 0 && j > fec_k && fec_n <= 255 && false)
                {
                    ret = j;
                    frame_timestamp[0] = nack_time;//timestamp;
                    //complete[0] = 1;
                    sprintf(complete, "%d", ref_idc);
                    strcat(complete, ",complete");
                }
                else if(idr_flag)
                {
                    if(sps_flag && pps_flag && marker && (!loss_num))
                    {
                        //complete[0] = 1;
                        sprintf(complete, "%d", ref_idc);
                        strcat(complete, ",complete");
                    }
                    else if(delay >= delay_time)
                    {
                        sprintf(complete, "%d", ref_idc);
                        float flrate = (float)(loss_num) / (float)(j + loss_num);
                        int ilrate = (int)(flrate * 100);
                        strcat(complete, ",loss:");
                        sprintf(&complete[strlen(complete)],"%d", ilrate);
                        if(delay > (delay_time << 1) && packet_distance > (obj->buf_size >> 3))
                        {
                            printf("warning: get_frame: sps_flag= %d, marker= %d, loss_num= %d \n", sps_flag, marker, loss_num);
                            printf("warning: get_frame: delay= %d, ret= %d, packet_distance= %d \n", delay, ret, packet_distance);
                        }

                    }
                    else{
                        ret = 0;
                    }
                }
                else
                {
                    if(first_slice && marker && (!loss_num))
                    {
                        //complete[0] = 1;
                        sprintf(complete, "%d", ref_idc);
                        strcat(complete, ",complete");
                    }
                    else if(delay >= delay_time)
                    {
                        //(1 << 32) / 90 / 1000 / 3600 = 13.256 (hours)
                        uint32_t start_time_stamp = obj->start_time_stamp;
                        //uint32_t last_frame_time_stamp = obj->last_frame_time_stamp;
                        int diff_time0 = (int)(last_timestamp - start_time_stamp) / 90;//90000Hz
                        if(start_time_stamp > last_timestamp)
                        {
                            diff_time0 = (int)((last_timestamp + LEFT_SHIFT32 - start_time_stamp) / 90);
                        }
                        now_ms = api_get_time_stamp_ll();
                        int diff_time1 = (int)(now_ms - obj->start_frame_time);
                        int delay2 = (int)(diff_time1 - diff_time0);
                        if(start_time_stamp && delay2 < delay_time)
                        {
                            //如果编码时间间隔本身较长，这里可以进行抑制
                            //如何编码取帧过慢？
                            ret = 0;
                        }
                        else
                        {
                            sprintf(complete, "%d", ref_idc);
                            float flrate = (float)(loss_num) / (float)(j + loss_num);
                            int ilrate = (int)(flrate * 100);
                            strcat(complete, ",loss:");
                            sprintf(&complete[strlen(complete)],"%d", ilrate);
                            if(delay > (delay_time * 3) && packet_distance > (obj->buf_size >> 3))
                            {
                                printf("warning: video: get_frame: delay= %d, ret= %d, packet_distance= %d \n", delay, ret, packet_distance);

                            }
                            //if(timestamp0 == obj->last_frame_time_stamp)
                            if(0)
                            {
                                printf("get_frame: ===========================\n");
                                printf("get_frame: delay= %d \n", delay);
                                printf("get_frame: delay2= %d \n", delay2);
                                printf("get_frame: fec_k= %d \n", fec_k);
                                printf("get_frame: fec_n= %d \n", fec_n);
                                printf("get_frame: obj->min_packet= %d \n", obj->min_packet);
                                printf("get_frame: obj->max_packet= %d \n", obj->max_packet);
                                printf("get_frame: obj->old_seqnum= %d \n", obj->old_seqnum);
                                //printf("get_frame: size0= %d \n", size0);
                                //printf("get_frame: size1= %d \n", size1);
                                //printf("get_frame: timestamp0= %u \n", timestamp0);
                                //printf("get_frame: obj->last_frame_time_stamp= %u \n", obj->last_frame_time_stamp);
                                printf("get_frame: ret= %d \n", ret);
                                printf("get_frame: first_slice= %d \n", first_slice);
                                printf("get_frame: marker= %d \n", marker);
                                printf("get_frame: loss_num= %d \n", loss_num);
                                printf("get_frame: ===========================\n");
                            }

                        }
                    }
                    else{
                        ret = 0;
                    }
                }
                //
                if (ret > 0)
                {
                    if(!test_end_packet)
                    {
                        //printf("warning: get_frame: start_seq_num= %d \n", start_seq_num);
                        //printf("warning: get_frame: stop_seq_num= %d \n", stop_seq_num);
                    }
                    if(last_timestamp)
                    {
                        if(last_timestamp == obj->last_frame_time_stamp)
                        {
                            printf("error: get_frame: last_timestamp= %u \n", last_timestamp);
                        }
                        obj->last_frame_time_stamp = last_timestamp;//old packet if timestamp < last_frame_timestamp_
                    }
                    //obj->last_frame_time = api_get_time_stamp_ll();
                    if(!obj->start_time_stamp)
                    {
                        obj->start_time_stamp = last_timestamp;
                    }
                    if(!obj->start_frame_time)
	                {
	                    obj->start_frame_time = api_get_time_stamp_ll();
	                }
	                int test_old_seqnum = obj->old_seqnum;
                    obj->old_seqnum = stop_seq_num;//start_seq_num;
                    //clear buffer
                    int start = start_seq_num;
                    int end = stop_seq_num;
                    if(start > end)
                    {
                        end += LEFT_SHIFT16;//MAX_USHORT;
                    }
                    //for(int i = obj->min_packet; i < obj->max_packet; i++)
                    for(int i = start; i <= end; i++)
                    {
                        //if(i == LEFT_SHIFT16)
                        //{
                        //    continue;
                        //}
                        int this_seq_num = i % LEFT_SHIFT16;//i > MAX_USHORT ? (i - MAX_USHORT) : i;
                        int I = this_seq_num % obj->buf_size;
                        //memcpy(&obj->recv_buf[I][sizeof(int)], data, insize);
                        uint8_t *buf = (uint8_t *)&obj->recv_buf[I][sizeof(int)];
                        int *p = (int *)&obj->recv_buf[I][0];
                        p[0] = 0;
                    }
                    //reset min/max seqnum
                    int withpacket = 0;
                    uint16_t seqnum = stop_seq_num + 1;
                    if(stop_seq_num == MAX_USHORT)
                    {
                        seqnum = 0;
                    }
                    for (int i = 0; i < obj->buf_size; ++i)
                    {
                        int I = seqnum % obj->buf_size;
                        //memcpy(&obj->recv_buf[I][sizeof(int)], data, insize);
                        uint8_t *buf = (uint8_t *)&obj->recv_buf[I][sizeof(int)];
                        int *p = (int *)&obj->recv_buf[I][0];
                        //p[0] = 0;
                        int size = p[0];
                        //
                        if(obj->max_packet < QUART_USHORT && seqnum > HALF_QUART_USHORT)
                        {
                            //printf("warning: get_frame: obj->max_packet= %u \n", obj->max_packet);
                        }
                        else if(seqnum >= obj->max_packet)
                        {
                            if(size > 0)
                            {
                                //printf("get_frame: obj->max_packet= %d \n", obj->max_packet);
                                //printf("get_frame: stop_seq_num= %d \n", stop_seq_num);
                                //printf("get_frame: seqnum= %d \n", seqnum);
                                obj->min_packet = seqnum;
                                withpacket = 1;
                            }
                            break;
                        }
                        //
                        if(size > 0)
                        {
                            obj->min_packet = seqnum;
                            withpacket = 1;
                            break;
                        }
                        //
                        if(seqnum == MAX_USHORT)
                        {
                            seqnum = 0;
                        }
                        else {
                            seqnum++;
                        }
                    }
                    if(!withpacket)
                    {
                        if(stop_seq_num == obj->max_packet)
                        {
                            //last_frame_timestamp_保证旧包不会入栈
                            ///min_seq_num_ = MAX_USHORT;
                            ///max_seq_num_ = 0;
                            //test
                            obj->min_packet = stop_seq_num + 1;
                            obj->max_packet = stop_seq_num + 1;
                       }
                        else {
                            //obj->min_packet = obj->max_packet;
                            obj->min_packet = stop_seq_num + 1;
                            obj->max_packet = stop_seq_num + 1;
                        }
                    }
                    if (obj->logfp && obj->loglevel == 4)
                    {
                        //printf("get_frame: obj->min_packet= %d \n", obj->min_packet);
                        //printf("get_frame: obj->logfp= %x \n", obj->logfp);

                        fprintf(obj->logfp, "get_frame:test_old_seqnum= %d \n", test_old_seqnum);
                        fprintf(obj->logfp, "get_frame:stop_seq_num= %d \n", stop_seq_num);
                        fprintf(obj->logfp, "get_frame:obj->min_packet= %d \n", obj->min_packet);
                        fprintf(obj->logfp, "get_frame:obj->max_packet= %d \n", obj->max_packet);
                        fflush(obj->logfp);
                        //printf("get_frame: obj->max_packet= %d \n", obj->max_packet);
                    }
                    if(adapt_cpu)
                    {
                        //int ref_idx = 0;
                        //int refs = 0;
                        //int pict_type = 0;
                        int skip_frame = adapt_by_cpu(ref_idx, refs, pict_type);
                        if(skip_frame)
                        {
                            ret = 0;
                        }
                    }
                    if(is_header)
                    {
                        strcat(complete, ",is_header");
                    }
                }
                else{
                    //printf("get_frame: j= %d \n", j);
                    //printf("get_frame: delay= %d \n", delay);
                    //printf("get_frame: obj->min_packet= %d \n", obj->min_packet);
                    //printf("get_frame: obj->max_packet= %d \n", obj->max_packet);
                }

            }//
        }
     }
    //printf("get_frame: end: ret= %d \n", ret);
    return ret;
}

static int add_delay(DelayInfo *delay_info, int delay)
{
    int ret = delay;
    if(delay >= 0)
    {
        for(int i = 0; i < NETN; i++)
        {
            if(delay_info[i].max_delay < delay)
            {
                delay_info[i].max_delay = delay;
            }
            delay_info[i].sum_delay += delay;
            delay_info[i].cnt_delay += 1;
            if(delay_info[i].cnt_delay >= delay_info[i].cnt_max)
            {
                delay_info[i].delay = delay_info[i].sum_delay / delay_info[i].cnt_max;
                delay_info[i].last_max_delay = delay_info[i].max_delay;
                delay_info[i].max_delay = delay;
                delay_info[i].sum_delay = 0;
                delay_info[i].cnt_delay = 0;
            }
            //else{
            //    delay_info[i].delay = delay_info[i].sum_delay / delay_info[i].cnt_delay;
            //}
        }
        if(delay_info[1].delay > 0)
        {
            ret = delay_info[1].max_delay;
            if(delay_info[1].last_max_delay > 0)
            {
                ret = delay_info[1].last_max_delay;
            }
            if(ret < delay_info[2].delay)//瞬间波谷，容许在下个大周期更新；
            {
                ret = delay_info[2].delay;
            }
            //if(ret > delay_info[2].last_max_delay)//瞬间峰值
            //{
            //    ret = delay_info[2].last_max_delay;
            //}
            //持续偏大，必须快速更新
            //if(ret < delay_info[1].delay)
            //{
            //    ret = delay_info[1].delay;
            //}
        }
    }
    return ret;
}
// 与rtt不同，loss_rate更具突发性
static int add_loss_rate(LossRateInfo *loss_rate_info, int loss_rate)
{
    int ret = loss_rate;
    int64_t now_time = get_sys_time();
    if(loss_rate >= 0)
    {
        for(int i = 0; i < NETN; i++)
        {
            //printf("add_loss_rate: i=%d \n", i);
            if(!loss_rate_info[i].now_time)
            {
                loss_rate_info[i].now_time = now_time;
            }
            int difftime = now_time - loss_rate_info[i].now_time;
            if(loss_rate_info[i].max_loss_rate < loss_rate)
            {
                loss_rate_info[i].max_loss_rate = loss_rate;
            }
            loss_rate_info[i].sum_loss_rate += loss_rate;
            loss_rate_info[i].cnt_loss_rate += 1;
            //printf("add_loss_rate: loss_rate_info[i].cnt_max=%d \n", loss_rate_info[i].cnt_max);
            //printf("add_loss_rate: difftime=%d \n", difftime);
            //printf("add_loss_rate: loss_rate_info[i].time_len=%d \n", loss_rate_info[i].time_len);
            if( loss_rate_info[i].cnt_loss_rate >= loss_rate_info[i].cnt_max ||
                difftime > loss_rate_info[i].time_len)
            {
                if(!loss_rate_info[i].cnt_loss_rate)
                {
                    printf("add_loss_rate: loss_rate_info[i].cnt_loss_rate=%d, i=%d \n", loss_rate_info[i].cnt_loss_rate, i);
                    printf("add_loss_rate: loss_rate_info[2].cnt_max=%d \n", loss_rate_info[2].cnt_max);
                    printf("add_loss_rate: loss_rate_info[i].cnt_max=%d \n", loss_rate_info[i].cnt_max);
                    printf("add_loss_rate: difftime=%d \n", difftime);
                    printf("add_loss_rate: loss_rate_info[i].time_len=%d \n", loss_rate_info[i].time_len);
                }
                //
                loss_rate_info[i].loss_rate = loss_rate_info[i].sum_loss_rate / loss_rate_info[i].cnt_loss_rate;//loss_rate_info[i].cnt_max;
                loss_rate_info[i].last_max_loss_rate = loss_rate_info[i].max_loss_rate;
                loss_rate_info[i].max_loss_rate = loss_rate;//将平均值作为其初始值
                loss_rate_info[i].sum_loss_rate = 0;
                loss_rate_info[i].cnt_loss_rate = 0;
                loss_rate_info[i].now_time = 0;
            }
            //else{
            //    loss_rate_info[i].loss_rate = loss_rate_info[i].sum_loss_rate / loss_rate_info[i].cnt_loss_rate;
            //}
        }
        //if(loss_rate_info[1].loss_rate > 0)
        {
            //ret = loss_rate_info[1].max_loss_rate;
            if( loss_rate_info[1].last_max_loss_rate > 0 &&
                loss_rate < loss_rate_info[1].last_max_loss_rate)
            {
                ret = loss_rate_info[1].last_max_loss_rate;
            }
            if(ret < loss_rate_info[2].loss_rate)//瞬间波谷，容许在下个大周期更新；
            {
                ret = loss_rate_info[2].loss_rate;
            }
            if( ret > loss_rate_info[2].last_max_loss_rate &&
                loss_rate_info[2].last_max_loss_rate > 0)//瞬间峰值
            {
                //ret = loss_rate_info[2].last_max_loss_rate;
            }
            //持续偏大，必须快速更新
            //if(ret < loss_rate_info[1].loss_rate)
            //{
            //    ret = loss_rate_info[1].loss_rate;
            //}
        }
    }
    return ret;
}
//需要注意：非对时情况下的错误
//rtt = ((rt0 - st0) + (rt1 - st1)) / 2 = ((rt1 - st0) - (st1 - rt0)) / 2
//rtt不受对时影响，但(rt0 - st0)必须对时
static int renew_delay_time(void *hnd, uint8_t *buf, int size, int delay_time)
{
    int ret = delay_time;
    RtpObj *obj = (RtpObj *)hnd;
    cJSON *json = obj->json;
    int selfChanId = GetvalueInt(json, "selfChanId");
    //注意：此处取的是rtt1的信息，rtt0继续转发，这就是"信道复用"
    //int *chanIdList = GetArrayValueInt(json, "chanId", &chanNum);
    if(selfChanId > 0)
    {
        NetInfo netInfo = {};
        int enable_nack = GetNetInfo((uint8_t*)buf, size, &netInfo, 0);
        if(enable_nack)
        {
            //printf("renew_delay_time: netInfo.chanId= %d, selfChanId=%d \n", netInfo.chanId, selfChanId);
            //printf("renew_delay_time: netInfo.decodeId= %d, netInfo.info_status= %d \n", netInfo.decodeId, netInfo.info_status);
            //printf("renew_delay_time: netInfo.rtt= %d \n", netInfo.rtt);
#ifndef SELF_TEST_MODE
            if(netInfo.decodeId == selfChanId)
#endif
            {
                if(netInfo.info_status)
                {
                    int diff_time = (netInfo.rtt << 3);
                    //printf("video: renew_delay_time: diff_time= %d \n", diff_time);
                    ret = add_delay(obj->delay_info, diff_time) + 1;
                }
            }
        }
    }
    return ret;
}
void renew_netinfo(void *hnd, char *buf, int size, char **net_info)
{
    RtpObj *obj = (RtpObj *)hnd;
    cJSON *json = obj->json;
    int selfChanId = GetvalueInt(json, "selfChanId");
    if(selfChanId > 0)
    {
        int loss_rate = GetvalueInt(json, "loss_rate");
        cJSON *jsonInfo = NULL;
        cJSON *jsonArray = NULL;
        int resort_type = GetvalueInt(json, "resort_type");
        if(resort_type)
        {
            jsonInfo = (cJSON *)*net_info;
        }
        int chanNum = 0;
        int *chanIdList = GetArrayValueInt(json, "chanId", &chanNum);

        NetInfo netInfo = {};
        int enable_nack = GetNetInfo((uint8_t*)buf, size, &netInfo, 1);
        if(enable_nack)
        {
            if(!netInfo.info_status)
            {
                //该信息的终极拥有者是此处的selfChanId
                //待反馈信息，非测试情况下，存在netInfo.chanId == selfChanId
                //printf("get_frame: netInfo.st0= %d \n", netInfo.st0);
                //printf("get_frame: netInfo.rt0= %d \n", netInfo.rt0);
                //printf("get_frame: netInfo.chanId= %d \n", netInfo.chanId);
                cJSON *thisjson = NULL;
                if(loss_rate > 0)
                {
                    //printf("get_frame: loss_rate= %d \n", loss_rate);
                    netInfo.loss_rate = loss_rate;// + 1;
                }
                thisjson = get_net_info2(&netInfo, thisjson);
                cJSON * jsonRet = renewJsonArray3(&jsonInfo, &jsonArray, "netInfo", thisjson);
            }
            else{
                //从中调出本端信息，其他的丢弃，此处获得的是终极信息???中级信息???
                if(chanNum > 0)
                {
                    //
                    for(int jj = 0; jj < chanNum; jj++)
                    {
                        //只是转发信息，该信息的拥有者是此处chanId的对端
                        int chanId = chanIdList[jj];
                        if((chanId + 1) == netInfo.chanId)
                        {
                            //printf("get_frame: netInfo.chanId= %d \n", netInfo.chanId);
                            //int ret3 = get_net_info(&netInfo, extend_info);
                            netInfo.decodeId = netInfo.chanId;//意味着将对端（终极接收端)的编码ID写入）
                            cJSON *thisjson = NULL;
                            thisjson = get_net_info2(&netInfo, thisjson);
                            cJSON * jsonRet = renewJsonArray3(&jsonInfo, &jsonArray, "netInfo", thisjson);
                        }
                    }
                }
            }
        }

        if(chanIdList)
        {
            free(chanIdList);
        }
        if(jsonInfo)
        {
            if(resort_type)
            {
                *net_info = (void *)jsonInfo;
            }
            else{
                char *jsonStr = api_json2str(jsonInfo);//比较耗时
                api_json_free(jsonInfo);
                strcpy(*net_info, jsonStr);
                if(strlen(*net_info) >= MAX_OUTCHAR_SIZE)
                {
                    printf("error: get_frame: strlen(net_info)= %d \n", strlen(*net_info));
                }
                api_json2str_free(jsonStr);
            }
        }
    }
}
//(seqnum % obj->buf_size) != ((seqnum + MAX_USHORT) % obj->buf_size);
//(MAX_USHORT % obj->buf_size) != 0;
//(MAX_USHORT % 1024) == 1023;
int resort_packet(void *hnd, char *data, char *outbuf, short *rtpSize, char *complete, char **net_info, char *outparam[])
{
    int ret = 0;
    //printf("resort_packet: 0 \n");
    RtpObj *obj = (RtpObj *)hnd;
    cJSON *json = obj->json;
    int insize = GetvalueInt(json, "insize");
    obj->loglevel = GetvalueInt(json, "loglevel");
    int delay_time = GetvalueInt(json, "delay_time");
    int min_distance = GetvalueInt(json, "min_distance");///10
    RTP_FIXED_HEADER  *rtp_hdr = NULL;
	FU_HEADER *fu_hdr     = NULL;
	EXTEND_HEADER *rtp_ext = NULL;
	uint32_t last_frame_time_stamp = obj->last_frame_time_stamp;

    //printf("resort_packet: delay_time= %d \n", delay_time);
	cJSON *item = cJSON_GetObjectItem(json, "buf_size");
    if(cJSON_IsNumber(item))
    {
        obj->buf_size = item->valueint;
    }
    item = cJSON_GetObjectItem(json, "min_distance");
    if(cJSON_IsNumber(item))
    {
        min_distance = item->valueint;
    }
    //printf("resort_packet: min_distance= %d \n", min_distance);
    //printf("resort_packet: obj->buf_size= %d \n", obj->buf_size);
    //分配固定  長度的緩存空間，長度參數可調
    //printf("resort_packet: obj->buf_size= %d \n", obj->buf_size);
    if(obj->recv_buf == NULL)
    {
        printf("video: resort_packet: obj->Obj_id= %x \n", obj->Obj_id);
        printf("video: resort_packet: obj->buf_size= %d \n", obj->buf_size);
        obj->recv_buf = calloc(1, sizeof(uint8_t *) * obj->buf_size);
        for(int i = 0; i < obj->buf_size; i++)
        {
            obj->recv_buf[i] = calloc(1, sizeof(uint8_t) * MAX_PACKET_SIZE);
            //printf("resort_packet: i= %d \n", i);
            //printf("resort_packet: obj->recv_buf[i]= %x \n", obj->recv_buf[i]);
        }
    }
    //printf("resort_packet: 0 \n");
    rtp_hdr = (RTP_FIXED_HEADER *)data;
    unsigned  int timestamp = rtp_hdr->timestamp;
    //printf("resort_packet: timestamp= %d \n", timestamp);
    int seqnum = rtp_hdr->seq_no;
	//seqnum = (int)(((rtp_hdr->seq_no & 0xFF) << 8) | ((rtp_hdr->seq_no >> 8)));
	//printf("resort_packet: seqnum= %d \n", seqnum);
	//get extend info
	//char *extend_info = get_extern_info(data);
    if (obj->logfp && obj->loglevel)
    {
        if(obj->loglevel == 1)
        {
            fprintf(obj->logfp, "%d \n", seqnum);
        }
        else if(obj->loglevel == 2)
        {
            fprintf(obj->logfp, "%u \n", timestamp);
        }


        fflush(obj->logfp);
    }

	//數據入列
    int is_old_packet = 0;//(timestamp > obj->last_frame_time_stamp) || (!obj->last_frame_time_stamp);
    is_old_packet =  (obj->old_seqnum >= 0) &&
            (seqnum <= obj->old_seqnum) &&
            ((obj->old_seqnum - seqnum) < HALF_USHORT);
    is_old_packet |= (obj->old_seqnum >= 0) &&
            (seqnum > obj->old_seqnum) &&
            ((seqnum - obj->old_seqnum) > HALF_USHORT);
    if(!insize)
    {
        printf("error: resort_packet: insize= %d \n", insize);
        is_old_packet  = 1;
    }
    if(is_old_packet)
    {
        //printf("resort_packet: obj->old_seqnum= %d \n", obj->old_seqnum);
        //printf("resort_packet: seqnum= %d \n", seqnum);
    }
    if(obj->old_seqnum >= 0 && false)
    {
        if(obj->old_seqnum > HALF_QUART_USHORT && seqnum < QUART_USHORT)
        {
        }
        else if(obj->old_seqnum < QUART_USHORT && seqnum > HALF_QUART_USHORT)
        {
            is_old_packet  = 1;
        }
        else if(seqnum <= obj->old_seqnum)
        {
            is_old_packet  = 1;
        }
    }
    if((obj->last_frame_time_stamp - timestamp) > LEFT_SHIFT16)
    {
    }
    else if(obj->last_frame_time_stamp && (timestamp <= obj->last_frame_time_stamp))
    {
        is_old_packet  = 1;
        //printf("resort_packet: obj->last_frame_time_stamp= %u \n", obj->last_frame_time_stamp);
        //printf("resort_packet: timestamp= %u \n", timestamp);
    }
    if(timestamp == obj->last_frame_time_stamp && !is_old_packet)
    {
        printf("error: resort_packet: is_old_packet= %d \n", is_old_packet);
    }

    if(!is_old_packet)
    {
        //非旧包
        if((obj->min_packet == MAX_USHORT) && (obj->max_packet < 0))
        {
            obj->min_packet = seqnum;
            obj->max_packet = seqnum;
        }
        else if(obj->min_packet == obj->max_packet)
        {
            int I = obj->min_packet % obj->buf_size;
            uint8_t *buf = (uint8_t *)&obj->recv_buf[I][sizeof(int)];
            int *p = (int *)&obj->recv_buf[I][0];
            int size = p[0];
            if(!size)
            {
                obj->min_packet = seqnum;
                obj->max_packet = seqnum;
            }
            else{
                //if(seqnum < obj->max_packet && (obj->old_seqnum >= 0))
                //{
                //    printf("error: resort_packet: seqnum= %d \n", seqnum);
                //    printf("error: resort_packet: obj->max_packet= %d \n", obj->max_packet);
                //    printf("error: resort_packet: obj->old_seqnum= %d \n", obj->old_seqnum);
                //}
                //else
                if(seqnum < obj->min_packet)
                {
                    obj->min_packet = seqnum;
                }
                else{
                    obj->max_packet = seqnum;
                }
            }
        }
        else{
            if(obj->min_packet > HALF_QUART_USHORT && seqnum < QUART_USHORT)
            {
            }
            else if(seqnum < obj->min_packet)
            {
                obj->min_packet = seqnum;
            }
            //else if(obj->min_packet < HALF_USHORT && (seqnum > HALF_USHORT))
            else if(obj->min_packet < QUART_USHORT && (seqnum > HALF_QUART_USHORT))
            {
                obj->min_packet = seqnum;
            }

            if(obj->max_packet < QUART_USHORT && seqnum > HALF_QUART_USHORT)
            {
            }
            else if(seqnum > obj->max_packet)
            {
                obj->max_packet = seqnum;
            }
            //else if(obj->max_packet > HALF_USHORT && (seqnum < HALF_USHORT))
            else if(obj->max_packet > HALF_QUART_USHORT && (seqnum < QUART_USHORT))
            {
                obj->max_packet = seqnum;
            }
        }
        if(obj->delay_time <= 0)
        {
            obj->delay_time = delay_time;
        }
        obj->delay_time = renew_delay_time(hnd, data, insize, obj->delay_time);
        renew_netinfo(hnd, data, insize, net_info);
        //先取後存
        int I = seqnum % obj->buf_size;
        //printf("resort_packet: I= %d \n", I);
        //printf("resort_packet: I = %d \n", I);
        //printf("resort_packet: obj->recv_buf[I]= %x \n", obj->recv_buf[I]);
        memcpy(&obj->recv_buf[I][sizeof(int)], data, insize);
        int *p = (int *)&obj->recv_buf[I][0];
        p[0] = insize;
    }
    int packet_distance = obj->max_packet - obj->min_packet;
	if(obj->min_packet > obj->max_packet)
    {
        packet_distance = ((int)obj->max_packet + LEFT_SHIFT16) - obj->min_packet;
    }
	//printf("resort_packet: packet_distance= %d \n", packet_distance);
	//if(obj->max_packet >= 0 && obj->min_packet >= 0)
    {
        if(packet_distance > min_distance)
        {
            //printf("get_frame: \n");
            //從緩存中獲取一幀數據
            long long frame_timestamp = 0;
            ret = get_frame(hnd, outbuf, rtpSize, complete, net_info, &frame_timestamp);
            //printf("resort_packet: strlen(*net_info)= %d \n", strlen(*net_info));
            //printf("resort_packet: ret= %d \n", ret);
            if(ret > 0)
            {
                sprintf(outparam[3], "%lld", frame_timestamp);
            }
        }
        //先處理切換點
        //1111****3333****
        //seqnum = correct_seqnum(hnd, seqnum);

    }
    //printf("resort_packet: 4 \n");
    //printf("resort_packet: end: ret= %d \n", ret);
    return ret;
}
HCSVC_API
int api_resort_packet(char *handle, char *data, char *param, char *outbuf, char *outparam[])
{
    int ret = 0;
    //initobj(id);
    //printf("api_resort_packet: 0 \n");
    //return ret;//test
    //if (id < MAX_OBJ_NUM)
    {
        long long *testp = (long long *)handle;
        CodecObj *codecObj = (CodecObj *)testp[0];
        RtpObj *obj = codecObj->resortObj;//(RtpObj *)&global_rtp_objs[id];
        int id = codecObj->Obj_id;
        if (!obj)
        {
            obj = (RtpObj *)calloc(1, sizeof(RtpObj));
            codecObj->resortObj = obj;

            obj->pRet = (StructPointer)calloc(1, sizeof(StructPointerTest));
            for(int i = 0; i < MAX_RESORT_FRAME_NUM; i++)
            {
                obj->pRet->complete[i] = (char *)calloc(1, 32 * sizeof(char));
            }

            strcpy(obj->pRet->name, "api_resort_packet_all");

            ResetObj(obj);
            obj->Obj_id = id;
            //
#if 1
            obj->json = (cJSON *)api_str2json(param);
            cJSON *json = obj->json;
            obj->loglevel = GetvalueInt(json, "loglevel");
            if(!obj->logfp && obj->loglevel)
            {
                char filename[256] = "/home/gxh/works/rtp_resort_gxh_";
                char ctmp[32] = "";
                int fileidx = id;//random() % 10000;
                sprintf(ctmp, "%d", fileidx);
                strcat(filename, ctmp);
                strcat(filename, ".txt");
                obj->logfp = fopen(filename, "w");
            }
#endif
        }

        if(!obj->json)
        {
            obj->json = (cJSON *)api_str2json(param);
        }
        obj->param = param;
        int selfChanId = GetvalueInt(obj->json, "selfChanId");
        int insize = GetvalueInt(obj->json, "insize");
        if(!insize)
        {
            printf("error: api_resort_packet: insize= %d \n", insize);
            printf("error: api_resort_packet: param= %s \n", param);
        }

        //printf("api_resort_packet: 0 \n");
        short rtpSize[MAX_FEC_PKT_NUM];
        //int complete = 0;
        char *net_info = NULL;
        //printf("api_resort_packet: selfChanId=%d \n", selfChanId);
        if(selfChanId > 0)
        {
            net_info = obj->outparam[1];
            memset(obj->outparam[1], 0, sizeof(char) * MAX_OUTCHAR_SIZE);
        }
        memset(obj->outparam[2], 0, sizeof(char) * 16);
        memset(obj->outparam[3], 0, sizeof(char) * 16);
        outparam[0] = obj->outparam[0];
        outparam[1] = obj->outparam[1];
        outparam[2] = obj->outparam[2];
        outparam[3] = obj->outparam[3];
        char *complete = (char *)outparam[2];
        //printf("api_resort_packet: 1 \n");
        ret = resort_packet(obj, data, outbuf, rtpSize, complete, &net_info, outparam);
        //printf("api_resort_packet: 2 \n");
        //printf("api_resort_packet: strlen(net_info)= %d \n", strlen(net_info));
        if(ret > 0)// && !strlen(outparam[0]) && !strlen(outparam[1]))
        {
            //char text[2048] = "";//2048/4=512//512*1400=700kB
            //printf("api_resort_packet: tail \n");
            if(selfChanId > 0)
            {
                //printf("api_resort_packet: strlen(net_info)=%d \n", strlen(net_info));
            }
            char *text = obj->outparam[0];
	        int sum = 0;
	        memset(obj->outparam[0], 0, sizeof(char) * MAX_OUTCHAR_SIZE);
#if 1
            for (int i = 0; i < ret; i++)
	        {
	            int size = rtpSize[i];
                sum += size;
	        }

            cJSON *json = NULL;
            //json = renewJsonArray2(json, "oRtpSize", rtpSize);
            json = renewJsonArray1(json, "oRtpSize", rtpSize, ret);
            char *jsonStr = api_json2str(json);
            strcpy(outparam[0], jsonStr);
            api_json_free(json);
            api_json2str_free(jsonStr);

            ret = sum;
#else
	        for (int i = 0; i < ret; i++)
	        {
                char ctmp[32] = "";
                int size = rtpSize[i];
                sum += size;
	            sprintf(ctmp, "%d", size);
	            if(i)
	            {
	                strcat(text, ",");
	            }
                strcat(text, ctmp);
	        }
	        ret = sum;
	        outparam[0] = text;
#endif
	        //if(complete)
	        //{
	        //    outparam[2] = "complete";
	        //}

	        //printf("api_resort_packet: sum= %d \n", sum);
        }
        //printf("api_resort_packet: end: ret= %d \n", ret);
        //printf("api_resort_packet: 3 \n");
        api_json_free(obj->json);
        obj->json = NULL;
        //printf("api_resort_packet: 4 \n");
    }
    return ret;
}
int resort_packet2(void *hnd, char *data, int insize, char *outbuf, short *rtpSize, char *complete, char **net_info, int64_t *frame_timestamp)
{
    int ret = 0;
    //printf("resort_packet: 0 \n");
    RtpObj *obj = (RtpObj *)hnd;
    cJSON *json = obj->json;
    obj->loglevel = GetvalueInt(json, "loglevel");
    int delay_time = GetvalueInt(json, "delay_time");
    int min_distance = GetvalueInt(json, "min_distance");///10
    RTP_FIXED_HEADER  *rtp_hdr = NULL;
	FU_HEADER *fu_hdr     = NULL;
	EXTEND_HEADER *rtp_ext = NULL;
	uint32_t last_frame_time_stamp = obj->last_frame_time_stamp;

    //printf("resort_packet: delay_time= %d \n", delay_time);
	cJSON *item = cJSON_GetObjectItem(json, "buf_size");
    if(cJSON_IsNumber(item))
    {
        obj->buf_size = item->valueint;
    }
    item = cJSON_GetObjectItem(json, "min_distance");
    if(cJSON_IsNumber(item))
    {
        min_distance = item->valueint;
    }
    //printf("resort_packet: min_distance= %d \n", min_distance);
    //printf("resort_packet: obj->buf_size= %d \n", obj->buf_size);
    //分配固定  長度的緩存空間，長度參數可調
    //printf("resort_packet: obj->buf_size= %d \n", obj->buf_size);
    if(obj->recv_buf == NULL)
    {
        printf("video: resort_packet: obj->Obj_id= %x \n", obj->Obj_id);
        printf("video: resort_packet: obj->buf_size= %d \n", obj->buf_size);
        obj->recv_buf = calloc(1, sizeof(uint8_t *) * obj->buf_size);
        for(int i = 0; i < obj->buf_size; i++)
        {
            obj->recv_buf[i] = calloc(1, sizeof(uint8_t) * MAX_PACKET_SIZE);
            //printf("resort_packet: i= %d \n", i);
            //printf("resort_packet: obj->recv_buf[i]= %x \n", obj->recv_buf[i]);
        }
    }
    //printf("resort_packet: 0 \n");
    rtp_hdr = (RTP_FIXED_HEADER *)data;
    unsigned  int timestamp = rtp_hdr->timestamp;
    //printf("resort_packet: timestamp= %d \n", timestamp);
    int seqnum = rtp_hdr->seq_no;
	//seqnum = (int)(((rtp_hdr->seq_no & 0xFF) << 8) | ((rtp_hdr->seq_no >> 8)));
	//printf("resort_packet: seqnum= %d \n", seqnum);
	//get extend info
	//char *extend_info = get_extern_info(data);
    if (obj->logfp && obj->loglevel)
    {
        if(obj->loglevel == 1)
        {
            fprintf(obj->logfp, "%d \n", seqnum);
        }
        else if(obj->loglevel == 2)
        {
            fprintf(obj->logfp, "%u \n", timestamp);
        }


        fflush(obj->logfp);
    }

	//數據入列
    int is_old_packet = 0;//(timestamp > obj->last_frame_time_stamp) || (!obj->last_frame_time_stamp);
    is_old_packet =  (obj->old_seqnum >= 0) &&
            (seqnum <= obj->old_seqnum) &&
            ((obj->old_seqnum - seqnum) < HALF_USHORT);
    is_old_packet |= (obj->old_seqnum >= 0) &&
            (seqnum > obj->old_seqnum) &&
            ((seqnum - obj->old_seqnum) > HALF_USHORT);
    if(!insize)
    {
        printf("error: resort_packet: insize= %d \n", insize);
        is_old_packet  = 1;
    }
    if(is_old_packet)
    {
        //printf("resort_packet: obj->old_seqnum= %d \n", obj->old_seqnum);
        //printf("resort_packet: seqnum= %d \n", seqnum);
    }
    if(obj->old_seqnum >= 0 && false)
    {
        if(obj->old_seqnum > HALF_QUART_USHORT && seqnum < QUART_USHORT)
        {
        }
        else if(obj->old_seqnum < QUART_USHORT && seqnum > HALF_QUART_USHORT)
        {
            is_old_packet  = 1;
        }
        else if(seqnum <= obj->old_seqnum)
        {
            is_old_packet  = 1;
        }
    }
    if((obj->last_frame_time_stamp - timestamp) > LEFT_SHIFT16)
    {
    }
    else if(obj->last_frame_time_stamp && (timestamp <= obj->last_frame_time_stamp))
    {
        is_old_packet  = 1;
        //printf("resort_packet: obj->last_frame_time_stamp= %u \n", obj->last_frame_time_stamp);
        //printf("resort_packet: timestamp= %u \n", timestamp);
    }
    if(timestamp == obj->last_frame_time_stamp && !is_old_packet)
    {
        printf("error: resort_packet: is_old_packet= %d \n", is_old_packet);
    }

    if(!is_old_packet)
    {
        //非旧包
        if((obj->min_packet == MAX_USHORT) && (obj->max_packet < 0))
        {
            obj->min_packet = seqnum;
            obj->max_packet = seqnum;
        }
        else if(obj->min_packet == obj->max_packet)
        {
            int I = obj->min_packet % obj->buf_size;
            uint8_t *buf = (uint8_t *)&obj->recv_buf[I][sizeof(int)];
            int *p = (int *)&obj->recv_buf[I][0];
            int size = p[0];
            if(!size)
            {
                obj->min_packet = seqnum;
                obj->max_packet = seqnum;
            }
            else{
                //if(seqnum < obj->max_packet && (obj->old_seqnum >= 0))
                //{
                //    printf("error: resort_packet: seqnum= %d \n", seqnum);
                //    printf("error: resort_packet: obj->max_packet= %d \n", obj->max_packet);
                //    printf("error: resort_packet: obj->old_seqnum= %d \n", obj->old_seqnum);
                //}
                //else
                if(seqnum < obj->min_packet)
                {
                    obj->min_packet = seqnum;
                }
                else{
                    obj->max_packet = seqnum;
                }
            }
        }
        else{
            if(obj->min_packet > HALF_QUART_USHORT && seqnum < QUART_USHORT)
            {
            }
            else if(seqnum < obj->min_packet)
            {
                obj->min_packet = seqnum;
            }
            //else if(obj->min_packet < HALF_USHORT && (seqnum > HALF_USHORT))
            else if(obj->min_packet < QUART_USHORT && (seqnum > HALF_QUART_USHORT))
            {
                obj->min_packet = seqnum;
            }

            if(obj->max_packet < QUART_USHORT && seqnum > HALF_QUART_USHORT)
            {
            }
            else if(seqnum > obj->max_packet)
            {
                obj->max_packet = seqnum;
            }
            //else if(obj->max_packet > HALF_USHORT && (seqnum < HALF_USHORT))
            else if(obj->max_packet > HALF_QUART_USHORT && (seqnum < QUART_USHORT))
            {
                obj->max_packet = seqnum;
            }
        }
        if(obj->delay_time <= 0)
        {
            obj->delay_time = delay_time;
        }
        obj->delay_time = renew_delay_time(hnd, data, insize, obj->delay_time);
        renew_netinfo(hnd, data, insize, net_info);
        //先取後存
        int I = seqnum % obj->buf_size;
        //printf("resort_packet: I= %d \n", I);
        //printf("resort_packet: I = %d \n", I);
        //printf("resort_packet: obj->recv_buf[I]= %x \n", obj->recv_buf[I]);
        memcpy(&obj->recv_buf[I][sizeof(int)], data, insize);
        int *p = (int *)&obj->recv_buf[I][0];
        p[0] = insize;
    }

    int packet_distance = obj->max_packet - obj->min_packet;
	if(obj->min_packet > obj->max_packet)
    {
        packet_distance = ((int)obj->max_packet + LEFT_SHIFT16) - obj->min_packet;
    }
	//printf("resort_packet: packet_distance= %d \n", packet_distance);
	//if(obj->max_packet >= 0 && obj->min_packet >= 0)
    {
        if(packet_distance > min_distance)
        {
            //printf("get_frame: \n");
            //從緩存中獲取一幀數據
            //long long frame_timestamp = 0;
            ret = get_frame(hnd, outbuf, rtpSize, complete, net_info, &frame_timestamp);
            //printf("resort_packet: strlen(net_info)= %d \n", strlen(net_info));
            //printf("resort_packet: ret= %d \n", ret);
            //if(ret > 0)
            //{
            //    sprintf(outparam[3], "%lld", frame_timestamp);
            //}
        }
        //先處理切換點
        //1111****3333****
        //seqnum = correct_seqnum(hnd, seqnum);

    }
    //printf("resort_packet: 4 \n");
    //printf("resort_packet: end: ret= %d \n", ret);
    return ret;
}
#if 0
HCSVC_API
int api_renew_netinfo(char *handle, char *data, int insize, cJSON **jsonInfo)
{
    long long *testp = (long long *)handle;
    CodecObj *codecObj = (CodecObj *)testp[0];
    RtpObj *obj = codecObj->resortObj;
    if(obj)
    {
        //obj->delay_time = renew_delay_time(hnd, data, insize, obj->delay_time);
        //renew_netinfo(hnd, data, insize, net_info);
        cJSON *json = obj->json;
        int selfChanId = GetvalueInt(json, "selfChanId");
        if(selfChanId > 0)
        {
            int loss_rate = GetvalueInt(json, "loss_rate");
            //cJSON *jsonInfo = NULL;
            cJSON *jsonArray = NULL;
            //int resort_type = GetvalueInt(json, "resort_type");

            int chanNum = 0;
            int *chanIdList = GetArrayValueInt(json, "chanId", &chanNum);

            NetInfo netInfo = {};
            int enable_nack = GetNetInfo((uint8_t*)buf, size, &netInfo, 1);
            if(enable_nack)
            {
                if(!netInfo.info_status)
                {
                    //该信息的终极拥有者是此处的selfChanId
                    //待反馈信息，非测试情况下，存在netInfo.chanId == selfChanId
                    //printf("get_frame: netInfo.st0= %d \n", netInfo.st0);
                    //printf("get_frame: netInfo.rt0= %d \n", netInfo.rt0);
                    //printf("get_frame: netInfo.chanId= %d \n", netInfo.chanId);
                    cJSON *thisjson = NULL;
                    if(loss_rate > 0)
                    {
                        //printf("get_frame: loss_rate= %d \n", loss_rate);
                        netInfo.loss_rate = loss_rate;// + 1;
                    }
                    thisjson = get_net_info2(&netInfo, thisjson);
                    cJSON * jsonRet = renewJsonArray3(&jsonInfo, &jsonArray, "netInfo", thisjson);
                }
                else{
                    //从中调出本端信息，其他的丢弃，此处获得的是终极信息???中级信息???
                    if(chanNum > 0)
                    {
                        //
                        for(int jj = 0; jj < chanNum; jj++)
                        {
                            //只是转发信息，该信息的拥有者是此处chanId的对端
                            int chanId = chanIdList[jj];
                            if((chanId + 1) == netInfo.chanId)
                            {
                                //printf("get_frame: netInfo.chanId= %d \n", netInfo.chanId);
                                //int ret3 = get_net_info(&netInfo, extend_info);
                                netInfo.decodeId = netInfo.chanId;//意味着将对端（终极接收端)的编码ID写入）
                                cJSON *thisjson = NULL;
                                thisjson = get_net_info2(&netInfo, thisjson);
                                cJSON * jsonRet = renewJsonArray3(&jsonInfo, &jsonArray, "netInfo", thisjson);
                            }
                        }
                    }
                }
            }
            if(chanIdList)
            {
                free(chanIdList);
            }
        }
    }
}
#endif
HCSVC_API
StructPointer api_resort_packet_all(char *handle, char *dataList[], int *dataSize, int num, char *param, char *outbuf, char *outparam[])
{
    int ret = 0;
    long long *testp = (long long *)handle;
    CodecObj *codecObj = (CodecObj *)testp[0];
    RtpObj *obj = codecObj->resortObj;//(RtpObj *)&global_rtp_objs[id];
    int id = codecObj->Obj_id;
    if (!obj)
    {
        obj = (RtpObj *)calloc(1, sizeof(RtpObj));
        codecObj->resortObj = obj;
        obj->pRet = (StructPointer)calloc(1, sizeof(StructPointerTest));
        for(int i = 0; i < MAX_RESORT_FRAME_NUM; i++)
        {
            obj->pRet->complete[i] = (char *)calloc(1, 32 * sizeof(char));
        }

        strcpy(obj->pRet->name, "api_resort_packet_all");
        ResetObj(obj);
        obj->Obj_id = id;
        //
        obj->json = (cJSON *)api_str2json(param);
        cJSON *json = obj->json;
        obj->loglevel = GetvalueInt(json, "loglevel");
        int selfChanId = GetvalueInt(obj->json, "selfChanId");
        if(!obj->logfp && obj->loglevel)
        {
            char filename[256] = "/home/gxh/works/rtp_resort_gxh_";
            char ctmp[32] = "";
            int fileidx = selfChanId;//id;//random() % 10000;
            sprintf(ctmp, "%d", fileidx);
            strcat(filename, ctmp);
            strcat(filename, ".txt");
            obj->logfp = fopen(filename, "w");
        }
    }
    if(!obj->json)
    {
        obj->json = (cJSON *)api_str2json(param);
    }
    obj->param = param;
    int selfChanId = GetvalueInt(obj->json, "selfChanId");
    int loss_rate = 0;//GetvalueInt(obj->json, "loss_rate");
    //int max_loss_rate = loss_rate;
    int frame_num = 0;

    int offset = 0;
    int k = 0;
    obj->pRet->frame_num = 0;
    void *pNetInfo = NULL;
    outparam[1] = obj->outparam[1];
    memset(obj->outparam[1], 0, sizeof(char) * MAX_OUTCHAR_SIZE);
    //printf("api_resort_packet_all: num= %d\n",num);
    for(int i = 0; i < num; i++)
    {
        //printf("api_resort_packet: dataSize[i]= %d \n", dataSize[i]);
        //printf("api_resort_packet: dataList[i]= %x \n", dataList[i]);
        char *dataPtr = (char *)dataList[i];
        int isrtp = api_isrtp(dataPtr, dataSize[i], 0, H264_PLT);
        if(isrtp != 1)
        {
            printf("api_resort_packet: isrtp= %d \n", isrtp);
            continue;
        }
        short rtpSize[MAX_FEC_PKT_NUM];
        int64_t frame_timestamp = 0;
        memset(obj->outparam[2], 0, sizeof(char) * 16);
        memset(obj->outparam[3], 0, sizeof(char) * 16);
        char *complete = (char *)obj->outparam[2];

        int this_loss_rate = api_count_loss_rate2(handle, dataPtr, dataSize[i], 90);
        if(this_loss_rate > 0)
        {
            loss_rate = this_loss_rate;
            char *key = "loss_rate";
		    int ivalue = loss_rate;
		    obj->json = api_renew_json_int(obj->json, key, ivalue);
		    if(this_loss_rate > 1)
		    {
		        if(obj->loglevel == 3)
                {
                    fprintf(obj->logfp, "%d \n", this_loss_rate);
                }
		    }
        }
        if(frame_num < MAX_RESORT_FRAME_NUM)
        {
            memset(obj->pRet->complete[frame_num], 0, sizeof(char) * 16);
        }
        //pNetInfo = NULL;//test
        int ret2 = resort_packet2(obj, dataPtr, dataSize[i], &outbuf[offset], rtpSize, complete, &pNetInfo, &frame_timestamp);

        if(ret2 > 0)
        {
            if(frame_num < MAX_RESORT_FRAME_NUM)
            {
                int sum = 0;
                for (int j = 0; j < ret2; j++)
	            {
	                int size = rtpSize[j];
	                obj->pRet->rtp_size[k] = size;
	                k++;
                    sum += size;
	            }
	            obj->pRet->frame_size[frame_num] = sum;
	            obj->pRet->pkt_num[frame_num] = ret2;
	            obj->pRet->frame_timestamp[frame_num] = frame_timestamp;
	            strcpy(obj->pRet->complete[frame_num], complete);
	            offset += sum;
            }
            frame_num++;
            if(frame_num >= (MAX_RESORT_FRAME_NUM - 1) && obj->loglevel)
            {
                printf("warning: api_resort_packet_all: frame_num= %d \n", frame_num);
            }
        }
    }
    cJSON *jsonInfo = (cJSON *)pNetInfo;
    if(selfChanId > 0)
    {
        if(jsonInfo)
        {
#if 0
            int arraySize = 0;
            long long *objList = GetArrayObj(jsonInfo, "netInfo", &arraySize);
            printf("api_resort_packet_all: arraySize= %d\n",arraySize);
            if(objList)
            {
                for(int j = 0; j < arraySize; j++)
                {
                    cJSON *thisjson = (cJSON *)objList[j];
                    if(thisjson)
                    {
                        int chanId = GetvalueInt(thisjson, "chanId");//已增1
                        int info_status = GetvalueInt(thisjson, "info_status");
                        printf("api_resort_packet_all: chanId= %d \n", chanId);
                        printf("api_resort_packet_all: info_status= %d \n", info_status);
                    }
                }
                free(objList);
            }
#endif
            char *net_info = obj->outparam[1];

            char *jsonStr = api_json2str(jsonInfo);//比较耗时
            api_json_free(jsonInfo);
            strcpy(net_info, jsonStr);
            if(strlen(net_info) >= MAX_OUTCHAR_SIZE)
            {
                printf("error: get_frame: strlen(net_info)= %d \n", strlen(net_info));
            }
            api_json2str_free(jsonStr);
        }
    }
    if(frame_num >= (MAX_RESORT_FRAME_NUM - 1) && obj->loglevel)
    {
        printf("warning: api_resort_packet_all: frame_num= %d \n", frame_num);
    }

    api_json_free(obj->json);
    obj->json = NULL;
    obj->pRet->frame_num = frame_num;
    obj->pRet->loss_rate = loss_rate;
    //return ret;
    return obj->pRet;
}
HCSVC_API
int api_renew_delay_time(void *handle, int selfChanId, uint8_t *buf, int size, int delay_time)
{
    int ret = 0;
    //if (id < MAX_OBJ_NUM)
    {
        long long *testp = (long long *)handle;
        CodecObj *codecObj = (CodecObj *)testp[0];
        RtpObj *obj = codecObj->resortObj;//(RtpObj *)&global_rtp_objs[id];
        int id = codecObj->Obj_id;
        if (!obj)
        {
            obj = (RtpObj *)calloc(1, sizeof(RtpObj));
            codecObj->resortObj = obj;
            ResetObj(obj);
            obj->Obj_id = id;
        }
        NetInfo netInfo = {};
        int enable_nack = GetNetInfo((uint8_t*)buf, size, &netInfo, 0);

        if(enable_nack)
        {
            //printf("video: api_renew_delay_time: netInfo.info_status= %d \n", netInfo.info_status);
            if(netInfo.info_status)
            {
                //printf("video: api_renew_delay_time: netInfo.decodeId= %d \n", netInfo.decodeId);
                //printf("video: api_renew_delay_time: selfChanId= %d \n", selfChanId);
            }
            //printf("video: api_renew_delay_time: netInfo.info_status=%d, netInfo.decodeId=%d, selfChanId= %d \n", netInfo.info_status, netInfo.decodeId, selfChanId);
#ifndef SELF_TEST_MODE
            if(netInfo.decodeId == selfChanId)
#endif
            {
                if(netInfo.info_status)
                {
                    int diff_time = (netInfo.rtt << 3);
                    //printf("video: api_renew_delay_time: diff_time= %d \n", diff_time);
                    if(diff_time > 1000)
                    {
                        printf("video: api_renew_delay_time: diff_time=%d, netInfo.decodeId=%d, selfChanId= %d \n", diff_time, netInfo.decodeId, selfChanId);
                    }

                    ret = add_delay(obj->delay_info, diff_time);
                }

            }
        }
    }
    return ret;
}
HCSVC_API
void api_test_write_str(int num)
{
    NetInfo netInfo = {0, 0, 0, 0, 0, 0, 1, 0};
    int flag = 1;
    if(flag == 1)
    {
        char pinfo[1024 * 1024];
        memset(pinfo, 0, 1024 * 1024);

        cJSON *json = NULL;
        cJSON *array = NULL;

        for(int i = 0; i < num; i++)
        {
            cJSON *thisjson = NULL;
            thisjson = get_net_info2(&netInfo, thisjson);
            cJSON * jsonRet = renewJsonArray3(&json, &array, "arraytest", thisjson);
        }
        //
        int flag2 = 1;//0;
        if(flag2)
        {
            int arraySize = 0;
            long long *objList = GetArrayObj(json, "arraytest", &arraySize);
            printf("api_test_write_str: arraySize= %d\n",arraySize);
            if(objList)
            {
                for(int i = 0; i < arraySize; i++)
                {
                    cJSON *thisjson = (cJSON *)objList[i];
                    if(thisjson)
                    {
                        int chanId = GetvalueInt(thisjson, "chanId");//已增1
                        int loss_rate = GetvalueInt(thisjson, "loss_rate");
                        int st0 = GetvalueInt(thisjson, "st0");
                        int rt0 = GetvalueInt(thisjson, "rt0");
                        int st1 = GetvalueInt(thisjson, "st1");
                        int rt1 = GetvalueInt(thisjson, "rt1");
                        printf("%d, %d, %d, %d, %d, %d \n", chanId, loss_rate, st0, rt0, st1, rt1);
                    }
                }
                free(objList);
            }
        }
        //
        char *ret = api_json2str(json);//比较耗时
        //printf("api_test_write_str: ret= %s\n",ret);
        api_json_free(json);
        strcpy(pinfo, ret);
        free(ret);
    }
    else if(flag == 2)
    {
        cJSON *json = NULL;
        char *key = "arraytest";
        if(NULL == json)
        {
            json = cJSON_CreateObject(); //创建JSON对象
        }

        cJSON *array = cJSON_CreateArray();
        cJSON_AddItemToObject(json, key, array);
        for(int i = 0; i < num; i++)
        {
            cJSON *thisjson = NULL;
            thisjson = get_net_info2(&netInfo, thisjson);
            cJSON_AddItemToArray(array, thisjson);//cJSON_CreateNumber(value[i]));
            //cJSON_AddItemToArray(array, cJSON_CreateObject(thisjson));
        }
        char *ret = api_json2str(json);
        //printf("api_test_write_str: ret= %s\n",ret);
    }
    else if(flag == 3)
    {
        cJSON *pJsonRoot = NULL;
        for(int i = 0; i < num; i++)
        {
            pJsonRoot = get_net_info2(&netInfo, pJsonRoot);
        }
    }
#if 0
    else{
        char pinfo[1024 * 1024];
        memset(pinfo, 0, 1024 * 1024);
        int sumsize = 0;
        for(int i = 0; i < num; i++)
        {

            int ret3 = get_net_info(&netInfo, pinfo);
            sumsize += ret3;
            if(sumsize >= 1024 * 1024)
            {
                break;
            }
        }
    }
#endif
}
HCSVC_API
int api_reset_seqnum(char* dataPtr, int insize, int seqnum)
{
    int ret = 0;
    RTP_FIXED_HEADER* rtp_hdr = (RTP_FIXED_HEADER*)&dataPtr[0];
    rtp_hdr->seq_no = seqnum;
    if (seqnum >= MAX_USHORT)
    {
	    seqnum = 0;
	}
	else{
	    seqnum++;
	}
	return seqnum;
}
int get_rtpheader(char* dataPtr, int *rtpSize, int size, int raw_offset)
{
    int ret = 0;
    int head_size = 0;
    int offset = 0;
    int offset1 = 0;
    for(int i = 0; i < size; i++)
    {
        int insize = rtpSize[i];
        if(i)
        {
            memcpy(&dataPtr[offset1], &dataPtr[offset], head_size);
        }
        else{
            head_size = GetRtpHeaderSize((uint8_t *)&dataPtr[raw_offset], insize);
            head_size += raw_offset;
        }
        offset += insize;
        offset1 += head_size;
        rtpSize[i] = head_size;
    }
    ret = offset1;
    return ret;
}
HCSVC_API
int api_get_rtpheader(char *data, char *param, char *outparam[])
{
    int ret = 0;
    //printf("api_get_rtpheader: param=%s \n", param);
    cJSON* json = (cJSON *)api_str2json(param);
    int size = 0;
    int *rtpSize = GetArrayValueInt(json, "rtpSize", &size);
    //
    ret = get_rtpheader(data, rtpSize, size, 0);
    //
    cJSON *json2 = NULL;
    json2 = renewJsonArray4(json2, "rtpSize", rtpSize, size);
    char *jsonStr = api_json2str(json2);//比较耗时
    api_json_free(json2);
    strcpy(outparam[0], jsonStr);
    //free(jsonStr);
    api_json2str_free(jsonStr);
    //
    if(rtpSize)
    {
        free(rtpSize);
    }
    api_json_free(json);
    return ret;
}
HCSVC_API
int api_get_rtpheader2(char* dataPtr, int *rtpSize, int size, int raw_offset)
{
    return get_rtpheader(dataPtr, rtpSize, size, raw_offset);
}
HCSVC_API
void* api_add_array2json(void **jsonInfo, void **jsonArray, void *thisjson, char *key)
{
    cJSON * thisjson2 = cJSON_Duplicate((cJSON *)thisjson, 0);//复制cjson对象
    cJSON *jsonRet = renewJsonArray3((cJSON **)jsonInfo, (cJSON **)jsonArray, key, thisjson2);
    return (void *)jsonRet;
}
HCSVC_API
void* api_add_netinfos2json(void **jsonInfo, void **jsonArray, NetInfo *netInfo)
{
    cJSON *thisjson = NULL;
    thisjson = get_net_info2(netInfo, thisjson);
    cJSON *jsonRet = renewJsonArray3((cJSON **)jsonInfo, (cJSON **)jsonArray, "netInfo", thisjson);
    return (void *)jsonRet;
}
//==============================================================================
//==============================================================================

extern void PacketManagerInit(PacketManager *obj, unsigned int ssrc, RtpInfo *info, int freq);
extern int CountLossRate2(PacketManager *manager, uint8_t* dataPtr, int insize, int freq);


HCSVC_API
int api_count_loss_rate2(void *handle, uint8_t* dataPtr, int insize, int freq)
{
    int ret = -1;//-2;
    //if (id < MAX_OBJ_NUM)
    {
        PacketManager *obj = NULL;
        long long *testp = (long long *)handle;
        int is_video = freq == 90;
        if(is_video)
        {
            CodecObj *codecObj = (CodecObj *)testp[0];
            obj = (PacketManager *)codecObj->pktManObj;//(RtpObj *)&global_rtp_objs[id];
        }
        else{
            AudioCodecObj *codecObj = (AudioCodecObj *)testp[0];
            obj = (PacketManager *)codecObj->pktManObj;//(RtpObj *)&global_rtp_objs[id];
        }

        //printf("CountLossRate2: obj=%x \n", obj);
        if (!obj)
        {
            obj = (PacketManager *)calloc(1, sizeof(PacketManager));
            if(is_video)
            {
                CodecObj *codecObj = (CodecObj *)testp[0];
                codecObj->pktManObj = obj;
            }
            else{
                AudioCodecObj *codecObj = (AudioCodecObj *)testp[0];
                codecObj->pktManObj = obj;
            }
            //ResetObj(obj);
            //obj->Obj_id = id;
            //
            RtpInfo info = {0};
            info.raw_offset = -1;
            int plt = is_video ? H264_PLT : AAC_PLT;
            int is_hcsvcrtp = GetRtpInfo(dataPtr, insize, &info, plt);
            if(is_hcsvcrtp > 0)
            {
                unsigned int ssrc = info.ssrc;
                PacketManagerInit(obj, ssrc, &info, freq);
                printf("api_count_loss_rate2: is_hcsvcrtp=%d \n", is_hcsvcrtp);
                //return -1;
            }
            else{
                printf("api_count_loss_rate2: not rtp: is_hcsvcrtp=%d \n", is_hcsvcrtp);
                return -100;
            }
        }
        //printf("api_count_loss_rate2: CountLossRate2 \n");
        ret = CountLossRate2(obj, dataPtr, insize, freq);
        //printf("api_count_loss_rate2: ret=%d \n", ret);
        if(ret >= 0)
        {
            if(ret > 0)
            {
                printf("api_count_loss_rate2: ret=%d, freq=%d \n", ret, freq);
                if(freq == 90)
                {
                    printf("api_count_loss_rate2: obj->max_loss_series_num=%d, obj->max_fec_n=%d \n", obj->max_loss_series_num, obj->max_fec_n);
                }
            }
            int ret2 = add_loss_rate(obj->loss_rate_info, ret);
            //printf("api_count_loss_rate2: ret2=%d \n", ret2);
            obj->max_loss_series_num = 0;
            obj->max_fec_n = 0;
            ret = ret2;
            ret += 1;
        }

    }
    return ret;
}
HCSVC_API
int api_add_loss_rate(void *handle, int loss_rate)
{
    int ret = loss_rate;
    long long *testp = (long long *)handle;
    CodecObj *codecObj = (CodecObj *)testp[0];
    RtpObj *obj = codecObj->rtpObj;
    if(!obj)
    {
        return ret;
    }
    ret = add_loss_rate(obj->loss_rate_info, ret);
    return ret;
}
HCSVC_API
int api_get_loss_rate(void *handle, int idx0)
{
    int ret = -1;
    long long *testp = (long long *)handle;
    CodecObj *codecObj = (CodecObj *)testp[0];
    RtpObj *obj = codecObj->rtpObj;
    if(obj)
    {
        int idx = idx0 > 2 ? 2 : idx0;
        int a = obj->loss_rate_info[idx].max_loss_rate;
        int b = obj->loss_rate_info[idx].last_max_loss_rate;
        ret = a > b ? a : b;
        if(idx > 1)
        {
            int c = obj->loss_rate_info[1].max_loss_rate;
            ret = c > ret ? c : ret;
        }
        if(idx0 > 2)
        {
            int d = obj->loss_rate_info[3].loss_rate;
            //printf("api_get_loss_rate: d=%d \n", d);
            ret = d > ret ? d : ret;
        }
    }

    return ret;
}
HCSVC_API
int InSertRawOffset(uint8_t* dataPtr, uint8_t* dst, int dataSize)
{
    int ret = dataSize;
    RTP_FIXED_HEADER* rtp_hdr = (RTP_FIXED_HEADER*)&dataPtr[0];
    if(rtp_hdr->version == 2 &&  //版本号，此版本固定为2								V
       rtp_hdr->padding	== 0 &&  //														P
       rtp_hdr->csrc_len == 0 && //												X
       rtp_hdr->payload == H264_PLT   //负载类型号
       )
    {
        int offset = sizeof(RTP_FIXED_HEADER);//1;
        if(rtp_hdr->extension)
        {
            EXTEND_HEADER *rtp_ext = (EXTEND_HEADER *)&dataPtr[sizeof(RTP_FIXED_HEADER)];
            int rtp_extend_length = rtp_ext->rtp_extend_length;
            rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
            //printf("IsRtpData: rtp_extend_length= %d \n", rtp_extend_length);
            rtp_extend_length = (rtp_extend_length + 1) << 2;
            //printf("IsRtpData: 2: rtp_extend_length= %d \n", rtp_extend_length);
            offset = sizeof(RTP_FIXED_HEADER) + rtp_extend_length;
            //info->rtp_header_size = offset;

            rtp_ext->raw_offset = 1;
            int raw_offset = RAW_OFFSET;//2;
            //info->raw_offset = 2;
            ret = dataSize + raw_offset;
            memcpy(dst, &dataPtr[offset], raw_offset);
            NALU_HEADER* nalu_hdr = (NALU_HEADER*)&dst[0];
            if(nalu_hdr->TYPE == 24)
            {
                nalu_hdr->TYPE = 7;
            }
            memcpy(&dst[raw_offset], &dataPtr[0], dataSize);

        }
        else{
            ///printf("error: InSertRawOffset: rtp_hdr->extension=%d \n", rtp_hdr->extension);
        }
    }
    else{
        ///printf("error: InSertRawOffset: rtp_hdr->version=%d \n", rtp_hdr->version);
        ///printf("error: InSertRawOffset: rtp_hdr->padding=%d \n", rtp_hdr->padding);
        ///printf("error: InSertRawOffset: rtp_hdr->csrc_len=%d \n", rtp_hdr->csrc_len);
        ///printf("error: InSertRawOffset: rtp_hdr->payload=%d \n", rtp_hdr->payload);
    }
    return ret;
}
HCSVC_API
int hcsvc2h264stream(unsigned char *src, int insize, int rtpheadersize, unsigned char *dst)
{
    int ret = 0;
    RtpInfo info = {0};
    info.raw_offset = -1;
    int rtptype = GetRtpInfo((uint8_t*)&src[rtpheadersize], insize - rtpheadersize, &info, H264_PLT);
    if(rtptype)
    {
        if(info.enable_fec)
        {
            if(info.is_fec)
            {
            }
            else{
                short *raw_size = (short *)&src[insize - 2];
                ret = raw_size[0];//insize - info.rtp_header_size;
                ///printf("hcsvc2h264stream: ret= %d \n", ret);
                memmove(dst, &src[rtpheadersize + info.rtp_header_size], ret);
                if(info.nal_marker == 2)
                {
                    RTP_FIXED_HEADER* rtp_hdr = (RTP_FIXED_HEADER*)&src[0];
                    rtp_hdr->marker = 1;
                }

            }
        }
        else{
            ret = insize - info.rtp_header_size - rtpheadersize;
            memmove(dst, &src[rtpheadersize + info.rtp_header_size], ret);
        }
    }

    return ret;
}
HCSVC_API
int distill_spatial_layer(unsigned char *src, int insize, int rtpheadersize, unsigned char *dst, int layerId, int multlayer)
{
    int ret = 0;
    RtpInfo info = {0};
    info.raw_offset = -1;
    int rtptype = GetRtpInfo((uint8_t*)&src[rtpheadersize], insize - rtpheadersize, &info, H264_PLT);
    if(rtptype)
    {
        if(info.spatial_idx >= layerId)
        {

            if(multlayer)
            {
                if(src != dst)
                {
                    memmove(dst, src, insize);
                }

                EXTEND_HEADER *rtp_ext = (EXTEND_HEADER *)&dst[rtpheadersize + info.raw_offset + sizeof(RTP_FIXED_HEADER)];
                rtp_ext->main_spatial_idx = layerId;
                rtp_ext->mult_spatial = 1;
                ret = insize;
            }
            else if(info.spatial_idx == layerId)
            {

                if(src != dst)
                {
                    memmove(dst, src, insize);
                }
                EXTEND_HEADER *rtp_ext = (EXTEND_HEADER *)&dst[rtpheadersize + info.raw_offset + sizeof(RTP_FIXED_HEADER)];
                rtp_ext->main_spatial_idx = layerId;
                rtp_ext->mult_spatial = 0;
                ret = insize;
            }
        }
    }
    return ret;
}
HCSVC_API
int GetRtpHeaderSize(uint8_t* dataPtr, int dataSize)
{
    int ret = 0;
    RTP_FIXED_HEADER* rtp_hdr = (RTP_FIXED_HEADER*)&dataPtr[0];
    if(rtp_hdr->version == 2  //版本号，此版本固定为2								V
       )
    {
        ret = sizeof(RTP_FIXED_HEADER);//1;
        if(rtp_hdr->extension)
        {
            EXTEND_HEADER *rtp_ext = (EXTEND_HEADER *)&dataPtr[sizeof(RTP_FIXED_HEADER)];
            int rtp_extend_length = rtp_ext->rtp_extend_length;
            rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
            //printf("IsRtpData: rtp_extend_length= %d \n", rtp_extend_length);
            rtp_extend_length = (rtp_extend_length + 1) << 2;
            ret = sizeof(RTP_FIXED_HEADER) + rtp_extend_length;
        }
    }
    return ret;
}

HCSVC_API
int GetNetInfo(uint8_t* dataPtr, int insize, NetInfo *info, int times)
{
    int ret = 0;
    RTP_FIXED_HEADER* rtp_hdr = (RTP_FIXED_HEADER*)&dataPtr[0];
    EXTEND_HEADER *rtp_ext = NULL;
    if(rtp_hdr->extension)
    {
        rtp_ext = (EXTEND_HEADER *)&dataPtr[sizeof(RTP_FIXED_HEADER)];
        if(rtp_ext->nack_type == 2)
        {
            info->is_rtx = 1;
            info->nack = rtp_ext->nack;
            ret = 1;
        }
        else if(rtp_ext->nack_type == 1 && rtp_ext->nack.nack0.enable_nack)
        {
            int64_t now_time = (int64_t)get_time_stamp();
            //不仅要获得网络信息，还需要获得全线信息，最终的效果是由全线的信息决定的
            RTT_HEADER *rtt_list = (RTT_HEADER *)&rtp_ext->nack.nack0.time_info.rtt_list;
            info->ssrc = rtp_hdr->ssrc;
            uint64_t time_stamp0 = rtp_ext->nack.nack0.time_info.time_stamp0;
	        uint64_t time_stamp1 = rtp_ext->nack.nack0.time_info.time_stamp1;
	        info->time_stamp = time_stamp0 | (time_stamp1 << 32);
            //info->time_stamp = rtp_ext->nack.nack0.time_info.time_stamp;
            info->loss_rate = rtp_ext->nack.nack0.loss_rate;//loss_rate in [1,101]
		    info->st0 = rtt_list->rtt0.st0;//共享编码端写入
    		info->rt0 = rtt_list->rtt0.rt0;//观看解码端（rtp接收）写入
    		info->chanId = rtp_ext->nack.nack0.chanId;//if chanId == selfChanId then getting net info sucess!
    		info->info_status = rtp_ext->nack.nack0.info_status;
            //printf("GetNetInfo: times= %d \n", times);
            //printf("GetNetInfo: info->info_status= %d \n", info->info_status);
	    	if(info->info_status)
		    {
		        //可能由多方重复转来
		        //printf("GetNetInfo: 0: rtt_list->rtt1.decodeId= %d \n", rtt_list->rtt1.decodeId);
		        //printf("GetNetInfo: times= %d \n", times);
		        info->st1 = rtt_list->rtt0.st1;//观看编码端(rtp发送)写入
		        //不允许重复访问
		        if(!times)
		        {
		            //printf("GetNetInfo: rtt_list->rtt1.decodeId= %d \n", rtt_list->rtt1.decodeId);
		            //route 3
		            if(rtt_list->rtt1.decodeId)
    	    	    {
	        	        info->rtt = rtt_list->rtt1.rtt;
	        	        info->decodeId = rtt_list->rtt1.decodeId;
	    	            rtt_list->rtt1.rtt = 0;//复用
	    	            rtt_list->rtt1.decodeId = 0;//复用
	    	        }
	    	        //route 2
		            if(!rtt_list->rtt0.rt1)
	    	        {
	    	            info->rt1 = now_time & 0xFFFF;//共享解码端（rtp接收）写入
	    	            rtt_list->rtt0.rt1 = now_time & 0xFFFF;
	    	        }
	    	        else{
	    	            info->rt1 = rtt_list->rtt0.rt1;
	    	        }
		        }
		        else{
		            info->rt1 = rtt_list->rtt0.rt1;
		        }

	    	}
	    	//最终交由共享编码端自己去统计并处理信息
		    //info->chanId = rtp_ext->nack.nack0.chanId;//if chanId == selfChanId then getting net info sucess!
    		//info->info_status = rtp_ext->nack.nack0.info_status;
	    	if(!info->info_status)
		    {
		        //不存在重复转发
    		    info->st0 = rtt_list->rtt0.st0;
	    	    if(!times)
		        {
		            //route 1
        		    if(!rtt_list->rtt0.rt0)//允许重复访问
        		    {
    	    	        rtt_list->rtt0.rt0 = now_time & 0xFFFF;
    		            info->rt0 = now_time & 0xFFFF;
    		        }
    	    	    else{
	        	        info->rt0 = rtt_list->rtt0.rt0;
	        	    }
	    	        if(rtt_list->rtt1.decodeId)//no used
	    	        {
	    	            info->rtt = rtt_list->rtt1.rtt;
    	    	        info->decodeId = rtt_list->rtt1.decodeId;
	        	        rtt_list->rtt1.rtt = 0;//复用
	        	        rtt_list->rtt1.decodeId = 0;//复用
	    	        }
	    	    }
	    	    else{
	    	        info->rt0 = rtt_list->rtt0.rt0;
	    	    }
		        info->st1 = 0;
    		    info->rt1 = 0;
	    	    info->loss_rate = 0;
		        //info->chanId = rtp_ext->nack.nack0.chanId;
    		    //info->info_status = rtp_ext->nack.nack0.info_status;
	    	}
		    ret = 1;
        }
    }
    return ret;
}
HCSVC_API
int GetAudioNetInfo(uint8_t* dataPtr, int insize, NetInfo *info, int times)
{
    int ret = 0;
    RTP_FIXED_HEADER* rtp_hdr = (RTP_FIXED_HEADER*)&dataPtr[0];
    AUDIO_EXTEND_HEADER *rtp_ext = NULL;
    if(rtp_hdr->extension)
    {
        rtp_ext = (AUDIO_EXTEND_HEADER *)&dataPtr[sizeof(RTP_FIXED_HEADER)];
        if(rtp_ext->nack.nack0.enable_nack)
        {
            int64_t now_time = (int64_t)get_time_stamp();
            //不仅要获得网络信息，还需要获得全线信息，最终的效果是由全线的信息决定的
            RTT_HEADER *rtt_list = (RTT_HEADER *)&rtp_ext->nack.nack0.time_info.rtt_list;
            info->ssrc = rtp_hdr->ssrc;
            uint64_t time_stamp0 = rtp_ext->nack.nack0.time_info.time_stamp0;
	        uint64_t time_stamp1 = rtp_ext->nack.nack0.time_info.time_stamp1;
	        info->time_stamp = time_stamp0 | (time_stamp1 << 32);
            //info->time_stamp = rtp_ext->nack.nack0.time_info.time_stamp;
            info->loss_rate = rtp_ext->nack.nack0.loss_rate;//loss_rate in [1,101]
		    info->st0 = rtt_list->rtt0.st0;//共享编码端写入
    		info->rt0 = rtt_list->rtt0.rt0;//观看解码端（rtp接收）写入
            info->chanId = rtp_ext->nack.nack0.chanId;//if chanId == selfChanId then getting net info sucess!
    		info->info_status = rtp_ext->nack.nack0.info_status;
	    	if(info->info_status)
		    {
		        info->st1 = rtt_list->rtt0.st1;//观看编码端(rtp发送)写入
		        //不允许重复访问
		        if(!times)
		        {
		            if(rtt_list->rtt1.decodeId)
    	    	    {
	        	        info->rtt = rtt_list->rtt1.rtt;
	        	        info->decodeId = rtt_list->rtt1.decodeId;
	    	            rtt_list->rtt1.rtt = 0;
	    	            rtt_list->rtt1.decodeId = 0;
	    	        }
		            if(!rtt_list->rtt0.rt1)
	    	        {
	    	            info->rt1 = now_time & 0xFFFF;//共享解码端（rtp接收）写入
	    	            rtt_list->rtt0.rt1 = now_time & 0xFFFF;
	    	        }
	    	        else{
	    	            info->rt1 = rtt_list->rtt0.rt1;
	    	        }
		        }
		        else{
		            info->rt1 = rtt_list->rtt0.rt1;
		        }
	    	}
	    	//最终交由共享编码端自己去统计并处理信息
	    	if(!info->info_status)
		    {
    		    info->st0 = rtt_list->rtt0.st0;
    		    if(!times)
		        {
        		    if(!rtt_list->rtt0.rt0)//允许重复访问
        		    {
    	    	        rtt_list->rtt0.rt0 = now_time & 0xFFFF;
    		            info->rt0 = now_time & 0xFFFF;
    		        }
    	    	    else{
	        	        info->rt0 = rtt_list->rtt0.rt0;
	        	    }
	    	        if(rtt_list->rtt1.decodeId)
	    	        {
	    	            info->rtt = rtt_list->rtt1.rtt;
    	    	        info->decodeId = rtt_list->rtt1.decodeId;
	        	        rtt_list->rtt1.rtt = 0;
	        	        rtt_list->rtt1.decodeId = 0;
	    	        }
	    	    }
	    	    else{
	    	        info->rt0 = rtt_list->rtt0.rt0;
	    	    }
		        info->st1 = 0;
    		    info->rt1 = 0;
	    	    info->loss_rate = 0;
		        //info->chanId = rtp_ext->nack.nack0.chanId;
    		    //info->info_status = rtp_ext->nack.nack0.info_status;
	    	}
		    ret = 1;
        }
    }
    return ret;
}
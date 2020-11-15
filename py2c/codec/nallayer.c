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


static int global_rtp_obj_status = 0;
static RtpObj global_rtp_objs[MAX_OBJ_NUM];
static char global_info_outparam[MAX_OBJ_NUM][MAX_OUTCHAR_SIZE];
static char global_time_outparam[MAX_OBJ_NUM][TIME_OUTCHAR_SIZE];

extern cJSON* mystr2json(char *text);
extern int GetvalueInt(cJSON *json, char *key);
extern char* GetvalueStr(cJSON *json, char *key);
extern cJSON* renewJson(cJSON *json, char *key, int ivalue, char *cvalue, cJSON *subJson);
extern cJSON* renewJsonInt(cJSON *json, char *key, int ivalue);
extern cJSON* renewJsonStr(cJSON *json, char *key, char *cvalue);
extern cJSON* deleteJson(cJSON *json);
extern cJSON* renewJsonArray2(cJSON *json, char *key, short *value);
extern inline int GetRtpInfo(uint8_t* dataPtr, int insize, RtpInfo *info);

//extern int api_fec_encode(int id, char *data, char *param, char *outbuf, char *outparam[]);
//extern int api_fec_decode(int id, char *data, char *param, char *outbuf, char *outparam[]);

int FindStartCode2 (unsigned char *Buf);//查找开始字符0x000001
int FindStartCode3 (unsigned char *Buf);//查找开始字符0x00000001

unsigned char test_data[100 * 1024];
static int test_flag0 = 0;
static int test_flag1 = 0;

#ifdef _WIN32
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
    return time;
}
#endif

void get_time(int64_t time, char *ctime, int offset)
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
int64_t get_time_stamp(void)
{
    int64_t time_stamp = get_sys_time();
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
}
/*
static int initobj(int id)
{
    if (global_rtp_obj_status <= 0)
    {
        for (int i = 0; i < MAX_OBJ_NUM; i++)
        {
            ResetObj(&global_rtp_objs[i]);
        }
        global_rtp_obj_status += 1;
    }
    else
    {
    }
    return global_rtp_obj_status;
}
*/
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
  //static int info2=0, info3=0;

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
int SplitNal(SVCNalu *svc_nalu)
{
	int i = 0, offset = 0;
	while(offset < svc_nalu->size)
	{
		//printf("SplitNal: offset= %d \n", offset);
		offset += GetAnnexbNALU(svc_nalu, i, offset);
		i++;
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
    int nal_mem_num = ((int)(size / mtu_size) << 1) + 3;//3:sps/pps/sei
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
    for(int i = 0; i < nal_mem_num; i++)
	{
		//nal[I].buf = new char [mtu_size+100];
		svc_nalu->nal[i].buf = calloc(1, sizeof(char) * size);
	}
	//printf("data2nalus: start 2 \n");
    int ret = SplitNal(svc_nalu);
    svc_nalu->nal_num = ret;
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
int get_important_packet(SVCNalu *svc_nalu, cJSON *json, char *outBuf, short *rtpSize, uint16_t *seq_num, uint16_t *idx)
{
    //int ret = 0;
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
    int time_offset = GetvalueInt(json, "time_offset");
    int nal_num = svc_nalu->nal_num;
    int slice_type = 0;
    int offset = 0;
    int i = 0;
    //int idx = 0;
    int extlen = 0;
    int bytes = 0;
    mtu_size = mtu_size ? mtu_size : FIX_MTU_SIZE;
    while(i < nal_num)
    {
        NALU_t *n = &svc_nalu->nal[i];
		//fprintf(logfp, "rtp count = %d \n", count); fflush(logfp);
		//printf("get_important_packet: res_idx = %d \n", res_idx);
		//printf("get_important_packet: res_num = %d \n", res_num);
		if ( (n->nal_unit_type == 7) || (n->nal_unit_type == 8) || (n->nal_unit_type == 6))
		{
			if (n->nal_unit_type == 7)
			{
				//printf("enc IDR = %d \n", IDRCnt);
				//LogOut(format1, (void *)&IDRCnt);
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
			rtp_hdr->ssrc        = (unsigned int)svc_nalu;//htonl(10);    //随机指定为10，并且在本RTP会话中全局唯一	SSRC

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
				rtp_ext->rtp_extend_profile = 0;
				extlen = (rtp_extend_length + 1) << 2;
				rtp_ext->rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
				//

				rtp_ext->refs = refs & 0x1F;
				rtp_ext->ref_idx = ref_idx & 0x1F;
				int ref_idc = ref_idx == 0 ? 0 : ref_idx == refs ? 1 : ref_idx == 1 ? 3 : 2;
				if(refs == 1)
				{
				    int picType = n->nal_reference_idc >> 5;
				    if(picType == 1)
				    {
				        ref_idc = 0;
				    }
				    else if(picType == 2)
				    {
				        ref_idc = 1;
				    }
				    else if(picType == 3)
				    {
				        ref_idc = 2;
				    }
				}
				rtp_ext->ref_idc = ref_idc & 0x3;
				rtp_ext->res_num = res_num;//1;
				rtp_ext->res_idx = res_idx;//0;
				rtp_ext->main_spatial_idx = main_spatial_idx;
				rtp_ext->mult_spatial = 0;//res_num > 0;//默认只有一层给解码器
				rtp_ext->qua_num = 0;
				rtp_ext->qua_idx = 0;
				rtp_ext->type = n->nal_unit_type;
				rtp_ext->enable_fec = 0;//svc_nalu->enable_fec;
				rtp_ext->refresh_idr = svc_nalu->refresh_idr;
				//
				rtp_ext->nack.is_lost_packet = 0;
				rtp_ext->nack.lost_packet_rate = 0;
				//if(!time_offset)
				{
				    rtp_ext->nack.time_status = 0;
				    rtp_ext->nack.time_offset = time_offset;
				    rtp_ext->nack.time_info.time_stamp = (int64_t)get_time_stamp();
				}
				//
				//int picType = n->nal_reference_idc >> 5;
				//rtp_ext->is_b_slice = picType == 3 || picType == 2 || is_key_frame ? false : true;
				//rtp_ext->pic_type = n->nal_reference_idc >> 5;

			}
			extlen = rtp_hdr->extension ? extlen : 0;
			//
			nalu_hdr =(NALU_HEADER*)&sendptr[sizeof(RTP_FIXED_HEADER) + extlen];//sendbuf[RTPHEADSIZE + offset]; //将sendbuf[12]的地址赋给nalu_hdr，之后对nalu_hdr的写入就将写入sendbuf中；
			nalu_hdr->F=n->forbidden_bit;
			//if (n->forbidden_bit) printf("n->forbidden_bit \n", n->forbidden_bit);
			nalu_hdr->NRI=n->nal_reference_idc>>5;//有效数据在n->nal_reference_idc的第6，7位，需要右移5位才能将其值赋给nalu_hdr->NRI。
			nalu_hdr->TYPE=n->nal_unit_type;

			nalu_payload=&sendptr[sizeof(RTP_FIXED_HEADER) + extlen + 1];//sendbuf[13 + offset];//同理将sendbuf[13]赋给nalu_payload
			memcpy(nalu_payload,n->buf + 1,n->len-1);//去掉nalu头的nalu剩余内容写入sendbuf[13]开始的字符串。
			bytes=n->len + sizeof(RTP_FIXED_HEADER) + extlen;						//获得sendbuf的长度,为nalu的长度（包含NALU头但除去起始前缀）加上rtp_header的固定长度12字节
			offset += bytes;
			rtpSize[(*idx)++] = bytes;
            if(rtp_hdr->extension)
            {
                rtp_ext->rtp_pkt_size = bytes;
            }
		}
		else
		{
			slice_type = n->nal_reference_idc >> 5;
			slice_type = slice_type == 3 ? AV_PICTURE_TYPE_I : slice_type == 2 ? AV_PICTURE_TYPE_P : AV_PICTURE_TYPE_B;
		}
		i++;
    }
    return offset;
}
int get_slice_packet(SVCNalu *svc_nalu, cJSON *json, char *outBuf, short *rtpSize, uint16_t *seq_num, uint16_t *idx, int offset)
{
    int ret = 0;
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
    int time_offset = GetvalueInt(json, "time_offset");
    int nal_num = svc_nalu->nal_num;
    int slice_type = 0;
    //int offset = 0;
    int i = 0;
    //int idx = 0;
    int extlen = 0;
    int bytes = 0;
    int firstSlice = 0;
	int nextFirstSlice = 0;
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
		if ( (n->nal_unit_type != 7) && (n->nal_unit_type != 8) && (n->nal_unit_type != 6))
		{
            rtp_hdr =(RTP_FIXED_HEADER*)&outBuf[offset];
			rtp_hdr->payload     = H264_PLT;  //负载类型号，									PT
			rtp_hdr->version     = 2;  //版本号，此版本固定为2								V
			rtp_hdr->padding	 = 0;//														P
			rtp_hdr->csrc_len	 = 0;//														CC
			rtp_hdr->marker		 = 0;//(size >= len);   //标志位，由具体协议规定其值。		M
			rtp_hdr->ssrc        = (unsigned int)svc_nalu;;//htonl(10);    //随机指定为10，并且在本RTP会话中全局唯一	SSRC
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
				rtp_ext->rtp_extend_profile = 0;
				extlen = (rtp_extend_length + 1) << 2;
				rtp_ext->rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
				//
				rtp_ext->refs = refs & 0x1F;
				rtp_ext->ref_idx = ref_idx & 0x1F;
				int ref_idc = ref_idx == 0 ? 0 : ref_idx == refs ? 1 : ref_idx == 1 ? 3 : 2;
				if(refs == 1)
				{
				    int picType = n->nal_reference_idc >> 5;
				    if(picType == 1)
				    {
				        ref_idc = 0;
				    }
				    else if(picType == 2)
				    {
				        ref_idc = 1;
				    }
				    else if(picType == 3)
				    {
				        ref_idc = 2;
				    }
				    //printf("get_slice_packet: picType= %d \n", picType);
				}
				rtp_ext->ref_idc = ref_idc & 0x3;
				rtp_ext->res_num = res_num;//1;
				rtp_ext->res_idx = res_idx;//0;
				rtp_ext->main_spatial_idx = main_spatial_idx;
				rtp_ext->mult_spatial = 0;//res_num > 0;//默认只有一层给解码器
				rtp_ext->qua_num = 0;
				rtp_ext->qua_idx = 0;
				rtp_ext->first_slice = firstSlice;
				rtp_ext->type = n->nal_unit_type;
				rtp_ext->enable_fec = 0;//svc_nalu->enable_fec;
				rtp_ext->refresh_idr = svc_nalu->refresh_idr;

				//
				rtp_ext->nack.is_lost_packet = 0;
				rtp_ext->nack.lost_packet_rate = 0;
				//if(!time_offset)
				{
				    rtp_ext->nack.time_status = 0;
				    rtp_ext->nack.time_offset = time_offset;
				    rtp_ext->nack.time_info.time_stamp = (int64_t)get_time_stamp();
				}

				//
				//int picType = n->nal_reference_idc >> 5;;
				//rtp_ext->is_b_slice = picType == 3 || picType == 2 || is_key_frame ? false : true;
				//rtp_ext->pic_type = n->nal_reference_idc >> 5; //isKeyFrame;
			}
			if(firstSlice && nextFirstSlice)
			{
			    //printf("get_slice_packet: 000000000000000000 \n");
				//rtp_hdr->seq_no = 0;///htons(seq_num ++); //序列号，每发送一个RTP包增1
				//设置rtp M 位；
				//rtp_hdr->marker=1;
				slice_type = n->nal_reference_idc >> 5;
				slice_type = slice_type == 3 ? AV_PICTURE_TYPE_I : slice_type == 2 ? AV_PICTURE_TYPE_P : AV_PICTURE_TYPE_B;
				//rtp_hdr->seq_no     = 0;///htons(seq_num ++); //序列号，每发送一个RTP包增1	SQUENCE NUMBER
				//设置NALU HEADER,并将这个HEADER填入sendbuf[12]
				//rtp_ext = (EXTEND_HEADER *)&sendptr[0];
				//rtp_ext->rtp_extend_profile = 0;
				//rtp_ext->rtp_extend_length = rtp_extend_length;//1;//0;
				//extlen = (rtp_ext->rtp_extend_length + 1) << 2;
				extlen = rtp_hdr->extension ? extlen : 0;
				//rtp_ext->rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
				nalu_hdr =(NALU_HEADER*)&sendptr[extlen];//&sendbuf[RTPHEADSIZE + offset]; //将sendbuf[12]的地址赋给nalu_hdr，之后对nalu_hdr的写入就将写入sendbuf中；
				nalu_hdr->F = n->forbidden_bit;
				//if (n->forbidden_bit) printf("n->forbidden_bit \n", n->forbidden_bit);
				nalu_hdr->NRI = n->nal_reference_idc>>5;//有效数据在n->nal_reference_idc的第6，7位，需要右移5位才能将其值赋给nalu_hdr->NRI。
				nalu_hdr->TYPE = n->nal_unit_type;
				nalu_payload=&sendptr[extlen + 1];//sendbuf[13 + offset];//同理将sendbuf[13]赋给nalu_payload
				memcpy(nalu_payload,n->buf+1,n->len-1);//去掉nalu头的nalu剩余内容写入sendbuf[13]开始的字符串。

                //printf("get_slice_packet: nalu_hdr->TYPE = %d \n", nalu_hdr->TYPE);

				///ts_current=ts_current+timestamp_increse;
				rtp_hdr->timestamp = svc_nalu->timestamp;//htonl(ts_current);										TIMESTAMP
				bytes = n->len + sizeof(RTP_FIXED_HEADER) + extlen;						//获得sendbuf的长度,为nalu的长度（包含NALU头但除去起始前缀）加上rtp_header的固定长度12字节

				offset += bytes;//send( socket1, sendbuf, bytes, 0 );//发送rtp包
				if(rtp_hdr->extension)
					rtp_ext->rtp_pkt_size = bytes;
				firstSlice = 0;
				nextFirstSlice = 0;
			}
			else if(firstSlice && !nextFirstSlice)//发送一个需要分片的NALU的第一个分片，置FU HEADER的S位
			{
			    //printf("get_slice_packet: 11111111111111111111 \n");
				//rtp_hdr->extension	 = 1;//
				slice_type = n->nal_reference_idc >> 5;
				slice_type = slice_type == 3 ? AV_PICTURE_TYPE_I : slice_type == 2 ? AV_PICTURE_TYPE_P : AV_PICTURE_TYPE_B;
				//设置rtp M 位；
				//rtp_hdr->marker=0;
				//设置FU INDICATOR,并将这个HEADER填入sendbuf[12]
				//rtp_ext = (EXTEND_HEADER *)&sendptr[0];
				//rtp_ext->rtp_extend_profile =0;
				//rtp_ext->rtp_extend_length = rtp_extend_length;//1;//0;
				//extlen = (rtp_ext->rtp_extend_length + 1) << 2;
				extlen = rtp_hdr->extension ? extlen : 0;
				//rtp_ext->rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));

				fu_ind =(FU_INDICATOR*)&sendptr[extlen];//sendbuf[RTPHEADSIZE + VIDIORASHEADSIZE + offset]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
				fu_ind->F = n->forbidden_bit;
				//if (n->forbidden_bit) printf("n->forbidden_bit \n", n->forbidden_bit);
				fu_ind->NRI = n->nal_reference_idc>>5;
				fu_ind->TYPE = 28;
                if (rtp_hdr->extension)
			    {
			        rtp_ext->type = 28;
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
                //printf("get_slice_packet: 2222222222222222222222222 \n");
				//设置rtp M 位；当前传输的是最后一个分片时该位置1
				//rtp_hdr->marker = 0;//1;
				//设置FU INDICATOR,并将这个HEADER填入sendbuf[12]
				slice_type = n->nal_reference_idc >> 5;
				slice_type = slice_type == 3 ? AV_PICTURE_TYPE_I : slice_type == 2 ? AV_PICTURE_TYPE_P : AV_PICTURE_TYPE_B;
				//rtp_ext = (EXTEND_HEADER *)&sendptr[0];
				//rtp_ext->rtp_extend_profile = 0;
				//rtp_ext->rtp_extend_length = rtp_extend_length;//1;//0;
				//extlen = (rtp_ext->rtp_extend_length + 1) << 2;
				extlen = rtp_hdr->extension ? extlen : 0;
				//rtp_ext->rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
				fu_ind =(FU_INDICATOR*)&sendptr[extlen];//&sendbuf[RTPHEADSIZE + offset]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
				fu_ind->F=n->forbidden_bit;
				//if (n->forbidden_bit) printf("n->forbidden_bit \n", n->forbidden_bit);
				fu_ind->NRI=n->nal_reference_idc>>5;
				fu_ind->TYPE=28;
                if (rtp_hdr->extension)
			    {
			        rtp_ext->type = 28;
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
			    //printf("get_slice_packet: 333333333333333333333333 \n");
				//设置rtp M 位；
				//rtp_hdr->marker=0;
				//设置FU INDICATOR,并将这个HEADER填入sendbuf[12]
				slice_type = n->nal_reference_idc >> 5;
				slice_type = slice_type == 3 ? AV_PICTURE_TYPE_I : slice_type == 2 ? AV_PICTURE_TYPE_P : AV_PICTURE_TYPE_B;
				//rtp_ext = (EXTEND_HEADER *)&sendptr[0];
				//rtp_ext->rtp_extend_profile = 0;
				//rtp_ext->rtp_extend_length = rtp_extend_length;//1;//0;
				//extlen = (rtp_ext->rtp_extend_length + 1) << 2;
				extlen = rtp_hdr->extension ? extlen : 0;
				//rtp_ext->rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
				fu_ind =(FU_INDICATOR*)&sendptr[extlen];//&sendbuf[RTPHEADSIZE + offset]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
				fu_ind->F=n->forbidden_bit;
				//if (n->forbidden_bit) printf("n->forbidden_bit \n", n->forbidden_bit);
				fu_ind->NRI=n->nal_reference_idc>>5;
				fu_ind->TYPE=28;
                if (rtp_hdr->extension)
			    {
			        rtp_ext->type = 28;
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
    unsigned short seq_num = obj->seq_num;
    offset = get_important_packet(svc_nalu, json, outBuf, rtpSize, &obj->seq_num, &idx);
#if 1
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
    ret = get_slice_packet(svc_nalu, json, outBuf, rtpSize, &obj->seq_num, &idx, offset);
    //

    return ret;
}
HCSVC_API
int api_raw2rtp_packet(char *handle, char *data, char *param, char *outbuf, char *outparam[])
{
    int ret = 0;

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
        obj->json = mystr2json(param);

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
            obj->time_stamp += 40;//test
        }

        //printf("api_raw2rtp_packet: id= %d \n", id);
        //printf("api_raw2rtp_packet: obj->time_stamp= %u \n", obj->time_stamp);
        int insize = GetvalueInt(obj->json, "insize");
        //printf("api_raw2rtp_packet: param= %s \n", param);
        //printf("api_raw2rtp_packet: insize= %d \n", insize);
        //printf("api_raw2rtp_packet: mtu_size= %d \n", mtu_size);

        ret = data2nalus(obj, data);

        //printf("api_raw2rtp_packet: ret= %d \n", ret);
        //print output
        SVCNalu *svc_nalu = &obj->svc_nalu;
        int nal_num = svc_nalu->nal_num;
        for(int i = 0; i < nal_num; i++)
	    {
	        int len = svc_nalu->nal[i].len;
	        //printf("api_raw2rtp_packet: i= %d, len= %d \n", i, len);
	    }
	    //printf("api_raw2rtp_packet: nal_num= %d \n", nal_num);
	    //
	    //obj->time_stamp = (uint32_t)get_time_stamp();
	    ret = raw2rtp_packet(obj, outbuf);
	    //printf("api_raw2rtp_packet: ret= %d \n", ret);

        sprintf(obj->outparam[1], "%d", obj->seq_num);
        outparam[1] = obj->outparam[1];
	    //char text[2048] = "";//2048/4=512//512*1400=700kB
	    char *text = obj->outparam[0];
	    int sum = 0;
	    memset(obj->outparam[0], 0, sizeof(char) * MAX_OUTCHAR_SIZE);
	    for (int i = 0; i < ret; i++)
	    {
            char ctmp[32] = "";
            int size = obj->rtpSize[i];
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
	    //free
        int nal_mem_num = svc_nalu->nal_mem_num;
        for(int i = 0; i < nal_mem_num; i++)
	    {
		    free(svc_nalu->nal[i].buf);
	    }
	    free(svc_nalu->nal);
	    free(obj->rtpSize);
	    deleteJson(obj->json);
	    obj->json = NULL;
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
	short slice_type = 0;
	unsigned  long timestamp = 0;
	int last_timestamp = 0;
	int rptHeadSize = sizeof(RTP_FIXED_HEADER);
	int rtp_extend_length = 0;
	int redundancy_code = 0;//rtPacket->redundancy;//VP8REDUNDANCYSIZE;//0;//12;//RTPHEADSIZE;//vp8为了区分连续不同的MTU而插入了12字节的特征码
	static int frmnum = 0;
	int discard = 0;
	int sps_pps_flag = 0;
	int rtp_pkt_size = 0;
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

			extprofile = rtp_ext->rtp_extend_profile & 7;
			rtp_extend_length = rtp_ext->rtp_extend_length;
			rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
			extlen = (rtp_extend_length + 1) << 2;
			rtp_pkt_size = rtp_ext->rtp_pkt_size;
			//printf("rtp_unpacket: rtpSize[idx]=%d \trtp_pkt_size=%d \n", rtpSize[idx], rtp_pkt_size);
			rtpSize[idx] = rtp_pkt_size;
			if(rtp_pkt_size <=0 || rtp_pkt_size > mtu_size)
			{
				if(rtp_pkt_size < 1500 && rtp_pkt_size > mtu_size)
				{

				}
				else
				{
                    //LogOut(" rtp_pkt_size = %d \n",(void *)&rtp_pkt_size);
                    printf("error: rtp_unpacket: rtp_pkt_size=%d \n",rtp_pkt_size);
                    printf("error: rtp_unpacket: i=%d \n",i);
					return -1;
				}
			}
            if (extlen < 0 || extlen > mtu_size) {
                printf("error: rtp_unpacket: extlen=%d \n",extlen);
                return -1;
            }
            //get extend info
            {
                int refs = rtp_ext->refs;
		        int ref_idx = rtp_ext->ref_idx;
		        int ref_idc = rtp_ext->ref_idc;
		        int res_num = rtp_ext->res_num;
		        int res_idx = rtp_ext->res_idx;
		        int qua_num = rtp_ext->qua_num;
		        int qua_idx = rtp_ext->qua_idx;
		        int is_lost_packet = rtp_ext->nack.is_lost_packet;
		        int lost_packet_rate = rtp_ext->nack.lost_packet_rate;
		        int time_status = rtp_ext->nack.time_status;
		        int time_offset = rtp_ext->nack.time_offset;
		        int64_t packet_time_stamp = rtp_ext->nack.time_info.time_stamp;
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
				slice_type = n->nal_reference_idc >> 5;
				slice_type == 3 ? AV_PICTURE_TYPE_I : slice_type == 2 ? AV_PICTURE_TYPE_P : slice_type == 0 ? AV_PICTURE_TYPE_B : -1;
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
				slice_type = AV_PICTURE_TYPE_I;
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
			        if(itype == 6)
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
				slice_type = n->nal_reference_idc >> 5;
				slice_type == 3 ? AV_PICTURE_TYPE_I : slice_type == 2 ? AV_PICTURE_TYPE_P : slice_type == 0 ? AV_PICTURE_TYPE_B : -1;
				//设置FU HEADER,并将这个HEADER填入sendbuf[13]
				if(fu_hdr->S)
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
    //printf("data2nalus: start \n");
    int nal_mem_num = 0;//((int)(size / mtu_size) << 1) + 3;//3:sps/pps/sei
    //printf("rtp_packet2raw: start 0 \n");
    int count = 0;//no used; use rtpLen
	int rtpLen = 0;
	int oSize = 0;

    cJSON *cjsonArr = cJSON_GetObjectItem(json, "rtpSize");
    if( NULL != cjsonArr ){
        int i = 0;
        do
        {
            cJSON *cjsonTmp = cJSON_GetArrayItem(cjsonArr, i);
            if( NULL == cjsonTmp )
            {
                //printf("rtp_packet2raw: no member \n");
                break;
            }
            int num = cjsonTmp->valueint;
            //printf("rtp_packet2raw: num= %d \n", num);
            i++;
        }while(1);

        //int  array_size = cJSON_GetArraySize(cjsonArr);
        //nal_mem_num = array_size;
        nal_mem_num = i;
        //printf("rtp_packet2raw: nal_mem_num= %d \n", nal_mem_num);
        obj->rtpSize = calloc(1, sizeof(short) * nal_mem_num);

        for( int i = 0 ; i < nal_mem_num ; i ++ ){
            cJSON * pSub = cJSON_GetArrayItem(cjsonArr, i);
            if(NULL == pSub ){ continue ; }
            //char * ivalue = pSub->valuestring ;
            int ivalue = pSub->valueint;
            obj->rtpSize[i] = (short)ivalue;//可以不用傳入，通過擴展字段讀入：rtpSize[idx] = rtp_pkt_size;
            rtpLen += ivalue;

        }
    }
    else{
        return ret;
    }
    //printf("rtp_packet2raw: nal_mem_num= %d \n", nal_mem_num);
    svc_nalu->nal_mem_num = nal_mem_num;
    svc_nalu->buf = inBuf;
    svc_nalu->nal = calloc(1, sizeof(NALU_t) * (nal_mem_num << 1) );
    //printf("data2nalus: start 1 \n");
    //for(int i = 0; i < nal_mem_num; i++)
	//{
	//	svc_nalu->nal[i].buf = calloc(1, sizeof(char) * rtpLen);
	//}
	//printf("data2nalus: start 2 \n");
    short *rtpSize = obj->rtpSize;
	ret = rtp_unpacket(svc_nalu, inBuf, rtpSize, count, rtpLen, mtu_size, outBuf, &oSize, &obj->seq_num);
    if(ret < 0)
    {
        //lost packet
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
        obj->json = mystr2json(param);

        //printf("api_rtp_packet2raw: start \n");

        ret = rtp_packet2raw((void *)obj, data, outbuf);

        //printf("api_rtp_packet2raw: ret= %d \n", ret);
#if 0
        if(!test_flag1)
        {
            printf("api_rtp_packet2raw: start test !!!!!!!!!!!\n");
            int offset = 0;
            for(int i = 0; i < ret; i++)
            {
                unsigned char v0 = test_data[i];
                unsigned char v1 = outbuf[offset + i];
                if(v0 != v1)
                {
                    //printf("error: i= %d \n", i);
                    //break;
                    offset += 1;
                    for(int I = i-8; I < i+16; I++)
                    {
                        unsigned char v0 = test_data[I];
                        printf("%02x", v0);
                    }
                    printf("\n");
                    printf("\n");
                    for(int I = i-8; I < i+16; I++)
                    {
                        unsigned char v0 = outbuf[offset + I];
                        printf("%02x", v0);
                    }
                    printf("\n");
                    printf("\n");
                }
                if(i < 2000)
                {
                    //printf("%02x", v0);
                }
            }
            printf("\n");
            printf("\n");
#if 0
            for(int i = 1000; i < ret; i++)
            {
                unsigned char v0 = test_data[i];
                unsigned char v1 = outbuf[offset + i];
                if(v0 != v1)
                {
                    //printf("error: i= %d \n", i);
                    offset += 1;
                }
                if(i < 2000)
                {
                    printf("%02x", v1);
                }
            }
#endif
            printf("\n");
            printf("offset= %d \n", offset);
            test_flag1 = 1;
        }
#endif
        //free
        SVCNalu *svc_nalu = &obj->svc_nalu;
        int nal_mem_num = svc_nalu->nal_mem_num;
	    char *text = obj->outparam[0];
	    int sum = 0;
	    memset(obj->outparam[0], 0, sizeof(char) * MAX_OUTCHAR_SIZE);
	    //printf("api_rtp_packet2raw: nal_mem_num= %d \n", nal_mem_num);
	    for(int i = 0; i < nal_mem_num; i++)
	    {
	        //free(svc_nalu->nal[i].buf);
            char ctmp[32] = "";
            int size = svc_nalu->nal[i].len;
            //printf("api_rtp_packet2raw: size= %d \n", size);
            sum += size;
	        sprintf(ctmp, "%d", size);
	        if(i)
	        {
	            strcat(text, ",");
	        }
            strcat(text, ctmp);
	    }
	    outparam[0] = text;
	    if(sum != ret)
	    {
	        printf("error: api_rtp_packet2raw: sum= %d \n", sum);
	        printf("error:api_rtp_packet2raw: ret= %d \n", ret);
	    }

	    free(svc_nalu->nal);
	    free(obj->rtpSize);
	    deleteJson(obj->json);
	    obj->json = NULL;
	}
    return ret;
}
//
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
	    sprintf(ctmp, "%lld", packet_time_stamp);
		pJsonRoot = renewJsonStr(pJsonRoot, key, ctmp);

		ret = cJSON_Print(pJsonRoot); //free(ret);//返回json字符串，注意外面用完要记得释放空间

		deleteJson(pJsonRoot);

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
int get_frame(void *hnd, char *outbuf, short *rtpSize, char *complete, char *extend_info, long long *frame_timestamp)
{
    int ret = 0;
    RtpObj *obj = (RtpObj *)hnd;
    cJSON *json = obj->json;
    //int insize = GetvalueInt(json, "insize");
    int delay_time = GetvalueInt(json, "delay_time");
    int qos_level = GetvalueInt(json, "qos_level");
    int adapt_cpu = GetvalueInt(json, "adapt_cpu");
    RTP_FIXED_HEADER  *rtp_hdr = NULL;
	EXTEND_HEADER *rtp_ext = NULL;
	NALU_HEADER *nalu_hdr   = NULL;
	FU_INDICATOR *fu_ind     = NULL;
	FU_HEADER *fu_hdr     = NULL;
	uint32_t last_frame_time_stamp = obj->last_frame_time_stamp;
	int packet_distance = obj->max_packet - obj->min_packet;
	long long now_ms = get_sys_time();

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
                printf("warning: get_frame: delay= %u \n", delay);
                //printf("warning: get_frame: delay2= %u \n", delay2);
                printf("warning: get_frame: obj->Obj_id= %u \n", obj->Obj_id);
                printf("warning: get_frame: timestamp0= %u \n", timestamp0);
                printf("warning: get_frame: timestamp1= %u \n", timestamp1);
                printf("warning: get_frame: I0= %u \n", I0);
                printf("warning: get_frame: I1= %u \n", I1);
                printf("warning: get_frame: obj->min_packet= %u \n", obj->min_packet);
                printf("warning: get_frame: obj->max_packet= %u \n", obj->max_packet);
                printf("warning: get_frame: obj->old_seqnum= %u \n", obj->old_seqnum);

                for(int t = obj->min_packet; t <= obj->max_packet; t++)
                {
                    int I = t % obj->buf_size;
                    uint8_t *buf = (uint8_t *)&obj->recv_buf[I][sizeof(int)];
                    int *p = (int *)&obj->recv_buf[I][0];
                    int size = p[0];
                    if(!size)
                        printf("warning: get_frame: t= %d, t.size=%d \n", t, size);
                }
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
                char *p_extend_info = NULL;
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

                    printf("warning: get_frame: obj->Obj_id= %u \n", obj->Obj_id);
                    printf("warning: get_frame: packet_distance= %d \n", packet_distance);
                    printf("warning: get_frame: delay= %d \n", delay);
                    printf("warning: get_frame: min_packet size=%d \n", size);
                    printf("warning: get_frame: obj->min_packet= %d \n", obj->min_packet);
                    printf("warning: get_frame: obj->max_packet= %d \n", obj->max_packet);
                }
                //for(int i = obj->min_packet; i < obj->max_packet; i++)
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
                        printf("error: get_frame: obj->Obj_id= %x \n", obj->Obj_id);
                        printf("error: get_frame: I=%d \n", I);
                        printf("error: get_frame: obj->recv_buf[I]=%x \n", obj->recv_buf[I]);
                    }

                    uint8_t *buf = (uint8_t *)&obj->recv_buf[I][sizeof(int)];
                    int *p = (int *)&obj->recv_buf[I][0];
                    //p[0] = 0;
                    int size = p[0];
                    if(size > MAX_PACKET_SIZE)
                    {
                        printf("error: get_frame: size=%d \n\n\n\n\n\n\n\n\n\n", size);
                    }
                    if(size > 0)
                    {
                        rtp_hdr = (RTP_FIXED_HEADER *)buf;
                        int nal_unit_type = -1;
                        int extlen = 0;
                        int this_seqnum = rtp_hdr->seq_no;
                        RtpInfo info = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

                        int is_hcsvc_rtp = GetRtpInfo((uint8_t*)buf, size, &info);

                        if(this_seq_num != this_seqnum)
                        {
                            printf("error: get_frame: this_seq_num= %d \n", this_seq_num);
                            printf("error: get_frame: this_seqnum= %d \n", this_seqnum);
                            printf("error: get_frame: obj->min_packet= %d \n", obj->min_packet);
                            printf("error: get_frame: obj->max_packet= %d \n", obj->max_packet);
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
                        stop_seq_num = this_seqnum;
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
			                nal_unit_type = rtp_ext->type;
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
			                if (obj->logfp && false) {
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
			                if (obj->logfp && false) {
                                fprintf(obj->logfp, "get_frame: sps: size= %d \n", size);
                                fflush(obj->logfp);
                            }
			            }
			            else if(nal_unit_type == 8)
			            {
			                pps_flag += 1;
			                memcpy(obj->pps, buf, size);//can be used when pps loss and refresh_idr zero
			                printf("get_frame: 2: size= %d \n", size);
			                if (obj->logfp && false) {
                                fprintf(obj->logfp, "get_frame: pps: size= %d \n", size);
                                fflush(obj->logfp);
                            }
			            }
			            else{
			                if(extend_info && !p_extend_info)
                            {
                                p_extend_info = get_extern_info(buf);
                                memcpy(extend_info, p_extend_info, strlen(p_extend_info));
                                free(p_extend_info);
                            }
			            }
                        if(rtp_hdr->marker)
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
                        strcat(complete, ",loss");
                        if(delay > (delay_time << 1))
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
                        now_ms = get_sys_time();
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
                            strcat(complete, ",loss");
                            if(delay > (delay_time * 3))
                            {
                                printf("warning: get_frame: delay= %d, ret= %d, packet_distance= %d \n", delay, ret, packet_distance);

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
                    //obj->last_frame_time = get_sys_time();
                    if(!obj->start_time_stamp)
                    {
                        obj->start_time_stamp = last_timestamp;
                    }
                    if(!obj->start_frame_time)
	                {
	                    obj->start_frame_time = get_sys_time();
	                }
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
                    if (obj->logfp && false) {
                        //printf("get_frame: obj->min_packet= %d \n", obj->min_packet);
                        //printf("get_frame: obj->logfp= %x \n", obj->logfp);
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

#if 0
int correct_seqnum(void *hnd, int seqnum)
{
    RtpObj *obj = (RtpObj *)hnd;
    if(1)
    {
        if((seqnum > HALF_QUART_USHORT) && (!obj->cut_flag))
        {
            //異常包判斷
            //
            //if(!obj->cut_flag)
            {
                obj->cut_flag = 1;
                if(obj->min_packet < QUART_USHORT)
                {
                    obj->min_packet += MAX_USHORT;
                }
                if(obj->max_packet < QUART_USHORT)
                {
                    obj->max_packet += MAX_USHORT;
                }

            }
        }
        if((seqnum > QUART_USHORT && seqnum < HALF_USHORT) && (obj->cut_flag))
        {
            //異常包判斷
            //
            //if(obj->cut_flag)
            {
                obj->cut_flag = 0;
                if(obj->min_packet > MAX_USHORT)
                {
                    obj->min_packet -= MAX_USHORT;
                }
                if(obj->max_packet > MAX_USHORT)
                {
                    obj->max_packet -= MAX_USHORT;
                }
            }

        }
        if(obj->cut_flag)
        {
            if(seqnum < QUART_USHORT)
            {
                seqnum += MAX_USHORT;
            }
        }
        //經過截斷處理後，變得簡單了
        if(seqnum < obj->min_packet)
        {
            obj->min_packet = seqnum;
        }
        else if(seqnum > obj->max_packet)
        {
            obj->max_packet = seqnum;
        }
        else{
            //repeat packet
            printf("correct_seqnum: repeat packet");
        }
    }

    return seqnum;
}
#endif

//(seqnum % obj->buf_size) != ((seqnum + MAX_USHORT) % obj->buf_size);
//(MAX_USHORT % obj->buf_size) != 0;
//(MAX_USHORT % 1024) == 1023;
int resort_packet(void *hnd, char *data, char *outbuf, short *rtpSize, char *complete, char *extend_info, char *outparam[])
{
    int ret = 0;
    //printf("resort_packet: 0 \n");
    RtpObj *obj = (RtpObj *)hnd;
    cJSON *json = obj->json;
    int insize = GetvalueInt(json, "insize");
    int loglevel = GetvalueInt(json, "loglevel");
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
        printf("resort_packet: obj->Obj_id= %x \n", obj->Obj_id);
        printf("resort_packet: obj->buf_size= %d \n", obj->buf_size);
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
    if (obj->logfp && loglevel)
    {
        if(loglevel == 1)
        {
            fprintf(obj->logfp, "%d \n", seqnum);
        }
        else if(loglevel == 2)
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
            ret = get_frame(hnd, outbuf, rtpSize, complete, extend_info, &frame_timestamp);
            //printf("resort_packet: ret= %d \n", ret);
            if(ret > 0)
            {
                sprintf(outparam[3], "%lld", frame_timestamp);
            }
#if 0
            cJSON *json = mystr2json(extend_info);
            int enable_fec = GetvalueInt(json, "enable_fec");
            if(enable_fec)
            {
                obj->json = renewJsonArray2(obj->json, "inSize", rtpSize);
                if(obj->param)
                {
                    free(obj->param);
                }
                obj->param = cJSON_Print(obj->json);
                int total_size = 0;
                for(int i = 0; i < ret; i++)
                {
                    total_size += rtpSize[i];
                }
                char *data2 = malloc(total_size * sizeof(char));
                memcpy(data2, outbuf, total_size);
                ret = api_fec_decode(obj->Obj_id, data, obj->param, outbuf, outparam);
                free(data2);
                //if(!complete)
                //{
                //    //fec decode
                //}
                //else{
                //    //discard
                //}
            }
#endif
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
            //
#if 0
            if(!obj->logfp)
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

        obj->json = mystr2json(param);
        obj->param = param;

        int insize = GetvalueInt(obj->json, "insize");
        if(!insize)
        {
            printf("error: api_resort_packet: insize= %d \n", insize);
            printf("error: api_resort_packet: param= %s \n", param);
        }

        //printf("api_resort_packet: 0 \n");
        short rtpSize[MAX_PKT_NUM];
        //int complete = 0;
        char *extend_info = obj->outparam[1];
        memset(obj->outparam[1], 0, sizeof(char) * MAX_OUTCHAR_SIZE);
        memset(obj->outparam[2], 0, sizeof(char) * 16);
        memset(obj->outparam[3], 0, sizeof(char) * 16);
        outparam[0] = obj->outparam[0];
        outparam[1] = obj->outparam[1];
        outparam[2] = obj->outparam[2];
        outparam[3] = obj->outparam[3];
        char *complete = (char *)outparam[2];
        //printf("api_resort_packet: 1 \n");
        ret = resort_packet(obj, data, outbuf, rtpSize, complete, extend_info, outparam);
        //printf("api_resort_packet: 2 \n");
#if 0
        //char *extend_info = get_extern_info(data);
        if(NULL != extend_info)
        {
            memset(obj->outparam[1], 0, sizeof(char) * MAX_OUTCHAR_SIZE);
            memcpy(obj->outparam[1], extend_info, strlen(extend_info));
            free(extend_info);
            //outparam[1] = extend_info;
            outparam[1] = obj->outparam[1];
        }
#endif
        if(ret > 0)// && !strlen(outparam[0]) && !strlen(outparam[1]))
        {
            //char text[2048] = "";//2048/4=512//512*1400=700kB
            //printf("api_resort_packet: tail \n");

            char *text = obj->outparam[0];
	        int sum = 0;
	        memset(obj->outparam[0], 0, sizeof(char) * MAX_OUTCHAR_SIZE);
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
	        //if(complete)
	        //{
	        //    outparam[2] = "complete";
	        //}

	        //printf("api_resort_packet: sum= %d \n", sum);
        }
        //printf("api_resort_packet: end: ret= %d \n", ret);
        //printf("api_resort_packet: 3 \n");
        deleteJson(obj->json);
        obj->json = NULL;
        //printf("api_resort_packet: 4 \n");
    }
    return ret;
}

//==============================================================================


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


extern cJSON* mystr2json(char *text);
extern int GetvalueInt(cJSON *json, char *key);
extern char* GetvalueStr(cJSON *json, char *key);
extern cJSON* renewJson(cJSON *json, char *key, int ivalue, char *cvalue, cJSON *subJson);
extern cJSON* renewJsonInt(cJSON *json, char *key, int ivalue);
extern cJSON* renewJsonStr(cJSON *json, char *key, char *cvalue);
extern cJSON* deleteJson(cJSON *json);
extern cJSON* renewJsonArray2(cJSON *json, char *key, short *value);
//extern inline int GetRtpInfo(uint8_t* dataPtr, int insize, RtpInfo *info);
extern int64_t get_sys_time();

static void AudioResetObj(AudioRtpObj *obj)
{
    obj->Obj_id = -1;
    obj->logfp = NULL;
    obj->seq_num = 0;
    obj->ssrc = 0;
    obj->time_stamp = 0;
    obj->last_frame_time_stamp = 0;
    obj->start_time_stamp = 0;
    obj->start_frame_time = 0;
    obj->old_seqnum = -1;
    obj->json = NULL;
    obj->param = NULL;
    obj->recv_buf = NULL;
    //obj->send_buf = NULL;
    obj->min_packet = MAX_USHORT;//-1;
    obj->max_packet = -1;
}

int audio_raw2rtp_packet(void *hnd, char *inBuf, char *outBuf)
{
    int ret = 0;
    AudioRtpObj *obj = (AudioRtpObj *)hnd;
    cJSON *json = obj->json;
    int insize = GetvalueInt(json, "insize");
    obj->seq_num = GetvalueInt(json, "seqnum");
    unsigned short seq_num = obj->seq_num;

    RTP_FIXED_HEADER        *rtp_hdr    = NULL;
	AUDIO_EXTEND_HEADER	    *rtp_ext    = NULL;
	int rtp_extend_length = (sizeof(AUDIO_EXTEND_HEADER) >> 2) - 1;//rtp_extend_profile,rtp_extend_length之外1个字（4个字节）
    int time_offset = GetvalueInt(json, "time_offset");
    int extlen = 0;

    rtp_hdr = (RTP_FIXED_HEADER*)&outBuf[0];
	rtp_hdr->payload     = AAC_PLT;  //负载类型号，									PT
	rtp_hdr->version     = 2;  //版本号，此版本固定为2								V
	rtp_hdr->padding	 = 0;//														P
	rtp_hdr->csrc_len	 = 0;//														CC
	rtp_hdr->marker		 = 0;//(size >= len);   //标志位，由具体协议规定其值。		M
	rtp_hdr->ssrc        = (unsigned int)hnd;;//htonl(10);    //随机指定为10，并且在本RTP会话中全局唯一	SSRC
	rtp_hdr->extension	 = 1;//														X
	rtp_hdr->timestamp = obj->time_stamp;;///htonl(ts_current);
	rtp_hdr->seq_no = obj->seq_num;///htons(seq_num ++); //序列号，每发送一个RTP包增1
	if (obj->seq_num >= MAX_USHORT)
	{
		obj->seq_num = 0;
	}
	else{
	    obj->seq_num++;
	}
	if (rtp_hdr->extension)
	{
		rtp_ext = (AUDIO_EXTEND_HEADER *)&outBuf[sizeof(RTP_FIXED_HEADER)];
		memset(rtp_ext, 0, sizeof(AUDIO_EXTEND_HEADER));
		rtp_ext->rtp_extend_profile = 0;
		extlen = (rtp_extend_length + 1) << 2;
		rtp_ext->rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
		//
		rtp_ext->nack.is_lost_packet = 0;
		rtp_ext->nack.lost_packet_rate = 0;
		//if(!time_offset)
		{
		    rtp_ext->nack.time_status = 0;
		    rtp_ext->nack.time_offset = time_offset;
		    rtp_ext->nack.time_info.time_stamp = (int64_t)api_get_time_stamp_ll();
		}
	}
	int header_size = (sizeof(RTP_FIXED_HEADER) + sizeof(AUDIO_EXTEND_HEADER));
	memcpy(&outBuf[header_size], inBuf, insize);
	ret = sizeof(RTP_FIXED_HEADER) + sizeof(AUDIO_EXTEND_HEADER) + insize;


    return ret;
}
HCSVC_API
int api_audio_raw2rtp_packet(char *handle, char *data, char *param, char *outbuf, char *outparam[])
{
    int ret = 0;

    {
        long long *testp = (long long *)handle;
        AudioCodecObj *codecObj = (AudioCodecObj *)testp[0];
        AudioRtpObj *obj = codecObj->rtpObj;//(RtpObj *)&global_rtp_objs[id];
        int id = codecObj->Obj_id;
        if (!obj)
        {
            obj = (AudioRtpObj *)calloc(1, sizeof(AudioRtpObj));
            codecObj->rtpObj = obj;
            AudioResetObj(obj);
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
        int insize = GetvalueInt(obj->json, "insize");


	    ret = audio_raw2rtp_packet(obj, data, outbuf);
	    //printf("api_raw2rtp_packet: ret= %d \n", ret);

        sprintf(obj->outparam[1], "%d", obj->seq_num);
        outparam[1] = obj->outparam[1];
	    //char text[2048] = "";//2048/4=512//512*1400=700kB
	    deleteJson(obj->json);
	    obj->json = NULL;
	}
    return ret;
}

//===================unpacket===================================================
int audio_rtp_packet2raw(void *hnd, char *inBuf, char *outBuf)
{
    int ret = 0;
    AudioRtpObj *obj = (AudioRtpObj *)hnd;
    cJSON *json = obj->json;
    int insize = GetvalueInt(json, "insize");
    //printf("audio_rtp_packet2raw: insize= %d \n", insize);
    RTP_FIXED_HEADER  *rtp_hdr = NULL;
	AUDIO_EXTEND_HEADER *rtp_ext = NULL;
	short extprofile = -1;
	short extlen = 0;
	int ssrc = 0;
	int marker = 0;
	unsigned  long timestamp = 0;
	int last_timestamp = 0;
	int rptHeadSize = sizeof(RTP_FIXED_HEADER);
	int rtp_extend_length = 0;
	static int frmnum = 0;
	int discard = 0;
	int rtp_pkt_size = insize;

	rtp_hdr =(RTP_FIXED_HEADER*)&inBuf[0];
	ssrc = rtp_hdr->ssrc;
	unsigned short seqnum = rtp_hdr->seq_no;
	int diff = seqnum - obj->seq_num;
	if(abs(diff) > 1 && diff != 256 && diff > -256)
	{
		//printf("lost packet: diff=%d \tseqnum=%d \t test_last_seqnum=%d \n", diff, seqnum, *last_seq_num);
		//ret = -1;
	}
	obj->seq_num = seqnum;

	timestamp = rtp_hdr->timestamp;
	marker = rtp_hdr->marker;
	//printf("seqnum=%d \n", seqnum);
	extprofile = -1;
	extlen = 0;
	if(rtp_hdr->extension)
	{
		rtp_ext = (AUDIO_EXTEND_HEADER *)&inBuf[sizeof(RTP_FIXED_HEADER)];
		extprofile = rtp_ext->rtp_extend_profile & 7;
		rtp_extend_length = rtp_ext->rtp_extend_length;
		rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
		extlen = (rtp_extend_length + 1) << 2;
        if (extlen < 0 || extlen > insize) {
           printf("error: audio_rtp_unpacket: extlen=%d \n",extlen);
           return -1;
        }
        //get extend info
        {
	        int is_lost_packet = rtp_ext->nack.is_lost_packet;
	        int lost_packet_rate = rtp_ext->nack.lost_packet_rate;
	        int time_status = rtp_ext->nack.time_status;
	        int time_offset = rtp_ext->nack.time_offset;
	        int64_t packet_time_stamp = rtp_ext->nack.time_info.time_stamp;
        }
    }
    int header_size = (sizeof(RTP_FIXED_HEADER) + sizeof(AUDIO_EXTEND_HEADER));
    ret = insize - header_size;
    memcpy(outBuf, &inBuf[header_size], ret);
    //printf("audio_rtp_packet2raw: header_size=%d \n", header_size);
    //printf("audio_rtp_packet2raw: ret=%d \n", ret);
    return ret;
}
HCSVC_API
int api_audio_rtp_packet2raw(char *handle, char *data, char *param, char *outbuf, char *outparam[])
{
    int ret = 0;

    //initobj(id);
    {
        long long *testp = (long long *)handle;
        AudioCodecObj *codecObj = (AudioCodecObj *)testp[0];
        AudioRtpObj *obj = codecObj->rtpObj;//(RtpObj *)&global_rtp_objs[id];
        int id = codecObj->Obj_id;
        if (!obj)
        {
            obj = (AudioRtpObj *)calloc(1, sizeof(AudioRtpObj));
            codecObj->rtpObj = obj;
            AudioResetObj(obj);
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
        int insize = GetvalueInt(obj->json, "insize");
        //printf("api_audio_rtp_packet2raw: insize= %d \n", insize);

        ret = audio_rtp_packet2raw((void *)obj, data, outbuf);

	    //outparam[0] = text;

	    deleteJson(obj->json);
	    obj->json = NULL;
	}
    return ret;
}

//
static int get_frame(void *hnd, char *outbuf, char *complete, long long *frame_timestamp)
{
    int ret = 0;
    AudioRtpObj *obj = (AudioRtpObj *)hnd;
    cJSON *json = obj->json;
    //int insize = GetvalueInt(json, "insize");
    int delay_time = GetvalueInt(json, "delay_time");
    int qos_level = GetvalueInt(json, "qos_level");
    RTP_FIXED_HEADER  *rtp_hdr = NULL;
	AUDIO_EXTEND_HEADER *rtp_ext = NULL;

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
                printf("warning: audio: get_frame: delay= %u \n", delay);
                //printf("warning: get_frame: delay2= %u \n", delay2);
                printf("warning: audio: get_frame: obj->Obj_id= %u \n", obj->Obj_id);
                printf("warning: audio: get_frame: timestamp0= %u \n", timestamp0);
                printf("warning: audio: get_frame: timestamp1= %u \n", timestamp1);
                printf("warning: audio: get_frame: I0= %u \n", I0);
                printf("warning: audio: get_frame: I1= %u \n", I1);
                printf("warning: audio: get_frame: obj->min_packet= %u \n", obj->min_packet);
                printf("warning: audio: get_frame: obj->max_packet= %u \n", obj->max_packet);
                printf("warning: audio: get_frame: obj->old_seqnum= %u \n", obj->old_seqnum);

                for(int t = obj->min_packet; t <= obj->max_packet; t++)
                {
                    int I = t % obj->buf_size;
                    uint8_t *buf = (uint8_t *)&obj->recv_buf[I][sizeof(int)];
                    int *p = (int *)&obj->recv_buf[I][0];
                    int size = p[0];
                    if(!size)
                        printf("warning: audio: get_frame: t= %d, t.size=%d \n", t, size);
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
                int marker = 0;
                int64_t nack_time = 0;
                unsigned  int timestamp = 0;
                unsigned  int start_timestamp = 0;
                unsigned  int last_timestamp = 0;
                int j = 0;
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

                    printf("warning: audio: get_frame: obj->Obj_id= %u \n", obj->Obj_id);
                    printf("warning: audio: get_frame: packet_distance= %d \n", packet_distance);
                    printf("warning: audio: get_frame: delay= %d \n", delay);
                    printf("warning: audio: get_frame: min_packet size=%d \n", size);
                    printf("warning: audio: get_frame: obj->min_packet= %d \n", obj->min_packet);
                    printf("warning: audio: get_frame: obj->max_packet= %d \n", obj->max_packet);
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
                        printf("error: audio: get_frame: obj->Obj_id= %x \n", obj->Obj_id);
                        printf("error: audio: get_frame: I=%d \n", I);
                        printf("error: audio: get_frame: obj->recv_buf[I]=%x \n", obj->recv_buf[I]);
                    }

                    uint8_t *buf = (uint8_t *)&obj->recv_buf[I][sizeof(int)];
                    int *p = (int *)&obj->recv_buf[I][0];
                    //p[0] = 0;
                    int size = p[0];
                    if(size > MAX_PACKET_SIZE)
                    {
                        printf("error: audio: get_frame: size=%d \n\n\n\n\n\n\n\n\n\n", size);
                    }
                    if(size > 0)
                    {
                        rtp_hdr = (RTP_FIXED_HEADER *)buf;
                        int nal_unit_type = -1;
                        int extlen = 0;
                        int this_seqnum = rtp_hdr->seq_no;

                        if(this_seq_num != this_seqnum)
                        {
                            printf("error: audio: get_frame: this_seq_num= %d \n", this_seq_num);
                            printf("error: audio: get_frame: this_seqnum= %d \n", this_seqnum);
                            printf("error: audio: get_frame: obj->min_packet= %d \n", obj->min_packet);
                            printf("error: audio: get_frame: obj->max_packet= %d \n", obj->max_packet);
                        }
                        //last_frame_time_stamp
                        timestamp = rtp_hdr->timestamp;

                        if(!start_timestamp)
                        {
                            start_timestamp = timestamp;
                            //obj->min_packet = this_seqnum;
                            start_seq_num = this_seqnum;
                            stop_seq_num = this_seqnum;
                            if(rtp_hdr->extension)
			                {
				                rtp_ext = (AUDIO_EXTEND_HEADER *)&buf[sizeof(RTP_FIXED_HEADER)];
                                nack_time = rtp_ext->nack.time_info.time_stamp;
                            }
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
                            //ret = j;
                            frame_timestamp[0] = nack_time;//timestamp;
                            //printf("get_frame: j= %d \n", j);
                            test_end_packet = 1;
                            break;
                        }
                        else{
                            //printf("get_frame: size=%d \n", size);
                            memcpy(&outbuf[offset], buf, size);
                            offset += size;
                            ret = size;
                            j += 1;
                        }
                        stop_seq_num = this_seqnum;

                        if(last_seq_num >= 0)
                        {
                            int diff_seqnum = this_seqnum - last_seq_num;
                            if(this_seqnum < last_seq_num)
                            {
                                diff_seqnum = (this_seqnum + LEFT_SHIFT16) - last_seq_num;
                            }
                            loss_num += (diff_seqnum - 1);
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

                {
                    if((!loss_num))
                    {
                        int ref_idc = 0;
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
                            int ref_idc = 0;
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
                                printf("get_frame: obj->min_packet= %d \n", obj->min_packet);
                                printf("get_frame: obj->max_packet= %d \n", obj->max_packet);
                                printf("get_frame: obj->old_seqnum= %d \n", obj->old_seqnum);
                                printf("get_frame: ret= %d \n", ret);
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

int audio_resort_packet(void *hnd, char *data, char *outbuf, char *complete, char *outparam[])
{
    int ret = 0;
    //printf("resort_packet: 0 \n");
    AudioRtpObj *obj = (AudioRtpObj *)hnd;
    cJSON *json = obj->json;
    int insize = GetvalueInt(json, "insize");
    int loglevel = GetvalueInt(json, "loglevel");
    int delay_time = GetvalueInt(json, "delay_time");
    int min_distance = GetvalueInt(json, "min_distance");///10
    RTP_FIXED_HEADER  *rtp_hdr = NULL;
	uint32_t last_frame_time_stamp = obj->last_frame_time_stamp;

    //printf("audio_resort_packet: delay_time= %d \n", delay_time);
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
    //printf("audio_resort_packet: min_distance= %d \n", min_distance);
    //printf("audio_resort_packet: obj->buf_size= %d \n", obj->buf_size);
    //分配固定  長度的緩存空間，長度參數可調
    //printf("resort_packet: obj->buf_size= %d \n", obj->buf_size);
    //printf("audio_resort_packet: obj->recv_buf= %x \n", obj->recv_buf);
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

    //printf("audio_resort_packet: 0: is_old_packet= %d \n", is_old_packet);
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
    //printf("audio_resort_packet: 1: is_old_packet= %d \n", is_old_packet);
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
        //printf("audio_resort_packet: insize= %d \n", insize);
        p[0] = insize;
    }
    else{
        printf("warning: audio_resort_packet: old packet: insize= %d \n", insize);
    }
    //printf("audio_resort_packet: obj->min_packet= %d \n", obj->min_packet);
    //printf("audio_resort_packet: obj->max_packet= %d \n", obj->max_packet);
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
            ret = get_frame(hnd, outbuf, complete, &frame_timestamp);
            //printf("resort_packet: ret= %d \n", ret);
            if(ret > 0)
            {
                sprintf(outparam[3], "%lld", frame_timestamp);
            }

        }
    }
    return ret;
}
HCSVC_API
int api_audio_resort_packet(char *handle, char *data, char *param, char *outbuf, char *outparam[])
{
    int ret = 0;
    //initobj(id);
    //printf("api_resort_packet: 0 \n");
    //if (id < MAX_OBJ_NUM)
    {
        long long *testp = (long long *)handle;
        AudioCodecObj *codecObj = (AudioCodecObj *)testp[0];
        AudioRtpObj *obj = codecObj->resortObj;//(RtpObj *)&global_rtp_objs[id];
        //printf("api_audio_resort_packet: codecObj= %x \n", codecObj);
        //printf("api_audio_resort_packet: codecObj->resortObj= %x \n", codecObj->resortObj);
        int id = codecObj->Obj_id;
        if (!obj)
        {
            obj = (AudioRtpObj *)calloc(1, sizeof(AudioRtpObj));
            codecObj->resortObj = obj;
            AudioResetObj(obj);
            obj->Obj_id = id;
            printf("api_audio_resort_packet: obj->recv_buf= %x \n", obj->recv_buf);
            //
#if 0
            if(!obj->logfp)
            {
                char filename[256] = "/home/gxh/works/audio_rtp_resort_gxh_";
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

        memset(obj->outparam[1], 0, sizeof(char) * MAX_OUTCHAR_SIZE);
        memset(obj->outparam[2], 0, sizeof(char) * 16);
        memset(obj->outparam[3], 0, sizeof(char) * 16);
        outparam[0] = obj->outparam[0];
        outparam[1] = obj->outparam[1];
        outparam[2] = obj->outparam[2];
        outparam[3] = obj->outparam[3];
        char *complete = (char *)outparam[2];
        //printf("api_resort_packet: 1 \n");
        //printf("audio_resort_packet: insize= %d \n", insize);
        ret = audio_resort_packet(obj, data, outbuf, complete, outparam);

        if(ret > 0)// && !strlen(outparam[0]) && !strlen(outparam[1]))
        {
        }
        deleteJson(obj->json);
        obj->json = NULL;
    }
    return ret;
}

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

extern int GetvalueInt(cJSON *json, char *key);
extern int *GetArrayValueInt(cJSON *json, char *key, int *arraySize);
extern long long *GetArrayObj(cJSON *json, char *key, int *arraySize);
extern cJSON* renewJsonArray3(cJSON **json, cJSON **array, char *key, cJSON *item);
extern cJSON *get_net_info2(NetInfo *info, cJSON *pJsonRoot);
extern int GetAudioNetInfo(uint8_t* dataPtr, int insize, NetInfo *info, int times);


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
    obj->min_packet2 = -1;
    obj->max_packet2 = -1;
    obj->min_packet = MAX_USHORT;//-1;
    obj->max_packet = -1;
    //
    obj->delay_time = 0;
    obj->offset_time = 1;
    obj->delay_info[0].delay = -1;
    obj->delay_info[0].max_delay = 0;
    obj->delay_info[0].last_max_delay = 0;
    obj->delay_info[0].sum_delay = 0;
    obj->delay_info[0].cnt_delay = 0;
    obj->delay_info[0].cnt_max = 1;
    obj->delay_info[1].delay = -1;
    obj->delay_info[1].max_delay = 0;
    obj->delay_info[1].last_max_delay = 0;
    obj->delay_info[1].sum_delay = 0;
    obj->delay_info[1].cnt_delay = 0;
    obj->delay_info[1].cnt_max = 10;
    obj->delay_info[2].delay = -1;
    obj->delay_info[2].max_delay = 0;
    obj->delay_info[2].last_max_delay = 0;
    obj->delay_info[2].sum_delay = 0;
    obj->delay_info[2].cnt_delay = 0;
    obj->delay_info[2].cnt_max = 100;
}
int audio_raw2rtp_packet(void *hnd, char *inBuf, char *outBuf)
{
    int ret = 0;
    //printf("audio_raw2rtp_packet:start \n");
    AudioRtpObj *obj = (AudioRtpObj *)hnd;
    cJSON *json = obj->json;
    int selfChanId = GetvalueInt(json, "selfChanId");
    int chanId = GetvalueInt(json, "chanId");//已增1
    int loss_rate = GetvalueInt(json, "loss_rate");
    int *speakerList = NULL;
    int speakerNum = 0;
    int maxSpeakerId = -1;
    long long *objList = NULL;
    long long *objList2 = NULL;
    int arraySize = 0;
    int arraySize2 = 0;
    int netIdx = 0;

    int insize = GetvalueInt(json, "insize");
    obj->seq_num = GetvalueInt(json, "seqnum");
    //unsigned short seq_num = obj->seq_num;

    RTP_FIXED_HEADER        *rtp_hdr    = NULL;
	AUDIO_EXTEND_HEADER	    *rtp_ext    = NULL;
	int rtp_extend_length = (sizeof(AUDIO_EXTEND_HEADER) >> 2) - 1;//rtp_extend_profile,rtp_extend_length之外1个字（4个字节）
    int time_offset = GetvalueInt(json, "time_offset");
    int extlen = 0;

    if(selfChanId > 0)
    {
        objList = GetArrayObj(json, "netInfo", &arraySize);
        objList2 = GetArrayObj(json, "rttInfo", &arraySize2);
        //printf("audio_raw2rtp_packet: arraySize2= %d\n",arraySize2);
        speakerList = GetArrayValueInt(json, "speakers", &speakerNum);
        for(int i = 0; i < speakerNum; i++)
        {
            int thisSpeakerId = speakerList[i];
            if(thisSpeakerId > maxSpeakerId)
            {
                maxSpeakerId = thisSpeakerId;
            }
        }
    }

    rtp_hdr = (RTP_FIXED_HEADER*)&outBuf[0];
	rtp_hdr->payload     = AAC_PLT;  //负载类型号，									PT
	rtp_hdr->version     = 2;  //版本号，此版本固定为2								V
	rtp_hdr->padding	 = 0;//														P
	rtp_hdr->csrc_len	 = 0;//														CC
	rtp_hdr->marker		 = 0;//(size >= len);   //标志位，由具体协议规定其值。		M
	rtp_hdr->ssrc        = (unsigned int)hnd;;//htonl(10);    //随机指定为10，并且在本RTP会话中全局唯一	SSRC
	rtp_hdr->extension	 = 1;//														X
	rtp_hdr->timestamp = obj->time_stamp;///htonl(ts_current);
	//printf("audio_raw2rtp_packet: obj->time_stamp= %u \n", obj->time_stamp);
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
		rtp_ext->rtp_extend_profile = EXTEND_PROFILE_ID;//0;
		extlen = (rtp_extend_length + 1) << 2;
		rtp_ext->rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
		//
		{
		    //rtp_ext->seq_no = rtp_hdr->seq_no;
		    rtp_ext->nack.nack0.info_status = 0;
		    rtp_ext->nack.nack0.enable_nack = 0;
			rtp_ext->nack.nack0.is_lost_packet = 0;
			rtp_ext->nack.nack0.loss_rate = 0;
			rtp_ext->nack.nack0.time_offset = time_offset;
			int64_t now_time = (int64_t)api_get_time_stamp_ll();
			rtp_ext->nack.nack0.time_info.time_stamp0 = now_time & 0xFFFFFFFF;
			rtp_ext->nack.nack0.time_info.time_stamp1 = (now_time >> 32) & 0xFFFFFFFF;
			RTT_HEADER *rtt_list = (RTT_HEADER *)&rtp_ext->nack.nack0.time_info.rtt_list;

			//printf("audio_raw2rtp_packet: selfChanId=%d \n", selfChanId);
			if(selfChanId > 0 && speakerNum > 0)
			{
			    int I = obj->netIdx % (maxSpeakerId + 1);
			    //printf("audio_raw2rtp_packet: maxSpeakerId=%d \n", maxSpeakerId);
			    //printf("audio_raw2rtp_packet: I=%d \n", I);
                //printf("audio_raw2rtp_packet: selfChanId=%d \n", selfChanId);
                //printf("audio_raw2rtp_packet: maxSpeakerId=%d \n", maxSpeakerId);
	    		if(I == (selfChanId - 1))//if(!(obj->seq_num & 1))
		    	{
			        rtp_ext->nack.nack0.enable_nack = 1;
				    rtt_list->rtt0.st0 = now_time & 0xFFFF;
    				rtt_list->rtt0.rt0 = 0;
	    			rtt_list->rtt0.st1 = 0;
		    		rtt_list->rtt0.rt1 = 0;
			    	rtp_ext->nack.nack0.chanId = selfChanId;
				    rtp_ext->nack.nack0.loss_rate = 0;
    				rtp_ext->nack.nack0.info_status = 0;
    				if(objList2)
    				{
    				    cJSON *thisjson = (cJSON *)objList2[0];
                        if(thisjson)
                        {
                            int decodeId = GetvalueInt(thisjson, "decodeId");
                            int rtt = GetvalueInt(thisjson, "rtt");
                            rtt_list->rtt1.rtt = (rtt + 7) >> 3;
		    		        rtt_list->rtt1.decodeId = decodeId;
                        }
    				}
    				//printf("audio_raw2rtp_packet: rtp_ext->nack.nack0.info_status=%d \n", rtp_ext->nack.nack0.info_status);
	    		}
		    	else{
			        //接收端收到了对端的时间信息
			        //信息中含有终极信息接收者的ID
    				if(netIdx < arraySize && objList)
	    			{
		    		    cJSON *thisjson = (cJSON *)objList[netIdx++];
                        if(thisjson)
                        {
                            int chanId = GetvalueInt(thisjson, "chanId");//已增1
                            if(true)//if((chanId - 1) == I)//如何安排，需要优化
                           {
                                int loss_rate = GetvalueInt(thisjson, "loss_rate");
                                int st0 = GetvalueInt(thisjson, "st0");
                                int rt0 = GetvalueInt(thisjson, "rt0");
                                //int st1 = GetvalueInt(thisjson, "st1");
                                //int rt1 = GetvalueInt(thisjson, "rt1");
                                //printf("%d, %d, %d, %d, %d, %d \n", chanId, loss_rate, st0, rt0, st1, rt1);
				                rtp_ext->nack.nack0.enable_nack = 1;
    				            rtp_ext->nack.nack0.loss_rate = loss_rate;//loss_rate in [1,101]
	    			            rtt_list->rtt0.st0 = st0;
		    		            rtt_list->rtt0.rt0 = rt0;
			    	            rtt_list->rtt0.st1 = now_time & 0xFFFF;
			    	            rtt_list->rtt0.rt1 = 0;
				                rtp_ext->nack.nack0.chanId = chanId;
				                rtp_ext->nack.nack0.info_status = 1;
				                //printf("audio_raw2rtp_packet: rtp_ext->nack.nack0.info_status=%d \n", rtp_ext->nack.nack0.info_status);
				                if(objList2)
    				            {
    				                cJSON *thisjson = (cJSON *)objList2[0];
                                    if(thisjson)
                                    {
                                        int decodeId = GetvalueInt(thisjson, "decodeId");
                                        int rtt = GetvalueInt(thisjson, "rtt");
                                        rtt_list->rtt1.rtt = (rtt + 7) >> 3;
		    		                    rtt_list->rtt1.decodeId = decodeId;
                                    }
    				            }
                            }

				        }
				    }
			    }
			    obj->netIdx++;
			}

		}
	}
	int header_size = (sizeof(RTP_FIXED_HEADER) + sizeof(AUDIO_EXTEND_HEADER));
	memcpy(&outBuf[header_size], inBuf, insize);
	ret = sizeof(RTP_FIXED_HEADER) + sizeof(AUDIO_EXTEND_HEADER) + insize;
    if(objList)
	{
	    free(objList);
	}
	if(objList2)
	{
	    free(objList2);
	}
	if(speakerList)
	{
	    free(speakerList);
	}


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
            obj->time_stamp += 40;//test
        }
        int insize = GetvalueInt(obj->json, "insize");

	    ret = audio_raw2rtp_packet(obj, data, outbuf);
	    //printf("api_raw2rtp_packet: ret= %d \n", ret);

        sprintf(obj->outparam[1], "%d", obj->seq_num);
        outparam[1] = obj->outparam[1];
	    //char text[2048] = "";//2048/4=512//512*1400=700kB
	    api_json_free(obj->json);
	    obj->json = NULL;
	}
    return ret;
}
#if 0
HCSVC_API
int api_audio_reset_seqnum(void *hnd, uint8_t* dataPtr, int insize, int seq_num)
{
    int ret = 0;
    //RTP_FIXED_HEADER* rtp_hdr = (RTP_FIXED_HEADER*)&dataPtr[0];
    AUDIO_EXTEND_HEADER *rtp_ext = NULL;
    rtp_ext = (AUDIO_EXTEND_HEADER *)&dataPtr[sizeof(RTP_FIXED_HEADER)];
    if(seq_num >= 0)
    {
        rtp_ext->seq_no = seq_num;
        if (seq_num >= MAX_USHORT)
	    {
		    seq_num = 0;
    	}
	    else{
	        seq_num++;
	    }
    }
    else if(hnd){
        AudioRtpObj *obj = (AudioRtpObj *)hnd;
        seq_num = obj->seq_num;//rtp_hdr->seq_no;
        if (seq_num >= MAX_USHORT)
	    {
		    seq_num = 0;
    	}
	    else{
	        seq_num++;
	    }
	    rtp_ext->seq_no = seq_num;
	    obj->seq_num = seq_num;
    }
    ret = seq_num;
    return ret;
}
#endif
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
	unsigned int rtp_pkt_size = insize;

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
		extprofile = rtp_ext->rtp_extend_profile;// & 7;
		rtp_extend_length = rtp_ext->rtp_extend_length;
		rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
		extlen = (rtp_extend_length + 1) << 2;
        if (extlen < 0 || extlen > insize) {
           printf("error: audio_rtp_unpacket: extlen=%d \n",extlen);
           printf("error: audio_rtp_unpacket: insize=%d \n",insize);
           return -1;
        }
        //get extend info
        {
	        int is_lost_packet = rtp_ext->nack.nack0.is_lost_packet;
	        int loss_rate = rtp_ext->nack.nack0.loss_rate;
	        //int time_status = rtp_ext->nack.nack0.time_status;
	        int time_offset = rtp_ext->nack.nack0.time_offset;
	        uint64_t time_stamp0 = rtp_ext->nack.nack0.time_info.time_stamp0;
	        uint64_t time_stamp1 = rtp_ext->nack.nack0.time_info.time_stamp1;
	        int64_t packet_time_stamp = time_stamp0 | (time_stamp1 << 32);
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
        obj->json = (cJSON *)api_str2json(param);
        int insize = GetvalueInt(obj->json, "insize");
        //printf("api_audio_rtp_packet2raw: insize= %d \n", insize);

        ret = audio_rtp_packet2raw((void *)obj, data, outbuf);

	    //outparam[0] = text;

	    api_json_free(obj->json);
	    obj->json = NULL;
	}
    return ret;
}

#if 0
//注意：冗余包未计入丢包率统计，将会严重影响丢包率统计的准确性
//实验结果证明，该算法是正确的，只是精度，依赖于延迟时间的长度，
//单位时间内，音频包的个数少，因此，要获得“稳定的“结果，需要足够长的延时（一般一秒以上）
//但更贴近真实的是：丢包率是波动的，所谓的稳定，一定是在某一时间尺度上而言的
//滑动窗前段是历史数据，后段是更新数据，更新数据尽量相对短，以提高响应速度，
//但同时它也是丢包判定的容限，过短会形成丢包误判
static int count_loss_rate(void *hnd)
{
    int ret = -1;
    AudioRtpObj *obj = (AudioRtpObj *)hnd;
    cJSON *json = obj->json;
    int delay_time = GetvalueInt(json, "delay_time");//这是真正的丢包临界点
    int max_pkt_delay = delay_time;//丢包判定的容限
    delay_time = delay_time << 3;
    //max_pkt_delay是给予包达到的最大延迟时间，否则，判断丢包
    //int threshold = delay_time >> 1;//实际上，这是申请丢包（心理预计丢包）重发的临界点
    //由于超过delay_time的包已被取走，而没有设计多余的缓存，退而求其次，用丢包申请临界点代替丢包临界点
    //更合理的做法，应该是另建缓存，来计算丢包率
    //一个更好的处理方法是：在足够大的缓存中，重复利用历史数据
    RTP_FIXED_HEADER  *rtp_hdr = NULL;
    if(true)
    {
        int64_t time_stamp = get_sys_time();
        if(!obj->net_time_stamp)
        {
            obj->net_time_stamp = time_stamp;
            return ret;
        }
        int diff_time = (int)(time_stamp - obj->net_time_stamp);
        //实际延时:检测时间间隔
        if(diff_time > delay_time)
        {
            //printf("count_loss_rate: diff_time=%d \n", diff_time);
            //printf("count_loss_rate: obj->buf_size=%d \n", obj->buf_size);
            int I1 = obj->max_packet % obj->buf_size;
            uint8_t *buf1 = (uint8_t *)&obj->recv_buf[I1][sizeof(int)];
            RTP_FIXED_HEADER *rtp_hdr1 = (RTP_FIXED_HEADER *)buf1;
            int *p1 = (int *)&obj->recv_buf[I1][0];
            int size1 = p1[0];
            int64_t timestamp1 = rtp_hdr1->timestamp;//基准时间
            int returnflag = 0;
            int min_packet = obj->min_packet2;
            if(min_packet < 0)
            {
                returnflag = 1;
                min_packet = obj->min_packet;
                int start = min_packet + LEFT_SHIFT16;
                int count = 0;
                do
                {
                    start -= 8;//快速收敛
                    int I0 = start % obj->buf_size;
                    uint8_t *buf0 = (uint8_t *)&obj->recv_buf[I0][sizeof(int)];
                    RTP_FIXED_HEADER *rtp_hdr0 = (RTP_FIXED_HEADER *)buf0;
                    int *p0 = (int *)&obj->recv_buf[I0][0];
                    int size0 = p0[0];
                    if(size0 > 0)
                    {
                        int64_t timestamp0 = rtp_hdr0->timestamp;
                        int delay = (int)(timestamp1 - timestamp0) / 27;//27000Hz
                        if(timestamp0 > timestamp1)
                        {
                            delay = (int)((timestamp1 + LEFT_SHIFT32 - timestamp0) / 27);
                        }
                        if(delay > (delay_time + max_pkt_delay))
                        {
                            min_packet = rtp_hdr0->seq_no;
                            returnflag = 0;
                            printf("count_loss_rate: init min_packet2=%d \n", min_packet);
                            break;
                        }
                    }
                    else{
                        break;
                    }
                    count++;
                    if(count >= obj->buf_size)
                    {
                        break;
                    }
                }while(true);
            }
            if(returnflag)
            {
                return ret;
            }
            int I0 = min_packet % obj->buf_size;
            uint8_t *buf0 = (uint8_t *)&obj->recv_buf[I0][sizeof(int)];
            RTP_FIXED_HEADER *rtp_hdr0 = (RTP_FIXED_HEADER *)buf0;
            int *p0 = (int *)&obj->recv_buf[I0][0];
            int size0 = p0[0];
            int64_t timestamp0 = rtp_hdr0->timestamp;
            int delay = (int)(timestamp1 - timestamp0) / 27;//27000Hz
            if(timestamp0 > timestamp1)
            {
                delay = (int)((timestamp1 + LEFT_SHIFT32 - timestamp0) / 27);
            }
            //printf("count_loss_rate: delay=%d \n", delay);
            //包间距,保证缓存中多出MAX_PKT_DELAY冗余
            if(delay > (delay_time + max_pkt_delay))
            {
                //printf("count_loss_rate: delay=%d \n", delay);
                ret = 0;
                int stop_seq_num = -1;
                int start = min_packet;
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

                        //if(this_timestamp >= 0 && this_seqnum >= 0)
                        {
                            //检测单元的时间跨度
                            delay = (int)(this_timestamp - timestamp0) / 27;//27000Hz
                            if(timestamp0 > this_timestamp)
                            {
                                delay = (int)((this_timestamp + LEFT_SHIFT32 - timestamp0) / 27);
                            }
                            //
                            if(last_seq_num >= 0)
                            {
                                int diff_seqnum = this_seqnum - last_seq_num;
                                if(this_seqnum < last_seq_num)
                                {
                                    diff_seqnum = (this_seqnum + LEFT_SHIFT16) - last_seq_num;
                                }
                                loss_num += (diff_seqnum - 1);
                            }
                            pkt_num = i - start + 1;
                            last_seq_num = this_seqnum;
                            //printf("count_loss_rate: delay=%d \n", delay);
                            if(delay >= delay_time)
                            {
                                if(!pkt_num)
                                {
                                    printf("error: count_loss_rate: start= %d, end= %d \n", start, end);
                                }
                                ret = (int)(100 * ((float)loss_num / pkt_num));
                                if(loss_num != j)
                                {
                                    printf("warning: CountLossRate: j= %d, loss_num= %d \n", j, loss_num);
                                }
                                //printf("CountLossRate: loss_num= %d, pkt_num= %d, ret=%d \n", loss_num, pkt_num, ret);
                                //printf("CountLossRate: ret= %d \n", ret);
                                obj->net_time_stamp = get_sys_time();
                                stop_seq_num = this_seqnum;
                                break;
                            }
                        }
                    }
                    else{
                        j++;
                        //printf("CountLossRate: j= %d, pkt_num= %d \n", j, pkt_num);
                    }
                }//i
                if(pkt_num)
                {
                    ret = (int)(100 * ((float)loss_num / pkt_num));
                }
                //printf("CountLossRate: loss_num= %d, pkt_num= %d, ret=%d \n", loss_num, pkt_num, ret);
                //printf("count_loss_rate: stop_seq_num=%d \n", stop_seq_num);
                if(stop_seq_num >= 0)
                {
                    //clear buffer
                    int start = min_packet;
                    int end = stop_seq_num;
                    if(start > end)
                    {
                        end += LEFT_SHIFT16;//MAX_USHORT;
                    }
                    //for(int i = obj->min_packet; i < obj->max_packet; i++)
                    for(int i = start; i <= end; i++)
                    {
                        int this_seq_num = i % LEFT_SHIFT16;//i > MAX_USHORT ? (i - MAX_USHORT) : i;
                        int I = this_seq_num % obj->buf_size;
                        //memcpy(&obj->recv_buf[I][sizeof(int)], data, insize);
                        uint8_t *buf = (uint8_t *)&obj->recv_buf[I][sizeof(int)];
                        int *p = (int *)&obj->recv_buf[I][0];
                        int size = p[0];
                        if(size > 0)
                        {
                            rtp_hdr = (RTP_FIXED_HEADER *)buf;
                            //rtp_hdr->timestamp = 0;
                            //rtp_hdr->seq_no = 0;
                        }
                        p[0] = 0;
                    }
                    obj->min_packet2 = stop_seq_num + 1;
                }
            }
        }
    }
    //printf("count_loss_rate: end: ret=%d \n", ret);
    return ret;
}
#endif
static int get_frame(void *hnd, char *outbuf, char *complete, char *extend_info, long long *frame_timestamp)
{
    int ret = 0;
    AudioRtpObj *obj = (AudioRtpObj *)hnd;
    cJSON *json = obj->json;
    cJSON *jsonInfo = NULL;
    cJSON *jsonArray = NULL;
    //int insize = GetvalueInt(json, "insize");
    int delay_time = GetvalueInt(json, "delay_time");
    int qos_level = GetvalueInt(json, "qos_level");
    int selfChanId = GetvalueInt(json, "selfChanId");
    int loss_rate = GetvalueInt(json, "loss_rate");
    int chanNum = 0;
    int *chanIdList = GetArrayValueInt(json, "chanId", &chanNum);
    int counted_loss = 0;
    int counted_loss_rate = 0;
    RTP_FIXED_HEADER  *rtp_hdr = NULL;
	AUDIO_EXTEND_HEADER *rtp_ext = NULL;
    if(obj->delay_time > 0)
    {
        delay_time = obj->delay_time;
        //printf("get_frame: delay_time= %d \n", delay_time);
        if(delay_time > MAX_DELAY_TIME)
        {
            delay_time = MAX_DELAY_TIME;
        }
    }
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
            int delay = (int)(timestamp1 - timestamp0) / 27;//27000Hz
            if(timestamp0 > timestamp1)
            {
                delay = (int)((timestamp1 + LEFT_SHIFT32 - timestamp0) / 27);
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
                    if(size <= 1)
                    {
                        printf("warning: audio: get_frame: t= %d, t.size=%d \n", t, size);
                        break;
                    }

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
                int this_seqnum = -1;
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
                    if(size > 1)
                    {
                        rtp_hdr = (RTP_FIXED_HEADER *)buf;
                        int nal_unit_type = -1;
                        int extlen = 0;
                        this_seqnum = rtp_hdr->seq_no;

                        if(this_seq_num != this_seqnum)
                        {
                            printf("error: audio: get_frame: this_seq_num= %d \n", this_seq_num);
                            printf("error: audio: get_frame: this_seqnum= %d \n", this_seqnum);
                            printf("error: audio: get_frame: obj->min_packet= %d \n", obj->min_packet);
                            printf("error: audio: get_frame: obj->max_packet= %d \n", obj->max_packet);
                        }
                        //last_frame_time_stamp
                        timestamp = rtp_hdr->timestamp;
                        //printf("audio: get_frame: timestamp= %u \n", timestamp);
                        //printf("audio: get_frame: this_seqnum= %u \n", this_seqnum);

                        if(!start_timestamp)
                        {
                            start_timestamp = timestamp;
                            //obj->min_packet = this_seqnum;
                            start_seq_num = this_seqnum;
                            stop_seq_num = this_seqnum;
                            if(rtp_hdr->extension)
			                {
				                rtp_ext = (AUDIO_EXTEND_HEADER *)&buf[sizeof(RTP_FIXED_HEADER)];
				                uint64_t time_stamp0 = rtp_ext->nack.nack0.time_info.time_stamp0;
	                            uint64_t time_stamp1 = rtp_ext->nack.nack0.time_info.time_stamp1;
	                            nack_time = time_stamp0 | (time_stamp1 << 32);
                                //nack_time = rtp_ext->nack.nack0.time_info.time_stamp;
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
                            if(last_seq_num >= 0)
                            {
                                int diff_seqnum = this_seqnum - last_seq_num;
                                if(this_seqnum < last_seq_num)
                                {
                                    diff_seqnum = (this_seqnum + LEFT_SHIFT16) - last_seq_num;
                                }
                                loss_num += (diff_seqnum - 1);
                            }
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
                            if(selfChanId > 0)
                            {
                                //printf("get_frame: selfChanId= %d \n", selfChanId);
                                NetInfo netInfo = {};
                                int enable_nack = GetAudioNetInfo((uint8_t*)buf, size, &netInfo, 1);
                                if(enable_nack)
                                {
                                    //printf("get_frame: netInfo.info_status= %d \n", netInfo.info_status);
                                    //printf("get_frame: netInfo.chanId= %d \n", netInfo.chanId);
                                    if(!netInfo.info_status)
                                    {
                                        //待反馈信息，非测试情况下，存在netInfo.chanId == selfChanId
                                        //printf("get_frame: netInfo.st0= %d \n", netInfo.st0);
                                        //printf("get_frame: netInfo.rt0= %d \n", netInfo.rt0);
                                        //printf("get_frame: netInfo.chanId= %d \n", netInfo.chanId);
                                        cJSON *thisjson = NULL;
#if 0
                                        if(!counted_loss)
                                        {
                                            //printf("get_frame: obj->buf_size=%d \n", obj->buf_size);
                                            counted_loss_rate = count_loss_rate(hnd);
                                            //printf("get_frame: counted_loss_rate= %d \n", counted_loss_rate);
                                        }
                                        if(counted_loss_rate >= 0)
                                        {
                                            netInfo.loss_rate = counted_loss_rate + 1;
                                        }
#else
                                        if(loss_rate > 0)
                                        {
                                            netInfo.loss_rate = loss_rate;// + 1;
                                        }
#endif
                                        thisjson = get_net_info2(&netInfo, thisjson);
                                        cJSON * jsonRet = renewJsonArray3(&jsonInfo, &jsonArray, "netInfo", thisjson);

                                    }
                                    else{
                                        //从中调出本端信息，其他的丢弃，此处获得的是终极信息
                                        if(chanNum > 0)
                                        {
                                            //
                                            for(int jj = 0; jj < chanNum; jj++)
                                            {
                                                //
                                                int chanId = chanIdList[jj];
                                                if((chanId + 1) == netInfo.chanId)
                                                {
                                                    //printf("get_frame: netInfo.chanId= %d \n", netInfo.chanId);
                                                    //int ret3 = get_net_info(&netInfo, extend_info);
                                                    cJSON *thisjson = NULL;
                                                    thisjson = get_net_info2(&netInfo, thisjson);
                                                    cJSON * jsonRet = renewJsonArray3(&jsonInfo, &jsonArray, "netInfo", thisjson);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            //printf("get_frame: size=%d \n", size);
                            stop_seq_num = this_seqnum;
                            if(!offset)
                            {
                                memcpy(&outbuf[offset], buf, size);
                                offset += size;
                                ret = size;
                            }
                            j += 1;
                        }
                        //stop_seq_num = this_seqnum;

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
                        //(1 << 32) / 27 / 1000 / 3600 = 13.256 (hours)
                        uint32_t start_time_stamp = obj->start_time_stamp;
                        //uint32_t last_frame_time_stamp = obj->last_frame_time_stamp;
                        int diff_time0 = (int)(last_timestamp - start_time_stamp) / 27;//27000Hz
                        if(start_time_stamp > last_timestamp)
                        {
                            diff_time0 = (int)((last_timestamp + LEFT_SHIFT32 - start_time_stamp) / 27);
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
                            if(delay > (delay_time * 3) && (delay > 300))
                            {
                                printf("warning: audio :get_frame: delay= %d, ret= %d, packet_distance= %d \n", delay, ret, packet_distance);
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
                            if(this_seqnum > 0)
                            {
                                //stop_seq_num = this_seqnum;
                            }
                            else{
                                printf("warning: get_frame: this_seqnum= %d \n", this_seqnum);
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
                    //printf("audio: get_frame: j= %d \n", j);
                    //printf("audio: get_frame: start_seq_num= %d \n", start_seq_num);
                    //printf("audio: get_frame: stop_seq_num= %d \n", stop_seq_num);
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
                        p[0] = 1;//0;
                    }
                    //reset min/max seqnum
                    int withpacket = 0;
                    uint16_t seqnum = stop_seq_num + 1;
                    if(stop_seq_num == MAX_USHORT)
                    {
                        seqnum = 0;
                    }
                    //处理需慎重，否则会错误遗失将要到来的包
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
                            //printf("warning: audio: get_frame: obj->max_packet= %u \n", obj->max_packet);
                        }
                        else if(seqnum >= obj->max_packet)
                        {
                            if(size > 1)
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
                        if(size > 1)
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
                        //缓存中无有效数据
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
                            //理论上，不存在，至少obj->max_packet是有效数据
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
    if(chanIdList)
    {
        free(chanIdList);
    }
    if(jsonInfo)
    {
        char *jsonStr = api_json2str(jsonInfo);//比较耗时
        api_json_free(jsonInfo);
        strcpy(extend_info, jsonStr);
        if(strlen(extend_info) >= MAX_OUTCHAR_SIZE)
        {
            printf("error: get_frame: strlen(extend_info)= %d \n", strlen(extend_info));
        }
        api_json2str_free(jsonStr);
    }
    //printf("get_frame: end: ret= %d \n", ret);
    return ret;
}
static int add_delay(DelayInfo *delay_info, int delay)
{
    int ret = delay;
    if(delay >= 0)
    {
        for(int i = 0; i < 3; i++)
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
            if(ret > delay_info[2].last_max_delay)//瞬间峰值
            {
                ret = delay_info[2].last_max_delay;
            }
            //持续偏大，必须快速更新
            //if(ret < delay_info[1].delay)
            //{
            //    ret = delay_info[1].delay;
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
    AudioRtpObj *obj = (AudioRtpObj *)hnd;
    cJSON *json = obj->json;
    int selfChanId = GetvalueInt(json, "selfChanId");
    //int *chanIdList = GetArrayValueInt(json, "chanId", &chanNum);
    if(selfChanId > 0)
    {
        NetInfo netInfo = {};
        int enable_nack = GetAudioNetInfo((uint8_t*)buf, size, &netInfo, 0);
        if(enable_nack)
        {
            //printf("renew_delay_time: netInfo.decodeId= %d, selfChanId=%d \n", netInfo.decodeId, selfChanId);
            //printf("renew_delay_time: netInfo.rtt= %d \n", netInfo.rtt);
            if(netInfo.decodeId == selfChanId)
            {
                int diff_time = (netInfo.rtt << 3);
                //printf("audio: renew_delay_time: diff_time= %d \n", diff_time);
                ret = add_delay(obj->delay_info, diff_time);
            }
#if 0
            if(!netInfo.info_status)
            {
                int diff_time = netInfo.rt0 - netInfo.st0;
                if(diff_time <= 0)
                {
                    if(diff_time < obj->offset_time)
                    {
                        obj->offset_time = diff_time;
                    }
                }
                if(obj->offset_time <= 0)
                {
                    diff_time -= obj->offset_time;
                }
                ret = add_delay(obj->delay_info, diff_time);

            }
#endif
        }
    }
    return ret;
}
int audio_resort_packet(void *hnd, char *data, char *outbuf, char *complete, char *extend_info, char *outparam[])
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
        printf("audio: resort_packet: obj->Obj_id= %x \n", obj->Obj_id);
        printf("audio: resort_packet: obj->buf_size= %d \n", obj->buf_size);
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
            //if(!size)//size == 0
            if(size <= 1)
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
        if(obj->delay_time <= 0)
        {
            obj->delay_time = delay_time;
        }
        obj->delay_time = renew_delay_time(hnd, data, insize, obj->delay_time);
        memcpy(&obj->recv_buf[I][sizeof(int)], data, insize);
        int *p = (int *)&obj->recv_buf[I][0];
        //printf("audio_resort_packet: insize= %d \n", insize);
        p[0] = insize;
    }
    else{
        //printf("warning: audio_resort_packet: old packet: insize= %d \n", insize);
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
            ret = get_frame(hnd, outbuf, complete, extend_info, &frame_timestamp);
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

        obj->json = (cJSON *)api_str2json(param);
        obj->param = param;
        int selfChanId = GetvalueInt(obj->json, "selfChanId");
        int insize = GetvalueInt(obj->json, "insize");
        if(!insize)
        {
            printf("error: api_resort_packet: insize= %d \n", insize);
            printf("error: api_resort_packet: param= %s \n", param);
        }
        char *extend_info = NULL;
        if(selfChanId > 0)
        {
            extend_info = obj->outparam[1];
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
        //printf("audio_resort_packet: insize= %d \n", insize);
        ret = audio_resort_packet(obj, data, outbuf, complete, extend_info, outparam);

        if(ret > 0)// && !strlen(outparam[0]) && !strlen(outparam[1]))
        {
        }
        api_json_free(obj->json);
        obj->json = NULL;
    }
    return ret;
}

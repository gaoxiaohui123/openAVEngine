/*****************************************************************************
 * hcsvc.h: hcsvc public header
 *****************************************************************************
 * Copyright (C) 2020-2020 hcsc project
 *
 * Authors: Xiaohui Gao <waky_7635@126.com>
  *****************************************************************************/

#ifndef HCSVC_HCSVC_H
#define HCSVC_HCSVC_H


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

#ifdef _WIN32
#   define X264_DLL_IMPORT __declspec(dllimport)
#   define X264_DLL_EXPORT __declspec(dllexport)
#else
#   if defined(__GNUC__) && (__GNUC__ >= 4)
#       define X264_DLL_IMPORT
#       define X264_DLL_EXPORT __attribute__((visibility("default")))
#   else
#       define X264_DLL_IMPORT
#       define X264_DLL_EXPORT
#   endif
#endif


#ifdef X264_API_IMPORTS
#   define X264_API X264_DLL_IMPORT
#else
#   ifdef X264_API_EXPORTS
#       define X264_API X264_DLL_EXPORT
#   else
#       define X264_API
#   endif
#endif

/* x264_t:
 *      opaque handler for encoder */
//typedef struct x264_t x264_t;

#define H264_PLT 127
#define AAC_PLT  126
#define FIX_MTU_SIZE 1400
#define MTU_SIZE 1100

/****************************************************************************
 * Encoder parameters
 ****************************************************************************/
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

typedef struct {
	short rtp_extend_profile;       //profile used
	short rtp_extend_length;        //扩展字段的长度，为4的整数倍；1表示4个字节
	NACK_HEADER nack;
} AUDIO_EXTEND_HEADER;

#endif
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
    long long nack_time;
}RtpInfo;
inline int GetRtpInfo(uint8_t* dataPtr, int insize, RtpInfo *info)
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
            rtp_hdr->payload == H264_PLT   //负载类型号
            );
    if(!isrtp)
    {
        //printf("GetRtpInfo: raw_offset not 2 \n");
        rtp_hdr = (RTP_FIXED_HEADER*)&dataPtr[0];
        isrtp = (rtp_hdr->version == 2 &&  //版本号，此版本固定为2								V
            rtp_hdr->padding	== 0 &&  //														P
            rtp_hdr->csrc_len == 0 && //												X
            rtp_hdr->payload == H264_PLT   //负载类型号
            );
        if(isrtp)
        {
            info->raw_offset = 0;
        }
    }
    if(isrtp)
    {
        if(insize < (sizeof(RTP_FIXED_HEADER) + sizeof(EXTEND_HEADER)))
        {
            return ret;
        }
        int offset = sizeof(RTP_FIXED_HEADER);//1;
        info->seqnum = rtp_hdr->seq_no;
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
                //printf("GetRtpInfo: rtp_extend_length= %d \n", rtp_extend_length);
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
            info->nack_time = rtp_ext->nack.time_info.time_stamp;
            info->rtp_header_size = offset + info->raw_offset;
            info->ref_idc = rtp_ext->ref_idc + 1;
            info->refs = rtp_ext->refs;
            info->ref_idx = rtp_ext->ref_idx;
            info->spatial_num = rtp_ext->res_num;
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
inline int InSertRawOffset(uint8_t* dataPtr, uint8_t* dst, int dataSize)
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
            int raw_offset = 2;
            //info->raw_offset = 2;
            ret = dataSize + 2;
            memcpy(dst, &dataPtr[offset], raw_offset);
            NALU_HEADER* nalu_hdr = (NALU_HEADER*)&dst[0];
            if(nalu_hdr->TYPE == 24)
            {
                nalu_hdr->TYPE = 7;
            }
            memcpy(&dst[raw_offset], &dataPtr[0], dataSize);

        }
    }
    return ret;
}
inline int hcsvc2h264stream(unsigned char *src, int insize, int rtpheadersize, unsigned char *dst)
{
    int ret = 0;
    RtpInfo info = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int rtptype = GetRtpInfo((uint8_t*)&src[rtpheadersize], insize - rtpheadersize, &info);
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
                printf("hcsvc2h264stream: ret= %d \n", ret);
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
inline int distill_spatial_layer(unsigned char *src, int insize, int rtpheadersize, unsigned char *dst, int layerId, int multlayer)
{
    int ret = 0;
    RtpInfo info = {0, 0, 0, 0, 0, 0, 0, 0};
    int rtptype = GetRtpInfo((uint8_t*)&src[rtpheadersize], insize - rtpheadersize, &info);
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
inline int GetRtpHearderSize(uint8_t* dataPtr, int dataSize)
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

/****************************************************************************
 * Encoder functions
 ****************************************************************************/
HCSVC_API
int hcsvc_dll_init(char *filename);
HCSVC_API
int hcsvc_dll_close();

HCSVC_API
void i2_video_encode_open(int id, char *param);
HCSVC_API
int i2_video_encode_one_frame(int id, char *data, char *param, char *outbuf, char *outparam[]);
HCSVC_API
int i2_video_encode_close(int id);
HCSVC_API
void* i2_renew_json_int(void *json, char *key, int ivalue);
HCSVC_API
void* i2_renew_json_str(void *json, char *key, char *cvalue);
HCSVC_API
char* i2_json2str(void *json);
HCSVC_API
void i2_json2str_free(char *jsonstr);
HCSVC_API
int* i2_get_array_by_str(char *text, char tok, int *num);
HCSVC_API
void i2_get_array_free(int *pktSize);
HCSVC_API
long long i2_get_time_stamp_ll(void);
//
#if 1
HCSVC_API
int api_create_codec_handle(char *handle);
HCSVC_API
int api_free_codec_handle(char *handle);
HCSVC_API
int api_video_encode_open(char *handle, char *param);
HCSVC_API
int api_video_encode_one_frame(char *handle, char *data, char *param, char *outbuf, char *outparam[]);
HCSVC_API
int api_video_encode_close(char *handle);
HCSVC_API
int api_video_decode_open(char *handle, char *param);
HCSVC_API
int api_video_decode_one_frame(char *handle, char *data, char *param, char *outbuf, char *outparam[]);
HCSVC_API
int api_video_decode_close(char *handle);
HCSVC_API
void* api_renew_json_float(void *json, char *key, float fvalue);
HCSVC_API
void* api_renew_json_int(void *json, char *key, int ivalue);
HCSVC_API
void* api_renew_json_str(void *json, char *key, char *cvalue);
HCSVC_API
void* api_renew_json_array(void *json, char *key, int *value, int len);
HCSVC_API
void* api_delete_item(void *json, char *key);
HCSVC_API
char* api_json2str(void *json);
HCSVC_API
void api_json2str_free(char *jsonstr);
HCSVC_API
int* api_get_array_by_str(char *text, char tok, int *num);
HCSVC_API
void api_get_array_free(int *pktSize);
HCSVC_API
void api_json_free(void *json);
//HCSVC_API
//long long api_get_time_stamp_ll(void);
//
HCSVC_API
int api_raw2rtp_packet(char *handle, char *data, char *param, char *outbuf, char *outparam[]);
HCSVC_API
int api_rtp_packet2raw(char *handle, char *data, char *param, char *outbuf, char *outparam[]);
HCSVC_API
int api_resort_packet(char *handle, char *data, char *param, char *outbuf, char *outparam[]);
HCSVC_API
long long api_get_time_stamp_ll(void);

HCSVC_API
int api_renew_time_stamp(char *data);

HCSVC_API
int api_get_extern_info(char *data, char *outparam[]);


HCSVC_API
void api_show_device();
HCSVC_API
int api_create_sdl_handle(char *handle);
HCSVC_API
int api_split_screen(char *handle, char * show_buffer, char *param, int width);
HCSVC_API
int api_sdl_status(char *handle);
HCSVC_API
void api_sdl_clear(char *handle);
HCSVC_API
void api_sdl_stop(char *handle);
HCSVC_API
void api_sdl_show_run(char *handle);
HCSVC_API
void api_sdl_close(char *handle);
HCSVC_API
int api_sdl_init(char *handle, char *param);


HCSVC_API
int api_fec_encode(char *handle, char *data, char *param, char *outbuf, char *outparam[]);
HCSVC_API
int api_fec_decode(char *handle, char *data, char *param, char *outbuf, char *outparam[]);

HCSVC_API
int api_check_packet(uint8_t* dataPtr, int insize, unsigned int ssrc, int lossRate);
HCSVC_API
int api_count_loss_rate(uint8_t* dataPtr, int insize);

HCSVC_API
int api_create_capture_handle(char *handle);
HCSVC_API
int api_capture_init(char *handle, char *param);
HCSVC_API
void api_capture_close(char *handle);
HCSVC_API
int api_capture_read_frame(char *handle, char *outbuffer);


HCSVC_API
int ICreateVideoDenoise(char *handle);
HCSVC_API
int IVideoDenoiseClose(char *handle);
HCSVC_API
int IVideoDenoise(char *handle, unsigned char *data[3], int linesize[3], int width, int height);

HCSVC_API
int I2CreateVideoDenoise2(char *handle);
HCSVC_API
int I2VideoDenoiseClose2(char *handle);
HCSVC_API
int I2VideoDenoise2(char *handle, unsigned char *data[3], int linesize[3], int width, int height);

HCSVC_API
int ICreateAudioProcess(char *handle);
HCSVC_API
int IAudioProcessClose(char *handle);
HCSVC_API
int IAudioProcess(char *handle, unsigned char *data, int insize);

HCSVC_API
int I2CreateAudioProcess(char *handle);
HCSVC_API
int I2AudioProcessClose(char *handle);
HCSVC_API
int I2AudioProcess(char *handle, unsigned char *data, int insize);

HCSVC_API
int api_get_cmd(char *cmd, char *buf, int read_size);
HCSVC_API
int api_get_dev_info(char *cmd, char *buf);

HCSVC_API
void api_sdl_push_event(int id);

HCSVC_API
int api_create_simple_osd_handle(char *handle);
HCSVC_API
void api_simple_osd_close(char *handle);
HCSVC_API
int api_simple_osd_init(char *handle, char *param);
HCSVC_API
int api_simple_osd_process(char *handle, char *data, char *param);
//HCSVC_API
//void api_get_info_test(char *outparam[]);

#else
#define api_video_encode_open i2_video_encode_open
#define api_video_encode_one_frame i2_video_encode_one_frame
#define api_video_encode_close i2_video_encode_close
#define api_renew_json_int i2_renew_json_int
#define api_renew_json_str i2_renew_json_str
#define api_json2str i2_json2str
#define api_json2str_free i2_json2str_free
#define api_get_array_by_str i2_get_array_by_str
#define api_get_array_free i2_get_array_free
#define api_get_time_stamp_ll i2_get_time_stamp_ll

#endif

#ifdef __cplusplus
}
#endif

#endif

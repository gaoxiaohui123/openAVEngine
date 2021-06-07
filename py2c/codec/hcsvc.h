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

#if 0
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

#endif
/* x264_t:
 *      opaque handler for encoder */
//typedef struct x264_t x264_t;

#include <stdint.h>
#include <string.h>

//===================for webrtc
//#define RTP_HCSVC

#define HCSVC_OPT_OPENH264_DEC
//
#define HCSVC_OPT_ENC
#define HCSVC_OPT_DEC
#define HCSVC_OPT_RTP
#define HCSVC_OPT_PKT
#define HCSVC_OPT_NET
#define HCSVC_OPT
#define RTP4WEBRTC
//#define TESTCONTROL
//==============================

#define H264_PLT 127
#define AAC_PLT  126
#define FIX_MTU_SIZE 1400
#define MTU_SIZE 1100
#define RAW_OFFSET 4//2
#define EXTEND_PROFILE_ID   0xA55A


//#define SELF_TEST_MODE
//buffer_shift不能过于接近seqnum的最大值,不能超过15位
#define MAX_PKT_BUF_SIZE (1 << 16) //utility_server.c
//#define H264_PLT 196
#define LEFT_SHIFT32 ((long long)1 << 32) //注意类型，防止当成负数
#define HALF_UINT ((long long)1 << 31)
#define QUART_UINT ((long long)1 << 30)
#define HALF_QUART_UINT (HALF_UINT + QUART_UINT) //注意类型，防止当成负数
#define MAX_UINT    (((long long)1 << 32) - 1)
#define LEFT_SHIFT16 ((int)1 << 16)
#define MAX_USHORT (((int)1 << 16) - 1)
#define HALF_USHORT ((int)1 << 15)
#define QUART_USHORT ((int)1 << 14)
#define HALF_QUART_USHORT (HALF_USHORT + QUART_USHORT)
#define	VIDIORASHEADSIZE	4
#define RECV_BUF_NUM  1000
#define MAX_PACKET_SIZE  1500


#define MAX_PKT_NUM (1 << 10) //1024//2048 //1024
#define MAX_FEC_PKT_NUM (1 << 14) //丢包率到达90%,会导致包数增加10倍以上
//#define MAX_FEC_PKT_NUM (1 << 13)

#define INBUF_SIZE 4096
#define AUDIO_INBUF_SIZE 20480
#define AUDIO_REFILL_THRESH 4096

#define MAX_OUTCHAR_SIZE (128 * MAX_FEC_PKT_NUM)
//#define MAX_OUTCHAR_SIZE (128 * 1024)

#define MAX_SPS_SIZE MAX_PACKET_SIZE //128//64
#define TIME_OUTCHAR_SIZE 16
#define MAX_DELAY_TIME 2000
#define MAX_RESORT_FRAME_NUM 3//10//20//10
#define MAX_SKIP_FRAME_NUM 8
#define SPLIT_LEN (2 * 60 * 1000) //2 * 60 * 60 * 1000;//2hours
#define MAX_GOP_SIZE 50 //100000
#define MAX_MB_SIZE ((3840 * 2160) >> 12)//((((1920 << 1) >> 4) * ((1080 << 1) >> 4)) >> 3) //(1920*2/16)x(1080*2/16)/8
//#define MAX_MB_SIZE ((1920 >> 4) * (1080 >> 4))
//#define OPEN_SKIP_MODE 1
#define OPEN_SKIP_FRAME 1
#define OPEN_PACED_DISPLAY 1
#define MAX_QP_NUM  3
#define NETN 4

//#define PRINT_LEVEL     1

#ifdef PRINT_LEVEL
#define MYPRINT printf
#else
#define MYPRINT
#define MYPRINT2 printf
#endif

/****************************************************************************
 * Encoder parameters
 ****************************************************************************/
enum { kIsVideo = 1, kIsAudio};
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

#if 1
typedef struct {
	unsigned int codec_id : 3;//4; //3	// identifies the code/codec being used. In practice, the "FEC encoding ID" that identifies the FEC Scheme should
					 // be used instead (see [RFC5052]). In our example, we are not compliant with the RFCs anyway, so keep it simple.
	unsigned int k : 11;//8; //9 : 512
	unsigned int n : 14;//12
	unsigned int rsv0 : 4;
	unsigned short symbol_size : 10;//symbol_size >> 2
	unsigned short rsv1 : 6;
	unsigned short fec_seq_no : 14;//12
	unsigned short rsv2 : 2;
	//unsigned short resv;
	//8b
}FEC_HEADER;
#else
typedef struct {
	unsigned short codec_id : 3;//4; //3	// identifies the code/codec being used. In practice, the "FEC encoding ID" that identifies the FEC Scheme should
					 // be used instead (see [RFC5052]). In our example, we are not compliant with the RFCs anyway, so keep it simple.
	unsigned short k : 11;//8; //9 : 512
	unsigned short n : 14;//12
	unsigned short symbol_size : 10;//symbol_size >> 2
	unsigned short fec_seq_no : 14;//12
	//unsigned int rsv0 : 4;
	//unsigned short rsv1 : 6;
	//unsigned short rsv2 : 2;
	//unsigned short resv;
	//8b
}FEC_HEADER;
#endif

typedef struct {
    unsigned short st0; //client send packet time
    unsigned short rt0; //server receive packet time
    unsigned short st1; //server send packet time
    unsigned short rt1; //client receive packet time
}RTT_HEADER0;
typedef struct {
    unsigned short st0; //client send packet time
    unsigned short rt0; //server receive packet time
    unsigned short rsv;
    unsigned short decodeId : 7;// maxId = 64
    unsigned short rtt : 9;//((rtt + 4) >> 3)//2^12=4096;maxRtt = 4095;511
}RTT_HEADER1;

typedef union
{
    RTT_HEADER0 rtt0;
    RTT_HEADER1 rtt1;
    //8b
}RTT_HEADER;
//typedef union
typedef struct
{
    //int64_t time_stamp;//8b
    uint32_t time_stamp0;//4b
    uint32_t time_stamp1;//4b
    RTT_HEADER rtt_list;//8b
}TIME_HEADER;//16b
//
//结构体大小必须是所有成员(不包括结构体)大小的整数倍
//注意在 linux 下时，整个结构体的大小应该是：
//char 对齐模数是 1，short 是 2，int 是 4，float 是 4，double(linux 是 4，windows 是 8)
typedef struct {
    unsigned short enable_nack : 1;
    unsigned short time_offset : 13;//15;
    unsigned short res_idx : 2;
    unsigned char loss_rate : 7; //a% * 100
    unsigned char is_lost_packet : 1;
    unsigned char info_status : 1;   //0:self
    //unsigned char time_status : 1;      //0:time_stamp; 1:rtt_list
    unsigned char chanId : 7; //循环将各个接收端的丢包信息回馈给对端;
    //4b
    TIME_HEADER time_info;
}NACK_NORM;//24b

/*
typedef struct
{
    uint32_t time_stamp0;//4b
    uint32_t time_stamp1;//4b
    RTT_HEADER rtt_list;//8b
}TIME_TEST;//16b
typedef struct {
    unsigned short enable_nack : 1;
    unsigned short time_offset : 13;//15;
    unsigned short res_idx : 2;
    unsigned char loss_rate : 7; //a% * 100
    unsigned char is_lost_packet : 1;
    unsigned char info_status : 1;   //0:self
    //unsigned char time_status : 1;      //0:time_stamp; 1:rtt_list
    unsigned char chanId : 7; //循环将各个接收端的丢包信息回馈给对端;
    //4b
    TIME_TEST time_info;
}NACK_TEST;//20b
*/
typedef struct {
    uint32_t send_time;//4b
    uint16_t start_seqnum : 16;//2b//不同于识别帧丢失的start_seqnum，此处为标识首个丢失包
    uint16_t chanId : 7;
    uint16_t loss_type : 2;//0:normal;1:len;2:frame loss
    uint16_t loss0 : 7;//frame loss
    uint32_t loss1;//4b,32bits
    uint32_t loss2;//4b,32bits
    uint32_t loss3;//4b,32bits
    uint32_t loss4;//4b,32bits
    //128
    //20b
}NACK_LOSS;

typedef union
{
    NACK_NORM nack0;
    NACK_LOSS nack1;
}NACK_HEADER;

typedef struct {
	unsigned short rtp_extend_profile;       //profile used
	unsigned short rtp_extend_length;        //扩展字段的长度，为4的整数倍；1表示4个字节
    //4
	unsigned int rtp_pkt_size : 12;	    //rtp包数据大小（包含包头、扩展包）//小于MTU_SIZE
	unsigned int rtp_pkt_num : 14;     //每帧rtp包个数;//<=1024 * 10;bits14//fec_k
	unsigned int is_rtx : 1;
	unsigned int nack_type : 2;         //0:no;1:rtt;2:nack
	unsigned short start_seqnum;        //用于识别帧丢失
	//unsigned short seq_no;			//注意：此包序独立于整个rtp包序，是单一域的包序
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
	unsigned short nal_type : 5; //nal_unit_type
	unsigned short refresh_idr : 1;//有更新置1,下一個IDR恢復到0
	unsigned short raw_offset : 1; //视频裸流偏移//0：0; 1: 2字节
	unsigned short main_spatial_idx : 2; //空域信息：主分辨率序号（0为最大分辨率）
	unsigned short mult_spatial : 1;
	//unsigned short rsv : 2;
    //12
	NACK_HEADER nack;
} EXTEND_HEADER;

typedef struct {
	unsigned short rtp_extend_profile;       //profile used
	unsigned short rtp_extend_length;        //扩展字段的长度，为4的整数倍；1表示4个字节
	NACK_HEADER nack;
	//unsigned short seq_no;
} AUDIO_EXTEND_HEADER;

#define MAX_RTP_EXTEND_SIZE (sizeof(EXTEND_HEADER) + sizeof(NACK_HEADER) + sizeof(FEC_HEADER))
#define MAX_RTP_HEAD_SIZE (sizeof(RTP_FIXED_HEADER) + sizeof(EXTEND_HEADER))

typedef struct {
    int st0; //client send packet time
    int rt0; //server receive packet time
    int st1; //server send packet time
    int rt1; //client receive packet time
    int rtt;
    int loss_rate;
    int info_status;
    int res_idx;
    int chanId;
    int decodeId;
    //int time_offset;
    unsigned int ssrc;
    int64_t time_stamp;
    int is_rtx;
    NACK_HEADER nack;
}NetInfo;

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
    int is_header;
    int rtx_seqnum;//重传包序
    int rtp_pkt_num;//每帧rtp包个数
    int max_pkt_num;
    int is_rtx;
    int start_seqnum;
}RtpInfo;


HCSVC_API
int GetRtpInfo(uint8_t* dataPtr, int insize, RtpInfo *info, int type);
HCSVC_API
int InSertRawOffset(uint8_t* dataPtr, uint8_t* dst, int dataSize);
HCSVC_API
int hcsvc2h264stream(unsigned char *src, int insize, int rtpheadersize, unsigned char *dst);
HCSVC_API
int distill_spatial_layer(unsigned char *src, int insize, int rtpheadersize, unsigned char *dst, int layerId, int multlayer);
HCSVC_API
int GetRtpHeaderSize(uint8_t* dataPtr, int dataSize);
HCSVC_API
int GetNetInfo(uint8_t* dataPtr, int insize, NetInfo *info, int times);
HCSVC_API
int GetAudioNetInfo(uint8_t* dataPtr, int insize, NetInfo *info, int times);

HCSVC_API
int api_isrtp(char *dataPtr, int size, int raw_offset, int type);
HCSVC_API
int api_get_ref_idc(char *dataPtr, int insize, int raw_offset);
HCSVC_API
int api_memcpy(char *data, int size, char *outparam[]);
HCSVC_API
int api_get_pkt_delay2(char *dataPtr, int insize, int raw_offset, int time_offset, int type);

HCSVC_API
void api_ffmpeg_register();

HCSVC_API
int api_vaapi_encode_test(char *infile, char *outfile, int width, int height, char *enc_name, int loopn);
HCSVC_API
int api_vaapi_decode_test(char *infile, char *outfile, char *codec_name, int loopn);
HCSVC_API
int api_sw_encode_test(char *infile, char *outfile, int width, int height, int codec_id, int loopn);
HCSVC_API
int api_read_i420(char *infile, int frame_size, uint8_t **outbuf, int *file_size, int is_nv12);
/****************************************************************************
 * Encoder functions
 ****************************************************************************/
#if 1
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
HCSVC_API
int api_create_codec_handle(char *handle);
HCSVC_API
int api_free_codec_handle(char *handle);
HCSVC_API
int api_video_encode_open(char *handle, char *param);
HCSVC_API
int api_video_set_skip_mb(char *handle, uint8_t *data, int size);
HCSVC_API
int api_video_encode_one_frame(char *handle, char *data, char *param, char *outbuf, char *outparam[]);
HCSVC_API
int api_control_max_bitrate(char *handle, int frame_idx, uint8_t *frame_data, float threshold);
HCSVC_API
int api_control_max_bitrate2(char *handle, int frame_idx, uint8_t *frame_data, float threshold, int difftime, int basetime);
HCSVC_API
int api_br_push_data(char *handle, int size, int new_frame, int64_t now_time);
HCSVC_API
int api_video_encode_close(char *handle);
HCSVC_API
int api_video_decode_open(char *handle, char *param);
HCSVC_API
int api_video_decode_one_frame(char *handle, char *data, char *param, char *outbuf, char *outparam[]);
HCSVC_API
int api_video_decode_close(char *handle);
HCSVC_API
int api_create_audio_codec_handle(char *handle);
HCSVC_API
int api_audio_codec_close(char *handle);
HCSVC_API
int api_audio_codec_init(char *handle, char *param);
HCSVC_API
int api_audio_codec_one_frame(char *handle, char *data, char *param, char *outbuf, char *outparam[]);
HCSVC_API
int api_create_scale_handle(char *handle);
HCSVC_API
int api_scale_init(char *handle, char *param);
HCSVC_API
int api_ff_scale(int id, char *data, char *param, char *outbuf, char *outparam[]);
HCSVC_API
int api_create_player_handle(char *handle);
HCSVC_API
void api_player_close(char *handle);
HCSVC_API
int api_player_init(char *handle, char *param);
HCSVC_API
int audio_play_frame(char *handle, char *param, char *indata, int insize);
HCSVC_API
int audio_play_frame_mix(char *handle, char *param, char *indata[], int insize);
HCSVC_API
int api_create_audio_capture_handle(char *handle);
HCSVC_API
void api_audio_capture_close(char *handle);
HCSVC_API
int api_audio_capture_init(char *handle, char *param);
HCSVC_API
int api_audio_capture_read_frame(char *handle);
HCSVC_API
int api_audio_capture_read_frame2(char *handle, char *outbuf);
HCSVC_API
int api_capture_read_frame3(char *handle, char **outbuf);

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
int* api_array_alloc(int num);
HCSVC_API
void api_json_free(void *json);
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
int api_get_time(char *outparam[]);
HCSVC_API
int api_get_time2(char *handle, char *outparam[]);

HCSVC_API
void api_get_char_time(char *name);

HCSVC_API
void api_set_time_offset(int time_offset);

HCSVC_API
int api_renew_time_stamp(char *data);

HCSVC_API
int api_get_extern_info(char *data, int insize, char *outparam[]);


HCSVC_API
void api_show_device();
HCSVC_API
int api_create_sdl_handle(char *handle);
HCSVC_API
int api_split_screen(char *handle, char * show_buffer, char *param, int width, int height);
HCSVC_API
int api_render_data(char *handle, char *data, void *rect, int show_flag, int width, int height);
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
int api_av_sdl_init();

HCSVC_API
int api_fec_encode(char *handle, char *data, char *param, char *outbuf, char *outparam[]);
HCSVC_API
int api_fec_decode(char *handle, char *data, char *param, char *outbuf, char *outparam[]);

//HCSVC_API
//int api_check_packet(uint8_t* dataPtr, int insize, unsigned int ssrc, int lossRate);
HCSVC_API
int api_count_loss_rate(uint8_t* dataPtr, int insize, int freq);

HCSVC_API
int api_create_capture_handle(char *handle);
HCSVC_API
int api_capture_init(char *handle, char *param);
HCSVC_API
void api_capture_close(char *handle);
HCSVC_API
int api_capture_read_frame(char *handle, char *outbuffer);
HCSVC_API
int api_capture_read_frame2(char *handle, char *outbuf);

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
HCSVC_API
int api_get_cpu_info(char *outparam[]);
HCSVC_API
int api_get_cpu_info2(int *icpurate, int *memrate, int *devmemrate);
HCSVC_API
int api_clear_yuv420p(char *data, int w, int h);
HCSVC_API
int api_reset_seqnum(char* dataPtr, int insize, int seqnum);
HCSVC_API
void api_test_write_str(int num);

HCSVC_API
void* api_add_array2json(void **jsonInfo, void **jsonArray, void *thisjson, char *key);
HCSVC_API
void* api_add_netinfos2json(void **jsonInfo, void **jsonArray, NetInfo *netInfo);
HCSVC_API
int* api_get_array_int(char *parmstr, char *key, int *arraySize);
HCSVC_API
void *api_str2json(char *parmstr);

HCSVC_API
int api_renew_delay_time(void *handle, int selfChanId, uint8_t *buf, int size, int delay_time);

HCSVC_API
int api_count_loss_rate2(void *handle, uint8_t* dataPtr, int insize, int freq);
HCSVC_API
int api_add_loss_rate(void *handle, int loss_rate);
HCSVC_API
int api_get_loss_rate(void *handle, int idx);
HCSVC_API
int api_get_rtpheader2(char* dataPtr, int *rtpSize, int size, int raw_offset);

HCSVC_API
int api_count_loss_rate3(char *handle, uint8_t* dataPtr, int insize, int freq);

HCSVC_API
int api_getCpuRate(void **pcpu_stat, int cpurate, int threshold);
//
HCSVC_API
int api_create_avstream_handle(char *handle);
HCSVC_API
int api_avstream_status(char *handle, int status);
HCSVC_API
int api_avstream_close(char *handle);
HCSVC_API
int api_avstream_init(char *handle, char *param);
HCSVC_API
void api_avstream_set_video_handle(char *handle, char *video_codec_handle);
HCSVC_API
void api_avstream_set_audio_handle(char *handle, char *audio_codec_handle);
HCSVC_API
int api_audio_codec_one_frame_stream(char *handle, char *data, char *param, char *outbuf, char *outparam[]);
HCSVC_API
int api_video_encode_one_frame_stream(char *handle, char *data, char *param, char *outbuf, char *outparam[]);
HCSVC_API
int api_avstream_write_tail(char *handle);
HCSVC_API
int api_avstream_loopread(char *handle);
HCSVC_API
void * api_avstream_pop_data(char *handle, int avtype);
//
HCSVC_API
unsigned int api_create_id(unsigned int range);
//
HCSVC_API
void api_register_test();

HCSVC_API
int api_pool_start(char *handle, char *params);
HCSVC_API
int api_start_broadcast(char *handle, char *params);
HCSVC_API
int api_pool_stop(char *handle);
HCSVC_API
int api_broadcast_cmd(char *handle, char *params);

//HCSVC_API
//int api_get_sessionId(char *handle, char *params);
//HCSVC_API
//int api_get_chanId(char *handle, char *params);
//HCSVC_API
//int api_get_rtt(char *handle, char *params);
//HCSVC_API
//int api_exit(char *handle, char *params);
HCSVC_API
int api_start_video(char *handle, char *params);
HCSVC_API
int api_start_audio(char *handle, char *params);
HCSVC_API
int api_start_capture(char *handle, char *params0);
HCSVC_API
int api_start_render(char *handle, char *params0);
HCSVC_API
int api_set4codec_video(char *handle, char *handle2, char *params);
HCSVC_API
int api_setstream2video(char *handle, char *handle2, char *params);
HCSVC_API
int api_start_acapture(char *handle, char *params0);
HCSVC_API
int api_start_player(char *handle, char *params0);
HCSVC_API
int api_set4codec_audio(char *handle, char *handle2, char *params);
HCSVC_API
int api_setstream2audio(char *handle, char *handle2, char *params);
HCSVC_API
int api_setreadstream2video(char *handle, char *handle2, char *params);
HCSVC_API
int api_setreadstream2audio(char *handle, char *handle2, char *params);
HCSVC_API
int api_stop_task(char *handle, int taskId);
HCSVC_API
int api_poll_rect(char *handle, int taskId);
HCSVC_API
int api_reset_rect(char *handle, int taskId, int modeId, int w, int h);
HCSVC_API
int api_start_mcu(char *handle, char *params0);
HCSVC_API
int api_set4mcu(char *handle, char *handle2, char *params);

HCSVC_API
int api_taskpool_init(char *handle, char *params);
HCSVC_API
int api_taskpool_addtask(char *handle, char *params, void *thread_fun, int taskId);
HCSVC_API
int api_taskpool_addtask(char *handle, char *params, void *thread_fun, int taskId);
HCSVC_API
int pool_start_server(char *handle, int taskId, int port, char *host);
HCSVC_API
int api_stop_server_task(char *handle, int taskId);
HCSVC_API
int pool_stop_server(char *handle);

HCSVC_API
void api_mem_lead_cjson(int64_t start, int64_t loopn);

#else

/*
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
*/

#endif

#ifdef __cplusplus
}
#endif

#endif

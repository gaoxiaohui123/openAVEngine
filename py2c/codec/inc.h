/*
 * Copyright (c) 2020
 * Created by gxh_20200602
 */
#ifndef HCSVC_INC_H
#define HCSVC_INC_H

#ifdef __cplusplus
#define EXTERNC extern "C"
EXTERNC {
#else
#define EXTERNC
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#if 0
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#else
#include "libavutil/opt.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"
#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"
#include "libavutil/samplefmt.h"

#include "libavutil/imgutils.h"
#include "libavutil/parseutils.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/log.h"
#include "libavutil/audio_fifo.h"
#include "wels/codec_api.h"

//#include "sdl/SDL.h"
//#include "sdl/SDL_thread.h"
#include <SDL2/SDL.h>
#include <alsa/asoundlib.h>
#ifdef linux
#include <pthread.h>
#endif

#endif

#include "cJSON.h"
#include "simple_client_server.h"
#include "../webrtc/hcsvc.h"

#ifndef false
#define false   (0)
#endif

#ifndef true
#define true    (!false)
#endif


//#define H264_PLT 196
#define LEFT_SHIFT32 ((long long)1 << 32)
//#define MAX_UINT    (((long long)1 << 32) - 1)
#define LEFT_SHIFT16 ((int)1 << 16)
#define MAX_USHORT (((int)1 << 16) - 1)
#define HALF_USHORT (1 << 15)
#define QUART_USHORT (1 << 14)
#define HALF_QUART_USHORT (HALF_USHORT + QUART_USHORT)
#define	VIDIORASHEADSIZE	4
#define RECV_BUF_NUM  1000
#define MAX_PACKET_SIZE  1500
//#define MAX_RTP_NUM         128

#define MAX_PKT_NUM 1024

#define INBUF_SIZE 4096
#define AUDIO_INBUF_SIZE 20480
#define AUDIO_REFILL_THRESH 4096

#define MAX_OBJ_NUM  20//100
#define MAX_OUTCHAR_SIZE (5 * MAX_PKT_NUM)
#define MAX_SPS_SIZE MAX_PACKET_SIZE //128//64
#define TIME_OUTCHAR_SIZE 16

#ifdef _WIN32
#ifdef  GVEngine_EXPORTS
#define HCSVC_API __declspec(dllexport)
#else
#define HCSVC_API __declspec(dllimport)
#endif
#else
#define HCSVC_API __attribute__ ((__visibility__("default")))
#endif

typedef struct
{
    int Obj_id;
    cJSON *json;
    char *param;
    uint8_t *src_data[4];
    uint8_t *dst_data[4];
    enum AVPixelFormat src_pix_fmt;
    enum AVPixelFormat dst_pix_fmt;
    struct SwsContext *sws_ctx;
    int src_w;// = 320,
    int src_h;// = 240,
    int dst_w;
    int dst_h;
    const char *dst_size;// = NULL;
    int src_linesize[4];
    int dst_linesize[4];
    int scale_mode;
    char *outbuf;
}ScaleObj;


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

typedef struct
{
    int Obj_id;
    FILE *logfp;
    uint16_t seq_num;
    uint16_t refresh_idr;
    uint16_t enable_fec;
    uint32_t ssrc;
    uint32_t time_stamp;
    uint32_t last_frame_time_stamp;
    uint32_t start_time_stamp;
    int64_t start_frame_time;
    int old_seqnum;
    cJSON *json;
    char *param;
    short *rtpSize;
    SVCNalu svc_nalu;
    uint8_t **recv_buf;
    //uint8_t **send_buf;
    int min_packet;
    int max_packet;
    int cut_flag;
    int buf_size;
    char outparam[4][MAX_OUTCHAR_SIZE];
    char sps[MAX_SPS_SIZE];
    char pps[MAX_SPS_SIZE];

}RtpObj;
typedef struct
{
    int Obj_id;
    FILE *logfp;
    uint16_t seq_num;
    uint32_t ssrc;
    uint32_t time_stamp;
    uint32_t last_frame_time_stamp;
    uint32_t start_time_stamp;
    int64_t start_frame_time;
    int old_seqnum;
    cJSON *json;
    char *param;
    uint8_t **recv_buf;
    //uint8_t **send_buf;
    int min_packet;
    int max_packet;
    int buf_size;
    char outparam[4][MAX_OUTCHAR_SIZE];
}AudioRtpObj;
typedef struct
{
    int Obj_id;
    cJSON *json;
    char *param;
    AudioRtpObj *rtpObj;
    AudioRtpObj *resortObj;
    AVCodec *codec;
    AVCodecContext *c;
    struct SwrContext   *audio_convert_ctx;
    AVFrame *frame;
    AVPacket avpkt;
    enum AVCodecID codec_id;
    int sample_fmt;
    int bit_rate;
    int codec_type;
    int out_nb_samples;
    int out_channel_layout;
    int out_channels;
    int out_sample_rate;
    int out_size;
    int print;
    FILE *fp;
}AudioCodecObj;

typedef struct{
    int Obj_id;
    cJSON *json;
    char outparam[4][MAX_OUTCHAR_SIZE];
    short *inSize;
    uint16_t seq_num;
    int rtp_head_size;
    of_codec_id_t	codec_id;				/* identifier of the codec to use */
	of_session_t	*ses;// 		= NULL;			/* openfec codec instance identifier */
	of_parameters_t	*params;//		= NULL;			/* structure used to initialize the openfec session */
	void**		enc_symbols_tab;//	= NULL;			/* table containing pointers to the encoding (i.e. source + repair) symbols buffers */
	void**		src_symbols_tab;//	= NULL;
	void**		recvd_symbols_tab;//= NULL;
	UINT32      symbol_size;
	UINT32		symb_sz_32;//	= SYMBOL_SIZE / 4;	/* symbol size in units of 32 bit words */
	UINT32		k;// = DEFAULT_K;					/* number of source symbols in the block */
	UINT32		n;					/* number of encoding symbols (i.e. source + repair) in the block */
	float       loss_rate;
	float       code_rate;
	UINT32*		rand_order;//	= NULL;			/* table used to determine a random transmission order. This randomization process is essential for LDPC-Staircase optimal performance */
    char		*pkt_with_fpi;//	= NULL;			/* buffer containing a fixed size packet plus a header consisting only of the FPI */
	fec_oti_t	fec_oti;				/* FEC Object Transmission Information as sent to the client */
	INT32		lost_after_index;// = -1;
}FecObj;
typedef struct
{
    int qp;
    int base_qp;
    int bytes_last;
    int bytes_target_last;
    int bytes_sum;
    int bytes_target_sum;
}BitRateControl;
typedef struct
{
    int Obj_id;
    RtpObj *resortObj;
    RtpObj *rtpObj;
    FecObj *fecObj;
    FILE *logfp;
	int codec_id;
	int frame_idx;
	AVCodecContext *c;
	AVCodecParserContext *parser;
	AVFrame *frame;
	AVPacket pkt;
	//char buf[512 * 1024];
	cJSON *json;
	void *param;
	uint8_t *inbuf;
	BitRateControl brctrl;
	char outparam[4][MAX_OUTCHAR_SIZE];
	//
	char *osd_handle;
	int handle_size;
	int osd_enable;
	int orgX;
	int orgY;
	int scale;
	int color;
	int width;
	int height;
	int sum_bits;
	int frame_rate;
	int bits_rate;
	int64_t frame_num;
	int64_t start_time;
    //
	//AVPacket pkt;
	FILE *f;
	FILE *f2;
}CodecObj;

typedef struct
{
    int Obj_id;
    cJSON *json;
    char *param;
    AVInputFormat *inputFmt;
    AVFormatContext	*pFormatCtx;
	int				videoindex;
	AVCodecContext	*pCodecCtx;
	AVCodec			*pCodec;
	AVFrame	*pFrame,*pFrameYUV;
	unsigned char *out_buffer;
	char *img_hnd;
	//AVPacket *packet;
	struct SwsContext *img_convert_ctx;
	char *scale_handle;
	pthread_mutex_t mutex;
	int max_buf_num;
	char **cap_buf;
	char head_size;
	char *tmp_buf;
    int64_t last_time_stamp;
	int in_idx;
	int out_idx;
	int framerate;
	int cap_width;
	int cap_height;
	int64_t cap_start_time_stamp;
    int cap_framerate;
    int cap_read_error_cnt;
	char *input_format;
	int width;
	int height;
	int frame_size;
	int scale_width;
	int scale_height;
	int scale_buf_size;
	int pixformat;
	int scale_type;
	int ratio;
	int denoise;
	//
	char *osd_handle;
	int handle_size;
	int osd_enable;
	int orgX;
	int orgY;
	int scale;
	int color;
	int64_t frame_num;
	int64_t start_time;
    //
	int print;
	char* input_name;//= "video4linux2";
    char* device_name;// = "/dev/video0";
}CaptureObj;

#ifdef __cplusplus
}
#endif

#endif

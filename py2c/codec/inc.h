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
//#include<fcntl.h>

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
#include "libavformat/avio.h"
#include "libavformat/internal.h"
#include "libavutil/buffer.h"
#include "libavutil/error.h"
#include "libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"
#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"
#include "libavutil/samplefmt.h"
//#include "libavutil/time_internal.h"


#include "libavutil/pixdesc.h"
#include "libavutil/hwcontext.h"
#include "libavutil/hwcontext_qsv.h"
#include "libavutil/mem.h"
#include "libavutil/time.h"

#include "libavutil/imgutils.h"
#include "libavutil/parseutils.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/log.h"
#include "libavutil/audio_fifo.h"
//#include "wels/codec_api.h"


//#include "sdl/SDL.h"
//#include "sdl/SDL_thread.h"
#include <SDL2/SDL.h>
#include <pthread.h>

#ifdef _WIN32
#include <windows.h>
//#include <vector>
//#include <string>
//#include <memory>

#else
#include <alsa/asoundlib.h>
#endif

#endif

#include "cJSON.h"
#include "simple_client_server.h"
//#include "../webrtc/hcsvc.h"
//#include "hcsvc.h"
#include "udpbase.h"
#include "utility_server.h"

#ifndef false
#define false   (0)
#endif

#ifndef true
#define true    (!false)
#endif

#define MAX_PATH 256
#define MAX_AUDIO_SIZE		(8192 * 3)



#if 0
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
#endif


enum { kIsStream, kIsFile};
typedef struct StructPointerTest
{
    char name[32];
    char *complete[MAX_RESORT_FRAME_NUM];
    int frame_num;
    int loss_rate;
	int64_t frame_timestamp[MAX_RESORT_FRAME_NUM];
	int frame_size[MAX_RESORT_FRAME_NUM];
	int pkt_num[MAX_RESORT_FRAME_NUM];
	short rtp_size[MAX_RESORT_FRAME_NUM * MAX_FEC_PKT_NUM];
}StructPointerTest, *StructPointer;

typedef struct SessInfoPointerTest
{
    char note[256];
    int sessionId;
    int status;
	int modeId;
	int avtype;
}SessInfoPointerTest, *SessInfoPointer;

typedef struct
{
	struct SwrContext *swr_ctx;
	AVFrame srcFrame;
	AVFrame dstFrame;
}FFResample;
typedef struct
{
	struct SwsContext *sws_ctx;
	AVFrame srcFrame;
	AVFrame dstFrame;
}FFScale;

struct AVReadList{
    int num;
    int idx;
    int id;
    int size;
    uint8_t *data;
    int64_t frame_timestamp;
    struct AVReadList *tail;
    struct AVReadList *next;
};
typedef struct AVReadList AVReadNode;

typedef struct
{
	AVFormatContext *fmtctx;
	AVIOContext *avio;
	AVInputFormat *ifmt;
	AVOutputFormat *ofmt;
	AVDictionary *opt;
	//
	AVCodecContext *codecCtx;
	AVCodec *codec;
	AVStream *ast;
	AVStream *vst;
	FFScale *scale;
	pthread_mutex_t mutex;
	//
	//AVCodecContext *vpCodecCtx;
	//AVCodecContext *apCodecCtx;
	//AVCodec *video_codec;
	//AVCodec *audio_codec;
	//
	int WriteHead;
	int64_t start_time;
	int64_t last_time;
	int64_t time0;
	int split_len;//ms
	int WriteIsOpen;
	int RecordStatus;//0:Stop,1:Paus,2:Start;//also can stop live stream
	int AVWriteFlag;//1:audio,2:video
	int AVReadFlag;//1:audio,2:video
	int video_index;
	int audio_index;
#ifdef _WIN32
	LARGE_INTEGER ptime;
	LARGE_INTEGER pfreq;
#endif
	//int64_t timeout;
}AVMuxer;
typedef struct
{
    AVFormatContext *fmtctx;
    AVIOContext *avio;
	AVInputFormat *ifmt;
	AVOutputFormat *ofmt;
	AVDictionary *opt;
	AVCodecContext *vpCodecCtx;
	AVCodecContext *apCodecCtx;
	AVCodec *video_codec;
	AVCodec *audio_codec;
    //AVStream *st;
	//
	FFResample *resample;
	struct SwsContext *img_convert_ctx;
	AVReadNode *videohead;
	AVReadNode *audiohead;
	//
	int status;
	AVPacket pkt;
	//AVPacket pkt2;
	AVFrame *frame;
	AVFrame *tmp_frame;
	AVPicture pic;
	AVPicture alloc_pic;
	AVBitStreamFilterContext* bsfc;

	pthread_mutex_t mutex;
	char buf[MAX_AUDIO_SIZE];
	int64_t timeout;
	int64_t time0;
	int video_index;
	int audio_index;
	int pos;
	int width;
	int height;
}AVDeMuxer;
typedef struct
{
    int Obj_id;
    cJSON *json;
    int is_write;//else is_read
    int is_file;//else is_stream
    int is_audio;//else is_video
    //char *filename;
    char infile[256];
    char outfile[256];
    int width;
    int height;
    char *picData;
    AVDeMuxer *demux;
	AVMuxer *mux;
	char *video_codec_handle;//codec
	char *audio_codec_handle;
	//ScaleObj *scale;
	//pthread_mutex_t mutex;
	char fileType[32];
}AVStreamObj;
typedef struct {
    int delay;
    int max_delay;
    int last_max_delay;
    int sum_delay;
    int cnt_delay;
    int cnt_max;
    //
    int64_t now_time;
    int time_len;
}DelayInfo;


typedef struct {
    int info2;
    int info3;
    int nal_mem_num;
    int nal_num;
    int size;
    uint32_t timestamp;
    uint16_t refresh_idr;
    uint16_t enable_fec;
    uint32_t ssrc;
    char *buf;
    FILE *fp;
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
    int64_t time0;
    int old_seqnum;
    cJSON *json;
    char *param;
    short *rtpSize;
    SVCNalu svc_nalu;
    uint8_t **recv_buf;
    int64_t net_time_stamp;
    //uint8_t **send_buf;
    int min_packet2;//用于网络信息获取
    int min_packet;
    int max_packet;
    int cut_flag;
    int buf_size;
    int loglevel;
    int frame_idx;
    StructPointer pRet;
    char outparam[4][MAX_OUTCHAR_SIZE];
    char sps[MAX_SPS_SIZE];
    char pps[MAX_SPS_SIZE];
    DelayInfo delay_info[NETN];
    LossRateInfo loss_rate_info[NETN];
    int delay_time;
    int offset_time;
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
    int64_t net_time_stamp;
    //uint8_t **send_buf;
    int64_t netIdx;
    int min_packet2;//用于网络信息获取
    int max_packet2;//用于网络信息获取
    int min_packet;
    int max_packet;
    int buf_size;
    char outparam[4][MAX_OUTCHAR_SIZE];
    DelayInfo delay_info[3];
    int delay_time;
    int offset_time;
}AudioRtpObj;
typedef struct
{
    int Obj_id;
    cJSON *json;
    char *param;
    AudioRtpObj *rtpObj;
    AudioRtpObj *resortObj;
    void *pktManObj;
    AVCodec *codec;
    AVCodecContext *c;
    AVCodecParserContext *parser;
    struct SwrContext   *audio_convert_ctx;
    AVFrame *frame;
    AVPacket pkt;
    enum AVCodecID codec_id;
    AVStreamObj *stream;
    //char stream_handle[8];
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

struct BitRateList{
    int num;
    int idx;
    int id;
    int size;
    int target_bitrate;
    int cur_bitrate;
    int target_framerate;
    int cur_framerate;
    int64_t frame_timestamp;
    int64_t frame_idx;
    struct BitRateList *tail;
    struct BitRateList *next;
};
typedef struct BitRateList BitRateNode;

struct HasNetList{
    int num;
    int idx;
    int chanId;
    int status;
    int is_rtx;
    struct HasNetList *tail;
    struct HasNetList *next;
};
typedef struct HasNetList HasNetNode;

typedef struct{
    char sadHnd[8];
    char *last_frame[MAX_SKIP_FRAME_NUM];
    char *static_frame;
    int skip_frame_idx;
    int bmb_size;
    uint8_t skip_mb[MAX_MB_SIZE];//(1920*2/16)x(1080*2/16)/8
    //uint8_t skip_mb_uv[MAX_MB_SIZE];
    BitRateNode *brhead;
    struct SwsContext *img_convert_ctx;
}PreProcObj;

typedef struct
{
    int Obj_id;
    RtpObj *resortObj;
    RtpObj *rtpObj;
    FecObj *fecEncObj;
    FecObj *fecDecObj;
    PreProcObj *ppObj;
    void *pktManObj;
    FILE *logfp;
	int codec_id;
	int frame_idx;
	struct SwsContext *img_convert_ctx;
	AVBufferRef *hw_device_ctx;
	AVCodecContext *c;
	AVCodecParserContext *parser;
	AVCodec *codec;
	AVFrame *frame;
	AVFrame *hw_frame;
	AVPacket pkt;
	AVStreamObj *stream;
	unsigned char lastQp[MAX_QP_NUM];
	int qp_idx;
	//char stream_handle[8];
	//char buf[512 * 1024];
	cJSON *json;
	void *param;
	uint8_t *inbuf;
	BitRateControl brctrl;
	int brc_pause;
	char outparam[4][MAX_OUTCHAR_SIZE];
	//
	char *osd_handle;
	char *osd_buf;
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
	int target_framerate;
	int new_framerate;
	int adjust_framerate;
	int skip_freq;
	int skip_num;
	int bits_rate;
	int64_t frame_num;
	int64_t start_time;
	int64_t last_frame_num;
	int interval;
	int is_hw;
	int last_skip_frame;
	int codec_mode;//0: x264; 1: x265; 2: hw264; 3: hw265; 4: adapt; 5: others
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
	//char *scale_handle;
	pthread_mutex_t mutex;
	int max_buf_num;
	char **cap_buf;
	char head_size;
	char *tmp_buf;
	char *outbuf;
    int64_t last_time_stamp;
	int in_idx;
	int out_idx;
	int framerate;
	int cap_width;
	int cap_height;
	int64_t cap_start_time_stamp;
    int cap_framerate;
    int cap_read_error_cnt;
	char input_format[32];
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
	char input_name[64];//= "video4linux2";
    char device_name[64];// = "/dev/video0";
}CaptureObj;
//
struct FrameList{
    int num;
    int idx;
    int id;
    uint8_t *data;
    int size;
    int width;
    int height;
    int64_t frame_timestamp;
    int64_t now_time;
    struct FrameList *tail;
    struct FrameList *next;
};
typedef struct FrameList FrameNode;

struct FrameBufferList{
    int num;
    int idx;
    int id;
    int64_t init_timestamp;//  # 防止漂移
    int64_t init_start_time;// = 0  # 防止漂移
    int64_t base_timestamp;// = 0
    int64_t base_start_time;// = 0
    int64_t time_offset;// = 0
    int64_t last_timestamp;// = 0
    int search_count;// = 0
    int64_t audio_timestamp;// = 0
    int64_t audio_start_time;// = 0
    int64_t audio_frequence;// = 0

    FrameNode *frameListHead;

    struct FrameBufferList *tail;
    struct FrameBufferList *next;
};
typedef struct FrameBufferList FrameBufferNode;



typedef struct
{
    int32_t selfChanId;// = -1;
    int32_t selfLossRate;// = 0;
    int32_t selfMaxLossRate;// = 0;
    int32_t initBitRate[4];// = {0, 0, 0, 0};
    int32_t bitRate;// = 0;
    int32_t avtype;
    void *codec;//CallCodecVideo
}Encoder;

struct EncoderList{
    int num;
    int audio_num;
    int video_num;
    int idx;
    Encoder *encoder;
    struct EncoderList *tail;
    struct EncoderList *next;
};
typedef struct EncoderList EncoderNode;

struct NetInfoList{
    int num;
    int idx;
    NetInfo netinfo;
    struct NetInfoList *tail;
    struct NetInfoList *next;
};
typedef struct NetInfoList NetInfoNode;

struct RTTList{
    int num;
    int idx;
    cJSON *json;
    struct RTTList *tail;
    struct RTTList *next;
};
typedef struct RTTList RTTNode, RTXNode;

struct RectList{
    int num;
    int idx;
    int id;
    SDL_Rect rect;
    struct RectList *tail;
    struct RectList *next;
};
typedef struct RectList RectNode;

typedef struct{
    char handle[8];
    char *params;
    cJSON *json;
    pthread_mutex_t status_lock;
    int recv_status;
    int send_status;
    int64_t start_time;
    int offset;
    int frame_size;
    char *outbuf;
    char *data_buf;
    char *readbuf;
}CallCapture, CallAudioCapture;

typedef struct{
    int chanId;
    char *data;
    int width;
    int height;
}ChanInfo;
typedef struct{
    int layerId;
    //int chanId;
    //char *data;
    //int width;
    //int height;
    ChanInfo chanInfo;
    SDL_Rect rect;
}MultRect;
typedef struct{
    int64_t last_frame_time_stamp;
    int modeId;
    int ways;
    int maxLayerId;
    MultRect *pRect;
}MultLayer;
typedef struct{
    char handle[8];
    char *params;
    cJSON *json;
    void *sock;
    pthread_mutex_t lock;
    pthread_mutex_t status_lock;
    int recv_status;
    int send_status;
    int frame_size;
    MultLayer *pMultLayer;
    RectNode *rectHead;
    FrameNode *frameListHead;
    FrameBufferNode *frameBufferHead;
}CallRender, CallPlayer;
typedef struct{
    int actor;
    int sessionId;
    int chanId;
    int idx;
    int modeId;
    int avtype;
    int selfmode;
    int testmode;
    int nettime;
    int width;
    int height;
    int screen_width;
    int screen_height;
    int status;
    //char streamName[256];
    //
    int bitrate;
    int mtu_size;
    int fecenable;
    int fec_level;
    int buffer_shift;
    int denoise;
    int osd_enable;
    int framerate;
    int refs;
    float bwthreshold;
    float lossrate;
    char scodec[32];
    char yuvfilename[256];
    int enable_netack;
}SessionInfoObj;

typedef struct{
    NetInfoNode *otherNetInfoHead;
    EncoderNode *encoderHead;
    pthread_mutex_t lock;
}NetInfoObj;

typedef struct{
    int lossless;
    int gop_size;
    int refs;
    int max_refs;
    int enable_fec;
    int width;
    int height;
    int screen_width;
    int screen_height;
    int frame_rate;
    int bit_rate;
    int mtu_size;
    int qmin;
    int qmax;
    int max_b_frames;
    float loss_rate;
    float code_rate;
    int osd;
    int adapt_cpu;
    int main_spatial_idx;
    int fec_level;
    int codec_mode;
    float bwthreshold;
    int enable_netack;
}CodecInfoObj;

typedef struct{
    void *sock;
    SessionInfoObj *sessionInfo;//由外传入
    NetInfoObj *netInfoObj;//public//由外传入
    void *x264_hnd;
    char handle[8];
    //char sadHnd[8];
    //char *last_frame[MAX_SKIP_FRAME_NUM];
    //char *static_frame;
    //int skip_frame_idx;
    //int bmb_size;
    //uint8_t skip_mb[MAX_MB_SIZE];//(1920*2/16)x(1080*2/16)/8
    //BitRateNode *brhead;
    //struct SwsContext *img_convert_ctx;
    cJSON *json;
    char *params;
    RtxNode *rtxHead;
    CodecInfoObj codecInfo;
    NetInfoNode *selfNetInfoHead;//private
    Encoder *encoder;
    int status;
    uint32_t ssrc;
    uint16_t seqnum;
    int64_t start_time;
    int frame_size;
    int frame_idx;
    int pict_type;
    int bitrate;
    int framerate;
    int lossrate;
    int idr_status;
    //int nettime;
    int loop_loss_rate;
    int delay_time;
    int push_delay_time;
    void *pCpurate;
    int icpurate;
    char *outparam[4];
    uint8_t * outbuffer;
    uint8_t * rtpbuffer;
    uint8_t * outrtp;
    FILE *outfp;
    FILE *logfp;
    pthread_mutex_t lock;
}CallCodecVideo, CallCodecAudio;

typedef struct{
    char streamHnd[8];
    char *params;
    pthread_mutex_t lock;
    pthread_mutex_t status_lock;
    int audio_status;
    int video_status;
    int64_t start_time;
    int offset;
    CallRender *render;
    CallPlayer *player;
    CallCodecVideo *video;
    CallCodecAudio *audio;
    McuNode *datahead[2];
    uint16_t seqnum;
}McuObj;

typedef struct{
    CallCapture *capture;
    CallRender *render;
    CallCodecVideo *codec;
    SessionInfoObj *sessionInfo;
    NetInfoObj *netInfoObj;
    McuObj *mcu;
    char handle[8];
}SessionObj;



#ifdef __cplusplus
}
#endif

#endif

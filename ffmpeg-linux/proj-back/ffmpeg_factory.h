//edited by gxh
#ifndef FFMPEG_FACTORY_H
#define FFMPEG_FACTORY_H

#ifdef  __cplusplus    
extern "C" {
#endif

	//宽高最小单元为2x2，即宽高必须为偶数，否则编码异常退出；
	//#define GLOB_DEC
#define MAX_AUDIO_SIZE		(8192 * 3)//(8192 << 1)
#define GLOBAL_HEADER
#define SYS_TIMER
#define AVIOTEST

#ifndef _WIN32
	typedef int64_t __int64;
	typedef int64_t DWORD;
#endif

	typedef std::map<int, void*> MediaMap;

	enum {kInData, kOutData};
	typedef struct
	{
		long long time_stamp_start;
		long long pkt_time_stamp_start;
	}TimeStampInfo;
	typedef struct
	{
		int left;
		int right;
		int top;
		int bottom;
		float alpha;
		bool fit;
	}WaterMarkParams;;
	typedef struct
	{
		int width;
		int height;
		int size;
		int type;
		char *data;
	}BaseData;
	//out_flag;	b0:high stream,	b1:middle stream,	b2:low stream	|
	//			b3:high file,	b4:middle file,		b5:low file		|
	typedef struct
	{
		//video
		int in_data_format;		//raw(yuv) or stream
		int width;
		int height;
		int width1;
		int height1;
		int width2;
		int height2;
		int gop_size;
		int mtu_size;
		int frame_rate;
		int min_bits_rate;
		int max_bits_rate;
		int out_data_format;	//ras(yuv) or stream
		int video_port;
		int video_bits_rate;
		int video_bits_rate1;
		int video_bits_rate2;
		int video_codec_id;
		char preset[32];
		char video_sdp_filename[MAX_PATH];
		//audio
		int audio_bits_rate;
		int audio_codec_id;
		int frame_size;
		int in_channel_count;
		int in_sample_rate;
		int in_sample_fmt;
		int out_channel_count;
		int out_sample_rate;
		int out_sample_fmt;
		int audio_port;
		char sdp_filename[MAX_PATH];
		//common
		int streamId;
		unsigned int debugInfo;
		unsigned int out_flag;
		char in_filename[MAX_PATH];
		char out_filename[MAX_PATH];
		char out_streamname[MAX_PATH];
		char ipaddr[64];
	}CodecParams;


	typedef struct
	{
		struct SwrContext *swr_ctx;
		AVFrame srcFrame;
		AVFrame dstFrame;
	}ffmpegResample;
	typedef struct
	{
		struct SwsContext *sws_ctx;
		AVFrame srcFrame;
		AVFrame dstFrame;
	}ffmpegScale;
	typedef struct
	{
		AVFormatContext *fmtctx;
		AVIOContext *avio;
		AVInputFormat *ifmt;
		AVOutputFormat *ofmt;
		AVDictionary *opt;
		//
		AVCodecContext *vpCodecCtx;
		AVCodecContext *apCodecCtx;
		AVCodec *video_codec;
		AVCodec *audio_codec;
		//
		int ReadIsOk;
		int WriteHead;
		int WriteAudio;
		__int64 time0;
		__int64 start_time;
		int split_len;//ms
		int WriteIsOpen;
		int RecordStatus;//0:Stop,1:Paus,2:Start,3:InSet;//also can stop live stream
		int AVWriteFlag;//1:audio,2:video
		int AVReadFlag;//1:audio,2:video
		int video_index;
		int audio_index;
#ifdef _WIN32
		LARGE_INTEGER ptime;
		LARGE_INTEGER pfreq;
#endif
		__int64 timeout;
#if 1
		webrtc::CriticalSectionWrapper* write_critsect;
		webrtc::CriticalSectionWrapper* read_critsect;
#else
		void* write_critsect;
		void* read_critsect;
#endif

	}AVStatus, AVMuxer;
	//读取数据/读流
	//解码
	//编码
	//输出数据/写流
	typedef struct
	{
		char inFile[MAX_PATH];
		char outFile[MAX_PATH];
		char fileType[32];
		char buf[MAX_AUDIO_SIZE];
		//
		AVStream *st;
		//
		AVCodecContext *codecCtx;
		AVCodec *codec;
		//
		AVPacket pkt;
		//AVPacket pkt2;
		AVFrame *frame;
		AVFrame *tmp_frame;
		AVPicture pic;
		AVPicture alloc_pic;
		AVBitStreamFilterContext* bsfc;
		ffmpegScale *scale;
		ffmpegResample *resample;
		int stream_idx;
		//AVStatus *mux;
		AVMuxer *mux;
		void *av_filter;
		void *wave_stream;
		//int type;//audio/video/av
		int levelId;
		int id;
		int FileOrStream;//file:1, stream:0
		int EncOrDec;//encoder:1, decoder:0
		int AudioOrVideo;//audio:1, video:0, av:2
		int WriteIsCpy;//0:new,1:copy
		int ReadIsCpy;//0:new,1:copy
		int pos;
		//
		int sdp_flag;
		int debug_flag;
		FILE *ifp;
		FILE *ofp;
	}CoreStream;

	typedef struct
	{
		char cparams[PARAMS_SIZE];
		void *jsonObj;
		//jsonObject *json_obj;
		CodecParams params;
		CoreStream *CodecSt;
		CoreStream *ReadSt;
		CoreStream *WriteSt[6];
		BaseData img[2][2];//inset image, water mark image //in, out
		WaterMarkParams waterMarkParams;
		int streamId;//long long
		int ChanId;
		int winWidth;
		int winHeight;
		int orgX;
		int orgY;
		int showWidth;
		int showHeight;
		int mode;
		int reset;
		//ffmpegResample *resample;
		char filePath[MAX_PATH];
		char jpgFile[3][MAX_PATH];
		int sdp_flag;
		int codecType;
		int debug_flag;
		FILE *ifp;
		FILE *ofp;
	}MediaStream;

	typedef struct
	{
		char _cparams[PARAMS_SIZE];
		//jsonObject _json_obj;
		MediaMap _ffmap;
		int status;
		//webrtc::CriticalSectionWrapper* _critsect;
	}ffmpegFactory;

#ifdef  __cplusplus    
}
#endif

#endif
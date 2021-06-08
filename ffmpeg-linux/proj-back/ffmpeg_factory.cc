//edited by gxh
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include <map>
#include "webrtc/system_wrappers/interface/critical_section_wrapper.h"
#include "webrtc/system_wrappers/interface/tick_util.h"
#include "webrtc/base/basetype.h"
//#include "webrtc/modules/rtp_rtcp/source/ssrc_database.h"
#include "webrtc/avengine/interface/avengAPI.h"
#include "webrtc/avengine/source/gxhlog.h"
#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
//#include <vld.h>
#include <Mmsystem.h>
#else
#include <sys/time.h>
#endif

#include "jsonstr.h"
#if defined(WEBRTC_IOS) || defined(WEBRTC_MAC) || defined(IOS)
#include "../interface/file_ios.h"
#endif

#ifdef  __cplusplus    
extern "C" {    
#endif
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libavfilter/avfilter.h"
#include "libavutil/avutil.h"
#include "libavutil/file.h"
#include "libavutil/avassert.h"
#include "libavutil/channel_layout.h"
#include "libavutil/opt.h"
#include "libavutil/mathematics.h"
#include "libavutil/imgutils.h"
//#include <libavutil/timestamp.h>

#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/time_internal.h"
#include "libavutil/time.h"
//
#if 0
#include "sdl/SDL.h"
#include "sdl/SDL_thread.h"
#endif
//#include "jsonstr.h"
#include "ffmpeg_scaler.h"
#include "ffmpeg_resample.h"
#include "ffmpeg_factory.h"
#include "ffmpeg_filter.h"
#include "webrtc/modules/av_coding/codecs/ffmpeg/main/interface/ffmpeg_api.h"

#ifdef _WIN32
#pragma comment(lib,"webrtc/gxh/win/ffmpeg/lib/avcodec.lib")
#pragma comment(lib,"webrtc/gxh/win/ffmpeg/lib/avutil.lib")
#pragma comment(lib,"webrtc/gxh/win/ffmpeg/lib/avformat.lib")
#pragma comment(lib,"webrtc/gxh/win/ffmpeg/lib/swresample.lib")
#pragma comment(lib,"webrtc/gxh/win/ffmpeg/lib/swscale.lib")
#pragma comment(lib,"webrtc/gxh/win/ffmpeg/lib/avfilter.lib")
	/*
	#pragma comment(lib,"webrtc/gxh/win/ffmpeg/libswresample/swresample.lib")
	#pragma comment(lib,"webrtc/gxh/win/ffmpeg/libavfilter/avfilter.lib")
	#pragma comment(lib,"webrtc/gxh/win/ffmpeg/libavdevice/avdevice.lib")
	*/
#endif 

#ifdef  __cplusplus    
}    
#endif
//宽高最小单元为2x2，即宽高必须为偶数，否则编码异常退出；
#if 0
//#define GLOB_DEC
#define MAX_AUDIO_SIZE		(8192 * 3)//(8192 << 1)
#define GLOBAL_HEADER
#define SYS_TIMER
#define AVIOTEST

#ifndef _WIN32
typedef int64_t __int64;
typedef int64_t DWORD;
#endif
#endif
//enum StretchMode
//{
//	kStretchFull = 0,
//	kStretchToInsideEdge = 1,
//	kStretchToOutsideEdge = 2,
//	kStretchMatchWidth = 3,
//	kStretchMatchHeight = 4,
//	kStretchNone = 5
//};
//extern FILE *logfp;
FILE *logfp = NULL;

#ifdef _WIN32

#define SPEEXECHO_TEST

#ifndef SPEEXECHO_TEST
#pragma comment(lib,"/webrtc/modules/AudioPreprocess/Speex/lib/libspeexdspMTD.lib")
static webrtc::CriticalSectionWrapper* g_echo_cri = webrtc::CriticalSectionWrapper::CreateCriticalSection();
#endif
#endif

#define SETVALUE2KEY0(pHnd, target, default, key)\
{\
	jsonData *data = (jsonData *)json_1d_get_data(pHnd, "codecParams", key);\
	if(data && data->cValue)\
{\
	target = data->iValue;\
}\
	else\
{\
	target = default;\
}\
}
#define SETVALUE2KEY1(pHnd, target, default, key)\
{\
	jsonData *data = (jsonData *)json_1d_get_data(pHnd, "codecParams", key);\
	if(data && data->cValue)\
{\
	strcpy(target, data->cValue);\
}\
	else\
{\
	strcpy(target, default);\
}\
}
#if 0
typedef std::map<int,void*> MediaMap;


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
	/*char inFile[MAX_PATH];
	char outFile[MAX_PATH];
	char fileType[32];*/
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
	__int64 time0;
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
	__int64 timeout;
	webrtc::CriticalSectionWrapper* write_critsect;
	webrtc::CriticalSectionWrapper* read_critsect;
}AVStatus,AVMuxer;
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
#endif
static ffmpegFactory ffFactory = {};
static webrtc::CriticalSectionWrapper* g_critsect = webrtc::CriticalSectionWrapper::CreateCriticalSection();
static void *g_ffFactory = NULL;// (void *)&ffFactory;
void *GetInFFmpegFactory(int streamId, char *inFile, int type);
typedef void(*pCritsCallback)(void **hnd, int flag);
pCritsCallback g_cb = NULL;
#ifdef GLOB_DEC
//static AVCodec *g_audio_codec = NULL;
static AVCodecContext *g_audio_c = NULL;
static AVCodecContext *g_video_c = NULL;
#endif
//int64_t testtime = webrtc::TickTime::MillisecondTimestamp();
/////////////////////////////////////////////////////////////////////////////////
int WriteOpen(MediaStream *mst, CoreStream *pWrite);
int WriteHeader(AVFormatContext *oc);
int audio_decode2(MediaStream *mst, CoreStream *audio, char * inBuf[3], int len, char *outBuf[3], int oLen, int &frame_type);
void GetWaterMarkImage(void *hnd, char *infile, char *outBuf0[3], int oWidth, int oHeight);
void GetInsetImage(void *hnd, char *infile, char *outBuf[3], int oWidth, int oHeight);
#ifndef _WIN32
static char *itoa(int value,char *string,int radix)
{
    int rt=0;
    if(string==NULL)
        return NULL;
    if(radix<=0||radix>30)
        return NULL;
    rt=snprintf(string,radix,"%d",value);
    if(rt>radix)
        return NULL;
    string[rt]='\0';
    return string;
}
#endif

#if 0
// Ue find the num of zeros and get (num+1) bits from the first 1, and 
// change it to decimal
// e.g. 00110 -> return 6(110)
UINT Ue(BYTE *pBuff, UINT nLen, UINT &nStartBit)
{
	//count 0bit
	UINT nZeroNum = 0;
	while (nStartBit < nLen * 8)
	{
		if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8))) 
		{
			break;
		}
		nZeroNum++;
		nStartBit++;
	}
	nStartBit++;

	DWORD dwRet = 0;
	for (UINT i = 0; i<nZeroNum; i++)
	{
		dwRet <<= 1;
		if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8)))
		{
			dwRet += 1;
		}
		nStartBit++;
	}
	return (1 << nZeroNum) - 1 + dwRet;
}

int Se(BYTE *pBuff, UINT nLen, UINT &nStartBit)
{
	int UeVal = Ue(pBuff, nLen, nStartBit);
	double k = UeVal;
	int nValue = std::ceil(k / 2);//ceil(2)=ceil(1.2)=cei(1.5)=2.00
	if (UeVal % 2 == 0)
		nValue = -nValue;
	return nValue;
}

// u Just returns the BitCount bits of buf and change it to decimal.
// e.g. BitCount = 4, buf = 01011100, then return 5(0101)
DWORD u(UINT BitCount, BYTE * buf, UINT &nStartBit)
{
	DWORD dwRet = 0;
	for (UINT i = 0; i<BitCount; i++)
	{
		dwRet <<= 1;
		if (buf[nStartBit / 8] & (0x80 >> (nStartBit % 8)))
		{
			dwRet += 1;
		}
		nStartBit++;
	}
	return dwRet;
}
bool getResolution(int channel, int &Width, int& Height)
{
	BYTE *buf = new BYTE[1024];
	int nLen;
	AVPacket packet;
	BOOL bSpsComplete = FALSE;
	// Find SPS
	
	BYTE* p = packet.data;
	BYTE last_nal_type = 0;
	int last_nal_pos = 0;
	for (int i = 0; i<packet.size - 5; i++)
	{
		p = packet.data + i;
		if (p[0] == 0x00 && p[1] == 0x00 && p[2] == 0x00 && p[3] == 0x01)
		{
			if (last_nal_type == 0x67)
			{
				nLen = i - last_nal_pos;
				memcpy(buf, packet.data + last_nal_pos, nLen);
				bSpsComplete = TRUE;
			}
			last_nal_type = p[4];
			last_nal_pos = i;
			if (bSpsComplete)
			{
				break;
			}
		}
	}
	if (last_nal_type == 0x67 && bSpsComplete == FALSE)
	{
		nLen = packet.size - last_nal_pos;
		memcpy(buf, packet.data + last_nal_pos, nLen);
		bSpsComplete = TRUE;
	}
	// Analyze SPS to find width and height
	UINT StartBit = 0;
	buf = buf + 4;
	int forbidden_zero_bit = u(1, buf, StartBit);
	int nal_ref_idc = u(2, buf, StartBit);
	int nal_unit_type = u(5, buf, StartBit);
	if (nal_unit_type == 7)
	{
		int profile_idc = u(8, buf, StartBit);
		int constraint_set0_flag = u(1, buf, StartBit);//(buf[1] & 0x80)>>7;
		int constraint_set1_flag = u(1, buf, StartBit);//(buf[1] & 0x40)>>6;
		int constraint_set2_flag = u(1, buf, StartBit);//(buf[1] & 0x20)>>5;
		int constraint_set3_flag = u(1, buf, StartBit);//(buf[1] & 0x10)>>4;
		int reserved_zero_4bits = u(4, buf, StartBit);
		int level_idc = u(8, buf, StartBit);

		int seq_parameter_set_id = Ue(buf, nLen, StartBit);

		if (profile_idc == 100 || profile_idc == 110 ||
			profile_idc == 122 || profile_idc == 144)
		{
			int chroma_format_idc = Ue(buf, nLen, StartBit);
			if (chroma_format_idc == 3)
				int residual_colour_transform_flag = u(1, buf, StartBit);
			int bit_depth_luma_minus8 = Ue(buf, nLen, StartBit);
			int bit_depth_chroma_minus8 = Ue(buf, nLen, StartBit);
			int qpprime_y_zero_transform_bypass_flag = u(1, buf, StartBit);
			int seq_scaling_matrix_present_flag = u(1, buf, StartBit);

			int seq_scaling_list_present_flag[8];
			if (seq_scaling_matrix_present_flag)
			{
				for (int i = 0; i < 8; i++) {
					seq_scaling_list_present_flag[i] = u(1, buf, StartBit);
				}
			}
		}
		int log2_max_frame_num_minus4 = Ue(buf, nLen, StartBit);
		int pic_order_cnt_type = Ue(buf, nLen, StartBit);
		if (pic_order_cnt_type == 0)
			int log2_max_pic_order_cnt_lsb_minus4 = Ue(buf, nLen, StartBit);
		else if (pic_order_cnt_type == 1)
		{
			int delta_pic_order_always_zero_flag = u(1, buf, StartBit);
			int offset_for_non_ref_pic = Se(buf, nLen, StartBit);
			int offset_for_top_to_bottom_field = Se(buf, nLen, StartBit);
			int num_ref_frames_in_pic_order_cnt_cycle = Ue(buf, nLen, StartBit);

			int *offset_for_ref_frame = new int[num_ref_frames_in_pic_order_cnt_cycle];
			for (int i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++)
				offset_for_ref_frame[i] = Se(buf, nLen, StartBit);
			delete[] offset_for_ref_frame;
		}
		int num_ref_frames = Ue(buf, nLen, StartBit);
		int gaps_in_frame_num_value_allowed_flag = u(1, buf, StartBit);
		int pic_width_in_mbs_minus1 = Ue(buf, nLen, StartBit);
		int pic_height_in_map_units_minus1 = Ue(buf, nLen, StartBit);

		Width = (pic_width_in_mbs_minus1 + 1) * 16;
		Height = (pic_height_in_map_units_minus1 + 1) * 16;

		return true;
	}
	else
	{
		return false;
	}
}
#endif
void SetCodeType(void *hnd, int type)
{
	MediaStream *mst = (MediaStream *)hnd;
	mst->codecType = type;
}
void FF_GET_TIME2(int64_t time, char *ctime, int offset)
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
void FF_GET_TIIME(char *name)
//static void get_time(char *name)
{
	//char tmp[64];
	time_t t = time(0);
	strftime(name, 64, "%Y%m%d%H%M%S", localtime(&t));
}
#if 0
static void get_date(char *now_time)
{
	//char now_time[256];
	time_t now = time(0);
	struct tm *ptm, tmbuf;
	ptm = localtime_r(&now, &tmbuf);
	int size = 64;// sizeof(now_time);
	if (ptm) {
		strftime(now_time, size, "%Y-%m-%d %H:%M:%S", ptm);
	}
	//strftime(cDate, 64, "%Y-%m-%d %H:%M:%S", localtime(&t));
}
#endif
static int renew_filename(char *oldname, char *head, char *tail)
{
	char newname[255] = "";
	if (oldname == NULL)
		return -1;
	FF_GET_TIIME(newname);
	strcpy(oldname, head);
	strcat(oldname, "-");
	strcat(oldname, newname);
	strcat(oldname, tail);
	//
	/*mystrcat(oldname, newname, '_');
	strcpy(oldname, newname);*/
	return 0;
}
static void renew_filename2(CoreStream *pWrite, char *new_name)
{
	//char new_name[255] = "";
	char* pPos = NULL;
	char id[8] = "";
	char tail[16] = "";
	char *filename = new_name;
	strcpy(new_name, pWrite->outFile);
	itoa(pWrite->levelId, id, 10);
	pPos = strrchr(filename, '.');
	strcpy(tail, pPos);
	*pPos = '\0';
	//char *tail = pWrite->fileType;
	renew_filename(filename, filename, tail);
}
static int get_extend(char *filename, char *extend, int levelId, int fileOrStream)
{
	int ret = 0;
	char *pPos = NULL;
	char cLevelId[32] = "";
	if(fileOrStream)
	{
		pPos = strrchr(filename,'.');
		if(pPos)
		{
			strcpy(extend, ++pPos);
			ret = strlen(pPos);

			if (!strncmp(extend, "ffm", strlen("ffm")))
			{
				//pPos = strrchr(filename, '.');
				//strcpy(extend, ++pPos);
				ret = strlen(filename);
			}
			else if (levelId >= 0)
			{
				itoa(levelId, cLevelId, 10);
				pPos--;
				pPos[0] = '\0';
				strcat(pPos, "-");
				strcat(pPos, cLevelId);
				strcat(pPos, ".");
				strcat(pPos, extend);
				ret = strlen(pPos);
			}
		}
	}
	else
	{
		pPos = strchr(filename,':');
		if(pPos)
		{
			strncpy(extend, filename, (int)(pPos - filename) * sizeof(char));
			if(!strncmp(extend, "rtmp",strlen("rtmp")))
			{
				strcpy(extend, "flv");

				ret = strlen(pPos);

				if (levelId >= 0)
				{
					itoa(levelId, cLevelId, 10);
					strcat(filename, "-");
					strcat(filename, cLevelId);
					if (!strncmp(extend, "flv", strlen("flv")))
					{
						strcat(filename, " live=1");
					}
				}
			}
			else if (!strncmp(extend, "http", strlen("http")))
			{
				pPos = strrchr(filename, '.');
				strcpy(extend, ++pPos);
			}
			ret = strlen(filename);
		}
	}
	return ret;
}
static void fill_yuv_image(MediaStream *mst, CoreStream *video)
{
	AVCodecContext *c = video->codecCtx;
	AVPicture *pict = &video->pic;
	int frame_index = c->frame_number;
	int width = c->width;
	int height = c->height;
	int x, y, i;
	i = frame_index;
#if 0
	// Y 
	for (y = 0; y < height; y++)
		for (x = 0; x < width; x++)
			pict->data[0][y * pict->linesize[0] +x] = x + y + i * 3;
	// Cb and Cr 
	for (y = 0; y < height / 2; y++) 
	{
		for (x = 0; x < width / 2; x++) 
		{
			pict->data[1][y * pict->linesize[1] +x] = 128 + y + i * 2;
			pict->data[2][y * pict->linesize[2] +x] = 64 + x + i * 5;
		}
	}
#else
	static FILE *fp = NULL;
	if(!fp)
	{
#if 1
		fp = fopen("e://works//video//d1_640x480_0.yuv", "rb");//plane_704x576
#else
		fp = fopen("c://works//videos//disney-1200x480.yuv", "rb");
#endif
		avpicture_alloc(pict, AV_PIX_FMT_YUV420P, width, height);
	}
read_file:
	if(fp)
	{
		int rsize = 0;
		// Y 
		for (y = 0; y < height; y++)
		{
			rsize = fread(&pict->data[0][y * pict->linesize[0]], 1, width, fp);
			if(rsize != width)
			{
				fseek( fp, 0, SEEK_SET );
				goto read_file;
			}
		}

		for (y = 0; y < (height >> 1); y++) 
		{
			rsize = fread(&pict->data[1][y * pict->linesize[1]], 1, (width >> 1), fp);
			if(rsize != (width >> 1))
			{
				fseek( fp, 0, SEEK_SET );
				goto read_file;
			}
		}
		for (y = 0; y < (height >> 1); y++) 
		{
			rsize = fread(&pict->data[2][y * pict->linesize[2]], 1, (width >> 1), fp);
			if(rsize != (width >> 1))
			{
				fseek( fp, 0, SEEK_SET );
				goto read_file;
			}
		}
	}
#endif 
}
static void fill_yuv_image2(char *data[3], int w, int h)
{
	int width = w;
	int height = h;
	int size = w * h;
	static int frmNum = 0;
	static FILE *fp = NULL;
	if(!fp)
	{
#ifdef _WIN32
#if 0
		fp = fopen("e://works//video//foreman_cif.yuv", "rb");//plane_704x576
#else
		fp = fopen("c://works//videos//disney-1200x480.yuv", "rb");
#endif
#elif defined(__ANDROID__)
        fp = fopen("/sdcard/foreman_cif.yuv", "rb");
#elif defined(WEBRTC_IOS)
		char dirName[256] = "foreman_cif.yuv";
		char *pName = MAKE_FILE_NAME(dirName);//makePreferencesFilename("webrtc_log.txt");
		fp = fopen(pName, "rb");
		free(pName); 
#else
#endif
	}
read_file:
	if(fp)
	{
		int rsize = 0;
		rsize = fread(data[0], 1, size, fp);
		if(rsize != size)
		{
			fseek( fp, 0, SEEK_SET );
			goto read_file;
		}
		rsize = fread(data[1], 1, (size >> 2), fp);
		if(rsize != (size >> 2))
		{
			fseek( fp, 0, SEEK_SET );
			goto read_file;
		}
		rsize = fread(data[2], 1, (size >> 2), fp);
		if(rsize != (size >> 2))
		{
			fseek( fp, 0, SEEK_SET );
			goto read_file;
		}
	}
	frmNum++;
}
static char *tmpBuf = NULL;// [576000] = {};//1200x480
static void fill_yuv_image3(char *data[3], int w, int h)
{
	int width = w;
	int height = h;
	int size = w * h;
	static int frmNum = 0;
	if (!tmpBuf)
	{
		tmpBuf = (char *)malloc(size * 3 * sizeof(char));
	}
	static FILE *fp = NULL;
	if (!fp)
	{
#ifdef _WIN32
#if 0
		fp = fopen("e://works//video//foreman_cif.yuv", "rb");//plane_704x576
#else
		fp = fopen("c://works//videos//disney-1200x480.yuv", "rb");
#endif
#elif defined(__ANDROID__)
		fp = fopen("/sdcard/foreman_cif.yuv", "rb");
#elif defined(IOS)
		char dirName[256] = "foreman_cif.yuv";
		char *pName = MAKE_FILE_NAME(dirName);//makePreferencesFilename("webrtc_log.txt");
		fp = fopen(pName, "rb");
		free(pName);
#else
#endif
	}
read_file:
	if (fp)
	{
		int rsize = 0;
		//if (!(frmNum & 1))
		{
			rsize = fread(tmpBuf, 1, size * 3, fp);
			if (rsize != size * 3)
			{
				fseek(fp, 0, SEEK_SET);
				goto read_file;
			}
		}
		
		int offset = !(frmNum & 1) ? 0 : w;		offset = 0;
		for (int i = 0; i < h; i++)
		{
			memcpy(&(data[0][i*w]), &tmpBuf[offset + i*(w << 1)], w);
			if (!(i & 1))
			{
				int offset1 = (w << 1) * h + (offset >> 1);
				int offset2 = offset1 + w * (h >> 1);
				memcpy(&(data[1][(i >> 1) * (w >> 1)]), &tmpBuf[offset1 + (i >> 1) * w], w >> 1);
				memcpy(&(data[2][(i >> 1) * (w >> 1)]), &tmpBuf[offset2 + (i >> 1) * w], w >> 1);
			}
		}
	}
	frmNum++;
}
static AVFrame *get_audio_frame(MediaStream *mst, CoreStream *audio)//OutputStream *ost)
{
	CodecParams *params = &mst->params;
	AVFrame *frame = audio->tmp_frame;
	int j, i, v;
	int16_t *q = (int16_t*)frame->data[0];
#if 0
	// check if we want to generate more frames 
	//    if (av_compare_ts(ost->next_pts, ost->st->codec->time_base, STREAM_DURATION, (AVRational){ 1, 1 }) >= 0)
	//        return NULL;

	for (j = 0; j <frame->nb_samples; j++) 
	{
		v = (int)(sin(audio->t) * 10000);
		for (i = 0; i < audio->st->codec->channels; i++)
			*q++ = (short)v;
		audio->t     += audio->tincr;
		audio->tincr += audio->tincr2;
	}
#else
	//static FILE *fp = NULL;
	if(!audio->ifp)
	{
		if(params->in_sample_rate == 48000)
		{
#ifdef _WIN32
			audio->ifp = fopen("e://works//video//20063.wav", "rb");
#elif defined(__ANDROID__)
            audio->ifp = fopen("/sdcard/20063.wav", "rb");
#elif defined(WEBRTC_IOS)
			char dirName[256] = "20063.wav";
			char *pName = MAKE_FILE_NAME(dirName);//makePreferencesFilename("webrtc_log.txt");
			audio->ifp = fopen(pName, "rb");
			free(pName);
#else
#endif
		}
		else if(params->in_sample_rate == 44100)
		{
#ifdef _WIN32
			audio->ifp = fopen("e://works//video//441-wf-xj.wav", "rb");
#elif defined(__ANDROID__)
            audio->ifp = fopen("/sdcard/441-wf-xj.wav", "rb");
#elif defined(WEBRTC_IOS)
			char dirName[256] = "441-wf-xj.wav";
			char *pName = MAKE_FILE_NAME(dirName);//makePreferencesFilename("webrtc_log.txt");
			audio->ifp = fopen(pName, "rb");
			free(pName);
#else
#endif
		}
	}
	if(audio->ifp)
	{
		short pcmData[4096];
		//int rsize = fread(frame->data[0], 1, 8192, audio->ifp);
		int rsize = fread((void *)pcmData, 1, 8192, audio->ifp);
		if(rsize != 8192)
		{
			fseek( audio->ifp, 0, SEEK_SET );
			//rsize = fread(frame->data[0], 1, 8192, audio->ifp);
			rsize = fread((void *)pcmData, 1, 8192, audio->ifp);
		}
		short *src = (short *)frame->data[0];
		for (int i = 0; i < 4096; i++)
		{
#if 1
			if (i & 1)//right
			{
				src[i] = src[i - 1];
			}
			else//left
			{
				src[i] = (src[i] + pcmData[i] + 1) >> 1;
			}
#else
			src[i] = (src[i] + pcmData[i] + 1) >> 1;
#endif
		}
	}
#endif 
	//frame->pts = audio->next_pts;
	//audio->next_pts  += frame->nb_samples;

	return frame;
}
#ifdef _WIN32
int64_t get_sys_time()
{
	LARGE_INTEGER m_nFreq;
	LARGE_INTEGER m_nTime;
	QueryPerformanceFrequency(&m_nFreq); // 获取时钟周期
	QueryPerformanceCounter(&m_nTime);//获取当前时间
	//long long time = (m_nTime.QuadPart * 1000000 / m_nFreq.QuadPart);//微妙
	int64_t time = (m_nTime.QuadPart * 1000 / m_nFreq.QuadPart);//毫秒
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
static int is_time(CoreStream *pCodec)
{
	int ret = 0;
	if (pCodec->mux->RecordStatus == kRecordStart && pCodec->mux->split_len && pCodec->FileOrStream)
	{
#if 1
		__int64 time1 = webrtc::TickTime::MillisecondTimestamp();
#else
		__int64 time1 = get_sys_time();
		if (g_cb)
		{
			g_cb((void **)&time1, 4);
		}
#endif
		int diff_time = time1 - pCodec->mux->start_time;// pCodec->mux->time0;
		ret = diff_time >= pCodec->mux->split_len;
	}
	return ret;
}
static int frame_write2(MediaStream *mst, CoreStream *pCodec, AVPacket *pkt0)
{
	AVCodecContext *c = pCodec->codecCtx;
	int ret = 0;
	int frame_number = c->frame_number;
#ifdef AVIOTEST
	if(pCodec->mux->fmtctx && pCodec->mux->WriteHead)//pCodec->mux->RecordStatus
	{
		AVPacket pkt = { 0 };
		av_init_packet(&pkt);

		pkt.data = pkt0->data;
		pkt.size = pkt0->size;
		pkt.duration = pkt0->duration;
		pkt.pts = pkt0->pts;
		pkt.dts = pkt0->dts;
		//pkt.test_pts[0] = pkt0->test_pts[0];
		//pkt.stream_index = pkt0->stream_index;
		pkt.stream_index = pCodec->st->index;
		pkt.flags = pkt0->flags;
		//pkt.frame_number = pkt0->frame_number;

		__int64 time0 = 0;
#ifndef FFMSTREAM
		if(pCodec->FileOrStream)
		{
#if 0
			//avformat_write_header()
			//__int64 time_stamp = (c->frame_number - 1) * 40;
			
			av_packet_rescale_ts(&pkt, c->time_base, pCodec->st->time_base);
#elif(1)
#if 1
			__int64 time_stamp = webrtc::TickTime::MillisecondTimestamp();
#else
			__int64 time_stamp = get_sys_time();
			if (g_cb)
			{
				g_cb((void **)&time_stamp, 4);
			}
#endif
			pCodec->mux->time0 = time_stamp;
			__int64 time_diff = time_stamp - pCodec->mux->start_time;// pCodec->mux->time0;
			pkt.dts = pkt.pts = time_diff * pCodec->st->time_base.den / 1000;
#else
			__int64 time_stamp = webrtc::TickTime::MillisecondTimestamp();
			__int64 time_diff = time_stamp - pCodec->mux->time0;
			__int64 fram_num = 0;
			//framnum = difftime/frametime
			if(pCodec->AudioOrVideo)
			{
				//framtime = c->sample_rate / c->frame_size
				//framnum = time_diff / framtime
				fram_num = (__int64)((double)time_diff * c->frame_size / c->sample_rate + 0.5);
				fram_num = c->frame_size * fram_num;
			}
			else
			{
				fram_num = (__int64)((double)time_diff * c->time_base.den / 1000 + 0.5);
				fram_num = (pCodec->st->time_base.den / c->time_base.den) * fram_num;
			}

			printf("av=%d \t fn=%d \n", pCodec->AudioOrVideo, fram_num);
			pkt.dts = pkt.pts = fram_num;
#endif
		}
		else
		{
#if 1
			__int64 time_stamp = webrtc::TickTime::MillisecondTimestamp();
#else
			__int64 time_stamp = get_sys_time();
			if (g_cb)
			{
				g_cb((void **)&time_stamp, 4);
			}
#endif
			time0 = 
			pCodec->mux->time0 = time_stamp;
#if 1
			__int64 time_diff = time_stamp - pCodec->mux->start_time;// pCodec->mux->time0;
			pkt.dts = pkt.pts = time_diff * pCodec->st->time_base.den / 1000;
#else
			pkt.pts = time_stamp;//(48000 / 25);
			pkt.dts = time_stamp;//(48000 / 25);
#endif

		}
#else
		
		av_packet_rescale_ts(&pkt, c->time_base, pCodec->st->time_base);
		
#endif
		static int count = 0;
		//printf("av_interleaved_write_frame: count= %d \t AudioOrVideo= %d \n", count++, pCodec->AudioOrVideo);
		if (pkt.size <= 0 || pkt.data == NULL)
		{
			printf("error: av_interleaved_write_frame: pkt.size= %d  \n", pkt.size);
		}
		else
		{
			if (pCodec->mux->AVWriteFlag == 3)
			{
				ret = av_interleaved_write_frame(pCodec->mux->fmtctx, &pkt);
			}
			else
			{
				ret = av_write_frame(pCodec->mux->fmtctx, &pkt);
			}
        }
#if 1
			__int64 time1 = webrtc::TickTime::MillisecondTimestamp();
#else
		__int64 time1 = get_sys_time();
		if (g_cb)
		{
			g_cb((void **)&time1, 4);
		}
#endif
		int diffTime = (int)(time1 - time0);
		if (pCodec->AudioOrVideo)//audio/flush
		{
			//printf("frame_write2: diffTime = %d \n", diffTime);
		}

		if(ret < 0)
		{
			printf("av_interleaved_write_frame: ret= %d \t index= %d \n", ret, pCodec->st->index);
			if (logfp) { fprintf(logfp, "frame_write2:ret= %d \n", ret); fflush(logfp); }
			printf("error \n");
			avio_close(pCodec->mux->fmtctx->pb);
			pCodec->mux->fmtctx->pb = NULL;
			pCodec->mux->WriteIsOpen = -1;
			pCodec->mux->WriteHead = 0;
            pCodec->mux->WriteAudio = 0;
		}
        av_free_packet(&pkt);
	}
	else if (pCodec->mux->WriteIsOpen < 0)
	{
		//ret = WriteInit(mst, pCodec);//???
		ret = WriteOpen(mst, pCodec);
		if (ret >= 0)
		{
			if (logfp) { fprintf(logfp, "WriteOpen-1 = %d \n", ret); fflush(logfp); }
			pCodec->mux->WriteIsOpen = 1;
			if (!pCodec->FileOrStream && pCodec->mux->RecordStatus)
			{
				if (logfp) { fprintf(logfp, "WriteHeader-0 = %d \n", ret); fflush(logfp); }
				pCodec->mux->WriteHead = WriteHeader(pCodec->mux->fmtctx) == 0;
				if (logfp) { fprintf(logfp, "WriteHeader-1 = %d \n", pCodec->mux->WriteHead); fflush(logfp); }
				if (!pCodec->mux->WriteHead)
				{
					printf("write head error \n");
					avio_close(pCodec->mux->fmtctx->pb);
					pCodec->mux->fmtctx->pb = NULL;
					pCodec->mux->WriteIsOpen = -1;
					pCodec->mux->WriteHead = 0;
					pCodec->mux->WriteAudio = 0;
				}
			}
		}
	}
    //fprintf(logfp,"frame_write2 1 %d \n",ret); fflush(logfp);
#endif
	return ret;
}
#if 0
static int frame_write(CoreStream *pCodec, AVPacket *pkt0)
{
	AVCodecContext *c = pCodec->codecCtx;
	int ret = 0;
	if(pCodec->mux->fmtctx)
	{
		AVPacket pkt = { 0 };
		av_init_packet(&pkt);
		pkt = *pkt0;
		pkt.stream_index = pCodec->st->index;
		pkt.duration;
#ifndef SYS_TIMER
		//__int64 time_stamp = (c->frame_number - 1) * 40;
		av_packet_rescale_ts(&pkt, c->time_base, pCodec->st->time_base);
#else
		__int64 time_stamp = webrtc::TickTime::MillisecondTimestamp();
		pkt.pts = time_stamp;//(48000 / 25);
		pkt.dts = time_stamp;//(48000 / 25);
#endif
		//pCodec->pkt = pkt;
		ret = av_interleaved_write_frame(pCodec->mux->fmtctx, &pkt);
		//ret1 = av_write_frame(pCodec->mux->fmtctx,&pkt);
		if(ret < 0)
		{
			printf("error \n");
		}
	}
	return ret;
}
static int audio_write(CoreStream *audio, AVPacket *pkt)
{
	AVCodecContext *c = audio->codecCtx;
	int ret = 0;
	if(audio->mux->fmtctx)
	{
		pkt->stream_index = audio->st->index;
		//
#ifndef SYS_TIMER
		//__int64 time_stamp = (c->frame_number - 1) * frame->nb_samples;
		av_packet_rescale_ts(pkt, c->time_base, audio->st->time_base);
#else
		__int64 time_stamp = webrtc::TickTime::MillisecondTimestamp();
		pkt->pts = time_stamp;
		pkt->dts = time_stamp;
#endif

		pkt->flags = 0;
		pkt->duration;
		int ret1 = 0;
		//audio->pkt = pkt;
		ret = av_interleaved_write_frame(audio->mux->fmtctx, pkt);
		//ret1 = av_write_frame(audio->mux->fmtctx,&pkt);
		if(ret < 0)
		{
			printf("error \n");
		}
	}
	return ret;
}
static int video_write(CoreStream *video, AVPacket *pkt)
{
	AVCodecContext *c = video->codecCtx;
	int ret = 0;
	if(video->mux->fmtctx)
	{
		pkt->stream_index = video->st->index;
#if 0
		AVPacket tmpPkt = *pkt;

		int ret = av_bitstream_filter_filter(video->bsfc, video->mux->fmtctx->streams[video->st->index]->codec, NULL, &tmppkt->data, &tmppkt->size, pkt->data, pkt->size, pkt->flags & AV_PKT_FLAG_KEY);
		if(ret > 0)
		{
			*pkt = tmpPkt;
		}
#endif
		pkt->duration;
#ifndef SYS_TIMER
		//__int64 time_stamp = (c->frame_number - 1) * 40;
		av_packet_rescale_ts(pkt, c->time_base, video->st->time_base);
#else
		__int64 time_stamp = webrtc::TickTime::MillisecondTimestamp();
		pkt->pts = time_stamp;//(48000 / 25);
		pkt->dts = time_stamp;//(48000 / 25);
#endif

		ret = 0;
		//video->pkt = pkt;
		ret = av_interleaved_write_frame(video->mux->fmtctx,pkt);
		//ret1 = av_write_frame(video->mux->fmtctx,&pkt);
		if(ret < 0)
		{
			printf("error \n");
		}
	}
	return ret;
}
#endif

static int video_encode(MediaStream *mst, CoreStream *video, char * inBuf[3], int len, char *outBuf[3], int oLen, int &frame_type)
{
	CodecParams *params = &mst->params;
	//AVFormatContext *mux->fmtctx = video->mux->fmtctx;
	//AVStream *video_st = video->st;
	AVCodec *video_codec = video->codec; 
	AVFrame* frame = video->frame;
	char sdp[2048];

	// encode the image 
	AVPacket pkt;
	int got_output = 0;
	int ret = 0;
	AVCodecContext *c = video->codecCtx;

	av_init_packet(&pkt);
	pkt.data = NULL;    // packet data will be allocated by the encoder
	pkt.size = 0;
	//pkt.frame_number = c->frame_number;
#ifndef FFMSTREAM
	pkt.pts = 0;//c->frame_number * (48000 / 25);//AV_NOPTS_VALUE;
	pkt.dts = 0;//c->frame_number * (48000 / 25);//AV_NOPTS_VALUE;
#endif
	frame->pts = c->frame_number;
	//
	if (
		(video->mux && video->mux->WriteHead == 0 && video->mux->fmtctx && !video->FileOrStream)
///		|| (video->mux && video->mux->WriteAudio == 0)
		)
	{
		return ret;
	}
	
	//if(1)
	if(!inBuf)
	{
		fill_yuv_image(mst, video);//(&video->pic, c->frame_number,c->width, c->height);//OUTWIDTH, OUTHEIGHT);
	}
	else
	{
		int width = c->width;//params->width;//OUTWIDTH;
		int height = c->height;//params->height;//OUTHEIGHT;
		int imgSize = width * height;
		if(!video->levelId)
		{
			if (video->mux->RecordStatus == kRecordInSet)
			{
				if (mst->jpgFile[kInset] && strlen(mst->jpgFile[kInset]))
				{
					GetInsetImage((void *)mst, mst->jpgFile[kInset], inBuf, c->width, c->height);
				}
			}
			if (mst->jpgFile[kWaterMark] && strlen(mst->jpgFile[kWaterMark]))
			{
				GetWaterMarkImage((void *)mst, mst->jpgFile[kWaterMark], inBuf, c->width, c->height);
			}
			//else
			{
				video->pic.data[0] = (uint8_t *)inBuf[0];
				video->pic.data[1] = (uint8_t *)inBuf[1];//&inBuf[imgSize];
				video->pic.data[2] = (uint8_t *)inBuf[2];//&inBuf[imgSize + (imgSize >> 2)];
			}
			video->pic.linesize[0] = width;
			video->pic.linesize[1] = width >> 1;
			video->pic.linesize[2] = width >> 1;
#if 0
			//fill_yuv_image2(inBuf, width, height);
			fill_yuv_image3(inBuf, width, height);
#endif
		}
		else
		{
			//avpicture_alloc(&video->pic, AV_PIX_FMT_YUV420P, width, height);
		}
	}

	*((AVPicture *)video->frame) = video->pic;
	video->frame->width = c->width;//OUTWIDTH;
	video->frame->height = c->height;//OUTHEIGHT;
	video->frame->format = c->pix_fmt;//params->in_data_format;//AV_PIX_FMT_YUV420P;
#ifdef SYS_TIMER
	//if(!video->FileOrStream)
	//{
	//	__int64 time_stamp = webrtc::TickTime::MillisecondTimestamp();
	//	frame->pts = c->frame_number ? time_stamp : 0;
	//}
#endif	

#if 0
	char args[256] = "";
	char cTime[64] = "";
	GetSysTime(cTime);
	char context[255] = "drawtext=fontfile=arial.ttf:fontcolor=blue:fontsize=50:y=0:text=";
	strcat(context, "\'");
	strcat(context, cTime);
	strcat(context, "\'");
	
	//*frame_rate = c->framerate.num / c->framerate.den;
	//*frame_rate = c->time_base.den / c->time_base.num;
	static FILE *fp_out = NULL;
	///sprintf(args, "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d", c->width, c->height, c->pix_fmt, c->time_base.num, c->time_base.den, 1, 1);
	sprintf(args, "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d", c->width, c->height, c->pix_fmt, 1, 1, 1, 1);
	if (video->av_filter == NULL)
	{
		video->av_filter = FilterInit(video->av_filter, args, context);
		/*fp_out = fopen("E://works/test/output.yuv", "wb+");
		if (fp_out == NULL){
			printf("Error open output file.\n");
			return -1;
	}*/
	}
	else
	{
		ret = FilterRenew(video->av_filter, context);
	}
	FilterSetParams(video->av_filter, c->pix_fmt, c->width, c->height, c->width, inBuf[0], inBuf[1], inBuf[2], 0);
	FilterSetParams(video->av_filter, c->pix_fmt, c->width, c->height, c->width, inBuf[0], inBuf[1], inBuf[2], 1);
	ret = FilterProcess(video->av_filter, fp_out);
#elif defined(GXH_OSD)
	char cTime[64] = "";
	//GetSysTime(cTime);
	FF_GET_TIME2(0, cTime, 800 * 1000);
	char context[255] = "";
	strcat(context, cTime);
	sprintf(&context[strlen(context)], "  %d", c->frame_number);
	//sprintf(&context[strlen(context)], ",%dx", );
	if (video->av_filter == NULL)
	{
		video->av_filter = FontInit((void *)video->av_filter);
        printf("video_encode: context= %s \t c->width= %d ,c->height= %d \n",context, c->width, c->height);
	}
	FontOsdProcess((void *)video->av_filter, (unsigned char **)inBuf, c->width, c->height, 0, 2, 0, 4, context);
#endif	
	ret = avcodec_encode_video2(c, &pkt,frame, &got_output);
	if (ret < 0) 
	{
		return -1;
	}

	// If size is zero, it means the image was buffered. 
	if (got_output) 
	{
		//if (c->coded_frame->key_frame)
		//pkt.flags |= AV_PKT_FLAG_KEY;
		//c->coded_frame->key_frame = 0;
		if(pkt.flags & AV_PKT_FLAG_KEY)
		{
			frame_type = PIC_TYPE_KEYFRAME;
			//printf("");
		}
		//pkt.stream_index = video_st->index;
		if (pkt.pts != AV_NOPTS_VALUE )
		{
//			pkt.pts = av_rescale_q(pkt.pts,c->time_base, video_st->time_base);
		}
		if(pkt.dts !=AV_NOPTS_VALUE )
		{
//			pkt.dts = av_rescale_q(pkt.dts,c->time_base, video_st->time_base);
		}
		if(video->sdp_flag && (pkt.flags & AV_PKT_FLAG_KEY))
		{
			memcpy(outBuf[0], c->extradata, c->extradata_size);
			memcpy(&outBuf[0][c->extradata_size], pkt.data, pkt.size);
			ret = pkt.size + c->extradata_size;
		}
		else
		{
			memcpy(outBuf[0], pkt.data, pkt.size);
			ret = pkt.size;
		}
		video->pkt = pkt;
		//video->pkt2 = pkt;
#if 1
		//video->debug_flag = 1;
		if (!video->ofp && video->debug_flag)
		{
#ifdef _WIN32
			video->ofp = fopen("c://works//test//video.h264", "wb");
#elif defined(__ANDROID__)
			video->ofp = fopen("/sdcard/video.h264", "wb");
#elif defined(WEBRTC_IOS)
			char dirName[256] = "video.h264";
			char *pName = MAKE_FILE_NAME(dirName);//makePreferencesFilename("webrtc_log.txt");
			video->ofp = fopen(pName, "wb");
			free(pName);
#else
#endif
			
		}
		if(video->ofp)
		{
			fwrite(outBuf[0],1,ret, video->ofp);
		}
#endif
		//av_free_packet(&pkt);
#if 0
		if(video->mux->fmtctx)
		{
			pkt.stream_index = video->st->index;
#if 0
			AVPacket tmpPkt = pkt;

			int ret = av_bitstream_filter_filter(video->bsfc, video->mux->fmtctx->streams[video->st->index]->codec, NULL, &tmpPkt.data, &tmpPkt.size, pkt.data, pkt.size, pkt.flags & AV_PKT_FLAG_KEY);
			if(ret > 0)
			{
				pkt = tmpPkt;
			}
#endif
			pkt.duration;
#ifndef SYS_TIMER
			//__int64 time_stamp = (c->frame_number - 1) * 40;
			av_packet_rescale_ts(&pkt, c->time_base, video->st->time_base);
#else
			pkt.pts = time_stamp;//(48000 / 25);
			pkt.dts = time_stamp;//(48000 / 25);
#endif
			
			int ret1 = 0;
			//video->pkt = pkt;
			ret1 = av_interleaved_write_frame(video->mux->fmtctx,&pkt);
			//ret1 = av_write_frame(video->mux->fmtctx,&pkt);
			if(ret1 < 0)
			{
				printf("error \n");
			}
		}
#endif
	} 
	else {
		ret = 0;
		//len = 0;
	}
	//c->frame_number++;

	return ret;
}
static int video_decode(MediaStream *mst, CoreStream *video, char * inBuf[3], int len, char *outBuf[3], int oLen, int &frame_type)
{
	//AVFormatContext *fmtctx = video->mux->fmtctx;
	//AVStream *video_st = video->st;
	//AVCodec *video_codec = video->codec; 
	AVCodecContext *c = video->codecCtx;
	AVFrame* frame = video->frame;
	char sdp[2048];
	AVPacket pkt;
	int got_picture = 0;
	int ret = 0;
	int frameNumber = frame_type;
	//
	DWORD time0 = 0;
	DWORD time1 = 0;
	DWORD time2 = 0;
	DWORD time3 = 0;
	DWORD time4 = 0;
	int diff = 0;
#ifdef GLOB_DEC
	//audio_codec = g_audio_codec;
	c = g_video_c;
#endif
	if (strcmp(video->inFile, ""))
	{
		strcpy(video->inFile, mst->params.in_filename);
		c = (AVCodecContext *)GetInFFmpegFactory(mst->streamId, video->inFile, 0);
		if (c == NULL)
		{
			return -1;
		}
	}
	//
	av_init_packet(&pkt);
	pkt.size = len;
	pkt.data = (uint8_t *)inBuf[0];
	pkt.flags = 0;//AV_PKT_FLAG_KEY;

	//typedef struct RTPDemuxContext RTPDemuxContext;
	if (!strcmp(mst->params.in_filename, ""))
	{
#if 1
		if (video->sdp_flag)
		{
			AVPacket tmpPkt = pkt;
			int ret = av_bitstream_filter_filter(video->bsfc, video->codecCtx, NULL, &tmpPkt.data, &tmpPkt.size, pkt.data, pkt.size, pkt.flags & AV_PKT_FLAG_KEY);
			if (ret > 0)
			{
				pkt = tmpPkt;
			}
		}
#endif
		if (video->pic.data[0] == NULL)
		{
			avpicture_alloc(&video->pic, (enum AVPixelFormat)mst->params.in_data_format, mst->params.width, mst->params.height);//avpicture_free(AVPicture *picture);
			video->alloc_pic = video->pic;
		}
		*((AVPicture *)video->frame) = video->pic;
	}
	else
	{
		//video->frame = avcodec_alloc_frame();
		//printf("%s \n", mst->params.in_filename);
	}
	frame->key_frame = 0;
	while (pkt.size > 0)
	{
		//time0 = timeGetTime();
		ret = avcodec_decode_video2(c, frame, &got_picture, &pkt);
		//{
		//	time1 = timeGetTime();
		//	diff = (int)(time1 - time0);
		//	if(diff > 40)
		//		printf("a:time = %d \n", diff);
		//}


		if (ret < 0)
		{
			return -2;
		}

		if (got_picture)
		{
			if ((pkt.flags & AV_PKT_FLAG_KEY) || (frame->pict_type == AV_PICTURE_TYPE_I) || (frame->key_frame & AV_PKT_FLAG_KEY))
			{
				frame_type = PIC_TYPE_KEYFRAME;
				//printf("");
			}
			frame_type = frame->pict_type;
			if ((frame->key_frame & AV_PKT_FLAG_CORRUPT))
			{
				printf("error %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% \n");
			}
			int mode = 0;
			int winWidth = 0;
			int winHeight = 0;
			int orgX = 0;
			int orgY = 0;
			int showWidth = 0;
			int showHeight = 0;
            int reset = 0;
#if 1
			g_critsect->Enter();
#else
			if (g_cb)
			{
				g_cb(&g_critsect, 0);
			}
#endif
            reset = mst->reset;
			mode = mst->mode;
			winWidth = mst->winWidth;
			winHeight = mst->winHeight;
			orgX = mst->orgX;
			orgY = mst->orgY;
			showWidth = mst->showWidth;
			showHeight = mst->showHeight;
#if 1
			g_critsect->Leave();
#else
			if (g_cb)
			{
				g_cb(&g_critsect, 1);
			}
#endif
            
			if (!mode || winWidth < 0)
			{
				//if (logfp) { fprintf(logfp, "ffmpeg_params mode = %d \n", mode); fflush(logfp); }
				//if (logfp) { fprintf(logfp, "ffmpeg_params winWidth = %d, winHeight = %d \n", winWidth, winHeight); fflush(logfp); }
				//if (logfp) { fprintf(logfp, "ffmpeg_params width = %d ,height = %d \n", frame->width, frame->height); fflush(logfp); }
				//Y
				for (int i = 0; i < frame->height; i++)
				{
					memcpy(&outBuf[0][i * frame->width], &frame->data[0][i * frame->linesize[0]], frame->width);
				}
				//U
				for (int i = 0; i < frame->height; i++)
				{
					if (!(i & 1))
					{
						memcpy(&outBuf[1][(i >> 1) * (frame->width >> 1)], &frame->data[1][(i >> 1) * frame->linesize[1]], frame->width >> 1);
					}
				}
				//V
				for (int i = 0; i < frame->height; i++)
				{
					if (!(i & 1))
					{
						memcpy(&outBuf[2][(i >> 1) * (frame->width >> 1)], &frame->data[2][(i >> 1) * frame->linesize[2]], frame->width >> 1);
					}
				}
			}
			else
			{
				float left = 0.0;
				float top = 0.0;
				float right = 1.0;
				float bottom = 1.0;
				int width = frame->width;
				int height = frame->height;
				if (mode == kStretchMatchWidth)
				{
					int BackWidth = winWidth;// 640;
					int BackHeight = winHeight;// 480;
					int ImgWidth = width;
					int ImgHeight = height;
					float factor = (float)BackWidth / ImgWidth;
					float BackHeight2 = ImgHeight * factor;
					top = (BackHeight - BackHeight2) / BackHeight;
					top /= 2.0;
					bottom -= top;
				}
				else if (mode == kStretchMatchHeight)
				{
					int BackWidth = winWidth;// 640;
					int BackHeight = winHeight;// 480;
					int ImgWidth = width;
					int ImgHeight = height;
					float factor = (float)BackHeight / ImgHeight;
					float BackWidth2 = ImgWidth * factor;
					left = (BackWidth - BackWidth2) / BackWidth;
					left /= 2.0;
					right -= left;
				}
				else if (mode == kStretchToInsideEdge)
				{
					int BackWidth = winWidth;// 640;
					int BackHeight = winHeight;// 480;
					int ImgWidth = width;
					int ImgHeight = height;
					float factor0 = (float)BackWidth / ImgWidth;
					float factor1 = (float)BackHeight / ImgHeight;
					if (factor0 < factor1)
					{
						float BackHeight2 = ImgHeight * factor0;
						top = (BackHeight - BackHeight2) / BackHeight;
						top /= 2.0;
						bottom -= top;
					}
					else
					{
						float BackWidth2 = ImgWidth * factor1;
						left = (BackWidth - BackWidth2) / BackWidth;
						left /= 2.0;
						right -= left;
					}
				}
				orgX = (int)(left * width);
				orgY = (int)(top * height);
				int dstWidth = (int)((right - left) * width);
				int dstHeight = (int)((bottom - top) * height);
				if (video->scale == NULL)
                {
					video->scale = (ffmpegScale *)ffmpeg_create(video->scale);
                }
                else if(reset)
                {
                    ffmpeg_close(video->scale);
                    video->scale = NULL;
					video->scale = (ffmpegScale *)ffmpeg_create(video->scale);
#if 1
                    g_critsect->Enter();
#else
					if (g_cb)
					{
						g_cb(&g_critsect, 0);
					}
#endif
                    mst->reset = 0;
#if 1
                    g_critsect->Leave();
#else
					if (g_cb)
					{
						g_cb(&g_critsect, 1);
					}
#endif
                }
				if (video->scale)
				{
					void *scale = video->scale;
					unsigned char *sY = (unsigned char *)&frame->data[0][0];
					unsigned char *sU = (unsigned char *)&frame->data[1][0];
					unsigned char *sV = (unsigned char *)&frame->data[2][0];
					unsigned char *dY = (unsigned char *)&outBuf[0][orgY * width + orgX];
					unsigned char *dU = (unsigned char *)&outBuf[1][(orgY >> 1) * (width >> 1) + (orgX >> 1)];
					unsigned char *dV = (unsigned char *)&outBuf[2][(orgY >> 1) * (width >> 1) + (orgX >> 1)];
                    //unsigned char *dY = (unsigned char *)&outBuf[0][0];//[orgY * width + orgX];
                    //unsigned char *dU = (unsigned char *)&outBuf[1][0];//[0];//[(orgY * (width >> 1) + orgX) >> 1];
                    //unsigned char *dV = (unsigned char *)&outBuf[2][0];//[(orgY * (width >> 1) + orgX) >> 1];
/*
					if (logfp){
						fprintf(logfp, "ffmpeg_params orgX = %d, orgY = %d \n", orgX, orgY); fflush(logfp);
						fprintf(logfp, "ffmpeg_params winWidth = %d, winHeight = %d \n", winWidth, winHeight); fflush(logfp);
						fprintf(logfp, "ffmpeg_params width = %d ,height = %d \n", width, height); fflush(logfp);
						fprintf(logfp, "ffmpeg_params stride = %d \n", frame->linesize[0]); fflush(logfp);
						fprintf(logfp, "ffmpeg_params dstWidth = %d dstHeight= %d \n", dstWidth, dstHeight); fflush(logfp);
					}
*/
					ffmpeg_params(scale, AV_PIX_FMT_YUV420P, sY, sU, sV, width, frame->linesize[0], height, 0);
					//ffmpeg_params(scale, AV_PIX_FMT_YUV420P, dY, dU, dV, dstWidth, dstWidth, dstHeight, 1);
					ffmpeg_params(scale, AV_PIX_FMT_YUV420P, dY, dU, dV, dstWidth, width, dstHeight, 1);
					ffmpeg_scaleInit(scale);
					ffmpeg_scaler(scale);

				}
			}
#if 0
			//{
				int ret1 = 0;
				char args[256] = "";
				char cTime[64] = "";
				GetSysTime(cTime);
				char context[255] = "drawtext=fontfile=arial.ttf:fontcolor=white:fontsize=50:y=60:text=";//
				//char context[255] = "drawtext=fontfile=arial.ttf:fontcolor=white:fontsize=50:y=60:text='%{localtime\:%T}'";
				strcat(context, "\'");
				strcat(context, cTime);
				strcat(context, "\'");

				//*frame_rate = c->framerate.num / c->framerate.den;
				//*frame_rate = c->time_base.den / c->time_base.num;
				static FILE *fp_out = NULL;
				//sprintf(args, "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d", frame->width, frame->height, c->pix_fmt, c->framerate.den, c->framerate.num, 1, 1);
				sprintf(args, "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d", frame->width, frame->height, c->pix_fmt, 1, 1, 1, 1);
				//’%m/%d/%y %H\:%M\:%S'
				if (video->av_filter == NULL)
				{
					video->av_filter = FilterInit(video->av_filter, args, context);
					/*fp_out = fopen("E://works/test/output.yuv", "wb+");
					if (fp_out == NULL){
						printf("Error open output file.\n");
						return -1;
					}*/
					FilterSetParams(video->av_filter, c->pix_fmt, frame->width, frame->height, frame->width, outBuf[0], outBuf[1], outBuf[2], 0);
					FilterSetParams(video->av_filter, c->pix_fmt, frame->width, frame->height, frame->width, outBuf[0], outBuf[1], outBuf[2], 1);
				}
				else
				{
///					ret1 = FilterRenew(video->av_filter, context);
					FilterSetParams(video->av_filter, c->pix_fmt, frame->width, frame->height, frame->width, outBuf[0], outBuf[1], outBuf[2], 0);
				}
				/*FilterSetParams(video->av_filter, c->pix_fmt, frame->width, frame->height, frame->width, outBuf[0], outBuf[1], outBuf[2], 0);
				FilterSetParams(video->av_filter, c->pix_fmt, frame->width, frame->height, frame->width, outBuf[0], outBuf[1], outBuf[2], 1);*/
				ret1 = FilterProcess(video->av_filter, fp_out);
			//}
#elif defined(GXH_OSD)
			char cTime[64] = "";
			//GetSysTime(cTime);
			FF_GET_TIME2(0, cTime, 800 * 1000);
			char context[255] = "";
			strcat(context, cTime);
			sprintf(&context[strlen(context)], "  %d", frameNumber);
			sprintf(&context[strlen(context)], "  %dx", (pkt.size >> 3));
			
			if (video->av_filter == NULL)
			{
				video->av_filter = FontInit((void *)video->av_filter);
			}
			FontOsdProcess((void *)video->av_filter, (unsigned char **)outBuf, frame->width, frame->height, 0, 26, 0, 4, context);
#endif
#if 1
			//video->debug_flag = 0;
			if(!video->ofp && video->debug_flag)
            {
#ifdef _WIN32
				video->ofp = fopen("e://works//test//video-d.yuv", "wb");
#elif defined(__ANDROID__)
				video->ofp = fopen("/sdcard/video-d.yuv", "wb");
#elif defined(WEBRTC_IOS)
				char dirName[256] = "video-d.yuv";
				char *pName = MAKE_FILE_NAME(dirName);//makePreferencesFilename("webrtc_log.txt");
				video->ofp = fopen(pName, "wb");
				free(pName);
#else
#endif
            }
			if(video->ofp)
			{
				fwrite(outBuf[0],1,frame->width * frame->height, video->ofp);
				fwrite(outBuf[1],1,((frame->width * frame->height) >> 2), video->ofp);
				fwrite(outBuf[2],1,((frame->width * frame->height) >> 2), video->ofp);				
			}			
#endif
		}
		//{
		//	time2 = timeGetTime();
		//	diff = (int)(time2 - time1);
		//	if(diff > 40)
		//		printf("b:time = %d \n", diff);
		//}
		if (pkt.data)
		{
			pkt.size -= ret;
			pkt.data += ret;
		}
	}

	return ret;
}
static int audio_encode2(MediaStream *mst, CoreStream *audio, char * inBuf[3], int len, char *outBuf[3], int oLen, int &frame_type)
{
	int ret = 0;
	//AVFormatContext *fmtctx = audio->mux->fmtctx;
	//AVStream *audio_st = audio->st;
	AVCodec *audio_codec = audio->codec; 
	//AVFrame* frame = audio->frame;
	AVFrame* frame;
	char sdp[2048];

	// encode the image 
	AVPacket pkt;
	int got_output = 0;
	AVCodecContext *c = audio->codecCtx;

	//write_audio_frame(audio->oc, &audio->oSt);
	int dst_nb_samples;

	if (audio->mux && audio->mux->WriteHead == 0 && audio->mux->fmtctx && !audio->FileOrStream)
	{
		return ret;
	}
     //c = audio_st->codec;
	if(audio->frame->data[0] == NULL)
	{
		ret = av_frame_get_buffer(audio->frame, 0);
	}
	if(audio->tmp_frame->data[0] == NULL)
	{
		ret = av_frame_get_buffer(audio->tmp_frame, 0);
	}
	frame = audio->tmp_frame;
	if (audio->mux->RecordStatus == kRecordInSet)
	{
		memset(inBuf[0], 0, len);
	}
	memcpy(frame->data[0], inBuf[0], len);
	if (audio->mux->RecordStatus && strlen(mst->jpgFile[kWavFile]))
	{
		if (audio->wave_stream == NULL)
		{
			audio->wave_stream = FF_WAV_CREATE(audio->wave_stream);
			FF_WAV_INIT(audio->wave_stream, (const char *)mst->jpgFile[kWavFile], 1, frame->sample_rate, 16);
		}
		FF_WAV_WRITE(audio->wave_stream, (unsigned char *)inBuf[0], len, frame->channels);
	}
	//if(1)
	if(!inBuf)
	{
		frame = get_audio_frame(mst, audio);//audio->tmp_frame;
	}
	else
	{
#if 0
		//audio->debug_flag = 0;
		//static FILE* fp = NULL;
		if(!audio->ifp  && audio->debug_flag) audio->ifp = fopen("e://works//test//iaudio.pcm","wb");
		if(audio->ifp)
		{
			int wsize = fwrite(inBuf[0], 1, len, audio->ifp);
			//if(wsize > 0)
			//{
			//	printf("");
			//}
		}
#endif
		//frame = audio->tmp_frame;
		//memcpy(frame->data[0], inBuf[0], len);
		//
		//frame->pts = audio->next_pts;
		//audio->next_pts  += frame->nb_samples;
	}

    if (frame) {
		if (audio->resample == NULL)
		{
			audio->resample = (ffmpegResample *)ResampleCreate(audio->resample);
		}
		ResampleParams((void *)audio->resample, frame->data, frame->format, frame->channels, frame->nb_samples, frame->sample_rate, 0);
		ResampleParams((void *)audio->resample, audio->frame->data, frame->format, frame->channels, frame->nb_samples, frame->sample_rate, 1);
		ResampleInit((void *)audio->resample);
		ret = Resample((void *)audio->resample);
#if 0
		dst_nb_samples = av_rescale_rnd(swr_get_delay(audio->resample->swr_ctx, c->sample_rate) + frame->nb_samples,
			c->sample_rate, c->sample_rate, AV_ROUND_UP);
		av_assert0(dst_nb_samples == frame->nb_samples);
		ret = av_frame_make_writable(audio->frame);
		if (ret < 0)    exit(1);
		// convert to destination format 
		ret = swr_convert(audio->resample->swr_ctx,audio->frame->data, dst_nb_samples,(const uint8_t **)frame->data, frame->nb_samples);
#endif
		
        if (ret < 0) {
            fprintf(stderr, "Error while converting\n");
            exit(1);
        }
        frame = audio->frame;
		AVRational q = {1, c->sample_rate};
        frame->pts = av_rescale_q(c->frame_number * frame->nb_samples, q, c->time_base);
        //audio->samples_count += dst_nb_samples;
    }
	av_init_packet(&pkt);
	pkt.data = NULL;    // packet data will be allocated by the encoder
	pkt.size = 0;
#ifndef FFMSTREAM
	pkt.pts = 0;//c->frame_number * frame->nb_samples;//AV_NOPTS_VALUE;
	pkt.dts = 0;//c->frame_number * frame->nb_samples;//AV_NOPTS_VALUE;
#endif

#ifdef SYS_TIMER
	//if(!audio->FileOrStream)
	//{
	//	__int64 time_stamp = webrtc::TickTime::MillisecondTimestamp();
	//	frame->pts = c->frame_number ? time_stamp : 0;
	//}
#endif
    ret = avcodec_encode_audio2(c, &pkt, frame, &got_output);
    if (ret < 0) {
        //fprintf(stderr, "Error encoding audio frame: %s\n", av_err2str(ret));
        exit(1);
    }

    if (got_output) {
		//pkt.stream_index = audio_st->index;
		if (pkt.pts != AV_NOPTS_VALUE )
		{
//			pkt.pts = av_rescale_q(pkt.pts,audio_st->codec->time_base, audio_st->time_base);
		}
		if(pkt.dts !=AV_NOPTS_VALUE )
		{
//			pkt.dts = av_rescale_q(pkt.dts,audio_st->codec->time_base, audio_st->time_base);
		}
		if(audio->sdp_flag)
		{
			//memcpy(outBuf[0], c->extradata, c->extradata_size);
			memcpy(&outBuf[0][c->extradata_size], pkt.data, pkt.size);
#if 1
			char *bits = outBuf[0];
			int sample_index = 0 , channel = 0;
			char temp = 0;
			int length = 7 + pkt.size;
			AVCodecContext *audioCodecCtx = audio->mux->fmtctx->streams[audio->st->index]->codec;
			sample_index = (audioCodecCtx->extradata[0] & 0x07) << 1;
			temp = (audioCodecCtx->extradata[1]&0x80);
			switch(audioCodecCtx->sample_rate)
			{
			case 44100:
				{
					sample_index = 0x7;
				}break;
			default:
				{
					sample_index = sample_index + (temp>>7);
				}break;
			}
			channel = ((audioCodecCtx->extradata[1] - temp) & 0xff) >> 3;
			bits[0] = 0xff;
			bits[1] = 0xf1;
			bits[2] = 0x40 | (sample_index<<2) | (channel>>2);
			bits[3] = ((channel&0x3)<<6) | (length >>11);
			bits[4] = (length>>3) & 0xff;
			bits[5] = ((length<<5) & 0xff) | 0x1f;
			bits[6] = 0xfc;
#endif
			ret = pkt.size + c->extradata_size;
		}
		else
		{
			memcpy(outBuf[0], pkt.data, pkt.size);
			ret = pkt.size;
		}
		audio->pkt = pkt;
		//audio->pkt2 = pkt;
#if 1
		//static FILE* fp = NULL;
		if (!audio->ofp && audio->debug_flag)
		{
#ifdef _WIN32
			audio->ofp = fopen("e://works//test//audio.aac", "wb");
#elif defined(__ANDROID__)
			audio->ofp = fopen("/sdcard/audio.aac", "wb");
#elif defined(WEBRTC_IOS)
			char dirName[256] = "audio.aac";
			char *pName = MAKE_FILE_NAME(dirName);//makePreferencesFilename("webrtc_log.txt");
			audio->ofp = fopen(pName, "wb");
			free(pName);
#else
			audio->ofp = fopen("audio.aac", "wb");
#endif
		}
		if (audio->ofp)
		{
			fwrite(outBuf[0], 1, ret, audio->ofp);
		}
#endif
#if 0
		if(audio->mux->fmtctx)
		{
			pkt.stream_index = audio->st->index;
			//
#ifndef SYS_TIMER
			//__int64 time_stamp = (c->frame_number - 1) * frame->nb_samples;
			av_packet_rescale_ts(&pkt, c->time_base, audio->st->time_base);
#else
			
			pkt.pts = time_stamp;
			pkt.dts = time_stamp;
#endif
			
			pkt.flags = 0;
			pkt.duration;
			int ret1 = 0;
			//audio->pkt = pkt;
			ret1 = av_interleaved_write_frame(audio->mux->fmtctx,&pkt);
			//ret1 = av_write_frame(audio->mux->fmtctx,&pkt);
			if(ret1 < 0)
			{
				printf("error \n");
			}
		}
#endif
    }
	
	return ret;
}


void IIISpeexAECPlayPort(void **hnd, char *playBuf, int frameBytes)
{
#ifdef _WIN32
#ifndef SPEEXECHO_TEST
#if 1
	g_echo_cri->Enter();
#else
	if (g_cb)
	{
		g_cb(&g_echo_cri, 0);
	}
#endif
	IISpeexAECPlayPort(NULL, playBuf, frameBytes);
#if 1
	g_echo_cri->Leave();
#else
	if (g_cb)
	{
		g_cb(&g_echo_cri, 1);
	}
#endif

#endif
#endif
}
int IIISpeexAECMicPort(void **hnd, char *micBuf, int frameBytes)
{
	int ret = 1;
#ifdef _WIN32
#ifndef SPEEXECHO_TEST
#if 1
	__int64 time0 = webrtc::TickTime::MillisecondTimestamp();
	g_echo_cri->Enter();
#else
	if (g_cb)
	{
		g_cb(&g_echo_cri, 0);
	}
#endif
	if (IISpeexAECMicPort(NULL, micBuf, frameBytes) == 0)
	{
		g_echo_cri->Leave();
		return 0;
	}

#if 1
	g_echo_cri->Leave();
#else
	if (g_cb)
	{
		g_cb(&g_echo_cri, 1);
	}
#endif

#endif
#endif
	return ret;
}
static int audio_encode(MediaStream *mst, CoreStream *audio, char * inBuf[3], int len, char *outBuf[3], int oLen, int &frame_type)
{
	int ret = 0;
	static int test_cnt = 0;

	if (IIISpeexAECMicPort(NULL, inBuf[0], len) == 0)
	{
		return ret;
	}
	ret = audio_encode2(mst, audio, inBuf, len, outBuf, oLen, frame_type);

	return ret;
}
int audio_decode(MediaStream *mst, CoreStream *audio, char * inBuf[3], int len, char *outBuf[3], int oLen, int &frame_type)
{
	int ret = 0;
	ret = audio_decode2(mst, audio, inBuf, len, outBuf, oLen, frame_type);
	return ret;
}
int audio_decode2(MediaStream *mst, CoreStream *audio, char * inBuf[3], int len, char *outBuf[3], int oLen, int &frame_type)
{
	//AVFormatContext *fmtctx = audio->mux->fmtctx;
	//AVStream *audio_st = audio->st;
	//AVCodec *audio_codec = audio->codec; 
	AVCodecContext *c = audio->codecCtx;
	AVFrame* frame = audio->frame;
	char sdp[2048];
	AVPacket pkt;
	int ret = 0, got_frame = 0;
	int out_size = 0;
#ifdef GLOB_DEC
	//audio_codec = g_audio_codec;
	c = g_audio_c;
#endif
	av_init_packet(&pkt);
	pkt.size = len;
	pkt.data = (uint8_t *)inBuf[0];
	//pkt.flags = AV_PKT_FLAG_KEY;
#if 0
	if(audio->sdp_flag)
	{
		char bits[7] = {0};
		int sample_index = 0 , channel = 0;
		char temp = 0;
		int length = 7 + pkt.size;
		AVCodecContext *audioCodecCtx = audio->mux->fmtctx->streams[0]->codec;
		sample_index = (audioCodecCtx->extradata[0] & 0x07) << 1;
		temp = (audioCodecCtx->extradata[1]&0x80);
		switch(audioCodecCtx->sample_rate)
		{
		case 44100:
			{
				sample_index = 0x7;
			}break;
		default:
			{
				sample_index = sample_index + (temp>>7);
			}break;
		}
		channel = ((audioCodecCtx->extradata[1] - temp) & 0xff) >> 3;
		bits[0] = 0xff;
		bits[1] = 0xf1;
		bits[2] = 0x40 | (sample_index<<2) | (channel>>2);
		bits[3] = ((channel&0x3)<<6) | (length >>11);
		bits[4] = (length>>3) & 0xff;
		bits[5] = ((length<<5) & 0xff) | 0x1f;
		bits[6] = 0xfc;
		memmove(&pkt.data[7], pkt.data, pkt.size);
		memcpy(pkt.data, bits, 7);
		pkt.size += 7;
	}
#endif
	//typedef struct RTPDemuxContext RTPDemuxContext;
	int offset = 0;
	while (pkt.size > 0)
	{
		//int16_t out_buf[1024 << 4];
		//int out_size = 0;
		
		int read_size = 
		ret = avcodec_decode_audio4(c, frame, &got_frame, &pkt);
		//int ret = avcodec_decode_audio3(c, out_buf, &out_size, &pkt);
		if (ret < 0)
		{
			return -2;
		}

		if (got_frame)
		{
			int data_size = av_get_bytes_per_sample(c->sample_fmt);
			size_t unpadded_linesize = audio->frame->nb_samples * av_get_bytes_per_sample((AVSampleFormat)frame->format);
			//int outSize = audio_swr_resampling_audio(mux->swr_ctx, mux->urlStr.apCodecCtx, mux->audioFrame, &mux->urlStr.outbuf,&dst);
			//int outSize = swr_convert(swr_ctx,targetData,audioFrame->nb_samples,audioFrame->extended_data,audioFrame->nb_samples);
			// convert to destination format 
			//audio->tmp_frame->data[0] = (uint8_t *)&outBuf[0][offset];
			audio->tmp_frame->data[0] = (uint8_t *)&audio->buf[audio->pos];// outBuf[0][offset];
			//audio->tmp_frame->linesize[0] = frame->nb_samples << 2;
#if 1
			if (audio->resample == NULL)
			{
				audio->resample = (ffmpegResample *)ResampleCreate(audio->resample);
			}
			//frame->extended_data;
			//frame->channels = 1;//test
			ResampleParams((void *)audio->resample, frame->data, frame->format, frame->channels, frame->nb_samples, frame->sample_rate, 0);
			///ResampleParams((void *)audio->resample, audio->tmp_frame->data, AV_SAMPLE_FMT_S16, frame->channels, frame->nb_samples, frame->sample_rate, 1);
			int dst_nb_samples = (48000 * frame->nb_samples) / frame->sample_rate;// 44100;
			///ResampleParams((void *)audio->resample, audio->tmp_frame->data, AV_SAMPLE_FMT_S16, frame->channels, dst_nb_samples, 48000, 1);
			ResampleParams((void *)audio->resample, audio->tmp_frame->data, AV_SAMPLE_FMT_S16, 2, dst_nb_samples, 48000, 1);
			ResampleInit((void *)audio->resample);
			ret = Resample((void *)audio->resample);

			//ret = swr_convert(audio->resample->swr_ctx,audio->tmp_frame->data, frame->nb_samples,(const uint8_t **)frame->data, frame->nb_samples);
#endif
			ret = data_size * ret;//unpadded_linesize;

#if 0
			if(!audio->ifp)
			{
				audio->ifp = fopen("e://works//video//20063.wav", "rb");
			}
			if(audio->ifp)
			{
				int rsize = fread(outBuf[0], 1, 8192, audio->ifp);
				if(rsize != 8192)
				{
					fseek( audio->ifp, 0, SEEK_SET );
					rsize = fread(outBuf[0], 1, 8192, audio->ifp);
				}
			}
#endif

#if 0
			int16_t *out_data = (int16_t *)inBuf;
			float *p[2];
			p[0] = (float *)frame->data[0];
			p[1] = (float *)frame->data[1];
			for (int i = 0; i < frame->nb_samples; i++)
			{
				for(int j = 0; j < frame->channels; j++)
				{
					//*out_data = (int16_t)(p[j][i] >> 16);
					int16_t value = (int)(p[j][i] * 32767);
					*out_data = value;
					out_data++;
				}
			}
			static FILE* fp = NULL;
			if(!fp) fp = fopen("audio.pcm","wb");
			if(fp)
			{
				fwrite(inBuf, 1, (1024 << 3), fp);
			}
#endif
#if 0
			static FILE* fp = NULL;
			if(!fp) fp = fopen("audio.pcm","wb");
			if(fp)
			{
				int data_size = av_get_bytes_per_sample(c->sample_fmt);
				if (data_size < 0) {
					// This should not occur, checking just for paranoia 
					fprintf(stderr, "Failed to calculate data size\n");
					exit(1);
				}
				for (int i=0; i<frame->nb_samples; i++)
					for (int ch=0; ch<c->channels; ch++)
						fwrite(frame->data[ch] + data_size*i, 1, data_size, fp);

			}			
#endif
			audio->pos += ret;
			if (audio->pos >= 8192)
			{
#if 1
				audio->debug_flag = 0;
				//static FILE* fp = NULL;
				if (!audio->ofp  && audio->debug_flag)
				{
#ifdef _WIN32
					audio->ofp = fopen("e://works//test//audio-d.pcm", "wb");
#elif defined(__ANDROID__)
					audio->ofp = fopen("/sdcard/audio-d.pcm", "wb");
#elif defined(WEBRTC_IOS)
					char dirName[256] = "audio-d.pcm";
					char *pName = MAKE_FILE_NAME(dirName);//makePreferencesFilename("webrtc_log.txt");
					audio->ofp = fopen(pName, "wb");
					free(pName);
#else
					audio->ofp = fopen("audio-d.pcm", "wb");
#endif
				}
				if (audio->ofp)
				{
					int wsize = fwrite(audio->buf, 1, 8192, audio->ofp);
					
					//if(wsize > 0)
					//{
					//	printf("");
					//}
				}
#endif
				memcpy(outBuf[0], audio->buf, 8192);
				audio->pos -= 8192;
				if (audio->pos)
				{
					memmove(audio->buf, &audio->buf[8192], audio->pos);
				}
				out_size = 8192;
			}
		}
		offset += ret;
		if (pkt.data)
		{
			pkt.size -= read_size;
			pkt.data += read_size;
///			memmove(pkt.data, pkt.data + read_size, pkt.size);
			//pkt.data += read_size;
			
			//pkt.dts =
			//pkt.pts = AV_NOPTS_VALUE;
			//if (pkt.size < AUDIO_REFILL_THRESH) 
			//{
			//	// Refill the input buffer, to avoid trying to decode
			//	// incomplete frames. Instead of this, one could also use
			//	// a parser, or use a proper container format through
			//	// libavformat. 
			//	memmove(inbuf, pkt.data, pkt.size);
			//	pkt.data = inbuf;
			//	len = fread(pkt.data + pkt.size, 1,	AUDIO_INBUF_SIZE - pkt.size, f);
			//	if (len > 0)
			//		pkt.size += len;
			//}
		}
	}

	return out_size;// offset;// ret;
}
/////////////////////////////////////////////////////////////////////////////////
static int interrupt_cb(void *hnd)
{
	AVMuxer *mux = (AVMuxer *)hnd;
	//	mux->read_critsect->Enter();
	//__int64 timeout = 100 * 1000000;
	//LARGE_INTEGER ptime;
	//#ifdef _WIN32
	//	QueryPerformanceCounter(&mux->ptime);
	//	if (mux->ptime.QuadPart > mux->timeout && mux->timeout)  //timeout arrived?这个就是超时的返回
	//		return 1;
	//#endif
#if 1
	int64_t time1 = webrtc::TickTime::MillisecondTimestamp();
#else
	int64_t time1 = get_sys_time(); //IGetTime();
	if (g_cb)
	{
		g_cb((void **)&time1, 4);
	}
#endif
	int difftime = (int)(time1 - mux->time0);
	if (difftime > mux->timeout && mux->timeout)
	{

		//mux->time0 = time1;
		//mux->read_critsect->Leave();
		printf("interrupt_cb: over time \n");
        if (logfp) { fprintf(logfp, "interrupt_cb: difftime= %d \n", difftime); fflush(logfp); }
		return 1;
	}
	//	mux->read_critsect->Leave();
	//return 0 for no timeout
	return 0;
}
static AVIOInterruptCB int_cb = { interrupt_cb, NULL };
int WriteTail(AVFormatContext *oc)
{
	if(!oc)
		return -1;
	return av_write_trailer(oc);
}
int WriteHeader(AVFormatContext *oc)
{
	// Write the stream header, if any. 
	int ret = 1;
#ifdef AVIOTEST
	if(oc->pb)
	{
		ret = avformat_write_header(oc, NULL);//写视频文件的文件头，//time_base在此生成
	}
#endif
	return ret;
}
int WriteStart(CoreStream *pWrite, int splitLen)
{
#if 1
	pWrite->mux->time0 = webrtc::TickTime::MillisecondTimestamp();
	pWrite->mux->start_time = pWrite->mux->time0;
#else
	pWrite->mux->time0 = get_sys_time();
	if (g_cb)
	{
		g_cb((void **)&pWrite->mux->time0, 4);
	}
	pWrite->mux->start_time = pWrite->mux->time0;
#endif
	pWrite->mux->WriteAudio = 0;
	pWrite->mux->split_len = splitLen <= 0 ? SPLIT_LEN : splitLen;
	pWrite->mux->WriteHead = WriteHeader(pWrite->mux->fmtctx) == 0;
	//printf("status = %x \n", (unsigned int)&pWrite->mux);
	return pWrite->mux->WriteHead;
}
int WriteStop(AVFormatContext **oc)
{
	int ret = 0;
	ret = WriteTail(*oc);
	avio_close((*oc)->pb);
	(*oc)->pb = NULL;
	return ret;
}
int WriteOpen(MediaStream *mst, CoreStream *pWrite)
{
	int ret = 0;
	AVFormatContext **oc = &pWrite->mux->fmtctx;
	AVOutputFormat *fmt = (*oc)->oformat;
	char *filename = pWrite->outFile;
	if (logfp) { fprintf(logfp, "WriteOpen 0:%d \n", ret); fflush(logfp); }
#ifdef AVIOTEST
	char new_name[255] = "";
	if (pWrite->FileOrStream && strcmp(pWrite->fileType, "ffm"))
	{
		renew_filename2(pWrite, new_name);
		filename = new_name;
	}
	printf("filename= %s \n", filename);
		//char new_name[255] = "";
	if (!(fmt->flags & AVFMT_NOFILE))
	{
		int status = 0;
#if 1
		if (g_critsect)
#endif
		{
#if 1
			g_critsect->Enter();
#else
			if (g_cb)
			{
				g_cb(&g_critsect, 0);
			}
#endif
			ffmpegFactory *pFactory = (ffmpegFactory *)g_ffFactory;
			status = pFactory->status;
#if 1
			g_critsect->Leave();
#else
			if (g_cb)
			{
				g_cb(&g_critsect, 1);
			}
#endif
		}
		if (!status)
		{
			return -1;
		}
		if(filename && *oc && (*oc)->pb == NULL)
		{
			if (logfp) { fprintf(logfp, "WriteOpen 1:%d \n", ret); fflush(logfp); }
			
			if (!strncmp(filename, "http", strlen("http")))
			{
				pWrite->mux->timeout = 5000;
#if 1
				pWrite->mux->time0 = webrtc::TickTime::MillisecondTimestamp();
#else
				pWrite->mux->time0 = get_sys_time(); //IGetTime();
				if (g_cb)
				{
					g_cb((void **)&pWrite->mux->time0, 4);
				}
#endif
				//pWrite->mux->fmtctx = avformat_alloc_context();
				pWrite->mux->fmtctx->interrupt_callback.callback = interrupt_cb;//--------注册回调函数
				pWrite->mux->fmtctx->interrupt_callback.opaque = (void *)pWrite->mux;
				int_cb.opaque = (void *)pWrite->mux;
				//ret = av_dict_set(&pWrite->mux->opt, "chunked_post", "0", 0);
				//ret = av_dict_set(&pWrite->mux->opt, "metadata", "creation_time=now", 0);

				AVFormatContext *ic = avformat_alloc_context();
				ic->interrupt_callback = int_cb;
				ret = avformat_open_input(&ic, filename, NULL, NULL);
				//(*oc)->iformat = ic->iformat;
				//(*oc)->pb = ic->pb;
				avformat_close_input(&ic);
				if (ret >= 0)
				{
					ret = avio_open2(&(*oc)->pb, filename, AVIO_FLAG_WRITE, &int_cb, &pWrite->mux->opt);
					printf("avio_open2 = %d \n", ret);
#if 0
					//
					//ret = av_dict_set(&pWrite->mux->opt, "creation_time", NULL, 0);
					//ret = av_dict_set(&pWrite->mux->opt, "encoder", NULL, 0);
					//ret = av_dict_set(&pWrite->mux->opt, "rotate", NULL, 0);
					char now_time[256] = "";
					get_date(now_time);
					ret = av_dict_set(&pWrite->mux->opt, "creation_time", now_time, 0);
#endif
				}
				else
				{
					printf("avformat_open_input = %d \n", ret);
				}
			}
			else
			{
			do
			{
				ret = avio_open(&(*oc)->pb, filename, AVIO_FLAG_WRITE);
				//ret = avio_open2(&mux->urlStr.ofmt_ctx->pb, mux->urlStr.filename, AVIO_FLAG_WRITE, &mux->urlStr.ofmt_ctx->interrupt_callback, dict);
				if (logfp) { fprintf(logfp, "avio_open:ret = %d \n", ret); fflush(logfp); }
				if(ret < 0)
				{
					printf("avio_open failed : %s \n",filename);
					ret = -1;
				}
			}while(ret < 0);
		}
	}
	}
#endif
	return ret;
}
int WriteInit(MediaStream *mst, CoreStream *pWrite)
{
	int ret = 0;
	char *filename = pWrite->outFile;
	AVFormatContext **oc = &pWrite->mux->fmtctx;
	pWrite->mux->ofmt = av_guess_format(pWrite->fileType, NULL, NULL);
	//avformat_alloc_output_context2(oc, NULL, NULL, filename);
	avformat_alloc_output_context2(oc, NULL, pWrite->fileType, filename);//gxh_20160728
	if (!*oc) 
	{
		//AVOutputFormat *ofmt = av_guess_format(pWrite->fileType, NULL, NULL);
		//avformat_alloc_output_context2(&mux->stream[i].oc, NULL, "flv", mux->stream[i].filename);///"mpeg"
		avformat_alloc_output_context2(oc, NULL, pWrite->fileType, filename);
		if(!*oc)
		{
			printf("stream: Could not deduce output format from file extension: using MPEG.\n");
			return -1;
		}
	}
	(*oc)->flags |= AVFMT_FLAG_FLUSH_PACKETS;
	return ret;
}
#if 0
static int interrupt_cb(void *hnd)
{
	AVMuxer *mux = (AVMuxer *)hnd;
//	mux->read_critsect->Enter();
	//__int64 timeout = 100 * 1000000;
	//LARGE_INTEGER ptime;
//#ifdef _WIN32
//	QueryPerformanceCounter(&mux->ptime);
//	if (mux->ptime.QuadPart > mux->timeout && mux->timeout)  //timeout arrived?这个就是超时的返回
//		return 1;
//#endif
	int64_t time1 = get_sys_time(); //IGetTime();
	if (g_cb)
	{
		g_cb((void **)&time1, 4);
	}
	int difftime = (int)(time1 - mux->time0);
	if (difftime > mux->timeout && mux->timeout)
	{
		
		//mux->time0 = time1;
		//mux->read_critsect->Leave();
		return 1;
	}
//	mux->read_critsect->Leave();
	//return 0 for no timeout
	return 0;
}
static const AVIOInterruptCB int_cb = { interrupt_cb, NULL };
#endif
int ReadInit(MediaStream *mst, CoreStream *pRead)
{
	int ret = 0;
	char szError[256] = {0};

	pRead->mux->timeout = 2000 * 5;
#if 1
        pRead->mux->time0 = IGetTime();
#else
	pRead->mux->time0 = get_sys_time(); //IGetTime();
	if (g_cb)
	{
		g_cb((void **)&pRead->mux->time0, 4);
	}
#endif
	pRead->mux->fmtctx = avformat_alloc_context();
	pRead->mux->fmtctx->interrupt_callback.callback = interrupt_cb;//--------注册回调函数
	pRead->mux->fmtctx->interrupt_callback.opaque = (void *)pRead->mux;

	//pRead->mux->fmtctx->flags |= AVFMT_FLAG_NONBLOCK;//非阻塞

	char option_key[]="probesize";
	//char option_value[] = "30000000";
	char option_value[] = "50000";//50kB

	char option_key2[]="analyzeduration";//"max_analyze_duration";
	char option_value2[] = "30000000";
	//char option_value2[]="50000";
	char option_key3[]="rtmp_transport";
	char option_value3[]="tcp";
	char option_key0[] = "max_delay";
	char option_value0[] = "100";// "500000";
	//char option_key5[] = "low_delay";
	//char option_value5[] = "1";
	char option_key5[] = "fflags";// for ffplay
	char option_value5[] = "nobuffer";// "true";
	//将avformat_find_stream_info阻塞时间从5秒增加到50秒
//	av_dict_set(&pRead->mux->opt, "re", "", 0);
	av_dict_set(&pRead->mux->opt,option_key,option_value,0);	
//	av_dict_set(&pRead->mux->opt,option_key2,option_value2,0);
	//ret = av_dict_set(&pRead->mux->opt, option_key0, option_value0, 0);
	//ret = av_dict_set(&pRead->mux->opt, option_key5, option_value5, 0);
	
	char filename[MAX_PATH] = "";

	int status = 0;
#if 1
	if (g_critsect)
#endif
	{
#if 1
		g_critsect->Enter();
#else
		if (g_cb)
		{
			g_cb(&g_critsect, 0);
		}
#endif
		ffmpegFactory *pFactory = (ffmpegFactory *)g_ffFactory;
		status = pFactory->status;
#if 1
		g_critsect->Leave();
#else
		if (g_cb)
		{
			g_cb(&g_critsect, 1);
		}
#endif
	}
	if (!status)
	{
		return -1;
	}
	strcpy(filename, pRead->inFile);
#if 1
	if (!strncmp(filename, "rtmp", strlen("rtmp")))
	{
		av_dict_set(&pRead->mux->opt, option_key3, option_value3, 0);
		//av_dict_set(&pRead->mux->opt, "timeout", "6", 0); // in secs

		strcat(filename, " live=1");
	}
	else if (!strncmp(filename, "rtsp", strlen("rtsp")))
	{
#if 1
		char option_key[] = "rtsp_transport";
		char option_value[] = "tcp";
		char option_key2[] = "max_delay";
		char option_value2[] = "5000000";

		av_dict_set(&pRead->mux->opt, option_key, option_value, 0);
		av_dict_set(&pRead->mux->opt, option_key2, option_value2, 0);
#else
		strcat(mux->urlStr.url, "?tcp");
#endif

	}
	else if (!strncmp(filename, "udp", strlen("udp")))
	{
		char * format = "mpegts";
		pRead->mux->ifmt = av_find_input_format(format);
		//mpegts udp://127.0.0.1:10000?pkt_size=1316
		strcat(filename, "?pkt_size=1316");
	}
	else
	{
		//av_dict_set(&pRead->mux->opt, "timeout", "6000", 0); // in ms
		//av_dict_set(&pRead->mux->opt, option_key, option_value, 0);

	}
#endif
    //LogOut("ReadInit 1:filename= %s \n", (void *)filename);
	if (logfp) { fprintf(logfp, "ReadInit 1:filename = %s \n", filename); fflush(logfp); }
	//ret = avformat_open_input(&pRead->mux->fmtctx, filename, pRead->mux->ifmt, &pRead->mux->opt);
	ret = avformat_open_input(&pRead->mux->fmtctx, filename, pRead->mux->ifmt, &pRead->mux->opt);
	if (logfp) { fprintf(logfp, "avformat_open_input:ret = %d \n", ret); fflush(logfp); }
	if (ret != 0)
	{
		if (pRead->mux->fmtctx)
		{
			avformat_close_input(&pRead->mux->fmtctx);
			pRead->mux->fmtctx = NULL;
		}
		if (pRead->mux->opt)
		{
			av_dict_free(&pRead->mux->opt);
			pRead->mux->opt = NULL;
		}
		av_strerror(ret, szError, 256);
		printf(szError);
		printf("\n");
		printf("Call avformat_open_input function failed!\n");

		return -1;
	}
	//pRead->mux->timeout = 0;
	if(avformat_find_stream_info(pRead->mux->fmtctx, &pRead->mux->opt) < 0)
	{
		if (pRead->mux->fmtctx)
		{
			avformat_close_input(&pRead->mux->fmtctx);
			pRead->mux->fmtctx = NULL;
		}
		if (pRead->mux->opt)
		{
			av_dict_free(&pRead->mux->opt);
			pRead->mux->opt = NULL;
		}
		return -1;
	}
#if 1
	if (logfp) { fprintf(logfp, "ReadInit 2: ret = %d \n", ret); fflush(logfp); }
	int stream_index = 0;
	// create stream
	if ((stream_index = av_find_best_stream(pRead->mux->fmtctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0)) >= 0)
	{
		AVStream* istream = pRead->mux->fmtctx->streams[stream_index];
		
	}
	if ((stream_index = av_find_best_stream(pRead->mux->fmtctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0)) >= 0)
	{
		AVStream* istream = pRead->mux->fmtctx->streams[stream_index];
	}
	if (logfp) { fprintf(logfp, "ReadInit 3: ret = %d \n", ret); fflush(logfp); }
	//for (int i = 0; i < pRead->mux->fmtctx->nb_streams && (pRead->mux->video_index < 0 || pRead->mux->audio_index < 0); i++)
	for (int i = 0; i < (int)pRead->mux->fmtctx->nb_streams; i++)
	{
		switch (pRead->mux->fmtctx->streams[i]->codec->codec_type)
		{
		case AVMEDIA_TYPE_VIDEO:
			pRead->mux->video_index = i;
			pRead->mux->fmtctx->streams[i]->discard = AVDISCARD_NONE;
			break;

		case AVMEDIA_TYPE_AUDIO:
			pRead->mux->audio_index = i;
			pRead->mux->fmtctx->streams[i]->discard = AVDISCARD_NONE;
			break;

		default:
			pRead->mux->fmtctx->streams[i]->discard = AVDISCARD_ALL;
			break;

		}
	}
	if (logfp) { fprintf(logfp, "ReadInit 4: ret = %d \n", ret); fflush(logfp); }
	if (pRead->mux->video_index != -1)
	{
		pRead->mux->vpCodecCtx = pRead->mux->fmtctx->streams[pRead->mux->video_index]->codec;
		pRead->mux->vpCodecCtx->thread_type = FF_THREAD_SLICE; //FF_THREAD_FRAME;//0
		AVCodecContext *c = pRead->mux->vpCodecCtx;
		AVStream* istream = pRead->mux->fmtctx->streams[pRead->mux->video_index];
		if (c->width > 0 && c->height > 0)
		{
			pRead->mux->video_codec = avcodec_find_decoder(pRead->mux->vpCodecCtx->codec_id);
		}
		else//if (pRead->mux->video_codec == NULL)
		{
			printf("Call avcodec_find_decoder function failed!\n");
			if (pRead->mux->fmtctx)
			{
				if (pRead->mux->vpCodecCtx)
				{
					avcodec_close(pRead->mux->vpCodecCtx);
					pRead->mux->vpCodecCtx = NULL;
				}
				if (pRead->mux->apCodecCtx)
				{
					avcodec_close(pRead->mux->apCodecCtx);
					pRead->mux->apCodecCtx = NULL;
				}

				if (pRead->mux->fmtctx->pb)
				{
					avio_close(pRead->mux->fmtctx->pb);
					pRead->mux->fmtctx->pb = NULL;
				}
				avformat_close_input(&pRead->mux->fmtctx);
				pRead->mux->fmtctx = NULL;
				if (pRead->mux->opt)
				{
					av_dict_free(&pRead->mux->opt);
					pRead->mux->opt = NULL;
				}
			}
			return -1;
		}

		if (avcodec_open2(pRead->mux->vpCodecCtx, pRead->mux->video_codec, &pRead->mux->opt) < 0)
		{
			printf("Call avcodec_open function failed !\n");
			if (pRead->mux->fmtctx)
			{
				if (pRead->mux->vpCodecCtx)
				{
					avcodec_close(pRead->mux->vpCodecCtx);
					pRead->mux->vpCodecCtx = NULL;
				}
				if (pRead->mux->apCodecCtx)
				{
					avcodec_close(pRead->mux->apCodecCtx);
					pRead->mux->apCodecCtx = NULL;
				}

				if (pRead->mux->fmtctx->pb)
				{
					avio_close(pRead->mux->fmtctx->pb);
					pRead->mux->fmtctx->pb = NULL;
				}
				avformat_close_input(&pRead->mux->fmtctx);
				pRead->mux->fmtctx = NULL;
				if (pRead->mux->opt)
				{
					av_dict_free(&pRead->mux->opt);
					pRead->mux->opt = NULL;
				}
			}
			return -1;
		}

		///mux->urlStr.pFrame = avcodec_alloc_frame();
		///mux->urlStr.pFrameRGB = avcodec_alloc_frame();
	}
	if (logfp) { fprintf(logfp, "ReadInit 5: ret = %d \n", ret); fflush(logfp); }
	if (pRead->mux->audio_index != -1)
	{
		pRead->mux->apCodecCtx = pRead->mux->fmtctx->streams[pRead->mux->audio_index]->codec;
		//pRead->mux->audio_codec = avcodec_find_decoder(pRead->mux->apCodecCtx->codec_id);
		AVCodecContext *c = pRead->mux->apCodecCtx;
		if (!(c->extradata && c->extradata_size <= 0))
		{
		pRead->mux->audio_codec = avcodec_find_decoder(pRead->mux->apCodecCtx->codec_id);
		}
		else//if (pRead->mux->video_codec == NULL)
		{
			printf("Call avcodec_find_decoder function failed!\n");
			if (pRead->mux->fmtctx)
			{
				if (pRead->mux->vpCodecCtx)
				{
					avcodec_close(pRead->mux->vpCodecCtx);
					pRead->mux->vpCodecCtx = NULL;
				}
				if (pRead->mux->apCodecCtx)
				{
					avcodec_close(pRead->mux->apCodecCtx);
					pRead->mux->apCodecCtx = NULL;
				}

				if (pRead->mux->fmtctx->pb)
				{
					avio_close(pRead->mux->fmtctx->pb);
					pRead->mux->fmtctx->pb = NULL;
				}
				avformat_close_input(&pRead->mux->fmtctx);
				pRead->mux->fmtctx = NULL;
				if (pRead->mux->opt)
				{
					av_dict_free(&pRead->mux->opt);
					pRead->mux->opt = NULL;
				}
			}
			return -1;
		}

		if (avcodec_open2(pRead->mux->apCodecCtx, pRead->mux->audio_codec, &pRead->mux->opt) < 0)
		{
			printf("Call avcodec_open function failed !\n");
			if (pRead->mux->fmtctx)
			{
				if (pRead->mux->vpCodecCtx)
				{
					avcodec_close(pRead->mux->vpCodecCtx);
					pRead->mux->vpCodecCtx = NULL;
				}
				if (pRead->mux->apCodecCtx)
				{
					avcodec_close(pRead->mux->apCodecCtx);
					pRead->mux->apCodecCtx = NULL;
				}

				if (pRead->mux->fmtctx->pb)
				{
					avio_close(pRead->mux->fmtctx->pb);
					pRead->mux->fmtctx->pb = NULL;
				}
				avformat_close_input(&pRead->mux->fmtctx);
				pRead->mux->fmtctx = NULL;
				if (pRead->mux->opt)
				{
					av_dict_free(&pRead->mux->opt);
					pRead->mux->opt = NULL;
				}
			}
			return -1;
		}
	}
#ifdef GLOB_DEC
	//g_audio_codec = pRead->mux->audio_codec;
	g_audio_c = pRead->mux->apCodecCtx;
	g_video_c = pRead->mux->vpCodecCtx;
#endif
#if 0
	// create resampler context 
	c = mux->urlStr.apCodecCtx;
	if (c)
	{
		if (c->sample_fmt != AV_SAMPLE_FMT_S16) {
			mux->swr_ctx = swr_alloc();
			if (!mux->swr_ctx) {
				fprintf(stderr, "Could not allocate resampler context\n");
				exit(1);
			}

			// set options 
			av_opt_set_int(mux->swr_ctx, "in_channel_count", c->channels, 0);
			av_opt_set_int(mux->swr_ctx, "in_sample_rate", c->sample_rate, 0);
			av_opt_set_sample_fmt(mux->swr_ctx, "in_sample_fmt", c->sample_fmt, 0);
			av_opt_set_int(mux->swr_ctx, "out_channel_count", c->channels, 0);
			av_opt_set_int(mux->swr_ctx, "out_sample_rate", c->sample_rate, 0);
			av_opt_set_sample_fmt(mux->swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

			// initialize the resampling context 
			if ((ret = swr_init(mux->swr_ctx)) < 0) {
				fprintf(stderr, "Failed to initialize the resampling context\n");
				exit(1);
			}
		}
		if (mux->audioFrame == NULL)
        {
#ifdef FFMPEG_V_30
            mux->audioFrame = av_frame_alloc();
#else
			mux->audioFrame = avcodec_alloc_frame();
#endif
        }
	}
#endif
	mst->params.width;
#endif
	if (logfp) { fprintf(logfp, "ReadInit ok: ret = %d \n", ret); fflush(logfp); }
	//pRead->mux->timeout = 0;
	pRead->mux->ReadIsOk = 1;
	
	return ret;
}
int CodecCodeded(MediaStream *mst, CoreStream *pCodec, char * inBuf[3], int len, char *outBuf[3], int oLen,int &frame_type,int codec_flag)
{
	int ret = 0;

	//ret = !codec_flag ? video_encode(pCodec, inBuf, len) : codec_flag == 1 ? video_decode(pCodec, inBuf, len) : video_decode2(pCodec, inBuf, len);
	switch(codec_flag)
	{
	case kVideoEncRaw :
		ret = video_encode(mst, pCodec, inBuf, len, outBuf, oLen, frame_type);
		break;
	case kVideoEncStream :
		ret = video_encode(mst, pCodec, inBuf, len, outBuf, oLen, frame_type);
		break;
	case kVideoDecRaw :
		ret = video_decode(mst, pCodec, inBuf, len, outBuf, oLen, frame_type);
		break;
	case kVideoDecStream :
		//ret = video_decode2(pCodec, inBuf, len, outBuf, oLen, frame_type);
		break;
	case kAudioEncRaw :
		ret = audio_encode(mst, pCodec, inBuf, len, outBuf, oLen, frame_type);
		break;
	case kAudioEncStream :
		ret = audio_encode(mst, pCodec, inBuf, len, outBuf, oLen, frame_type);
		break;
	case kAudioDecRaw :
		ret = audio_decode(mst, pCodec, inBuf, len, outBuf, oLen, frame_type);
		break;
	case kAudioDecStream :
		//ret = audio_decode2(pCodec, inBuf, len, outBuf, oLen, frame_type);
		break;
	}
	return ret;
}
int CodecOpen(MediaStream *mst, CoreStream *pCodec)
{
	CodecParams *params = &mst->params;
	int ret = 0;
	int EncOrDec = pCodec->EncOrDec;
	int AudioOrVideo = pCodec->AudioOrVideo;
	AVCodec **codec = &pCodec->codec;
	AVFormatContext **oc = pCodec->mux ? &pCodec->mux->fmtctx : NULL;
	AVStream **st = &pCodec->st;
	AVCodecContext *c= NULL;
	enum AVCodecID codec_id = (AVCodecID)(AudioOrVideo ? params->audio_codec_id : params->video_codec_id);//AV_CODEC_ID_H264;
	int width = !pCodec->levelId ? params->width : pCodec->levelId == 1 ? params->width1 : params->width2;
	int height = !pCodec->levelId ? params->height : pCodec->levelId == 1 ? params->height1 : params->height2;
	int bitsrate = !pCodec->levelId ? params->video_bits_rate : pCodec->levelId == 1 ? params->video_bits_rate1 : params->video_bits_rate2;
	if(EncOrDec)
	{
		
		*codec = avcodec_find_encoder(codec_id);
		if (logfp) { fprintf(logfp, "avcodec_find_encoder = %x \n", *((int *)codec)); fflush(logfp); }
	}
	else
	{
		*codec = avcodec_find_decoder(codec_id);
		if (logfp) { fprintf(logfp, "avcodec_find_decoder = %x \n", *((int *)codec)); fflush(logfp); }
	}
    
	if (!*codec) 
	{
        ret = -1;
		if (logfp) { fprintf(logfp, "CodecOpen = %d \n", ret); fflush(logfp); }
		return -1;
	}
	if(oc == NULL || *oc == NULL)
	{
		c = avcodec_alloc_context3(*codec);
		if (!c) 
		{
            ret = -2;
			if (logfp) { fprintf(logfp, "CodecOpen = %d \n", ret); fflush(logfp); }
			return -2;
		}
	}
	else
	{
		if (pCodec->st == NULL)
		{
		*st = avformat_new_stream(*oc, *codec);
		if(*st == NULL)
		{
            ret = -3;
			if (logfp) { fprintf(logfp, "CodecOpen = %d \n", ret); fflush(logfp); }
			return -3;
		}
			if (pCodec->stream_idx < 0)
			{
		(*st)->id = (*oc)->nb_streams-1;
				pCodec->stream_idx = (*st)->id;
			}
			else
			{
				(*st)->id = pCodec->stream_idx;
			}
		}
		else
		{
			*st = pCodec->st;
		}
		c = (*st)->codec;
	}
	if(EncOrDec)
	{
		if (logfp) { fprintf(logfp, "EncOrDec = %d \n", EncOrDec); fflush(logfp); }
		if(AudioOrVideo)
		{
			if (logfp) { fprintf(logfp, "AudioOrVideo = %d \n", AudioOrVideo); fflush(logfp); }
			c->codec_id = codec_id;
			c->sample_fmt  = (*codec)->sample_fmts ? (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
			if (logfp) { fprintf(logfp, "sample_fmt = %d \n", c->sample_fmt); fflush(logfp); }
			c->bit_rate    = params->audio_bits_rate;//64000;
			c->sample_rate = params->in_sample_rate;//;//44100;
			c->frame_size = params->frame_size;
			if ((*codec)->supported_samplerates) {
				c->sample_rate = (*codec)->supported_samplerates[0];
				for (int i = 0; (*codec)->supported_samplerates[i]; i++) {
					if ((*codec)->supported_samplerates[i] == 48000)//44100)
						c->sample_rate = 48000;//44100;
				}
			}
			c->channels  = av_get_channel_layout_nb_channels(c->channel_layout);
			if (logfp) { fprintf(logfp, "channels = %d \n", c->channels); fflush(logfp); }
			c->channel_layout = AV_CH_LAYOUT_STEREO;
			if ((*codec)->channel_layouts) {
				c->channel_layout = (*codec)->channel_layouts[0];
				for (int i = 0; (*codec)->channel_layouts[i]; i++) {
					if ((*codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
						c->channel_layout = AV_CH_LAYOUT_STEREO;
				}
			}
			c->channels        = av_get_channel_layout_nb_channels(c->channel_layout);
			///        ost->st->time_base = (AVRational){ 1, c->sample_rate };
			c->time_base.den = c->sample_rate;
			c->time_base.num = 1;
			//
			if (logfp) {
				fprintf(logfp, "pCodec->frame = %x \n", *((int *)&pCodec->frame)); fflush(logfp);
				fprintf(logfp, "pCodec->tmp_frame = %x \n", *((int *)&pCodec->tmp_frame)); fflush(logfp);
			}
			//
			if (c->codec && c->codec->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE)
			{
				pCodec->frame->nb_samples = pCodec->tmp_frame->nb_samples = 10000;
			}
			else
			{
				pCodec->frame->nb_samples = pCodec->tmp_frame->nb_samples = c->frame_size;
			}
			if (logfp) { fprintf(logfp, "nb_samples = %d \n", pCodec->frame->nb_samples); fflush(logfp); }
			pCodec->frame->channels = pCodec->tmp_frame->channels = c->channels;
			if (logfp) { fprintf(logfp, "channels = %d \n", pCodec->frame->channels); fflush(logfp); }
			pCodec->frame->sample_rate = pCodec->tmp_frame->sample_rate = c->sample_rate;
			if (logfp) { fprintf(logfp, "sample_rate-0 = %d \n", pCodec->frame->sample_rate); fflush(logfp); }
			pCodec->frame->format = pCodec->tmp_frame->format = c->sample_fmt;
			if (logfp) { fprintf(logfp, "format = %d \n", pCodec->frame->format); fflush(logfp); }
			pCodec->frame->channel_layout = pCodec->tmp_frame->channel_layout = c->channel_layout;
			if (logfp) { fprintf(logfp, "sample_rate-1 = %d \n", pCodec->frame->sample_rate); fflush(logfp); }
		}
		else
		{
			if (logfp) { fprintf(logfp, "AudioOrVideo = %d \n", AudioOrVideo); fflush(logfp); }
			c->codec_id = codec_id;
			c->width = width;//params->width;//OUTWIDTH;
			c->height = height;//params->height;//OUTHEIGHT;
			c->time_base.den = params->frame_rate;//FPS;
			c->time_base.num = 1;
			c->pix_fmt = (AVPixelFormat)params->in_data_format;//PIX_FMT_YUV420P;
			c->qmax = 40;
			c->qmin = 20;// 26;
			//c->qmax = c->qmin = 30;// 26;//const qp//test
			//
			//c->refs = 2;
			c->b_frame_strategy = false;
			c->max_b_frames = 0;// 3;// 2;// 1;// 0;// 1;// 4;// 1;// 0;
			c->coder_type = c->max_b_frames ? FF_CODER_TYPE_AC : FF_CODER_TYPE_VLC;
			c->coder_type = FF_CODER_TYPE_AC;//test
			//
			//c->cqp = 24;
			c->bit_rate = bitsrate;//params->video_bits_rate;//768000;
			//c->rc_max_rate = bitsrate;
			c->gop_size = params->gop_size;
			//
			c->thread_type = FF_THREAD_SLICE; //FF_THREAD_FRAME;
			//c->thread_count = 1;
			//
			if (!strcmp(params->preset, ""))
			{
				av_opt_set(c->priv_data, "preset", "superfast", 0);
			}
			else
			{
				av_opt_set(c->priv_data, "preset", params->preset, 0);
			}
			av_opt_set(c->priv_data, "tune", "zerolatency", 0);
			char ctmp[32] = "1400";
			char text[128] = "slice-max-size=";
			itoa(params->mtu_size, ctmp, 10);
			strcat(text, ctmp);
///			av_opt_set(c->priv_data, "x264opts",text,0);
			//av_opt_set(c->priv_data, "x264opts","slice-max-size=1400:keyint=50",0);
			//av_opt_set(c->priv_data, "preset", "ultrafast", 0);
			//av_opt_set(c->priv_data, "tune","stillimage,fastdecode,zerolatency",0);
			//av_opt_set(c->priv_data, "x264opts","crf=26:vbv-maxrate=728:vbv-bufsize=364:keyint=50:slice-max-size=1400",0);//:mtu=1400:sliced-threads=1

		}
	}
	else
	{
		if (!AudioOrVideo)
		{
			c->thread_type = FF_THREAD_SLICE;
		}
		if((*codec)->capabilities&CODEC_CAP_TRUNCATED)
			c->flags|= CODEC_FLAG_TRUNCATED; // we do not send complete frames 
	}
	pCodec->sdp_flag = 0;
#ifdef GLOBAL_HEADER
	//if(oc->oformat->flags & AVFMT_GLOBALHEADER)
		c->flags|= CODEC_FLAG_GLOBAL_HEADER;
	pCodec->sdp_flag = 1;
#endif
	if (logfp) { fprintf(logfp, "avcodec_open2 = %d \n", ret); fflush(logfp); }
	if (avcodec_open2(c, *codec, NULL) < 0) 
	{
		return -3;
	}
	pCodec->codecCtx = c;
	if (logfp) { fprintf(logfp, "avcodec_open2:c = %x \n", *((int *)&c)); fflush(logfp); }
	
	if(AudioOrVideo)
	{
		pCodec->bsfc = av_bitstream_filter_init("aac_adtstoasc");
	}
	else
	{
		pCodec->bsfc = av_bitstream_filter_init("h264_mp4toannexb");
	}
	if (logfp) { fprintf(logfp, "CodecOpen = %d \n", ret); fflush(logfp); }
	return ret;
}
void *GetMediaStreaCreated(void *hnd, int flag)
{
	MediaStream *mst = (MediaStream *)hnd;
	CodecParams *params = &mst->params;
	void *ret = 0;
	//ffFactory._critsect->Enter();
	MediaMap::iterator it = ffFactory._ffmap.begin();
	while(it != ffFactory._ffmap.end())
	{
		MediaStream *mst0 = (MediaStream *)it->second;
		if(mst != mst0 && mst0)
		{
			if(!flag)
			{
				if((!strcmp(mst->params.out_filename, mst0->params.out_filename) && 
					strlen(mst0->params.out_filename)) ||
					(!strcmp(mst->params.out_streamname, mst0->params.out_streamname) && 
					strlen(mst0->params.out_streamname)))
				{
					ret = it->second;
					break;
				}
			}
			else
			{
				if(!strcmp(mst->params.in_filename, mst0->params.in_filename) &&
					strlen(mst0->params.out_filename))
				{
					ret = it->second;
					break;
				}
			}
		}

		it++;
	}
	//ffFactory._critsect->Leave();
	return ret;
}
int MediaStreamOpen(void *hnd, int codecFlag)
{
	MediaStream *mst = (MediaStream *)hnd;
	CodecParams *params = &mst->params;
	int ret = 0;
	int codec_flag = codecFlag;
	int EncOrDec = !(codec_flag & 1);
	int AudioOrVideo = ((codec_flag >= kAudioEncRaw) && (codec_flag < kAVEnc));
	mst->codecType = codecFlag;
	if(mst && strcmp(mst->cparams, ""))
	{
#if 1
		g_critsect->Enter();
#else
		if (g_cb)
		{
			g_cb(&g_critsect, 0);
		}
#endif

		av_register_all();                   
		ret = avformat_network_init();
		//
		MediaStream *mst0 = NULL;
		int read_flag = strcmp(params->in_filename, "");
		unsigned int out_flag = params->out_flag;
		//read
#if 0
		if(read_flag)
		{
			mst0 = (MediaStream *)GetMediaStreaCreated((void *)mst, 1);
			mst->ReadSt = (CoreStream *)calloc(1, sizeof(CoreStream));
			CoreStream *pCodec = mst->ReadSt;

			if(mst0 == NULL || mst0->ReadSt == NULL || mst0->ReadSt->mux == NULL)//mst0->WriteSt[i]->fmtctx == NULL)
			{
				pCodec->ReadIsCpy = 0;
				pCodec->mux = (AVMuxer *)calloc(1, sizeof(AVMuxer));//as same mux of writer ?
				pCodec->mux->read_critsect = webrtc::CriticalSectionWrapper::CreateCriticalSection();
				pCodec->mux->AVReadFlag |= 1 << pCodec->ReadIsCpy;
				strcpy(pCodec->inFile, params->in_filename);
				ret = ReadInit(mst, pCodec);
			}
			else
			{
				//缺陷：音视频必须都正常				
				pCodec->ReadIsCpy = 1;
				//pCodec->fmtctx = mst0->WriteSt[i]->fmtctx;	
				pCodec->mux = mst0->ReadSt->mux;
				pCodec->mux->AVReadFlag |= 1 << pCodec->ReadIsCpy;
			}
			
			//decode init
		}
#endif
		if(!out_flag || !EncOrDec)
		{
			mst->CodecSt = (CoreStream *)calloc(1, sizeof(CoreStream));
			mst->CodecSt->stream_idx = -1;
			CoreStream *pCodec = mst->CodecSt;
			pCodec->EncOrDec = !(codec_flag & 1);
			if (logfp) { fprintf(logfp, "EncOrDec = %d \n", pCodec->EncOrDec); fflush(logfp); }
			pCodec->AudioOrVideo = ((codec_flag >= kAudioEncRaw) && (codec_flag < kAVEnc));
			if (logfp) { fprintf(logfp, "AudioOrVideo = %d \n", pCodec->AudioOrVideo); fflush(logfp); }
#ifdef FFMPEG_V_30
			pCodec->frame = av_frame_alloc();
			pCodec->tmp_frame = av_frame_alloc();
#else
			pCodec->frame = avcodec_alloc_frame(); 
			pCodec->tmp_frame = avcodec_alloc_frame();
#endif
            if (pCodec->frame) {
				if (logfp) { fprintf(logfp, "pCodec->frame = %x \n", *((int *)&pCodec->frame)); fflush(logfp); }
				if (logfp) { fprintf(logfp, "pCodec->tmp_frame = %x \n", *((int *)&pCodec->tmp_frame)); fflush(logfp); }
            }
            else
            {
				if (logfp) { fputs("frame alloc fail \n", logfp);  fflush(logfp); }
            }
			ret = CodecOpen(mst, pCodec);
			if (ret < 0)
			{
#if 1
				g_critsect->Leave();
#else
				if (g_cb)
				{
					g_cb(&g_critsect, 1);
				}
#endif
				return ret;
			}
			if (read_flag)
			{
				strcpy(pCodec->inFile, params->in_filename);
				printf("infile= %s \n", pCodec->inFile);
			}
			if (logfp) { fprintf(logfp, "CodecOpen = %d \n", pCodec->AudioOrVideo); fflush(logfp); }
		}
		if(out_flag)
		{
			mst0 = (MediaStream *)GetMediaStreaCreated((void *)mst, 0);
			if (logfp) { fprintf(logfp, "out_flag:mst0 = %x \n", *((int *)&mst0)); fflush(logfp); }
			for(int i = 0; i < 6; i++)
			{
				if(!(out_flag & (1 << i)))
					continue;
				mst->WriteSt[i] = (CoreStream *)calloc(1, sizeof(CoreStream));
				mst->WriteSt[i]->stream_idx = -1;
				CoreStream *pCodec = mst->WriteSt[i];
				if(i > 2)
				{
					pCodec->levelId = i - 3;
					pCodec->FileOrStream = 1;
					strcpy(pCodec->outFile, mst->filePath);
                    printf("filePath= %s \n",mst->filePath);
					strcat(pCodec->outFile, params->out_filename);
					get_extend(pCodec->outFile, pCodec->fileType, pCodec->levelId, pCodec->FileOrStream);
					//strcpy(pCodec->fileType, "flv");
				}
				else
				{
					pCodec->levelId = i;
					pCodec->FileOrStream = 0;
					strcpy(pCodec->outFile, params->out_streamname);
					get_extend(pCodec->outFile, pCodec->fileType, pCodec->levelId, pCodec->FileOrStream);
				}
				pCodec->EncOrDec = !(codec_flag & 1);
				pCodec->AudioOrVideo = ((codec_flag >= kAudioEncRaw) && (codec_flag < kAVEnc));
#ifdef FFMPEG_V_30
				pCodec->frame = av_frame_alloc();
				pCodec->tmp_frame = av_frame_alloc();
#else
				pCodec->frame = avcodec_alloc_frame(); 
				pCodec->tmp_frame = avcodec_alloc_frame(); 
#endif
                if (pCodec->frame) {
					if (logfp) {
						fprintf(logfp, "pCodec->frame = %x \n", *((int *)&pCodec->frame)); fflush(logfp);
						fprintf(logfp, "pCodec->tmp_frame = %x \n", *((int *)&pCodec->tmp_frame)); fflush(logfp);
					}
                }
                else
                {
					if (logfp) { fputs("frame alloc fail \n", logfp);  fflush(logfp); }
                }
				if(mst0 == NULL || mst0->WriteSt[i] == NULL || mst0->WriteSt[i]->mux == NULL)//mst0->WriteSt[i]->fmtctx == NULL)
				{
					pCodec->WriteIsCpy = 0;
					pCodec->mux = (AVMuxer *)calloc(1, sizeof(AVMuxer));
#if 1
					pCodec->mux->write_critsect = webrtc::CriticalSectionWrapper::CreateCriticalSection();
#else
					if (g_cb)
					{
						g_cb(&pCodec->mux->write_critsect, -1);
					}
#endif
					pCodec->mux->AVWriteFlag |= 1 << pCodec->WriteIsCpy;
					if (logfp) { fprintf(logfp, "out_flag:WriteInit-0 = %d \n", ret); fflush(logfp); }
					ret = WriteInit(mst, pCodec);
					if (logfp) { fprintf(logfp, "out_flag:WriteInit-1 = %d \n", ret); fflush(logfp); }
				}
				else
				{
					//缺陷：音视频必须都正常
					pCodec->WriteIsCpy = 1;
					//pCodec->fmtctx = mst0->WriteSt[i]->fmtctx;	
					pCodec->mux = mst0->WriteSt[i]->mux;
					pCodec->mux->AVWriteFlag |= 1 << pCodec->WriteIsCpy;
				}
				ret = CodecOpen(mst, pCodec);
				//
				pCodec->mux->RecordStatus = kRecordStart;//test
				if(pCodec->WriteIsCpy == 1)
				{
					//缺陷：音视频必须都正常
					if (logfp) { fprintf(logfp, "WriteOpen-0 = %d \n", ret); fflush(logfp); }
					ret = WriteOpen(mst, pCodec);
					if (ret >= 0)
					{
						if (logfp) { fprintf(logfp, "WriteOpen-1 = %d \n", ret); fflush(logfp); }
						pCodec->mux->WriteIsOpen = 1;
						if (!pCodec->FileOrStream && pCodec->mux->RecordStatus)
						{
							if (logfp) { fprintf(logfp, "WriteHeader-0 = %d \n", ret); fflush(logfp); }
							pCodec->mux->WriteHead = WriteHeader(pCodec->mux->fmtctx) == 0;
							if (logfp) { fprintf(logfp, "WriteHeader-1 = %d \n", pCodec->mux->WriteHead); fflush(logfp); }
						}
					}
					else
					{
						pCodec->mux->WriteIsOpen = -1;
					}
					ret = 0;
				}
			}
		}
#if 1
		g_critsect->Leave();
#else
		if (g_cb)
		{
			g_cb(&g_critsect, 1);
		}
#endif
	}
	return ret;
}
static void *json_1d_get_data(void *hnd, char *title, char *key)
{
	MediaStream* pCodec = (MediaStream*)hnd;
	jsonObject *obj = NULL;// Json1DGetJsonObj(title, pCodec->json_obj);
	return (void *)Json1DGetJsonData(key, obj);
}
static void set_params(void *hnd)
{
	//MediaStream* mst = (MediaStream*)hnd;
	
#if 1
	MediaStream* pCodec = (MediaStream*)hnd;
	
	pCodec->jsonObj = IStr2Json(pCodec->cparams, pCodec->jsonObj);

	pCodec->params.in_data_format = AV_PIX_FMT_YUV420P;		//raw(yuv) or stream
	IReadMember(pCodec->jsonObj, "codecParams.in_data_format", &pCodec->params.in_data_format, NULL);
//	SETVALUE2KEY0(pCodec, (pCodec->params.in_data_format), AV_PIX_FMT_YUV420P, "in_data_format")
		pCodec->params.width = OUTWIDTH;
	IReadMember(pCodec->jsonObj, "codecParams.width", &pCodec->params.width, NULL);
//	SETVALUE2KEY0(pCodec, (pCodec->params.width), OUTWIDTH, "width")
		pCodec->params.height = OUTHEIGHT;
	IReadMember(pCodec->jsonObj, "codecParams.height", &pCodec->params.height, NULL);
//	SETVALUE2KEY0(pCodec, (pCodec->params.height), OUTHEIGHT, "height")
		pCodec->params.width1 = OUTWIDTH >> 1;
	IReadMember(pCodec->jsonObj, "codecParams.width1", &pCodec->params.width1, NULL);
//	SETVALUE2KEY0(pCodec, (pCodec->params.width1), OUTWIDTH >> 1, "width1")
		IReadMember(pCodec->jsonObj, "codecParams.height1", &pCodec->params.height1, NULL);
//	SETVALUE2KEY0(pCodec, (pCodec->params.height1), OUTHEIGHT >> 1, "height1")
		IReadMember(pCodec->jsonObj, "codecParams.width2", &pCodec->params.width2, NULL);
//	SETVALUE2KEY0(pCodec, (pCodec->params.width2), OUTWIDTH >> 2, "width2")
		IReadMember(pCodec->jsonObj, "codecParams.height2", &pCodec->params.height2, NULL);
//	SETVALUE2KEY0(pCodec, (pCodec->params.height2), OUTHEIGHT >> 2, "height2")
		pCodec->params.gop_size = 50;
	IReadMember(pCodec->jsonObj, "codecParams.gop_size", &pCodec->params.gop_size, NULL);
//	SETVALUE2KEY0(pCodec, (pCodec->params.gop_size), 50, "gop_size")

	memset(pCodec->params.preset, 0, 32 * sizeof(char));
	IReadMember(pCodec->jsonObj, "codecParams.preset", NULL, pCodec->params.preset);
	
		pCodec->params.mtu_size = MUT_SIZE;
	IReadMember(pCodec->jsonObj, "codecParams.mtu_size", &pCodec->params.mtu_size, NULL);
//	SETVALUE2KEY0(pCodec, (pCodec->params.mtu_size), MUT_SIZE, "mtu_size")
		pCodec->params.frame_rate = 25;
	IReadMember(pCodec->jsonObj, "codecParams.frame_rate", &pCodec->params.frame_rate, NULL);
//	SETVALUE2KEY0(pCodec, (pCodec->params.frame_rate), 25, "frame_rate")
		pCodec->params.min_bits_rate = 128000;
	IReadMember(pCodec->jsonObj, "codecParams.min_bits_rate", &pCodec->params.min_bits_rate, NULL);
//	SETVALUE2KEY0(pCodec, (pCodec->params.min_bits_rate), 128000, "min_bits_rate")
		pCodec->params.max_bits_rate = 512000;
	IReadMember(pCodec->jsonObj, "codecParams.max_bits_rate", &pCodec->params.max_bits_rate, NULL);
//	SETVALUE2KEY0(pCodec, (pCodec->params.max_bits_rate), 512000, "max_bits_rate")
		pCodec->params.out_data_format = AV_PIX_FMT_YUV420P;	//ras(yuv) or stream
	IReadMember(pCodec->jsonObj, "codecParams.out_data_format", &pCodec->params.out_data_format, NULL);
//	SETVALUE2KEY0(pCodec, (pCodec->params.out_data_format), AV_PIX_FMT_YUV420P, "out_data_format")
		pCodec->params.video_bits_rate = 256000;
	IReadMember(pCodec->jsonObj, "codecParams.video_bits_rate", &pCodec->params.video_bits_rate, NULL);
//	SETVALUE2KEY0(pCodec, (pCodec->params.video_bits_rate), 256000, "video_bits_rate")
		IReadMember(pCodec->jsonObj, "codecParams.video_bits_rate1", &pCodec->params.video_bits_rate1, NULL);
//	SETVALUE2KEY0(pCodec, (pCodec->params.video_bits_rate1), 256000, "video_bits_rate1")
		IReadMember(pCodec->jsonObj, "codecParams.video_bits_rate2", &pCodec->params.video_bits_rate2, NULL);
//	SETVALUE2KEY0(pCodec, (pCodec->params.video_bits_rate2), 256000, "video_bits_rate2")
		pCodec->params.video_codec_id = AV_CODEC_ID_H264;
		IReadMember(pCodec->jsonObj, "codecParams.video_codec_id", &pCodec->params.video_codec_id, NULL);
//		SETVALUE2KEY0(pCodec, (pCodec->params.video_codec_id), CODEC_ID_H264, "video_codec_id")
		//
		pCodec->params.audio_bits_rate = 24000;
	IReadMember(pCodec->jsonObj, "codecParams.audio_bits_rate", &pCodec->params.audio_bits_rate, NULL);
//	SETVALUE2KEY0(pCodec, (pCodec->params.audio_bits_rate), 24000, "audio_bits_rate")
		pCodec->params.audio_codec_id = AV_CODEC_ID_AAC;
	IReadMember(pCodec->jsonObj, "codecParams.audio_codec_id", &pCodec->params.audio_codec_id, NULL);
//	SETVALUE2KEY0(pCodec, (pCodec->params.audio_codec_id), AV_CODEC_ID_AAC, "audio_codec_id")
		pCodec->params.frame_size = 2048;
	IReadMember(pCodec->jsonObj, "codecParams.frame_size", &pCodec->params.frame_size, NULL);
//	SETVALUE2KEY0(pCodec, (pCodec->params.frame_size), 2048, "frame_size")
		pCodec->params.in_channel_count = 2;
	IReadMember(pCodec->jsonObj, "codecParams.in_channel_count", &pCodec->params.in_channel_count, NULL);
//	SETVALUE2KEY0(pCodec, (pCodec->params.in_channel_count), 2, "in_channel_count")
		pCodec->params.in_sample_rate = 48000;
	IReadMember(pCodec->jsonObj, "codecParams.in_sample_rate", &pCodec->params.in_sample_rate, NULL);
//	SETVALUE2KEY0(pCodec, (pCodec->params.in_sample_rate), 48000, "in_sample_rate")
		pCodec->params.in_sample_fmt = AV_SAMPLE_FMT_FLTP;
	IReadMember(pCodec->jsonObj, "codecParams.in_sample_fmt", &pCodec->params.in_sample_fmt, NULL);
//	SETVALUE2KEY0(pCodec, (pCodec->params.in_sample_fmt), AV_SAMPLE_FMT_FLTP, "in_sample_fmt")
		pCodec->params.out_channel_count = 2;
	IReadMember(pCodec->jsonObj, "codecParams.out_channel_count", &pCodec->params.out_channel_count, NULL);
//	SETVALUE2KEY0(pCodec, (pCodec->params.out_channel_count), 2, "out_channel_count")
		pCodec->params.out_sample_rate = 48000;
	IReadMember(pCodec->jsonObj, "codecParams.out_sample_rate", &pCodec->params.out_sample_rate, NULL);
//	SETVALUE2KEY0(pCodec, (pCodec->params.out_sample_rate), 48000, "out_sample_rate")
		pCodec->params.out_sample_fmt = AV_SAMPLE_FMT_S16;
	IReadMember(pCodec->jsonObj, "codecParams.out_sample_fmt", &pCodec->params.out_sample_fmt, NULL);
//	SETVALUE2KEY0(pCodec, (pCodec->params.out_sample_fmt), AV_SAMPLE_FMT_S16, "out_sample_fmt")
		//
		//strcpy(pCodec->filePath, "");
		//SETVALUE2KEY1(pCodec, (pCodec->filePath), "", "filePath")
//	g_critsect->Enter();
	pCodec->params.streamId = 0;
	IReadMember(pCodec->jsonObj, "codecParams.streamId", &pCodec->params.streamId, NULL);
//	SETVALUE2KEY0(pCodec, (pCodec->params.streamId), 0, "streamId")
	pCodec->streamId = pCodec->params.streamId;
	pCodec->params.debugInfo = 0;
	IReadMember(pCodec->jsonObj, "codecParams.debugInfo", (int *)&pCodec->params.debugInfo, NULL);
//	SETVALUE2KEY0(pCodec, (pCodec->params.debugInfo), 0, "debugInfo")
	if (logfp) { fprintf(logfp, "debugInfo = %d \n", pCodec->params.debugInfo); fflush(logfp); }
		pCodec->params.out_flag = 0;
		IReadMember(pCodec->jsonObj, "codecParams.out_flag", (int *)&pCodec->params.out_flag, NULL);
//	SETVALUE2KEY0(pCodec, (pCodec->params.out_flag), 0, "out_flag")
		strcpy(pCodec->params.in_filename, "");
		memset(pCodec->params.in_filename, 0, MAX_PATH * sizeof(char));
	IReadMember(pCodec->jsonObj, "codecParams.in_filename", NULL, pCodec->params.in_filename);
	//memset(pCodec->params.in_filename, 0, MAX_PATH * sizeof(char));
//	SETVALUE2KEY1(pCodec, (pCodec->params.in_filename), "", "in_filename")
		strcpy(pCodec->params.out_filename, "");
		memset(pCodec->params.out_filename, 0, MAX_PATH * sizeof(char));
	IReadMember(pCodec->jsonObj, "codecParams.out_filename", NULL, pCodec->params.out_filename);
	//memset(pCodec->params.out_filename, 0, MAX_PATH * sizeof(char));
//	SETVALUE2KEY1(pCodec, (pCodec->params.out_filename), "", "out_filename")
		strcpy(pCodec->params.out_streamname, "");
		memset(pCodec->params.out_streamname, 0, MAX_PATH * sizeof(char));
	IReadMember(pCodec->jsonObj, "codecParams.out_streamname", NULL, pCodec->params.out_streamname);
	//memset(pCodec->params.out_streamname, 0, MAX_PATH * sizeof(char));
//	SETVALUE2KEY1(pCodec, (pCodec->params.out_streamname), "", "out_streamname")
//	g_critsect->Leave();

		strcpy(pCodec->params.sdp_filename, "audio.sdp");
		IReadMember(pCodec->jsonObj, "codecParams.sdp_filename", NULL, pCodec->params.sdp_filename);
//	SETVALUE2KEY1(pCodec, (pCodec->params.sdp_filename), "audio.sdp", "sdp_filename")
#endif
}
void MediaStreamSetParams(void *hnd, char *text)
{
	MediaStream *mst = (MediaStream *)hnd;
	if(text && strlen(text))
	{
///		int ret = Json1D(text, mst->json_obj);
		strcpy(mst->cparams, text);
	}
	else
	{
		//default
		FILE *fp = NULL;
		char pathName[256];
		strcpy(pathName, mst->filePath);
		strcat(pathName, "ffconfig.text");
		fp = fopen(pathName, "r");
		if(fp)
		{
			char cBuf[PARAMS_SIZE] = "";
			fseek(fp, 0L, SEEK_END); 
			int fsize = ftell(fp); 
			rewind(fp);
			int ret = fread(cBuf,1, fsize, fp);
			strcpy(mst->cparams, cBuf);
			//ret = Json1D(cBuf, mst->json_obj);
		}
	}
#if 1
	g_critsect->Enter();
#else
	if (g_cb)
	{
		g_cb(&g_critsect, 0);
	}
#endif

#if 1
	set_params(hnd);
#else
	if (g_cb)
	{
		g_cb(&hnd, 3);
	}
#endif

#if 1
	g_critsect->Leave();
#else
	if (g_cb)
	{
		g_cb(&g_critsect, 1);
	}
#endif
}
//aduio encoder will be create 2 times and delete 1 times
//create 3 times and delete 2 times
int CoreStreamRelease(CoreStream *coreSt)
{
	int ret = 0;
	if (coreSt->scale)
	{
		ffmpeg_close((void *)coreSt->scale);
		coreSt->scale = NULL;
	}
	if (coreSt->resample)
	{
		ResampleClose((void *)coreSt->resample);
		coreSt->resample = NULL;
	}
#if 1
	if(coreSt->st)
	{
		avcodec_close(coreSt->st->codec);
		coreSt->st = NULL;
	}

	if(coreSt->codecCtx)
	{
		avcodec_close(coreSt->codecCtx);
		coreSt->codecCtx = NULL;
	}
	if(coreSt->codec)
	{
		//avcodec_close(coreSt->codec);
		coreSt->codec = NULL;
	}
#endif
	if (coreSt->mux)
	{
		if (coreSt->mux->opt)
		{
			av_dict_free(&coreSt->mux->opt);
			coreSt->mux->opt = NULL;
		}
		coreSt->mux->AVReadFlag &= ~(1 << coreSt->ReadIsCpy);
		if (coreSt->mux->ifmt)
		{
			if (!coreSt->mux->AVReadFlag)
			{
				if (coreSt->mux->fmtctx)
				{
					avformat_close_input(&coreSt->mux->fmtctx);
					coreSt->mux->fmtctx = NULL;
				}
				coreSt->mux->ifmt = NULL;
			}
			else
			{
				coreSt->mux = NULL;
			}
		}
		coreSt->mux->AVWriteFlag &= ~(1 << coreSt->WriteIsCpy);
		if (coreSt->mux->ofmt)
		{
			if (!coreSt->mux->AVWriteFlag)
			{
				coreSt->mux->WriteHead = 0;
#if 1			
				coreSt->mux->write_critsect->Enter();
#else
				if (g_cb)
				{
					g_cb(&coreSt->mux->write_critsect, 0);
				}
#endif
				if (coreSt->mux->WriteHead && coreSt->mux->RecordStatus)
				{
					ret = WriteTail(coreSt->mux->fmtctx);
				}
#if 1
				if (coreSt->st)
				{
					avcodec_close(coreSt->st->codec);
					coreSt->st = NULL;
				}

				if (coreSt->codecCtx)
				{
					avcodec_close(coreSt->codecCtx);
					coreSt->codecCtx = NULL;
				}
				if (coreSt->codec)
				{
					//avcodec_close(coreSt->codec);
					coreSt->codec = NULL;
				}
#endif
				if (coreSt->mux->avio)
				{
					coreSt->mux->avio = NULL;
				}
				if (coreSt->mux->fmtctx)
				{
					if (coreSt->mux->fmtctx->pb)
					{
						avio_close(coreSt->mux->fmtctx->pb);
						coreSt->mux->fmtctx->pb = NULL;
					}
					//mux->callback(mux->audioChanId, mux->file[i].filename, get_timelen(mux), 0);
					if (coreSt->mux->fmtctx)
					{
						avformat_free_context(coreSt->mux->fmtctx);
						coreSt->mux->fmtctx = NULL;
					}
					coreSt->mux->ofmt = NULL;
				}
#if 1
				coreSt->mux->write_critsect->Leave();
				if (coreSt->mux->write_critsect)
				{
					delete coreSt->mux->write_critsect;
					coreSt->mux->write_critsect = NULL;
				}
#else
				if (g_cb)
				{
					g_cb(&coreSt->mux->write_critsect, 1);
					g_cb(&coreSt->mux->write_critsect, 2);
				}
#endif
			}
			else
			{
				coreSt->mux = NULL;
			}
		}
		if (coreSt->mux && !coreSt->mux->AVWriteFlag && !coreSt->mux->AVReadFlag)
		{
			free(coreSt->mux);
			coreSt->mux = NULL;
		}
	}
	
	//coreSt->pkt = ;
	if(coreSt->frame)
	{
		av_free(coreSt->frame);
		coreSt->frame = NULL;
	}
	if (coreSt->tmp_frame)
	{
		av_free(coreSt->tmp_frame);
		coreSt->tmp_frame = NULL;
	}
	if (coreSt->alloc_pic.data[0])
	{
		avpicture_free(&coreSt->alloc_pic);
		//av_free(coreSt->alloc_pic.data[0]);
		//av_free(coreSt->alloc_pic.data[1]);
		//av_free(coreSt->alloc_pic.data[2]);
		//coreSt->pic = NULL;
		//printf("delete pic");
	}
	if(coreSt->bsfc)
	{
		av_bitstream_filter_close(coreSt->bsfc);
		coreSt->bsfc = NULL;
	}
	//coreSt->stream_idx = -1;
	coreSt->id = -1;
	coreSt->WriteIsCpy = -1;
	coreSt->ReadIsCpy = -1;
	return ret;
}
#if 0
void CoreStreamInit(CoreStream *coreSt)
{
	memset(coreSt->inFile, 0, MAX_PATH * sizeof(char));
	memset(coreSt->outFile, 0, MAX_PATH * sizeof(char));
	//memset(coreSt->outStream, 0, MAX_PATH * sizeof(char));
	//memset(coreSt->fileType, 0, 32 * sizeof(char));
	memset(coreSt->streamType, 0, 32 * sizeof(char));
	//memset(coreSt->streamExtend, 0, 32 * sizeof(char));
	coreSt->fmt = NULL;
	coreSt->opt = NULL;
	coreSt->fmtctx = NULL;
	coreSt->st = NULL;
	coreSt->codecCtx = NULL;
	coreSt->codec = NULL;
	coreSt->avio = NULL;
	//coreSt->pkt = ;
	coreSt->frame = NULL;
	//coreSt->pic;
	coreSt->bsfc = NULL;
	coreSt->stream_idx = -1;
	coreSt->id = -1;
}
#endif
void set_path(MediaStream *ffcodec)
{
	char achTemp[256];
	char* pPos = NULL;
	//getcwd(achTemp, 255);
#ifdef _WIN32
	GetModuleFileNameA(NULL, achTemp, sizeof(achTemp));
    pPos = strrchr(achTemp, '\\');
    *++pPos = '\0';
#elif defined(__ANDROID__)
    //strcpy(achTemp, "/storage/emulated/0/");
	strcpy(achTemp, "/sdcard/");
#elif defined(WEBRTC_IOS) || defined(IOS)
    char dirName[256] = "";
    char *pName = MAKE_FILE_NAME(dirName);//makePreferencesFilename("webrtc_log.txt");
    strcpy(achTemp, pName);
    strcat(achTemp, "/");
    free(pName);
#elif defined(WEBRTC_LINUX)
    printf("gxhtest: set_path: linux version \n");
#elif defined(WEBRTC_MAC)
    printf("gxhtest: set_path: mac version \n");
#else
#endif
	//strcat(achTemp,"\\");
	strcpy(ffcodec->filePath,achTemp);
	//
}
int MediaStreamWriteClose(void *hnd, int codec_flag)
{
	int ret = 0;
	MediaStream *mst = (MediaStream *)hnd;
	CodecParams *params = &mst->params;
	int EncOrDec = !(codec_flag & 1);
	int read_flag = strcmp(params->in_filename, "");
	unsigned int out_flag = params->out_flag;

	if (read_flag)
	{
		CoreStream *pCodec = mst->ReadSt;
		//ret = CodecCodeded(mst, pCodec, inBuf, len, outBuf, oLen,frame_type,codec_flag);
	}
	if (!out_flag || !EncOrDec)
	{
		CoreStream *pCodec = mst->CodecSt;
		//if(pCodec->WriteIsCpy)
		//{
		//ret = WriteTail(pCodec->fmtctx);
		//}
	}
	else if (out_flag)
	{
		for (int i = 0; i < 6; i++)
		{
			if (!(out_flag & (1 << i)))
				continue;
			CoreStream *pCodec = mst->WriteSt[i];
			if (pCodec->FileOrStream && pCodec->mux)
			{
				pCodec->mux->WriteHead = 0;
				if (pCodec->WriteIsCpy == 1)
				{
#if 1
					pCodec->mux->write_critsect->Enter();
#else
					if (g_cb)
					{
						g_cb(&pCodec->mux->write_critsect, 0);
					}
#endif
					ret = WriteTail(pCodec->mux->fmtctx);

#if 1
					if (pCodec->mux->fmtctx)
					{
						if (pCodec->mux->fmtctx->pb)
						{
							avio_close(pCodec->mux->fmtctx->pb);
							pCodec->mux->fmtctx->pb = NULL;
						}
						avformat_close_input(&pCodec->mux->fmtctx);
						pCodec->mux->fmtctx = NULL;
						if (pCodec->mux->opt)
						{
							av_dict_free(&pCodec->mux->opt);
							pCodec->mux->opt = NULL;
						}
					}
#endif

#if 1
					pCodec->mux->write_critsect->Leave();
#else
					if (g_cb)
					{
						g_cb(&pCodec->mux->write_critsect, 1);
					}
#endif
				}
			}
		}
	}

	return ret;
}
int MediaStreamRelease(void *hnd)
{
	MediaStream *mst = (MediaStream *)hnd;
	int ret = 0;
	if(mst)
	{
		int codecFlag = mst->codecType;
		//ret = MediaStreamWriteClose(hnd, codecFlag);
		if (mst->ifp)
		{
			fclose(mst->ifp);
		}
		if (mst->ofp)
		{
			fclose(mst->ofp);
		}
		memset(mst->cparams, 0, PARAMS_SIZE * sizeof(char));
		memset(mst->filePath, 0, MAX_PATH * sizeof(char));
		
		//if(mst->json_obj)
		//{
		//	Json1DRelease(mst->json_obj);
		//	free(mst->json_obj);
		//	mst->json_obj = NULL;
		//}
		for(int i = 0; i < 6; i++)
		{
			if(mst->WriteSt[i])
			{
				CoreStreamRelease(mst->WriteSt[i]);
				free(mst->WriteSt[i]);
				mst->WriteSt[i] = NULL;
			}
		}
		if(mst->CodecSt)
		{
			CoreStreamRelease(mst->CodecSt);
			free(mst->CodecSt);
			mst->CodecSt = NULL;
		}
		if(mst->ReadSt)
		{
			CoreStreamRelease(mst->ReadSt);
			free(mst->ReadSt);
			mst->ReadSt = NULL;
		}
#if 1
		if(mst->jsonObj)
		{
		IDeleteJson(mst->jsonObj);
		mst->jsonObj = NULL;
		}
#endif
		//if(mst->resample)
		//{
		//	mst->resample = NULL;
		//}
		//if(mst->scale)
		//{
		//	mst->scale = NULL;
		//}
		for (int i = 0; i < 2; i++)//inset image, water mark image 
		{
			for (int j = 0; j < 2; j++)//in, out
			{
				if (mst->img[i][j].data)
				{
					//avpicture_free(&mst->img[i][j]);
					delete mst->img[i][j].data;
					mst->img[i][j].data = NULL;
				}
			}
		}
	}
	return 0;
}
int MediaStreamInit(void *hnd)
{
	MediaStream *mst = (MediaStream *)hnd;
	if(mst)
	{
		memset(mst->cparams, 0, PARAMS_SIZE * sizeof(char));
		memset(mst->filePath, 0, MAX_PATH * sizeof(char));
		//mst->json_obj = NULL;
		mst->ifp = NULL;
		mst->ofp = NULL;
		for(int i = 0; i < 6; i++)
		{
			mst->WriteSt[i] = NULL;
		}
		//mst->scale = NULL;
		//mst->resample = NULL;
		mst->CodecSt = NULL;
		mst->ReadSt = NULL;
		//mst->json_obj = (jsonObject *)calloc(1,sizeof(jsonObject));
		set_path(mst);
		mst->ChanId = -1;
		mst->jsonObj = NULL;
        mst->reset = 0;
		//
		for (int i = 0; i < 2; i++)//inset image, water mark image 
		{
			for (int j = 0; j < 2; j++)//in, out
			{
				if (mst->img[i][j].data)
				{
					mst->img[i][j] = {};
					mst->img[i][j].data = NULL;
				}
			}
			
		}
		for (int i = 0; i < 3; i++)
		{
			memset(mst->jpgFile[i], 0, MAX_PATH * sizeof(char));
		}
		mst->waterMarkParams = {};
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
int FF_CODEC_RENEWURL(int chanId, char *inFile, int InOrOut, int FileOrStream, int type)
{
	int ret = 0;
#if 1
	g_critsect->Enter();
#else
	if (g_cb)
	{
		g_cb(&g_critsect, 0);
	}
#endif
	MediaMap::iterator it = ffFactory._ffmap.begin();
	while (it != ffFactory._ffmap.end())
	{
		MediaStream *mst = (MediaStream *)it->second;
		static const uint8_t kAVMux = 16;
		static const uint8_t kAudio = 1;
		static const uint8_t kVideo = 2;
		//int AudioOrVideo = ((mst->codecType >= kAudioEncRaw) && (mst->codecType < kAVEnc)) ? kAudio : kVideo;//1:audio; 0:video
		int AudioOrVideo = mst->codecType < kAudioEncRaw ? kVideo : mst->codecType < kAVEnc ? kAudio : kAVMux;
		if (mst->codecType == kAVDecStream)
		{
			printf("kAVDecStream \n");
		}
		if ((AudioOrVideo == type) && mst->ChanId == chanId)
		{
			if (!InOrOut)//in
			{
				CoreStream *pRead = (CoreStream *)mst->ReadSt;
				//
				if (pRead)
				{
					strcpy(mst->params.in_filename, inFile);
					strcpy(pRead->inFile, inFile);
#if 1
					pRead->mux->read_critsect->Enter();
#else
					if (g_cb)
					{
						g_cb(&pRead->mux->read_critsect, 0);
					}
#endif
					if (pRead->mux->fmtctx)
					{
						avcodec_close(pRead->mux->vpCodecCtx);
						pRead->mux->vpCodecCtx = NULL;
						avcodec_close(pRead->mux->apCodecCtx);
						pRead->mux->apCodecCtx = NULL;
						//avcodec_close(&pRead->mux->video_codec);
						//pRead->mux->video_codec = NULL;
						//avcodec_close(pRead->mux->audio_codec);
						//pRead->mux->audio_codec = NULL;

						if (pRead->mux->fmtctx->pb)
						{
							avio_close(pRead->mux->fmtctx->pb);
							pRead->mux->fmtctx->pb = NULL;
						}
						avformat_close_input(&pRead->mux->fmtctx);						
						pRead->mux->fmtctx = NULL;
						av_dict_free(&pRead->mux->opt);
						pRead->mux->opt = NULL;
						///ret = ReadInit(mst, pRead);
					}
#if 1
					pRead->mux->read_critsect->Leave();
#else
					if (g_cb)
					{
						g_cb(&pRead->mux->read_critsect, 1);
					}
#endif
					//
					ret = 1;
					break;
				}
			}
			else
			{
				for (int i = 0; i < 6; i++)
				{
					CoreStream *pWrite = (CoreStream *)mst->WriteSt[i];
					if (i <= 2 && FileOrStream)//file
					{
						ret = 1;
						break;
					}
					else if (i > 2 && !FileOrStream)
					{
						ret = 1;
						break;
					}
				}
			}
			//if (pRead && inFile && !strcmp(pRead->inFile, inFile))
			//{
			//ret = (void *)pRead->mux->vpCodecCtx;
			//break;
			//}
		}
		it++;
	}
#if 1
	g_critsect->Leave();
#else
	if (g_cb)
	{
		g_cb(&g_critsect, 1);
	}
#endif
	return ret;
}
void *GetInFFmpegFactory(int streamId, char *inFile, int type)
{
	void *ret = NULL;
#if 1
	g_critsect->Enter();
#else
	if (g_cb)
	{
		g_cb(&g_critsect, 0);
	}
#endif
	MediaMap::iterator it = ffFactory._ffmap.begin();
	while (it != ffFactory._ffmap.end())
	{
		MediaStream *mst = (MediaStream *)it->second;
		CoreStream *pRead = (CoreStream *)mst->ReadSt;
		if (pRead)
		{
#if 1
			pRead->mux->read_critsect->Enter();
#else
			if (g_cb)
			{
				g_cb(&pRead->mux->read_critsect, 0);
			}
#endif
			if (pRead && inFile && !strcmp(pRead->inFile, inFile) && (mst->streamId == streamId))
			{
				ret = (void *)pRead->mux->vpCodecCtx;
#if 1
				pRead->mux->read_critsect->Leave();
#else
				if (g_cb)
				{
					g_cb(&pRead->mux->read_critsect, 1);
				}
#endif
				break;
			}
#if 1
			pRead->mux->read_critsect->Leave();
#else
			if (g_cb)
			{
				g_cb(&pRead->mux->read_critsect, 1);
			}
#endif
		}
		it++;
	}
#if 1
	g_critsect->Leave();
#else
	if (g_cb)
	{
		g_cb(&g_critsect, 1);
	}
#endif
	return ret;
}
int Add2FFmpegFactory(void *hnd)
{
	int result = 0;	
#if 1
	g_critsect->Enter();
#else
	if (g_cb)
	{
		g_cb(&g_critsect, 0);
	}
#endif
	int idx = ffFactory._ffmap.size();
	ffFactory._ffmap.insert(std::pair<int,void *>(idx,hnd));
#if 1
	g_critsect->Leave();
#else
	if (g_cb)
	{
		g_cb(&g_critsect, 1);
	}
#endif
	return idx;
}
int Sub2FFmpegFactory(int idx)
{
	int result = 0;
	//int idx = ffFactory._ffmap.size();
	//ffFactory._ffmap.insert(std::pair<int, void *>(idx, hnd));
	MediaMap::iterator it = ffFactory._ffmap.find(idx);
	if (it != ffFactory._ffmap.end())
	{
		void *st = it->second;
		MediaStreamRelease(st);
		//
		//delete st;//it->second;
		ffFactory._ffmap.erase(it++);
	}
	return idx;
}
int CreateMediaStream(void** pHandle)
{
	int ret;
	if(pHandle == NULL)
		return 0;
	else
	{
		//g_critsect->Enter();
		MediaStream *hnd = new MediaStream;
		ret = Add2FFmpegFactory((void *)hnd);
		MediaStreamInit(hnd);
		*pHandle = hnd;
		//g_critsect->Leave();
	}
	return ret;
}
void *FF_CODEC_GET_HND(int chanId, int type)
{
	void *ret = NULL;
#if 1
	g_critsect->Enter();
#else
	if (g_cb)
	{
		g_cb(&g_critsect, 0);
	}
#endif
	MediaMap::iterator it = ffFactory._ffmap.begin();
	while (it != ffFactory._ffmap.end())
	{
		MediaStream *mst = (MediaStream *)it->second;
		//static const uint8_t kAVMux = 16;
		//static const uint8_t kAudio = 1;
		//static const uint8_t kVideo = 2;
		//int AudioOrVideo = mst->codecType < kAudioEncRaw ? kVideo : mst->codecType < kAVEnc ? kAudio : kAVMux;
		//kVideoDecRaw
		if ((mst->codecType == type) && mst->ChanId == chanId)
		{
			ret = it->second;
			break;
		}
		it++;
	}
#if 1
	g_critsect->Leave();
#else
	if (g_cb)
	{
		g_cb(&g_critsect, 1);
	}
#endif
	return ret;
}
void ffmpegFactoryStop(void)
{
#if 1
	if (g_critsect)
#endif
	{
#if 1
		g_critsect->Enter();
#else
		if (g_cb)
		{
			g_cb(&g_critsect, 0);
		}
#endif
		ffmpegFactory *pFactory = (ffmpegFactory *)g_ffFactory;
		pFactory->status = 0;
#if 1
		g_critsect->Leave();
#else
		if (g_cb)
		{
			g_cb(&g_critsect, 1);
		}
#endif
	}
	}
void  ffmpegFactorySetCb(void *cb)
{
	g_cb = (pCritsCallback)cb;
}
int ffmpegFactoryInit(void **hnd, int flag)
{
	ffmpegFactory *pFactory = NULL;
#if 1
	if (g_critsect == NULL)
	{
		g_critsect = webrtc::CriticalSectionWrapper::CreateCriticalSection();
	}
#else
	if (g_cb)
	{
		g_cb(&g_critsect, -1);
	}
#endif
	if(flag)
	{
		if(hnd == NULL)
		{
			return -1;
		}
		else
		{
			//ffmpegFactory *
			pFactory = new ffmpegFactory;
			pFactory->status = 1;
			*hnd = (void*)pFactory;
			g_ffFactory = (void*)pFactory;//rsv
		}
	}
	else
	{
		ffFactory.status = 1;
		pFactory = &ffFactory;
		if (hnd)
		{
			*hnd = (void*)pFactory;
		}
	}
	//pFactory->_critsect = webrtc::CriticalSectionWrapper::CreateCriticalSection();
	memset(pFactory->_cparams, 0, PARAMS_SIZE * sizeof(char));

	return 0;
}
void  ffmpegFactoryRelease(void *hnd)
{
	ffmpegFactory *pFactory = (ffmpegFactory *)hnd;
	if(pFactory == NULL)
	{
		pFactory = &ffFactory;
	}
#if 1
	if(g_critsect)
	{
		delete g_critsect;
		g_critsect = NULL;
	}
#else
	if (g_cb)
	{
		g_cb(&g_critsect, 2);
	}
#endif
	MediaMap::iterator it = ffFactory._ffmap.begin();
	while(it != ffFactory._ffmap.end())
	{
		MediaStream *st = (MediaStream *)it->second;
		//
        delete st;//it->second;
		ffFactory._ffmap.erase(it++);
		if (ffFactory._ffmap.empty()) 
		{
			break;
		}
	}
}
#if 1
int MediaReadStreamOpen(void *hnd, int codecFlag)
{
	MediaStream *mst = (MediaStream *)hnd;
	CodecParams *params = &mst->params;
	int ret = 0;
	int codec_flag = codecFlag;
	int EncOrDec = !(codec_flag & 1);
	int AudioOrVideo = ((codec_flag >= kAudioEncRaw) && (codec_flag < kAVEnc));
	mst->codecType = codecFlag;
	//if (mst && strcmp(mst->cparams, ""))
	if (mst)
	{
#if 1
		g_critsect->Enter();
#else
		if (g_cb)
		{
			g_cb(&g_critsect, 0);
		}
#endif

		av_register_all();
		ret = avformat_network_init();
		//
		MediaStream *mst0 = NULL;
		int read_flag = strcmp(params->in_filename, "");
		unsigned int out_flag = params->out_flag;
		//read
		if (read_flag)
		{
			//mst0 = (MediaStream *)GetMediaStreaCreated((void *)mst, 1);
			mst->ReadSt = (CoreStream *)calloc(1, sizeof(CoreStream));
			mst->ReadSt->stream_idx = -1;
			CoreStream *pCodec = mst->ReadSt;

			//if (mst0 == NULL || mst0->ReadSt == NULL || mst0->ReadSt->mux == NULL)//mst0->WriteSt[i]->fmtctx == NULL)
			{
				pCodec->ReadIsCpy = 0;
				pCodec->mux = (AVMuxer *)calloc(1, sizeof(AVMuxer));//as same mux of writer ?
#if 1
				pCodec->mux->read_critsect = webrtc::CriticalSectionWrapper::CreateCriticalSection();
#else
				if (g_cb)
				{
					g_cb(&pCodec->mux->read_critsect, -1);
				}
#endif
				pCodec->mux->AVReadFlag |= 1 << pCodec->ReadIsCpy;
				strcpy(pCodec->inFile, params->in_filename);
				///ret = ReadInit(mst, pCodec);
			}
			//
			mst->CodecSt = (CoreStream *)calloc(1, sizeof(CoreStream));
			mst->CodecSt->stream_idx = -1;
			pCodec = mst->CodecSt;
			pCodec->EncOrDec = !(codec_flag & 1);
			if (logfp) { fprintf(logfp, "EncOrDec = %d \n", pCodec->EncOrDec); fflush(logfp); }
			pCodec->AudioOrVideo = ((codec_flag >= kAudioEncRaw) && (codec_flag < kAVEnc));
			if (logfp) { fprintf(logfp, "AudioOrVideo = %d \n", pCodec->AudioOrVideo); fflush(logfp); }
#ifdef FFMPEG_V_30
			pCodec->frame = av_frame_alloc();
			pCodec->tmp_frame = av_frame_alloc();
#else
			pCodec->frame = avcodec_alloc_frame();
			pCodec->tmp_frame = avcodec_alloc_frame();
#endif
			if (pCodec->frame) {
				if (logfp) {
					fprintf(logfp, "pCodec->frame = %x \n", *((int *)&pCodec->frame)); fflush(logfp);
					fprintf(logfp, "pCodec->tmp_frame = %x \n", *((int *)&pCodec->tmp_frame)); fflush(logfp);
				}
			}
			else
			{
				if (logfp) { fputs("frame alloc fail \n", logfp);  fflush(logfp); }
			}
			strcpy(pCodec->inFile, params->in_filename);
		}
#if 1
		g_critsect->Leave();
#else
		if (g_cb)
		{
			g_cb(&g_critsect, 1);
		}
#endif
	}
	return ret;
}
int StreamIsOverFlow(MediaStream *hnd, unsigned char *buf, int *len, int *isKeyFrame)
{
	int ret = 0;
	int out_size = -1;
	MediaStream *mst = (MediaStream *)hnd;
	CoreStream *pRead = mst->ReadSt;
	CoreStream *audio = mst->CodecSt;
	AVCodecContext *c = pRead->mux->apCodecCtx;
	static const uint8_t kRtpHeaderSize = 12;
#if 1
	pRead->mux->read_critsect->Enter();
#else
	if (g_cb)
	{
		g_cb(&pRead->mux->read_critsect, 0);
	}
#endif
	if (audio->pos >= 8192)
	{
#if 0
		//audio->debug_flag = 1;
		//static FILE* fp = NULL;
		if (!audio->ofp  && audio->debug_flag)
		{
#ifdef _WIN32
			audio->ofp = fopen("e://works//test//audio-d.pcm", "wb");
#elif defined(__ANDROID__)
			audio->ofp = fopen("/sdcard/audio-d.pcm", "wb");
#elif defined(WEBRTC_IOS)
			char dirName[256] = "audio-d.pcm";
			char *pName = MAKE_FILE_NAME(dirName);//makePreferencesFilename("webrtc_log.txt");
			audio->ofp = fopen(pName, "wb");
			free(pName);
#else
			audio->ofp = fopen("audio-d.pcm", "wb");
#endif
		}
		if (audio->ofp)
		{
			int wsize = fwrite(audio->buf, 1, 8192, audio->ofp);

			//if(wsize > 0)
			//{
			//	printf("");
			//}
		}
#endif
		memcpy(&buf[kRtpHeaderSize], audio->buf, 8192);
		audio->pos -= 8192;
		if (audio->pos)
		{
			memmove(audio->buf, &audio->buf[8192], audio->pos);
		}
		out_size = 8192 + kRtpHeaderSize;
	}
	if (out_size > 0)
	{
		ret = out_size;
	}
	else
	{
		ret = 0;
	}
	if (ret > 0)
	{
		//printf("audio stream \n");
		int *frame_size = (int *)buf;//mst->params.frame_size;
		*frame_size = c->frame_size;
		int time_step = *frame_size;
		uint8_t *payload_type = (uint8_t *)&buf[4];//rsv for extend
		*payload_type = 115;
		int *sample_rate = (int *)&buf[8];// mst->params.in_sample_rate;
		*sample_rate = 48000;// c->sample_rate;
		//pkt.pts
#if 1
		__int64 time_stamp0 = webrtc::TickTime::MillisecondTimestamp();
#else
		__int64 time_stamp0 = get_sys_time();
		if (g_cb)
		{
			g_cb((void **)&time_stamp0, 4);
		}
#endif
		//memcpy(&buf[kRtpHeaderSize], pkt.data, pkt.size);
		//printf("pkt.size= %d \n", pkt.size);
		//*len = pkt.size + kRtpHeaderSize;
		*len = ret;
		*isKeyFrame = 0;// pkt.flags;
	}
#if 1
	pRead->mux->read_critsect->Leave();
#else
	if (g_cb)
	{
		g_cb(&pRead->mux->read_critsect, 1);
	}
#endif
	return ret;
}
int MediaReadFrame(MediaStream *hnd, unsigned char *buf, int *len, int *isKeyFrame, int *width, int *height)
{
	int ret = -1;
	MediaStream *mst = (MediaStream *)hnd;
	CoreStream *pRead = mst->ReadSt;
	static const uint8_t kRtpHeaderSize = 12;
	static const uint8_t kRtpExtendSize = (sizeof(EXTEND_HEADER) >> 2) - 1;
	static const uint8_t kRtpGenSize = 1;
	static const uint8_t kAudio = 1;
	static const uint8_t kVideo = 2;
	//
	if (*width < 0 || *height < 0)
	{
		return StreamIsOverFlow(hnd, buf, len, isKeyFrame);
	}
	//
	AVPacket pkt;
	av_init_packet(&pkt);
#if 1
	pRead->mux->read_critsect->Enter();
#else
	if (g_cb)
	{
		g_cb(&pRead->mux->read_critsect, 0);
	}
#endif
	if (!pRead->mux->fmtctx)
	{
		ret = ReadInit(mst, pRead);
        printf("readinit ret = %d \n", ret);
        if (logfp) { fprintf(logfp, "ReadInit: ret=%d \n", ret); fflush(logfp); }
		if (ret < 0)
		{
#if 1
			pRead->mux->read_critsect->Leave();
#else
			if (g_cb)
			{
				g_cb(&pRead->mux->read_critsect, 1);
			}
#endif
			av_free_packet(&pkt);
			return ret;
		}
		//av_seek_frame(pRead->mux->fmtctx, -1, 20 * AV_TIME_BASE, AVSEEK_FLAG_ANY);
	}
	else if (!pRead->mux->ReadIsOk)
	{
		av_free_packet(&pkt);
		return -1;
	}
#if 1
	pRead->mux->time0 = IGetTime();
#else
	pRead->mux->time0 = get_sys_time(); //IGetTime();
	if (g_cb)
	{
		g_cb((void **)&pRead->mux->time0, 4);
	}
#endif
	ret = av_read_frame(pRead->mux->fmtctx, &pkt);
	if (ret < 0)
	{
		//printf("read frame error ############################################\n");
        if (logfp) { fprintf(logfp, "av_read_frame: ret=%d \n", ret); fflush(logfp); }
		if (pRead->mux->fmtctx)
		{
			avcodec_close(pRead->mux->vpCodecCtx);
			pRead->mux->vpCodecCtx = NULL;
			avcodec_close(pRead->mux->apCodecCtx);
			pRead->mux->apCodecCtx = NULL;
			//avcodec_close(&pRead->mux->video_codec);
			//pRead->mux->video_codec = NULL;
			//avcodec_close(pRead->mux->audio_codec);
			//pRead->mux->audio_codec = NULL;

			if (pRead->mux->fmtctx->pb)
			{
				avio_close(pRead->mux->fmtctx->pb);
				pRead->mux->fmtctx->pb = NULL;
			}
			avformat_close_input(&pRead->mux->fmtctx);
			pRead->mux->fmtctx = NULL;
			av_dict_free(&pRead->mux->opt);
			pRead->mux->opt = NULL;
			///ret = ReadInit(mst, pRead);
		}
#if 1
		pRead->mux->read_critsect->Leave();
#else
		if (g_cb)
		{
			g_cb(&pRead->mux->read_critsect, 1);
		}
#endif
		av_free_packet(&pkt);
		return ret;
	}
#ifdef GXH_TEST_FLV
	//int64_t 
	time0 = pkt.test_pts[0];
	//int64_t 
	time1 = pkt.test_pts[1];
	int64_t time2 = pkt.test_pts[2];
	int64_t time3 = pkt.test_pts[3];
	int64_t time = av_gettime() + 800 * 1000;//get_sys_time();
	//int 
	diffTime = (int)(time - time0) / 1000;
	
	if (diffTime > 1000 || diffTime < 0)
	{
		printf("MediaReadFrame: idx= %d (ms) \n", pkt.stream_index);
	}
	else if (diffTime > 100)
	{
		//printf("MediaReadFrame: %d: diffTime= %d (ms) \n", pkt.stream_index, diffTime);
	}
#endif
	if (pkt.stream_index == pRead->mux->video_index)
	{
		//printf("av_read_frame: video: diffTime = %d \t pkt.size= %d \n", diffTime, pkt.size);
		AVCodecContext *c = pRead->mux->vpCodecCtx;
		//printf("video stream \n");
		AVStream* istream = pRead->mux->fmtctx->streams[pRead->mux->video_index];
		int extlen = (kRtpExtendSize + 1) << 2;
		
		mst->params.width = c->width; //istream->parser->coded_width;//test
		mst->params.height = c->height; //istream->parser->coded_height;
		*width = c->width;
		*height = c->height;
		int *frame_rate = (int *)buf;// mst->params.frame_rate;
#if defined(__ANDROID__) || defined(WEBRTC_LINUX)
        *frame_rate = c->time_base.den / c->time_base.num;
#else
		if (c->framerate.den)
		{
		*frame_rate = c->framerate.num / c->framerate.den;
		}
		else
		{
			*frame_rate = c->time_base.den / c->time_base.num;
		}
#endif
		if (*frame_rate < 1)
		{
#if 1
			pRead->mux->read_critsect->Leave();
#else
			if (g_cb)
			{
				g_cb(&pRead->mux->read_critsect, 1);
			}
#endif
			printf("MediaReadFrame: frame_rate = %d \n", *frame_rate);
			return -3;
		}
		int time_step = 90000 / *frame_rate;
		uint8_t *payload_type = (uint8_t *)&buf[4];//rsv for extend
		*payload_type = 96;
		//pkt.pts
		int InsetBytes = kRtpHeaderSize + kRtpGenSize + kRtpHeaderSize + extlen;
		//__int64 time_stamp0 = webrtc::TickTime::MillisecondTimestamp();
#ifdef GXH_TEST_FLV
		int64_t *test_time = (int64_t *)&buf[InsetBytes];
		*test_time = time0;
		InsetBytes += 8;
		int *frame_number = (int *)&buf[InsetBytes];
		*frame_number = pkt.frame_number;
		InsetBytes += 4;
#endif
		memcpy(&buf[InsetBytes], pkt.data, pkt.size);
		*len = pkt.size + InsetBytes;
		*isKeyFrame = pkt.flags;
		ret = kVideo;//enum AVType{audio = 0,video };
	}
	else
	{
		AVCodecContext *c = pRead->mux->apCodecCtx; 
#if 1
		CoreStream *audio = mst->CodecSt;
		AVFrame* frame = audio->frame;
		//AVPacket pkt;
		int got_frame = 0;
		int out_size = -1;
		//av_init_packet(&pkt);
		//pkt.size = len;
		//pkt.data = (uint8_t *)inBuf[0];
		int read_size =
			ret = avcodec_decode_audio4(c, frame, &got_frame, &pkt);
		if (ret < 0)
		{
#if 1
			pRead->mux->read_critsect->Leave();
#else
			if (g_cb)
			{
				g_cb(&pRead->mux->read_critsect, 1);
			}
#endif
			av_free_packet(&pkt);
			return -2;
		}
		if (got_frame)
		{
			int data_size = av_get_bytes_per_sample(c->sample_fmt);
			size_t unpadded_linesize = audio->frame->nb_samples * av_get_bytes_per_sample((AVSampleFormat)frame->format);
			audio->tmp_frame->data[0] = (uint8_t *)&audio->buf[audio->pos];// outBuf[0][offset];
			if (audio->resample == NULL)
			{
				audio->resample = (ffmpegResample *)ResampleCreate(audio->resample);
			}
			frame->extended_data;
			ResampleParams((void *)audio->resample, frame->data, frame->format, frame->channels, frame->nb_samples, frame->sample_rate, 0);
			///ResampleParams((void *)audio->resample, audio->tmp_frame->data, AV_SAMPLE_FMT_S16, frame->channels, frame->nb_samples, frame->sample_rate, 1);
			int dst_nb_samples = (48000 * frame->nb_samples) / frame->sample_rate;// 44100;
			ResampleParams((void *)audio->resample, audio->tmp_frame->data, AV_SAMPLE_FMT_S16, frame->channels, dst_nb_samples, 48000, 1);
			ResampleInit((void *)audio->resample);
			ret = Resample((void *)audio->resample);
			ret = data_size * ret;

			///IIISpeexAECPlayPort(NULL, audio->buf, ret);	

			audio->pos += ret;
			if (audio->pos >= 8192)
			{
#if 0
				//audio->debug_flag = 1;
				//static FILE* fp = NULL;
				if (!audio->ofp  && audio->debug_flag)
				{
#ifdef _WIN32
					audio->ofp = fopen("e://works//test//audio-d.pcm", "wb");
#elif defined(__ANDROID__)
					audio->ofp = fopen("/sdcard/audio-d.pcm", "wb");
#elif defined(WEBRTC_IOS)
					char dirName[256] = "audio-d.pcm";
					char *pName = MAKE_FILE_NAME(dirName);//makePreferencesFilename("webrtc_log.txt");
					audio->ofp = fopen(pName, "wb");
					free(pName);
#else
					audio->ofp = fopen("audio-d.pcm", "wb");
#endif
				}
				if (audio->ofp)
				{
					int wsize = fwrite(audio->buf, 1, 8192, audio->ofp);

					//if(wsize > 0)
					//{
					//	printf("");
					//}
				}
#endif
				memcpy(&buf[kRtpHeaderSize], audio->buf, 8192);
				audio->pos -= 8192;
				if (audio->pos)
				{
					memmove(audio->buf, &audio->buf[8192], audio->pos);
				}
				out_size = 8192 + kRtpHeaderSize;
			}
			if (out_size > 0)
			{
				ret = out_size;
			}
			else
			{
				ret = 0;
			}
		}
		if (ret > 0)
		{
			//printf("audio stream \n");
			int *frame_size = (int *)buf;//mst->params.frame_size;
			*frame_size = c->frame_size;
			int time_step = *frame_size;
			uint8_t *payload_type = (uint8_t *)&buf[4];//rsv for extend
			*payload_type = 115;
			int *sample_rate = (int *)&buf[8];// mst->params.in_sample_rate;
			*sample_rate = 48000;// c->sample_rate;
			//pkt.pts
#if 1
			__int64 time_stamp0 = webrtc::TickTime::MillisecondTimestamp();
#else
			__int64 time_stamp0 = get_sys_time();
			if (g_cb)
			{
				g_cb((void **)&time_stamp0, 4);
			}
#endif
			//memcpy(&buf[kRtpHeaderSize], pkt.data, pkt.size);
			//printf("pkt.size= %d \n", pkt.size);
			//*len = pkt.size + kRtpHeaderSize;
			*len = ret;
			*isKeyFrame = 0;// pkt.flags;
		}
#endif
	}
#if 1
	pRead->mux->read_critsect->Leave();
#else
	if (g_cb)
	{
		g_cb(&pRead->mux->read_critsect, 1);
	}
#endif
	av_free_packet(&pkt);
	return ret;
}
//static int64_t testData[4][10000] = {};
//static int testCnt = 0;
//static int testCnt2 = 0;
int MediaReadFrame2(MediaStream *hnd, unsigned char *buf, int *len, int *isKeyFrame, int *width, int *height, int64_t *timeStamp)
{
	int ret = -1;
	MediaStream *mst = (MediaStream *)hnd;
	CoreStream *pRead = mst->ReadSt;
	static const uint8_t kRtpHeaderSize = 12;
	static const uint8_t kRtpExtendSize = (sizeof(EXTEND_HEADER) >> 2) - 1;
	static const uint8_t kRtpGenSize = 1;
	static const uint8_t kAudio = 1;
	static const uint8_t kVideo = 2;
	//
	if (*width < 0 || *height < 0)
	{
		return StreamIsOverFlow(hnd, buf, len, isKeyFrame);
	}
	//
	AVPacket pkt;
	av_init_packet(&pkt);
#if 1
	pRead->mux->read_critsect->Enter();
#else
	if (g_cb)
	{
		g_cb(&pRead->mux->read_critsect, 0);
	}
#endif
	if (!pRead->mux->fmtctx)
	{
		ret = ReadInit(mst, pRead);
		printf("readinit ret = %d \n", ret);
		if (logfp) { fprintf(logfp, "ReadInit: ret=%d \n", ret); fflush(logfp); }
		if (ret < 0)
		{
#if 1
			pRead->mux->read_critsect->Leave();
#else
			if (g_cb)
			{
				g_cb(&pRead->mux->read_critsect, 1);
			}
#endif
			av_free_packet(&pkt);
			return ret;
		}
		//av_seek_frame(pRead->mux->fmtctx, -1, 20 * AV_TIME_BASE, AVSEEK_FLAG_ANY);
	}
	else if (!pRead->mux->ReadIsOk)
	{
		av_free_packet(&pkt);
		return -1;
	}
#if 1
	pRead->mux->time0 = IGetTime();
#else
	pRead->mux->time0 = get_sys_time(); //IGetTime();
	if (g_cb)
	{
		g_cb((void **)&pRead->mux->time0, 4);
	}
#endif
	ret = av_read_frame(pRead->mux->fmtctx, &pkt);
	if (ret < 0)
	{
		//printf("read frame error ############################################\n");
		if (logfp) { fprintf(logfp, "av_read_frame: ret=%d \n", ret); fflush(logfp); }
		if (pRead->mux->fmtctx)
		{
			avcodec_close(pRead->mux->vpCodecCtx);
			pRead->mux->vpCodecCtx = NULL;
			avcodec_close(pRead->mux->apCodecCtx);
			pRead->mux->apCodecCtx = NULL;
			//avcodec_close(&pRead->mux->video_codec);
			//pRead->mux->video_codec = NULL;
			//avcodec_close(pRead->mux->audio_codec);
			//pRead->mux->audio_codec = NULL;

			if (pRead->mux->fmtctx->pb)
			{
				avio_close(pRead->mux->fmtctx->pb);
				pRead->mux->fmtctx->pb = NULL;
			}
			avformat_close_input(&pRead->mux->fmtctx);
			pRead->mux->fmtctx = NULL;
			av_dict_free(&pRead->mux->opt);
			pRead->mux->opt = NULL;
			///ret = ReadInit(mst, pRead);
		}
#if 1
		pRead->mux->read_critsect->Leave();
#else
		if (g_cb)
		{
			g_cb(&pRead->mux->read_critsect, 1);
		}
#endif
		av_free_packet(&pkt);
		return ret;
	}
#ifdef GXH_TEST_FLV
	//int64_t 
	time0 = pkt.test_pts[0];
	//int64_t 
	time1 = pkt.test_pts[1];
	int64_t time2 = pkt.test_pts[2];
	int64_t time3 = pkt.test_pts[3];
	int64_t time = av_gettime() + 800 * 1000;//get_sys_time();
	//int 
	diffTime = (int)(time - time0) / 1000;

	if (diffTime > 1000 || diffTime < 0)
	{
		printf("MediaReadFrame: idx= %d (ms) \n", pkt.stream_index);
	}
	else if (diffTime > 100)
	{
		//printf("MediaReadFrame: %d: diffTime= %d (ms) \n", pkt.stream_index, diffTime);
	}
#endif
	if (pkt.stream_index == pRead->mux->video_index)
	{
		//printf("av_read_frame: video: diffTime = %d \t pkt.size= %d \n", diffTime, pkt.size);
		AVCodecContext *c = pRead->mux->vpCodecCtx;
		//printf("video stream \n");
		AVStream* istream = pRead->mux->fmtctx->streams[pRead->mux->video_index];
		//int extlen = (kRtpExtendSize + 1) << 2;

		mst->params.width = c->width; //istream->parser->coded_width;//test
		mst->params.height = c->height; //istream->parser->coded_height;
		*width = c->width;
		*height = c->height;
		int *frame_rate = (int *)buf;// mst->params.frame_rate;
#if defined(__ANDROID__) || defined(WEBRTC_LINUX)
		*frame_rate = c->time_base.den / c->time_base.num;
#else
		if (c->framerate.den)
		{
			*frame_rate = c->framerate.num / c->framerate.den;
		}
		else
		{
			*frame_rate = c->time_base.den / c->time_base.num;
		}
#endif
		if (*frame_rate < 1)
		{
#if 1
			pRead->mux->read_critsect->Leave();
#else
			if (g_cb)
			{
				g_cb(&pRead->mux->read_critsect, 1);
			}
#endif
			printf("MediaReadFrame: frame_rate = %d \n", *frame_rate);
			return -3;
		}
		int frame_type = -1;
		int got_picture = 0;
		CoreStream *video = mst->CodecSt;
		AVFrame* frame = video->frame;
		if (video->pic.data[0] == NULL)
		{
			//avpicture_alloc(&pRead->pic, (enum AVPixelFormat)mst->params.in_data_format, mst->params.width, mst->params.height);//avpicture_free(AVPicture *picture);
			//avpicture_alloc(&pRead->pic, (enum AVPixelFormat)c->pix_fmt, c->width, c->height);
			avpicture_alloc(&video->pic, (enum AVPixelFormat)c->pix_fmt, c->width, c->height);
		}
		*((AVPicture *)video->frame) = video->pic;
		frame->key_frame = 0;
		//testData[0][testCnt] = frame->pkt_pts;
		ret = avcodec_decode_video2(c, frame, &got_picture, &pkt);
		//testData[1][testCnt++] = pkt.pts;
		if (ret < 0)
		{
			pRead->mux->read_critsect->Leave();
			av_free_packet(&pkt);
			return -2;
		}
		
		*timeStamp = pkt.pts;
		if (got_picture)
		{
			if ((pkt.flags & AV_PKT_FLAG_KEY) || (frame->pict_type == AV_PICTURE_TYPE_I) || (frame->key_frame & AV_PKT_FLAG_KEY))
			{
				frame_type = PIC_TYPE_KEYFRAME;
				*isKeyFrame = 0;
			}
			frame_type = frame->pict_type;
			if ((frame->key_frame & AV_PKT_FLAG_CORRUPT))
			{
				printf("error %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% \n");
			}
			char *outBuf[3] = {};
			outBuf[0] = (char *)&buf[0];
			outBuf[1] = (char *)&buf[c->width * c->height];
			outBuf[2] = (char *)&outBuf[1][(c->width >> 1) * (c->height >> 1)];
			//Y
			for (int i = 0; i < frame->height; i++)
			{
				memcpy(&outBuf[0][i * frame->width], &frame->data[0][i * frame->linesize[0]], frame->width);
			}
			//U
			for (int i = 0; i < frame->height; i++)
			{
				if (!(i & 1))
				{
					memcpy(&outBuf[1][(i >> 1) * (frame->width >> 1)], &frame->data[1][(i >> 1) * frame->linesize[1]], frame->width >> 1);
				}
			}
			//V
			for (int i = 0; i < frame->height; i++)
			{
				if (!(i & 1))
				{
					memcpy(&outBuf[2][(i >> 1) * (frame->width >> 1)], &frame->data[2][(i >> 1) * frame->linesize[2]], frame->width >> 1);
				}
			}
			*len = ((c->width * c->height * 3) >> 1);
		}
		else
		{
			char *outBuf[3] = {};
			outBuf[0] = (char *)&buf[0];
			outBuf[1] = (char *)&buf[c->width * c->height];
			outBuf[2] = (char *)&outBuf[1][(c->width >> 1) * (c->height >> 1)];
			memset(outBuf[0], 128, c->width * c->height);
			memset(outBuf[1], 128, (c->width >> 1) * (c->height >> 1));
			memset(outBuf[2], 128, (c->width >> 1) * (c->height >> 1));
			*len = ((c->width * c->height * 3) >> 1);
		}
		ret = kVideo;//enum AVType{audio = 0,video };;
	}
	else
	{
		AVCodecContext *c = pRead->mux->apCodecCtx;
#if 1
		CoreStream *audio = mst->CodecSt;
		AVFrame* frame = audio->frame;
		//AVPacket pkt;
		int got_frame = 0;
		int out_size = -1;
		//av_init_packet(&pkt);
		//pkt.size = len;
		//pkt.data = (uint8_t *)inBuf[0];
		//testData[2][testCnt2] = frame->pkt_pts;
		int read_size =
			ret = avcodec_decode_audio4(c, frame, &got_frame, &pkt);
		//testData[3][testCnt2++] = pkt.pts;
		if (ret < 0)
		{
#if 1
			pRead->mux->read_critsect->Leave();
#else
			if (g_cb)
			{
				g_cb(&pRead->mux->read_critsect, 1);
			}
#endif
			av_free_packet(&pkt);
			return -2;
		}
		
		*timeStamp = pkt.pts;
		if (got_frame)
		{
			int data_size = av_get_bytes_per_sample(c->sample_fmt);
			size_t unpadded_linesize = audio->frame->nb_samples * av_get_bytes_per_sample((AVSampleFormat)frame->format);
			audio->tmp_frame->data[0] = (uint8_t *)&audio->buf[audio->pos];// outBuf[0][offset];
			if (audio->resample == NULL)
			{
				audio->resample = (ffmpegResample *)ResampleCreate(audio->resample);
			}
			frame->extended_data;
			ResampleParams((void *)audio->resample, frame->data, frame->format, frame->channels, frame->nb_samples, frame->sample_rate, 0);
			///ResampleParams((void *)audio->resample, audio->tmp_frame->data, AV_SAMPLE_FMT_S16, frame->channels, frame->nb_samples, frame->sample_rate, 1);
			int dst_nb_samples = (48000 * frame->nb_samples) / frame->sample_rate;// 44100;
			ResampleParams((void *)audio->resample, audio->tmp_frame->data, AV_SAMPLE_FMT_S16, frame->channels, dst_nb_samples, 48000, 1);
			ResampleInit((void *)audio->resample);
			ret = Resample((void *)audio->resample);
			ret = data_size * ret;

			///IIISpeexAECPlayPort(NULL, audio->buf, ret);	

			audio->pos += ret;
			if (audio->pos >= 8192)
			{
#if 1
				audio->debug_flag = 0;
				//static FILE* fp = NULL;
				if (!audio->ofp  && audio->debug_flag)
				{
#ifdef _WIN32
					audio->ofp = fopen("c://works//test//audio-d.pcm", "wb");
#elif defined(__ANDROID__)
					audio->ofp = fopen("/sdcard/audio-d.pcm", "wb");
#elif defined(WEBRTC_IOS)
					char dirName[256] = "audio-d.pcm";
					char *pName = MAKE_FILE_NAME(dirName);//makePreferencesFilename("webrtc_log.txt");
					audio->ofp = fopen(pName, "wb");
					free(pName);
#else
					audio->ofp = fopen("audio-d.pcm", "wb");
#endif
				}
				if (audio->ofp)
				{
					int wsize = fwrite(audio->buf, 1, 8192, audio->ofp);
				}
#endif
				memcpy(&buf[0], audio->buf, 8192);
				audio->pos -= 8192;
				if (audio->pos)
				{
					memmove(audio->buf, &audio->buf[8192], audio->pos);
				}
				out_size = 8192;// +kRtpHeaderSize;
			}
			if (out_size > 0)
			{
				*len = out_size;
			}
			else
			{
				*len = 0;
			}
		}
		//if (ret > 0)
		//{
		//	*frame_size = c->frame_size;
		//	
		//	*sample_rate = 48000;// c->sample_rate;
		//	__int64 time_stamp0 = webrtc::TickTime::MillisecondTimestamp();
		//}
		ret = kAudio;
#endif
	}
#if 1
	pRead->mux->read_critsect->Leave();
#else
	if (g_cb)
	{
		g_cb(&pRead->mux->read_critsect, 1);
	}
#endif
	av_free_packet(&pkt);
	return ret;
}
#else
int ffmpegReadStream()
{
	int ret = 0;
	char *filename = "";
	av_register_all();

	AVFormatContext *pFormatCtx;

	// Open video file  

	if (av_open_input_file(&pFormatCtx, filename, NULL, 0, NULL) != 0)
		return -1; // Couldn't open file  

	if (av_find_stream_info(pFormatCtx)<0)
		return -1; // Couldn't find stream information  

	int i = 0;
	int videoStream = -1;
	AVCodecContext *pCodecCtx = NULL;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO)
		{
			videoStream = i;
			break;
		}
	}

	if (videoStream == -1)
		return -1; // Didn't find a video stream  

	// Get a pointer to the codec context for the video stream  
	pCodecCtx = pFormatCtx->streams[videoStream]->codec;

	AVCodec *pCodec = NULL;

	// Find the decoder for the video stream  
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

	if (pCodec == NULL)
	{
		fprintf(stderr, "Unsupported codec!\n");
		return -1; // Codec not found     
	}

	// Open codec     
	if (avcodec_open(pCodecCtx, pCodec)<0)
		return -1; // Could not open codec  


	AVFrame *pFrame, *pFrameRGB;
	// Allocate video frame  
#ifdef FFMPEG_V_30
    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();
#else
	pFrame = avcodec_alloc_frame();
	pFrameRGB = avcodec_alloc_frame();
#endif
	if (pFrameRGB == NULL)
		return -1;

	uint8_t *buffer;
	int numBytes;
	// Determine required buffer size and allocate buffer  
	numBytes = avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
	buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
	avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);

	int frameFinished;

	AVPacket packet;

	i = 0;

	while (av_read_frame(pFormatCtx, &packet) >= 0)
	{
		// Is this a packet from the video stream?        
		if (packet.stream_index == videoStream)
		{
			// Decode video frame     
			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);// packet.data, packet.size);
			// Did we get a video frame?      
			if (frameFinished)
			{
				// Convert the image from its native format to RGB    
				//img_convert2((AVPicture *)pFrameRGB, PIX_FMT_RGB24, (AVPicture*)pFrame, pCodecCtx->pix_fmt,pCodecCtx->width, pCodecCtx->height);

				// Save the frame to disk         
				//if (++i <= 100) SaveFrame(pFrameRGB, pCodecCtx->width, pCodecCtx->height, i);

			}

		}

		// Free the packet that was allocated by av_read_frame  
		av_free_packet(&packet);
	}


	// Free the RGB image     
	av_free(buffer);

	av_free(pFrameRGB);

	// Free the YUV frame     
	av_free(pFrame);

	// Close the codec    
	avcodec_close(pCodecCtx);

	//av_close_input_file(pFormatCtx);


	return 0;
	return ret;
}
#endif
/////////////////////////////////////////////////////////////////////////////////////
void FF_FACTORY_SET_CB(void *cb)
{
	ffmpegFactorySetCb(cb);
}
int FF_FACTORY_CREATE2(void** pHandle, int flag)
{
	return ffmpegFactoryInit(pHandle, flag);
}
int FF_FACTORY_CREATE(void** pHandle, int flag)
{
	return ffmpegFactoryInit(pHandle, flag);
}
void FF_FACTORY_STOP(void)
{
	ffmpegFactoryStop();
}
void FF_FACTORY_DELETE(void* pHandle)
{
	ffmpegFactoryRelease(pHandle);
}
int FF_AVCODEC_CREATE(void** pHandle)
{
	
#ifndef GXH_TEST
	FILE *fp = NULL;
	char cLogId[32] = "1";
	int logId = LogId(cLogId);
#ifdef _WIN32
	char dirName[256] = "log/webrtc_log-";
	strcat(dirName, cLogId);
	strcat(dirName, ".txt");
#elif defined(__ANDROID__)
	char dirName[256] = "/sdcard/webrtc_log-";
	strcat(dirName, cLogId);
	strcat(dirName, ".txt");
#elif defined(WEBRTC_IOS) || defined(WEBRTC_MAC) || defined(IOS)
    char dirName[256] = "webrtc_log-";
	strcat(dirName, cLogId);
	strcat(dirName, ".txt");
    char *pNmae = MAKE_FILE_NAME(dirName);//makePreferencesFilename("webrtc_log.txt");
    strcpy(dirName, pNmae);
    free(pNmae);
#elif defined(WEBRTC_LINUX)
    printf("gxhtest: FF_AVCODEC_CREATE: linux version \n");
    char dirName[256] = "log/webrtc_log-";
    strcat(dirName, cLogId);
    strcat(dirName, ".txt");
#elif defined(WEBRTC_MAC)
    printf("gxhtest: FF_AVCODEC_CREATE: mac version \n");
    char dirName[256] = "log/webrtc_log-";
    strcat(dirName, cLogId);
    strcat(dirName, ".txt");
#else
    char dirName[256] = "webrtc_log-";
	strcat(dirName, cLogId);
	strcat(dirName, ".txt");
#endif
	char con_text[256] = "voice start log:";
	if (logfp == NULL) {
		logfp = fopen(dirName, "wb");
		fp = logfp;
		if (logfp){
			fputs(con_text, fp);  fflush(fp);
			fprintf(fp, "%s \n", con_text); fflush(fp);
			fprintf(fp, "%s \n", dirName); fflush(fp);
		}
	}
	fp = logfp;//fopen(dirName,"wb");
#endif
#if 1
	g_critsect->Enter();
#else
	if (g_cb)
	{
		g_cb(&g_critsect, 0);
	}
#endif
	if (g_ffFactory == NULL)
	{
		g_ffFactory = (void *)&ffFactory;
	}
	int ret = CreateMediaStream(pHandle);
#if 1
	g_critsect->Leave();
#else
	if (g_cb)
	{
		g_cb(&g_critsect, 1);
	}
#endif
	return ret;
}
int FF_AVCODEC_DELETE(void* pHandle)
{
	int idx = *((int *)pHandle);
#if 1
	g_critsect->Enter();
#else
	if (g_cb)
	{
		g_cb(&g_critsect, 0);
	}
#endif
	int ret = Sub2FFmpegFactory(idx);
#if 1
	g_critsect->Leave();
#else
	if (g_cb)
	{
		g_cb(&g_critsect, 1);
	}
#endif
	return ret;
}
//extern void SetCodeType(void *hnd, int type);
int FF_CODEC_INIT(void *hnd, int codec_flag)
{
#if 1
	g_critsect->Enter();
#else
	if (g_cb)
	{
		g_cb(&g_critsect, 0);
	}
#endif
	SetCodeType(hnd, codec_flag);
	if (logfp) { fprintf(logfp, "MediaStreamOpen 0 %d \n", codec_flag); fflush(logfp); }
	int ret = 0;
	if (codec_flag == kAVDecStream)
	{
		MediaReadStreamOpen(hnd, codec_flag);
	}
	else
	{
		ret = MediaStreamOpen(hnd, codec_flag);
	}
	if (logfp) { fprintf(logfp, "MediaStreamOpen end %d \n", ret); fflush(logfp); }
#if 1
	g_critsect->Leave();
#else
	if (g_cb)
	{
		g_cb(&g_critsect, 1);
	}
#endif
	return ret;
}
//void FF_CODEC_CLOSE(void *hnd)
//{
//	int idx = *((int *)hnd);
//	Sub2FFmpegFactory(idx);
//	//MediaStreamRelease(hnd);
//}
int FF_READ_FRAM(void *hnd, unsigned char *buf, int *len, int *isKeyFrame, int *width, int *height)
{
	int ret = -1;
	MediaStream *mst = (MediaStream *)hnd;
	CodecParams *params = &mst->params;
	//int EncOrDec = !(codec_flag & 1);
	//int AudioOrVideo = ((codec_flag >= kAudioEncRaw) && (codec_flag < kAVEnc));
	int read_flag = strcmp(params->in_filename, "");
	unsigned int out_flag = params->out_flag;
	
	if (read_flag)
	{
		CoreStream *pCodec = mst->ReadSt;
		ret = MediaReadFrame(mst, buf, len, isKeyFrame, width, height);
	}
	return ret;
}
int FF_READ_FRAM2(void *hnd, unsigned char *buf, int *len, int *isKeyFrame, int *width, int *height, int64_t *timeStamp)
{
	int ret = -1;
	MediaStream *mst = (MediaStream *)hnd;
	CodecParams *params = &mst->params;
	//int EncOrDec = !(codec_flag & 1);
	//int AudioOrVideo = ((codec_flag >= kAudioEncRaw) && (codec_flag < kAVEnc));
	int read_flag = strcmp(params->in_filename, "");
	unsigned int out_flag = params->out_flag;

	if (read_flag)
	{
		CoreStream *pCodec = mst->ReadSt;
		ret = MediaReadFrame2(mst, buf, len, isKeyFrame, width, height, timeStamp);
	}
	return ret;
}
int FF_CODEC_CODEDED(void *hnd, char * inBuf[3], int len, char *outBuf[3], int oLen, int *frameType, int codec_flag)
{
	int ret = 0;
	MediaStream *mst = (MediaStream *)hnd;
	CodecParams *params = &mst->params;
	int EncOrDec = !(codec_flag & 1);
	int AudioOrVideo = ((codec_flag >= kAudioEncRaw) && (codec_flag < kAVEnc));
	int read_flag = strcmp(params->in_filename, "");
	unsigned int out_flag = params->out_flag;
    int frame_type = *frameType;

	int64_t time = av_gettime() + 800 * 1000;
	char cTime0[64] = "";
	FF_GET_TIME2(time, cTime0, 0);
	//printf("FF_CODEC_CODEDED: T0= %s \n", cTime0);

	if(read_flag)
	{
		CoreStream *pCodec = mst->ReadSt;
		//ret = CodecCodeded(mst, pCodec, inBuf, len, outBuf, oLen,frame_type,codec_flag);
	}
	if(!out_flag || !EncOrDec)
	{
		CoreStream *pCodec = mst->CodecSt;
        //fprintf(logfp,"CodecCodeded 0 %d \t %d \n",out_flag, EncOrDec); fflush(logfp);
		pCodec->debug_flag = params->debugInfo;
		ret = CodecCodeded(mst, pCodec, inBuf, len, outBuf, oLen,frame_type,codec_flag);
        //fprintf(logfp,"CodecCodeded 1 %d \t %d \n",out_flag, EncOrDec); fflush(logfp);
	}
	else if(out_flag)
	{
		int coded[6] = {};
		for(int i = 0; i < 6; i++)
		{
			if(!(out_flag & (1 << i)))
				continue;
			CoreStream *pCodec = mst->WriteSt[i];
			pCodec->debug_flag = params->debugInfo;
			if(EncOrDec)
			{
				if(!AudioOrVideo)
				{
					if(!coded[i%3])
					{
						if(pCodec->levelId)//!pCodec->AudioOrVideo
						{
							AVCodecContext *c = pCodec->codecCtx;
							if(pCodec->pic.data[0] == NULL)
							{
								avpicture_alloc(&pCodec->pic, c->pix_fmt, c->width, c->height);
								pCodec->alloc_pic = pCodec->pic;
							}							
							int src_width = params->width; 
							int src_height = params->height;
							int dst_width = c->width; 
							int dst_height = c->height;

							if(pCodec->scale == NULL)
								pCodec->scale = (ffmpegScale *)ffmpeg_create(pCodec->scale);
							if(pCodec->scale)
							{
								void *scale = pCodec->scale;
								unsigned char *sY = (unsigned char *)inBuf[0];
								unsigned char *sU = (unsigned char *)inBuf[1];
								unsigned char *sV = (unsigned char *)inBuf[2];
								unsigned char *dY = (unsigned char *)pCodec->pic.data[0];
								unsigned char *dU = (unsigned char *)pCodec->pic.data[1];
								unsigned char *dV = (unsigned char *)pCodec->pic.data[2];
								int stride_s = src_width;
								int stride_d = dst_width;
#if 0
								static FILE *fp = NULL;
#ifdef _WIN32
                                if(!fp) fp = fopen("e://works//test//video-dist.yuv","wb");
#elif defined(__ANDROID__)
                                if(!fp) fp = fopen("/sdcard/video-dist.yuv","wb");
#elif defined(_IOS_)
                                f(!fp) fp = fopen("video-dist.yuv","wb");
#else
#endif
								
								if(fp)
								{
									fwrite(sY,1,src_width * src_height, fp);
									fwrite(sU,1,((src_width * src_height) >> 2), fp);
									fwrite(sV,1,((src_width * src_height) >> 2), fp);				
								}		
#endif
								ffmpeg_params(scale,AV_PIX_FMT_YUV420P, sY, sU, sV, src_width, stride_s, src_height, 0);
								ffmpeg_params(scale,AV_PIX_FMT_YUV420P, dY, dU, dV, dst_width , stride_d, dst_height, 1);
								ffmpeg_scaleInit(scale);
								ffmpeg_scaler(scale);

							}
						}
						
						char context[255] = "video encode: ";
						strcat(context, cTime0);
						sprintf(&context[strlen(context)], "  %d", pCodec->codecCtx->frame_number);
						//printf("T0= %s \n", context);
                        //fprintf(logfp,"CodecCodeded video 0 %d \t %d \n",out_flag, EncOrDec); fflush(logfp);
						ret = CodecCodeded(mst, pCodec, inBuf, len, outBuf, oLen,frame_type,codec_flag);
                        //fprintf(logfp,"CodecCodeded video 1 %d \t %d \n",out_flag, EncOrDec); fflush(logfp);
						//
						if(pCodec->mux)
						{
#if 1
							pCodec->mux->write_critsect->Enter();
#else
							if (g_cb)
							{
								g_cb(&pCodec->mux->write_critsect, 0);
							}
#endif
							if(pCodec->FileOrStream)
							{
								if(pCodec->mux->AVWriteFlag == 3) // test
								if(pCodec->mux->RecordStatus == kRecordStart && pCodec->mux->WriteHead == 0 && frame_type)
								{
									WriteStart(pCodec, -1);
								}
								if(!is_time(pCodec))
								{						
									//pCodec->pkt.test_pts[0] = time;
									frame_write2(mst, pCodec, &pCodec->pkt);
								}
								else
								{
									if(pCodec->mux->WriteHead)
									{
										int ret2 = WriteTail(pCodec->mux->fmtctx);
										pCodec->mux->WriteHead = 0;
										pCodec->mux->WriteAudio = 0;
										pCodec->mux->time0 = 0;
										pCodec->mux->start_time = 0;
										pCodec->mux->RecordStatus = 0;//test
									}						
								}
							}
							else
							{				
								//pCodec->pkt.test_pts[0] = time;
								frame_write2(mst, pCodec, &pCodec->pkt);
									
							}
#if 1
							pCodec->mux->write_critsect->Leave();
#else
							if (g_cb)
							{
								g_cb(&pCodec->mux->write_critsect, 1);
							}
#endif
						}
						///av_free_packet(&pCodec->pkt);//!!!
						coded[i%3] = 1;
					}
					else
					{
						if(EncOrDec)
						{
							CoreStream *pCodec0 = mst->WriteSt[i%3];
							//AVPacket pkt = { 0 };
							//av_init_packet(&pkt);
							//pkt = pCodec0->pkt;						
							if(pCodec->mux)
							{
#if 1
								pCodec->mux->write_critsect->Enter();
#else
								if (g_cb)
								{
									g_cb(&pCodec->mux->write_critsect, 0);
								}
#endif
								if(pCodec->FileOrStream)
								{
									if(pCodec->mux->RecordStatus == kRecordStart && pCodec->mux->WriteHead == 0 && pCodec0->pkt.flags)
									{
										WriteStart(pCodec, -1);
									}
									if(!is_time(pCodec))
									{
										//pCodec->pkt.test_pts[0] = time;
										frame_write2(mst, pCodec, &pCodec0->pkt);
									}
									else
									{
										if(pCodec->mux->WriteHead)
										{
											int ret2 = WriteTail(pCodec->mux->fmtctx);
											pCodec->mux->WriteHead = 0;
											pCodec->mux->WriteAudio = 0;
											pCodec->mux->time0 = 0;
											pCodec->mux->start_time = 0;
											pCodec->mux->RecordStatus = 0;//test
										}						
									}
								}
								else
								{
									//pCodec->pkt.test_pts[0] = time;
									frame_write2(mst, pCodec, &pCodec0->pkt);
								}
#if 1
								pCodec->mux->write_critsect->Leave();
#else
								if (g_cb)
								{
									g_cb(&pCodec->mux->write_critsect, 1);
								}
#endif
							}
							//av_free_packet(&pkt);
						}
					}
				}
				else//audio
				{
					if(!coded[0])
					{
                        //fprintf(logfp,"CodecCodeded audio 0 %d \t %d \n",out_flag, EncOrDec); fflush(logfp);
						ret = CodecCodeded(mst, pCodec, inBuf, len, outBuf, oLen,frame_type,codec_flag);
                        //fprintf(logfp,"CodecCodeded audio 1 %d \t %d \n",out_flag, EncOrDec); fflush(logfp);
						if(pCodec->mux)
						{
#if 1
							pCodec->mux->write_critsect->Enter();
#else
							if (g_cb)
							{
								g_cb(&pCodec->mux->write_critsect, 0);
							}
#endif
							if(pCodec->FileOrStream)
							{
								if(!is_time(pCodec))
								{						
									//pCodec->pkt.test_pts[0] = time;
									frame_write2(mst, pCodec, &pCodec->pkt);
									pCodec->mux->WriteAudio = 1;
								}
								else
								{
									if(pCodec->mux->WriteHead)
									{
										int ret2 = WriteTail(pCodec->mux->fmtctx);
										pCodec->mux->WriteHead = 0;
										pCodec->mux->WriteAudio = 0;
										pCodec->mux->time0 = 0;
										pCodec->mux->start_time = 0;
										pCodec->mux->RecordStatus = 0;//test
									}						
								}
							}
							else
							{				
								//pCodec->pkt.test_pts[0] = time;
								if (ret > 0)
								{
									frame_write2(mst, pCodec, &pCodec->pkt);
									pCodec->mux->WriteAudio = 1;
								}

							}
#if 1
							pCodec->mux->write_critsect->Leave();
#else
							if (g_cb)
							{
								g_cb(&pCodec->mux->write_critsect, 1);
							}
#endif
						}
						//av_free_packet(&pCodec->pkt);//!!!!
						coded[0] = i + 1;
					}
					else
					{
						CoreStream *pCodec0 = mst->WriteSt[coded[0] - 1];
						if(pCodec->mux)
						{
#if 1
							pCodec->mux->write_critsect->Enter();
#else
							if (g_cb)
							{
								g_cb(&pCodec->mux->write_critsect, 0);
							}
#endif
							if(pCodec->FileOrStream)
							{
								if(!is_time(pCodec))
								{
									//pCodec->pkt.test_pts[0] = time;
									frame_write2(mst, pCodec, &pCodec0->pkt);
									pCodec->mux->WriteAudio = 1;
								}
								else
								{
									if(pCodec->mux->WriteHead)
									{
										int ret2 = WriteTail(pCodec->mux->fmtctx);
										pCodec->mux->WriteHead = 0;
										pCodec->mux->WriteAudio = 0;
										pCodec->mux->time0 = 0;
										pCodec->mux->start_time = 0;
										pCodec->mux->RecordStatus = 0;//test
									}						
								}
							}
							else
							{
								//pCodec->pkt.test_pts[0] = time;
								frame_write2(mst, pCodec, &pCodec0->pkt);
								pCodec->mux->WriteAudio = 1;
							}
#if 1
							pCodec->mux->write_critsect->Leave();
#else
							if (g_cb)
							{
								g_cb(&pCodec->mux->write_critsect, 1);
							}
#endif
						}
					}
					///av_free_packet(&pCodec->pkt);//!!!!
				}
			}
			else
			{

			}
			
		}
	}
	for (int i = 0; i < 6; i++)
	{
		if (!(out_flag & (1 << i)))
			continue;
		CoreStream *pCodec = mst->WriteSt[i];
		av_free_packet(&pCodec->pkt);
	}
	*frameType = frame_type;
	return ret;
}
int FF_WRITE_TAIL(void *hnd, int codec_flag)
{
	int ret = 0;
	MediaStream *mst = (MediaStream *)hnd;
	CodecParams *params = &mst->params;
	int EncOrDec = !(codec_flag & 1);
	int read_flag = strcmp(params->in_filename, "");
	unsigned int out_flag = params->out_flag;

	if(read_flag)
	{
		CoreStream *pCodec = mst->ReadSt;
		//ret = CodecCodeded(mst, pCodec, inBuf, len, outBuf, oLen,frame_type,codec_flag);
	}
	if(!out_flag || !EncOrDec)
	{
		CoreStream *pCodec = mst->CodecSt;
		//if(pCodec->WriteIsCpy)
		//{
		//	ret = WriteTail(pCodec->fmtctx);
		//}
	}
	else if(out_flag)
	{
		for(int i = 0; i < 6; i++)
		{
			if(!(out_flag & (1 << i)))
				continue;
			CoreStream *pCodec = mst->WriteSt[i];
			if(pCodec->FileOrStream && pCodec->mux)
			{
				pCodec->mux->WriteHead = 0;
				pCodec->mux->WriteAudio = 0;
				if(pCodec->WriteIsCpy == 1)
				{
#if 1
					pCodec->mux->write_critsect->Enter();
#else
					if (g_cb)
					{
						g_cb(&pCodec->mux->write_critsect, 0);
					}
#endif
					ret = WriteTail(pCodec->mux->fmtctx);					
#if 1
					pCodec->mux->write_critsect->Leave();
#else
					if (g_cb)
					{
						g_cb(&pCodec->mux->write_critsect, 1);
					}
#endif
				}				
			}
		}
	}

	return ret;
}
void FF_CODEC_SET_PARAMS(void *hnd, char *cParams)
{
	MediaStreamSetParams(hnd, cParams);
}
void FF_CODEC_SET_WIDTH(void *hnd, int width, int levelId)
{
	MediaStream* pCodec = (MediaStream*)hnd;

	if (levelId == 0)
	{
		pCodec->params.width = width;
	}
	else if (levelId == 1)
	{
		pCodec->params.width1 = width;
	}
	else if (levelId == 2)
	{
		pCodec->params.width2 = width;
	}
}
void FF_CODEC_SET_HEIGHT(void *hnd, int height, int levelId)
{
	MediaStream* pCodec = (MediaStream*)hnd;

	if (levelId == 0)
	{
		pCodec->params.height = height;
	}
	else if (levelId == 1)
	{
		pCodec->params.height1 = height;
	}
	else if (levelId == 2)
	{
		pCodec->params.height2 = height;
	}
}
int FF_CODEC_GET_FILENAME(void *hnd, char *oStr)
{
	int ret = 0;
	MediaStream* mst = (MediaStream*)hnd;

	if (strcmp(mst->params.in_filename, ""))
	{
		if (oStr)
		{
			strcpy(oStr, mst->params.in_filename);
		}
		ret = 1;
	}
	
	return ret;
} 
void FF_CODEC_SET_FILENAME(void *hnd, char *name, int flag)
{
	MediaStream* mst = (MediaStream*)hnd;

	if (flag == 0 && mst)
	{
		strcpy(mst->params.in_filename, name);
	}
}
void FF_CODEC_SET_OUT_FILENAME(void *hnd, char *name, int flag)
{
	MediaStream* mst = (MediaStream*)hnd;

	if (flag == 0 && mst)
	{
		strcpy(mst->params.out_filename, name);
	}
}
void FF_CODEC_SET_OUT_STREAMNAME(void *hnd, char *name, int flag)
{
	MediaStream* mst = (MediaStream*)hnd;

	if (flag == 0 && mst)
	{
		strcpy(mst->params.out_streamname, name);
	}
}
void FF_CODEC_SET_RENDEWWIN(void *hnd, int orgX, int orgY, int winWidth, int winHeight, int showWidth, int showHeight, int mode)
{
	MediaStream* mst = (MediaStream*)hnd;
#if 1
	g_critsect->Enter();
#else
	if (g_cb)
	{
		g_cb(&g_critsect, 0);
	}
#endif
    mst->reset = 1;
	mst->winWidth = winWidth;
	mst->winHeight = winHeight;
	mst->orgX = orgX;
	mst->orgY = orgY;
	mst->showWidth = showWidth;
	mst->showHeight = showHeight;
	mst->mode = mode;
#if 1
	g_critsect->Leave();
#else
	if (g_cb)
	{
		g_cb(&g_critsect, 1);
	}
#endif
}
void FF_CODEC_SET_CHANID(void *hnd, int chanId)
{
	MediaStream* mst = (MediaStream*)hnd;
	mst->ChanId = chanId;
}
void FF_CODEC_SET_STREAMID(void *hnd, int streamId)
{
	MediaStream* mst = (MediaStream*)hnd;
	mst->streamId = streamId;
}
void FF_CODEC_SET_RECORDSTATUS(void *hnd, int RecordStatus, int flag)
{
	MediaStream* mst = (MediaStream*)hnd;
	for (int i = 0; i < 6; i++)
	{
		CoreStream *pCodec = mst->WriteSt[i];
		if ((1 << i) & flag)
		{
			if (pCodec && pCodec->mux && pCodec->mux->write_critsect)
			{
				pCodec->mux->write_critsect->Enter();
				pCodec->mux->RecordStatus = RecordStatus;
				pCodec->mux->write_critsect->Leave();
			}
		}
	}
}
void FF_CODEC_SET_WATERMARK(void *hnd, int left, int right, int top, int bottom, float alpha, int fit )
{
	MediaStream* mst = (MediaStream*)hnd;
	mst->waterMarkParams.left = left;
	mst->waterMarkParams.right = right;
	mst->waterMarkParams.top = top;
	mst->waterMarkParams.bottom = bottom;
	mst->waterMarkParams.alpha = alpha;
	mst->waterMarkParams.fit = fit ? true : false;
}
void FF_CODEC_SET_FILEPATH(void *hnd, char *filename, int type)
{
	MediaStream* mst = (MediaStream*)hnd;
	if (mst && filename)
	{
		strcpy(mst->jpgFile[type], filename);
	}
}
#if 0
int FF_AVCODEC_DELETE(void* pHandle)
{
    FFCodec* pCodec = NULL;
    
    if(pHandle == NULL)
        return 0;
    pCodec = (FFCodec*)pHandle;
    
    //delete pDec;
    free(pCodec);
    return 1;
}
void FF_CODEC_CLOSE(void *hnd)
{
    FFCodec* pCodec = (FFCodec*)hnd;
    if(pCodec->codecType < kAudioEncRaw || pCodec->codecType >= kAVEnc)
        video_close(pCodec);
    if(pCodec->codecType >= kAudioEncRaw )
        audio_close(pCodec);
    if(pCodec->json_obj)
    {
        Json1DRelease(pCodec->json_obj);
        free(pCodec->json_obj);
    }
}
#endif

#if 1
void GetWaterMarkImage(void *hnd, char *infile, char *outBuf0[3], int oWidth, int oHeight)
{
	MediaStream* mst = (MediaStream*)hnd;
	//read jpeg
	//decode
	int width = 0;
	int height = 0;
	if (mst->img[kWaterMark][kInData].data == NULL)
	{
		FF_JpegDecode2(infile, NULL, 0, &mst->img[kWaterMark][kInData].data, &width, &height);
		mst->img[kWaterMark][kInData].width = width;
		mst->img[kWaterMark][kInData].height = height;
	}
	else
	{
		width = mst->img[kWaterMark][kInData].width;
		height = mst->img[kWaterMark][kInData].height;
	}
	//scale
	void *_scalHandle = NULL;
	int srcSize[3] = {};
	int dstSize[3] = {};
	char *outBuf[3] = {};
	char *inBuf[3] = {};
	srcSize[0] = width;
	srcSize[1] = height;
	srcSize[2] = width;
	inBuf[0] = (char *)mst->img[kWaterMark][kInData].data;
	inBuf[1] = (char *)&mst->img[kWaterMark][kInData].data[width * height];
	inBuf[2] = (char *)&mst->img[kWaterMark][kInData].data[width * height + (width >> 1) * (height >> 1)];
	if (mst->waterMarkParams.fit == true)
	{
		dstSize[0] = width;// mst->params.width;
		dstSize[1] = height;// mst->params.height;
	}
	else
	{
		dstSize[0] = mst->waterMarkParams.right - mst->waterMarkParams.left;
		dstSize[1] = mst->waterMarkParams.top - mst->waterMarkParams.bottom;
	}
	
	dstSize[2] = oWidth;// mst->params.width;;
	outBuf[0] = (char *)&outBuf0[0][mst->waterMarkParams.top * oWidth + mst->waterMarkParams.left];
	outBuf[1] = (char *)&outBuf0[1][(mst->waterMarkParams.top >> 1) * (oWidth >> 1) + (mst->waterMarkParams.left >> 1)];
	outBuf[2] = (char *)&outBuf0[2][(mst->waterMarkParams.top >> 1) * (oWidth >> 1) + (mst->waterMarkParams.left >> 1)];
	
	_scalHandle = FF_SCALE(_scalHandle, inBuf, srcSize, outBuf, dstSize);
	FF_SCALE_DELETE(_scalHandle);	_scalHandle = NULL;
}
void GetInsetImage(void *hnd, char *infile, char *outBuf[3], int oWidth, int oHeight)
{
	MediaStream* mst = (MediaStream*)hnd;
	//read jpeg
	//decode
	int width = 0;
	int height = 0;
	if (mst->img[kInset][kInData].data == NULL)
	{
		FF_JpegDecode2(infile, NULL, 0, &mst->img[kInset][kInData].data, &width, &height);
		mst->img[kInset][kInData].width = width;
		mst->img[kInset][kInData].height = height;
	}
	else
	{
		width = mst->img[kInset][kInData].width;
		height = mst->img[kInset][kInData].height;
	}
	//scale
	void *_scalHandle = NULL;
	int srcSize[3] = {};
	int dstSize[3] = {};
	//char *outBuf[3] = {};
	char *inBuf[3] = {};
	srcSize[0] = width;
	srcSize[1] = height;
	srcSize[2] = width;
	inBuf[0] = (char *)mst->img[kInset][kInData].data;
	inBuf[1] = (char *)&mst->img[kInset][kInData].data[width * height];
	inBuf[2] = (char *)&mst->img[kInset][kInData].data[width * height + (width >> 1) * (height >> 1)];
	dstSize[0] = mst->params.width;
	dstSize[1] = mst->params.height;
	dstSize[2] = mst->params.width;;
	//mst->waterMarkImg = new char[(mst->params.width * mst->params.height * 3) >> 1];
	_scalHandle = FF_SCALE(_scalHandle, inBuf, srcSize, outBuf, dstSize);
	FF_SCALE_DELETE(_scalHandle);	_scalHandle = NULL;
	
}
void *FF_SCALE_CREATE(void *handle)
{
	if (handle == NULL)
	{
		handle = (void *)ffmpeg_create(handle);
	}

	return (void *)handle;
}
void FF_SCALE_DELETE(void *handle)
{
	if (handle)
	{
		ffmpeg_close(handle);
	}
}
void *FF_SCALE(void *handle, char *inBuf[3], int srcSize[3], char *outBuf[3], int dstSize[3])
{
	if (handle == NULL)
	{
		handle = FF_SCALE_CREATE(handle);
	}
		
	if (handle)
	{
		void *scale = handle;
		unsigned char *sY = (unsigned char *)inBuf[0];
		unsigned char *sU = (unsigned char *)inBuf[1];
		unsigned char *sV = (unsigned char *)inBuf[2];
		unsigned char *dY = (unsigned char *)outBuf[0];
		unsigned char *dU = (unsigned char *)outBuf[1];
		unsigned char *dV = (unsigned char *)outBuf[2];
		int src_width = srcSize[0];
		int src_height = srcSize[1];
		int stride_s = srcSize[2];
		int dst_width = dstSize[0];
		int dst_height = dstSize[1];
		int stride_d = dstSize[2];
		//int stride_s = src_width;
		//int stride_d = dst_width;

		ffmpeg_params(scale, AV_PIX_FMT_YUV420P, sY, sU, sV, src_width, stride_s, src_height, 0);
		ffmpeg_params(scale, AV_PIX_FMT_YUV420P, dY, dU, dV, dst_width, stride_d, dst_height, 1);
		ffmpeg_scaleInit(scale);
		ffmpeg_scaler(scale);

	}
	return handle;
}

#endif

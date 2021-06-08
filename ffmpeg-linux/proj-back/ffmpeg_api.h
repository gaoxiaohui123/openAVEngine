//edited by gxh
#ifndef FFMPEG_API_H_
#define FFMPEG_API_H_

#ifdef __cplusplus
#define EXTERNC extern "C"
EXTERNC {
#else
#define EXTERNC
#endif
    
#ifdef _WIN32
#define FFAVCODEC_API EXTERNC __declspec(dllexport)
#else
//extern "C"
#define FFAVCODEC_API EXTERNC __attribute__ ((__visibility__("default")))
#endif

#define FFMPEG_V_30

#ifndef _WIN32
#define MAX_PATH        256
#endif
#define MAX_USHOT		((1 << 16) - 1)
#define MUT_SIZE		1400//0
#define PARAMS_SIZE		4096
#define MAX_RTP_NUM		100
#define	MAX_BITS_BUF_SIZE	512000
#define H264_PLT            96
//
#define FPS				25
#define RDIP			"127.0.0.1"
#define	RDPORT0			19000
#define	RDPORT1			19001
#define OUTWIDTH		640//704
#define OUTHEIGHT		480//576
#define SPLIT_LEN		(1000*60000)//ms

enum { kInset, kWaterMark, kWavFile };
enum { kIsFile, kIsStream };
enum RecordStatus
{
	kRecordStop, 
	kRecordPaus,
	kRecordStart,
	kRecordInSet
};
enum CodecType
{
	kVideoEncRaw,//0
	kVideoDecRaw,//1
	kVideoEncStream,	
	kVideoDecStream,	
	kAudioEncRaw,		
	kAudioDecRaw,		
	kAudioEncStream,	
	kAudioDecStream,	
	kAVEnc,				
	kAVDec,				
	kAVEncStream,		
	kAVDecStream,		
};
enum CodecAVID
{
	kVideoH264,
	kVideoOpenH264,
	kVideoH265,
	kVideoVP8,
	kAudioAac,
	kAudioAacPlus,
};
enum PicType
{
	PIC_TYPE_AUTO = 0,        
	PIC_TYPE_IDR,           
	PIC_TYPE_I,             
	PIC_TYPE_P,             
	PIC_TYPE_BREF,            
	PIC_TYPE_B,             
	PIC_TYPE_KEYFRAME,   
};

FFAVCODEC_API
int FF_TEST_MAIN(int flag);


FFAVCODEC_API
void FF_FACTORY_SET_CB(void *cb);
FFAVCODEC_API
int FF_FACTORY_CREATE(void** pHandle, int flag);
FFAVCODEC_API
int FF_FACTORY_CREATE2(void** pHandle, int flag);
FFAVCODEC_API
void FF_FACTORY_DELETE(void* pHandle);
FFAVCODEC_API
int FF_WRITE_TAIL(void *hnd, int codec_flag);
///////////////////////////////////////////////////////////////////////////////////
FFAVCODEC_API
int FF_RTP_PACKAGE(char *inBuf, int size, char *outBuf, short *rtpSize, int isKeyFrame, int width, int height, unsigned short *seq_num, unsigned short *last_seq_num);
FFAVCODEC_API 
int FF_RTP_UNPACKAGE(char *inBuf, short *rtpSize, int count, int rtpLen, char *outBuf, int *oSize, unsigned short *last_seq_num);
FFAVCODEC_API
int FF_RTP_UNPACKAGE2(char *inBuf, short *rtpSize, int count, int rtpLen, char *outBuf, int *oSize, int *frmNumber);
FFAVCODEC_API 
int FF_AVCODEC_CREATE(void** pHandle);
FFAVCODEC_API 
int FF_AVCODEC_DELETE(void* pHandle);
FFAVCODEC_API 
int FF_CODEC_INIT(void *hnd, int codec_flag);
FFAVCODEC_API
int FF_CODEC_CODEDED(void *hnd, char * inBuf[3], int len, char *outBuf[3], int oLen, int *pic_type, int codec_flag);
//FFAVCODEC_API 
//void FF_CODEC_CLOSE(void *hnd);
FFAVCODEC_API
void FF_CODEC_SET_PARAMS(void *hnd, char *cParams);
FFAVCODEC_API
void FF_GET_TIIME(char *name);
FFAVCODEC_API
int FF_READ_FRAM(void *hnd, unsigned char *buf, int *len, int *isKeyFrame, int *width, int *height);
FFAVCODEC_API
int FF_READ_FRAM2(void *hnd, unsigned char *buf, int *len, int *isKeyFrame, int *width, int *height, long long *timeStamp);
FFAVCODEC_API
void FF_CODEC_SET_WIDTH(void *hnd, int width, int levelId);
FFAVCODEC_API
void FF_CODEC_SET_HEIGHT(void *hnd, int height, int levelId);
FFAVCODEC_API
void FF_CODEC_SET_FILENAME(void *hnd, char *name, int flag);
FFAVCODEC_API
int FF_CODEC_GET_FILENAME(void *hnd, char *oStr);
FFAVCODEC_API
void FF_CODEC_SET_OUT_FILENAME(void *hnd, char *name, int flag);
FFAVCODEC_API
void FF_CODEC_SET_OUT_STREAMNAME(void *hnd, char *name, int flag);
FFAVCODEC_API
void FF_CODEC_SET_CHANID(void *hnd, int chanId);
FFAVCODEC_API
void FF_CODEC_SET_STREAMID(void *hnd, int streamId);
FFAVCODEC_API
int FF_CODEC_RENEWURL(int chanId, char *inFile, int InOrOut, int FileOrStream, int type);
FFAVCODEC_API
void FF_CODEC_SET_RENDEWWIN(void *hnd, int orgX, int orgY, int winWidth, int winHeight, int showWidth, int showHeight, int mode);
FFAVCODEC_API
void *FF_CODEC_GET_HND(int chanId, int type);
FFAVCODEC_API
void FF_CODEC_SET_RECORDSTATUS(void *hnd, int RecordStatus, int flag);
FFAVCODEC_API
void FF_CODEC_SET_WATERMARK(void *hnd, int left, int right, int top, int bottom, float alpha, int fit);
FFAVCODEC_API
void FF_CODEC_SET_FILEPATH(void *hnd, char *filename, int type);
FFAVCODEC_API
int FFResample(void **hnd, unsigned char *inBuf, int inLen, unsigned char *outBuf, int outLen, int inSampleRate);
FFAVCODEC_API
void FFResampleClose(void *handle);
FFAVCODEC_API
int FFScale(void **handle, char *inBuf[3], char *outBuf[3], int stride, int width, int height, int dstWidth, int dstHeight);
FFAVCODEC_API
void FFScaleClose(void *handle);
FFAVCODEC_API
int FF_JpegEncode(char *outPath, char *outBuf, int outSize, char *inBuf, int width, int height, int quality);
FFAVCODEC_API
int FF_JpegDecode(char *inPath, char *inBuf, int inSize, char *outBuf0, int *width, int *height);
FFAVCODEC_API
int FF_JpegDecode2(char *inPath, char *inBuf, int inSize, char **outBuf, int *width, int *height);
//
FFAVCODEC_API
void *FF_WAV_CREATE(void *handle);
FFAVCODEC_API
void FF_WAV_DELETE(void *handle);
FFAVCODEC_API
void FF_WAV_INIT(void *handle, const char *ofile, unsigned short int channels, unsigned long samplespersec, unsigned short int nBitsPerSamples);
FFAVCODEC_API
void FF_WAV_WRITE(void *handle, unsigned char * data, int len, int channel);
//
FFAVCODEC_API
void *FF_SCALE_CREATE(void *handle);
FFAVCODEC_API
void FF_SCALE_DELETE(void *handle);
FFAVCODEC_API
void *FF_SCALE(void *handle, char *inBuf[3], int srcSize[3], char *outBuf[3], int dstSize[3]);
FFAVCODEC_API
void FF_MCUTest(int chanId);
//


#ifdef _WIN32

int ICreateSpeexAECObj(void** pHnd);
int IDeleteSpeexAEObj(void* pHnd);
void ISpeexEchoInit(void *h, int channel);
int ISpeexEchoExecute(void *h, short *in, short* far_end, short *out);
//
FFAVCODEC_API
void IISpeexAECPlayPort(void **hnd, char *playBuf, int frameBytes);
FFAVCODEC_API
int IISpeexAECMicPort(void **hnd, char *micBuf, int frameBytes);
//
FFAVCODEC_API
void IIISpeexAECPlayPort(void **hnd, char *playBuf, int frameBytes);
FFAVCODEC_API
int IIISpeexAECMicPort(void **hnd, char *micBuf, int frameBytes);
#endif

#ifdef  __cplusplus
}
#endif

#endif
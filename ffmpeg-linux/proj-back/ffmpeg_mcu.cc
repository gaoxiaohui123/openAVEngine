//edited by gxh
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include <vector>
#include <list>
#include <map>
#include "webrtc/system_wrappers/interface/critical_section_wrapper.h"
#include "webrtc/system_wrappers/interface/event_wrapper.h"
#include "webrtc/system_wrappers/interface/thread_wrapper.h"
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
#include "ffmpeg_mcu.h"
#include "ffmpeg_filter.h"
#include "webrtc/modules/av_coding/codecs/ffmpeg/main/interface/ffmpeg_api.h"

#ifdef  __cplusplus    
}    
#endif
//宽高最小单元为2x2，即宽高必须为偶数，否则编码异常退出；
static char *cparams[8] =
{
"{\"genParams\":{\
\"debugInfo\":\"1\",\
\"audioName\":\"aac1\",\
\"videoName\":\"H264\",\
\"fec\":\"0\",\
\"nack\":\"0\",\
\"agc\":\"0\",\
\"denoise\":\"0\"\
},\
\"codecParams\":{\
\"out_flag\":\"9\",\
\"streamId\":\"1\",\
\"in_data_format\":\"0\",\
\"width\":\"640\",\
\"height\":\"480\",\
\"width1\":\"320\",\
\"height1\":\"240\",\
\"width2\":\"160\",\
\"height2\":\"120\",\
\"gop_size\":\"50\",\
\"mtu_size\":\"1400\",\
\"frame_rate\":\"15\",\
\"min_bits_rate\":\"256000\",\
\"max_bits_rate\":\"1024000\",\
\"out_data_format\":\"0\",\
\"video_port\":\"19000\",\
\"video_bits_rate\":\"512000\",\
\"video_bits_rate1\":\"128000\",\
\"video_bits_rate2\":\"64000\",\
\"video_codec_id\":\"28\",\
\"frame_size\":\"2048\",\
\"in_channel_count\":\"2\",\
\"in_sample_rate\":\"48000\",\
\"in_sample_fmt\":\"8\",\
\"out_channel_count\":\"2\",\
\"out_sample_rate\":\"48000\",\
\"out_sample_fmt\":\"1\",\
\"audio_port\":\"18001\",\
\"in_filename\":\"\",\
\"out_filename\":\"live.flv\",\
\"out_streamname\":\"http://192.168.120.95:8090/feed3.ffm\",\
\"sdp_filename\":\"video.sdp\",\
\"filePath\":\"\",\
\"ipaddr\":\"127.0.0.1\"}}\
",
"{\"genParams\":{\
\"debugInfo\":\"0\",\
\"audioName\":\"aac1\",\
\"videoName\":\"H264\",\
\"fec\":\"0\",\
\"nack\":\"0\",\
\"agc\":\"0\",\
\"denoise\":\"0\"\
},\
\"codecParams\":{\
\"out_flag\":\"0\",\
\"streamId\":\"1\",\
\"in_data_format\":\"0\",\
\"width\":\"640\",\
\"height\":\"480\",\
\"gop_size\":\"50\",\
\"mtu_size\":\"1400\",\
\"frame_rate\":\"25\",\
\"min_bits_rate\":\"128000\",\
\"max_bits_rate\":\"512000\",\
\"out_data_format\":\"0\",\
\"video_port\":\"19000\",\
\"video_bits_rate\":\"256000\",\
\"video_codec_id\":\"28\",\
\"frame_size\":\"2048\",\
\"in_channel_count\":\"2\",\
\"in_sample_rate\":\"48000\",\
\"in_sample_fmt\":\"8\",\
\"out_channel_count\":\"2\",\
\"out_sample_rate\":\"48000\",\
\"out_sample_fmt\":\"1\",\
\"audio_port\":\"19001\",\
\"out_filename\":\"\",\
\"out_streamname\":\"\",\
\"sdp_filename\":\"video.sdp\",\
\"filePath\":\"\",\
\"ipaddr\":\"127.0.0.1\"}}\
",
"{\"genParams\":{\
\"debugInfo\":\"4\",\
\"audioName\":\"aac1\",\
\"videoName\":\"H264\",\
\"fec\":\"0\",\
\"nack\":\"0\",\
\"agc\":\"0\",\
\"denoise\":\"0\"\
},\
\"codecParams\":{\
\"out_flag\":\"1\",\
\"streamId\":\"1\",\
\"in_data_format\":\"0\",\
\"width\":\"640\",\
\"height\":\"480\",\
\"gop_size\":\"50\",\
\"mtu_size\":\"1400\",\
\"frame_rate\":\"25\",\
\"min_bits_rate\":\"128000\",\
\"max_bits_rate\":\"512000\",\
\"out_data_format\":\"0\",\
\"video_port\":\"19000\",\
\"audio_bits_rate\":\"24000\",\
\"audio_codec_id\":\"86018\",\
\"frame_size\":\"2048\",\
\"in_channel_count\":\"2\",\
\"in_sample_rate\":\"48000\",\
\"in_sample_fmt\":\"8\",\
\"out_channel_count\":\"2\",\
\"out_sample_rate\":\"48000\",\
\"out_sample_fmt\":\"1\",\
\"audio_port\":\"18001\",\
\"in_filename\":\"\",\
\"out_filename\":\"live.flv\",\
\"out_streamname\":\"rtmp://127.0.0.1/live/live\",\
\"sdp_filename\":\"audio.sdp\",\
\"filePath\":\"\",\
\"ipaddr\":\"127.0.0.1\"}}\
",
"{\"genParams\":{\
\"debugInfo\":\"0\",\
\"audioName\":\"aac1\",\
\"videoName\":\"H264\",\
\"fec\":\"0\",\
\"nack\":\"0\",\
\"agc\":\"0\",\
\"denoise\":\"0\"\
},\
\"codecParams\":{\
\"out_flag\":\"0\",\
\"streamId\":\"1\",\
\"in_data_format\":\"0\",\
\"width\":\"640\",\
\"height\":\"480\",\
\"gop_size\":\"50\",\
\"mtu_size\":\"1400\",\
\"frame_rate\":\"25\",\
\"min_bits_rate\":\"128000\",\
\"max_bits_rate\":\"512000\",\
\"out_data_format\":\"0\",\
\"video_port\":\"19000\",\
\"audio_bits_rate\":\"24000\",\
\"audio_codec_id\":\"86018\",\
\"frame_size\":\"2048\",\
\"in_channel_count\":\"2\",\
\"in_sample_rate\":\"48000\",\
\"in_sample_fmt\":\"8\",\
\"out_channel_count\":\"2\",\
\"out_sample_rate\":\"48000\",\
\"out_sample_fmt\":\"1\",\
\"audio_port\":\"19001\",\
\"in_filename\":\"\"rtmp://127.0.0.1/live/live-0,\
\"out_filename\":\"\",\
\"out_streamname\":\"\",\
\"sdp_filename\":\"audio.sdp\",\
\"filePath\":\"\",\
\"ipaddr\":\"127.0.0.1\"}}\
",
"{\"genParams\":{\
\"debugInfo\":\"1\",\
\"audioName\":\"aac1\",\
\"videoName\":\"H264\",\
\"fec\":\"0\",\
\"nack\":\"0\",\
\"agc\":\"0\",\
\"denoise\":\"0\"\
},\
\"codecParams\":{\
\"out_flag\":\"8\",\
\"in_data_format\":\"0\",\
\"width\":\"640\",\
\"height\":\"480\",\
\"width1\":\"320\",\
\"height1\":\"240\",\
\"width2\":\"160\",\
\"height2\":\"120\",\
\"gop_size\":\"50\",\
\"mtu_size\":\"1400\",\
\"frame_rate\":\"25\",\
\"min_bits_rate\":\"256000\",\
\"max_bits_rate\":\"1024000\",\
\"out_data_format\":\"0\",\
\"video_port\":\"19000\",\
\"video_bits_rate\":\"512000\",\
\"video_bits_rate1\":\"128000\",\
\"video_bits_rate2\":\"64000\",\
\"video_codec_id\":\"28\",\
\"frame_size\":\"2048\",\
\"in_channel_count\":\"2\",\
\"in_sample_rate\":\"48000\",\
\"in_sample_fmt\":\"8\",\
\"out_channel_count\":\"2\",\
\"out_sample_rate\":\"48000\",\
\"out_sample_fmt\":\"1\",\
\"audio_port\":\"18001\",\
\"in_filename\":\"\",\
\"out_filename\":\"live.flv\",\
\"sdp_filename\":\"video.sdp\",\
\"filePath\":\"\",\
\"ipaddr\":\"127.0.0.1\"}}\
",
"{\"genParams\":{\
\"debugInfo\":\"0\",\
\"audioName\":\"aac1\",\
\"videoName\":\"H264\",\
\"fec\":\"0\",\
\"nack\":\"0\",\
\"agc\":\"0\",\
\"denoise\":\"0\"\
},\
\"codecParams\":{\
\"out_flag\":\"0\",\
\"streamId\":\"2\",\
\"in_data_format\":\"0\",\
\"width\":\"640\",\
\"height\":\"480\",\
\"gop_size\":\"50\",\
\"mtu_size\":\"1400\",\
\"frame_rate\":\"25\",\
\"min_bits_rate\":\"128000\",\
\"max_bits_rate\":\"512000\",\
\"out_data_format\":\"0\",\
\"video_port\":\"19000\",\
\"video_bits_rate\":\"256000\",\
\"video_codec_id\":\"28\",\
\"frame_size\":\"2048\",\
\"in_channel_count\":\"2\",\
\"in_sample_rate\":\"48000\",\
\"in_sample_fmt\":\"8\",\
\"out_channel_count\":\"2\",\
\"out_sample_rate\":\"48000\",\
\"out_sample_fmt\":\"1\",\
\"audio_port\":\"19001\",\
\"out_filename\":\"\",\
\"out_streamname\":\"\",\
\"sdp_filename\":\"video.sdp\",\
\"filePath\":\"\",\
\"ipaddr\":\"127.0.0.1\"}}\
"
};

class TransStream
{
public:
	TransStream(int chanId);
	~TransStream();
	int WriteVideoStream(AVRawData* packet);
	int WriteAudioStream(AVRawData* packet);

	static bool WriteVideoStreamRun(void* object);
	bool WriteVideoStreamProcess();
	static bool WriteAudioStreamRun(void* object);
	bool WriteAudioStreamProcess();
	void Init(char *fileName);
	static void RawCallback(void *hnd, int type, AVRawData *data);

public:
	webrtc::CriticalSectionWrapper* _critsect[2];
	webrtc::EventWrapper *_event[2];
	rtc::scoped_ptr<webrtc::ThreadWrapper> _thread[2];
	void *_codecHandle[2];
	int _hndIdx[2];
	unsigned int _waitTime;
	bool _work_status;
	int _chanId;
	int64_t _streamId;
	int _width;
	int _height;
	char _inFile[MAX_PATH];
	pRawCallback _cb;
	std::list<AVRawData*> _audioRawDataList;
	std::list<AVRawData*> _videoRawDataList;
	//StreamInfo _streamInfo;
	char _videoOutBitsData[512000];//test
	char _audioOutBitsData[1024];//test
};
TransStream::TransStream(int chanId) :
_chanId(chanId),
_streamId(chanId),
_work_status(true),
_cb(NULL),
_width(0),
_height(0),
_waitTime(1000)
{
	_hndIdx[kVideoChannel] = _hndIdx[kAudioChannel] = -1;
	_codecHandle[kVideoChannel] = _codecHandle[kAudioChannel] = NULL;
	for (int i = 0; i < 2; i++)
	{
		_critsect[i] = webrtc::CriticalSectionWrapper::CreateCriticalSection();
		_event[i] = webrtc::EventWrapper::Create();
	}
}
TransStream::~TransStream()
{
	for (int i = 0; i < 2; i++)
	{
		if (_thread[i].get())
		{
			if (_thread[i]->Stop())
			{
				_thread[i].reset();
				_thread[i] = NULL;
			}
		}
		if (_event[i])
		{
			delete _event[i];
			_event[i] = NULL;
		}
		if (_critsect[i])
		{
			delete _critsect[i];
			_critsect[i] = NULL;
		}
	}
	//
	for (int i = 0; i < 2; i++)
	{
		if (_codecHandle[i])
		{
			int ret = FF_AVCODEC_DELETE((void *)&_hndIdx[i]);//someone
			_codecHandle[i] = NULL;
		}
	}
	for (std::list<AVRawData*>::iterator it = _audioRawDataList.begin(); it != _audioRawDataList.end(); ++it)
	{
		delete (*it)->data[0];
		delete *it;
	}
	_audioRawDataList.clear();
	//
	for (std::list<AVRawData*>::iterator it = _videoRawDataList.begin(); it != _videoRawDataList.end(); ++it)
	{
		delete (*it)->data[0];
		delete (*it)->data[1];
		delete (*it)->data[2];
		delete *it;
	}
	_videoRawDataList.clear();
}
void TransStream::RawCallback(void *hnd, int type, AVRawData *data)
{
	TransStream *stream = (TransStream *)hnd;
	AVRawData *newData = new AVRawData;
	int cmd = -1;
	if (type == kVideo)
	{
		cmd = kVideo;
		newData->width = data->width;
		newData->height = data->height;
		newData->time_stamp = data->time_stamp;
		newData->data[0] = new char[newData->width * newData->height];
		newData->data[1] = new char[(newData->width >> 1) * (newData->height >> 1)];
		newData->data[2] = new char[(newData->width >> 1) * (newData->height >> 1)];
		memcpy(newData->data[0], data->data[0], newData->width * newData->height);
		memcpy(newData->data[1], data->data[1], (newData->width >> 1) * (newData->height >> 1));
		memcpy(newData->data[2], data->data[2], (newData->width >> 1) * (newData->height >> 1));
		stream->_critsect[kVideoChannel]->Enter();
		stream->_videoRawDataList.push_back(newData);
		stream->_critsect[kVideoChannel]->Leave();
	}
	else if (type == kAudio)
	{ 
		cmd = kAudio;
		newData->time_stamp = data->time_stamp;
		newData->data_size = data->data_size;
		newData->data[0] = new char[newData->data_size];
		memcpy(newData->data[0], data->data[0], newData->data_size);
		stream->_critsect[kAudioChannel]->Enter();
		stream->_audioRawDataList.push_back(newData);
		stream->_critsect[kAudioChannel]->Leave();
	}
	
}
void TransStream::Init(char *fileName)
{
	strcpy(_inFile, fileName);
	//_interval[kVideoChannel] = 1000 / 25;
	//_interval[kAudioChannel] = (2048 * 1000) / 48000;
	//video
	_hndIdx[kVideoChannel] = FF_AVCODEC_CREATE(&_codecHandle[kVideoChannel]);
	FF_CODEC_SET_PARAMS(_codecHandle[kVideoChannel], cparams[0]);
	//char testBuf[256] = {};
	FF_CODEC_SET_OUT_STREAMNAME((void *)_codecHandle[kVideoChannel], _inFile, 0);
	FF_CODEC_SET_CHANID((void *)_codecHandle[kVideoChannel], _chanId);
	FF_CODEC_SET_STREAMID((void *)_codecHandle[kVideoChannel], _streamId);
	FF_CODEC_SET_WIDTH((void *)_codecHandle[kVideoChannel], _width, 0);
	FF_CODEC_SET_HEIGHT((void *)_codecHandle[kVideoChannel], _height, 0);
	int ret = FF_CODEC_INIT(_codecHandle[kVideoChannel], kVideoEncStream);
	
	//audio
	_hndIdx[kAudioChannel] = FF_AVCODEC_CREATE(&_codecHandle[kAudioChannel]);
	FF_CODEC_SET_PARAMS(_codecHandle[kAudioChannel], cparams[0]);
	FF_CODEC_SET_OUT_STREAMNAME((void *)_codecHandle[kAudioChannel], _inFile, 0);
	FF_CODEC_SET_CHANID((void *)_codecHandle[kAudioChannel], _chanId);
	FF_CODEC_SET_STREAMID((void *)_codecHandle[kAudioChannel], _streamId);
	ret = FF_CODEC_INIT(_codecHandle[kAudioChannel], kAudioEncStream);
	//
#if 0
	for (int i = 0; i < 2; i++)
	{
		
		_thread[i] = i == kVideoChannel ? webrtc::ThreadWrapper::CreateThread(WriteVideoStreamRun, this, "WriteVideoStreamRun") :
				webrtc::ThreadWrapper::CreateThread(WriteAudioStreamRun, this, "WriteAudioStreamRun");
		_thread[i]->SetPriority(webrtc::kNormalPriority);
		_thread[i]->Start();
	}
#endif
}
int TransStream::WriteVideoStream(AVRawData* packet)
{
	int ret = 0;
	if (_codecHandle[kVideoChannel])
	{
		int frameType = 0;
		//char * inBuf[3] = {};//?
		int len = 0;
		char *outBuf[3] = {};//?
		int oLen = 0;
		outBuf[0] = _videoOutBitsData;
		ret = FF_CODEC_CODEDED(_codecHandle[kVideoChannel], packet->data, len, outBuf, oLen, &frameType, kVideoEncStream);
	}
	return ret;
}
bool TransStream::WriteVideoStreamRun(void* object)
{
	return static_cast<TransStream*>(object)->WriteVideoStreamProcess();
}
bool TransStream::WriteVideoStreamProcess()
{
	int ret = 0;
	unsigned int waitTime = _waitTime;
	_event[kVideoChannel]->Wait(waitTime);
	_critsect[kVideoChannel]->Enter();
	if (_work_status == false)
	{
		_waitTime = 1000;
		_critsect[kVideoChannel]->Leave();
		return true;
	}
	__int64 time0 = webrtc::TickTime::MillisecondTimestamp();
	while (!_videoRawDataList.empty())
	{
		int pktSize = _videoRawDataList.size();
		AVRawData* packet = _videoRawDataList.front();
		_videoRawDataList.pop_front();
		_critsect[kVideoChannel]->Leave();
		//
		if (pktSize > 20)
		{
			//LogOut("PopProcess:  pktSize= %d \n", (void *)&pktSize);
		}
		int frameType = 0;
		//char * inBuf[3] = {};//?
		int len = 0;
		char *outBuf[3] = {};//?
		int oLen = 0;
		outBuf[0] = _videoOutBitsData;
		ret = FF_CODEC_CODEDED(_codecHandle[kVideoChannel], packet->data, len, outBuf, oLen, &frameType, kVideoEncStream);
		//
		delete packet->data[0];
		delete packet->data[1];
		delete packet->data[2];
		delete packet;
		packet = NULL;
		_critsect[kVideoChannel]->Enter();
	}

	_critsect[kVideoChannel]->Leave();

	return true;
}
int TransStream::WriteAudioStream(AVRawData* packet)
{
	int ret = 0;
	if (_codecHandle[kAudioChannel])
	{
		int frameType = 0;
		//char * inBuf[3] = {};//?
		int len = 8192;
		char *outBuf[3] = {};//?
		int oLen = 0;
		outBuf[0] = _audioOutBitsData;
		ret = FF_CODEC_CODEDED(_codecHandle[kAudioChannel], packet->data, len, outBuf, oLen, &frameType, kAudioEncStream);
	}
	return ret;
}
bool TransStream::WriteAudioStreamRun(void* object)
{
	return static_cast<TransStream*>(object)->WriteAudioStreamProcess();
}
bool TransStream::WriteAudioStreamProcess()
{
	int ret = 0;
	unsigned int waitTime = _waitTime;
	_event[kAudioChannel]->Wait(waitTime);
	_critsect[kAudioChannel]->Enter();
	if (_work_status == false)
	{
		_waitTime = 1000;
		_critsect[kAudioChannel]->Leave();
		return true;
	}
	__int64 time0 = webrtc::TickTime::MillisecondTimestamp();
	while (!_audioRawDataList.empty())
	{
		int pktSize = _audioRawDataList.size();
		AVRawData* packet = _audioRawDataList.front();
		_audioRawDataList.pop_front();
		_critsect[kAudioChannel]->Leave();
		//
		if (pktSize > 20)
		{
			//LogOut("PopProcess:  pktSize= %d \n", (void *)&pktSize);
		}
		int frameType = 0;
		//char * inBuf[3] = {};//?
		int len = 8192;
		char *outBuf[3] = {};//?
		int oLen = 0;
		outBuf[0] = _audioOutBitsData;
		ret = FF_CODEC_CODEDED(_codecHandle[kAudioChannel], packet->data, len, outBuf, oLen, &frameType, kAudioEncStream);
		//
		delete packet->data[0];
		//delete packet->data[1];
		//delete packet->data[2];
		delete packet;
		packet = NULL;
		_critsect[kAudioChannel]->Enter();
	}
	_critsect[kAudioChannel]->Leave();

	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------------------
class ReadStream
{
public:
	ReadStream(int chanId);
	~ReadStream();
	static bool ReadStreamRun(void* object);
	bool ReadStreamProcess();
	void Init(char *fileName);
	void RegisterCallback(void *hnd, void *cb);
public:
	/*webrtc::CriticalSectionWrapper* _critsect;
	webrtc::EventWrapper *_event;*/
	rtc::scoped_ptr<webrtc::ThreadWrapper> _thread;
	//
	webrtc::CriticalSectionWrapper* _critsect[3];
	webrtc::EventWrapper *_event[3];
	//
	void *_codecHandle;
	int _hndIdx;
	unsigned int _waitTime;
	int _chanId;
	int64_t _streamId;
	int _width;
	int _height;
	int _frameNum[2];
	char _inFile[MAX_PATH];
	pRawCallback _cb;
	void *_obj;
	std::list<AVRawData*> _audioRawDataList;
	std::list<AVRawData*> _videoRawDataList;
	//StreamInfo _streamInfo;
	char *_buf;

};
typedef std::map<int, ReadStream *> ReadStreamMap;
typedef std::map<int, StreamInfo *> StreamInfoMap;

//static void RawCallback(void *hnd, int type, AVRawData *data)
//{
//	ReadStream *stream = (ReadStream *)hnd;
//}
ReadStream::ReadStream(int chanId) :
_chanId(chanId),
_streamId(chanId),
_hndIdx(-1),
_codecHandle(NULL),
_cb(NULL),
_obj(NULL),
_buf(NULL),
_width(0),
_height(0),
_waitTime(1000)
{
	_frameNum[kAudioChannel] = _frameNum[kVideoChannel] = 0;
	for (int i = 0; i < 3; i++)
	{
		_critsect[i] = webrtc::CriticalSectionWrapper::CreateCriticalSection();
		_event[i] = webrtc::EventWrapper::Create();
	}
	_thread = webrtc::ThreadWrapper::CreateThread(ReadStreamRun, this, "ReadStreamRun");
}
ReadStream::~ReadStream()
{
	if (_thread.get())
	{
		if (_thread->Stop())
		{
			_thread.reset();
			_thread = NULL;
		}
	}
	for (int i = 0; i < 3; i++)
	{
		if (_event[i])
		{
			delete _event[i];
			_event[i] = NULL;
		}
		if (_critsect[i])
		{
			delete _critsect[i];
			_critsect[i] = NULL;
		}
	}
	//
	if (_codecHandle)
	{
		int ret = FF_AVCODEC_DELETE((void *)&_hndIdx);//someone
		_codecHandle = NULL;
	}
	for (std::list<AVRawData*>::iterator it = _audioRawDataList.begin(); it != _audioRawDataList.end(); ++it)
	{
		delete (*it)->data[0];
		delete *it;
	}
	_audioRawDataList.clear();
	//
	for (std::list<AVRawData*>::iterator it = _videoRawDataList.begin(); it != _videoRawDataList.end(); ++it)
	{
		delete (*it)->data[0];
		delete (*it)->data[1];
		delete (*it)->data[2];
		delete *it;
	}
	_videoRawDataList.clear();
	//
	if (_buf)
	{
		delete _buf;
		_buf = NULL;
	}
}
void ReadStream::Init(char *fileName)
{
	strcpy(_inFile, fileName);
	_hndIdx = FF_AVCODEC_CREATE(&_codecHandle);
	FF_CODEC_SET_FILENAME(_codecHandle, fileName, 0);
	int ret = FF_CODEC_INIT(_codecHandle, kAVDecStream);
	FF_CODEC_SET_CHANID((void *)_codecHandle, _chanId);
	FF_CODEC_SET_STREAMID((void *)_codecHandle, _streamId);
	//
	int width = 1920;
	int height = 1080;
	int size = (width * height * 3) >> 1;
	_buf = new char[size];
	//
	_thread->SetPriority(webrtc::kNormalPriority);
	_thread->Start();
}
void ReadStream::RegisterCallback(void *hnd, void *cb)
{
	_cb = (pRawCallback)cb;
	_obj = hnd;
}
bool ReadStream::ReadStreamRun(void* object)
{
	return static_cast<ReadStream*>(object)->ReadStreamProcess();
}
bool ReadStream::ReadStreamProcess()
{
	int ret = -1;
	unsigned int waitTime = _waitTime;
	_event[2]->Wait(waitTime);
	_critsect[2]->Enter();
	__int64 time0 = webrtc::TickTime::MillisecondTimestamp();
	int isKeyFrame = 0;
	int len = 0;
	int64_t timeStamp = 0;
	ret = FF_READ_FRAM2(_codecHandle, (unsigned char *)_buf, &len, &isKeyFrame, &_width, &_height, &timeStamp);
	AVRawData *newData = new AVRawData;
	int type = -1;
	
	if (ret == kVideo  && len > 0)
	{
		type = kVideo;
		newData->width = _width;
		newData->height = _height;
		newData->time_stamp = timeStamp;//?
		newData->data[0] = new char [newData->width * newData->height];
		newData->data[1] = new char [(newData->width >> 1) * (newData->height >> 1)];
		newData->data[2] = new char [(newData->width >> 1) * (newData->height >> 1)];
		memcpy(newData->data[0], _buf, _width * _height);
		memcpy(newData->data[1], &_buf[_width * _height], (_width >> 1) * (_height >> 1));
		memcpy(newData->data[2], &_buf[_width * _height + (_width >> 1) * (_height >> 1)], (_width >> 1) * (_height >> 1));
		_waitTime = kWaitTime1;
		_frameNum[kVideoChannel]++;
	}
	else if (ret == kAudio && len > 0)
	{
		type = kAudio;
		newData->time_stamp = timeStamp;
		newData->data_size = len;
		newData->data[0] = new char[newData->data_size];
		memcpy(newData->data[0], _buf, newData->data_size);
		_waitTime = kWaitTime1;
		_frameNum[kAudioChannel]++;
	}
	if (ret >= 0)
	{
		_waitTime = kWaitTime1;
	}
	
	_critsect[2]->Leave();
	//
	if (ret == kVideo  && len > 0)
	{
		type = kVideo;
		_critsect[kVideoChannel]->Enter();
		_videoRawDataList.push_back(newData);
		if (_cb && _obj)
		{
			_cb(_obj, type, newData);
		}
		_critsect[kVideoChannel]->Leave();
	}
	else if (ret == kAudio && len > 0)
	{
		type = kAudio;
		_critsect[kAudioChannel]->Enter();
		_audioRawDataList.push_back(newData);
		if (_cb && _obj)
		{
			_cb(_obj, type, newData);
		}
		_critsect[kAudioChannel]->Leave();
	}

	return true;
}
//-------------------------------------------------------------------------------------------------------------
class MCUStream
{
public:
	MCUStream(int chanId);
	~MCUStream();
	static bool VirtualCaptureRun(void* object);
	bool VirtualCaptureProcess();
	static bool VirtualRecordRun(void* object);
	bool VirtualRecordProcess();

	void Init(char *fileName);
	void RegisterCallback(void *hnd, void *cb);
	void InsetStream(ReadStream *stream, StreamInfo *info);
	void SetBaseChanId(int chanId) { _baseChanId = chanId; }
	int64_t CheckAudioData();
	int64_t CheckVideoData();
	void ExchangeChannel(int chanId0, int chanId1);
public:
	webrtc::CriticalSectionWrapper* _critsect[2];
	webrtc::EventWrapper *_event[2];
	rtc::scoped_ptr<webrtc::ThreadWrapper> _thread[2];
	ReadStreamMap _ReadStreamMap;
	StreamInfoMap _StreamInfoMap;
	TransStream *_transStream;
	//
	void *_scalHandle;
	void *_resampleHandle;
	void *_mixHandle;
	//unsigned int _waitTime;
	int _waitTime0;
	int _waitTime1;
	__int64 _startTime[2];
	__int64 _frameNum[2];
	int _width;
	int _height;
	int _chanId;
	int _baseChanId;
	int64_t _streamId;
	char _inFile[MAX_PATH];
	pRawCallback _cb;
	void *_obj;
	std::list<AVRawData*> _audioRawDataList;
	std::list<AVRawData*> _videoRawDataList;
	char _videoOutBitsData[512000];//test
	char _audioOutBitsData[1024];//test
	bool _work_status;
	//int _interval[2];
	StreamInfo _streamInfo;
	//char *_videoLastImg[3];
	AVRawData *_lastImg;
	AVRawData *_lastPCM;
	//char *_mixData[kMaxTrannel];// [8192];
	short _mixData[kMaxTrannel][4096];
	int _delayTime;
	bool _startRecord;
	int64_t _videoLastTimeStamp;
	int64_t _audioLastTimeStamp;
	TimeStampInfo _timeStampInfo[2];
};
MCUStream::MCUStream(int chanId) :
_chanId(chanId),
_streamId(chanId),
_baseChanId(0),
_scalHandle(NULL),
_resampleHandle(NULL),
_mixHandle(NULL),
_lastImg(NULL),
_lastPCM(NULL),
_cb(NULL),
_obj(NULL),
_width(0),
_height(0),
_transStream(NULL),
_delayTime(500),
_work_status(true),
_startRecord(false),
_videoLastTimeStamp(-1),
_audioLastTimeStamp(-1),
//_waitTime(1000),
_waitTime0(200),
_waitTime1(200)
{
	_timeStampInfo[kVideoChannel] = { -1, -1 };
	_timeStampInfo[kAudioChannel] = { -1, -1 };
	_streamInfo.enable = false;
	_streamInfo.audio_enable = false;
	_streamInfo.video_enable = false;
	_startTime[kVideoChannel] = _startTime[kAudioChannel] = 0;
	_frameNum[kVideoChannel] = _frameNum[kAudioChannel] = 0;
	
	for (int i = 0; i < 2; i++)
	{
		_critsect[i] = webrtc::CriticalSectionWrapper::CreateCriticalSection();
		_event[i] = webrtc::EventWrapper::Create();
		_thread[i] = i == kVideoChannel ? webrtc::ThreadWrapper::CreateThread(VirtualCaptureRun, this, "VirtualCaptureRun") :
						  webrtc::ThreadWrapper::CreateThread(VirtualRecordRun, this, "VirtualRecordRun");
		
	
	}
}
MCUStream::~MCUStream()
{
	for (int i = 0; i < 2; i++)
	{
		if (_thread[i].get())
		{
			if (_thread[i]->Stop())
			{
				_thread[i].reset();
				_thread[i] = NULL;
			}
		}
		if (_event[i])
		{
			delete _event[i];
			_event[i] = NULL;
		}
		if (_critsect[i])
		{
			delete _critsect[i];
			_critsect[i] = NULL;
		}
	}
	//
	if (_transStream)
	{
		delete _transStream;
		_transStream = NULL;
	}
	if (_scalHandle)
	{
	}
	if (_resampleHandle)
	{
	}
	if (_mixHandle)
	{
	}
	for (std::list<AVRawData*>::iterator it = _audioRawDataList.begin(); it != _audioRawDataList.end(); ++it)
	{
		delete (*it)->data[0];
		delete *it;
	}
	_audioRawDataList.clear();
	//
	for (std::list<AVRawData*>::iterator it = _videoRawDataList.begin(); it != _videoRawDataList.end(); ++it)
	{
		delete (*it)->data[0];
		delete (*it)->data[1];
		delete (*it)->data[2];
		delete *it;
	}
	_videoRawDataList.clear();
	//
	ReadStreamMap::iterator it0 = _ReadStreamMap.begin();//.find(roomId);
	while (it0 != _ReadStreamMap.end())
	{
		//delete *it;
		ReadStream *ints = it0->second;
		delete ints;

		_ReadStreamMap.erase(it0++);
		//it++;
	}
	//
	StreamInfoMap::iterator it = _StreamInfoMap.begin();//.find(roomId);
	while (it != _StreamInfoMap.end())
	{
		//delete *it;
		StreamInfo *ints = it->second;
		delete ints;

		_StreamInfoMap.erase(it++);
		//it++;
	}
	//
	if (_lastImg)
	{
		for (int i = 0; i < 3; i++)
		{
			if (_lastImg->data[i])
			{
				delete _lastImg->data[i];
				_lastImg->data[i] = NULL;
			}
		}
		delete _lastImg;
		_lastImg = NULL;
	}
	//
	if (_lastPCM)
	{
		
		if (_lastPCM->data[0])
		{
			delete _lastPCM->data[0];
			_lastPCM->data[0] = NULL;
		}
		
		delete _lastPCM;
		_lastPCM = NULL;
	}
	
}
void MCUStream::Init(char *fileName)
{
	strcpy(_inFile, fileName);
	//_interval[kVideoChannel] = 1000 / 25;
	//_interval[kAudioChannel] = (2048 * 1000) / 48000;
	//video
	char *InParams[2] = {};//?
	//
	_width = 1280;//test
	_height = 720;//test
	//
	_lastImg = new AVRawData;
#if 1
	_lastImg->width = _width;
	_lastImg->height = _height;
	_lastImg->data[0] = new char[_width * _height];
	_lastImg->data[1] = new char[(_width >> 1) * (_height >> 1)];
	_lastImg->data[2] = new char[(_width >> 1) * (_height >> 1)];
	memset(_lastImg->data[0], 128, _width * _height);
	memset(_lastImg->data[1], 128, (_width >> 1) * (_height >> 1));
	memset(_lastImg->data[2], 128, (_width >> 1) * (_height >> 1));
	//
	_lastPCM = new AVRawData;
	_lastPCM->data_size = 8192;
	_lastPCM->data[0] = new char[_lastPCM->data_size];
#else
	_lastImg->width = 0;
	_lastImg->height = 0;
	_lastImg->data[0] = NULL;
	_lastImg->data[1] = NULL;
	_lastImg->data[2] = NULL;
#endif
	//
	_transStream = new TransStream(_chanId);
	RegisterCallback((void *)_transStream, (void *)&_transStream->RawCallback);
	_transStream->_width = _width;
	_transStream->_height = _height;
	//
	for (int i = 0; i < 2; i++)
	{
		_thread[i]->SetPriority(webrtc::kNormalPriority);
		_thread[i]->Start();
	}
}
void MCUStream::RegisterCallback(void *hnd, void *cb)
{
	_cb = (pRawCallback)cb;
	_obj = hnd;
}
void MCUStream::InsetStream(ReadStream *stream, StreamInfo *info)
{
	_ReadStreamMap.insert(std::pair<int, ReadStream *>(stream->_chanId, stream));
	StreamInfo *newStreamInfo = new StreamInfo;
	*newStreamInfo = *info;
	/*newStreamInfo->enable = info->enable;
	newStreamInfo->video_enable = info->video_enable;
	newStreamInfo->audio_enable = info->audio_enable;*/
	_StreamInfoMap.insert(std::pair<int, StreamInfo *>(stream->_chanId, newStreamInfo));
}
#if 1
static void audio_mix4(short sourseFile[kMaxTrannel][4096], short *dst, int framesize, int number)
{
	//归一化混音
	int const MAX = 32767;
	int const MIN = -32768;

	double f = 1;
	int output;
	int i = 0, j = 0;
	for (i = 0; i < (framesize << 1); i++)
	{
		int temp = 0;
		for (j = 0; j<number; j++)
		{
			temp += sourseFile[j][i];
		}
		output = (int)(temp*f);
		if (output>MAX)
		{
			f = (double)MAX / (double)(output);
			output = MAX;
		}
		if (output<MIN)
		{
			f = (double)MIN / (double)(output);
			output = MIN;
		}
		if (f<1)
		{
			f += ((double)1 - f) / (double)32;
		}
		dst[i] = (short)output;
	}
}
#endif
int64_t MCUStream::CheckAudioData()
{
	int64_t ret = -1;
	//_startRecord = true;//test
	if (_startRecord == false)
	{
		ReadStreamMap::iterator it = _ReadStreamMap.find(_baseChanId);
		if (it != _ReadStreamMap.end())
		{
			ReadStream *stream = it->second;
			int flag = 0;
			stream->_critsect[kAudioChannel]->Enter();
			int size = stream->_audioRawDataList.size();
			
			if (size > 1)
			{
				//std::list<AVRawData*>::iterator it0 = _audioRawDataList.begin();
				//std::list<AVRawData*>::iterator it1 = _audioRawDataList.end();
				AVRawData* packet0 = stream->_audioRawDataList.front();
				AVRawData* packet1 = stream->_audioRawDataList.back();

				int diffTime = (packet1->time_stamp - packet0->time_stamp);
				flag = diffTime > _delayTime;
				flag |= ((size * 128) / 3) > _delayTime;
				if (flag)
				{
					_startRecord = true;
					ret = 1;
				}
			}
			stream->_critsect[kAudioChannel]->Leave();
		}
	}
	else
	{
		
	}
	if (_startRecord == true)
	{
		int i = 0;
		/*ReadStreamMap::iterator it0 = _ReadStreamMap.find(_baseChanId);
		if (it0 != _ReadStreamMap.end())
		{
			ReadStream *stream = it0->second;
			stream->_critsect[kAudioChannel]->Enter();
			AVRawData *newData = stream->_audioRawDataList.front();
			ret = newData->time_stamp;
			stream->_critsect[kAudioChannel]->Leave();
		}
		if (ret < 0)
		{
			return ret;
		}*/
		bool hasBeenBaseChanId = false;
		ReadStreamMap::iterator 
		it0 = _ReadStreamMap.begin();
		while (it0 != _ReadStreamMap.end())
		{
			ReadStream *stream = it0->second;
			AVRawData *newData = NULL;
			stream->_critsect[kAudioChannel]->Enter();
			if (!stream->_audioRawDataList.empty())
			{
				newData = stream->_audioRawDataList.front();
				stream->_audioRawDataList.pop_front();
				stream->_critsect[kAudioChannel]->Leave();
				//add process
				int chanId = stream->_chanId;
				if (hasBeenBaseChanId == false)
				{
					ret = newData->time_stamp;
				}
				if (chanId == _baseChanId)
				{
					ret = newData->time_stamp;
					hasBeenBaseChanId = true;
				}
				StreamInfo *info = NULL;
				StreamInfoMap::iterator it = _StreamInfoMap.find(chanId);
				if (it != _StreamInfoMap.end())
				{
					info = it->second;
					if (info->enable == true && info->audio_enable == true)
					{
						memcpy(_mixData[i], newData->data[0], newData->data_size);//8192
						i++;
					}
				}
				//
				delete newData->data[0];
				delete newData;
			}
			else
			{
				stream->_critsect[kAudioChannel]->Leave();
			}
			//	
			it0++; 
		}
		if (_cb && _obj && i)
		{
#if 0
			AVRawData *newData = new AVRawData;
			newData->data_size = 8192;
			newData->data[0] = new char[newData->data_size];
			audio_mix4(_mixData, (short *)newData->data[0], 2048, i);
			_cb(_obj, kAudio, newData);
#else
			if (_transStream)
			{
				audio_mix4(_mixData, (short *)_lastPCM->data[0], 2048, i);
				_transStream->WriteAudioStream(_lastPCM);
			}
#endif
		}
	}
	return ret;
}
void MCUStream::ExchangeChannel(int chanId0, int chanId1)
{
	_critsect[kVideoChannel]->Enter();
	_critsect[kAudioChannel]->Enter();
	StreamInfo *info[2] = {};
	StreamInfoMap::iterator it0 = _StreamInfoMap.find(chanId0);
	info[0] = it0->second;
	StreamInfoMap::iterator it1 = _StreamInfoMap.find(chanId1);
	info[1] = it1->second;
	StreamInfo infoTmp = *info[0];
	*info[0] = *info[1];
	*info[1] = infoTmp;
	_critsect[kAudioChannel]->Leave();
	_critsect[kVideoChannel]->Leave();
}
int64_t MCUStream::CheckVideoData()
{
	int64_t ret = 0;
	//_startRecord = true;//test
	if (_startRecord == false)
	{
		ReadStreamMap::iterator it = _ReadStreamMap.find(_baseChanId);
		if (it != _ReadStreamMap.end())
		{
			ReadStream *stream = it->second;
			int flag = 0;
			stream->_critsect[kVideoChannel]->Enter();
			int size = stream->_videoRawDataList.size();

			if (size > 1)
			{
				AVRawData* packet0 = stream->_videoRawDataList.front();
				AVRawData* packet1 = stream->_videoRawDataList.back();

				int diffTime = (packet1->time_stamp - packet0->time_stamp);
				flag = diffTime > _delayTime;
				flag |= (size * 40) > _delayTime;
				if (flag)
				{
					_startRecord = true;
					ret = 1;
				}
			}
			stream->_critsect[kVideoChannel]->Leave();
		}
	}
	else
	{

	}
	if (_startRecord == true && _transStream)
	{
		int i = 0;
		bool hasBeenBaseChanId = false;
		ReadStreamMap::iterator it0 = _ReadStreamMap.begin();
		while (it0 != _ReadStreamMap.end())
		{
			ReadStream *stream = it0->second;
			stream->_critsect[kVideoChannel]->Enter();
			if (!stream->_videoRawDataList.empty())
			{
				AVRawData *newData = stream->_videoRawDataList.front();
				stream->_videoRawDataList.pop_front();
				stream->_critsect[kVideoChannel]->Leave();
				//add process
				int chanId = stream->_chanId;
				if (hasBeenBaseChanId == false)
				{
					ret = newData->time_stamp;
				}
				if (chanId == _baseChanId)
				{
					ret = newData->time_stamp;
					hasBeenBaseChanId = true;
				}
				StreamInfo *info = NULL;
				StreamInfoMap::iterator it = _StreamInfoMap.find(chanId);
				if (it != _StreamInfoMap.end())
				{
					info = it->second;
					if (info->enable == true && info->video_enable == true)
					{
						short rect[4][2] = {};
						for (int i = 0; i < 4; i++)
						{
							for (int j = 0; j < 2; j++)
							{
								rect[i][j] = info->rect[i][j];
							}
						}
						int srcSize[3] = {};
						int dstSize[3] = {};
						char *outBuf[3] = {};
						srcSize[0] = newData->width;
						srcSize[1] = newData->height;
						srcSize[2] = newData->width;
						dstSize[0] = rect[1][0] - rect[0][0];//x
						dstSize[1] = rect[2][1] - rect[0][1];//y
						dstSize[2] = _width;
						outBuf[0] = &_lastImg->data[0][rect[0][1] * _width + rect[0][0]];
						outBuf[1] = &_lastImg->data[1][(rect[0][1] >> 1) * (_width >> 1) + (rect[0][0] >> 1)];
						outBuf[2] = &_lastImg->data[2][(rect[0][1] >> 1) * (_width >> 1) + (rect[0][0] >> 1)];
						_scalHandle = FF_SCALE(_scalHandle, newData->data, srcSize, outBuf, dstSize);
						FF_SCALE_DELETE(_scalHandle);	_scalHandle = NULL;
						i++;
					}
				}
				//
				delete newData->data[0];
				delete newData->data[1];
				delete newData->data[2];
				delete newData;
			}
			else
			{
				stream->_critsect[kVideoChannel]->Leave();
			}
			//
			it0++;
		}
		if (_cb && _obj && i)
		{
#if 0
			AVRawData *newData = new AVRawData;
			newData->width = _width;
			newData->height = _height;
			newData->time_stamp = 0;
			newData->data[0] = new char[_width * _height];
			newData->data[1] = new char[(_width >> 1) * (_height >> 1)];
			newData->data[2] = new char[(_width >> 1) * (_height >> 1)];
			memcpy(newData->data[0], _lastImg->data[0], _width * _height);
			memcpy(newData->data[1], _lastImg->data[1], (_width >> 1) * (_height >> 1));
			memcpy(newData->data[2], _lastImg->data[2], (_width >> 1) * (_height >> 1));
			_cb(_obj, kVideo, newData);
#else
			
			if (_transStream)
			{
				_lastImg->width = 
				_transStream->WriteVideoStream(_lastImg);
			}
#endif
			
		}
	}
	return ret;
}
bool MCUStream::VirtualCaptureRun(void* object)
{
	return static_cast<MCUStream*>(object)->VirtualCaptureProcess();
}
bool MCUStream::VirtualCaptureProcess()
{
	int64_t ret = 0;
	unsigned int waitTime = _waitTime0;
	_event[kVideoChannel]->Wait(waitTime);
	_critsect[kVideoChannel]->Enter();
	if (_work_status == false)
	{
		_waitTime0 = 1000;
		_critsect[kVideoChannel]->Leave();
		return true;
	}
	__int64 now = webrtc::TickTime::MillisecondTimestamp();
	int videoTime = 66;//1000/15
	//int videoTime = 40;//1000/25
	//
	if (_startTime[kVideoChannel] == 0)
	{
		_startTime[kVideoChannel] = now;
		_waitTime0 = videoTime;// 40;//1000 / 25
	}
	else
	{
		//_interval[kVideoChannel] = 1000 / 25;
		//wait_time = (start_time + (frameNum + 1) * interval) - now_time;
		_waitTime0 = (_startTime[kVideoChannel] + ((_frameNum[kVideoChannel] + 1) * videoTime)) - now;
		_waitTime0 = _waitTime0 < 0 ? kWaitTime1 : _waitTime0;
	}
	//check video base on audio
	_videoLastTimeStamp;
	ret = CheckVideoData();
	if (ret > 0)
	{
		//time_stamp_next - time_stamp_start = pkt_time_stamp_next - pkt_time_stamp_start
		//time_stamp_next = time_stamp_start + (pkt_time_stamp_next - pkt_time_stamp_start)
		//wait_time = time_stamp_next - time_stamp_now
		//wait_time = (time_stamp_start + (pkt_time_stamp_next - pkt_time_stamp_start)) - time_stamp_now
		if (_timeStampInfo[kVideoChannel].time_stamp_start < 0)
		{
			_timeStampInfo[kVideoChannel].time_stamp_start = now;
			_timeStampInfo[kVideoChannel].pkt_time_stamp_start = ret;
		}
		else
		{
			_waitTime0 = (_timeStampInfo[kVideoChannel].time_stamp_start + (ret - _timeStampInfo[kVideoChannel].pkt_time_stamp_start)) - now;
			_waitTime0 = _waitTime0 < 0 ? kWaitTime1 : _waitTime0;
		}
		if (_videoLastTimeStamp >= 0)
		{
			int diffTime = (int)(ret - _videoLastTimeStamp);
		}
		_videoLastTimeStamp = ret;
	}

	_frameNum[kVideoChannel]++;
	//printf("video cap _waitTime0 = %d \n", _waitTime0);
	_critsect[kVideoChannel]->Leave();

	return true;
}
bool MCUStream::VirtualRecordRun(void* object)
{
	return static_cast<MCUStream*>(object)->VirtualRecordProcess();
}
bool MCUStream::VirtualRecordProcess()
{
	int64_t ret = 0;
	unsigned int waitTime = _waitTime1;
	_event[kAudioChannel]->Wait(waitTime);
	_critsect[kAudioChannel]->Enter();
	if (_work_status == false)
	{
		_waitTime1 = (2048 * 1000) / 48000;
		_critsect[kAudioChannel]->Leave();
		return true;
	}
	__int64 now = webrtc::TickTime::MillisecondTimestamp();
	//
	if (_startTime[kAudioChannel] == 0)
	{
		_startTime[kAudioChannel] = now;
		_waitTime1 = (2048 * 1000) / 48000;//2048 / 48 = 128 / 3
	}
	else
	{
		//_interval[kAudioChannel] = (2048 * 1000) / 48000;
		//(frameNum + 1) * interval = next_time - start_time;
		//next_time = (frameNum + 1) * interval + start_time;
		//wait_time = next_time - now_time
		//wait_time = (start_time + (frameNum + 1) * interval) - now_time;
		_waitTime1 = (_startTime[kAudioChannel] + ((_frameNum[kAudioChannel] + 1) * 128) / 3) - now;
		_waitTime1 = _waitTime1 < 0 ? kWaitTime1 : _waitTime1;
	}
	//check audio
	ret = CheckAudioData();
	if (ret > 0)
	{
		//time_stamp_next - time_stamp_start = pkt_time_stamp_next - pkt_time_stamp_start
		//time_stamp_next = time_stamp_start + (pkt_time_stamp_next - pkt_time_stamp_start)
		//wait_time = time_stamp_next - time_stamp_now
		//wait_time = (time_stamp_start + (pkt_time_stamp_next - pkt_time_stamp_start)) - time_stamp_now
		if (_timeStampInfo[kAudioChannel].time_stamp_start < 0)
		{
			_timeStampInfo[kAudioChannel].time_stamp_start = now;
			_timeStampInfo[kAudioChannel].pkt_time_stamp_start = ret;
		}
		else
		{
			_waitTime1 = (_timeStampInfo[kAudioChannel].time_stamp_start + (ret - _timeStampInfo[kAudioChannel].pkt_time_stamp_start)) - now;
			_waitTime1 = _waitTime1 < 0 ? kWaitTime1 : _waitTime1;
		}
		if (_audioLastTimeStamp >= 0)
		{
			int diffTime = (int)(ret - _audioLastTimeStamp);
		}
		_audioLastTimeStamp = ret;
	}

	_frameNum[kAudioChannel]++;
	//printf("audio record _waitTime1 = %d \n", _waitTime1);
	_critsect[kAudioChannel]->Leave();

	return true;
}

//--------------------------------------------------------------------------------------------------------------------------
class MCUWorks
{
public:
	MCUWorks(int chanId);
	~MCUWorks();
	//static bool ReadStreamRun(void* object);
	//bool ReadStreamProcess();
	void Init();
	void RenewMode(char *jsonModeInfo);
	void RenewRecordStatus(int status, int flag);
	void RenewJpeg(char *filename, int type);
	void RenewWaterMark(int left, int right, int top, int bottom, float alpha, int fit);
	void ExchangeChannel(int chanId0, int chanId1);
public:
	webrtc::CriticalSectionWrapper* _critsect;
	webrtc::EventWrapper *_event;
	rtc::scoped_ptr<webrtc::ThreadWrapper> _thread;
	//std::list<ReadStream*> _ReadStreamList;
	//std::list<MCUStream*> _MCUStreamList;
	MCUStream *_monitorMCU;
	MCUStream *_liveMCU;
	//
	void *_factory;
	void *_codecHandle;
	int _hndIdx;
	unsigned int _waitTime;
	int _chanId;
	int64_t _streamId;
	char _inFile[MAX_PATH];
	//pRawCallback _cb;
};
MCUWorks::MCUWorks(int chanId) :
_chanId(chanId),
_streamId(chanId),
_hndIdx(-1),
_codecHandle(NULL),
//_cb(NULL),
_waitTime(1000)
{
	_monitorMCU = NULL;
	_liveMCU = NULL;
	void *_factory = NULL;
	FF_FACTORY_CREATE(&_factory, 1);
	_critsect = webrtc::CriticalSectionWrapper::CreateCriticalSection();
	_event = webrtc::EventWrapper::Create();
	//_thread = webrtc::ThreadWrapper::CreateThread(ReadStreamRun, this, "ReadStreamRun");
}
MCUWorks::~MCUWorks()
{
	if (_thread.get())
	{
		if (_thread->Stop())
		{
			_thread.reset();
			_thread = NULL;
		}
	}
	if (_event)
	{
		delete _event;
		_event = NULL;
	}
	if (_critsect)
	{
		delete _critsect;
		_critsect = NULL;
	}
	//
	if (_codecHandle)
	{
		int ret = FF_AVCODEC_DELETE((void *)&_hndIdx);//someone
		_codecHandle = NULL;
	}
#if 0
	for (std::list<ReadStream*>::iterator it = _ReadStreamList.begin(); it != _ReadStreamList.end(); ++it)
	{
		//delete (*it)->data[0];
		delete *it;
	}
	_ReadStreamList.clear();
#else
	
#endif
	//
	//for (std::list<MCUStream*>::iterator it = _MCUStreamList.begin(); it != _MCUStreamList.end(); ++it)
	//{
	//	//delete (*it)->data[0];
	//	//delete (*it)->data[1];
	//	//delete (*it)->data[2];
	//	delete *it;
	//}
	//_MCUStreamList.clear();
	if (_monitorMCU)
	{
		delete _monitorMCU;
		_monitorMCU = NULL;
	}
	if (_liveMCU)
	{
		delete _liveMCU;
		_liveMCU = NULL;
	}
	//
	if (_factory)
	{
		FF_FACTORY_DELETE(_factory);//all
		_factory = NULL;
	}
}
void MCUWorks::Init()
{
	int chanId = kMaxTrannel;//test
	_monitorMCU = new MCUStream(chanId);
	chanId = kMaxTrannel + 1;
	_liveMCU = new MCUStream(chanId);
}
void MCUWorks::ExchangeChannel(int chanId0, int chanId1)
{
	if (_liveMCU)
	{
		_liveMCU->ExchangeChannel(chanId0, chanId1);
	}
}
void MCUWorks::RenewMode(char *jsonModeInfo)
{
	if (_liveMCU && _liveMCU->_transStream)
	{

	}
}
void MCUWorks::RenewRecordStatus(int status, int flag)
{
	if (_liveMCU && _liveMCU->_transStream)
	{
		FF_CODEC_SET_RECORDSTATUS((void *)_liveMCU->_transStream->_codecHandle[kVideoChannel], status, flag);
		FF_CODEC_SET_RECORDSTATUS((void *)_liveMCU->_transStream->_codecHandle[kAudioChannel], status, flag);
	}
}
void MCUWorks::RenewJpeg(char *filename, int type)
{
	if (_liveMCU && _liveMCU->_transStream)
	{
		if (type == kWavFile)
		{
			FF_CODEC_SET_FILEPATH((void *)_liveMCU->_transStream->_codecHandle[kAudioChannel], filename, type);
		}
		else
		{
			FF_CODEC_SET_FILEPATH((void *)_liveMCU->_transStream->_codecHandle[kVideoChannel], filename, type);
		}
	}
}
void MCUWorks::RenewWaterMark(int left, int right, int top, int bottom, float alpha, int fit)
{
	if (_liveMCU && _liveMCU->_transStream)
	{
		FF_CODEC_SET_WATERMARK((void *)_liveMCU->_transStream->_codecHandle[kVideoChannel], left, right, top, bottom, alpha, fit);
	}
}
//------------------------------------------------------------------------------------------------------------------------------------------
#if 1
void FF_MCUTest(int chanId)
{
	MCUWorks *works = new MCUWorks(chanId);
	//
	works->_liveMCU = new MCUStream(kMaxTrannel);

	works->_liveMCU->Init("http://192.168.120.95:8090/feed3.ffm");// ("http://172.16.173.188:8090/stream3-0.flv");
	//
	char streamChaniId = 0;
	works->_liveMCU->_baseChanId = streamChaniId;
	ReadStream *stream = new ReadStream(streamChaniId);
	stream->Init("http://192.168.120.95:8090/stream0-0.flv");
	StreamInfo Info = { true, true, true, { { 0, 0 }, { 320, 0 }, { 0, 240 }, { 320, 240 } }, 0 };
	works->_liveMCU->InsetStream(stream, &Info);//bool enable, bool video_enable, bool audio_enable
#if 1
	streamChaniId = 1;
	stream = new ReadStream(streamChaniId);
	stream->Init("http://192.168.120.95:8090/stream1-0.flv");
	StreamInfo Info1 = { true, true, true, { { 0, 240 }, { 320, 240 }, { 0, 240 + 240 }, { 320, 240 + 240 } }, 1 };
	works->_liveMCU->InsetStream(stream, &Info1);
	//

	/*
	streamChaniId = 2;
	stream = new ReadStream(streamChaniId);
	stream->Init("http://192.168.120.95:8090/stream1-0.flv");
	StreamInfo Info2 = { true, true, true, { { 0, 240 + 240 }, { 320, 240 + 240 }, { 0, 240 + 240 + 240 }, { 320, 240 + 240 + 240 } }, 2 };
	works->_liveMCU->InsetStream(stream, &Info2);
	//
	*/
	streamChaniId = 3;
	stream = new ReadStream(streamChaniId);
	stream->Init("http://192.168.120.95:8090/stream0-0.flv");
	StreamInfo Info3 = { true, true, true, { { 320, 0 }, { 1280, 0 }, { 320, 720 }, { 1280, 720 } }, 3 };
	works->_liveMCU->InsetStream(stream, &Info3);
	
#endif
	//
	works->_liveMCU->_transStream->Init("http://192.168.120.95:8090/feed3.ffm");//("http://172.16.173.188:8090/stream3-0.flv");
	//
	works->RenewJpeg("c://works/test/mcu.wav", kWavFile);
	works->RenewJpeg("c://works/video/Lighthouse.jpg", kInset);
	works->RenewJpeg("c://works/video/jy-logo.JPG", kWaterMark);
	works->RenewWaterMark(10, -1, 10, -1, 1.0, 1);
	//
	Sleep(60 * 1000);
	works->RenewRecordStatus(kRecordInSet, 9);
	Sleep(60 * 1000);
	works->RenewRecordStatus(kRecordStart, 9);
	Sleep(60 * 1000);
	///works->ExchangeChannel(1, 3);
	//
	Sleep(10000000000);
}
#else
void MCUTest()
{
	//MediaMCU mcu;
	//read stream
	//strcpy(mcu.sourceStream[0].inFile, "http://172.16.172.230:8090/stream0-0.flv");
	//strcpy(mcu.sourceStream[0].inFile, "http://172.16.172.230:8090/stream1-0.flv");
	//strcpy(mcu.sourceStream[0].inFile, "http://172.16.172.230:8090/stream2-0.flv");
	//write stream
	void *codecHandle[2][kMaxTrannel + 2] = {};
	int hndIdx[2][kMaxTrannel + 2] = {};
	//
	int ret = 0;
	int chanId = 0;
	int64_t streamId = 0;
	//enum {kVideoChannel, kAudioChannel};

	char name[kMaxTrannel + 2][MAX_PATH] = 
	{ 
		"http://172.16.172.230:8090/stream0-0.flv",
		"http://172.16.172.230:8090/stream0-0.flv",
		"http://172.16.172.230:8090/stream0-0.flv"
	};
	//"http://fwss.xiu.youku.com/live/f/v1/000000000000000000000000153AFFE5?token=98765";

	void *pFactory = NULL;
	FF_FACTORY_CREATE(&pFactory, 1);

	for (int i = 0; i < 3; i++)
	{
		chanId = i;	streamId = i;
		hndIdx[0][chanId] = FF_AVCODEC_CREATE(&codecHandle[0][chanId]);
		FF_CODEC_SET_FILENAME(codecHandle[0][chanId], name[i], 0);
		ret = FF_CODEC_INIT(codecHandle[0][chanId], kAVDecStream);
		FF_CODEC_SET_CHANID((void *)codecHandle[0][chanId], chanId);
		FF_CODEC_SET_STREAMID((void *)codecHandle[0][chanId], streamId);
	}
	//liveMCU
	int frameType = 0;
	char * inBuf[3] = {};//?
	int len = 0;
	char *outBuf[3] = {};//?
	int oLen = 0;
	//video
	char *InParams[2] = {};//?
	chanId = kMaxTrannel;	streamId = kMaxTrannel;
	hndIdx[kVideoChannel][chanId] = FF_AVCODEC_CREATE(&codecHandle[kVideoChannel][chanId]);
	FF_CODEC_SET_PARAMS(codecHandle[kVideoChannel][chanId], InParams[0]);
	ret = FF_CODEC_INIT(codecHandle[kVideoChannel][chanId], kVideoEncStream);
	FF_CODEC_SET_CHANID((void *)codecHandle[kVideoChannel][chanId], chanId);
	FF_CODEC_SET_STREAMID((void *)codecHandle[kVideoChannel][chanId], streamId);
	//
	ret = FF_CODEC_CODEDED(codecHandle[kVideoChannel][chanId], inBuf, len, outBuf, oLen, &frameType, kVideoEncStream);
	//audio
	chanId = kMaxTrannel;	streamId = kMaxTrannel;
	hndIdx[kAudioChannel][chanId] = FF_AVCODEC_CREATE(&codecHandle[kAudioChannel][chanId]);
	FF_CODEC_SET_PARAMS(codecHandle[kAudioChannel][chanId], InParams[0]);
	ret = FF_CODEC_INIT(codecHandle[kAudioChannel][chanId], kAudioEncStream);
	FF_CODEC_SET_CHANID((void *)codecHandle[kAudioChannel][chanId], chanId);
	FF_CODEC_SET_STREAMID((void *)codecHandle[kAudioChannel][chanId], streamId);
	//
	len = 8192;
	ret = FF_CODEC_CODEDED(codecHandle[kAudioChannel][chanId], inBuf, len, outBuf, oLen, &frameType, kAudioEncStream);
	//monitorMCU
	//video
	//char *InParams[2] = {};
	chanId = kMaxTrannel + 1;	streamId = kMaxTrannel + 1;
	hndIdx[kVideoChannel][chanId] = FF_AVCODEC_CREATE(&codecHandle[kVideoChannel][chanId]);
	FF_CODEC_SET_PARAMS(codecHandle[kVideoChannel][chanId], InParams[0]);
	ret = FF_CODEC_INIT(codecHandle[kVideoChannel][chanId], kVideoEncStream);
	FF_CODEC_SET_CHANID((void *)codecHandle[kVideoChannel][chanId], chanId);
	FF_CODEC_SET_STREAMID((void *)codecHandle[kVideoChannel][chanId], streamId);
	//
	
	ret = FF_CODEC_CODEDED(codecHandle[kVideoChannel][chanId], inBuf, len, outBuf, oLen, &frameType, kVideoEncStream);
	//audio
	chanId = kMaxTrannel + 1;	streamId = kMaxTrannel + 1;
	hndIdx[kAudioChannel][chanId] = FF_AVCODEC_CREATE(&codecHandle[kAudioChannel][chanId]);
	FF_CODEC_SET_PARAMS(codecHandle[kAudioChannel][chanId], InParams[0]);
	ret = FF_CODEC_INIT(codecHandle[kAudioChannel][chanId], kAudioEncStream);
	FF_CODEC_SET_CHANID((void *)codecHandle[kAudioChannel][chanId], chanId);
	FF_CODEC_SET_STREAMID((void *)codecHandle[kAudioChannel][chanId], streamId);
	//
	len = 8192;
	ret = FF_CODEC_CODEDED(codecHandle[kAudioChannel][chanId], inBuf, len, outBuf, oLen, &frameType, kAudioEncStream);
	//release
	FF_FACTORY_DELETE(pFactory);//all
	//
	for (int i = 0; i < 3; i++)
	{
		chanId = i;	streamId = i;
		ret = FF_AVCODEC_DELETE((void *)&hndIdx[0][chanId]);//someone
		codecHandle[0][chanId] = NULL;
	}
	//liveMCU
	chanId = kMaxTrannel;	streamId = kMaxTrannel;
	ret = FF_AVCODEC_DELETE((void *)&hndIdx[kVideoChannel][chanId]);
	codecHandle[kVideoChannel][chanId] = NULL;
	ret = FF_AVCODEC_DELETE((void *)&hndIdx[kAudioChannel][chanId]);
	codecHandle[kAudioChannel][chanId] = NULL;
	//monitorMCU
	chanId = kMaxTrannel + 1;	streamId = kMaxTrannel + 1;
	ret = FF_AVCODEC_DELETE((void *)&hndIdx[kVideoChannel][chanId]);
	codecHandle[kVideoChannel][chanId] = NULL;
	ret = FF_AVCODEC_DELETE((void *)&hndIdx[kAudioChannel][chanId]);
	codecHandle[kAudioChannel][chanId] = NULL;

}
#endif
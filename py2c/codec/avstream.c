
#include "inc.h"

extern cJSON* mystr2json(char *text);
extern int GetvalueInt(cJSON *json, char *key);
extern char* GetvalueStr(cJSON *json, char *key);
extern int *GetArrayValueInt(cJSON *json, char *key, int *arraySize);
extern cJSON* renewJson(cJSON *json, char *key, int ivalue, char *cvalue, cJSON *subJson);
extern cJSON* renewJsonInt(cJSON *json, char *key, int ivalue);
extern cJSON* renewJsonStr(cJSON *json, char *key, char *cvalue);
extern cJSON* deleteJson(cJSON *json);
extern long long *GetArrayObj(cJSON *json, char *key, int *arraySize);
extern cJSON* renewJsonArray2(cJSON *json, char *key, short *value);
extern cJSON* renewJsonArray3(cJSON **json, cJSON **array, char *key, cJSON *item);
extern cJSON *get_net_info2(NetInfo *info, cJSON *pJsonRoot);
extern inline int GetAudioNetInfo(uint8_t* dataPtr, int insize, NetInfo *info);

extern int64_t get_sys_time();

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
}AVMuxer;
typedef struct
{
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
}AVDeMuxer;
typedef struct
{
    int Obj_id;
    cJSON *json;
    int is_write;//else is_read
    int is_file;//else is_stream
    int is_audio;//else is_video
    AVDeMuxer *demux;
	AVMuxer *mux;
}AVStreamObj;


HCSVC_API
int api_create_avstream_handle(char *handle)
{
    long long ret = 0;
    if(handle)
    {
        AVStreamObj *obj = (AVStreamObj *)calloc(1, sizeof(AVStreamObj));

        ret = (long long)obj;
        //handle = (void *)obj;
        int handle_size = sizeof(long long);
        printf("api_create_avstream_handle: handle_size= %d \n", handle_size);
        printf("api_create_avstream_handle: obj= %x \n", obj);
        memcpy(handle, &ret, handle_size);
        long long *testp = (long long *)handle;
        printf("api_create_avstream_handle: testp[0]= %x \n", testp[0]);
        int id = (int)obj;//long long ???
        obj->Obj_id = id;
        ret = id;
    }
    return (int)(ret & 0x7FFFFFFF);
}

HCSVC_API
int api_avstream_close(char *handle)
{
    long long ret = 0;
    if(handle)
    {
        long long *testp = (long long *)handle;
        AudioCodecObj *obj = (AudioCodecObj *)testp[0];
        //

        //
        free(obj);
	    testp[0] = 0;
        printf("api_avstream_close: ok \n");
    }
    return (int)ret;
}
HCSVC_API
int api_avstream_init(char *handle, char *param)
{
    int ret = 0;

    ret = api_create_avstream_handle(handle);
    long long *testp = (long long *)handle;
    AVStreamObj *obj = (AVStreamObj *)testp[0];
    printf("api_avstream_init: param= %s \n", param);
    obj->json = mystr2json(param);
    printf("api_avstream_init: obj->json= %x \n", obj->json);
#if 0
    obj->print = GetvalueInt(obj->json, "print");

    obj->codec_type = GetvalueInt(obj->json, "codec_type");
    obj->out_nb_samples = GetvalueInt(obj->json, "out_nb_samples");
    obj->out_channels = GetvalueInt(obj->json, "out_channels");
    obj->out_sample_rate = GetvalueInt(obj->json, "out_sample_rate");


    char *text = GetvalueStr(obj->json, "codec_id");
	if (!strcmp(text, "AV_CODEC_ID_AAC"))
	{
       obj->codec_id = (enum AVCodecID)(AV_CODEC_ID_AAC);
	}
#endif
	av_register_all();

    return ret;
}

static int write_frame(AVStreamObj *stream, AVPacket *pkt0)
{
	AVCodecContext *c = stream->demux->codecCtx;
	int ret = 0;
	int frame_number = c->frame_number;

	if(stream->mux->fmtctx && stream->mux->WriteHead)//pCodec->mux->RecordStatus
	{
		AVPacket pkt = { 0 };
		av_init_packet(&pkt);

		pkt.data = pkt0->data;
		pkt.size = pkt0->size;
		pkt.duration = pkt0->duration;
		pkt.pts = pkt0->pts;
		pkt.dts = pkt0->dts;
		//pkt.stream_index = pkt0->stream_index;
		pkt.stream_index = pCodec->st->index;
		pkt.flags = pkt0->flags;

		if(stream->is_file)
		{
#if 0
			//avformat_write_header()
			//__int64 time_stamp = (c->frame_number - 1) * 40;

			av_packet_rescale_ts(&pkt, c->time_base, stream->st->time_base);
#elif(1)
			int64_t time_stamp = get_sys_time();
			int64_t time_diff = time_stamp - stream->mux->time0;
			pkt.dts = pkt.pts = time_diff * stream->st->time_base.den / 1000;
#else
			int64_t time_stamp = get_sys_time();
			int64_t time_diff = time_stamp - stream->mux->time0;
			int64_t fram_num = 0;
			//framnum = difftime/frametime
			if(stream->is_audio)
			{
				//framtime = c->sample_rate / c->frame_size
				//framnum = time_diff / framtime
				fram_num = (int64_t)((double)time_diff * c->frame_size / c->sample_rate + 0.5);
				fram_num = c->frame_size * fram_num;
			}
			else
			{
				fram_num = (int64_t)((double)time_diff * c->time_base.den / 1000 + 0.5);
				fram_num = (stream->st->time_base.den / c->time_base.den) * fram_num;
			}

			printf("av=%d \t fn=%d \n", stream->is_audio, fram_num);
			pkt.dts = pkt.pts = fram_num;
#endif
		}
		else
		{
			int64_t time_stamp = get_sys_time();
			pkt.pts = time_stamp;//(48000 / 25);
			pkt.dts = time_stamp;//(48000 / 25);
		}
		if (stream->mux->AVWriteFlag == 3)
		{
			ret = av_interleaved_write_frame(stream->mux->fmtctx, &pkt);
		}
		else
		{
			ret = av_write_frame(stream->mux->fmtctx, &pkt);
		}

		if(ret < 0)
		{
			printf("error \n");
		}
	}
    //fprintf(logfp,"frame_write2 1 %d \n",ret); fflush(logfp);

	return ret;
}
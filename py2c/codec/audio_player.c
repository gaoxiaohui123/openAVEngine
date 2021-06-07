

#include "inc.h"

/**
*
* 该版本使用SDL 2.0替换了第一个版本中的SDL 1.0。
* 注意：SDL 2.0中音频解码的API并无变化。唯一变化的地方在于
* 其回调函数的中的Audio Buffer并没有完全初始化，需要手动初始化。
* 本例子中即SDL_memset(stream, 0, len);
*
* This software decode and play audio streams.
* Suitable for beginner of FFmpeg.
*
* This version use SDL 2.0 instead of SDL 1.2 in version 1
* Note:The good news for audio is that, with one exception,
* it's entirely backwards compatible with 1.2.
* That one really important exception: The audio callback
* does NOT start with a fully initialized buffer anymore.
* You must fully write to the buffer in all cases. In this
* example it is SDL_memset(stream, 0, len);
*
* Version 2.0
*/

//#define __STDC_CONSTANT_MACROS

extern int glob_sld_status;

#define MAX_MIX_NUM 16

typedef struct
{
    int Obj_id;
    cJSON *json;
    char *param;
    AVAudioFifo* audiofifo;
    SDL_AudioSpec wanted_spec;
    uint32_t audio_len;
    uint8_t **audio_pos;
    int out_buffer_size;
    uint8_t ** audio_data_buffer;
    AVFrame* audio_frame;
    int out_channels;
    int out_nb_samples;
    int out_sample_fmt;
    int out_sample_rate;
    uint64_t out_channel_layout;// = AV_CH_LAYOUT_STEREO;
    uint64_t frame_idx;
    char *tmpbuf;
    char playbuf[8192 * MAX_MIX_NUM];
    int mix_num;
    int sdl_status;
    int print;
    FILE *fp_pcm;

}AudioPlayerObj;

extern cJSON* mystr2json(char *text);
extern int GetvalueInt(cJSON *json, char *key);
extern char* GetvalueStr(cJSON *json, char *key, char *result);
extern int64_t get_sys_time();

void  fill_audio(void *handle, Uint8 *stream, int len){
#if 0
    //注意：某些情况下会导致崩溃
    long long *testp = (long long *)handle;
    AudioPlayerObj *obj = (AudioPlayerObj *)testp[0];
    printf("fill_audio: testp[0]=%lld \n", testp[0]);
    printf("fill_audio: obj=%lld \n", obj);
    printf("fill_audio: obj=%x \n", obj);
#else
    AudioPlayerObj *obj = (AudioPlayerObj *)handle;
#endif
    //SDL 2.0
    SDL_memset(stream, 0, len);
    if (obj->audio_len == 0)
        return;
    //printf("fill_audio: 2: len= %d \n", len);

    len = (len > obj->audio_len ? obj->audio_len : len);    /*  Mix  as  much  data  as  possible  */
    //SDL_MixAudio(Uint8*       dst,
    //              const Uint8* src,
    //              Uint32       len,
    //              int          volume)
    int mix_num = obj->mix_num;//GetvalueInt(obj->json, "mix_num");
    //printf("fill_audio: mix_num= %d \n", mix_num);
    //mix_num = 1;//test
    int maxvolume = SDL_MIX_MAXVOLUME / mix_num;
    for(int i = 0; i < mix_num; i++)
    {
        //if(i)//test
        SDL_MixAudio(stream, obj->audio_pos[i], len, maxvolume);
    }
    //printf("fill_audio: mix_num= %d \n", mix_num);
    //printf("fill_audio: obj->audio_pos[0]= %x \n", obj->audio_pos[0]);
    //printf("fill_audio: obj->audio_len= %d \n", obj->audio_len);
    for(int i = 0; i < mix_num; i++)
    {
        obj->audio_pos[i] += len;//此步骤很重要！！！
    }
    //obj->audio_pos[1] += len;
    obj->audio_len -= len;
    //printf("fill_audio: len= %d \n", len);
}
#if 0
void  fill_audio(void *udata, Uint8 *stream, int len){
	SDL_memset(stream, 0, len);
	while (len > 0)
	{
		if (audio_len == 0)
			continue;
		int temp = (len > audio_len ? audio_len : len);
		SDL_MixAudio(stream, audio_pos, temp, SDL_MIX_MAXVOLUME);
		audio_pos += temp;
		audio_len -= temp;
		stream += temp;
		len -= temp;
	}
}
#endif

HCSVC_API
int api_create_player_handle(char *handle)
{
    long long ret = 0;
    if(handle)
    {
        AudioPlayerObj *obj = (AudioPlayerObj *)calloc(1, sizeof(AudioPlayerObj));
        //CodecObj *obj = (CodecObj *)&global_codec_objs[id];
        ret = (long long)obj;
        //handle = (void *)obj;
        int handle_size = sizeof(long long);
        printf("api_create_player_handle: handle_size= %d \n", handle_size);
        printf("api_create_player_handle: obj= %x \n", obj);
        printf("api_create_player_handle: ret= %x \n", ret);
        memcpy(handle, &ret, handle_size);
        long long *testp = (long long *)handle;
        printf("api_create_player_handle: testp[0]= %x \n", testp[0]);
        int id = (int)obj;//long long ???
        obj->Obj_id = id;
        ret = id;
    }
    return (int)(ret & 0x7FFFFFFF);
}
HCSVC_API
void api_player_close(char *handle)
{
    if(handle)
    {
        long long *testp = (long long *)handle;
        AudioPlayerObj *obj = (AudioPlayerObj *)testp[0];
        printf("api_player_close \n");

        if(!obj->sdl_status)
        {
            SDL_CloseAudio();//Close SDL//注意：如果视频退出，则不要重复退出，否则，线程无法退出
            printf("api_player_close: SDL_CloseAudio ok \n");
            SDL_Quit();//注意：如果视频退出，则不要重复退出
            printf("api_player_close: SDL_Quit ok \n");
        }
        glob_sld_status = 0;
        if(obj->audio_pos)
        {
            free(obj->audio_pos);
        }
        if(obj->tmpbuf)
        {
            av_free(obj->tmpbuf);
        }
        if (obj->audiofifo)
        {
            av_audio_fifo_free(obj->audiofifo);
        }
        if(obj->audio_frame)
        {
            av_frame_free(&obj->audio_frame);
        }
        if(obj->fp_pcm)
        {
            fclose(obj->fp_pcm);
        }
        if(obj->json)
        {
            api_json_free(obj->json);
            obj->json = NULL;
        }
        free(obj);
	    testp[0] = 0;
        printf("api_player_close: ok \n");
    }
}
HCSVC_API
int api_player_init(char *handle, char *param)
{
    int ret = 0;

    ret = api_create_player_handle(handle);
    long long *testp = (long long *)handle;
    AudioPlayerObj *obj = (AudioPlayerObj *)testp[0];

    printf("api_player_init: param= %s \n", param);
    obj->json = mystr2json(param);
    printf("api_player_init: obj->json= %x \n", obj->json);
    obj->param = param;
    obj->print = GetvalueInt(obj->json, "print");
    obj->sdl_status = GetvalueInt(obj->json, "sdl_status");
    char filename[256] = "";
    GetvalueStr(obj->json, "pcmfile", filename);
    if (strcmp(filename, ""))
    {
        printf("api_player_init: pcmfile= %s \n", filename);
        obj->fp_pcm = fopen(filename, "wb");
    }
    obj->mix_num = GetvalueInt(obj->json, "mix_num");
    obj->audio_pos = calloc(1, sizeof(uint8_t *) * MAX_MIX_NUM);//obj->mix_num);
    for(int i = 0; i < MAX_MIX_NUM; i++)
    {
        obj->audio_pos[i] = NULL;
    }

    obj->out_sample_rate = GetvalueInt(obj->json, "out_sample_rate");

    char cformat[64] = "";
    GetvalueStr(obj->json, "format", cformat);
	if (!strcmp(cformat, "AUDIO_S16SYS"))
	{
        obj->wanted_spec.format = AUDIO_S16SYS;// 采样格式：S表带符号，16是采样深度(位深)，SYS表采用系统字节序，这个宏在SDL中定义
        long testformat = AUDIO_S16SYS;
        printf("api_player_init: AUDIO_S16SYS=0x%x \n", testformat);
	}
	else{
	    obj->wanted_spec.format = AUDIO_F32;
	}
	obj->out_sample_fmt = GetvalueInt(obj->json, "out_sample_fmt");
	GetvalueStr(obj->json, "out_sample_fmt", cformat);
	if (!strcmp(cformat, "AV_SAMPLE_FMT_S16"))
	{
        obj->out_sample_fmt = AV_SAMPLE_FMT_S16;
	}

	GetvalueStr(obj->json, "out_channel_layout", cformat);
	if (!strcmp(cformat, "AV_CH_LAYOUT_STEREO"))
	{
        obj->out_channel_layout = AV_CH_LAYOUT_STEREO;
	}
	else if (!strcmp(cformat, "AV_CH_LAYOUT_MONO"))
	{
        obj->out_channel_layout = AV_CH_LAYOUT_MONO;
	}

    printf("api_player_init: obj->out_channel_layout= %d \n", obj->out_channel_layout);

	obj->out_channels = GetvalueInt(obj->json, "out_channels");
	obj->out_nb_samples = GetvalueInt(obj->json, "out_nb_samples");
    obj->wanted_spec.channels = obj->out_channels;
    obj->wanted_spec.freq = obj->out_sample_rate;
    obj->wanted_spec.silence = GetvalueInt(obj->json, "silence");// 静音值
    obj->wanted_spec.samples = obj->out_nb_samples;
    obj->wanted_spec.callback = fill_audio;// 回调函数，若为NULL，则应使用SDL_QueueAudio()机制
    obj->wanted_spec.userdata = obj;//pCodecCtx;
    printf("obj->wanted_spec.channels=%d obj->wanted_spec.samples=%d obj->wanted_spec.freq=%d obj->wanted_spec.format=%x \n",
        obj->wanted_spec.channels,
        obj->wanted_spec.samples,
        obj->wanted_spec.freq,
        obj->wanted_spec.format);
    // 打开音频设备并创建音频处理线程。期望的参数是wanted_spec，实际得到的硬件参数是spec
    // 1) SDL提供两种使音频设备取得音频数据方法：
    //    a. push，SDL以特定的频率调用回调函数，在回调函数中取得音频数据
    //    b. pull，用户程序以特定的频率调用SDL_QueueAudio()，向音频设备提供数据。此种情况wanted_spec.callback=NULL
    // 2) 音频设备打开后播放静音，不启动回调，调用SDL_PauseAudio(0)后启动回调，开始正常播放音频
    printf("api_player_init: obj->out_nb_samples= %d \n", obj->out_nb_samples);
    obj->out_buffer_size = av_samples_get_buffer_size(NULL, obj->out_channels, obj->out_nb_samples, obj->out_sample_fmt, 1);

    obj->audiofifo = av_audio_fifo_alloc(obj->out_sample_fmt, obj->out_channels, obj->mix_num);
    printf("api_player_init: obj->audiofifo= %x \n", obj->audiofifo);
    obj->tmpbuf = av_malloc((obj->out_buffer_size << 1) * obj->mix_num);
    //obj->audio_frame = av_frame_alloc();
#if 0
    obj->audio_frame->nb_samples = obj->out_nb_samples;
    obj->audio_frame->channel_layout = obj->out_channel_layout;
    obj->audio_frame->format = obj->out_sample_fmt;
    obj->audio_frame->sample_rate = obj->out_sample_rate;
    av_frame_get_buffer(obj->audio_frame, 0);
#endif

#ifdef _WIN32
    //char *cmd = "set SDL_AUDIODRIVER=directsound";
    //system(cmd);
    SetEnvironmentVariableA("SDL_AUDIODRIVER", "directsound");
#endif

#if 0
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    //if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        printf("api_player_init: Could not initialize SDL - %s\n", SDL_GetError());
        return -1;
    }
#endif
    SDL_AudioSpec obtained;// = NULL;
    if (SDL_OpenAudio(&obj->wanted_spec, &obtained) < 0){
        printf("api_player_init: can't open audio.\n");
        return -1;
    }
    printf("obtained.channels=%d obtained.samples=%d obtained.freq=%d obtained.format=%x \n",
        obtained.channels,
        obtained.samples,
        obtained.freq,
        obtained.format);
#if 0
    if(obtained.samples != obj->wanted_spec.samples)
    {
        obj->out_nb_samples = obtained.samples;
        obj->wanted_spec.samples = obj->out_nb_samples;
        obj->out_buffer_size = av_samples_get_buffer_size(NULL, obj->out_channels, obj->out_nb_samples, obj->out_sample_fmt, 1);
    }
#endif
    //Play
    //SDL_PauseAudio(0);
    obj->frame_idx = 0;
    printf("api_player_init: obj=%x \n", obj);
	return ret;
}

HCSVC_API
int audio_play_frame(char *handle, char *param, char *indata, int insize)
{
    int ret;
    if(handle)
    {
        long long *testp = (long long *)handle;
        AudioPlayerObj *obj = (AudioPlayerObj *)testp[0];
        obj->json = mystr2json(param);
        obj->param = (void *)obj->json;//param;

        obj->mix_num = GetvalueInt(obj->json, "mix_num");

        int out_framesize = obj->out_nb_samples;//obj->out_buffer_size;//
        int try_frame_size = av_samples_get_buffer_size(NULL, obj->out_channels, obj->out_nb_samples, obj->out_sample_fmt, 1);
        int factor = try_frame_size / obj->out_nb_samples;
        int convert_size = insize / factor;
        obj->audio_data_buffer = (uint8_t **)&indata;

        if(obj->fp_pcm)
        {
            //fwrite(indata, 1, insize, obj->fp_pcm);
        }

        if(!obj->frame_idx)
        {
            printf("audio_play_frame: start play \n");
            SDL_PauseAudio(0);
        }
        //obj->audiofifo = av_audio_fifo_alloc(obj->out_sample_fmt, obj->out_channels, obj->mix_num);
        int fifo_size = av_audio_fifo_size(obj->audiofifo);
        ret = av_audio_fifo_realloc(obj->audiofifo, av_audio_fifo_size(obj->audiofifo) + convert_size);
        //printf("audio_play_frame: fifo_size=%d, ret=%d \n", fifo_size, ret);
        if (ret < 0){
            printf("audio_play_frame: av_audio_fifo_realloc error\n");
            return -1;
        }
        fifo_size = av_audio_fifo_size(obj->audiofifo);
        int write_size = av_audio_fifo_write(obj->audiofifo, (void **)obj->audio_data_buffer, convert_size);
        if (write_size < convert_size){
            printf("audio_play_frame: av_audio_fifo_write error\n");
            return -1;
        }
        fifo_size = av_audio_fifo_size(obj->audiofifo);
        //printf("audio_play_frame: fifo_size=%d, out_framesize=%d \n", fifo_size, out_framesize);
        while ((fifo_size = av_audio_fifo_size(obj->audiofifo)) >= out_framesize)
        {
            //printf("audio_play_frame: fifo_size=%d, out_framesize=%d \n", fifo_size, out_framesize);
            int frame_size = FFMIN(av_audio_fifo_size(obj->audiofifo), out_framesize);
            if(1)
            {
                //av_frame_unref(&obj->audio_frame);
                if(false)
                {
                    //av_frame_free(&obj->audio_frame);
                    obj->audio_frame = av_frame_alloc();
                }
                obj->audio_frame = av_frame_alloc();

                obj->audio_frame->nb_samples = frame_size;
                obj->audio_frame->channel_layout = obj->out_channel_layout;
                obj->audio_frame->format = obj->out_sample_fmt;
                obj->audio_frame->sample_rate = obj->out_sample_rate;
                av_frame_get_buffer(obj->audio_frame, 0);
            }
            //printf("audio_play_frame: obj->audio_frame->data[0]=%x \n", obj->audio_frame->data[0]);
            //printf("audio_play_frame: obj->audio_frame->linesize[0]=%d \n", obj->audio_frame->linesize[0]);
            int read_size = av_audio_fifo_read(obj->audiofifo, (void **)obj->audio_frame->data, frame_size);
            //printf("audio_play_frame: read_size=%d \n", read_size);
            if (read_size < frame_size)
            {
                printf("audio_play_frame: av_audio_fifo_read error\n");
                return -1;
            }
            //if ((obj->wanted_spec.samples != frame_size2) && false)
            if (obj->wanted_spec.samples != frame_size)
            {
                printf("warning: audio_play_frame: frame_size2= %d \n");
                SDL_CloseAudio();
                obj->out_nb_samples = frame_size;//frame_size2;
                obj->out_buffer_size = av_samples_get_buffer_size(NULL, obj->out_channels, obj->out_nb_samples, obj->out_sample_fmt, 1);
                printf("audio_play_frame: obj->out_buffer_size=%d \n", obj->out_buffer_size);
                obj->wanted_spec.samples = obj->out_nb_samples;
                SDL_OpenAudio(&obj->wanted_spec, NULL);
            }
            //printf("audio_play_frame: 1: obj->audio_len=%d \n", obj->audio_len);
            while (obj->audio_len > 0)//Wait until finish
            {
                SDL_Delay(1);
            }
            //printf("audio_play_frame: 2: obj->audio_len=%d \n", obj->audio_len);
            //obj->audio_len = obj->out_buffer_size;
            obj->audio_pos[0] = *obj->audio_frame->data;
#if 1
            memcpy((void *)obj->playbuf, obj->audio_frame->data[0], insize);
            if(obj->audio_frame && true)
            {
                av_frame_free(&obj->audio_frame);
                obj->audio_frame = NULL;
                //obj->audio_frame = av_frame_alloc();
            }
            obj->audio_pos[0] = (uint8_t *)obj->playbuf;
#endif
            obj->audio_len = obj->out_buffer_size;
            //obj->audio_pos = obj->audio_frame->data[0];
        }
        //av_free(obj->audio_frame);//test
        obj->frame_idx++;
        deleteJson(obj->param);
        obj->param = NULL;
        obj->json = NULL;
    }

    return ret;
}
HCSVC_API
int audio_play_frame_mix(char *handle, char *param, char *indata[], int insize)
{
    int ret = 0;
    if(handle)
    {
        //printf("audio_play_frame_mix: insize=%d \n", insize);
        long long *testp = (long long *)handle;
        AudioPlayerObj *obj = (AudioPlayerObj *)testp[0];
        obj->json = mystr2json(param);
        obj->param = (void *)obj->json;

        if(obj->fp_pcm)
        {
            fwrite(indata[0], 1, insize, obj->fp_pcm);
        }
        if(!obj->frame_idx)
        {
            printf("audio_play_frame_mix: start play \n");
            SDL_PauseAudio(0);
        }
        int mix_num = GetvalueInt(obj->json, "mix_num");
        if(mix_num > MAX_MIX_NUM)
        {
            mix_num = MAX_MIX_NUM;
            printf("audio_play_frame_mix: too many mix_num= %d \n", mix_num);
        }
        if(mix_num != obj->mix_num)
        {
            if(mix_num > obj->mix_num)
            {
                MYPRINT("audio_play_frame_mix: mix_num= %d \n", mix_num);
                MYPRINT("audio_play_frame_mix: obj->mix_num= %d \n", obj->mix_num);
                if(obj->tmpbuf)
                {
                    av_free(obj->tmpbuf);
                    obj->tmpbuf = NULL;
                }
                if (obj->audiofifo)
                {
                    av_audio_fifo_free(obj->audiofifo);
                    obj->audiofifo = NULL;
                }
                obj->audiofifo = av_audio_fifo_alloc(obj->out_sample_fmt, obj->out_channels, mix_num);
                obj->tmpbuf = av_malloc((obj->out_buffer_size << 1) * mix_num);
            }
            //obj->mix_num = mix_num;
        }

        int out_framesize = obj->out_nb_samples;//obj->out_buffer_size;//
        int try_frame_size = av_samples_get_buffer_size(NULL, obj->out_channels, obj->out_nb_samples, obj->out_sample_fmt, 1);
        int factor = try_frame_size / obj->out_nb_samples;
        int convert_size = insize / factor;
        int fifo_size = 0;
        int offset = 0;
        ret = av_audio_fifo_realloc(obj->audiofifo, av_audio_fifo_size(obj->audiofifo) + convert_size * mix_num);
        //printf("audio_play_frame_mix: ret=%d \n", ret);
        if (ret < 0){
            printf("audio_play_frame_mix: av_audio_fifo_realloc error\n");
            return -1;
        }
        obj->audio_data_buffer = (uint8_t **)&obj->tmpbuf;
        for(int i = 0; i < mix_num; i++)
        {
            memcpy(&obj->tmpbuf[offset], indata[i], insize);
            offset += insize;
        }
        //printf("audio_play_frame: obj->out_nb_samples=%d \n", obj->out_nb_samples);
        //printf("audio_play_frame: convert_size=%d \n", convert_size);
        //printf("audio_play_frame: obj->out_buffer_size=%d \n", obj->out_buffer_size);
        //printf("audio_play_frame: offset=%d \n", offset);
        //obj->audio_data_buffer = (uint8_t **)&indata;
        int write_size = av_audio_fifo_write(obj->audiofifo, (void **)obj->audio_data_buffer, convert_size * mix_num);
        if (write_size < convert_size * mix_num){
            printf("audio_play_frame_mix: av_audio_fifo_write error\n");
            return -1;
        }

        //out_framesize = convert_size * mix_num;
        int frame_size = convert_size * mix_num;
        fifo_size = av_audio_fifo_size(obj->audiofifo);
        //printf("audio_play_frame_mix: fifo_size=%d, out_framesize=%d \n", fifo_size, out_framesize);
        while (av_audio_fifo_size(obj->audiofifo) >= out_framesize)
        {
            //int frame_size = FFMIN(av_audio_fifo_size(obj->audiofifo), out_framesize);
            if(1)
            {
                if(true)//obj->audio_frame &&
                {
                    //av_frame_free(&obj->audio_frame);
                    obj->audio_frame = av_frame_alloc();
                }
                obj->audio_frame->nb_samples = frame_size;
                obj->audio_frame->channel_layout = obj->out_channel_layout;
                obj->audio_frame->format = obj->out_sample_fmt;
                obj->audio_frame->sample_rate = obj->out_sample_rate;
                av_frame_get_buffer(obj->audio_frame, 0);
            }
            //printf("audio_play_frame_mix: obj->audio_frame->data[0]=%x \n", obj->audio_frame->data[0]);
            //printf("audio_play_frame_mix: obj->audio_frame->linesize[0]=%d \n", obj->audio_frame->linesize[0]);
            //printf("audio_play_frame_mix: obj->audio_frame->data[1]=%x \n", obj->audio_frame->data[1]);
            //printf("audio_play_frame_mix: obj->audio_frame->linesize[1]=%d \n", obj->audio_frame->linesize[1]);
            int read_size = av_audio_fifo_read(obj->audiofifo, (void **)obj->audio_frame->data, frame_size);
            //printf("audio_play_frame_mix: read_size=%d \n", read_size);

            //fifo_size = av_audio_fifo_size(obj->audiofifo);
            //printf("audio_play_frame_mix: 2: fifo_size=%d, out_framesize=%d \n", fifo_size, out_framesize);
            if (read_size < frame_size)
            {
                printf("audio_play_frame_mix: av_audio_fifo_read error\n");
                return -1;
            }
            //if ((obj->wanted_spec.samples != frame_size2) && false)
            if (obj->wanted_spec.samples != frame_size && false)
            {
                printf("warning: audio_play_frame_mix: frame_size2= %d \n");
                SDL_CloseAudio();
                obj->out_nb_samples = frame_size;//frame_size2;
                obj->out_buffer_size = av_samples_get_buffer_size(NULL, obj->out_channels, obj->out_nb_samples, obj->out_sample_fmt, 1);
                printf("audio_play_frame_mix: obj->out_buffer_size=%d \n", obj->out_buffer_size);
                obj->wanted_spec.samples = obj->out_nb_samples;
                SDL_OpenAudio(&obj->wanted_spec, NULL);
            }
            //printf("audio_play_frame_mix: 1: obj->audio_len=%d \n", obj->audio_len);
            while (obj->audio_len > 0 && glob_sld_status != 2)//Wait until finish
            {
                SDL_Delay(1);
                //printf("audio_play_frame_mix: 2: obj->audio_len=%d \n", obj->audio_len);
                //printf("audio_play_frame_mix: 2: glob_sld_status=%d \n", glob_sld_status);

            }
            //printf("audio_play_frame_mix: 3: obj->audio_len=%d \n", obj->audio_len);
            obj->mix_num = mix_num;
            //printf("audio_play_frame: ok \n");

            //obj->audio_len = obj->out_buffer_size;
            //printf("audio_play_frame_mix: obj->audio_len=%d \n", obj->audio_len);
            uint8_t *p = obj->audio_frame->data[0];
#if 1
            memcpy((void *)obj->playbuf, obj->audio_frame->data[0], insize * mix_num);
            p = (uint8_t *)obj->playbuf;
            if(obj->audio_frame && true)
            {
                av_frame_free(&obj->audio_frame);
                //obj->audio_frame = av_frame_alloc();
                obj->audio_frame = NULL;
            }
#endif
            for(int i = 0; i < mix_num; i++)
            {
                obj->audio_pos[i] = p;
                p += insize;
            }
            if(insize != obj->out_buffer_size)
            {
                printf("audio_play_frame_mix: insize=%d \n", insize);
                printf("audio_play_frame_mix: obj->out_buffer_size=%d \n", obj->out_buffer_size);
            }
            //obj->audio_pos[0] = indata[0];
            //obj->audio_pos = obj->audio_frame->data[0];
            obj->audio_len = obj->out_buffer_size;
            //obj->mix_num = mix_num;
            //printf("audio_play_frame_mix: obj->out_buffer_size=%d \n", obj->out_buffer_size);
            //printf("audio_play_frame_mix: out_framesize=%d \n", out_framesize);

            //printf("audio_play_frame_mix: obj=%x \n", obj);
        }

        //av_free(obj->audio_frame);//test
        obj->frame_idx++;
        deleteJson(obj->param);
        obj->param = NULL;
        obj->json = NULL;
    }

    return ret;
}

Uint32  audio_len;
Uint8  *audio_pos;
AVAudioFifo* audiofifo = NULL;

static int main_test(int argc, char* argv[])
{
    AVFormatContext *pFormatCtx;
    int             i, audioStream;
    AVCodecContext  *pCodecCtx;
    AVCodec         *pCodec;
    AVPacket        *packet;

    AVFrame         *pFrame;
    SDL_AudioSpec wanted_spec;
    int ret;
    uint32_t len = 0;
    int got_picture;
    int index = 0;
    int64_t in_channel_layout;
    struct SwrContext *au_convert_ctx;

    FILE *pFile = NULL;
    char url[] = "rtmp://live.hkstv.hk.lxdns.com/live/hks";

    ///av_register_all();
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();

    if (avformat_open_input(&pFormatCtx, url, NULL, NULL) != 0){
        printf("Couldn't open input stream.\n");
        return -1;
    }

    if (avformat_find_stream_info(pFormatCtx, NULL)<0){
        printf("Couldn't find stream information.\n");
        return -1;
    }

    av_dump_format(pFormatCtx, 0, url, false);

    // Find the first audio stream
    audioStream = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++)
    if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
        audioStream = i;
        break;
    }

    if (audioStream == -1){
        printf("Didn't find a audio stream.\n");
        return -1;
    }

    // Get a pointer to the codec context for the audio stream
    pCodecCtx = pFormatCtx->streams[audioStream]->codec;

    // Find the decoder for the audio stream
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL){
        printf("Codec not found.\n");
        return -1;
    }

    // Open codec
    if (avcodec_open2(pCodecCtx, pCodec, NULL)<0){
        printf("Could not open codec.\n");
        return -1;
    }

    packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    av_init_packet(packet);
    int out_framesize = 1024;
    //Out Audio Param
    uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
    //nb_samples: AAC-1024 MP3-1152
    int out_nb_samples = out_framesize;
    //AVSampleFormat
    int out_sample_fmt = AV_SAMPLE_FMT_S16;
    int out_sample_rate = 44100;// pCodecCtx->sample_rate;
    int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);
    //Out Buffer Size
    int out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, out_sample_fmt, 1);

    uint8_t ** audio_data_buffer = NULL;

    audiofifo = av_audio_fifo_alloc(out_sample_fmt, out_channels, 1);

    pFrame = av_frame_alloc();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        printf("Could not initialize SDL - %s\n", SDL_GetError());
        return -1;
    }
    //SDL_AudioSpec
    wanted_spec.freq = out_sample_rate;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.channels = out_channels;
    wanted_spec.silence = 0;
    wanted_spec.samples = out_nb_samples;
    wanted_spec.callback = fill_audio;
    wanted_spec.userdata = pCodecCtx;

    if (SDL_OpenAudio(&wanted_spec, NULL)<0){
        printf("can't open audio.\n");
        return -1;
    }


    //FIX:Some Codec's Context Information is missing
    in_channel_layout = av_get_default_channel_layout(pCodecCtx->channels);
    //Swr

    au_convert_ctx = swr_alloc();
    au_convert_ctx = swr_alloc_set_opts(au_convert_ctx, out_channel_layout, out_sample_fmt, out_sample_rate,
        in_channel_layout, pCodecCtx->sample_fmt, pCodecCtx->sample_rate, 0, NULL);
    swr_init(au_convert_ctx);

    //Play
    SDL_PauseAudio(0);

    AVFrame* audio_frame = av_frame_alloc();

    while (av_read_frame(pFormatCtx, packet) >= 0){
        if (packet->stream_index == audioStream){
            ret = avcodec_decode_audio4(pCodecCtx, pFrame, &got_picture, packet);
            if (ret < 0) {
                printf("Error in decoding audio frame.\n");
                return -1;
            }
            if (got_picture > 0){
                av_samples_alloc_array_and_samples(&audio_data_buffer, NULL, out_channels, pFrame->nb_samples, out_sample_fmt, 1);

                int convert_size = swr_convert(au_convert_ctx, audio_data_buffer, pFrame->nb_samples,
                    (const uint8_t**)pFrame->data, pFrame->nb_samples);


                ret = av_audio_fifo_realloc(audiofifo, av_audio_fifo_size(audiofifo) + convert_size);
                if (ret < 0){
                    printf("av_audio_fifo_realloc error\n");
                    return -1;
                }
                if (av_audio_fifo_write(audiofifo, (void **)audio_data_buffer, convert_size) < convert_size){
                    printf("av_audio_fifo_write error\n");
                    return -1;
                }
                while (av_audio_fifo_size(audiofifo) >= out_framesize){
                    int frame_size = FFMIN(av_audio_fifo_size(audiofifo), out_framesize);
                    audio_frame->nb_samples = frame_size;
                    audio_frame->channel_layout =out_channel_layout;
                    audio_frame->format = out_sample_fmt;
                    audio_frame->sample_rate = out_sample_rate;
                    av_frame_get_buffer(audio_frame, 0);
                    if (av_audio_fifo_read(audiofifo, (void **)audio_frame->data, frame_size) < frame_size){
                        printf("av_audio_fifo_read error\n");
                        return -1;
                    }
                    if (wanted_spec.samples != frame_size){
                        SDL_CloseAudio();
                        out_nb_samples = frame_size;
                        out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, out_sample_fmt, 1);

                        wanted_spec.samples = out_nb_samples;
                        SDL_OpenAudio(&wanted_spec, NULL);

                    }
                    while (audio_len>0)//Wait until finish
                        SDL_Delay(1);


                    audio_len = out_buffer_size;
                    audio_pos = *audio_frame->data;

                }

            }
        }
        av_free_packet(packet);
    }

    swr_free(&au_convert_ctx);

    SDL_CloseAudio();//Close SDL
    SDL_Quit();

    if (audiofifo)
        av_audio_fifo_free(audiofifo);

    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);

    return 0;
}


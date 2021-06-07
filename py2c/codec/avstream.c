
#include "inc.h"

extern cJSON* mystr2json(char *text);
extern int GetvalueInt(cJSON *json, char *key);
extern char* GetvalueStr(cJSON *json, char *key, char *result);
extern int *GetArrayValueInt(cJSON *json, char *key, int *arraySize);
extern cJSON* renewJson(cJSON *json, char *key, int ivalue, char *cvalue, cJSON *subJson);
extern cJSON* renewJsonInt(cJSON *json, char *key, int ivalue);
extern cJSON* renewJsonStr(cJSON *json, char *key, char *cvalue);
extern cJSON* deleteJson(cJSON *json);
extern long long *GetArrayObj(cJSON *json, char *key, int *arraySize);
//extern cJSON* renewJsonArray2(cJSON *json, char *key, short *value);
extern cJSON* renewJsonArray3(cJSON **json, cJSON **array, char *key, cJSON *item);
extern cJSON *get_net_info2(NetInfo *info, cJSON *pJsonRoot);

extern void *ResampleCreate(void *handle);
extern void ResampleClose(void *handle);
extern int ResampleParams(void *resampleHandle, unsigned char *data[3], int format, int channels, int nb_samples,int sample_rate, int flag);
extern int ResampleInit(void *resampleHandle);
extern int ResampleFrame(void *scaleHandle);

extern int FFGetFormat(char *cformat);
extern void *FFScaleCreate(void *handle);
extern void FFScaleClose(void *handle);
extern int FFScaleParams(void *scaleHandle, int format, unsigned char *y, unsigned char *u, unsigned char *v, int width, int stride, int height, int flag);
extern int FFScaleInit(void *scaleHandle);
extern int FFScaleFrame(void *scaleHandle);

extern int64_t get_sys_time();


int av_read_push_data(AVDeMuxer *demux, int avtype, uint8_t *data, int size, int64_t now_time)
{
    int ret = 0;
    //printf("av_read_push_data: avtype= %d \n", avtype);
    AVReadNode *head,*pnew, *p, *q;
    pthread_mutex_t *mutex = &demux->mutex;
    pthread_mutex_lock(mutex);
    AVReadNode **readhead = NULL;
    if(avtype == kIsVideo)
    {
        readhead = &demux->videohead;
    }
    else{
        readhead = &demux->audiohead;
    }
    if(!*readhead)
    {
        //head不存数据,head->next指向第一个数据,head->num保存总索引（也可以设计将第一个节点存入head）
        //注意链表元的2个组成部分，元和next指针，元控制的是整个链表元的地址，next指针则寄生在元下；
        head = (AVReadNode *)calloc(1, sizeof(AVReadNode));  //创建头节点。
        head->num = 0;
        head->next = NULL;  //头节点指针域置NULL
        head->tail = head;  // 开始时尾指针指向头节点；此时head->tail->next与head->next是同一地址
        *readhead = (void *)head;
    }
    head = (AVReadNode *)(*readhead);
    //printf("av_read_push_data: head->num=%d \n", head->num);
    if(head->num > 4)
    {
        //printf("av_read_push_data: head->num=%d \n", head->num);
        //AVReadNode *first_node = head->next;
        //AVReadNode *last_node = head->tail;
        p = head->next;
        q = head->next->next;
        head->next = q;
        head->frame_timestamp = q->frame_timestamp;
        if(p->data)
        {
            av_free(p->data);
        }
        if(p)
        {
            free(p);
        }
        head->num--;
    }
    {
        pnew = (AVReadNode *)calloc(1, sizeof(AVReadNode));  //创建新节点
        pnew->data = data;
        pnew->size = size;
        pnew->idx = head->idx;
        pnew->frame_timestamp = now_time;
        pnew->next = NULL;   //新节点指针域置NULL
        head->tail->next = pnew;  //新节点插入到表尾；首个节点时，则意味着ead->next也指向了pnew
        head->tail = pnew;   //为指针指向当前的尾节点;元地址迁移，元的原先的next脱离了元，新地址的next成为元的next指针；
        head->idx++;
        head->num++;
    }
    pthread_mutex_unlock(mutex);
    return ret;
}
void * av_read_pop_data(AVDeMuxer *demux, int avtype)
{
    void *ret = NULL;
    pthread_mutex_t *mutex = &demux->mutex;
    pthread_mutex_lock(mutex);
    AVReadNode **readhead = NULL;
    if(avtype == kIsVideo)
    {
        readhead = &demux->videohead;
    }
    else{
        readhead = &demux->audiohead;
    }
    AVReadNode *head, *q;
    head = (AVReadNode *)(*readhead);
    //printf("av_read_pop_data: head=%x \n", head);
    if(head)
    {
        q = head->next;
        if(q == NULL || q == head)
        {
        }
        else{
            head->next = head->next->next;
            ret = q;
            head->num--;
            if(head->next == NULL)
            {
                head->next = NULL;  //头节点指针域置NULL
                head->tail = head;
            }
        }
    }
    pthread_mutex_unlock(mutex);
    return ret;
}
static int interrupt_cb(void *hnd)
{
	AVDeMuxer *dmux = (AVDeMuxer *)hnd;

	int64_t time1 = get_sys_time();
	int difftime = (int)(time1 - dmux->time0);
	if (difftime > dmux->timeout && dmux->timeout)
	{
	    printf("interrupt_cb: difftime= %d \n", difftime);
		return 1;
	}
	return 0;
}
static const AVIOInterruptCB int_cb = { interrupt_cb, NULL };

void GetCharTime(char *name)
{
	//char tmp[64];
	time_t t = time(0);
	strftime(name, 64, "%Y%m%d%H%M%S", localtime(&t));
}
HCSVC_API
void api_get_char_time(char *name)
{
    GetCharTime(name);
}
static int RenewFilename(char *oldname, char *head, char *tail)
{
	char newname[255] = "";
	if (oldname == NULL)
		return -1;
	GetCharTime(newname);
	strcpy(oldname, head);
	strcat(oldname, "-");
	strcat(oldname, newname);
	strcat(oldname, tail);
	//
	/*mystrcat(oldname, newname, '_');
	strcpy(oldname, newname);*/
	return 0;
}
static void RenewFilename2(AVStreamObj *pWrite, char *new_name)
{
	//char new_name[255] = "";
	char* pPos = NULL;
	char id[8] = "";
	char tail[16] = "";
	char *filename = new_name;
	strcpy(new_name, pWrite->outfile);
	///itoa(pWrite->levelId, id, 10);
	pPos = strrchr(filename, '.');
	strcpy(tail, pPos);
	*pPos = '\0';
	//char *tail = pWrite->fileType;
	RenewFilename(filename, filename, tail);
}
int WriteTail(AVFormatContext *oc)
{
	if(!oc)
		return -1;
	return av_write_trailer(oc);
}
int WriteHeader(AVFormatContext *oc)
{
	/* Write the stream header, if any. */
	int ret = 1;

	if(oc->pb)
	{
		ret = avformat_write_header(oc, NULL);
		printf("WriteHeader: ret= %d \n", ret);
		if (ret < 0) {
            fprintf(stderr, "WriteHeader: %s\n", av_err2str(ret));
        }
	}

	return ret;
}
int WriteStart(AVStreamObj *pWrite, int splitLen)
{
    int64_t now_time = get_sys_time();
    if(!pWrite->mux->start_time)
    {
        pWrite->mux->time0 = now_time;
    }
    pWrite->mux->time0 = now_time;
	///webrtc::TickTime::MillisecondTimestamp();
	pWrite->mux->split_len = splitLen;
	printf("WriteStart: 1: pWrite->mux->ast->time_base.den=%d \n", pWrite->mux->ast->time_base.den);//test
	pWrite->mux->WriteHead = WriteHeader(pWrite->mux->fmtctx) == 0;
	//printf("status = %x \n", (unsigned int)&pWrite->mux);
	printf("WriteStart: pWrite->mux->WriteHead= %d \n", pWrite->mux->WriteHead);
	printf("WriteStart: 2: pWrite->mux->ast->time_base.den=%d \n", pWrite->mux->ast->time_base.den);//test
	return pWrite->mux->WriteHead;
}
int WriteStop(AVStreamObj *pWrite)
{
	int ret = 0;
	char *filename = pWrite->outfile;
	AVFormatContext **oc = &pWrite->mux->fmtctx;
	printf("WriteStop: 0 \n");
	ret = WriteTail(*oc);
	avio_close((*oc)->pb);
	(*oc)->pb = NULL;
	(*oc)->internal->nb_interleaved_streams = 0;
	printf("WriteStop: (*oc)->filename=%s \n", (*oc)->filename);
#if 0
	//删除保存的文件
	if (remove((*oc)->filename) == 0)
	{
		printf("WriteStop: Removed %s \n", filename);
		printf("WriteStop: 2: (*oc)->filename=%s \n", (*oc)->filename);
	}
	else
	{
		perror("WriteStop:remove \n");
		remove(filename);
	}
#endif
    //strcpy((*oc)->filename, "");
    avformat_free_context(*oc);
    pWrite->mux->fmtctx = NULL;
	printf("WriteStop: ret=%d \n", ret);
	return ret;
}
int WriteOpen(AVStreamObj *pWrite)
{
	int ret = 0;
	AVFormatContext **oc = &pWrite->mux->fmtctx;
	AVOutputFormat *fmt = (*oc)->oformat;
	char *filename = pWrite->outfile;
    //printf("WriteOpen: pWrite->mux->ast->time_base.den=%d \n", pWrite->mux->ast->time_base.den);
#if 1
    filename = (*oc)->filename;
    if(!strcmp(filename, ""))
    {
        char new_name[255] = "";
	    if (pWrite->is_file)
	    {
	    	RenewFilename2(pWrite, new_name);
	    	filename = new_name;
	    	//strcpy(filename, new_name);
	    }
    }
#endif
	printf("WriteOpen: fmt= %x \n", fmt);
	printf("WriteOpen: filename= %s \n", filename);

	if (!(fmt->flags & AVFMT_NOFILE))
	{
		if(filename && *oc && (*oc)->pb == NULL)
		{
		    //(*oc)->interrupt_callback.callback = interrupt_cb;
			//(*oc)>interrupt_callback.opaque = (void *)pWrite->mux;
			do
			{
			    printf("WriteOpen: avio_open start ! \n");
				ret = avio_open(&(*oc)->pb, filename, AVIO_FLAG_WRITE);
				//ret = avio_open2(&mux->urlStr.ofmt_ctx->pb, mux->urlStr.filename, AVIO_FLAG_WRITE, &mux->urlStr.ofmt_ctx->interrupt_callback, dict);
				if(ret < 0)
				{
					printf("WriteOpen:avio_open failed: filename=%s !\n", filename);
					ret = -1;
				}
				else
				{
				    strcpy((*oc)->filename, filename);//test
				}
			}while(ret < 0);
		}
	}
	else
	{
	    printf("WriteOpen: fmt->flags= %d \n", fmt->flags);
	}
	//printf("WriteOpen: pWrite->mux->fmtctx->streams[0]= %x \n", pWrite->mux->fmtctx->streams[0]);
	//printf("WriteOpen: pWrite->mux->fmtctx->streams[1]= %x \n", pWrite->mux->fmtctx->streams[1]);

	printf("WriteOpen: ret= %d \n", ret);
	return ret;
}
static int get_extend(char *filename, char *extend, int levelId, int fileOrStream)
{
	int ret = 0;
	char *pPos = NULL;
	char cLevelId[32] = "";
	if (fileOrStream == kIsFile)
	{
		pPos = strrchr(filename,'.');
		if(pPos)
		{
			strcpy(extend, ++pPos);
		}
	}
	return ret;
}
int WriteInit(AVStreamObj *pWrite)
{
	int ret = 0;
	char *filename = pWrite->outfile;

	pWrite->is_file = kIsFile;//test
#if 1
	char new_name[255] = "";
	if (pWrite->is_file)
	{
		RenewFilename2(pWrite, new_name);
		//strcpy(filename, new_name);
		filename = new_name;
	}
#endif
	AVFormatContext **oc = &pWrite->mux->fmtctx;

	get_extend(filename, pWrite->fileType, 0, pWrite->is_file);
	printf("WriteInit: pWrite->fileType=%s \n", pWrite->fileType);
	pWrite->mux->ofmt = av_guess_format(pWrite->fileType, NULL, NULL);

	printf("WriteInit: pWrite->mux->ofmt=%x \n", pWrite->mux->ofmt);
	if (pWrite->mux->ofmt)
	{
	    AVOutputFormat *fmt = pWrite->mux->ofmt;//(*oc)->oformat;
	    printf("WriteInit: pWrite->mux->ofmt->name=%s \n", pWrite->mux->ofmt->name);
		//AVOutputFormat *ofmt = av_guess_format(pWrite->fileType, NULL, NULL);
		//avformat_alloc_output_context2(&mux->stream[i].oc, NULL, /*"mpeg"*/"flv", mux->stream[i].filename);
		avformat_alloc_output_context2(oc, pWrite->mux->ofmt, pWrite->fileType, filename);
		//avformat_alloc_output_context2(oc, NULL, pWrite->fileType, filename);
		printf("WriteInit: *oc=%x \n", *oc);
		printf("WriteInit: (*oc)->streams= %x \n", (*oc)->streams);
		printf("WriteInit: fmt->video_codec=%d \n", fmt->video_codec);
		printf("WriteInit: fmt->audio_codec=%d \n", fmt->audio_codec);
		char *vname = avcodec_get_name(fmt->video_codec);
		char *aname = avcodec_get_name(fmt->audio_codec);
		printf("WriteInit: vname=%s \n", vname);
		printf("WriteInit: aname=%s \n", aname);
		printf("WriteInit: AV_CODEC_ID_H264=%d \n", AV_CODEC_ID_H264);
		printf("WriteInit: AV_CODEC_ID_AAC=%d \n", AV_CODEC_ID_AAC);
		fmt = (*oc)->oformat;
		printf("WriteInit: 2: fmt->video_codec=%d \n", fmt->video_codec);
		printf("WriteInit: 2: fmt->audio_codec=%d \n", fmt->audio_codec);
		vname = avcodec_get_name(fmt->video_codec);
		aname = avcodec_get_name(fmt->audio_codec);
		printf("WriteInit: 2: vname=%s \n", vname);
		printf("WriteInit: 2: aname=%s \n", aname);
		//
		//printf("WriteInit: (*oc)->streams[0]= %x \n", (*oc)->streams[0]);
	    //printf("WriteInit: (*oc)->streams[1]= %x \n", (*oc)->streams[1]);
		if(!*oc)
		{
			printf("WriteInit: Could not deduce output format from file extension: using MPEG.\n");
			return -1;
		}
	}
	(*oc)->flags |= AVFMT_FLAG_FLUSH_PACKETS;
	return ret;
}
int WiteFrame(AVStreamObj *stream, AVPacket *pkt0, int avtype)
{
    AVMuxer *mux = (AVMuxer *)stream->mux;
    AVStream *st = avtype == kIsVideo ? mux->vst : mux->ast;
	AVCodecContext *c = st->codec;

    //printf("WiteFrame: st=%x, st->time_base.den=%d, avtype=%d \n", st, st->time_base.den, avtype);
    //printf("WiteFrame: st->nb_frames=%d, st->start_time=%lld, avtype=%d \n", st->nb_frames, st->start_time, avtype);
    //printf("WiteFrame: st->first_dts=%lld, st->cur_dts=%lld, st->last_IP_pts=%lld \n", st->first_dts, st->cur_dts, st->last_IP_pts);
    //mux->ast->time_base = (AVRational){1, 48000};//test

	int ret = 0;
	//int frame_number = c->frame_number;
    int64_t now_time = get_sys_time();
	if(mux->fmtctx && mux->WriteHead)//pCodec->mux->RecordStatus
	{
		AVPacket pkt = { 0 };
		av_init_packet(&pkt);

		pkt.data = pkt0->data;
		pkt.size = pkt0->size;
		pkt.duration = pkt0->duration;
		pkt.pts = pkt0->pts;
		pkt.dts = pkt0->dts;
		//pkt.stream_index = pkt0->stream_index;
		pkt.stream_index = st->index;//
		//pkt.stream_index = 1 - st->index; //test
		pkt.flags = pkt0->flags;

		if(stream->is_file)
		{
#if 0
			//avformat_write_header()
			//__int64 time_stamp = (c->frame_number - 1) * 40;
			if(avtype == kIsVideo)
            {
                long long *testp = (long long *)stream->video_codec_handle;
                CodecObj *obj = (CodecObj *)testp[0];
                c = obj->c;
            }
            else{
                long long *testp = (long long *)stream->audio_codec_handle;
                AudioCodecObj *obj = (AudioCodecObj *)testp[0];
                c = obj->c;
            }
            printf("WiteFrame: st->time_base.den=%d \n", st->time_base.den);
            printf("WiteFrame: c->time_base.den=%d \n", c->time_base.den);
			av_packet_rescale_ts(&pkt, c->time_base, st->time_base);
			printf("WiteFrame: pkt.pts=%d \n", pkt.pts);
			printf("WiteFrame: pkt.dts=%d \n", pkt.dts);
			pkt.stream_index = st->index;
#elif(1)
			int64_t time_stamp = now_time;//get_sys_time();
			int64_t time_diff = time_stamp - stream->mux->time0;
			//int64_t time_diff = time_stamp - stream->mux->start_time;
			//printf("WiteFrame: st->time_base.den=%d \n", st->time_base.den);
			pkt.dts = pkt.pts = time_diff * st->time_base.den / 1000;
			//test
			if(avtype == kIsAudio)
			{
			    //pkt.dts = pkt.pts = 0;
			}
#else
			int64_t time_stamp = now_time;//get_sys_time();
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
				fram_num = (mux->st->time_base.den / c->time_base.den) * fram_num;
			}

			printf("av=%d \t fn=%d \n", stream->is_audio, fram_num);
			pkt.dts = pkt.pts = fram_num;
#endif
		}
		else
		{
			int64_t time_stamp = now_time;//get_sys_time();
			pkt.pts = time_stamp;//(48000 / 25);
			pkt.dts = time_stamp;//(48000 / 25);
		}
		if (mux->AVWriteFlag == 3)
		{
		    //printf("WiteFrame: pkt.size=%d \n", pkt.size);
			ret = av_interleaved_write_frame(mux->fmtctx, &pkt);
		}
		else
		{
			ret = av_write_frame(mux->fmtctx, &pkt);
		}

		if(ret < 0)
		{
		    //int diff = now_time - mux->last_time;
		    printf("WiteFrame: avtype=%d, st->index=%d, ret=%d \n", avtype, st->index, ret);
			//fprintf(stderr, "WiteFrame: %s\n", av_err2str(ret));
			//av_log(NULL, AV_LOG_DEBUG, "WiteFrame: %s\n", av_err2str(ret));
		}
		av_free_packet(&pkt);
	}
	mux->last_time = now_time;
    //fprintf(logfp,"frame_write2 1 %d \n",ret); fflush(logfp);

	return ret;
}
int WiteFrame2(AVStreamObj *stream, AVPacket *pkt, int avtype)
{
    int ret = 0;
    int splitLen = SPLIT_LEN;//2 * 60 * 60 * 1000;//2hours
    //splitLen = 1 * 60 * 1000;
    AVMuxer *mux = (AVMuxer *)stream->mux;
    AVStream *st = avtype == kIsVideo ? mux->vst : mux->ast;
    int64_t now_time = get_sys_time();
    int difftime = (int)(now_time - mux->time0);

    pthread_mutex_lock(&mux->mutex);
#if 0
    //test
    if(avtype == kIsVideo)
    {
    }
    else{
        if(!mux->AVWriteFlag)
        {
            pthread_mutex_unlock(&mux->mutex);
            return ret;
        }
    }
    //
#endif
    if(!(mux->AVWriteFlag & (1 << (avtype - 1))))
    {
        if(avtype == kIsVideo)
        {
            long long *testp = (long long *)stream->video_codec_handle;
            CodecObj *obj = (CodecObj *)testp[0];
            mux->AVWriteFlag |= (1 << (avtype - 1));//1;
            video_add_stream(obj, stream);
        }
        else{
            long long *testp = (long long *)stream->audio_codec_handle;
            AudioCodecObj *obj = (AudioCodecObj *)testp[0];
            mux->AVWriteFlag |= (1 << (avtype - 1));//2;
            audio_add_stream(obj, stream);
        }
        if (mux->AVWriteFlag == 3)
        {
            ret = WriteOpen(stream);
            ret = WriteStart(stream, splitLen);
        }
    }
    //st = avtype == kIsVideo ? mux->vst : mux->ast;
    //if(st)
    //{
    //    printf("WiteFrame2: st=%x, st->time_base.den=%d, avtype=%d \n", st, st->time_base.den, avtype);
    //}

    if (mux->AVWriteFlag == 3)
    {
        if((difftime >= splitLen || !mux->time0) && ((avtype == kIsVideo) && pkt->flags))
        //if((difftime >= splitLen || !mux->time0) && ((avtype == kIsAudio)))
        {
            if(mux->time0)
            {
                WriteStop(stream);
            }
#if 1
            WriteInit(stream);
            {
                long long *testp = (long long *)stream->audio_codec_handle;
                AudioCodecObj *obj = (AudioCodecObj *)testp[0];
                mux->AVWriteFlag |= 2;
                audio_add_stream(obj, stream);
            }
            //if(avtype == kIsVideo)
            {
                long long *testp = (long long *)stream->video_codec_handle;
                CodecObj *obj = (CodecObj *)testp[0];
                mux->AVWriteFlag |= 1;
                video_add_stream(obj, stream);
            }
#endif

#if 0
            //mux->ast->time_base = (AVRational){1, 48000};//test
            mux->ast->nb_frames = 0;
            mux->vst->nb_frames = 0;

            mux->ast->start_time = 0;
            mux->ast->first_dts = 0;
            mux->ast->cur_dts = 0;
            mux->ast->last_IP_pts = 0;

            mux->ast->start_time = 0;
            mux->ast->first_dts = 0;
            mux->ast->cur_dts = 0;
            mux->ast->last_IP_pts = 0;
#endif
            ret = WriteOpen(stream);
            ret = WriteStart(stream, splitLen);
        }
        ret = WiteFrame(stream, pkt, avtype);
    }
    else{
        printf("WiteFrame2: mux->AVWriteFlag=%d \n", mux->AVWriteFlag);
        printf("WiteFrame2: avtype=%d \n", avtype);
    }

    pthread_mutex_unlock(&mux->mutex);

    return ret;
}
int AVWriteFrame(AVStreamObj *stream, char *data, char *param, char *outbuf, char *outparam[])
{
    int ret = -1;
    cJSON *json = mystr2json(param);
    char *key = "get_pkt";
	int ivalue = 1;
	json = renewJsonInt(json, key, ivalue);
    AVMuxer *mux = (AVMuxer *)stream->mux;
    int is_video = GetvalueInt(json, "is_video");
    char *data2 = data;
    if(is_video)
    {
        int width = GetvalueInt(json, "width");
        int height = GetvalueInt(json, "height");

        key = "width";
		ivalue = width;
		json = renewJsonInt(json, key, ivalue);

		key = "height";
		ivalue = height;
		json = renewJsonInt(json, key, ivalue);

        if(width != stream->width || height != stream->height)
        {
            int src_width = stream->width;
            int src_height = stream->height;
            int dst_width = width;
            int dst_height = height;
            int size = src_width * src_height;
            int size2 = dst_width * dst_height;
            if(mux->scale == NULL)
            {
                mux->scale = (FFScale *)FFScaleCreate(mux->scale);
            }
            if(mux->scale)
            {
                void *scale = mux->scale;
                if(!stream->picData)
                {
                    stream->picData = av_malloc((size2 * 3) >> 1);
                }
                data2 = stream->picData;
                unsigned char *sY = (unsigned char *)&data[0];
                unsigned char *sU = (unsigned char *)&data[size];
                unsigned char *sV = (unsigned char *)&data[size + (size >> 2)];
                unsigned char *dY = (unsigned char *)&data2[0];
                unsigned char *dU = (unsigned char *)&data2[size2];
                unsigned char *dV = (unsigned char *)&data2[size2 + (size2 >> 2)];
                int stride_s = src_width;
                int stride_d = dst_width;

                FFScaleParams(scale,AV_PIX_FMT_YUV420P, sY, sU, sV, src_width, stride_s, src_height, 0);
                FFScaleParams(scale,AV_PIX_FMT_YUV420P, dY, dU, dV, dst_width , stride_d, dst_height, 1);
                FFScaleInit(scale);
                FFScaleFrame(scale);
            }
        }
        char *param2 = cJSON_Print(json);
        api_video_encode_one_frame(stream->video_codec_handle, data2, param2, outbuf, outparam);

        long long *testp = (long long *)stream->video_codec_handle;
        CodecObj *obj = (CodecObj *)testp[0];
        if(strcmp(stream->outfile, ""))
        {
            if(stream->is_file)
            {
            }
            pthread_mutex_lock(&mux->mutex);
            ret = WiteFrame(stream, &obj->pkt, 0);
            pthread_mutex_unlock(&mux->mutex);
        }
        av_free_packet(&obj->pkt);
        free(param2);
    }
    else{
        char *param2 = cJSON_Print(json);
        api_audio_codec_one_frame(stream->audio_codec_handle, data2, param2, outbuf, outparam);
        long long *testp = (long long *)stream->audio_codec_handle;
        CodecObj *obj = (CodecObj *)testp[0];

        if(strcmp(stream->outfile, ""))
        {
            if(stream->is_file)
            {
            }
            pthread_mutex_lock(&mux->mutex);
            ret = WiteFrame(stream, &obj->pkt, 0);
            pthread_mutex_unlock(&mux->mutex);
        }

        av_free_packet(&obj->pkt);
        free(param2);
    }
    deleteJson(json);
    return ret;
}
int ReadInit(AVStreamObj *pRead)
{
	int ret = 0;
	char szError[256] = {0};
	printf("ReadInit: 0 \n");
	AVDeMuxer *demux = (AVDeMuxer *)pRead->demux;

    demux->width = pRead->width;
    demux->height = pRead->height;
	demux->timeout = 500;//2000;//2s
	demux->time0 = get_sys_time();
	demux->fmtctx = avformat_alloc_context();
	demux->fmtctx->interrupt_callback.callback = interrupt_cb;
	demux->fmtctx->interrupt_callback.opaque = (void *)demux;

	char option_key[]="probesize";
	char option_value[] = "30000000";

	char option_key2[]="analyzeduration";//"max_analyze_duration";
	char option_value2[] = "30000000";
	//char option_value2[]="50000";
	char option_key3[]="rtmp_transport";
	char option_value3[]="tcp";
	//��avformat_find_stream_info����ʱ���5�����ӵ�50��
//	av_dict_set(&demux->opt, "re", "", 0);
	///av_dict_set(&demux->opt,option_key,option_value,0);
	///av_dict_set(&demux->opt,option_key2,option_value2,0);

	char filename[MAX_PATH] = "";
	strcpy(filename, pRead->infile);
#if 1
	if (!strncmp(filename, "rtmp", strlen("rtmp")))
	{
		av_dict_set(&demux->opt, option_key3, option_value3, 0);
		strcat(filename, " live=1");
	}
	else if (!strncmp(filename, "rtsp", strlen("rtsp")))
	{
#if 1
		char option_key[] = "rtsp_transport";
		char option_value[] = "tcp";
		char option_key2[] = "max_delay";
		char option_value2[] = "5000000";

		av_dict_set(&demux->opt, option_key, option_value, 0);
		av_dict_set(&demux->opt, option_key2, option_value2, 0);
#else
		strcat(mux->urlStr.url, "?tcp");
#endif

	}
	else if (!strncmp(filename, "udp", strlen("udp")))
	{
		char * format = "mpegts";
		demux->ifmt = av_find_input_format(format);
		//mpegts udp://127.0.0.1:10000?pkt_size=1316
		strcat(filename, "?pkt_size=1316");
	}
#endif
	//ret = avformat_open_input(&demux->fmtctx, filename, demux->ifmt, &demux->opt);
	ret = avformat_open_input(&demux->fmtctx, filename, demux->ifmt, &demux->opt);
	if (ret != 0)
	{
		if (demux->fmtctx)
		{
			avformat_close_input(&demux->fmtctx);
			demux->fmtctx = NULL;
		}
		av_strerror(ret, szError, 256);
		printf(szError);
		printf("\n");
		printf("ReadInit: Call avformat_open_input function failed!\n");

		return -1;
	}
	demux->timeout = 0;
	if(avformat_find_stream_info(demux->fmtctx, &demux->opt) < 0)
	{
	    printf("ReadInit: avformat_find_stream_info failed!\n");
		return -1;
	}
#if 1
	int stream_index = 0;
	// create stream
	if ((stream_index = av_find_best_stream(demux->fmtctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0)) >= 0)
	{
		AVStream* istream = demux->fmtctx->streams[stream_index];

	}
	if ((stream_index = av_find_best_stream(demux->fmtctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0)) >= 0)
	{
		AVStream* istream = demux->fmtctx->streams[stream_index];
	}
	//for (int i = 0; i < demux->fmtctx->nb_streams && (demux->video_index < 0 || demux->audio_index < 0); i++)
	for (int i = 0; i < demux->fmtctx->nb_streams; i++)
	{
		switch (demux->fmtctx->streams[i]->codec->codec_type)
		{
		case AVMEDIA_TYPE_VIDEO:
			demux->video_index = i;
			demux->fmtctx->streams[i]->discard = AVDISCARD_NONE;
			break;

		case AVMEDIA_TYPE_AUDIO:
			demux->audio_index = i;
			demux->fmtctx->streams[i]->discard = AVDISCARD_NONE;
			break;

		default:
			demux->fmtctx->streams[i]->discard = AVDISCARD_ALL;
			break;

		}
	}

	if (demux->video_index != -1)
	{
		demux->vpCodecCtx = demux->fmtctx->streams[demux->video_index]->codec;
		demux->video_codec = avcodec_find_decoder(demux->vpCodecCtx->codec_id);
		if (demux->video_codec == NULL)
		{
			printf("ReadInit: Call avcodec_find_decoder function failed!\n");
			return -1;
		}

		if (avcodec_open2(demux->vpCodecCtx, demux->video_codec, &demux->opt) < 0)
		{
			printf("ReadInit: Call avcodec_open function failed !\n");
			return -1;
		}

		///mux->urlStr.pFrame = avcodec_alloc_frame();
		///mux->urlStr.pFrameRGB = avcodec_alloc_frame();
	}
	if (demux->audio_index != -1)
	{
		demux->apCodecCtx = demux->fmtctx->streams[demux->audio_index]->codec;
		demux->audio_codec = avcodec_find_decoder(demux->apCodecCtx->codec_id);
		if (demux->audio_codec == NULL)
		{
			printf("ReadInit: Call avcodec_find_decoder function failed!\n");
			return -1;
		}

		if (avcodec_open2(demux->apCodecCtx, demux->audio_codec, &demux->opt) < 0)
		{
			printf("ReadInit: Call avcodec_open function failed !\n");
			return -1;
		}
	}
	demux->frame = av_frame_alloc();
	demux->tmp_frame = av_frame_alloc();
#ifdef GLOB_DEC
	//g_audio_codec = demux->audio_codec;
	g_audio_c = demux->apCodecCtx;
	g_video_c = demux->vpCodecCtx;
#endif
#if 0
	/* create resampler context */
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
			mux->audioFrame = avcodec_alloc_frame();
	}
#endif
	//mst->params.width;
#endif
	return ret;
}
int DecodeAudio(AVStreamObj *pRead, AVPacket pkt, uint8_t *buf)
{
    int ret = 0;
    AVDeMuxer *demux = (AVDeMuxer *)pRead->demux;
    while(pkt.size > 0)
    {
        AVDeMuxer *audio = (AVDeMuxer *)pRead->demux;
		AVCodecContext *c = audio->apCodecCtx;
		AVFrame* frame = audio->frame;
		int got_frame = 0;
		int out_size = -1;
		int ret2 = 0;
		int read_size =
			ret2 = avcodec_decode_audio4(c, frame, &got_frame, &pkt);
		//printf("DecodeAudio: ret2=%d \n", ret2);
		//printf("got_frame: got_frame=%d \n", got_frame);
		//printf("DecodeAudio: frame->nb_samples=%d \n", frame->nb_samples);
		if (ret2 < 0)
		{
			return -2;
		}
		if (got_frame)
		{
			int data_size = av_get_bytes_per_sample(c->sample_fmt);
			//printf("DecodeAudio: data_size=%d \n", data_size);
			size_t unpadded_linesize = frame->nb_samples * av_get_bytes_per_sample(frame->format);

			//printf("DecodeAudio: unpadded_linesize=%d \n", unpadded_linesize);
			audio->tmp_frame->data[0] = (uint8_t *)&audio->buf[audio->pos];// outBuf[0][offset];
			audio->tmp_frame->data[1] = NULL;

			if (audio->resample == NULL)
			{
				audio->resample = (FFResample *)ResampleCreate(audio->resample);
			}
			frame->extended_data;
			ResampleParams((void *)audio->resample, frame->data, frame->format, frame->channels, frame->nb_samples, frame->sample_rate, 0);
			//printf("DecodeAudio: frame->sample_rate=%d \n", frame->sample_rate);
			//printf("DecodeAudio: frame->channels=%d \n", frame->channels);
			int dst_nb_samples = (48000 * frame->nb_samples) / frame->sample_rate;// 44100;
			//printf("DecodeAudio: dst_nb_samples=%d \n", dst_nb_samples);
			ResampleParams((void *)audio->resample, audio->tmp_frame->data, AV_SAMPLE_FMT_S16, 2, dst_nb_samples, 48000, 1);
			ResampleInit((void *)audio->resample);
			//printf("DecodeAudio: ResampleFrame: audio->resample=%x \n", audio->resample);
			//printf("DecodeAudio:frame->data[5]=%x \n", frame->data[5]);
			ret2 = ResampleFrame((void *)audio->resample);
			//printf("DecodeAudio: ret2=%d \n", ret2);
			ret2 = data_size * ret2;
			audio->pos += ret2;
            //printf("DecodeAudio: audio->pos=%d \n", audio->pos);
			if (audio->pos >= 8192)
			{
                int size = 8192;
                int64_t now_time = get_sys_time();
                buf = av_malloc(size);
				memcpy(&buf[0], audio->buf, 8192);
				av_read_push_data(demux, kIsAudio, buf, size, now_time);
				audio->pos -= 8192;
				//printf("DecodeAudio: audio->pos=%d \n", audio->pos);
				if (audio->pos)
				{
					memmove(audio->buf, &audio->buf[8192], audio->pos);
				}
				out_size = 8192 + 0;
			}
			if (out_size > 0)
			{
				ret++;
			}
		}
		else{
		    pkt.data += read_size;
            pkt.size -= read_size;
		    //printf("got_frame: got_frame=%d, pkt.size=%d \n", got_frame, pkt.size);
		    break;
		}
		pkt.data += read_size;
        pkt.size -= read_size;
        //printf("DecodeAudio: pkt.size=%d \n", pkt.size);
	}
    return ret;
}
int DecodeVideo(AVStreamObj *pRead, AVPacket pkt, uint8_t *outBuf)
{
    int ret = 0;
    //printf("DecodeVideo \n");
    AVDeMuxer *demux = (AVDeMuxer *)pRead->demux;
    AVCodecContext *c = demux->vpCodecCtx;
    AVFrame* frame = demux->frame;
	//
	//printf("DecodeVideo: demux->fmtctx=%x \n", demux->fmtctx);
	//AVStream* istream = demux->fmtctx->streams[demux->video_index];
	int got_picture = 0;
	int ret2 = avcodec_decode_video2(c, frame, &got_picture, &pkt);
	//printf("DecodeVideo: ret2=%d \n", ret2);
	//printf("DecodeVideo: got_picture=%d \n", got_picture);
    if(got_picture)
    {
        //Y
        int w = frame->width;
        int h = frame->height;
        int w2 = demux->width;
        int h2 = demux->height;
        //printf("DecodeVideo: w=%d, h=%d \n", w, h);
        //printf("DecodeVideo: w2=%d, h2=%d \n", w2, h2);
        int size = (w * h * 3) >> 1;
        int size2 = (w2 * h2 * 3) >> 1;
        int64_t now_time = get_sys_time();
        outBuf = av_malloc(size2);
#if 1
        if(!demux->img_convert_ctx)
        {
            demux->img_convert_ctx = sws_getContext(    w,
	                                                    h,
	                                                    frame->format,
	                                                    w2,
	                                                    h2,
	                                                    AV_PIX_FMT_YUV420P,
	                                                    SWS_BILINEAR,
	                                                    NULL, NULL, NULL);
        }
        if(demux->img_convert_ctx)
        {
            //AVFrame src_frame = {};//注意：初始化至关重要，否则会导致异常崩溃
            //src_frame.data[0] = outbuf;
            //src_frame.data[1] = &outbuf[size];
            //src_frame.data[2] = &outbuf[size + (size >> 2)];
            //src_frame.linesize[0] = w;
            //src_frame.linesize[1] = w >> 1;
            //src_frame.linesize[2] = w >> 1;
            //src_frame.width = w;
	        //src_frame.height = h;
            //src_frame.format = AV_PIX_FMT_YUV420P;
            AVFrame dst_frame = {};//注意：初始化至关重要，否则会导致异常崩溃
            dst_frame.data[0] = outBuf;
            dst_frame.data[1] = &outBuf[w2 * h2];
            dst_frame.data[2] = &outBuf[(w2 * h2) + ((w2 >> 1) * (h2 >> 1))];
            dst_frame.linesize[0] = w2;
            dst_frame.linesize[1] = w2 >> 1;
            dst_frame.linesize[2] = w2 >> 1;
            dst_frame.width = w2;
	        dst_frame.height = h2;
            dst_frame.format = AV_PIX_FMT_YUV420P;
            sws_scale(  demux->img_convert_ctx,
	                    frame->data,
	    		        frame->linesize, 0,
	    		        h,
	    		        dst_frame.data,
	    		        dst_frame.linesize);
	        ret = size2 > 0;
        }
#else
        char *y = &outBuf[0];
        char *u = &outBuf[w * h];
        char *v = &outBuf[w * h + ((w * h) >> 2)];
		for (int i = 0; i < frame->height; i++)
		{
			memcpy(&y[i * frame->width], &frame->data[0][i * frame->linesize[0]], frame->width);
		}
		//U
		for (int i = 0; i < frame->height; i++)
		{
			if (!(i & 1))
			{
				memcpy(&u[(i >> 1) * (frame->width >> 1)], &frame->data[1][(i >> 1) * frame->linesize[1]], frame->width >> 1);
			}
		}
		//V
		for (int i = 0; i < frame->height; i++)
		{
			if (!(i & 1))
			{
				memcpy(&v[(i >> 1) * (frame->width >> 1)], &frame->data[2][(i >> 1) * frame->linesize[2]], frame->width >> 1);
			}
		}
		ret = (frame->width * frame->height * 3) >> 1;
#endif
		av_read_push_data(demux, kIsVideo, outBuf, size2, now_time);
    }
    return ret;
}
int LoopReadFrame(AVStreamObj *pRead)
{
    int ret = 0;
    AVDeMuxer *demux = (AVDeMuxer *)pRead->demux;
    AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;
    pkt.size = 0;
    int64_t start_time = 0;//get_sys_time();
    int64_t video_frame_num = 0;
    int64_t audio_frame_num = 0;
    float video_interval = 40;//ms
    float audio_interval = (2048 * 1000.0) / 48000.0;//1000 / (48000 / 2048);//50;//ms
    while (ret >= 0) {
        pthread_mutex_lock(&demux->mutex);
        int status = demux->status;
        pthread_mutex_unlock(&demux->mutex);
        if(status == 2)
        {
            printf("LoopReadFrame: status=%d \n", status);
            break;
        }
        else if(status == 0)//pause
        {
            printf("LoopReadFrame: status=%d \n", status);
            usleep(100000);//100ms
            continue;
        }
        int64_t now_time = get_sys_time();
        if(!start_time)
        {
            start_time = now_time;//get_sys_time();
        }
        //printf("LoopReadFrame: status=%d \n", status);
        ret = av_read_frame(demux->fmtctx, &pkt);
        int readtime = (int)(get_sys_time() - now_time);
        //printf("LoopReadFrame: ret=%d, readtime=%d \n", ret, readtime);
        if(ret >= 0)
        {
            uint8_t *outbuf = NULL;
            //printf("LoopReadFrame: pkt.stream_index=%d \n", pkt.stream_index);
            //printf("LoopReadFrame: pkt.data=%x \n", pkt.data);
            //printf("LoopReadFrame: pkt.size=%d \n", pkt.size);

            if (pkt.stream_index == demux->video_index)
	        {
                int ret2 = DecodeVideo(pRead, pkt, outbuf);
                now_time = get_sys_time();
                if(ret2 > 0)
                {
                    video_frame_num++;
                }
                int64_t difftime = (now_time - start_time);
                //printf("LoopReadFrame: difftime=%lld \n", difftime);
                float frame_rate = (float)video_frame_num / ((float)difftime / 1000.0);
                int64_t cap_time = start_time + (int64_t)(video_frame_num * video_interval);
                int wait_time = (int)(cap_time - now_time);
                wait_time = wait_time > 0 ? wait_time : 0;
                //printf("LoopReadFrame: wait_time=%d \n", wait_time);
                usleep(wait_time * 1000);
	        }
#if 1
	        else if(true)
	        {
	            int ret2 = DecodeAudio(pRead, pkt, outbuf);
	            now_time = get_sys_time();
	            if(ret2 > 0)
                {
                    audio_frame_num += ret2;
                }
	            int64_t difftime = (now_time - start_time);
	            //printf("LoopReadFrame: difftime=%lld \n", difftime);
                float frame_rate = (float)audio_frame_num / ((float)difftime / 1000.0);
                int64_t cap_time = start_time + (int64_t)(audio_frame_num * audio_interval);
                int wait_time = (int)(cap_time - now_time);
                wait_time = wait_time > 0 ? wait_time : 0;
                //printf("LoopReadFrame: wait_time=%d \n", wait_time);
                usleep(wait_time * 1000);
	        }
#endif
        }
        else{
            printf("LoopReadFrame: av_seek_frame \n");
            ret = av_seek_frame(demux->fmtctx, demux->video_index, 0, AVSEEK_FLAG_BACKWARD);
            if (ret < 0) {
                fprintf(stderr, "LoopReadFrame: %s\n", av_err2str(ret));
            }
        }
        av_packet_unref(&pkt);
    }
    printf("LoopReadFrame: over \n");
    return ret;
}
int ReadFrame4Rtp(AVStreamObj *pRead, unsigned char *buf, int *len, int *isKeyFrame, int *width, int *height)
{
	int ret = -1;
	AVDeMuxer *demux = (AVDeMuxer *)pRead->demux;
	static const uint8_t kRtpHeaderSize = 12;
	static const uint8_t kRtpExtendSize = (sizeof(EXTEND_HEADER) >> 2) - 1;
	static const uint8_t kRtpGenSize = 1;
	static const uint8_t kAudio = 1;
	static const uint8_t kVideo = 2;
	AVPacket pkt;
	av_init_packet(&pkt);
	pthread_mutex_lock(&demux->mutex);
	if (!demux->fmtctx)
	{
		ret = ReadInit(pRead);
		if (ret < 0)
		{
			pthread_mutex_unlock(&demux->mutex);
			return ret;
		}
	}
	ret = av_read_frame(demux->fmtctx, &pkt);
	if (ret < 0)
	{
		printf("read frame error \n");
		pthread_mutex_unlock(&demux->mutex);
		return ret;
	}
	if (pkt.stream_index == demux->video_index)
	{
		AVCodecContext *c = demux->vpCodecCtx;
		//printf("video stream \n");
		AVStream* istream = demux->fmtctx->streams[demux->video_index];
		int extlen = (kRtpExtendSize + 1) << 2;
		*width = c->width;
		*height = c->height;
		int *frame_rate = (int *)buf;// mst->params.frame_rate;
		*frame_rate = c->framerate.num / c->framerate.den;
		//*frame_rate = c->time_base.num / c->time_base.den;
		int time_step = 90000 / *frame_rate;
		uint8_t *payload_type = (uint8_t *)&buf[4];//rsv for extend
		*payload_type = 96;
		//pkt.pts
		int InsetBytes = kRtpHeaderSize + kRtpGenSize + kRtpHeaderSize + extlen;
		memcpy(&buf[InsetBytes], pkt.data, pkt.size);
		*len = pkt.size + InsetBytes;
		*isKeyFrame = pkt.flags;
		ret = kVideo;//enum AVType{audio = 0,video };
	}
	else
	{
	    AVDeMuxer *audio = (AVDeMuxer *)pRead->demux;
		AVCodecContext *c = audio->apCodecCtx;
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
			pthread_mutex_unlock(&demux->mutex);
			return -2;
		}
		if (got_frame)
		{
			int data_size = av_get_bytes_per_sample(c->sample_fmt);
			size_t unpadded_linesize = audio->frame->nb_samples * av_get_bytes_per_sample(frame->format);
			audio->tmp_frame->data[0] = (uint8_t *)&audio->buf[audio->pos];// outBuf[0][offset];
			if (audio->resample == NULL)
			{
				audio->resample = (FFResample *)ResampleCreate(audio->resample);
			}
			frame->extended_data;
			ResampleParams((void *)audio->resample, frame->data, frame->format, frame->channels, frame->nb_samples, frame->sample_rate, 0);
			///ResampleParams((void *)audio->resample, audio->tmp_frame->data, AV_SAMPLE_FMT_S16, frame->channels, frame->nb_samples, frame->sample_rate, 1);
			int dst_nb_samples = (48000 * frame->nb_samples) / frame->sample_rate;// 44100;
			ResampleParams((void *)audio->resample, audio->tmp_frame->data, AV_SAMPLE_FMT_S16, frame->channels, dst_nb_samples, 48000, 1);
			ResampleInit((void *)audio->resample);
			ret = ResampleFrame((void *)audio->resample);
			ret = data_size * ret;
			audio->pos += ret;
			if (audio->pos >= 8192)
			{
#if 0
				pRead->debug_flag = 1;
				//static FILE* fp = NULL;
				if (!pRead->ofp  && pRead->debug_flag)
				{
#ifdef _WIN32
					pRead->ofp = fopen("e://works//test//pRead-d.pcm", "wb");
#elif defined(__ANDROID__)
					pRead->ofp = fopen("/sdcard/pRead-d.pcm", "wb");
#elif defined(IOS)
					char dirName[256] = "audio-d.pcm";
					char *pName = MAKE_FILE_NAME(dirName);//makePreferencesFilename("webrtc_log.txt");
					pRead->ofp = fopen(pName, "wb");
					free(pName);
#else
					pRead->ofp = fopen("audio-d.pcm", "wb");
#endif
				}
				if (pRead->ofp)
				{
					int wsize = fwrite(pRead->buf, 1, 8192, pRead->ofp);

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
			*sample_rate = c->sample_rate;
			//pkt.pts
			//memcpy(&buf[kRtpHeaderSize], pkt.data, pkt.size);
			//printf("pkt.size= %d \n", pkt.size);
			//*len = pkt.size + kRtpHeaderSize;
			*len = ret;
			*isKeyFrame = 0;// pkt.flags;
		}
	}
	pthread_mutex_unlock(&demux->mutex);
	av_free_packet(&pkt);
	return ret;
}

#if 1
int AVStreamRelease(AVStreamObj *stream)
{
	int ret = 0;
#if 0
	if (stream->scale)
	{
		///ffmpeg_close((void *)stream->scale);
		stream->scale = NULL;
	}
	if (stream->demux->resample)
	{
		ResampleClose((void *)stream->demux->resample);
		stream->demux->resample = NULL;
	}

	if(stream->st)
	{
		avcodec_close(stream->st->codec);
		stream->st = NULL;
	}

	if(stream->codecCtx)
	{
		avcodec_close(stream->codecCtx);
		stream->codecCtx = NULL;
	}
	if(stream->codec)
	{
		//avcodec_close(stream->codec);
		stream->codec = NULL;
	}
#endif
    AVMuxer *mux = (AVMuxer *)stream->mux;
	if (mux)
	{
	    if(stream->is_file && mux->AVWriteFlag)
	    {
	        WriteStop(stream);
	    }
		if (mux->opt)
		{
			av_dict_free(&mux->opt);
			mux->opt = NULL;
		}
		//mux->AVWriteFlag &= ~(1 << stream->WriteIsCpy);
		mux->AVWriteFlag = 0;
		if (mux->ofmt)
		{
			if (!mux->AVWriteFlag)
			{
				mux->WriteHead = 0;

				pthread_mutex_lock(&mux->mutex);
#if 0
				if (stream->st)
				{
					avcodec_close(stream->st->codec);
					stream->st = NULL;
				}

				if (stream->codecCtx)
				{
					avcodec_close(stream->codecCtx);
					stream->codecCtx = NULL;
				}
				if (stream->codec)
				{
					//avcodec_close(stream->codec);
					stream->codec = NULL;
				}
#endif
				if (mux->avio)
				{
					mux->avio = NULL;
				}
				if (mux->fmtctx)
				{
					if (mux->fmtctx->pb)
					{
						avio_close(mux->fmtctx->pb);
						mux->fmtctx->pb = NULL;
					}
					//mux->callback(mux->audioChanId, mux->file[i].filename, get_timelen(mux), 0);
					if (mux->fmtctx)
					{
						avformat_free_context(mux->fmtctx);
						mux->fmtctx = NULL;
					}
					mux->ofmt = NULL;
				}
				pthread_mutex_unlock(&mux->mutex);
			}
			else
			{
				mux = NULL;
			}
		}
		if (mux && !mux->AVWriteFlag)
		{
		    pthread_mutex_destroy(&mux->mutex);
			free(mux);
			stream->mux = NULL;
		}
	}//mux
	//
	AVDeMuxer *demux = (AVMuxer *)stream->demux;
	if (demux)
	{
		if (demux->opt)
		{
			av_dict_free(&demux->opt);
			demux->opt = NULL;
		}
		if(demux->frame)
	    {
	    	av_free(demux->frame);
	    	demux->frame = NULL;
	    }
	    if (demux->tmp_frame)
	    {
	    	av_free(demux->tmp_frame);
	    	demux->tmp_frame = NULL;
	    }
		if (demux->ifmt)
		{
			if (demux->fmtctx)
		    {
		    	avformat_close_input(&demux->fmtctx);
		    	demux->fmtctx = NULL;
		    }
		    demux->ifmt = NULL;
		}

		if (demux)
		{
		    pthread_mutex_destroy(&demux->mutex);
			free(demux);
			stream->demux = NULL;
		}
	}
#if 0
	//stream->pkt = ;
	if(stream->frame)
	{
		av_free(stream->frame);
		stream->frame = NULL;
	}
	if (stream->tmp_frame)
	{
		av_free(stream->tmp_frame);
		stream->tmp_frame = NULL;
	}
	if (stream->alloc_pic.data[0])
	{
		avpicture_free(&stream->alloc_pic);
		//av_free(stream->alloc_pic.data[0]);
		/*av_free(stream->alloc_pic.data[1]);
		av_free(stream->alloc_pic.data[2]);*/
		//stream->pic = NULL;
		//printf("delete pic");
	}
	if(stream->bsfc)
	{
		av_bitstream_filter_close(stream->bsfc);
		stream->bsfc = NULL;
	}
#endif
	//pthread_mutex_destroy(&stream->mutex);
	///stream->stream_idx = -1;
	//stream->id = -1;
	///stream->WriteIsCpy = -1;
	///stream->ReadIsCpy = -1;
	return ret;
}
#endif
//===============================================================
int video_add_stream(CodecObj *obj, AVStreamObj *pWrite)
{
    int ret = 0;
    AVFormatContext **oc = &pWrite->mux->fmtctx;
	AVOutputFormat *fmt = (*oc)->oformat;
	AVCodec *codec = obj->c->codec;//obj->codec;//
	printf("video_add_stream: codec= %x \n", codec);
	printf("video_add_stream: obj->c->width= %d \n", obj->c->width);
	printf("video_add_stream: obj->c->height= %d \n", obj->c->height);
	printf("video_add_stream: obj->c->time_base.den= %d \n", obj->c->time_base.den);
	printf("video_add_stream: codec->name= %s \n", codec->name);

	pWrite->mux->vst = avformat_new_stream(*oc, codec);
	AVCodecContext *c = pWrite->mux->vst->codec;
	printf("video_add_stream: c->width= %d \n", c->width);
	printf("video_add_stream: c->height= %d \n", c->height);
	printf("video_add_stream: c->codec->name= %s \n", c->codec->name);
	//c->width = 1920;//test
	//c->height = 1080;//test
	int vstreamId = (*oc)->nb_streams - 1;
	printf("video_add_stream: pWrite->mux->vst->id= %d \n", pWrite->mux->vst->id);
	printf("video_add_stream: vstreamId= %d \n", vstreamId);
	pWrite->mux->vst->id = vstreamId;

	printf("video_add_stream: pWrite->mux->vst= %x \n", pWrite->mux->vst);
	printf("video_add_stream: vstreamId= %d \n", vstreamId);
	pWrite->mux->vst->time_base = (AVRational){ 1, 25 };//obj->c->time_base;
	ret = avcodec_parameters_from_context(pWrite->mux->vst->codecpar, obj->c);
	printf("video_add_stream: c->width= %d \n", c->width);
	printf("video_add_stream: c->height= %d \n", c->height);
	printf("video_add_stream: c->codec->name= %s \n", c->codec->name);
	printf("video_add_stream: ret= %d \n", ret);
	printf("video_add_stream: (*oc)->streams[vstreamId]->time_base.den= %d \n", (*oc)->streams[vstreamId]->time_base.den);

	return ret;
}
void set_avstream2video(char *handle, char *handle2)
{
    long long *testp = (long long *)handle;
    CodecObj *obj = (CodecObj *)testp[0];

    testp = (long long *)handle2;
    AVStreamObj *stream = (AVStreamObj *)testp[0];

    if(obj && stream)
    {
        AVMuxer *mux = (AVMuxer *)stream->mux;
        printf("set_avstream2video: mux=%x \n", mux);
        //pthread_mutex_lock(&mux->mutex);
        //mux->AVWriteFlag |= 1;
        //video_add_stream(obj, stream);
        //pthread_mutex_unlock(&mux->mutex);
        obj->stream = stream;
        stream->video_codec_handle = handle;
    }
}
int audio_add_stream(AudioCodecObj *obj, AVStreamObj *pWrite)
{
    int ret = 0;
    AVFormatContext **oc = &pWrite->mux->fmtctx;
	AVOutputFormat *fmt = (*oc)->oformat;
	AVCodec *codec = obj->c->codec;//obj->codec;//
    pWrite->mux->ast = avformat_new_stream(*oc, codec);
    AVCodecContext *c = pWrite->mux->ast->codec;
    printf("audio_add_stream: codec->name= %s \n", codec->name);
    printf("audio_add_stream: obj->c->sample_rate= %d \n", obj->c->sample_rate);
    //c->sample_rate = 48000;//test
	int astreamId = (*oc)->nb_streams - 1;
	pWrite->mux->ast->id = astreamId;
	printf("audio_add_stream: codec= %x \n", codec);
	printf("audio_add_stream: pWrite->mux->ast= %x \n", pWrite->mux->ast);
	printf("audio_add_stream: (*oc)->streams[astreamId]= %x \n", (*oc)->streams[astreamId]);
	printf("audio_add_stream: astreamId= %d \n", astreamId);
	pWrite->mux->ast->time_base = (AVRational){1, obj->c->sample_rate};//{ 1, obj->c->sample_rate };
	ret = avcodec_parameters_from_context(pWrite->mux->ast->codecpar, obj->c);
	printf("audio_add_stream: c->sample_rate= %d \n", c->sample_rate);
	printf("audio_add_stream: c->codec->name= %s \n", c->codec->name);
	printf("audio_add_stream: ret= %d \n", ret);
	printf("audio_add_stream: (*oc)->streams[astreamId]->time_base.den= %d \n", (*oc)->streams[astreamId]->time_base.den);
	printf("audio_add_stream: pWrite->mux->ast->time_base.den= %d \n", pWrite->mux->ast->time_base.den);
	return ret;
}
void set_avstream2audio(char *handle, char *handle2)
{
    long long *testp = (long long *)handle;
    AudioCodecObj *obj = (AudioCodecObj *)testp[0];

    testp = (long long *)handle2;
    AVStreamObj *stream = (AVStreamObj *)testp[0];

    if(obj && stream)
    {
        AVMuxer *mux = (AVMuxer *)stream->mux;
        //pthread_mutex_lock(&mux->mutex);
        //mux->AVWriteFlag |= 2;
        //audio_add_stream(obj, stream);
        //pthread_mutex_unlock(&mux->mutex);
        obj->stream = stream;
        stream->audio_codec_handle = handle;
    }
}

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
int api_avstream_status(char *handle, int status)
{
    int ret = 0;
    if(handle)
    {
        long long *testp = (long long *)handle;
        AVStreamObj *obj = (AVStreamObj *)testp[0];
        //
        printf("api_avstream_status: obj= %x \n", obj);
        if(obj)
        {
            AVDeMuxer *demux = (AVDeMuxer *)obj->demux;
            if(demux)
            {
                pthread_mutex_lock(&demux->mutex);
                demux->status = status;
                pthread_mutex_unlock(&demux->mutex);
                printf("api_avstream_status: status= %d \n", status);
            }
        }
        printf("api_avstream_status: ok \n");
    }
    return ret;
}
HCSVC_API
int api_avstream_close(char *handle)
{
    long long ret = 0;
    if(handle)
    {
        long long *testp = (long long *)handle;
        AVStreamObj *obj = (AVStreamObj *)testp[0];
        //
        if(obj)
        {
            AVStreamRelease(obj);
            free(obj);
        }
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

    GetvalueStr(obj->json, "infile", obj->infile);
    GetvalueStr(obj->json, "outfile", obj->outfile);
    printf("api_avstream_init: obj->infile=%s \n", obj->infile);
    printf("api_avstream_init: obj->outfile=%s \n", obj->outfile);

    obj->width = GetvalueInt(obj->json, "width");
    obj->height = GetvalueInt(obj->json, "height");
#if 0
    obj->print = GetvalueInt(obj->json, "print");

    obj->codec_type = GetvalueInt(obj->json, "codec_type");
    obj->out_nb_samples = GetvalueInt(obj->json, "out_nb_samples");
    obj->out_channels = GetvalueInt(obj->json, "out_channels");
    obj->out_sample_rate = GetvalueInt(obj->json, "out_sample_rate");

#endif
    if(!obj->mux && strcmp(obj->outfile, ""))
    {
        obj->mux = calloc(1, sizeof(AVMuxer));
    }
    if(!obj->demux && strcmp(obj->infile, ""))
    {
        obj->demux = calloc(1, sizeof(AVDeMuxer));
    }
    if(obj->mux)
    {
        pthread_mutex_init(&obj->mux->mutex,NULL);
        ret = WriteInit(obj);
        if(ret < 0)
        {
            return ret;
        }
    }
    printf("api_avstream_init: obj->demux=%x \n", obj->demux);
    if(obj->demux)
    {
        pthread_mutex_init(&obj->demux->mutex,NULL);
        printf("api_avstream_init: ReadInit \n");
        ret = ReadInit(obj);
        printf("api_avstream_init: ReadInit: ret=%d \n", ret);
        if(ret < 0)
        {
            return ret;
        }
    }
    printf("api_avstream_init: ok \n");
    return ret;
}
HCSVC_API
void api_avstream_set_video_handle(char *handle, char *video_codec_handle)
{
    long long *testp = (long long *)handle;
    AVStreamObj *obj = (AVStreamObj *)testp[0];
    obj->video_codec_handle = video_codec_handle;
}
HCSVC_API
void api_avstream_set_audio_handle(char *handle, char *audio_codec_handle)
{
    long long *testp = (long long *)handle;
    AVStreamObj *obj = (AVStreamObj *)testp[0];
    obj->audio_codec_handle = audio_codec_handle;
}
HCSVC_API
int api_audio_codec_one_frame_stream(char *handle, char *data, char *param, char *outbuf, char *outparam[])
{
    long long *testp = (long long *)handle;
    AVStreamObj *stream = (AVStreamObj *)testp[0];
    cJSON *json = mystr2json(param);
    char *key = "is_audio";
	int ivalue = 0;
	json = renewJsonInt(json, key, ivalue);
	char *param2 = cJSON_Print(json);
    AVWriteFrame(stream, data, param2, outbuf, outparam);
    free(param2);
}
HCSVC_API
int api_video_encode_one_frame_stream(char *handle, char *data, char *param, char *outbuf, char *outparam[])
{
    long long *testp = (long long *)handle;
    AVStreamObj *stream = (AVStreamObj *)testp[0];
    cJSON *json = mystr2json(param);
    char *key = "is_video";
	int ivalue = 1;
	json = renewJsonInt(json, key, ivalue);
	char *param2 = cJSON_Print(json);
    AVWriteFrame(stream, data, param2, outbuf, outparam);
    free(param2);
}
HCSVC_API
int api_avstream_write_tail(char *handle)
{
    int ret = -1;
    long long *testp = (long long *)handle;
    AVStreamObj *stream = (AVStreamObj *)testp[0];
    if(stream->is_file)
    {
        AVMuxer *mux = (AVMuxer *)stream->mux;

        pthread_mutex_lock(&mux->mutex);
        ret = WriteTail(mux->fmtctx);
        pthread_mutex_unlock(&mux->mutex);
    }
    else{
        ret = 0;
    }
    return ret;
}
HCSVC_API
int api_avstream_loopread(char *handle)
{
    int ret = 0;
    if(handle)
    {
        long long *testp = (long long *)handle;
        AVStreamObj *obj = (AVStreamObj *)testp[0];
        //
        printf("api_avstream_loopread: obj= %x \n", obj);
        if(obj)
        {
            ret = LoopReadFrame(obj);
        }
    }
    return ret;
}
HCSVC_API
void * api_avstream_pop_data(char *handle, int avtype)
{
    void *ret = 0;
    if(handle)
    {
        long long *testp = (long long *)handle;
        AVStreamObj *obj = (AVStreamObj *)testp[0];
        //printf("api_avstream_pop_data: obj= %x, avtype=%d \n", obj, avtype);
        //
        if(obj)
        {
            AVDeMuxer *demux = (AVDeMuxer *)obj->demux;
            ret = av_read_pop_data(demux, avtype);
            //printf("api_avstream_pop_data: ret= %x, avtype=%d \n", ret, avtype);
        }
    }
    return ret;
}
//==========================================================================

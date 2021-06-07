#include "inc.h"
#include "udpbase.h"

extern void set_avstream2video(char *handle, char *handle2);
extern void set_avstream2audio(char *handle, char *handle2);
extern int is_static_frame(CallCodecVideo *obj, char *outbuf);
extern char * get_cpx(int w, int h);

static void StopVideo(McuObj *obj)
{
    pthread_mutex_lock(&obj->status_lock);
    if(obj->video_status > 0)
    {
        obj->video_status = 0;
    }
    pthread_mutex_unlock(&obj->status_lock);
}
static void StopAudio(McuObj *obj)
{
    pthread_mutex_lock(&obj->status_lock);
    if(obj->audio_status > 0)
    {
        obj->audio_status = 0;
    }
    pthread_mutex_unlock(&obj->status_lock);
}
#if 0
static void WaitRender(McuObj *obj)
{
    CallRender *render = NULL;
    do{
        pthread_mutex_lock(&obj->status_lock);
        render = obj->render;
        pthread_mutex_unlock(&obj->status_lock);
        if(!render)
        {
            usleep(100000);//100ms
        }
    }while(!render);
}
static void WaitPlayer(McuObj *obj)
{
    CallPlayer *player = NULL;
    do{
        pthread_mutex_lock(&obj->status_lock);
        player = obj->player;
        pthread_mutex_unlock(&obj->status_lock);
        if(!player)
        {
            usleep(100000);//100ms
        }
    }while(!player);
}

int mcu_set_render(SocketObj *sock, CallRender *render)
{
    int ret = 0;
    pthread_mutex_lock(&sock->status_lock);
    SessionObj * session = (SessionObj *)sock->session;
    McuObj *obj = session->mcu;
    obj->render = render;
    pthread_mutex_unlock(&sock->status_lock);
    return ret;
}
int mcu_set_player(SocketObj *sock, CallPlayer *player)
{
    int ret = 0;
    pthread_mutex_lock(&sock->status_lock);
    SessionObj * session = (SessionObj *)sock->session;
    McuObj *obj = session->mcu;
    obj->player = player;
    pthread_mutex_unlock(&sock->status_lock);
    return ret;
}
#endif

void McuPushData(McuObj *obj, uint8_t *data, int size, int64_t now_time, int avtype)
{
    pthread_mutex_lock(&obj->lock);

    McuNode *head,*pnew, *q;
    if(!obj->datahead[avtype - 1])
    {
        //head不存数据,head->next指向第一个数据,head->num保存总索引（也可以设计将第一个节点存入head）
        //注意链表元的2个组成部分，元和next指针，元控制的是整个链表元的地址，next指针则寄生在元下；
        //printf("PushData: create head: obj->data_list=%x \n", obj->data_list);
        head = (McuNode *)calloc(1, sizeof(McuNode));  //创建头节点。
        head->num = 0;
        head->next = NULL;  //头节点指针域置NULL
        head->tail = head;  // 开始时尾指针指向头节点；此时head->tail->next与head->next是同一地址
        obj->datahead[avtype - 1] = (void *)head;
    }
    head = (FrameNode *)obj->datahead[avtype - 1];
    if(head->num > MAX_DELAY_FRAME_NUM)
    {
        q = head->next;
        head->next = head->next->next;
        head->num--;
        if(head->next == NULL)
        {
            head->next = NULL;  //头节点指针域置NULL
            head->tail = head;
        }
        free(q->data);
        free(q);
        printf("McuPushData: skip head->num=%d \n", head->num);
    }
    {
        pnew = (McuNode *)calloc(1, sizeof(McuNode));  //创建新节点
        pnew->data = (uint8_t *)data;
        pnew->size = size;
        pnew->idx = head->idx;
        pnew->frame_timestamp = now_time;
        pnew->next = NULL;   //新节点指针域置NULL
        head->tail->next = pnew;  //新节点插入到表尾；首个节点时，则意味着ead->next也指向了pnew
        head->tail = pnew;   //为指针指向当前的尾节点;元地址迁移，元的原先的next脱离了元，新地址的next成为元的next指针；
        head->idx++;
        head->num++;
    }
    pthread_mutex_unlock(&obj->lock);
}

void * McuPopData(McuObj *obj, int avtype)
{
    void *ret = NULL;

    pthread_mutex_lock(&obj->lock);
    McuNode *head, *q;
    head = (McuNode *)obj->datahead[avtype - 1];
    //printf("McuPopData: head=%x \n", head);
    //printf("McuPopData: avtype=%d \n", avtype);
    if(head)
    {
        //printf("McuPopData: head->num=%d \n", head->num);
        //printf("McuPopData: avtype=%d \n", avtype);
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
    pthread_mutex_unlock(&obj->lock);

    return ret;
}
static int video_encode_one_frame(CallCodecVideo *obj, uint8_t *frame_data, int insize)
{
    int ret = 0;
    if(obj)
    {
        CodecInfoObj *config = (CodecInfoObj *)&obj->codecInfo;
        if(!obj->outfp && false)
        {
            const char *filename = "/home/gxh/works/gxhshare/test.h264";
            obj->outfp = fopen(filename, "wb");
        }
        int data_size = insize;
        //int *p_pkt_size = NULL;
        //uint8_t *p_pkt_data = frame_data;
        //int p_pkt_num = 0;
        if(!config->lossless)
        {
            int frame_idx = obj->frame_idx;
            int gop_size = config->gop_size;
            //int res_num = codecInfo->res_num;
            int refs = config->refs;
            int max_refs = config->max_refs;
            int ref_idx = 0;
            int max_b_frames = config->max_b_frames;
#if 0
            int is_static = is_static_frame(obj, frame_data);
            if(is_static)
            {
                obj->json = api_renew_json_int(obj->json, "skip_frame", 1);
            }
#endif
            obj->pict_type = 0;
            if (frame_idx % gop_size)
            {
                int I = (frame_idx % gop_size);
                if((max_b_frames == 0) && (refs != 1))
                {
                    int j = (I - 1) % (refs);
                    //I P0 P1 P2 P0 P1 P2 (ref=3)
                    ref_idx = j + 1;
                    if(I >= ((int)(gop_size / max_refs) * max_refs))
                    {
                        ref_idx = 1;
                    }else if(((ref_idx & 1) == 0) &&
                             (ref_idx != refs) &&
                             (refs > 2))
                    {
                        ref_idx = 1;
                    }
                    else if(ref_idx == (refs - 1) && (refs > 4))
                    {
                        ref_idx = 1;
                    }
                }
                else {
                    int j = (I - 1) % (max_b_frames + 1);
                    //I P0 P1 P2 P0 P1 P2 (ref=3)
                    ref_idx = j + 1;
                    if(ref_idx == (max_b_frames + 1))
                    {
                        obj->pict_type = 2;//P Frame
                    }else{
                        obj->pict_type = 3;//B Frame
                    }

                    if(I == (gop_size - 1))
                    {
                        obj->pict_type = 2;//P Frame
                    }
                    //printf("gxh:HCSVCEncoderImpl::Encode: encoder->pict_type= %d \n", encoder->pict_type);
                }
            }
            else{
                ref_idx = 0;
                //encoder->pict_type = 1;//test
                if(frame_idx > 0)
                {
                    int refresh_idr = 0;
                    obj->json = api_renew_json_int(obj->json, "refresh_idr", refresh_idr);
#if 1
                    int large_gop_size = 0;
                    if(large_gop_size && true)//test
                    {
                        obj->pict_type = 2;
                        obj->json = api_renew_json_int(obj->json, "pict_type", obj->pict_type);
                        //
                        config->refs = 1;
                        //config->qmin = 30;
                        //obj->json = api_renew_json_int(obj->json , "qmin", config->qmin);
                        //obj->json = api_renew_json_int(obj->json , "qmax", config->qmax);
                        //obj->json = api_renew_json_int(obj->json , "qp", 26);
                        obj->json = api_renew_json_int(obj->json , "refs", config->refs);
                    }
                    else{
                        obj->pict_type = 1;
                        obj->json = api_renew_json_int(obj->json, "pict_type", obj->pict_type);
                        //
                        config->refs = 16;
                        obj->json = api_renew_json_int(obj->json , "refs", config->refs);
                    }
#endif
                }

            }
            //printf("video_encode_one_frame: ref_idx=%d \n", ref_idx);
            obj->json = api_renew_json_int(obj->json, "frame_idx", frame_idx);
            //self.param.update({"seqnum": self.seqnum})
            if ((config->refs & 1) == 0)
            {
                //self.param.update({"ref_idx": ref_idx})
                obj->json = api_renew_json_int(obj->json, "ref_idx", ref_idx);
            }
            else {
                obj->json = api_renew_json_int(obj->json, "pict_type", obj->pict_type);
            }
            int this_mtu_size = config->mtu_size;
            if(!ref_idx || (ref_idx == refs))
            {
                this_mtu_size = MTU_SIZE;
            }
            char ctmp[32] = "";
            sprintf(ctmp, "%d", this_mtu_size);
            obj->json = api_renew_json_str(obj->json, "slice-max-size", ctmp);
            obj->json = api_renew_json_int(obj->json, "mtu_size", this_mtu_size);
            int thread_type = 1;
            int thread_count = 1;
            if(!ref_idx)
            {
                thread_type = 2;
                thread_count = 4;
            }
            obj->json = api_renew_json_int(obj->json, "thread_type", thread_type);//FF_THREAD_FRAME: 1 FF_THREAD_SLICE: 2
            obj->json = api_renew_json_int(obj->json, "thread_count", thread_count);
            //
            if(obj->bitrate > 0)
            {
                obj->json = api_renew_json_int(obj->json, "up_bitrate", obj->bitrate);
            }
            //
            char* jsonstr = api_json2str(obj->json);
            //printf("video_encode_one_frame: jsonstr=%s \n", jsonstr);
            //encode frame
            ret = api_video_encode_one_frame(   obj->handle,
                                                (char *)frame_data,
                                                jsonstr,
                                                (char *)obj->outbuffer,
                                                obj->outparam);
            api_json2str_free(jsonstr);
            //printf("video_encode_one_frame: ret=%d \n", ret);
            obj->json = api_delete_item(obj->json, "pict_type");
            obj->json = api_delete_item(obj->json, "skip_frame");
            if(ret > 0)
            {
                if ( obj->outfp != NULL)
                {
                    //printf("video_encode_one_frame: ret=%d \n", ret);
                    fwrite(obj->outbuffer, 1, ret, obj->outfp);
                }

            }
            else{
                printf("video_encode_one_frame: ret=%d \n", ret);
                return ret;
            }
        }
        else{
            //av_free(frame_data);
        }
        obj->frame_idx++;
    }

    return ret;
}
static int audio_encode_one_frame(CallCodecAudio *obj, uint8_t *frame_data, int insize)
{
    int ret = 0;
    if(obj)
    {
        CodecInfoObj *config = (CodecInfoObj *)&obj->codecInfo;
        if(!obj->outfp && false)
        {
            const char *filename = "/home/gxh/works/gxhshare/test.aac";
            obj->outfp = fopen(filename, "wb");
        }
        int data_size = insize;
        //int *p_pkt_size = NULL;
        //uint8_t *p_pkt_data = frame_data;
        //int p_pkt_num = 0;
        if(!config->lossless)
        {
            //printf("audio_encode_one_frame: ref_idx=%d \n", ref_idx);

            //self.param.update({"seqnum": self.seqnum})

            int this_mtu_size = config->mtu_size;
            char ctmp[32] = "";
            sprintf(ctmp, "%d", this_mtu_size);
            obj->json = api_renew_json_int(obj->json, "mtu_size", this_mtu_size);
            obj->json = api_renew_json_int(obj->json, "insize", insize);
            if(obj->bitrate > 0)
            {
                //obj->json = api_renew_json_int(obj->json, "up_bitrate", obj->bitrate);
            }
            int is_skip = 0;///GetNetResult(obj, ref_idx);
            //
            char* jsonstr = api_json2str(obj->json);
            //printf("audio_encode_one_frame: jsonstr=%s \n", jsonstr);
            //encode frame
            ret = api_audio_codec_one_frame(    obj->handle,
                                                (char *)frame_data,
                                                jsonstr,
                                                (char *)obj->outbuffer,
                                                obj->outparam);
            api_json2str_free(jsonstr);
            //printf("audio_encode_one_frame: ret=%d \n", ret);
            if(ret > 0)
            {
                if ( obj->outfp != NULL)
                {
                    //printf("audio_encode_one_frame: ret=%d \n", ret);
                    fwrite(obj->outbuffer, 1, ret, obj->outfp);
                }
            }
            else{
                printf("audio_encode_one_frame: ret=%d \n", ret);
                return ret;
            }
        }
        obj->frame_idx++;
    }

    return ret;
}
int audio_read_run(McuObj *obj)
{
    int ret = 0;
    pthread_mutex_lock(&obj->status_lock);
    obj->audio_status = 1;
    int status = obj->audio_status;
    pthread_mutex_unlock(&obj->status_lock);
    //WaitPlayer(obj);
    int frame_num = 0;
    int64_t time0 = get_sys_time();
    obj->start_time = time0;
    while(status > 0)
    {
        int64_t now_time = get_sys_time();//api_get_time_stamp_ll();
        //
        //printf("audio_read_run: \n");
        McuNode *q = McuPopData(obj, kIsAudio);
        if(q)
        {
            //printf("audio_read_run: q->size=%d \n", q->size);
            ret = audio_encode_one_frame(obj->audio, q->data, q->size);
            //printf("audio_read_run: ret=%d \n", ret);
            //free(q->data);
            av_free(q->data);
            free(q);
            frame_num++;
        }
        else
        {
            usleep(10000);//10ms
        }
        int difftime = (int)(now_time - time0);
        if(difftime > 2000)
        {
            float framerate = (float)((frame_num * 1000.0) / difftime);
            time0 = now_time;
            frame_num = 0;
            printf("audio_read_run: framerate=%1.1f (fps) \n", framerate);
        }
        //
        pthread_mutex_lock(&obj->status_lock);
        status = obj->audio_status;// & obj->send_status;
        pthread_mutex_unlock(&obj->status_lock);
    }
    //RenderStop(obj);
    printf("audio_read_run: over \n");
    char *p = malloc(32);
    strcpy(p,"audio_read_run over");
    pthread_exit((void*)p);

    return ret;
}
int video_read_run(McuObj *obj)
{
    int ret = 0;
    pthread_mutex_lock(&obj->status_lock);
    obj->video_status = 1;
    int status = obj->video_status;
    pthread_mutex_unlock(&obj->status_lock);
    //WaitRender(obj);
    int frame_num = 0;
    int64_t time0 = get_sys_time();
    obj->start_time = time0;
    while(status > 0)
    {
        int64_t now_time = get_sys_time();
        //int difftime = (int)(now_time - obj->start_time);
        //
        //printf("video_read_run: \n");
        McuNode *q = McuPopData(obj, kIsVideo);
        if(q)
        {
            //printf("video_read_run: q->size=%d \n", q->size);
            ret = video_encode_one_frame(obj->video, q->data, q->size);
            //printf("video_read_run: ret=%d \n", ret);
            //free(q->data);
            av_free(q->data);
            free(q);
            frame_num++;
        }
        else
        {
            usleep(10000);//10ms
        }

        int difftime = (int)(now_time - time0);
        if(difftime > 2000)
        {
            float framerate = (float)((frame_num * 1000.0) / difftime);
            time0 = now_time;
            frame_num = 0;
            printf("video_read_run: framerate=%1.1f (fps) \n", framerate);
        }

        pthread_mutex_lock(&obj->status_lock);
        status = obj->video_status;
        pthread_mutex_unlock(&obj->status_lock);
    }
    printf("video_read_run: end \n");
    //CaptureStop(obj);
    //ExitRecv(obj);

    printf("video_read_run: over \n");
    return ret;
}
void VideoClose(CallCodecVideo *obj)
{
    if(obj)
    {
        if(obj->encoder)
        {
            free(obj->encoder);
            obj->encoder = NULL;
        }
        if(obj->outbuffer)
        {
            free(obj->outbuffer);
            obj->outbuffer = NULL;
        }
        if(obj->rtpbuffer)
        {
            free(obj->rtpbuffer);
            obj->rtpbuffer = NULL;
        }
        //if(obj->img_convert_ctx)
        //{
        //    sws_freeContext(obj->img_convert_ctx);
        //    obj->img_convert_ctx = NULL;
        //}
        api_video_encode_close(obj->handle);
#if 1
        if(obj->json)
        {
            api_json_free(obj->json);
            obj->json = NULL;
        }
        //for(int i = 0; i < MAX_SKIP_FRAME_NUM; i++)
        //{
        //    if(obj->last_frame[i])
        //    {
        //        int flag = !i;
        //        flag |= (i && (obj->last_frame[i] != obj->last_frame[i - 1]));
        //        if(flag)
        //        {
        //            av_free(obj->last_frame[i]);
        //            obj->last_frame[i] = NULL;
        //        }
        //    }
        //}
#endif
        pthread_mutex_destroy(&obj->lock);
        //free(obj);
    }
}
int VideoInit(McuObj *mcu)
{
    int ret = 0;
    CallCodecVideo *obj = (CallCodecVideo *)calloc(1, sizeof(CallCodecVideo));
    mcu->video = obj;
    char *params = mcu->params;//"{\"print\":0}";
    printf("VideoInit: params=%s \n", params);
    cJSON *json = (cJSON *)api_str2json(params);
    obj->json = json;
    CodecInfoObj *config = (CodecInfoObj *)&obj->codecInfo;
    config->lossless = LOSSLESS;
    pthread_mutex_init(&obj->lock,NULL);
    {
        int w = GetvalueInt(json, "width");
        int h = GetvalueInt(json, "height");
        obj->frame_size = (w * h * 3) >>1;
        char *cpx = get_cpx(w, h);

        int codec_mode = 0;
        char scodec[32] = "";
        GetvalueStr(json, "scodec", scodec);
        if(!strcmp(scodec, "qsv264"))
        {
            codec_mode = 1;
        }
        config->gop_size = 50;
        config->refs = GetvalueInt(json, "refs");
        config->max_refs = 16;
        config->max_b_frames = 0;
        config->frame_rate = GetvalueInt(json, "framerate");
        config->enable_fec = GetvalueInt(json, "enable_fec");
        config->loss_rate = GetvalueInt(json, "loss_rate");//0;//0.9;//0.05;
        config->code_rate = (1.0 - config->loss_rate);
        config->width = w;
        config->height = h;
        config->bit_rate = GetvalueInt(json, "bitrate");
        config->mtu_size = GetvalueInt(json, "mtu_size");
        config->qmin = 20;
        //config->qmin = 25;//30;//test
        config->qmax = 30;//40;
        config->osd = GetvalueInt(json, "osd_enable");
        config->adapt_cpu = 0;
        config->main_spatial_idx = 0;
        config->fec_level = config->fec_level;//2;
        obj->json = api_renew_json_int(obj->json , "codec_mode", codec_mode);//2
        obj->json = api_renew_json_int(obj->json , "seqnum", obj->seqnum);
        obj->json = api_renew_json_int(obj->json , "enable_fec", config->enable_fec);
        obj->json = api_renew_json_float(obj->json , "loss_rate", config->loss_rate);
        obj->json = api_renew_json_float(obj->json , "code_rate", config->code_rate);
        obj->json = api_renew_json_int(obj->json , "refresh_idr", 1);
        obj->json = api_renew_json_int(obj->json , "width", w);
        obj->json = api_renew_json_int(obj->json , "height", h);
        obj->json = api_renew_json_int(obj->json , "bit_rate", config->bit_rate);
        obj->json = api_renew_json_int(obj->json , "gop_size", config->gop_size);
        obj->json = api_renew_json_str(obj->json , "preset", cpx);
        obj->json = api_renew_json_str(obj->json , "tune", "zerolatency");
        char ctmp[64] = "";
        sprintf(ctmp, "%d", config->mtu_size);
        obj->json = api_renew_json_str(obj->json , "slice-max-size", ctmp);
        obj->json = api_renew_json_int(obj->json , "mtu_size", config->mtu_size);
        obj->json = api_renew_json_int(obj->json , "tff", 0);
        obj->json = api_renew_json_int(obj->json , "bff", 0);
        obj->json = api_renew_json_int(obj->json , "qmin", config->qmin);
        obj->json = api_renew_json_int(obj->json , "qmax", config->qmax);
        obj->json = api_renew_json_int(obj->json , "qp", 26);
        obj->json = api_renew_json_int(obj->json , "max_b_frames", config->max_b_frames);
        obj->json = api_renew_json_int(obj->json , "coder_type", 2);//1:cavlc/2:cabac
        obj->json = api_renew_json_int(obj->json , "refs", config->refs);//16
        obj->json = api_renew_json_int(obj->json , "frame_rate", config->frame_rate);
        obj->json = api_renew_json_int(obj->json , "qthread_type", 1);//FF_THREAD_FRAME: 1 FF_THREAD_SLICE: 2
        obj->json = api_renew_json_int(obj->json , "thread_count", 1);
        obj->json = api_renew_json_int(obj->json , "osd", config->osd);
        obj->json = api_renew_json_int(obj->json , "orgX", 0);
        obj->json = api_renew_json_int(obj->json , "orgY", 0);
        obj->json = api_renew_json_int(obj->json , "scale", 0);
        obj->json = api_renew_json_int(obj->json , "color", 1);
        obj->json = api_renew_json_int(obj->json , "adapt_cpu", config->adapt_cpu);
        obj->json = api_renew_json_int(obj->json , "print", 0);
        char *jsonStr = cJSON_Print(obj->json );
        printf("mcu: VideoInit: jsonStr=%s \n", jsonStr);
        api_video_encode_open(obj->handle, jsonStr);

        for(int l = 0; l < 4; l++)
        {
            obj->outparam[l] = NULL;
        }
        obj->outbuffer = malloc(obj->frame_size * sizeof(uint8_t));
        //obj->rtpbuffer = malloc(obj->frame_size * sizeof(uint8_t));

        obj->encoder = calloc(1, sizeof(Encoder));
        //obj->encoder->selfChanId = sessionInfo->chanId;
        obj->encoder->selfLossRate = 0;
        obj->encoder->selfMaxLossRate = 0;
        for(int i = 0; i < 4; i++)
        {
            obj->encoder->initBitRate[i] = config->bit_rate >> i;// = {0, 0, 0, 0};
        }
        //obj->skip_frame_idx = 0;
        //for(int i = 0; i < MAX_SKIP_FRAME_NUM; i++)
        //{
        //    obj->last_frame[i] = NULL;
        //}
        obj->encoder->bitRate = config->bit_rate;
        obj->encoder->codec = (void *)obj;
    }
    return ret;
}
int AudioInit(McuObj *mcu)
{
    int ret = 0;
    CallCodecAudio *obj = (CallCodecAudio *)calloc(1, sizeof(CallCodecAudio));
    mcu->audio = obj;

    pthread_mutex_init(&obj->lock,NULL);

    //obj->sessionInfo = sessionInfo;

    char *params = mcu->params;//"{\"print\":0}";
    obj->json = (cJSON *)api_str2json(params);
    //int w = DEFAULT_WIDTH;
    //int h = DEFAULT_HEIGHT;
    obj->frame_size = 8192;
    CodecInfoObj *config = (CodecInfoObj *)&obj->codecInfo;
    config->lossless = LOSSLESS;
    {
        config->enable_fec = 0;
        config->loss_rate = 0;//0.9;//0.05;
        config->code_rate = (1.0 - config->loss_rate);
        config->bit_rate = 24000;
        config->mtu_size = MTU_SIZE;
        config->adapt_cpu = 0;
        config->main_spatial_idx = 0;
        config->fec_level = 2;

        obj->json = api_renew_json_int(obj->json , "codec_mode", 0);//2
        obj->json = api_renew_json_int(obj->json , "seqnum", obj->seqnum);
        obj->json = api_renew_json_int(obj->json , "enable_fec", config->enable_fec);
        obj->json = api_renew_json_float(obj->json , "loss_rate", config->loss_rate);
        obj->json = api_renew_json_float(obj->json , "code_rate", config->code_rate);
        obj->json = api_renew_json_int(obj->json , "codec_type", 0);
        obj->json = api_renew_json_int(obj->json , "bit_rate", config->bit_rate);
        obj->json = api_renew_json_str(obj->json , "filename", "");
        obj->json = api_renew_json_str(obj->json , "sample_fmt", "AV_SAMPLE_FMT_S16");
        obj->json = api_renew_json_str(obj->json , "codec_id", "AV_CODEC_ID_AAC");
        obj->json = api_renew_json_str(obj->json , "out_channel_layout", "AV_CH_LAYOUT_STEREO");
        obj->json = api_renew_json_int(obj->json , "out_nb_samples", 2048);
        obj->json = api_renew_json_int(obj->json , "out_sample_rate", 48000);
        obj->json = api_renew_json_int(obj->json , "out_channels", 2);
        obj->json = api_renew_json_int(obj->json , "mtu_size", config->mtu_size);
        obj->json = api_renew_json_int(obj->json , "adapt_cpu", config->adapt_cpu);
        char *jsonStr = cJSON_Print(obj->json );
        printf("mcu: AudioInit: jsonStr=%s \n", jsonStr);
        api_audio_codec_init(obj->handle, jsonStr);

        for(int l = 0; l < 4; l++)
        {
            obj->outparam[l] = NULL;
        }
        obj->outbuffer = malloc(obj->frame_size * sizeof(uint8_t));
        //obj->rtpbuffer = malloc(obj->frame_size * sizeof(uint8_t));

        //obj->nettime = 0;

        obj->encoder = calloc(1, sizeof(Encoder));
        //obj->encoder->selfChanId = sessionInfo->chanId;
        obj->encoder->selfLossRate = 0;
        obj->encoder->selfMaxLossRate = 0;
        for(int i = 0; i < 4; i++)
        {
            obj->encoder->initBitRate[i] = config->bit_rate >> i;// = {0, 0, 0, 0};
        }
        obj->encoder->bitRate = config->bit_rate;
        obj->encoder->codec = (void *)obj;
    }

    return ret;
}

void AudioClose(CallCodecAudio *obj)
{
    if(obj)
    {
        if(obj->encoder)
        {
            free(obj->encoder);
            obj->encoder = NULL;
        }
        if(obj->outbuffer)
        {
            free(obj->outbuffer);
            obj->outbuffer = NULL;
        }
        if(obj->rtpbuffer)
        {
            free(obj->rtpbuffer);
            obj->rtpbuffer = NULL;
        }
        api_audio_codec_close(obj->handle);
        pthread_mutex_destroy(&obj->lock);
        //free(obj);
    }
}
int McuInit(McuObj *mcu)
{
    int ret = 0;
    printf("McuInit:  \n");
    ret = VideoInit(mcu);
    printf("McuInit: 1: ret=%d \n", ret);
    ret = AudioInit(mcu);
    printf("McuInit: 2: ret=%d \n", ret);
    cJSON *json = NULL;//mystr2json(params);
    ret = api_avstream_init(mcu->streamHnd, mcu->params);
    printf("McuInit: 3: ret=%d \n", ret);
    set_avstream2video(mcu->video->handle, mcu->streamHnd);
    set_avstream2audio(mcu->audio->handle, mcu->streamHnd);
    if(json)
    {
        deleteJson(json);
        json = NULL;
    }
    printf("McuInit: ok \n");
    return ret;
}
void * mcu_task(SocketObj *sock)
{
    int taskId = sock->id;
    printf("mcu_task: taskId=%d \n", taskId);
    pthread_mutex_init(&sock->lock,NULL);
    pthread_mutex_init(&sock->status_lock,NULL);
    SessionObj *session = (SessionObj *)calloc(1, sizeof(SessionObj));
    sock->session = (void *)session;
    McuObj *obj = (McuObj *)calloc(1, sizeof(McuObj));
    session->mcu = obj;
    obj->params = sock->params;
    printf("mcu_task: obj->params=%s \n", obj->params);
    McuInit(obj);
    pthread_t tid;
    if(pthread_create(&tid, NULL, audio_read_run, obj) < 0)
    {
        printf("mcu_task: Create audio_read_run failed!\n");
    }
    pthread_mutex_init(&obj->status_lock,NULL);
    video_read_run(obj);

    char *p0;
    if (pthread_join(tid, (void**)&p0))
    {
        printf("mcu_task: audio_read_run thread is not exit...\n");
    }
    else{
        printf("audio_read_run: p0=%s \n", p0);
        free(p0);
    }
    if(obj->video)
    {
        VideoClose(obj->video);
        free(obj->video);
    }
    if(obj->audio)
    {
        AudioClose(obj->audio);
        free(obj->audio);
    }
    pthread_mutex_lock(&obj->status_lock);
    if(sock->session)
    {
        SessionObj * session = (SessionObj *)sock->session;
        free(sock->session);
        sock->session = NULL;
    }
    pthread_mutex_unlock(&obj->status_lock);
    //pthread_mutex_destroy(&obj->lock);
    pthread_mutex_destroy(&obj->status_lock);
    free(obj);
    pthread_mutex_destroy(&sock->lock);
    pthread_mutex_destroy(&sock->status_lock);
    sock->status = -1;
    printf("mcu: over: taskId=%d \n", taskId);
    return 0;
}

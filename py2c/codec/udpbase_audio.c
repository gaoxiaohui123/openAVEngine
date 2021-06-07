#include "inc.h"
#include "udpbase.h"

extern char* GetvalueStr(cJSON *json, char *key, char *result);
extern int client_send_run(SocketObj *obj);
extern int client_recv_run(SocketObj *obj);
extern int client_broadcast_get(SocketObj *obj, UserData *udata0);
extern void StartSend(SocketObj *obj);
extern void StopSend(SocketObj *obj);
extern void StartRecv(SocketObj *obj);
extern void StopRecv(SocketObj *obj);
extern void ExitSend(SocketObj *obj);
extern void ExitRecv(SocketObj *obj);
extern void WaitSend(SocketObj *obj);
extern void WaitRecv(SocketObj *obj);
extern int PushData(SocketObj *obj, char *recv_buf, int recv_num, struct sockaddr_in addr_client);
extern void * PopDataAll(SocketObj *obj);
extern void SetPause(SocketObj *obj);
extern void ClearPause(SocketObj *obj);
extern int audio_read_frame(CallCapture *obj, char *outbuf);
extern void stop_capture_run(CallCapture *obj);
//extern int AddRect(CallRender *obj, SDL_Rect rect, int id);
//extern void AudioFramePushData(CallPlayer *obj, uint8_t *data, int id, int64_t frame_timestamp, int64_t now_time);
extern void AudioFrameBufferPushData(CallPlayer *obj, uint8_t *data, int size, int id, int64_t frame_timestamp, int64_t now_time);
extern int AddEncoder(pthread_mutex_t *lock, EncoderNode **encoderHead, Encoder *encoder);
extern int32_t GetNetResult(CallCodecAudio *obj, int ref_idx);
extern int push_net_info(NetInfoObj *netInfoObj, int avtype, int chanId, uint8_t *data, int insize, int loss_rate);
extern int push_rtx_info(NetInfoObj *netInfoObj, NACK_LOSS *nackLoss);
extern int64_t get_sys_time2();
extern int WaitCapture(SocketObj *obj, int status, int avtype);
extern void WaitRender(SocketObj *obj);
extern void set_avstream2audio(char *handle, char *handle2);
extern void WaitStream(SocketObj *obj);

static RtxNode * FindeRtxByTimestamp(pthread_mutex_t *lock, RtxNode *rtxHead, uint32_t send_time);


static void * test_encode(int difftime, int pkt_send_num, int64_t now_time, int frame_size, int *frame_bytes)
{
    int size = sizeof(UserData);
    char *send_buf = malloc(size);
    //
    int datatype = kDATA;
    UserData *udata = (UserData *)send_buf;
    if( difftime >= TEST_TIME_LEN )
    {
        MYPRINT("test_encode: difftime=%d \n", difftime);
        datatype = kCMD;
    }
    else{
        datatype = kDATA;
    }
    if(datatype == kDATA)
    {
        udata->data0.head.datatype = datatype;
        size = sizeof(AVData);
        int raw_size = MAX_DATA_SIZE;
        int head_size = size - MAX_DATA_SIZE;
        int tail = (frame_bytes[0] + MAX_DATA_SIZE) - frame_size;
        if(tail > 0)
        {
            raw_size = (MAX_DATA_SIZE - tail);
            size = head_size + raw_size;
            //MYPRINT("test_encode: tail=%d \n",tail);
        }
        udata->data0.head.size = size;
        udata->data0.seqnum = pkt_send_num;
        udata->data0.now_time = now_time;
        //
        udata->data0.sessionId = 0;
        udata->data0.chanId = 0;
        //
        frame_bytes[0] += raw_size;
    }
    else{
        udata->data1.head.datatype = datatype;
        size = sizeof(CmdData);
        udata->data1.head.size = size;
        udata->data1.cmdtype = (CMDType)kExit;
        udata->data1.status = 0;
        udata->data1.chanId = 0;//obj->id;
        udata->data1.avtype = 0;
        udata->data1.selfmode = 0;
        udata->data1.testmode = 0;
        udata->data1.modeId = 0;
        udata->data1.actor = 1;
        udata->data1.sessionId = 0;
    }
    return (void *)send_buf;
}
#if 0
int AddRtx(pthread_mutex_t *lock, RtxNode **rtxHead, AVData *data)
{
    int ret = 0;
    pthread_mutex_lock(lock);
    RtxNode *head,*pnew, *q;
    if(!*rtxHead)
    {
        //head不存数据,head->next指向第一个数据,head->num保存总索引（也可以设计将第一个节点存入head）
        //注意链表元的2个组成部分，元和next指针，元控制的是整个链表元的地址，next指针则寄生在元下；
        //MYPRINT("PushData: create head: obj->data_list=%x \n", obj->data_list);
        head = (RtxNode *)malloc(sizeof(RtxNode));  //创建头节点。
        head->num = 0;
        head->idx = 0;
        head->next = NULL;  //头节点指针域置NULL
        head->tail = head;  // 开始时尾指针指向头节点；此时head->tail->next与head->next是同一地址
        *rtxHead = (RtxNode *)head;
    }
    head = (RtxNode *)(*rtxHead);
    pnew = (RtxNode *)malloc(sizeof(RtxNode));  //创建新节点
    pnew->data = malloc(data->size);
    memcpy(pnew->data, data, data->size);
    AVData *data2 = pnew->data;
    data2->head.rtx = 1;
    pnew->idx = head->idx;
    pnew->next = NULL;   //新节点指针域置NULL
    head->tail->next = pnew;  //新节点插入到表尾；首个节点时，则意味着ead->next也指向了pnew
    head->tail = pnew;   //为指针指向当前的尾节点;元地址迁移，元的原先的next脱离了元，新地址的next成为元的next指针；
    head->idx++;//只增不减
    head->num++;//有增有减
    pthread_mutex_unlock(lock);
    return ret;
}
#endif
//transfer way same as rtt
static int GetRtxData(CallCodecAudio *obj, NetInfoNode *netNode)
{
    int ret;
    NetInfo *netinfo = &netNode->netinfo;
    NACK_LOSS *nackLoss = &netinfo->nack.nack1;
    uint32_t send_time = nackLoss->send_time;
    uint16_t start_seqnum = nackLoss->start_seqnum;
    uint16_t chanId = nackLoss->chanId;
    uint16_t loss_type = nackLoss->loss_type;
    uint16_t loss0 = nackLoss->loss0;
    uint32_t loss1 = nackLoss->loss1;
    uint32_t loss2 = nackLoss->loss2;
    uint32_t loss3 = nackLoss->loss3;
    uint32_t loss4 = nackLoss->loss4;
    if(loss0)
    {
        //frame resend
    }
    MYPRINT("GetRtxData: obj->rtxHead= %x \n", obj->rtxHead);
    RtxNode *q = FindeRtxByTimestamp(&obj->lock, obj->rtxHead, send_time);
    MYPRINT("GetRtxData: q= %x \n", q);
    if(q)
    {
        //q->max_pkt_num;
        //q->start_seqnum;
        //q->max_seqnum;
        //找到int与ushort包序之间的换算关系
        int start = q->min_seqnum;
        int end = q->min_seqnum + q->rtp_pkt_num;
        int has_zero = 0;
        int idx = 0;
        int k = 0;
        uint32_t urecvd[4] = {loss1, loss2, loss3, loss4};
        for(int i = start; i < end; i++)
        {
            uint16_t seqnum = i % LEFT_SHIFT16;
            if(seqnum == start_seqnum)
            {
                has_zero = 1;
            }
            if(has_zero)
            {
                idx = k / 32;
                int m = k % 32;
                uint32_t one = urecvd[idx];
                int flag = one & (1 << m);
                if(!flag)
                {
                    if(!q->rtp_pkt_num)
                    {
                        MYPRINT("GetRtxData: q->rtp_pkt_num=%d \n", q->rtp_pkt_num);
                    }
                    int j = seqnum % q->rtp_pkt_num;
                    AVData *thisAVData = &q->datalist[j];
                    int size = thisAVData->head.size;
                    AVData *data = malloc(size);
                    memcpy((void *)data, (void *)thisAVData, size);
                    SocketObj *sock = (SocketObj *)obj->sock;
                    PushData(sock, (char *)data, size, sock->addr_serv);
                }
                k++;
                if(k >= 128)
                {
                    break;
                }
            }
        }
    }

    return ret;
}
static int AddRtxData(pthread_mutex_t *lock, RtxNode *pnew, AVData *data, int seqnum)
{
    int ret = 0;
    pthread_mutex_lock(lock);
    if(!pnew->rtp_pkt_num)
    {
        MYPRINT("AddRtxData: pnew->rtp_pkt_num=%d \n", pnew->rtp_pkt_num);
    }
    int i = seqnum % pnew->rtp_pkt_num;
    AVData *thisAVData = &pnew->datalist[i];
    memcpy((void *)thisAVData, (void *)data, data->head.size);
    thisAVData->head.rtx = 1;
    pnew->pkt_sum_size += data->head.size;
    pnew->pkt_num += 1;
    pnew->max_seqnum = seqnum;
    pthread_mutex_unlock(lock);
    return ret;
}
static RtxNode * RtxPushData(pthread_mutex_t *lock, RtxNode **rtxHead, uint8_t *pkt_data, int pkt_size, int seqnum, int64_t frame_timestamp)
{
    int lossless = 0;
    pthread_mutex_lock(lock);
    RtxNode *head, *pnew, *p, *q;
    if(!*rtxHead)
    {
        //head不存数据,head->next指向第一个数据,head->num保存总索引（也可以设计将第一个节点存入head）
        //注意链表元的2个组成部分，元和next指针，元控制的是整个链表元的地址，next指针则寄生在元下；
        //MYPRINT("PushData: create head: sock->data_list=%x \n", sock->data_list);
        head = (RtxNode *)calloc(1, sizeof(RtxNode));  //创建头节点。
        head->num = 0;
        head->idx = 0;
        head->frame_timestamp = frame_timestamp;
        head->next = NULL;  //头节点指针域置NULL
        head->tail = head;  // 开始时尾指针指向头节点；此时head->tail->next与head->next是同一地址
        *rtxHead = (RtxNode *)head;
    }
    head = (RtxNode *)(*rtxHead);
    if(head->num > 10)
    {
        //MYPRINT("RtxPushData: head->num=%d \n", head->num);
        int difftime = (int)(frame_timestamp - head->frame_timestamp);
        //MYPRINT("RtxPushData: difftime=%d \n", difftime);
        if(difftime > 2000) //2s
        {
            p = head->next;
            q = head->next->next;
            head->next = q;
            head->frame_timestamp = q->frame_timestamp;
            free(p->datalist);
            free(p);
            head->num--;
        }
    }

    //创建新的帧
    pnew = (RtxNode *)calloc(1, sizeof(RtxNode));  //创建新节点
    //max_pkt_num 可根据扩展信息，赋予精确值
    RtpInfo info = {0};
    info.raw_offset = -1;
    int is_hcsvc_rtp = GetRtpInfo((uint8_t*)pkt_data, pkt_size, &info, AAC_PLT);
    pnew->rtp_pkt_num = info.rtp_pkt_num ? info.rtp_pkt_num : 1;
    pnew->max_pkt_num = info.max_pkt_num ? info.max_pkt_num : 1;
    pnew->start_seqnum = info.start_seqnum;
    if(!is_hcsvc_rtp)
    {
        MYPRINT("error:RtxPushData: is_hcsvc_rtp=%d \n", is_hcsvc_rtp);
    }
    pnew->datalist = calloc(1, pnew->rtp_pkt_num * sizeof(AVData));
    pnew->pkt_num = 0;
    pnew->min_seqnum = seqnum;//info.seqnum
    pnew->max_seqnum = seqnum;
    pnew->frame_timestamp = frame_timestamp;
    pnew->now_time = api_get_time_stamp_ll();
    //
    //int i = seqnum % pnew->rtp_pkt_num;
    //AVData *thisAVData = &pnew->datalist[i];
    pnew->pkt_sum_size = 0;//pkt_size;
    pnew->pkt_num = 0;
    //
    pnew->ref_idx = info.ref_idx;//test
    pnew->pict_type = info.ref_idc;//test
    //int64_t now_time = get_sys_time();
    pnew->idx = head->idx;
    //pnew->now_time = get_sys_time();
    pnew->next = NULL;   //新节点指针域置NULL
    head->tail->next = pnew;  //新节点插入到表尾；首个节点时，则意味着ead->next也指向了pnew
    head->tail = pnew;   //为指针指向当前的尾节点;元地址迁移，元的原先的next脱离了元，新地址的next成为元的next指针；
    //
    head->idx++;
    head->num++;
    //MYPRINT("PushData: head->num=%d \n", head->num);
    pthread_mutex_unlock(lock);
    return pnew;
}
static RtxNode * FindeRtxByTimestamp(pthread_mutex_t *lock, RtxNode *rtxHead, uint32_t send_time)
{
    RtxNode *ret = NULL;
    pthread_mutex_lock(lock);
    RtxNode *head, *pnew, *p, *q;
    head = rtxHead;
    q = head->next;
    while(q)
    {
        if((q->frame_timestamp & 0xFFFFFFFF) == send_time)
        {
            ret = q;
            break;
        }
        else{
        }
        q = q->next;
    }
    pthread_mutex_unlock(lock);
    return ret;
}
static int pkt2sendbuf(SocketObj *sock, uint8_t *p_pkt_data, int *p_pkt_size, int p_pkt_num, int insize, int *p_sumbytes,
    int *p_pkt_send_num, int *p_frame_num, int step, int wait_time)//,
{
    int ret = 0;
    pthread_mutex_lock(&sock->status_lock);
    SessionObj * session = (SessionObj *)sock->session;
    int chanId = 0;
    int sessionId = 0;
    CallCodecAudio *obj = NULL;
    if(session){
        obj = session->codec;
        sessionId = session->sessionInfo->sessionId;
        chanId = session->sessionInfo->chanId;
    }
    pthread_mutex_unlock(&sock->status_lock);
    RtxNode * thisRtxNode = NULL;

    int sumbytes = p_sumbytes[0];
    int pkt_send_num = p_pkt_send_num[0];

    int size = sizeof(AVData);
    int datatype = kDATA;
    int offset = 0;
    int idx = 0;
    int data_size = MAX_DATA_SIZE;
    //size = sizeof(AVData);
    int head_size = size - MAX_DATA_SIZE;

    int64_t now_time = get_sys_time();

    do
    {
        int tail = insize - offset;
        if(tail < MAX_DATA_SIZE)
        {
            //size -= (MAX_DATA_SIZE - tail);
            size = head_size + tail;
            data_size = tail;
        }
        if(p_pkt_size)
        {
            data_size = p_pkt_size[idx];
            size = head_size + data_size;
            if(!idx && OPEN_NETACK)
            {
                thisRtxNode = RtxPushData(&obj->lock, &obj->rtxHead, p_pkt_data, data_size, pkt_send_num, now_time);
            }
            //MYPRINT("pkt2sendbuf: size=%d, idx=%d \n", size, idx);
        }
        char *send_buf = malloc(size);//calloc(1, size);
        AVData *udata = (AVData *)send_buf;
        udata->head.datatype = datatype;
        udata->head.codec = 1;
        memcpy(udata->data, &p_pkt_data[offset], data_size);
        udata->head.size = size;
        udata->head.rtx = 0;
        udata->seqnum = pkt_send_num;
        udata->now_time = now_time;
        //
        udata->sessionId = sessionId;
        udata->chanId = chanId;
        if(thisRtxNode && idx < thisRtxNode->rtp_pkt_num && OPEN_NETACK)
        {
            //数据缓存，用于重传备份
            AddRtxData(&obj->lock, thisRtxNode, udata, pkt_send_num);
        }
#ifdef SIMULATOR_LOSSRATE
        //loss test
        int send = 1;
        unsigned int seed = (unsigned int)get_sys_time2();
        srand(seed);
        int randidx = rand() % 100;
        if( randidx < SIMULATOR_LOSSRATE && true)
        {
            send = 0;
            free(send_buf);
        }
        if(send)
        {
            PushData(sock, (char *)udata, udata->head.size, sock->addr_serv);
            sumbytes += size;
        }
#else
        PushData(sock, (char *)udata, udata->head.size, sock->addr_serv);
        offset += data_size;
        sumbytes += size;
#endif
        offset += data_size;
        pkt_send_num++;
        idx++;
//#ifndef _WIN32
        if((pkt_send_num % step) == (step - 1))
        {
            usleep(wait_time);
        }
//#endif
    }while(offset < insize);
    if(p_pkt_num > 0 && p_pkt_num != idx)
    {
        MYPRINT("error: gxh:pkt2sendbuf: p_pkt_num=%d, idx=%d \n", p_pkt_num, idx);
    }
    //MYPRINT("gxh:pkt2sendbuf: p_pkt_num=%d, idx=%d \n", p_pkt_num, idx);
    //sumbytes += frame_bytes;
    p_frame_num[0]++;
    p_sumbytes[0] = sumbytes;
    p_pkt_send_num[0] = pkt_send_num;
    return ret;
}
static int HCSVCRpt(CallCodecAudio *obj, int idx, int insize, int is_sikp, int **p_pkt_size, int *p_pkt_num)
{
    int ret = 0;
    //LayerConfig *config = &configurations_[idx];
    CodecInfoObj *config = (CodecInfoObj *)&obj->codecInfo;

    obj->json = api_renew_json_int(obj->json, "insize", insize);
    obj->json = api_renew_json_int(obj->json, "seqnum", obj->seqnum);
    obj->json = api_renew_json_int(obj->json, "time_offset", obj->sessionInfo->nettime);
    //在服务器更新main_spatial_idx值
    obj->json = api_renew_json_int(obj->json, "main_spatial_idx", config->main_spatial_idx);

    char* jsonstr = api_json2str(obj->json);
    //MYPRINT("HCSVCRpt: jsonstr= %s \n", jsonstr);
    uint8_t *src = obj->outbuffer;
    uint8_t *dst = obj->rtpbuffer;
    obj->outrtp = dst;

    int ret2 = api_audio_raw2rtp_packet(obj->handle,
                                        (char *)src,
                                        jsonstr,
                                        (char *)dst,
                                        obj->outparam);

    obj->seqnum = atoi(obj->outparam[1]);
    int rtpNum = 0;
    char *paramstr = (char *)obj->outparam[0];
    char *key = "rtpSize";
    int *rtpSize = api_get_array_int(paramstr, key, &rtpNum);

    //MYPRINT("HCSVCRpt: paramstr= %s \n", paramstr);
    //MYPRINT("HCSVCRpt: ret2= %d \n", ret2);

    int enable_fec = config->enable_fec;
    obj->json = api_renew_json_int(obj->json, "enable_fec", enable_fec);
    obj->json = api_delete_item(obj->json, "insize");
    if(ret2 > 0)
    {
        ret = ret2;
        if(enable_fec)
        {
            src = obj->rtpbuffer;
            dst = obj->outbuffer;
            obj->outrtp = dst;
            obj->json = api_renew_json_int(obj->json, "enable_fec", enable_fec);
            obj->json = api_renew_json_float(obj->json, "loss_rate", config->loss_rate);
            obj->json = api_renew_json_float(obj->json, "code_rate", config->code_rate);
            obj->json = api_renew_json_array(obj->json, "inSize", rtpSize, rtpNum);
            api_get_array_free(rtpSize);
            rtpSize = NULL;
            //MYPRINT("HCSVCRpt: 0: rtpNum= %d \n", rtpNum);
            int fec_k = rtpNum;
            int fec_n = (int)(fec_k / config->code_rate + 0.9);
            //MYPRINT("HCSVCRpt: config->code_rate= %f, config->loss_rate=%f \n", config->code_rate, config->loss_rate);
            //MYPRINT("gxh:HCSVCRpt: fec_k= %d, fec_n=%d, idx=%d \n", fec_k, fec_n, idx);
            api_json2str_free(jsonstr);
            jsonstr = api_json2str(obj->json);
            obj->outparam[0] = "";
            obj->outparam[1] = "";
            ret2 = api_fec_encode(  obj->handle,
                                    (char *)src,
                                    jsonstr,
                                    (char *)dst,
                                    obj->outparam);
            obj->json = api_delete_item(obj->json, "inSize");
            if(ret2 > 0)
            {
                obj->seqnum = atoi(obj->outparam[1]);
                rtpNum = 0;
                //rtpSize = api_get_array_by_str(obj->outparam[0], ',', &rtpNum);
                if(rtpSize)
                {
                    api_get_array_free(rtpSize);
                    rtpSize = NULL;
                }
                paramstr = (char *)obj->outparam[0];
                key = "rtpSize";
                rtpSize = api_get_array_int(paramstr, key, &rtpNum);
                //ret = rtpNum;
                //MYPRINT("HCSVCRpt: 1: rtpNum= %d \n", rtpNum);
                api_get_array_free(rtpSize);
                rtpSize = NULL;
                ret = ret2;
            }
        }
        else {
            //保证数据最终存储在obj->outbuffer;
            //memcpy(src, dst,ret2); //obj->outbuffer;//obj->rtpbuffer;
        }
    }
    if(rtpSize)
    {
        api_get_array_free(rtpSize);
        rtpSize = NULL;
    }
    api_json2str_free(jsonstr);
    paramstr = (char *)obj->outparam[0];

    key = "rtpSize";
    *p_pkt_size = api_get_array_int(paramstr, key, p_pkt_num);
    if(!(*p_pkt_size) && ret > 0)
    {
        *p_pkt_size = malloc(sizeof(int));
        (*p_pkt_size)[0] = ret;
        p_pkt_num[0] = 1;
    }
    //MYPRINT("HCSVCRpt: p_pkt_num[0]= %d \n", p_pkt_num[0]);
    //src = obj->rtpbuffer;
    //dst = obj->outbuffer;
    if(is_sikp && false)
    {
        int bytes = api_get_rtpheader2((char*)dst, p_pkt_size, p_pkt_num, RAW_OFFSET);
    }

    return ret;
}
int audio_encode_one_frame(SocketObj *sock, uint8_t *frame_data, int insize,
    int *p_sumbytes, int *p_pkt_send_num, int *p_frame_num, int step, int wait_time)
{
    int ret = 0;
    pthread_mutex_lock(&sock->status_lock);
    SessionObj * session = (SessionObj *)sock->session;
    CallCodecAudio *obj = session->codec;
    pthread_mutex_unlock(&sock->status_lock);
    //session->sessionInfo
    if(obj)
    {
        CodecInfoObj *config = (CodecInfoObj *)&obj->codecInfo;
        if(!obj->outfp && false)
        {
            const char *filename = "/home/gxh/works/gxhshare/test.aac";
            obj->outfp = fopen(filename, "wb");
        }
        int data_size = insize;
        int *p_pkt_size = NULL;
        uint8_t *p_pkt_data = frame_data;
        int p_pkt_num = 0;
        if(!config->lossless)
        {
            //MYPRINT("audio_encode_one_frame: ref_idx=%d \n", ref_idx);

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
            //MYPRINT("audio_encode_one_frame: jsonstr=%s \n", jsonstr);
            //encode frame
            ret = api_audio_codec_one_frame(    obj->handle,
                                                (char *)frame_data,
                                                jsonstr,
                                                (char *)obj->outbuffer,
                                                obj->outparam);
            api_json2str_free(jsonstr);
            //MYPRINT("audio_encode_one_frame: ret=%d \n", ret);
            if(ret > 0)
            {
                if ( obj->outfp != NULL)
                {
                    //MYPRINT("audio_encode_one_frame: ret=%d \n", ret);
                    fwrite(obj->outbuffer, 1, ret, obj->outfp);
                }
                //raw2rtp
                //fec encode
                ret = HCSVCRpt(obj, 0, ret, is_skip, &p_pkt_size, &p_pkt_num);
                if(ret > 0)
                {
                    //MYPRINT("audio_encode_one_frame: 2: ret=%d \n", ret);
                    p_pkt_data = obj->outrtp;
                    data_size = ret;
                }
            }
            else{
                MYPRINT("audio_encode_one_frame: ret=%d \n", ret);
                return ret;
            }
        }
        ret = pkt2sendbuf(sock, p_pkt_data, p_pkt_size, p_pkt_num, data_size, p_sumbytes, p_pkt_send_num, p_frame_num, step, wait_time);
        if(p_pkt_size)
        {
            api_get_array_free(p_pkt_size);
        }
        obj->frame_idx++;
    }

    return ret;
}
static int encode_frame_run(SocketObj *sock)
{
    int ret = 0;
    FILE *pcmfp = NULL;
    char *readbuf = NULL;
    char *filename = NULL;
    pthread_mutex_lock(&sock->status_lock);
    sock->recv_status = 1;
    int status = sock->recv_status;
    SessionObj * session = (SessionObj *)sock->session;
    CallCodecAudio *obj = session->codec;
    void *handle = NULL;

    int frame_size = 8192;//1024*8
    if(session)
    {
        //char *filename = session->sessionInfo->streamName;
        filename = session->sessionInfo->yuvfilename;
        MYPRINT("encode_frame_run: filename=%s \n", filename);
        if(filename && strcmp(filename, ""))
        {
            if(false)
            {
                pcmfp = fopen(filename, "rb");
                readbuf = malloc(frame_size * sizeof(char));
            }
        }
    }
    pthread_mutex_unlock(&sock->status_lock);

    int frame_num = 0;
    int pkt_send_num = 0;
    int64_t sumbytes = 0;
    int64_t time0 = get_sys_time();
    sock->start_time = time0;
    int frame_bytes = 0;
    int64_t frame_time = 0;
    //
    int wait_time = 2000;//1000;//500;//1000;//125;//250;//500;//1000;//500;

    int max_pkt_num = AUDIO_MAX_DELAY_PKT_NUM;//16 * 40 = 640ms
    float factor = 10;
    int step = (int)(max_pkt_num / factor);//test
    //
#ifndef SIMULATOR_DATA
    if(session && filename && strcmp(filename, "") && !readbuf)
    {
        WaitStream(sock);
        handle = session->handle;
        //api_avstream_status(handle, 1);//start
    }
    if(!handle)
    {
        status = WaitCapture(sock, 0, kIsAudio);
    }
#endif

    CallCapture *capture = NULL;
    MYPRINT("audio encode_frame_run: status=%d \n",status);

    while(status > 0)
    {
        int64_t now_time = get_sys_time();
        int difftime = (int)(now_time - sock->start_time);
        //测试模式下，是全速运行，未进行速度控制
        pthread_mutex_lock(&sock->status_lock);
        session = (SessionObj *)sock->session;
        capture = session->capture;
        pthread_mutex_unlock(&sock->status_lock);
        ret = 0;
        if((capture || pcmfp || handle) && difftime < TEST_TIME_LEN)
        {
            char *outbuf = NULL;

            if(pcmfp)
            {
                ret = fread(readbuf, 1, frame_size, pcmfp);
                if(ret != frame_size)
                {
                    fseek(pcmfp, 0, SEEK_SET);
                    ret = fread(readbuf, 1, frame_size, pcmfp);
                }
                outbuf = readbuf;
            }
            else if(handle)
            {
                AVReadNode *p = (AVReadNode *)api_avstream_pop_data(handle, kIsAudio);
                //MYPRINT("audio encode_frame_run: p=%x \n", p);
                if(p)
                {
                    outbuf = p->data;
                    ret = p->size;
                    free(p);
                }
            }
            else{
                ret = audio_read_frame(capture, &outbuf);
            }

            if(ret > 0)
            {
                //MYPRINT("audio encode_frame_run: read_frame: ret=%d \n", ret);
                if(session->render)
                {
                    //MYPRINT("encode_frame_run: session->render=%x \n", session->render);
                    uint8_t *frame_data = (uint8_t *)malloc(ret * sizeof(uint8_t));
                    memcpy(frame_data, outbuf, ret);
                    int selfmode = session->sessionInfo->selfmode;
                    int chanId = session->sessionInfo->chanId;
                    int actor = session->sessionInfo->actor;
                    if(selfmode)
                    {
                        chanId = (actor < DECACTOR) ? 1 : 0;
                    }
                    //MYPRINT("encode_frame_run: read_frame: chanId=%d \n", chanId);
                    AudioFrameBufferPushData(session->render, frame_data, ret, chanId, now_time, now_time);
                }
#if 0
                int size = sizeof(UserData);
                int datatype = kDATA;
                int offset = 0;
                int data_size = MAX_DATA_SIZE;
                size = sizeof(AVData);
                int head_size = size - MAX_DATA_SIZE;

                do
                {
                    int tail = ret - offset;
                    if(tail < MAX_DATA_SIZE)
                    {
                        //size -= (MAX_DATA_SIZE - tail);
                        size = head_size + tail;
                        data_size = tail;
                    }
                    char *send_buf = malloc(size);
                    UserData *udata = (UserData *)send_buf;
                    udata->data0.head.datatype = datatype;

                    memcpy(udata->data0.data, &outbuf[offset], data_size);
                    udata->data0.head.size = size;
                    udata->data0.seqnum = pkt_send_num;
                    udata->data0.now_time = now_time;
                    //
                    udata->data0.sessionId = 0;
                    udata->data0.chanId = 0;
                    //
                    PushData(sock, (char *)udata, udata->data1.head.size, sock->addr_serv);
                    offset += data_size;
                    sumbytes += size;
                    pkt_send_num++;

                    if((pkt_send_num % step) == (step - 1))
                    {
                        usleep(wait_time);
                    }

                }while(offset < ret);
                //sumbytes += frame_bytes;
                frame_num++;
#else
                ret = audio_encode_one_frame(sock, outbuf, ret, &sumbytes, &pkt_send_num, &frame_num, step, wait_time);
                if(handle && outbuf)
                {
                    av_free(outbuf);
                }
#endif
            }
            else{
                if(handle && outbuf)
                {
                    av_free(outbuf);
                }
                usleep(10000);//10ms
            }
        }
        else{
            if(1)//(difftime > TEST_TIME_LEN)
            {
                //MYPRINT("encode_frame_run: frame_bytes=%d \n", frame_bytes);
                if(frame_time == 0)
                {
                    frame_time = now_time;
                }
                if(session && session->sessionInfo)
                {
                    int w = DEFAULT_WIDTH;//session->sessionInfo->width;
                    int h = DEFAULT_HEIGHT;//session->sessionInfo->height;
                    frame_size = (w * h * 3) >> 1;
                }
//#ifndef _WIN32
                if((pkt_send_num % step) == (step - 1))
                {
                    usleep(wait_time);//生成包频率过快，毫秒为单位的时间戳不足以区隔帧//并且存在编解码速度匹配问题
                }
//#endif
                if(frame_bytes >= frame_size)
                {
                    if((DEFAULT_WIDTH * DEFAULT_HEIGHT) == (1280 * 720))
                    {
                        //usleep(4000);
                        //usleep(5000);//5ms//生成包频率过快，毫秒为单位的时间戳不足以区隔帧
                    }
                    else if((DEFAULT_WIDTH * DEFAULT_HEIGHT) == (1920 * 1080))
                    {
                        //usleep(4000);
                        //usleep(11250);//11ms//生成包频率过快，毫秒为单位的时间戳不足以区隔帧
                    }

                    now_time = get_sys_time();
                    //MYPRINT("encode_frame_run: frame_size=%d \n", frame_size);
                    frame_time = now_time;
                    //sumbytes += frame_bytes;
                    frame_bytes = 0;
                    frame_num++;

                }
                //MYPRINT("encode_frame_run: frame_bytes=%d \n", frame_bytes);
                //MYPRINT("encode_frame_run: frame_size=%d \n", frame_size);
                UserData *udata = (UserData *)test_encode(difftime, pkt_send_num, frame_time, frame_size, &frame_bytes);
                if(udata)
                {
                    if(udata->data0.head.datatype == kCMD)
                    {
                        //test
                        MYPRINT("encode_frame_run: udata->data0.head.datatype=%d \n", udata->data0.head.datatype);
                        client_broadcast_get(sock, udata);

                        StopRecv(sock);
                    }
                    else{
                        PushData(sock, (char *)udata, udata->data1.head.size, sock->addr_serv);
                        sumbytes += udata->data1.head.size;
                        pkt_send_num++;
                    }
                }
                //MYPRINT("encode_frame_run: pkt_send_num=%d \n", pkt_send_num);
            }
            else{
                usleep(10000);//10ms
            }
        }

        //frame_num++;
        difftime = (int)(now_time - time0);
        if(difftime > MAX_INTERVAL_CLIENT)
        {
            float framerate = (float)((frame_num * 1000.0) / difftime); //fps
            float bitrate = (float)(((sumbytes << 3) * 1.0) / (difftime * 1000));//mbps
            if(obj)
            {
                obj->bitrate = (int)(((sumbytes << 3) * 1) / (difftime)); //kbps
                obj->framerate = (int)((frame_num * 1000) / difftime);//fps
            }
            time0 = now_time;
            frame_num = 0;
            sumbytes = 0;
            //frame_bytes = 0;
            MYPRINT("audio encode_frame_run: framerate=%2.1f (fps) \n", framerate);
            MYPRINT("audio encode_frame_run: bitrate=%5.2f (Mbps) \n", bitrate);
        }

        //MYPRINT("encode_frame_run: sock->recv_status=%d \n", sock->recv_status);
        //MYPRINT("encode_frame_run: sock->send_status=%d \n", sock->send_status);

        pthread_mutex_lock(&sock->status_lock);
        status = (sock->recv_status > 0) & (sock->send_status > 0);
        pthread_mutex_unlock(&sock->status_lock);
        //MYPRINT("encode_frame_run: status=%d \n", status);
    }
    if(session && capture)
    {
        MYPRINT("audio encode_frame_run: call stop_capture_run \n");
        stop_capture_run(capture);
    }
    ExitRecv(sock);
    StopSend(sock);
    WaitSend(sock);//停止发包
    if(readbuf)
    {
        free(readbuf);
    }
    if(pcmfp)
    {
        fclose(pcmfp);
    }
    MYPRINT("audio encode_frame_run: over \n");
    return ret;
}
static int RenewCountLossRateInfo(CallCodecAudio *obj, FrameJitterNode *head, char *recv_buf, int recv_size, int seqnum, int64_t frame_timestamp)
{
    int ret = 0;
    int head_size = sizeof(AVData) - MAX_DATA_SIZE;
    AVData *udata = (AVData *)recv_buf;
    int raw_size = udata->head.size - head_size;
    if(!udata->head.rtx)
    {
        //for count loss rate
        if(frame_timestamp <= (head->last_frame_timestamp + COUNT_LOSSRATE_INTERAVL))
        {
            if(seqnum < head->last_min_seqnum)
            {
                head->last_min_seqnum = seqnum;
            }
            if(seqnum > head->last_max_seqnum)
            {
                head->last_max_seqnum = seqnum;
            }
            head->last_pkt_num++;
        }
        else{
            if(seqnum < head->min_seqnum)
            {
                head->min_seqnum = seqnum;
            }
            if(seqnum > head->max_seqnum)
            {
                head->max_seqnum = seqnum;
            }
            head->pkt_num++;
        }
        //count loss rate
        int difftime = (int)(frame_timestamp - head->frame_timestamp);
        if(difftime > COUNT_LOSSRATE_INTERAVL)
        {
            if(head->last_frame_timestamp > 0)
            {
                int sum_pkt = head->last_max_seqnum - head->last_min_seqnum;
                //MYPRINT("JitterPushData: sum_pkt=%d \n", sum_pkt);
                //MYPRINT("JitterPushData: head->pkt_num=%d \n", head->pkt_num);
                //MYPRINT("JitterPushData: head->last_pkt_num=%d \n", head->last_pkt_num);
                obj->lossrate = 1 + (100 * (sum_pkt - head->last_pkt_num)) / sum_pkt;
            }
            head->last_min_seqnum = head->min_seqnum;
            head->last_max_seqnum = head->max_seqnum;
            head->last_pkt_num = head->pkt_num;
            head->last_frame_timestamp = head->frame_timestamp;
            //
            head->min_seqnum = seqnum;
            head->max_seqnum = seqnum;
            head->pkt_num = 1;
            head->frame_timestamp = frame_timestamp;
        }
    }
    return ret;
}
static int JitterPushData(SocketObj *sock, char *recv_buf, int recv_size, int seqnum, int64_t frame_timestamp)
{
    int ret = 0;
    int lossless = 0;
    int enable_netack = 0;
    pthread_mutex_lock(&sock->status_lock);
    SessionObj * session = (SessionObj *)sock->session;
    CallCodecAudio *obj = session->codec;
    pthread_mutex_unlock(&sock->status_lock);
    if(obj)
    {
        CodecInfoObj *config = (CodecInfoObj *)&obj->codecInfo;
        lossless = config->lossless;
        enable_netack = config->enable_netack;
    }
    //MYPRINT("JitterPushData: lossless=%d \n", lossless);
    pthread_mutex_lock(&sock->lock);
#if 1
    FrameJitterNode *head,*pnew, *p, *q;
    if(!sock->frameJitterHead)
    {
        //head不存数据,head->next指向第一个数据,head->num保存总索引（也可以设计将第一个节点存入head）
        //注意链表元的2个组成部分，元和next指针，元控制的是整个链表元的地址，next指针则寄生在元下；
        //MYPRINT("PushData: create head: sock->data_list=%x \n", sock->data_list);
        head = (FrameJitterNode *)calloc(1, sizeof(FrameJitterNode));  //创建头节点。
        //
        head->last_min_seqnum = 0;
        head->last_max_seqnum = 0;
        head->last_pkt_num = 0;
        head->last_frame_timestamp = 0;

        head->min_seqnum = seqnum;
        head->max_seqnum = seqnum;
        head->pkt_num = 1;
        head->frame_timestamp = frame_timestamp;
        //
        head->num = 0;
        head->idx = 0;
        head->ref_idx = -1;
        head->next = NULL;  //头节点指针域置NULL
        head->tail = head;  // 开始时尾指针指向头节点；此时head->tail->next与head->next是同一地址
        sock->frameJitterHead = (void *)head;
    }
    head = (FrameJitterNode *)sock->frameJitterHead;
#ifdef OPEN_LOSSNET
    if(!lossless)
    {
        RenewCountLossRateInfo(obj, head, recv_buf, recv_size, seqnum, frame_timestamp);
    }
#endif
    if(head->num > 100)
    {
        //free(recv_buf);
        ///MYPRINT("PushData: skip head->num=%d \n", head->num);
    }
    if(frame_timestamp <= head->last_frame_time)
    {
        pthread_mutex_unlock(&sock->lock);
        return -1;
    }
    //统计丢包率
    //else
    {
        q = head->next;
        int renewflag = 0;
        while(q)
        {
            if(q->frame_timestamp == frame_timestamp)
            {
                //FramePushData2(q->frameListHead, data, id, frame_timestamp, now_time);
                if(seqnum < q->min_seqnum)
                {
                    q->min_seqnum = seqnum;
                }
                if(seqnum > q->max_seqnum)
                {
                    q->max_seqnum = seqnum;
                }
                if(!q->max_pkt_num)
                {
                    MYPRINT("JitterPushData: q->max_pkt_num=%d \n", q->max_pkt_num);
                }
                int i = seqnum % q->max_pkt_num;
                SeqFrame *thisSeqFrame = &q->seqFrame[i];
                if(!thisSeqFrame->size)
                {
                    thisSeqFrame->data = (uint8_t *)recv_buf;
                    if(!lossless)
                    {
                        int head_size = sizeof(AVData) - MAX_DATA_SIZE;
                        AVData *udata = (AVData *)recv_buf;
                        int raw_size = udata->head.size - head_size;
                        //obj->lossrate = 5;//test
                        if(!udata->head.rtx)
                        {
#ifdef OPEN_DELAYNET
                            int delay_time = api_renew_delay_time(  obj->handle,
                                                                    (obj->sessionInfo->chanId + 1),
                                                                    udata->data, raw_size,
                                                                    obj->delay_time);
                            //MYPRINT("JitterPushData: delay_time=%d \n", delay_time);
                            if(delay_time > 0)
                            {
                                obj->delay_time = delay_time > MAX_DELAY_TIME ? MAX_DELAY_TIME : delay_time;
                                MYPRINT("JitterPushData: delay_time=%d \n", delay_time);
                            }
#endif
#ifdef OPEN_NETACK
                            if(obj->netInfoObj && enable_netack)
                            {
                                push_net_info(obj->netInfoObj, kIsAudio,obj->sessionInfo->chanId, udata->data, raw_size, obj->lossrate);
                            }
#endif
                        }
                        else{
                            MYPRINT("JitterPushData: udata->head.rtx=%d \n", udata->head.rtx);
                        }
                    }
                    thisSeqFrame->size = recv_size;
                    q->pkt_sum_size += recv_size;
                    q->pkt_num++;
                }
                else{
                    //重复包
                }

                //MYPRINT("JitterPushData: q->pkt_num=%d \n", q->pkt_num);
                renewflag = 1;
                break;
            }
            else{
                //MYPRINT("JitterPushData: frame_timestamp=%lld \n", frame_timestamp);
                //MYPRINT("JitterPushData: q->frame_timestamp=%lld \n", q->frame_timestamp);
            }
            q = q->next;
        }
        if(!renewflag)
        {
            if(head->num < 100 && false)
            {
                MYPRINT("JitterPushData: head->num=%d \n", head->num);
                MYPRINT("JitterPushData: frame_timestamp=%lld \n", frame_timestamp);
            }

            //创建新的帧
            pnew = (FrameJitterNode *)calloc(1, sizeof(FrameJitterNode));  //创建新节点

            int w = DEFAULT_WIDTH;//test
            int h = DEFAULT_HEIGHT;//test
            pnew->frame_size = (w * h * 3) >> 1;
            pnew->max_data_size = MAX_DATA_SIZE;
            //max_pkt_num 可根据扩展信息，赋予精确值
            if(lossless)
            {
                //int factor = 1;//10;
                //int m = (pnew->frame_size % pnew->max_data_size) > 0;
                pnew->max_pkt_num = AUDIO_MAX_DELAY_PKT_NUM;//factor * (pnew->frame_size / pnew->max_data_size + m);
                pnew->rtp_pkt_num = pnew->max_pkt_num;
            }
            else{
                RtpInfo info = {0};
                info.raw_offset = -1;
                int head_size = sizeof(AVData) - MAX_DATA_SIZE;
                AVData *udata = (AVData *)recv_buf;
                int raw_size = udata->head.size - head_size;
                int is_hcsvc_rtp = GetRtpInfo((uint8_t*)udata->data, raw_size, &info, AAC_PLT);
                pnew->rtp_pkt_num = info.rtp_pkt_num ? info.rtp_pkt_num : 1;
                pnew->max_pkt_num = info.max_pkt_num ? info.max_pkt_num : 1;
                pnew->start_seqnum = info.start_seqnum;
                pnew->ref_idx = info.ref_idx;
                if(!is_hcsvc_rtp)
                {
                    MYPRINT("error:JitterPushData: is_hcsvc_rtp=%d \n", is_hcsvc_rtp);
                }
                //obj->lossrate = 5;//test
                if(!udata->head.rtx)
                {
#ifdef OPEN_DELAYNET
                    //注意：必需先更新网络延时信息，再更新其他网络信息
                    int delay_time = api_renew_delay_time(  obj->handle,
                                                            (obj->sessionInfo->chanId + 1),
                                                            udata->data, raw_size,
                                                            obj->delay_time);
                    if(delay_time > 0)
                    {
                        obj->delay_time = delay_time > MAX_DELAY_TIME ? MAX_DELAY_TIME : delay_time;
                        MYPRINT("JitterPushData: delay_time=%d \n", delay_time);
                    }
#endif
#ifdef OPEN_NETACK
                    if(obj->netInfoObj && enable_netack)
                    {
                        push_net_info(obj->netInfoObj, kIsAudio,obj->sessionInfo->chanId, udata->data, raw_size, obj->lossrate);
                    }
#endif
                }
            }

            pnew->seqFrame = calloc(1, pnew->max_pkt_num * sizeof(SeqFrame));
            pnew->has_rtx = 0;
            pnew->pkt_num = 0;
            pnew->min_seqnum = seqnum;
            pnew->max_seqnum = seqnum;
            pnew->frame_timestamp = frame_timestamp;
            pnew->now_time = api_get_time_stamp_ll();
            //
            if(!pnew->max_pkt_num)
            {
                MYPRINT("audio JitterPushData: pnew->max_pkt_num=%d \n", pnew->max_pkt_num);
            }
            int i = seqnum % pnew->max_pkt_num;
            SeqFrame *thisSeqFrame = &pnew->seqFrame[i];
            thisSeqFrame->data = (uint8_t *)recv_buf;
            thisSeqFrame->size = recv_size;
            pnew->pkt_sum_size = recv_size;
            pnew->pkt_num = 1;
            //
            pnew->idx = head->idx;
            //pnew->now_time = get_sys_time();
            //按时间顺序插入
#if 1
            q = head->next;
            p = head;
            while(q)
            {
                if(q->frame_timestamp > frame_timestamp)
                {
                    //MYPRINT("JitterPushData: q->frame_timestamp=%lld \n", q->frame_timestamp);
                    break;
                }
                p = q;
                q = q->next;
            }
            //if(!q)
            if(!head->num)
            {
                pnew->next = NULL;   //新节点指针域置NULL
                head->tail->next = pnew;  //新节点插入到表尾；首个节点时，则意味着ead->next也指向了pnew
                head->tail = pnew;   //为指针指向当前的尾节点;元地址迁移，元的原先的next脱离了元，新地址的next成为元的next指针；
            }
            else{
                //插入新的帧
                pnew->next = q;//p->next;   //插入节点的指针域指向第i个节点的后继节点
                p->next = pnew;    //犟第i个节点的指针域指向插入的新节点
                if(!q)
                {
                    head->tail->next = pnew;
                    head->tail = pnew;
                }
            }
#else
            pnew->next = NULL;   //新节点指针域置NULL
            head->tail->next = pnew;  //新节点插入到表尾；首个节点时，则意味着ead->next也指向了pnew
            head->tail = pnew;   //为指针指向当前的尾节点;元地址迁移，元的原先的next脱离了元，新地址的next成为元的next指针；
#endif
            //
            head->idx++;
            head->num++;
            //MYPRINT("PushData: head->num=%d \n", head->num);
        }
    }

#endif
    pthread_mutex_unlock(&sock->lock);
    return ret;
}
static int generate_nack(SocketObj *sock, CallCodecAudio *obj)
{
    int ret = 0;

    FrameJitterNode *head, *p, *q;
    head = (FrameJitterNode *)sock->frameJitterHead;
    q = head->next;
    if(q->has_rtx)
    {
        return ret;
    }
    int rtp_pkt_num = q->rtp_pkt_num;
    int max_pkt_num = q->max_pkt_num;
    int start_seqnum = q->start_seqnum;//来自于rtp扩展数据
    int min_seqnum = q->min_seqnum;
    if(!max_pkt_num)
    {
        MYPRINT("audio: generate_nack: max_pkt_num=%d \n", max_pkt_num);
    }
    int i = min_seqnum % max_pkt_num;
    SeqFrame *thisSeqFrame = &q->seqFrame[i];
    RtpInfo info = {0};
    info.raw_offset = -1;
    int head_size = sizeof(AVData) - MAX_DATA_SIZE;
    AVData *udata = (AVData *)thisSeqFrame->data;
    int raw_size = udata->head.size - head_size;
    int is_hcsvc_rtp = GetRtpInfo((uint8_t*)udata->data, raw_size, &info, AAC_PLT);
    if(is_hcsvc_rtp)
    {
        if(info.refs == 1)
        {
            int diff_num = abs(q->ref_idx - head->ref_idx);
            if(diff_num != 1)
            {
                //loss frame
                MYPRINT("audio: generate_nack: diff_num=%d \n", diff_num);
            }
            //loss pkt
            int max_nack_num = 128;
            int idx = 0;
            int rtp_pkt_num32 = (rtp_pkt_num / 32) + ((rtp_pkt_num % 32) > 0);
            uint32_t *urecvd = calloc(1,rtp_pkt_num32 * sizeof(uint32_t));
            int rtp_pkt_num128 = (rtp_pkt_num / 128) + ((rtp_pkt_num % 128) > 0);
            uint16_t *useqnums = calloc(1,rtp_pkt_num128 * sizeof(uint16_t));
            int start = min_seqnum;//start_seqnum;
            int end = start + rtp_pkt_num;
            int k = 0;//
            int bidx = 0;//block idx
            int has_zero = 0;
            int zero_sum = 0;
            uint32_t one = 0xFFFFFFFF;
            for(int i = start; i < end; i++)
            {
                uint16_t seqnum = i % LEFT_SHIFT16;//???
                int j = i % max_pkt_num;
                SeqFrame *thisSeqFrame = &q->seqFrame[j];
                if(!thisSeqFrame->size)
                {
                    zero_sum++;
                    has_zero = 1;//第一次出现了0
                    if(!k)
                    {
                        useqnums[bidx] = seqnum;//将首个0的seqnum保存起来
                        bidx++;
                    }
                    one = one & (~(1 << k % 32));//将0值保存到变量中
                }
                if(k && !(k % 32))//4个变量中的序号
                {
                    urecvd[idx] = one;//将变量缓存起来
                    one = 0xFFFFFFFF;//变量重新初始化
                    idx++;//4个变量的序号递增
                }
                if(has_zero)//是否已经出现过0值
                {
                    k++;
                    if(k >= 128)
                    {
                        k = 0;
                        has_zero = 0;//重新初始化
                    }
                }
                if(!thisSeqFrame)
                {
                    MYPRINT("error:generate_nack: \n");
                }
            }
            MYPRINT("audio: generate_nack: bidx=%d \n", bidx);
            MYPRINT("audio: generate_nack: idx=%d \n", idx);
            MYPRINT("audio: generate_nack: zero_sum=%d \n", zero_sum);
            if(bidx)
            {
                int j = 0;
                for(int i = 0; i < bidx; i++)
                {
                    NACK_LOSS nackLoss;
                    nackLoss.start_seqnum = useqnums[i];
                    nackLoss.loss1 = 0xFFFFFFFF;
                    nackLoss.loss2 = 0xFFFFFFFF;
                    nackLoss.loss3 = 0xFFFFFFFF;
                    nackLoss.loss4 = 0xFFFFFFFF;
                    if(j < idx)
                    {
                        nackLoss.loss1 = urecvd[j]; j++;
                    }
                    if(j < idx)
                    {
                        nackLoss.loss2 = urecvd[j]; j++;
                    }
                    if(j < idx)
                    {
                        nackLoss.loss3 = urecvd[j]; j++;
                    }
                    if(j < idx)
                    {
                        nackLoss.loss4 = urecvd[j]; j++;
                    }
                    nackLoss.send_time = q->frame_timestamp & 0xFFFFFFFF;
                    nackLoss.chanId = obj->sessionInfo->chanId + 1;
                    push_rtx_info(obj->netInfoObj, &nackLoss);
                }
            }
            free(urecvd);
            free(useqnums);
        }
        else{
        }
    }
    q->has_rtx = 1;
    return ret;
}
static void * JitterPopData(SocketObj *sock)
{
    void *ret = NULL;
    int lossless = 0;
    int nack_time = 50;
    int max_delay_time = nack_time << 1;

    pthread_mutex_lock(&sock->status_lock);
    SessionObj * session = (SessionObj *)sock->session;
    CallCodecAudio *obj = NULL;
    if(session)
    {
        obj = session->codec;
        if(obj)
        {
            CodecInfoObj *config = (CodecInfoObj *)&obj->codecInfo;
            lossless = config->lossless;
            int rtt = obj->delay_time << 1;//网络往返时间
            nack_time = rtt > 25 ? rtt : 25;
            max_delay_time = nack_time << 1;//留出重传的时间;
            //需要进行播放平滑处理
        }
    }
    //max_delay_time = 100;//test
    pthread_mutex_unlock(&sock->status_lock);

    pthread_mutex_lock(&sock->lock);
    FrameJitterNode *head, *p, *q;
    head = (FrameJitterNode *)sock->frameJitterHead;
    if(head)
    {
        q = head->next;
        if(q == NULL || q == head)
        {

        }
        else{
            int flag = 0;
            if(head->num > 0 || q->pkt_num >= q->rtp_pkt_num)
            {
                int64_t now_time = get_sys_time();
                int delay_time0 = now_time - head->last_frame_time;//q->frame_timestamp;
                p = head->tail;
                int delay_time = p->frame_timestamp - q->frame_timestamp;

                //if((delay_time0 > max_delay_time || delay_time > max_delay_time) && head->num > 2)
                if((delay_time0 > max_delay_time || delay_time > max_delay_time) && head->num > 1)
                {
#ifdef SHOW_DELAY
                    MYPRINT("audio: JitterPopData: delay_time0=%d \n", delay_time0);
                    MYPRINT("audio: JitterPopData: delay_time=%d \n", delay_time);
                    MYPRINT("audio: JitterPopData: q->pkt_num=%d \n", q->pkt_num);
                    MYPRINT("audio: JitterPopData: q->rtp_pkt_num=%d \n", q->rtp_pkt_num);
                    MYPRINT("audio: JitterPopData: head->num=%d \n", head->num);
#endif
                    head->last_frame_time = q->frame_timestamp;
                    head->ref_idx = q->ref_idx;
                    head->num--;
                    flag = 1;
                }
                else if(q->pkt_num >= q->rtp_pkt_num)
                {
                    if(q->max_pkt_num > 255 && (q->pkt_num - q->rtp_pkt_num) < 2)
                    {
                        //test
                        head->last_frame_time = q->frame_timestamp;
                        head->ref_idx = q->ref_idx;
                        head->num--;
                        flag = 1;
                    }
                    else{
                        head->last_frame_time = q->frame_timestamp;
                        head->ref_idx = q->ref_idx;
                        head->num--;
                        flag = 1;
                    }
                }
                else if(delay_time0 > nack_time && delay_time > nack_time)
                {
                    //nack
                    if(!lossless && OPEN_RTX)
                    {
                        generate_nack(sock, obj);
                    }
                }
                else{
#if 0
                    RtpInfo info = {0};
                    info.raw_offset = -1;
                    int head_size = sizeof(AVData) - MAX_DATA_SIZE;
                    int i = q->min_seqnum % q->max_pkt_num;
                    SeqFrame *thisSeqFrame = &q->seqFrame[i];
                    AVData *udata = (AVData *)thisSeqFrame->data;
                    int raw_size = udata->head.size - head_size;
                    int is_hcsvc_rtp = GetRtpInfo((uint8_t*)udata->data, raw_size, &info, AAC_PLT);
                    if(!is_hcsvc_rtp)
                    {

                    }
#endif
                }
            }
            if(flag)
            {
                ret = q;
                head->next = head->next->next;//delete this node
                //MYPRINT("JitterPopData: q->idx=%d \n", q->idx);
                if(head->next == NULL)
                {
                    //MYPRINT("JitterPopData: head->next is null \n");
                    head->next = NULL;  //头节点指针域置NULL
                    head->tail = head;
                }
            }
        }
    }
    pthread_mutex_unlock(&sock->lock);
    return ret;
}
#if 0
void * JitterPopDataAll(SocketObj *obj)
{
    //减少临界阻塞
    //当发包和收包速率相当时,累积包会以2倍的指数增长,即延迟会越来越大;
    //因此,实际的最大收包速率是其上限的二分之一;
    void *ret = NULL;
    pthread_mutex_lock(&obj->lock);
    ret = obj->frameJitterHead;
    obj->frameJitterHead = NULL;
    pthread_mutex_unlock(&obj->lock);
    return ret;
}
#endif

static uint8_t * distill_frame(SocketObj *sock, FrameJitterNode *thisFrame, int *out_frame_size)
{
    uint8_t *ret = NULL;
    pthread_mutex_lock(&sock->status_lock);
    SessionObj * session = (SessionObj *)sock->session;
    CallCodecAudio *obj = session->codec;
    pthread_mutex_unlock(&sock->status_lock);
    //session->sessionInfo
    if(obj)
    {
        if(!obj->outfp)
        {
            const char *filename = "/home/gxh/works/gxhshare/test.h264";
            obj->outfp = fopen(filename, "wb");
        }
        CodecInfoObj *config = (CodecInfoObj *)&obj->codecInfo;
        if(!config->lossless)
        {
            int max_pkt_num = thisFrame->max_pkt_num;
            int rtpNum = thisFrame->pkt_num;
            int frame_size = thisFrame->frame_size;
            int pkt_sum_size = thisFrame->pkt_sum_size;
            //MYPRINT("distill_frame: max_pkt_num=%d, rtpNum=%d \n", max_pkt_num, rtpNum);
            int *rtpSize = (int *)calloc(1, rtpNum * sizeof(int));
            uint8_t *pktbuf = (uint8_t *)calloc(1, pkt_sum_size * sizeof(uint8_t));
            //
            int offset = 0;
            int idx = 0;
            if(false)
            {
                MYPRINT("distill_frame: thisFrame->idx=%d \n", thisFrame->idx);
                MYPRINT("distill_frame: thisFrame->min_seqnum=%d \n", thisFrame->min_seqnum);
                MYPRINT("distill_frame: thisFrame->max_seqnum=%d \n", thisFrame->max_seqnum);
                MYPRINT("distill_frame: thisFrame->max_pkt_num=%d \n", thisFrame->max_pkt_num);
                MYPRINT("distill_frame: thisFrame->pkt_num=%d \n", thisFrame->pkt_num);
                MYPRINT("distill_frame: thisFrame->seqFrame=%x \n", thisFrame->seqFrame);
            }
            if(!thisFrame->max_pkt_num)
            {
                MYPRINT("distill_frame: thisFrame->max_pkt_num=%d \n", thisFrame->max_pkt_num);
            }
            int head_size = sizeof(AVData) - MAX_DATA_SIZE;
            for(int i = thisFrame->min_seqnum; i <= thisFrame->max_seqnum; i++)
            {
                int j = i % thisFrame->max_pkt_num;
                SeqFrame *thisSeqFrame = &thisFrame->seqFrame[j];
                if(!thisSeqFrame)
                {
                    MYPRINT("error: distill_frame: thisFrame->min_seqnum=%d \n", thisFrame->min_seqnum);
                    MYPRINT("error: distill_frame: thisFrame->max_seqnum=%d \n", thisFrame->max_seqnum);
                }

                uint8_t *recv_buf = thisSeqFrame->data;
                if(recv_buf)
                {
                    short size0 = thisSeqFrame->size;
                    //MYPRINT("distill_frame: thisSeqFrame->size=%d \n", thisSeqFrame->size);
                    AVData *udata = (AVData *)recv_buf;
                    int64_t now_time = udata->now_time;
                    int seqnum = udata->seqnum;
                    int size = udata->head.size;
                    uint8_t *data = (uint8_t *)udata->data;
                    if(size0 != size || seqnum != i)
                    {
                        MYPRINT("warning: distill_frame: size0=%d, size=%d \n", size0, size);
                        MYPRINT("warning: distill_frame: seqnum=%d, i=%d \n", seqnum, i);
                        MYPRINT("warning: distill_frame: thisFrame->min_seqnum=%d \n", thisFrame->min_seqnum);
                        MYPRINT("warning: distill_frame: thisFrame->max_seqnum=%d \n", thisFrame->max_seqnum);
                        MYPRINT("warning: distill_frame: thisFrame->max_pkt_num=%d \n", thisFrame->max_pkt_num);
                    }
                    int raw_size = size - head_size;
                    memcpy(&pktbuf[offset], data, raw_size);
                    rtpSize[idx] = raw_size;
                    offset += raw_size;
                    idx += 1;
                    if(offset > pkt_sum_size)
                    {
                        MYPRINT("error: distill_frame: offset=%d, pkt_sum_size=%d \n", offset, pkt_sum_size);
                    }
                    //MYPRINT("distill_frame: offset=%d \n", offset);
                    //MYPRINT("distill_frame: offset=%d, pkt_sum_size=%d \n", offset, pkt_sum_size);
                    free(thisSeqFrame->data);
                    thisSeqFrame->data = NULL;
                    thisSeqFrame->size = 0;
                }
                else{
                    //offset += MAX_DATA_SIZE;
                    //MYPRINT("distill_frame: lost : i=%d \n", i);
                }
            }
            free(thisFrame->seqFrame);
            free(thisFrame);
            thisFrame = NULL;
            //MYPRINT("distill_frame: offset=%d \n", offset);
            //MYPRINT("distill_frame: idx=%d \n", idx);

            int ret2 = 0;
            int enable_fec = 0;
            uint8_t *src = pktbuf;
            uint8_t *dst = NULL;
            obj->json = api_renew_json_int(obj->json, "mtu_size", MTU_SIZE);
            RtpInfo info = {0};
            info.raw_offset = -1;
            int is_hcsvc_rtp = GetRtpInfo((uint8_t*)pktbuf, rtpSize[0], &info, AAC_PLT);
            enable_fec = info.enable_fec;
            if(enable_fec)
            {
                //MYPRINT("distill_frame: enable_fec=%d \n", enable_fec);
                //fecdec
                dst = (uint8_t *)calloc(1, pkt_sum_size * sizeof(uint8_t));
                obj->json = api_delete_item(obj->json, "insize");
                obj->json = api_renew_json_array(obj->json, "inSize", rtpSize, rtpNum);
                char *jsonstr = api_json2str(obj->json);
                //MYPRINT("distill_frame: 0: jsonstr=%s \n", jsonstr);
                ret2 = api_fec_decode(  obj->handle,
                                        src,
                                        jsonstr,
                                        dst,
                                        obj->outparam);
                obj->json = api_delete_item(obj->json, "inSize");
                api_get_array_free(rtpSize);
                api_json2str_free(jsonstr);
                rtpNum = 0;
                char *paramstr = (char *)obj->outparam[0];
                char *key = "rtpSize";
                rtpSize = api_get_array_int(paramstr, key, &rtpNum);
                pktbuf = NULL;
                free(src);
                src = dst;
                //MYPRINT("distill_frame: rtpNum=%d \n", rtpNum);
            }
            //pkt2raw
            if((!enable_fec || ret2 > 0))
            {
                dst = (uint8_t *)calloc(1, pkt_sum_size * sizeof(uint8_t));
                obj->json = api_renew_json_int(obj->json, "insize", rtpSize[0]);
                //obj->json = api_renew_json_array(obj->json, "rtpSize", rtpSize, rtpNum);
                char *jsonstr = api_json2str(obj->json);
                //MYPRINT("distill_frame: 1: jsonstr=%s \n", jsonstr);
                int ret2 = api_audio_rtp_packet2raw(obj->handle,
                                                    src,
                                                    jsonstr,
                                                    dst,
                                                    obj->outparam);
                //MYPRINT("distill_frame: 1: ret2=%d \n", ret2);
                //obj->json = api_delete_item(obj->json, "rtpSize");
                //api_get_array_free(rtpSize);
                obj->json = api_delete_item(obj->json, "insize");
                api_json2str_free(jsonstr);
                pktbuf = NULL;
                rtpSize = NULL;
                free(src);
                //MYPRINT("distill_frame: 2: ret2=%d \n", ret2);
                if(ret2 > 0)
                {
                    if ( obj->outfp != NULL)
                    {
                        fwrite(dst, 1, ret2, obj->outfp);
                    }
                    src = dst;
                    dst = (uint8_t *)malloc(frame_size * sizeof(uint8_t));

                    obj->json = api_renew_json_int(obj->json , "insize", ret2);
                    if(obj->bitrate > 0)
                    {
                        obj->json = api_renew_json_int(obj->json, "down_bitrate", obj->bitrate);
                    }
                    jsonstr = api_json2str(obj->json);
                    //MYPRINT("distill_frame: 2: jsonstr=%s \n", jsonstr);
                    //obj->outparam[0] = "";
                    //obj->outparam[1] = "";
                    //obj->outparam[2] = "";
                    ret2 = api_audio_codec_one_frame(   obj->handle,
                                                        src,
                                                        jsonstr,
                                                        dst,
                                                        obj->outparam);
                    //MYPRINT("distill_frame: 3: ret2=%d \n", ret2);
                    //obj->json = api_delete_item(obj->json, "insize");
                    api_json2str_free(jsonstr);
                    free(src);
                    if(ret2 <= 0)
                    {
                        free(dst);
                        dst = NULL;
                    }
                    else{
                        out_frame_size[0] = ret2;
                        ret = dst;
                    }
                    //MYPRINT("distill_frame: 4: ret2=%d \n", ret2);
                }
                else{
                    MYPRINT("distill_frame: lost frame: ret2=%d \n", ret2);
                    free(dst);
                }
            }
            if(rtpSize)
            {
                api_get_array_free(rtpSize);
            }
            if(pktbuf)
            {
                free(pktbuf);
            }
        }
        else{
            //lossless mode
            ret = (uint8_t *)calloc(1, thisFrame->frame_size * sizeof(uint8_t));
            int offset = 0;
            if(false)
            {
                MYPRINT("distill_frame: thisFrame->idx=%d \n", thisFrame->idx);
                MYPRINT("distill_frame: thisFrame->min_seqnum=%d \n", thisFrame->min_seqnum);
                MYPRINT("distill_frame: thisFrame->max_seqnum=%d \n", thisFrame->max_seqnum);
                MYPRINT("distill_frame: thisFrame->max_pkt_num=%d \n", thisFrame->max_pkt_num);
                MYPRINT("distill_frame: thisFrame->pkt_num=%d \n", thisFrame->pkt_num);
                MYPRINT("distill_frame: thisFrame->seqFrame=%x \n", thisFrame->seqFrame);
            }
            if(!thisFrame->max_pkt_num)
            {
                MYPRINT("distill_frame: thisFrame->max_pkt_num=%d \n", thisFrame->max_pkt_num);
            }
            int head_size = sizeof(AVData) - MAX_DATA_SIZE;
            for(int i = thisFrame->min_seqnum; i <= thisFrame->max_seqnum; i++)
            {
                int j = i % thisFrame->max_pkt_num;
                SeqFrame *thisSeqFrame = &thisFrame->seqFrame[j];
                if(!thisSeqFrame)
                {
                    MYPRINT("error: distill_frame: thisFrame->min_seqnum=%d \n", thisFrame->min_seqnum);
                    MYPRINT("error: distill_frame: thisFrame->max_seqnum=%d \n", thisFrame->max_seqnum);
                }

                uint8_t *recv_buf = thisSeqFrame->data;
                if(recv_buf)
                {
                    short size0 = thisSeqFrame->size;
                    //MYPRINT("distill_frame: thisSeqFrame->size=%d \n", thisSeqFrame->size);
                    UserData *udata = (UserData *)recv_buf;
                    int64_t now_time = udata->data0.now_time;
                    int seqnum = udata->data0.seqnum;
                    int size = udata->data0.head.size;
                    uint8_t *data = (uint8_t *)udata->data0.data;
                    if(size0 != size || seqnum != i)
                    {
                        MYPRINT("warning: distill_frame: size0=%d, size=%d \n", size0, size);
                        MYPRINT("warning: distill_frame: seqnum=%d, i=%d \n", seqnum, i);
                        MYPRINT("warning: distill_frame: thisFrame->min_seqnum=%d \n", thisFrame->min_seqnum);
                        MYPRINT("warning: distill_frame: thisFrame->max_seqnum=%d \n", thisFrame->max_seqnum);
                        MYPRINT("warning: distill_frame: thisFrame->max_pkt_num=%d \n", thisFrame->max_pkt_num);
                    }
                    int raw_size = size - head_size;
                    memcpy(&ret[offset], data, raw_size);
                    offset += raw_size;
                    //MYPRINT("distill_frame: offset=%d \n", offset);
                    free(thisSeqFrame->data);
                    thisSeqFrame->data = NULL;
                    thisSeqFrame->size = 0;
                }
                else{
                    offset += MAX_DATA_SIZE;
                }
            }
            free(thisFrame->seqFrame);
            free(thisFrame);
        }
    }
    return ret;
}

//纯数据处理，此处是瓶颈
static int pkt_resort(SocketObj *obj, int *decodeNum)
{
    int ret = 0;
    DataNode *head = (DataNode *)PopDataAll(obj);
    struct sockaddr_in addr_client;
    if(head && head->num)
    {
        //MYPRINT("pkt_resort: head->num=%d \n", head->num);
        do{
            DataNode *q;
            q = head->next;
            if(q == NULL || q == head)
            {
                break;
            }
            else{
                head->next = head->next->next;
                if(head->next == NULL)
                {
                    head->next = NULL;  //头节点指针域置NULL
                    head->tail = head;
                }
            }
            //
#if 1
            //resort and get frame here
            uint8_t *recv_buf = (uint8_t *)q->data;
            int64_t recv_time = q->now_time;
            addr_client = q->addr_client;
            //
            UserData *udata = (UserData *)recv_buf;
            if(udata->data0.head.datatype == kDATA)
            {
                int64_t now_time = udata->data0.now_time;
                int seqnum = udata->data0.seqnum;
                int size = udata->data0.head.size;
                uint8_t *data = (uint8_t *)recv_buf;//udata->data0.data;
                int ret2 = JitterPushData(obj, data, size, seqnum, now_time);
                if(ret2 < 0)
                {
                    //MYPRINT("pkt_resort: ret2=%d \n", ret2);
                    free(q->data);
                }
                FrameJitterNode *thisFrame = (FrameJitterNode *)JitterPopData(obj);
                if(thisFrame)
                {
                    int pkt_sum_size = thisFrame->pkt_sum_size;
                    pthread_mutex_lock(&obj->status_lock);
                    SessionObj *session = (SessionObj *)obj->session;
                    CallPlayer *player = session->render;
                    pthread_mutex_unlock(&obj->status_lock);
                    if(session)
                    {
                        int chanId = session->sessionInfo->chanId;
                        int frame_size = 0;
                        uint8_t *frame_data = distill_frame(obj, thisFrame, &frame_size);
                        if(frame_data)
                        {
                            ret += pkt_sum_size;
                            //MYPRINT("pkt_resort: pkt_sum_size=%d \n", pkt_sum_size);
                            decodeNum[0] += 1;
                            //MYPRINT("pkt_resort: session->render=%x \n", session->render);
                            if(player)
                            {
                                int selfmode = session->sessionInfo->selfmode;
                                int chanId = session->sessionInfo->chanId;
                                int actor = session->sessionInfo->actor;
                                if(selfmode)
                                {
                                    chanId = (actor < DECACTOR) ? 1 : 0;
                                }
                                //MYPRINT("pkt_resort: chanId=%d \n", chanId);
                                AudioFrameBufferPushData(session->render, frame_data, frame_size, chanId, thisFrame->frame_timestamp, thisFrame->now_time);
                            }
                            else{
                                //MYPRINT("pkt_resort: frame_data=%x \n", frame_data);
                                free(frame_data);
                                frame_data = NULL;
                                //MYPRINT("pkt_resort: 2: frame_data=%x \n", frame_data);
                            }
                        }
                    }
                    else{
                        if(thisFrame->seqFrame)
                        {
                            free(thisFrame->seqFrame);
                        }
                        free(thisFrame);
                    }
                }
            }
            else{
                //send_num = ProcessCmd(obj, send_buf, send_num, addr_client, sock_fd, len, now_time);
                MYPRINT("pkt_resort: head->num=%d \n", head->num);
                MYPRINT("pkt_resort: q->size=%d \n", q->size);
                MYPRINT("pkt_resort: q=%x \n", q);
                MYPRINT("pkt_resort: cmd \n");
                free(q->data);
            }
#else
            free(q->data);
#endif
            obj->pkt_send_num++;
            //free(q->data);
            free(q);   //释放节点i的内存单元
        }while(1);
        free(head);
    }
    else{
        usleep(1000);//1ms
    }
    return ret;
}
static int decode_frame_run(SocketObj *sock)
{
    int ret = 0;

    pthread_mutex_lock(&sock->status_lock);
    sock->send_status = 1;
    int status = sock->send_status;
    SessionObj * session = (SessionObj *)sock->session;
    CallCodecAudio *obj = session->codec;
    pthread_mutex_unlock(&sock->status_lock);

    int frame_num = 0;
    int64_t sumbytes = 0;
    int frame_size = 8192;
#ifndef SIMULATOR_DATA
    //WaitRender(sock);
#endif
    int64_t time0 = get_sys_time();
    while(status > 0)
    {
        int64_t now_time = get_sys_time();

        int decodeNum = 0;
        ret = pkt_resort(sock, &decodeNum);
        //usleep(100000);

        int difftime = (int)(now_time - time0);

        frame_num += decodeNum;
        sumbytes += ret;
        if(difftime > MAX_INTERVAL_CLIENT)
        {
            //注意:一旦编码速率超出了解码速率的能力,超时判断会严重制约解码速率,导致解码速率下降梯度非线性增长;
            float framerate = (float)((frame_num * 1000.0) / difftime);
            //sumbytes = frame_size * frame_num;
            float bitrate = (float)(((sumbytes << 3) * 1.0) / (difftime * 1000));
            if(obj)
            {
                obj->bitrate = (int)(((sumbytes << 3) * 1) / (difftime)); //kbps
                obj->framerate = (int)((frame_num * 1000) / difftime);//fps
            }
            time0 = now_time;
            frame_num = 0;
            sumbytes = 0;
            MYPRINT("audio decode_frame_run: framerate=%2.1f (fps) \n", framerate);
            MYPRINT("audio decode_frame_run: bitrate=%5.2f (Mbps) \n", bitrate);
            //MYPRINT("decode_frame_run: sock->recv_status=%d \n", sock->recv_status);
        }

        pthread_mutex_lock(&sock->status_lock);
        status = (sock->send_status > 0) & (sock->recv_status > 0);
        pthread_mutex_unlock(&sock->status_lock);
    }
    ExitSend(sock);
    StopRecv(sock);
    WaitRecv(sock);//停止收包
    MYPRINT("decode_frame_run: over \n");
    return ret;
}

int audio_set_capture(SocketObj *obj, CallCapture *capture, int capture_status)
{
    int ret = 0;
    pthread_mutex_lock(&obj->status_lock);
    if(capture_status < 0)
    {
        MYPRINT("audio_set_capture: capture_status=%d \n", capture_status);
        obj->recv_status = 0;
        pthread_mutex_unlock(&obj->status_lock);
        return -1;
    }
    else if(!obj->session)
    {
        //obj->recv_status = 0;
        MYPRINT("audio_set_capture: obj->session=%x \n",obj->session);
        pthread_mutex_unlock(&obj->status_lock);
        return -1;
    }
    else{
        SessionObj * session = (SessionObj *)obj->session;
        session->capture = capture;
        pthread_mutex_unlock(&obj->status_lock);
        MYPRINT("audio_set_capture: obj=%x \n",obj);
        MYPRINT("audio_set_capture: session=%x \n",session);
        MYPRINT("audio_set_capture: capture=%x \n", capture);
    }
    return ret;
}
int audio_set_player(SocketObj *obj, CallPlayer *player)
{
    int ret = 0;
    pthread_mutex_lock(&obj->status_lock);
    if(!obj->session)
    {
        pthread_mutex_unlock(&obj->status_lock);
        ret = -1;
    }
    else{
        SessionObj * session = (SessionObj *)obj->session;
        session->render = player;
        if(session->sessionInfo)
        {
            int selfmode = session->sessionInfo->selfmode;
            int actor = session->sessionInfo->actor;
            int modeId = session->sessionInfo->modeId;
            int chanId = session->sessionInfo->chanId;
            //int width = session->sessionInfo->width;
            //int height = session->sessionInfo->height;
            chanId = (actor < DECACTOR) ? 1 : 0;
            //SDL_Rect rect = RectMap(modeId, chanId, width, height);
            //AddRect(render, rect, chanId);
        }
    }
    pthread_mutex_unlock(&obj->status_lock);

    return ret;
}
int audio_set_netinfo(SocketObj *obj, NetInfoObj *netInfoObj)
{
    int ret = 0;
    pthread_mutex_lock(&obj->status_lock);
    if(!obj->session)
    {
        pthread_mutex_unlock(&obj->status_lock);
        return -1;
    }
    else{
        SessionObj * session = (SessionObj *)obj->session;
        session->netInfoObj = netInfoObj;
        MYPRINT("audio_set_netinfo: session->codec=%x \n", session->codec);
        if(session->codec)
        {
            if(session->codec->encoder)
            {
                AddEncoder(&netInfoObj->lock, &netInfoObj->encoderHead, session->codec->encoder);
            }
            session->codec->netInfoObj = netInfoObj;
            MYPRINT("audio_set_netinfo: session->codec->netInfoObj=%x \n", session->codec->netInfoObj);
        }
    }
    pthread_mutex_unlock(&obj->status_lock);
    return ret;
}
int audio_set_readstream(SocketObj *sock, char *handle)
{
    int ret = 0;
    pthread_mutex_lock(&sock->status_lock);
    SessionObj * session = (SessionObj *)sock->session;
    CallCodecAudio *obj = (CallCodecAudio *)session->codec;
    memcpy(session->handle, handle, 8);
    pthread_mutex_unlock(&sock->status_lock);
    return ret;
}
int audio_set_stream(SocketObj *sock, char *handle, int is_encoder)
{
    int ret = 0;
    pthread_mutex_lock(&sock->status_lock);
    SessionObj * session = (SessionObj *)sock->session;
    CallCodecAudio *obj = (CallCodecAudio *)session->codec;
    set_avstream2audio(obj->handle, handle);
    pthread_mutex_unlock(&sock->status_lock);
    return ret;
}
static int audio_set_codec(SocketObj *sock, SessionInfoObj *sessionInfo, int is_encoder)
{
    int ret = 0;
    pthread_mutex_lock(&sock->status_lock);
    SessionObj * session = (SessionObj *)sock->session;
    CallCodecAudio *obj = (CallCodecAudio *)calloc(1, sizeof(CallCodecAudio));
    obj->sock = (void *)sock;
    //
    MYPRINT("audio_set_codec: %d \n", is_encoder);
    pthread_mutex_init(&obj->lock,NULL);

    obj->sessionInfo = sessionInfo;

    char *params = "{\"print\":0}";
    obj->json = (cJSON *)api_str2json(params);
    //int w = DEFAULT_WIDTH;
    //int h = DEFAULT_HEIGHT;
    obj->frame_size = 8192;
    CodecInfoObj *config = (CodecInfoObj *)&obj->codecInfo;
    config->lossless = LOSSLESS;
    if(is_encoder)
    {
        config->loss_rate = sessionInfo->lossrate;
        config->enable_fec = 0;
        config->loss_rate = 0;//0.9;//0.05;
        config->code_rate = (1.0 - config->loss_rate);
        config->bit_rate = 24000;
        config->mtu_size = MTU_SIZE;
        config->adapt_cpu = 0;
        config->main_spatial_idx = 0;
        config->fec_level = 2;
        config->enable_netack = sessionInfo->enable_netack;

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
        api_audio_codec_init(obj->handle, jsonStr);

        for(int l = 0; l < 4; l++)
        {
            obj->outparam[l] = NULL;
        }
        obj->outbuffer = malloc(obj->frame_size * sizeof(uint8_t));
        obj->rtpbuffer = malloc(obj->frame_size * sizeof(uint8_t));

        //obj->nettime = 0;

        obj->encoder = calloc(1, sizeof(Encoder));
        obj->encoder->avtype = kIsAudio;
        obj->encoder->selfChanId = sessionInfo->chanId;
        obj->encoder->selfLossRate = 0;
        obj->encoder->selfMaxLossRate = 0;
        for(int i = 0; i < 4; i++)
        {
            obj->encoder->initBitRate[i] = config->bit_rate >> i;// = {0, 0, 0, 0};
        }
        obj->encoder->bitRate = config->bit_rate;
        obj->encoder->codec = (void *)obj;
    }
    else{
        config->enable_fec = 0;
        config->mtu_size = MTU_SIZE;
        config->adapt_cpu = 0;
        config->main_spatial_idx = 0;
        config->fec_level = 2;
        config->enable_netack = sessionInfo->enable_netack;

        obj->json = api_renew_json_int(obj->json , "codec_mode", 2);//2
        obj->delay_time = 100;
        obj->json = api_renew_json_int(obj->json , "codec_mode", 0);//2
        obj->json = api_renew_json_int(obj->json , "seqnum", obj->seqnum);
        obj->json = api_renew_json_int(obj->json , "enable_fec", config->enable_fec);
        obj->json = api_renew_json_float(obj->json , "loss_rate", config->loss_rate);
        obj->json = api_renew_json_float(obj->json , "code_rate", config->code_rate);
        obj->json = api_renew_json_int(obj->json , "codec_type", 1);
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
        api_audio_codec_init(obj->handle, jsonStr);
        for(int l = 0; l < 4; l++)
        {
            obj->outparam[l] = NULL;
        }
    }
    //
    session->codec = obj;
    pthread_mutex_unlock(&sock->status_lock);
    MYPRINT("audio_set_codec: ok \n");
    return ret;
}

void audio_close_codec(SocketObj *sock, int is_encoder)
{
    pthread_mutex_lock(&sock->status_lock);
    SessionObj * session = (SessionObj *)sock->session;
    CallCodecAudio *obj = session->codec;
    pthread_mutex_unlock(&sock->status_lock);

    if(obj)
    {
        if(obj->rtxHead)
        {
            //release
            RtxNode *head = (RtxNode *)obj->rtxHead;
            if(head && head->num)
            {
                do{
                    RtxNode *q;
                    q = head->next;
                    if(q == NULL || q == head)
                    {
                        break;
                    }
                    else{
                        head->next = head->next->next;
                    }
                    //
                    free(q->datalist);
                    free(q);   //释放节点i的内存单元
                }while(1);
                free(head);
            }
            obj->selfNetInfoHead = NULL;
        }
        if(obj->selfNetInfoHead)
        {
            //release
            NetInfoNode *head = (NetInfoNode *)obj->selfNetInfoHead;
            if(head && head->num)
            {
                do{
                    NetInfoNode *q;
                    q = head->next;
                    if(q == NULL || q == head)
                    {
                        break;
                    }
                    else{
                        head->next = head->next->next;
                    }
                    //
                    free(q);   //释放节点i的内存单元
                }while(1);
                free(head);
            }
            obj->selfNetInfoHead = NULL;
        }
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

        free(obj);
    }
    pthread_mutex_lock(&sock->status_lock);
    session->codec = NULL;
    pthread_mutex_unlock(&sock->status_lock);

}

//socket send
void * audio_encode(SocketObj *obj)
{
    MYPRINT("audio_encode: 0 \n");
    int ret = 0;
    obj->status = 1;
    cJSON * json = (cJSON *)api_str2json(obj->params);
    obj->port = GetvalueInt(json, "port");
    GetvalueStr(json, "server_ip", obj->server_ip);
    int taskId = obj->id;
    /* socket文件描述符 */
    SOCKFD sock_fd;

    /* 建立udp socket */
    obj->sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(obj->sock_fd < 0)
    {
        perror("audio_encode: socket");
        //exit(1);
        return 0;
    }
    //超时时间
#ifdef _WIN32
    int timeout = 500;//ms
#else
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 500000;//500ms
#endif
	socklen_t len = sizeof(timeout);
	ret = setsockopt(obj->sock_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, len);
	if (ret == -1) {
		perror("audio_encode error:");
		return 0;
	}
	ret = setsockopt(obj->sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, len);
	if (ret == -1) {
	    perror("audio_encode error:");
	    return 0;
	}
    /* 设置address */
    //struct sockaddr_in addr_serv;
    memset(&obj->addr_serv, 0, sizeof(obj->addr_serv));
    obj->addr_serv.sin_family = AF_INET;
    obj->addr_serv.sin_addr.s_addr = inet_addr(obj->server_ip);
    obj->addr_serv.sin_port = htons(obj->port);
    //

    //===
    MYPRINT("audio_encode: 1 \n");
    pthread_mutex_init(&obj->lock,NULL);
    pthread_mutex_init(&obj->status_lock,NULL);

    pthread_mutex_lock(&obj->status_lock);
    if(!obj->session)
    {
        SessionObj * session = (SessionObj *)calloc(1, sizeof(SessionObj));
        obj->session = (void *)session;
    }
    SessionObj * session = (SessionObj *)obj->session;
    SessionInfoObj *config = (SessionInfoObj *)calloc(1, sizeof(SessionInfoObj));
    //cJSON * json = (cJSON *)api_str2json(obj->params);
    MYPRINT("audio_encode: obj->params=%s \n", obj->params);
    config->chanId = GetvalueInt(json, "chanId");
    config->actor = GetvalueInt(json, "actor");
    config->sessionId = GetvalueInt(json, "sessionId");
    config->idx = GetvalueInt(json, "idx");
    config->modeId = GetvalueInt(json, "modeId");
    config->avtype = GetvalueInt(json, "avtype");
    config->selfmode = GetvalueInt(json, "selfmode");
    config->testmode = GetvalueInt(json, "testmode");
    config->nettime = GetvalueInt(json, "nettime");
    //config->width = GetvalueInt(json, "width");
    //config->height = GetvalueInt(json, "height");
    config->enable_netack = GetvalueInt(json, "enable_netack");
    config->status = GetvalueInt(json, "status");
    GetvalueStr(json, "yuvfilename", config->yuvfilename);
    config->lossrate = GetvalueFloat(json, "lossrate");
    ///config->streamName = GetvalueStr(json, "pcmfp");
    //MYPRINT("audio_encode: config->streamName=%s \n", config->streamName);
    //int w = config->width;
    //int h = config->height;
    //MYPRINT("audio_encode: w=%d, h=%d \n", w, h);
    session->sessionInfo = config;
    pthread_mutex_unlock(&obj->status_lock);

    int chanId = config->chanId;
    //===
    UserData udata;
    udata.data1.head.datatype = kCMD;
    int size = sizeof(CmdData);
    udata.data1.head.size = size;
    udata.data1.cmdtype = (CMDType)kReg;//kReg;//kExit;kGetSessionId,kGetChanId,kBye,kHeartBeat,kRTT,kBroadCast,kExit,
    udata.data1.status = 1;
    udata.data1.chanId = config->chanId;
    udata.data1.avtype = config->avtype;
    udata.data1.selfmode = config->selfmode;
    udata.data1.testmode = config->testmode;
    udata.data1.modeId = config->modeId;
    udata.data1.actor = config->actor;
    udata.data1.sessionId = config->sessionId;
    ret = client_broadcast_get(obj, &udata);
    if(ret < 0)
    {
        pthread_mutex_lock(&obj->status_lock);
        if(obj->session)
        {
            SessionObj * session = (SessionObj *)obj->session;
            if(config)
            {
                free(config);
            }
            free(obj->session);
        }
        pthread_mutex_unlock(&obj->status_lock);

        pthread_mutex_destroy(&obj->lock);
        pthread_mutex_destroy(&obj->status_lock);
        MYPRINT("error: audio_encode register fail: ret=%d \n", ret);
        return 0;
    }
    //
    audio_set_codec(obj, config, 1);
    //
    //pthread_t tid;
    //if(pthread_create(&tid, NULL, audio_codec_enc_run, obj) < 0)
    //{
    //    MYPRINT("server_main: Create audio_codec_enc_run failed!\n");
    //}

    StartSend(obj);
    pthread_t tid2;
    if(pthread_create(&tid2, NULL, client_send_run, obj) < 0)
    {
        MYPRINT("audio_encode: Create client_send_run failed!\n");
        //exit(0);
    }

    encode_frame_run(obj);
    //StopRecv(obj);

    //char *p0;
    //if (pthread_join(tid, (void**)&p0))
    //{
    //    MYPRINT("audio_encode: audio_codec_enc_run thread is not exit...\n");
    //}
    //else{
    //    MYPRINT("audio_encode: p0=%s \n", p0);
    //    free(p0);
    //}

    char *p1;
    if (pthread_join(tid2, (void**)&p1))
    {
        MYPRINT("audio_encode: client_send_run thread is not exit...\n");
    }
    else{
        MYPRINT("audio_encode: p1=%s \n", p1);
        free(p1);
    }
    audio_close_codec(obj, 1);

#if defined (WIN32)
    closesocket(obj->sock_fd);
#elif defined(__linux__)
    close(obj->sock_fd);
#endif

    pthread_mutex_lock(&obj->status_lock);
    if(obj->session)
    {
        SessionObj * session = (SessionObj *)obj->session;
        if(config)
        {
            free(config);
        }
        free(obj->session);
    }
    pthread_mutex_unlock(&obj->status_lock);

    pthread_mutex_destroy(&obj->lock);
    pthread_mutex_destroy(&obj->status_lock);
    obj->status = -1;
    MYPRINT("audio_encode: over: taskId=%d \n", taskId);
    return 0;
}

//socket recv
void * audio_decode(SocketObj *obj)
{
    MYPRINT("audio_decode: 0 \n");
    int ret = 0;
    obj->status = 1;
    cJSON * json = (cJSON *)api_str2json(obj->params);
    obj->port = GetvalueInt(json, "port");
    GetvalueStr(json, "server_ip", obj->server_ip);
    int taskId = obj->id;
    /* socket文件描述符 */
    SOCKFD sock_fd;

    /* 建立udp socket */
    obj->sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(obj->sock_fd < 0)
    {
        perror("audio_decode: socket");
        //exit(1);
        return 0;
    }
    //超时时间
#ifdef _WIN32
    int timeout = 500;//ms
#else
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 500000;//500ms
#endif
	socklen_t len = sizeof(timeout);
	ret = setsockopt(obj->sock_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, len);
	if (ret == -1) {
		perror("audio_decode error:");
		return 0;
	}
	ret = setsockopt(obj->sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, len);
	if (ret == -1) {
	    perror("audio_decode error:");
	    return 0;
	}
    /* 设置address */
    //struct sockaddr_in addr_serv;
    memset(&obj->addr_serv, 0, sizeof(obj->addr_serv));
    obj->addr_serv.sin_family = AF_INET;
    obj->addr_serv.sin_addr.s_addr = inet_addr(obj->server_ip);
    obj->addr_serv.sin_port = htons(obj->port);
    //===
    pthread_mutex_init(&obj->lock,NULL);
    pthread_mutex_init(&obj->status_lock,NULL);

    pthread_mutex_lock(&obj->status_lock);
    if(!obj->session)
    {
        SessionObj * session = (SessionObj *)calloc(1, sizeof(SessionObj));
        obj->session = (void *)session;
    }
    SessionObj * session = (SessionObj *)obj->session;
    SessionInfoObj *config = (SessionInfoObj *)calloc(1, sizeof(SessionInfoObj));
    //cJSON * json = (cJSON *)api_str2json(obj->params);
    config->chanId = GetvalueInt(json, "chanId");
    config->actor = GetvalueInt(json, "actor");
    config->sessionId = GetvalueInt(json, "sessionId");
    config->idx = GetvalueInt(json, "idx");
    config->modeId = GetvalueInt(json, "modeId");
    config->avtype = GetvalueInt(json, "avtype");
    config->selfmode = GetvalueInt(json, "selfmode");
    config->testmode = GetvalueInt(json, "testmode");
    config->nettime = GetvalueInt(json, "nettime");
    //config->width = GetvalueInt(json, "width");
    //config->height = GetvalueInt(json, "height");
    config->enable_netack = GetvalueInt(json, "enable_netack");
    config->status = GetvalueInt(json, "status");

    session->sessionInfo = config;
    pthread_mutex_unlock(&obj->status_lock);

    UserData udata;
    udata.data1.head.datatype = kCMD;
    int size = sizeof(CmdData);
    udata.data1.head.size = size;
    udata.data1.cmdtype = (CMDType)kReg;//kReg;//kExit;kGetSessionId,kGetChanId,kBye,kHeartBeat,kRTT,kBroadCast,kExit,
    udata.data1.status = 1;
    udata.data1.chanId = config->chanId;
    udata.data1.avtype = config->avtype;
    udata.data1.selfmode = config->selfmode;
    udata.data1.testmode = config->testmode;
    udata.data1.modeId = config->modeId;
    udata.data1.actor = config->actor;
    udata.data1.sessionId = config->sessionId;
    ret = client_broadcast_get(obj, &udata);
    if(ret < 0)
    {
        pthread_mutex_lock(&obj->status_lock);
        if(obj->session)
        {
            SessionObj * session = (SessionObj *)obj->session;
            if(config)
            {
                free(config);
            }
            free(obj->session);
        }
        pthread_mutex_unlock(&obj->status_lock);

        pthread_mutex_destroy(&obj->lock);
        pthread_mutex_destroy(&obj->status_lock);
        MYPRINT("error: audio_decode register fail: ret=%d \n", ret);
        return 0;
    }
    //===
    MYPRINT("audio_decode: 1 \n");
    audio_set_codec(obj, config, 0);
    //
    StartRecv(obj);
    pthread_t tid;
    if(pthread_create(&tid, NULL, client_recv_run, obj) < 0)
    {
        MYPRINT("audio_decode: Create client_recv_run failed!\n");
        //exit(0);
    }

    decode_frame_run(obj);
    //StopSend(obj);

    char *p0;
    if (pthread_join(tid, (void**)&p0))
    {
        MYPRINT("audio_decode: client_recv_run thread is not exit...\n");
    }
    else{
        MYPRINT("audio_decode: p0=%s \n", p0);
        free(p0);
    }
    audio_close_codec(obj, 0);

#if defined (WIN32)
    closesocket(obj->sock_fd);
#elif defined(__linux__)
    close(obj->sock_fd);
#endif
    pthread_mutex_lock(&obj->status_lock);
    if(obj->session)
    {
        SessionObj * session = (SessionObj *)obj->session;
        if(config)
        {
            free(config);
        }
        free(obj->session);
        obj->session = NULL;
    }
    pthread_mutex_unlock(&obj->status_lock);

    pthread_mutex_destroy(&obj->lock);
    pthread_mutex_destroy(&obj->status_lock);
    MYPRINT("audio_decode: over: taskId=%d \n", taskId);
    obj->status = -1;
    return 0;
}
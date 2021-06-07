#include "inc.h"
#include "udpbase.h"
//#include "x264.h"

extern float GetvalueFloat(cJSON *json, char *key);
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
extern int read_frame(CallCapture *obj, char *outbuf);
extern void stop_capture_run(CallCapture *obj);
extern int AddRect(CallRender *obj, SDL_Rect rect, int id);
extern void FramePushData(CallRender *obj, uint8_t *data, int width, int height, int id, int64_t frame_timestamp, int64_t now_time);
extern int AddEncoder(pthread_mutex_t *lock, EncoderNode **encoderHead, Encoder *encoder);
extern int32_t GetNetResult(CallCodecVideo *obj, int ref_idx);
extern int push_net_info(NetInfoObj *netInfoObj, int avtype,int chanId, uint8_t *data, int insize, int loss_rate);
extern int push_rtx_info(NetInfoObj *netInfoObj, NACK_LOSS *nackLoss);
extern int64_t get_sys_time2();
extern SDL_Rect RectMap(MultRect **pRect, int modeId, int id, int w, int h, int *num, int *maxLayerId);
extern void set_avstream2video(char *handle, char *handle2);
extern int control_max_bitrate(char *handle, int frame_idx, uint8_t *frame_data);

RtxNode * FindeRtxByTimestamp(pthread_mutex_t *lock, RtxNode *rtxHead, uint32_t send_time);

#if 0
int BRPushData(BitRateNode **brhead, int size, int new_frame, int64_t now_time)
{
    int ret = 0;
    BitRateNode *head,*pnew, *q;
    if(!*brhead)
    {
        //head不存数据,head->next指向第一个数据,head->num保存总索引（也可以设计将第一个节点存入head）
        //注意链表元的2个组成部分，元和next指针，元控制的是整个链表元的地址，next指针则寄生在元下；
        //MYPRINT("PushData: create head: obj->data_list=%x \n", obj->data_list);
        head = (BitRateNode *)calloc(1, sizeof(BitRateNode));  //创建头节点。
        head->num = 0;
        head->cur_bitrate = 0;
        head->cur_framerate = 0;
        head->frame_idx = 0;
        head->next = NULL;  //头节点指针域置NULL
        head->tail = head;  // 开始时尾指针指向头节点；此时head->tail->next与head->next是同一地址
        *brhead = (void *)head;
    }
    head = (BitRateNode *)(*brhead);
    if(head->num > 1)
    {
        BitRateNode *first_node = head->next;
        BitRateNode *last_node = head->tail;
        int difftime = (int)(last_node->frame_timestamp - first_node->frame_timestamp);
        int framenum = (int)(last_node->frame_idx - first_node->frame_idx);
        if(difftime >= 1000)
        {
            //
            int sumbits = 0;
            q = head->next;
            do{
                if(!q->next)
                {
                    break;
                }
                sumbits += q->size;
                q = q->next;
            }while(1);
            ret = sumbits;
            head->cur_bitrate = ret;
            head->cur_framerate = (int)((float)framenum / ((float)difftime / 1000.0) + 0.5);
            //
            q = head->next;
            head->next = head->next->next;
            head->num--;
            if(head->next == NULL)
            {
                head->next = NULL;  //头节点指针域置NULL
                head->tail = head;
            }
            free(q);
        }
    }
    {
        pnew = (BitRateNode *)calloc(1, sizeof(BitRateNode));  //创建新节点
        pnew->size = size;
        pnew->frame_idx = head->frame_idx;
        pnew->idx = head->idx;
        pnew->frame_timestamp = now_time;
        pnew->next = NULL;   //新节点指针域置NULL
        head->tail->next = pnew;  //新节点插入到表尾；首个节点时，则意味着ead->next也指向了pnew
        head->tail = pnew;   //为指针指向当前的尾节点;元地址迁移，元的原先的next脱离了元，新地址的next成为元的next指针；
        head->frame_idx += new_frame;
        head->idx++;
        head->num++;
    }
    return ret;
}

void release_brnode(BitRateNode *head)
{
    if(head && head->num)
    {
        do{
            BitRateNode *q;
            q = head->next;
            if(q == NULL || q == head)
            {
                break;
            }
            else{
                head->next = head->next->next;
            }
            free(q);   //释放节点i的内存单元
        }while(1);
        free(head);
    }
}
#endif
void WaitStream(SocketObj *obj)
{
    SessionObj *session = NULL;
    MYPRINT("WaitStream: 0 \n",obj);
    //char handle[8] = "";
    int count = 0;
    AVStreamObj *stream = NULL;
    do{
        pthread_mutex_lock(&obj->status_lock);
        session = (SessionObj *)obj->session;
        if(session)
        {
            //memcpy(handle, session->handle, 8);
            long long *testp = (long long *)session->handle;
            stream = (AVStreamObj *)testp[0];
        }
        pthread_mutex_unlock(&obj->status_lock);
        if(session && stream)//strcmp(handle, ""))
        {
            //long long *testp = (long long *)handle;
            //AVStreamObj *stream = (AVStreamObj *)testp[0];
            MYPRINT("WaitStream: stream=%x \n",stream);
            break;
        }
        else
        {
            if((count % 25) == 24)
            {
                MYPRINT("WaitStream: obj=%x \n",obj);
                MYPRINT("WaitStream: session=%x \n",session);
                //MYPRINT("WaitStream: handle=%s \n",handle);
            }
            usleep(100000);//100ms
            count++;
        }
    }while(1);
}
int WaitCapture(SocketObj *obj, int status0, int avtype)
{
    int ret = 1;
    SessionObj *session = NULL;
    int count = 0;
    int status = status0;
    do{
        pthread_mutex_lock(&obj->status_lock);
        session = (SessionObj *)obj->session;
        if(!status0)
        {
            status = obj->recv_status;
        }
        else{
            status = obj->status >= 0;
        }
        pthread_mutex_unlock(&obj->status_lock);
        if(session && session->capture)
        {
            break;
        }
        else if(!status)
        {
            MYPRINT("WaitCapture: status=%d, avtype=%d \n",status, avtype);
            ret = 0;
            break;
        }
        else
        {
            if((count % 25) == 24)
            {
                MYPRINT("WaitCapture: obj=%x, avtype=%d \n",obj, avtype);
                MYPRINT("WaitCapture: status0=%d, obj->recv_status=%d, obj->status=%d \n", status0, obj->recv_status, obj->status);
                MYPRINT("WaitCapture: session=%x \n",session);
                if(session)
                {
                    MYPRINT("WaitCapture: session->capture=%x \n",session->capture);
                }
            }

            usleep(100000);//100ms
            count++;
        }
    }while(1);
    return ret;
}
void WaitRender(SocketObj *obj)
{
    SessionObj *session = NULL;
    do{
        pthread_mutex_lock(&obj->status_lock);
        session = (SessionObj *)obj->session;
        pthread_mutex_unlock(&obj->status_lock);
        if(session && session->render)
        {
            break;
        }
        else
        {
            usleep(100000);//100ms
        }
    }while(1);
}
void * test_encode(int difftime, int pkt_send_num, int64_t now_time, int frame_size, int *frame_bytes)
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
int GetRtxData(CallCodecVideo *obj, NetInfoNode *netNode)
{
    int ret;
    SocketObj *sock = (SocketObj *)obj->sock;
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
                if(!flag && sock)
                {
                    if(!q->rtp_pkt_num)
                    {
                        MYPRINT("waring: GetRtxData: q->rtp_pkt_num=%d \n", q->rtp_pkt_num);
                    }
                    int j = seqnum % q->rtp_pkt_num;
                    AVData *thisAVData = &q->datalist[j];
                    int size = thisAVData->head.size;

                    //thisAVData->now_time = q->frame_timestamp;
                    //MYPRINT("GetRtxData: thisAVData->now_time=%lld, q->frame_timestamp=%lld \n", thisAVData->now_time, q->frame_timestamp);

                    AVData *data = malloc(size);
                    memcpy((void *)data, (void *)thisAVData, size);
                    PushData(sock, (char *)data, size, sock->addr_serv);
                    MYPRINT("GetRtxData: j=%d, chanId=%d \n", j, chanId);
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
int AddRtxData(pthread_mutex_t *lock, RtxNode *pnew, AVData *data, int seqnum)
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
RtxNode * RtxPushData(pthread_mutex_t *lock, RtxNode **rtxHead, uint8_t *pkt_data, int pkt_size, int seqnum, int64_t frame_timestamp)
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
    int is_hcsvc_rtp = GetRtpInfo((uint8_t*)pkt_data, pkt_size, &info, H264_PLT);
    if(!info.max_pkt_num)
    {
        MYPRINT("error:RtxPushData: info.max_pkt_num=%d \n", info.max_pkt_num);
        MYPRINT("error:RtxPushData: info.rtp_pkt_num=%d \n", info.rtp_pkt_num);
        MYPRINT("error:RtxPushData: pkt_size=%d \n", pkt_size);
        MYPRINT("error:RtxPushData: is_hcsvc_rtp=%d \n", is_hcsvc_rtp);
    }
    pnew->rtp_pkt_num = info.rtp_pkt_num;
    pnew->max_pkt_num = info.max_pkt_num;
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
RtxNode * FindeRtxByTimestamp(pthread_mutex_t *lock, RtxNode *rtxHead, uint32_t send_time)
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
int pkt2sendbuf(SocketObj *sock, uint8_t *p_pkt_data, int *p_pkt_size, int p_pkt_num, int insize, int *p_sumbytes,
    int *p_pkt_send_num, int *p_frame_num, int step, int wait_time)//,
{
    int ret = 0;
    pthread_mutex_lock(&sock->status_lock);
    SessionObj * session = (SessionObj *)sock->session;
    CallCodecVideo *obj = NULL;
    int chanId = 0;
    int sessionId = 0;
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
            if(!idx && OPEN_RTX)
            {
                thisRtxNode = RtxPushData(&obj->lock, &obj->rtxHead, p_pkt_data, data_size, pkt_send_num, now_time);
            }
        }
        char *send_buf = malloc(size);//calloc(1, size);
        AVData *udata = (AVData *)send_buf;
        udata->head.datatype = datatype;
        udata->head.codec = 0;
        memcpy(udata->data, &p_pkt_data[offset], data_size);
        udata->head.size = size;
        udata->head.rtx = 0;
        udata->seqnum = pkt_send_num;
        udata->now_time = now_time;
        //MYPRINT("pkt2sendbuf: now_time=%lld \n", now_time);
        //
        udata->sessionId = sessionId;
        udata->chanId = chanId;
        if(thisRtxNode && idx < thisRtxNode->rtp_pkt_num && OPEN_RTX)
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
            int difftime = PushData(sock, (char *)udata, udata->head.size, sock->addr_serv);
            obj->push_delay_time = difftime;
            sumbytes += size;
        }
#else
        int difftime = PushData(sock, (char *)udata, udata->head.size, sock->addr_serv);
        obj->push_delay_time = difftime;
        //MYPRINT("pkt2sendbuf: obj->push_delay_time=%d \n", obj->push_delay_time);
        //offset += data_size;
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
    ret = sumbytes;
    return ret;
}
static int HCSVCRpt(CallCodecVideo *obj, int idx, int ref_idx, int insize, int is_skip, int **p_pkt_size, int *p_pkt_num)
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

    int ret2 = api_raw2rtp_packet(  obj->handle,
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
    //MYPRINT("gxh:HCSVCEncoderImpl::HCSVCRpt: rtpNum= %d \n", rtpNum);

    int enable_fec = 0;
    obj->json = api_renew_json_int(obj->json, "enable_fec", enable_fec);
    obj->json = api_delete_item(obj->json, "insize");
    if(ret2 > 0)
    {
        ret = ret2;
        //ret = rtpNum;
        //MYPRINT("HCSVCRpt: config->enable_fec= %d, config->loss_rate=%f \n", config->enable_fec, config->loss_rate);
        if(config->enable_fec && (config->loss_rate > 0))
        {
            //MYPRINT("HCSVCRpt: config->refs= %d, config->fec_level=%d \n", config->refs, config->fec_level);
            if(config->refs != 1)
            {
                if(config->fec_level == 0)
                {
                    if((ref_idx == 0) || (ref_idx == config->refs))
                    {
                        enable_fec = 1;
                    }
                }
                else if(config->fec_level == 1)
                {
                    if((ref_idx != 1) || (config->refs == 2))
                    {
                        enable_fec = 1;
                    }
                }
                else{
                    enable_fec = 1;
                }
            }
            else {
                if(config->fec_level == 0)
                {
                    if((obj->pict_type == 1) || (obj->pict_type == 2))
                    {
                        //I/P
                        enable_fec = 1;
                    }
                }
                else if(config->fec_level == 1)
                {
                    if((obj->pict_type > 0))
                    {
                        //I/P/B
                        enable_fec = 1;
                    }
                }
                else {
                    enable_fec = 1;
                }
            }
        }

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
            if(fec_k < 5 && config->fec_level > 2)
            {
                if(config->fec_level == 3)
                {
                    obj->json = api_renew_json_float(obj->json, "loss_rate", 0.5);
                    obj->json = api_renew_json_float(obj->json, "code_rate", 0.5);
                }
                else if(config->fec_level == 4)
                {
                    obj->json = api_renew_json_float(obj->json, "loss_rate", 0.8);
                    obj->json = api_renew_json_float(obj->json, "code_rate", 0.2);
                }
            }
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
    //MYPRINT("HCSVCRpt: p_pkt_num[0]= %d \n", p_pkt_num[0]);
    //src = obj->rtpbuffer;
    //dst = obj->outbuffer;
    if(is_skip && false)
    {
        int bytes = api_get_rtpheader2((char*)dst, p_pkt_size, p_pkt_num, RAW_OFFSET);
    }

    return ret;
}

//注意：
//摄像头下开启，是因为存在噪声，噪声这种有害的干扰信息，会极大的占用码字，
//在静帧下，开启静帧检测，实际是以帧为单元，“过滤”这种噪声；
//桌面共享下，因不存在噪声干扰，静止宏块能被压缩算法(包括x264)准确检查出,
//基本不会占用码字，因此，无需开启静帧检测，并能获得更好的效果；
#if 0
char *renew_frame(CallCodecVideo *obj, char *new_frame, int idx)
{
    char *old_frame = obj->last_frame[idx];
    obj->last_frame[idx] = new_frame;//obj->last_frame[idx - 1];
    obj->skip_frame_idx = idx;
    idx++;
    if(idx >= MAX_SKIP_FRAME_NUM || !old_frame)
    {
        return old_frame;
    }
    old_frame = renew_frame(obj, old_frame, idx);
    return old_frame;
}
int is_static_frame(CallCodecVideo *obj, char *outbuf)
{
    int ret = 0;
    CodecInfoObj *config = (CodecInfoObj *)&obj->codecInfo;

    int w = config->width;
    int h = config->height;
    int size = w * h;

#if 0
    int m = 0;//16x16->64*64
    int w4 = w >> m;
    int h4 = h >> m;
    int size4 = w4 * h4;
    char *outbuf2 = (char *)av_malloc((size4 * 3) >> 1);
    memcpy(outbuf2, outbuf, ((size4 * 3) >> 1));
#else
    int m = 2;//16x16->64*64
    int w4 = w >> m;
    int h4 = h >> m;
    int size4 = w4 * h4;
    char *outbuf2 = (char *)av_malloc((size4 * 3) >> 1);
    if(!obj->img_convert_ctx)
    {
        obj->img_convert_ctx = sws_getContext(  w,
	                                            h,
	                                            AV_PIX_FMT_YUV420P,
	                                            w4,
	                                            h4,
	                                            AV_PIX_FMT_YUV420P,
	                                            SWS_FAST_BILINEAR,
	                                            NULL, NULL, NULL);
    }
    if(obj->img_convert_ctx)
    {
        AVFrame src_frame = {};//注意：初始化至关重要，否则会导致异常崩溃
        src_frame.data[0] = outbuf;
        src_frame.data[1] = &outbuf[size];
        src_frame.data[2] = &outbuf[size + (size >> 2)];
        src_frame.linesize[0] = w;
        src_frame.linesize[1] = w >> 1;
        src_frame.linesize[2] = w >> 1;
        src_frame.width = w;
	    src_frame.height = h;
        src_frame.format = AV_PIX_FMT_YUV420P;


        AVFrame dst_frame = {};//注意：初始化至关重要，否则会导致异常崩溃
        dst_frame.data[0] = outbuf2;
        dst_frame.data[1] = &outbuf2[size4];
        dst_frame.data[2] = &outbuf2[size4 + (size4 >> 2)];
        dst_frame.linesize[0] = w4;
        dst_frame.linesize[1] = w4 >> 1;
        dst_frame.linesize[2] = w4 >> 1;
        dst_frame.width = w4;
	    dst_frame.height = h4;
        dst_frame.format = AV_PIX_FMT_YUV420P;
        sws_scale(  obj->img_convert_ctx,
	                src_frame.data,
			        src_frame.linesize, 0,
			        h,
			        dst_frame.data,
			        dst_frame.linesize);
    }
#endif
    if(obj->skip_frame_idx)
    {
        int nsize = 4;
        int msize = nsize * nsize;//16;
        w = w4;
        h = h4;
        size = size4;
        int thread0 = 700;//600;//500;
        int thread1 = 600;//500;//400;
        int thread2 = 500;//400;
        //
        uint8_t *y0 = &outbuf2[0];
        uint8_t *u0 = &outbuf2[size];
        uint8_t *v0 = &outbuf2[size + (size >> 2)];
        uint8_t *y1 = &obj->last_frame[0][0];
        uint8_t *u1 = &obj->last_frame[0][size];
        uint8_t *v1 = &obj->last_frame[0][size + (size >> 2)];
        uint8_t *y2 = &obj->last_frame[obj->skip_frame_idx][0];
        uint8_t *u2 = &obj->last_frame[obj->skip_frame_idx][size];
        uint8_t *v2 = &obj->last_frame[obj->skip_frame_idx][size + (size >> 2)];
        int64_t sumy0 = 0;
        int64_t sumy1 = 0;
        int n = ((w - msize) >> nsize) * ((h - msize) >> nsize);
        //
        int mb_w = (w >> 4) + ((w % 16) ? 1 : 0);
        int mb_h = (h >> 4) + ((h % 16) ? 1 : 0);
        int bmb_w = (mb_w >> 3) + ((mb_w % 8) ? 1 : 0);
        int bmb_h = mb_h;
        //
        for(int i = 0; i < (h - msize); i += msize)
        {
            for(int j = 0; j < (w - msize); j += msize)
            {
                int sad = 0;
                sad = I2SADnxm(obj->sadHnd, 16, 16, &y0[i * w + j], w, &y1[i * w + j], w);
                //int sad2 = x264_pixel_sa8d_16x16_gxh(obj->x264_hnd, &y0[i * w + j], w, &y1[i * w + j], w);
                //MYPRINT("is_static_frame: sad=%d, sad2=%d, w=%d \n", sad, sad2, w);
                if(sad > (thread1 * 4))
                {
                    //MYPRINT("is_static_frame: 0: sad=%d, i=%d, j=%d \n", sad, i, j);
                    goto save_frame;//
                    //continue;
                }
                sumy0 += sad;
                sad = I2SADnxm(obj->sadHnd, 16, 16, &y0[i * w + j], w, &y2[i * w + j], w);
                //sad = x264_pixel_sa8d_16x16_gxh(obj->x264_hnd, &y0[i * w + j], w, &y2[i * w + j], w);
                if(sad > (thread0 * 4))
                {
                    //MYPRINT("is_static_frame: 1: sad=%d, i=%d, j=%d \n", sad, i, j);
                    goto save_frame;//
                    //continue;
                }
                sumy1 += sad;
#if 0
                int bmb_xy = i * bmb_w + (j >> 3);
                int bmb_bit = (j % 8);
                uint8_t value = obj->skip_mb[bmb_xy];
                value |= 1 << bmb_bit;
                obj->skip_mb[bmb_xy] = value;
#endif
            }
        }
        int avgsady0 = (int)(sumy0 / n);
        int avgsady1 = (int)(sumy1 / n);

        w = w >> 1;
        h = h >> 1;
        int64_t sumu0 = 0;
        int64_t sumu1 = 0;
        int64_t sumv0 = 0;
        int64_t sumv1 = 0;
        //n = ((w - msize) >> nsize) * ((h - msize) >> nsize);
        n = (w >> nsize) * (h >> nsize);
        for(int i = 0; i < h; i += msize)
        {
            for(int j = 0; j < w; j += msize)
            {
                if((j + msize) > w || (i + msize) > h)
                {
                    break;
                }
                int sad = 0;
                sad = I2SADnxm(obj->sadHnd, 16, 16, &u0[i * w + j], w, &u1[i * w + j], w);
                //int sad2 = x264_pixel_sa8d_16x16_gxh(obj->x264_hnd, &u0[i * w + j], w, &u1[i * w + j], w);
                //MYPRINT("is_static_frame: sad=%d, sad2=%d, w=%d \n", sad, sad2, w);
                if(sad > (thread2 << 1))
                {
                    //MYPRINT("is_static_frame: 2: sad=%d, i=%d, j=%d \n", sad, i, j);
                    goto save_frame;//continue;
                }
                sumu0 += sad;
                sad = I2SADnxm(obj->sadHnd, 16, 16, &u0[i * w + j], w, &u2[i * w + j], w);
                //sad = x264_pixel_sa8d_16x16_gxh(obj->x264_hnd, &u0[i * w + j], w, &u2[i * w + j], w);
                if(sad > (thread2 << 1))
                {
                    //MYPRINT("is_static_frame: 3: sad=%d, i=%d, j=%d \n", sad, i, j);
                    goto save_frame;//continue;
                }
                sumu1 += sad;

                sad = I2SADnxm(obj->sadHnd, 16, 16, &v0[i * w + j], w, &v1[i * w + j], w);
                //sad = x264_pixel_sa8d_16x16_gxh(obj->x264_hnd, &v0[i * w + j], w, &v1[i * w + j], w);
                if(sad > (thread2 << 1))
                {
                    //MYPRINT("is_static_frame: 4: sad=%d, i=%d, j=%d \n", sad, i, j);
                    goto save_frame;//continue;
                }
                sumv0 += sad;
                sad = I2SADnxm(obj->sadHnd, 16, 16, &v0[i * w + j], w, &v2[i * w + j], w);
                //sad = x264_pixel_sa8d_16x16_gxh(obj->x264_hnd, &v0[i * w + j], w, &v2[i * w + j], w);
                if(sad > (thread2 << 1))
                {
                    //MYPRINT("is_static_frame: 5: sad=%d, i=%d, j=%d \n", sad, i, j);
                    goto save_frame;//continue;
                }
                sumv1 += sad;

            }
        }
        int avgsadu0 = (int)(sumu0 / n);
        int avgsadu1 = (int)(sumu1 / n);
        int avgsadv0 = (int)(sumv0 / n);
        int avgsadv1 = (int)(sumv1 / n);

        ret |= avgsady0 < thread1 && avgsady1 < thread0 && avgsadu0 < thread2 && avgsadu1 < thread2 && avgsadv0 < thread2 && avgsadv1 < thread2;
        if(!ret)
        {
            if(!(avgsady0 < thread1))
                MYPRINT("is_static_frame: avgsady0=%d \n", avgsady0);
            if(!(avgsady1 < thread0))
                MYPRINT("is_static_frame: avgsady1=%d \n", avgsady1);
            if(!(avgsadu0 < thread2))
                MYPRINT("is_static_frame: avgsadu0=%d \n", avgsadu0);
            if(!(avgsadu1 < thread2))
                MYPRINT("is_static_frame: avgsadu1=%d \n", avgsadu1);
            if(!(avgsadv0 < thread2))
                MYPRINT("is_static_frame: avgsadv0=%d \n", avgsadv0);
            if(!(avgsadv1 < thread2))
                MYPRINT("is_static_frame: avgsadv1=%d \n", avgsadv1);
        }
        if(ret)
        {
            MYPRINT("is_static_frame: ret=%d \n", ret);
        }
    }
save_frame:
    if(obj->last_frame[0])
    {
        if(!(obj->frame_idx % 25))
        {
            char *old_frame = renew_frame(obj, obj->last_frame[0], 1);
            if(old_frame)
            {
                av_free(old_frame);
            }
            //obj->last_frame[1] = obj->last_frame[0];
        }
        else{
            av_free(obj->last_frame[0]);
        }
    }
    obj->last_frame[0] = outbuf2;

    return ret;
}

int is_static_frame2(CallCodecVideo *obj, char *outbuf)
{
    int ret = 0;
    CodecInfoObj *config = (CodecInfoObj *)&obj->codecInfo;

    int w = config->width;
    int h = config->height;
    int size = w * h;

#if 1
    int m = 0;//16x16->64*64
    int w4 = w >> m;
    int h4 = h >> m;
    int size4 = w4 * h4;
    char *outbuf2 = (char *)av_malloc((size4 * 3) >> 1);
    memcpy(outbuf2, outbuf, ((size4 * 3) >> 1));
#else
    int m = 2;//16x16->64*64
    int w4 = w >> m;
    int h4 = h >> m;
    int size4 = w4 * h4;
    char *outbuf2 = (char *)av_malloc((size4 * 3) >> 1);
    if(!obj->img_convert_ctx)
    {
        obj->img_convert_ctx = sws_getContext(  w,
	                                            h,
	                                            AV_PIX_FMT_YUV420P,
	                                            w4,
	                                            h4,
	                                            AV_PIX_FMT_YUV420P,
	                                            SWS_FAST_BILINEAR,
	                                            NULL, NULL, NULL);
    }
    if(obj->img_convert_ctx)
    {
        AVFrame src_frame = {};//注意：初始化至关重要，否则会导致异常崩溃
        src_frame.data[0] = outbuf;
        src_frame.data[1] = &outbuf[size];
        src_frame.data[2] = &outbuf[size + (size >> 2)];
        src_frame.linesize[0] = w;
        src_frame.linesize[1] = w >> 1;
        src_frame.linesize[2] = w >> 1;
        src_frame.width = w;
	    src_frame.height = h;
        src_frame.format = AV_PIX_FMT_YUV420P;


        AVFrame dst_frame = {};//注意：初始化至关重要，否则会导致异常崩溃
        dst_frame.data[0] = outbuf2;
        dst_frame.data[1] = &outbuf2[size4];
        dst_frame.data[2] = &outbuf2[size4 + (size4 >> 2)];
        dst_frame.linesize[0] = w4;
        dst_frame.linesize[1] = w4 >> 1;
        dst_frame.linesize[2] = w4 >> 1;
        dst_frame.width = w4;
	    dst_frame.height = h4;
        dst_frame.format = AV_PIX_FMT_YUV420P;
        sws_scale(  obj->img_convert_ctx,
	                src_frame.data,
			        src_frame.linesize, 0,
			        h,
			        dst_frame.data,
			        dst_frame.linesize);
    }
#endif
    if(obj->skip_frame_idx)
    {
        int motion = 0;
        int nsize = 4;
        int msize = nsize * nsize;//16;
        w = w4;
        h = h4;
        size = size4;
        int thread0 = 700;//distill 1/4
        int thread1 = 600;//distill 1/4
        int thread2 = 500;//distill 1/4

        thread0 = 1200;//700;//600;//500;
        thread1 = 800;//600;//500;//400;
        thread2 = 800;//500;//400;

        //thread0 = 300;
        //thread1 = 200;
        //thread2 = 200;
        //
        uint8_t *static_frame = obj->last_frame[obj->skip_frame_idx];
        if(obj->static_frame)
        {
            static_frame = obj->static_frame;//抑制长时间渐变累积
        }
        uint8_t *y0 = &outbuf2[0];
        uint8_t *u0 = &outbuf2[size];
        uint8_t *v0 = &outbuf2[size + (size >> 2)];
        uint8_t *y1 = &obj->last_frame[0][0];
        uint8_t *u1 = &obj->last_frame[0][size];
        uint8_t *v1 = &obj->last_frame[0][size + (size >> 2)];
        uint8_t *y2 = &static_frame[0];
        uint8_t *u2 = &static_frame[size];
        uint8_t *v2 = &static_frame[size + (size >> 2)];
        int64_t sumy0 = 0;
        int64_t sumy1 = 0;
        int64_t sumu0 = 0;
        int64_t sumu1 = 0;
        int64_t sumv0 = 0;
        int64_t sumv1 = 0;
        //int n = ((w - msize) >> nsize) * ((h - msize) >> nsize);
        int n = (w >> nsize) * (h >> nsize);
        //
        int mb_w = (w >> 4) + ((w % 16) ? 1 : 0);
        int mb_h = (h >> 4) + ((h % 16) ? 1 : 0);
        int bmb_w = (mb_w >> 3) + ((mb_w % 8) ? 1 : 0);
        int bmb_h = mb_h;
        int skip_mb_num = 0;
        //
        for(int i = 0; i < h; i += msize)
        {
            for(int j = 0; j < w; j += msize)
            {
                if((j + msize) > w || (i + msize) > h)
                {
                    break;
                }
                int is_skip_mb = 1;
                int sad = 0;
                //y
                int i8 = i + 8;
                int j8 = j + 8;
                //sad = I2SADnxm(obj->sadHnd, 16, 16, &y0[i * w + j], w, &y1[i * w + j], w);
                int sad0 = I2SADnxm(obj->sadHnd, 8, 8, &y0[i  * w + j ], w, &y1[i  * w + j ], w);
                int sad1 = I2SADnxm(obj->sadHnd, 8, 8, &y0[i  * w + j8], w, &y1[i  * w + j8], w);
                int sad2 = I2SADnxm(obj->sadHnd, 8, 8, &y0[i8 * w + j ], w, &y1[i8 * w + j ], w);
                int sad3 = I2SADnxm(obj->sadHnd, 8, 8, &y0[i8 * w + j8], w, &y1[i8 * w + j8], w);
                if(sad0 > thread1 || sad1 > thread1 || sad2 > thread1 || sad3 > thread1)
                {
                    is_skip_mb = 0;
                }
                sad = (sad0 + sad1 + sad2 + sad3);
                if(sad > (thread1))
                {
                    motion = 1;
                    MYPRINT("is_static_frame: 0: sad=%d, i=%d, j=%d \n", sad, i, j);
                    //goto save_frame;//
                    continue;
                }
                sumy0 += sad;

                //sad = I2SADnxm(obj->sadHnd, 16, 16, &y0[i * w + j], w, &y2[i * w + j], w);
                sad0 = I2SADnxm(obj->sadHnd, 8, 8, &y0[i  * w + j ], w, &y2[i  * w + j ], w);
                sad1 = I2SADnxm(obj->sadHnd, 8, 8, &y0[i  * w + j8], w, &y2[i  * w + j8], w);
                sad2 = I2SADnxm(obj->sadHnd, 8, 8, &y0[i8 * w + j ], w, &y2[i8 * w + j ], w);
                sad3 = I2SADnxm(obj->sadHnd, 8, 8, &y0[i8 * w + j8], w, &y2[i8 * w + j8], w);
                if(sad0 > thread0 || sad1 > thread0 || sad2 > thread0 || sad3 > thread0)
                {
                    is_skip_mb = 0;
                }
                sad = (sad0 + sad1 + sad2 + sad3);
                if(sad > (thread0))
                {
                    motion = 1;
                    MYPRINT("is_static_frame: 1: sad=%d, i=%d, j=%d \n", sad, i, j);
                    //goto save_frame;//
                    continue;
                }
                sumy1 += sad;

                //if(!(i & 1) && !(j & 1))
                {
                    int I = i >> 1;
                    int J = j >> 1;
                    int w2 = w >> 1;
                    int I4 = I + 4;
                    int J4 = J + 4;
#if 1
                    //u
                    //sad = I2SADnxm(obj->sadHnd, 8, 8, &u0[I * w2 + J], w2, &u1[I * w2 + J], w2) << 2;
                    sad0 = I2SADnxm(obj->sadHnd, 4, 4, &u0[I *  w2 + J ], w2, &u1[I  * w2 + J ], w2) << 2;
                    sad1 = I2SADnxm(obj->sadHnd, 4, 4, &u0[I *  w2 + J4], w2, &u1[I  * w2 + J4], w2) << 2;
                    sad2 = I2SADnxm(obj->sadHnd, 4, 4, &u0[I4 * w2 + J ], w2, &u1[I4 * w2 + J ], w2) << 2;
                    sad3 = I2SADnxm(obj->sadHnd, 4, 4, &u0[I4 * w2 + J4], w2, &u1[I4 * w2 + J4], w2) << 2;
                    if(sad0 > thread2 || sad1 > thread2 || sad2 > thread2 || sad3 > thread2)
                    {
                        is_skip_mb = 0;
                    }
                    sad = (sad0 + sad1 + sad2 + sad3);
                    if(sad > (thread2))
                    {
                        motion = 1;
                        MYPRINT("is_static_frame: 2: sad=%d, i=%d, j=%d \n", sad, i, j);
                        //goto save_frame;//
                        continue;
                    }
                    sumu0 += sad;

                    //sad = I2SADnxm(obj->sadHnd, 8, 8, &u0[I * w2 + J], w2, &u2[I * w2 + J], w2) << 2;
                    sad0 = I2SADnxm(obj->sadHnd, 4, 4, &u0[I *  w2 + J ], w2, &u2[I  * w2 + J ], w2) << 2;
                    sad1 = I2SADnxm(obj->sadHnd, 4, 4, &u0[I *  w2 + J4], w2, &u2[I  * w2 + J4], w2) << 2;
                    sad2 = I2SADnxm(obj->sadHnd, 4, 4, &u0[I4 * w2 + J ], w2, &u2[I4 * w2 + J ], w2) << 2;
                    sad3 = I2SADnxm(obj->sadHnd, 4, 4, &u0[I4 * w2 + J4], w2, &u2[I4 * w2 + J4], w2) << 2;
                    if(sad0 > thread2 || sad1 > thread2 || sad2 > thread2 || sad3 > thread2)
                    {
                        is_skip_mb = 0;
                    }
                    sad = (sad0 + sad1 + sad2 + sad3);
                    if(sad > (thread2))
                    {
                        motion = 1;
                        MYPRINT("is_static_frame: 3: sad=%d, i=%d, j=%d \n", sad, i, j);
                        //goto save_frame;//
                        continue;
                    }
                    sumu1 += sad;

                    //v
                    //sad = I2SADnxm(obj->sadHnd, 8, 8, &v0[I * w2 + J], w2, &v1[I * w2 + J], w2) << 2;
                    sad0 = I2SADnxm(obj->sadHnd, 4, 4, &v0[I *  w2 + J ], w2, &v1[I  * w2 + J ], w2) << 2;
                    sad1 = I2SADnxm(obj->sadHnd, 4, 4, &v0[I *  w2 + J4], w2, &v1[I  * w2 + J4], w2) << 2;
                    sad2 = I2SADnxm(obj->sadHnd, 4, 4, &v0[I4 * w2 + J ], w2, &v1[I4 * w2 + J ], w2) << 2;
                    sad3 = I2SADnxm(obj->sadHnd, 4, 4, &v0[I4 * w2 + J4], w2, &v1[I4 * w2 + J4], w2) << 2;
                    if(sad0 > thread2 || sad1 > thread2 || sad2 > thread2 || sad3 > thread2)
                    {
                        is_skip_mb = 0;
                    }
                    sad = (sad0 + sad1 + sad2 + sad3);
                    if(sad > (thread2))
                    {
                        motion = 1;
                        MYPRINT("is_static_frame: 4: sad=%d, i=%d, j=%d \n", sad, i, j);
                        //goto save_frame;//
                        continue;
                    }
                    sumv0 += sad;

                    //sad = I2SADnxm(obj->sadHnd, 8, 8, &v0[I * w2 + J], w2, &v2[I * w2 + J], w2) << 2;
                    sad0 = I2SADnxm(obj->sadHnd, 4, 4, &v0[I *  w2 + J ], w2, &v2[I  * w2 + J ], w2) << 2;
                    sad1 = I2SADnxm(obj->sadHnd, 4, 4, &v0[I *  w2 + J4], w2, &v2[I  * w2 + J4], w2) << 2;
                    sad2 = I2SADnxm(obj->sadHnd, 4, 4, &v0[I4 * w2 + J ], w2, &v2[I4 * w2 + J ], w2) << 2;
                    sad3 = I2SADnxm(obj->sadHnd, 4, 4, &v0[I4 * w2 + J4], w2, &v2[I4 * w2 + J4], w2) << 2;
                    if(sad0 > thread2 || sad1 > thread2 || sad2 > thread2 || sad3 > thread2)
                    {
                        is_skip_mb = 0;
                    }
                    sad = (sad0 + sad1 + sad2 + sad3) ;
                    if(sad > (thread2))
                    {
                        motion = 1;
                        MYPRINT("is_static_frame: 5: sad=%d, i=%d, j=%d \n", sad, i, j);
                        //goto save_frame;//
                        continue;
                    }
                    sumv1 += sad;
#endif
                }

#ifdef OPEN_SKIP_MODE
                if(is_skip_mb)
                {
                    int mb_y = i >> 4;
                    int mb_x = j >> 4;
                    int bmb_xy = mb_y * bmb_w + (mb_x >> 3);
                    int bmb_bit = (mb_x % 8);
                    uint8_t value = obj->skip_mb[bmb_xy];
                    int value1 = 1 << bmb_bit;
                    value |= 1 << bmb_bit;
                    obj->skip_mb[bmb_xy] = value;
                    //
                    int value2 = obj->skip_mb[bmb_xy];
                    //value2 &= ~(1 << bmb_bit);
                    value2 &= (1 << bmb_bit);
                    if(value1 != value2)
                    {
                        MYPRINT("is_static_frame: value1=%d, value2=%d \n", value1, value2);
                    }
                    skip_mb_num++;
                }
#endif
            }
        }
#ifdef OPEN_SKIP_MODE
        //腐蚀算法
        uint8_t skip_mb[MAX_MB_SIZE] = {0};
        memset(skip_mb, 0, obj->bmb_size);
        int offset = msize << 1;
        for(int i = offset; i < (h - offset); i += msize)
        {
            for(int j = offset; j < (w - offset); j += msize)
            {
                int none_zero = 1;
                for(int I = (i - offset); I <= (i + offset); I += msize)
                {
                    for(int J = (j - offset); J <= (j + offset); J += msize)
                    {
                        int mb_y = I >> 4;
                        int mb_x = J >> 4;
                        int bmb_xy = mb_y * bmb_w + (mb_x >> 3);
                        int bmb_bit = (mb_x % 8);
                        uint8_t value = obj->skip_mb[bmb_xy];
                        value &= (1 << bmb_bit);
                        if(!value)
                        {
                            none_zero = 0;
                        }
                    }
                }
                if(none_zero)
                {
                    int mb_y = i >> 4;
                    int mb_x = j >> 4;
                    int bmb_xy = mb_y * bmb_w + (mb_x >> 3);
                    int bmb_bit = (mb_x % 8);
                    uint8_t value = skip_mb[bmb_xy];
                    int value1 = 1 << bmb_bit;
                    value |= 1 << bmb_bit;
                    skip_mb[bmb_xy] = value;
                }
            }
        }
        memcpy(obj->skip_mb, skip_mb, obj->bmb_size);
#endif
        float skip_rate = (float)skip_mb_num / (float)n;
        MYPRINT("is_static_frame: skip_rate=%1.3f, n=%d, skip_mb_num=%d \n", skip_rate, n, skip_mb_num);
        int avgsady0 = (int)(sumy0 / n);
        int avgsady1 = (int)(sumy1 / n);
        int avgsadu0 = (int)(sumu0 / n);
        int avgsadu1 = (int)(sumu1 / n);
        int avgsadv0 = (int)(sumv0 / n);
        int avgsadv1 = (int)(sumv1 / n);


        if(motion)
        {
            ret = 0;
        }
        else{
            //单值在均值上下波动：理论上，均值的阈值更小
            ret |= avgsady0 < thread1 && avgsady1 < thread0 && avgsadu0 < thread2 && avgsadu1 < thread2 && avgsadv0 < thread2 && avgsadv1 < thread2;
            //ret |= avgsady1 < thread0 && avgsadu1 < thread2 && avgsadv1 < thread2;
        }
        MYPRINT("is_static_frame: avgsady0=%d \n", avgsady0);
        MYPRINT("is_static_frame: avgsadu0=%d \n", avgsadu0);
        MYPRINT("is_static_frame: avgsadv0=%d \n", avgsadv0);
        MYPRINT("is_static_frame: avgsady1=%d \n", avgsady1);
        MYPRINT("is_static_frame: avgsadu1=%d \n", avgsadu1);
        MYPRINT("is_static_frame: avgsadv1=%d \n", avgsadv1);

        if(!ret)
        {
            if(!(avgsady0 < thread1))
                MYPRINT("is_static_frame: avgsady0=%d \n", avgsady0);
            if(!(avgsady1 < thread0))
                MYPRINT("is_static_frame: avgsady1=%d \n", avgsady1);
            if(!(avgsadu0 < thread2))
                MYPRINT("is_static_frame: avgsadu0=%d \n", avgsadu0);
            if(!(avgsadu1 < thread2))
                MYPRINT("is_static_frame: avgsadu1=%d \n", avgsadu1);
            if(!(avgsadv0 < thread2))
                MYPRINT("is_static_frame: avgsadv0=%d \n", avgsadv0);
            if(!(avgsadv1 < thread2))
                MYPRINT("is_static_frame: avgsadv1=%d \n", avgsadv1);
        }
        if(ret)
        {
            //MYPRINT("is_static_frame: ret=%d \n", ret);
        }
        //ret = 0;//test
    }
save_frame:
    if(obj->last_frame[0])
    {
        if(!(obj->frame_idx % 25))
        {
            char *old_frame = renew_frame(obj, obj->last_frame[0], 1);
            if(old_frame && old_frame != obj->static_frame)
            {
                //MYPRINT("is_static_frame: free 1 \n");//可能被之前的obj->static_frame释放掉了
                av_free(old_frame);
            }
            //obj->last_frame[1] = obj->last_frame[0];
        }
        else{
            if(obj->last_frame[0] != obj->static_frame)
            {
                //MYPRINT("is_static_frame: free 2 \n");
                av_free(obj->last_frame[0]);
            }
        }
    }
    obj->last_frame[0] = outbuf2;

    if(!obj->static_frame && ret)
    {
        obj->static_frame = outbuf2;
    }
    else if(obj->static_frame && !ret)
    {
        //MYPRINT("is_static_frame: free 3 \n");
        int used = 0;
        for(int i = 0; i < obj->skip_frame_idx; i++)
        {
            if(obj->last_frame[i] == obj->static_frame)
            {
                used = 1;
            }
        }
        if(!used)
        {
            av_free(obj->static_frame);
        }
        obj->static_frame = NULL;
    }
    MYPRINT("is_static_frame: ret=%d \n", ret);
    return ret;
}
#endif

int video_encode_one_frame(SocketObj *sock, uint8_t *frame_data, int insize,
    int *p_sumbytes, int *p_pkt_send_num, int *p_frame_num, int step, int wait_time)
{
    int ret = 0;
    pthread_mutex_lock(&sock->status_lock);
    SessionObj * session = (SessionObj *)sock->session;
    CallCodecVideo *obj = session->codec;
    pthread_mutex_unlock(&sock->status_lock);
    //session->sessionInfo
    if(obj)
    {
        CodecInfoObj *config = (CodecInfoObj *)&obj->codecInfo;
        if(!obj->outfp && false)
        {
            const char *filename = "/home/gxh/works/gxhshare/test.h264";
            obj->outfp = fopen(filename, "wb");
        }
        int data_size = insize;
        int *p_pkt_size = NULL;
        uint8_t *p_pkt_data = frame_data;
        int p_pkt_num = 0;
        int skip_frame = 0;
        if(!config->lossless)
        {
            int64_t now_time = get_sys_time();
            int frame_idx = obj->frame_idx;
            int gop_size = config->gop_size;
            //int res_num = codecInfo->res_num;
            int refs = config->refs;
            int max_refs = config->max_refs;
            int ref_idx = 0;
            int max_b_frames = config->max_b_frames;
            int is_static = 0;
            //MYPRINT("video_encode_one_frame: refs=%d \n", refs);
#ifdef OPEN_SKIP_FRAME
            //MYPRINT("video_encode_one_frame: config->bwthreshold=%2.3f \n", config->bwthreshold);
#if 0
            int icpurate = 0;
            icpurate = api_getCpuRate(&obj->pCpurate, obj->icpurate, 500);
            if(icpurate > 0)
            {
                //MYPRINT("video_encode_one_frame: icpurate=%d \n", icpurate);
                obj->icpurate = icpurate;
            }
#endif
            //skip_frame = api_control_max_bitrate(obj->handle, frame_idx, frame_data, config->bwthreshold);
            skip_frame = api_control_max_bitrate2(  obj->handle, frame_idx, frame_data, config->bwthreshold,
                                                    //obj->push_delay_time, 250
                                                    (obj->icpurate - 80), 4
                                                    );
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

                }
            }
            else{
                ref_idx = 0;
                //encoder->pict_type = 1;//test
                if(frame_idx > 0)
                {
                    int refresh_idr = 0;
                    //obj->json = api_delete_item(obj->json, "refresh_idr");
                    obj->json = api_renew_json_int(obj->json, "refresh_idr", refresh_idr);
#if 0
                    int last_loss_rate = api_get_loss_rate(obj->handle, 2);
                    if(last_loss_rate < 10 && true)//test
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
                        MYPRINT("video_encode_one_frame: last_loss_rate=%d \n", last_loss_rate);
                        //
                        config->refs = 16;
                        obj->json = api_renew_json_int(obj->json , "refs", config->refs);
                    }
#endif
                }
            }
            //MYPRINT("video_encode_one_frame: obj->pict_type=%d \n", obj->pict_type);
            if(ref_idx)
            {

            }
            else{
                skip_frame = 0;
            }
            //MYPRINT("video_encode_one_frame: skip_frame=%d \n", skip_frame);
            if(skip_frame)
            {
                obj->json = api_renew_json_int(obj->json, "skip_frame", skip_frame);
            }
#if 0
            //if(obj->pict_type == 1)
            //{
            //    memset(obj->skip_mb, 0, MAX_MB_SIZE);
            //}
            api_video_set_skip_mb(obj->handle, obj->skip_mb, obj->bmb_size);
#endif

            //MYPRINT("video_encode_one_frame: ref_idx=%d \n", ref_idx);
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
            int is_skip = GetNetResult(obj, ref_idx);
            //

            char* jsonstr = api_json2str(obj->json);
            //MYPRINT("video_encode_one_frame: jsonstr=%s \n", jsonstr);
            //encode frame
            ret = api_video_encode_one_frame(   obj->handle,
                                                (char *)frame_data,
                                                jsonstr,
                                                (char *)obj->outbuffer,
                                                obj->outparam);

            api_json2str_free(jsonstr);

            //MYPRINT("video_encode_one_frame: ret=%d \n", ret);
            obj->json = api_delete_item(obj->json, "pict_type");
            obj->json = api_delete_item(obj->json, "skip_frame");

            if(ret > 0)
            {
                if(skip_frame && config->codec_mode && false)
                {
                    ret = 0;
                    api_br_push_data(obj->handle, 0, (!skip_frame), now_time);
                    obj->frame_idx++;
                    return ret;
                }
                if ( obj->outfp != NULL)
                {
                    //MYPRINT("video_encode_one_frame: ret=%d \n", ret);
                    fwrite(obj->outbuffer, 1, ret, obj->outfp);
                }
                //raw2rtp
                //fec encode
                ret = HCSVCRpt(obj, 0, ref_idx, ret, is_skip, &p_pkt_size, &p_pkt_num);
                if(ret > 0)
                {
                    //MYPRINT("video_encode_one_frame: 2: ret=%d \n", ret);
                    p_pkt_data = obj->outrtp;
                    data_size = ret;
                }
                //if(skip_frame)
                //{
                //    MYPRINT("video_encode_one_frame: ret=%d \n", ret);
                //}
                api_br_push_data(obj->handle, (ret << 3), (!skip_frame), now_time);
            }
            else{
                MYPRINT("video_encode_one_frame: ret=%d \n", ret);
                return ret;
            }
        }
        else{
            //av_free(frame_data);
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

int encode_frame_run(SocketObj *sock)
{
    int ret = 0;

    FILE *yuvfp = NULL;
    char *readbuf = NULL;
    char *filename = NULL;
    pthread_mutex_lock(&sock->status_lock);
    sock->recv_status = 1;
    int status = sock->recv_status;
    SessionObj * session = (SessionObj *)sock->session;
    CallCodecVideo *obj = session->codec;
    void *handle = NULL;
    int w = DEFAULT_WIDTH;
    int h = DEFAULT_HEIGHT;
    int frame_size = (w * h * 3) >> 1;

    if(session)
    {
        w = session->sessionInfo->width;
        h = session->sessionInfo->height;
        frame_size = (w * h * 3) >> 1;
        filename = session->sessionInfo->yuvfilename;
        MYPRINT("encode_frame_run: filename=%s \n", filename);
        if(filename && strcmp(filename, ""))
        {
            if(false) //yuv
            {
                yuvfp = fopen(filename, "rb");
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
    int m = (frame_size % MAX_DATA_SIZE) > 0;
    int max_pkt_num = frame_size / MAX_DATA_SIZE + m;
    float factor = 10;
    if((w * h) == (1280 * 720))
    {
        factor = 8;//3.5;//4;//5;
    }
    else if((w * h) == (1920 * 1080))
    {
        factor = 18;//10;
    }
    factor *= 0.26;//0.48;//0.9;//1.2;//7;//2.5;
    int step = (int)(max_pkt_num / factor);
    //
#ifndef SIMULATOR_DATA
    if(session && filename && strcmp(filename, "") && !readbuf)
    {
        WaitStream(sock);
        handle = session->handle;
        api_avstream_status(handle, 1);//start
    }
    if(!readbuf && !handle)
    {
        MYPRINT("video encode_frame_run: WaitCapture... \n");
        status = WaitCapture(sock, 0, kIsVideo);
    }
#endif

    CallCapture *capture = NULL;
    MYPRINT("video encode_frame_run: status=%d \n",status);

    while(status > 0)
    {
        int64_t now_time = get_sys_time();
        int difftime = (int)(now_time - sock->start_time);
        //测试模式下，是全速运行，未进行速度控制
        pthread_mutex_lock(&sock->status_lock);
        session = (SessionObj *)sock->session;
        if(session)
        {
            capture = session->capture;
        }
        pthread_mutex_unlock(&sock->status_lock);
        //MYPRINT("encode_frame_run: capture=%x \n", capture);
        ret = 0;
        if((capture || yuvfp || handle) && difftime < TEST_TIME_LEN)
        {
            char *outbuf = NULL;

            if(yuvfp)
            {
                ret = fread(readbuf, 1, frame_size, yuvfp);
                if(ret != frame_size)
                {
                    fseek(yuvfp, 0, SEEK_SET);
                    ret = fread(readbuf, 1, frame_size, yuvfp);
                }
                outbuf = readbuf;
                usleep(30000);//30ms
            }
            else if(handle)
            {
                AVReadNode *p = (AVReadNode *)api_avstream_pop_data(handle, kIsVideo);
                //MYPRINT("video encode_frame_run: p=%x \n", p);
                if(p)
                {
                    outbuf = p->data;
                    ret = p->size;
                    free(p);
                }
            }
            else{
                //MYPRINT("encode_frame_run: read_frame: 0 \n");
                ret = read_frame(capture, &outbuf);
                //MYPRINT("encode_frame_run: read_frame: ret=%d \n", ret);
                int64_t time2 = get_sys_time();
                difftime = (int)(time2 - now_time);
                //MYPRINT("encode_frame_run: read_frame: difftime=%d (ms) \n", difftime);
            }
            //MYPRINT("encode_frame_run: read_frame: ret=%d \n", ret);
            if(ret > 0)
            {
                //MYPRINT("encode_frame_run: read_frame: ret=%d \n", ret);
                if(session->render)
                {
                    int64_t time1 = get_sys_time();
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
                    //MYPRINT("encode_frame_run: selfmode=%d, actor=%d, chanId=%d \n", selfmode, actor, chanId);
                    FramePushData(session->render, frame_data, w, h, chanId, now_time, now_time);
                    //free(frame_data);//test
                    int64_t time2 = get_sys_time();
                    difftime = (int)(time2 - time1);
                    //MYPRINT("encode_frame_run: FramePushData: difftime=%d (ms) \n", difftime);
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
                ret = video_encode_one_frame(sock, outbuf, ret, &sumbytes, &pkt_send_num, &frame_num, step, wait_time);
                if(handle && outbuf)
                {
                    av_free(outbuf);
                }
                int64_t time2 = get_sys_time();
                difftime = (int)(time2 - now_time);
                //MYPRINT("encode_frame_run: video_encode_one_frame: difftime=%d (ms) \n", difftime);
                //MYPRINT("encode_frame_run: ret=%d \n", ret);
#endif
            }
            else{
                if(handle && outbuf)
                {
                    av_free(outbuf);
                }
                usleep(10000);//10ms
                //av_free(outbuf);
            }
        }
        else{
            //if(difftime > TEST_TIME_LEN)
            {
                //MYPRINT("encode_frame_run: frame_bytes=%d \n", frame_bytes);
                if(frame_time == 0)
                {
                    frame_time = now_time;
                }
                if(session && session->sessionInfo)
                {
                    int w = session->sessionInfo->width;
                    int h = session->sessionInfo->height;
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
            //else{
            //    usleep(10000);//10ms
            //}
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
            MYPRINT("encode_frame_run: framerate=%2.1f (fps) \n", framerate);
            MYPRINT("encode_frame_run: bitrate=%5.1f (Mbps) \n", bitrate);
        }

        //MYPRINT("encode_frame_run: sock->recv_status=%d \n", sock->recv_status);
        //MYPRINT("encode_frame_run: sock->send_status=%d \n", sock->send_status);

        pthread_mutex_lock(&sock->status_lock);
        status = (sock->recv_status > 0) & (sock->send_status > 0);
        pthread_mutex_unlock(&sock->status_lock);
        //MYPRINT("video encode_frame_run: status=%d \n", status);
    }
    if(session && capture)
    {
        MYPRINT("encode_frame_run: call stop_capture_run \n");
        stop_capture_run(capture);
    }
    MYPRINT("video encode_frame_run: start end \n");
    ExitRecv(sock);
    StopSend(sock);
    WaitSend(sock);//停止发包
    if(readbuf)
    {
        free(readbuf);
    }
    if(yuvfp)
    {
        fclose(yuvfp);
    }
    MYPRINT("video encode_frame_run: over \n");
    return ret;
}
int RenewCountLossRateInfo(CallCodecVideo *obj, FrameJitterNode *head, char *recv_buf, int recv_size, int seqnum, int64_t frame_timestamp)
{
    int ret = 0;
    int head_size = sizeof(AVData) - MAX_DATA_SIZE;
    AVData *udata = (AVData *)recv_buf;
    int raw_size = udata->head.size - head_size;
    int count_lossrate_interval = obj->delay_time < COUNT_LOSSRATE_INTERAVL ? COUNT_LOSSRATE_INTERAVL : obj->delay_time;
    if(!udata->head.rtx)
    {
        int difftime = (int)(frame_timestamp - head->frame_timestamp);
        //for count loss rate
        if(frame_timestamp <= (head->last_frame_timestamp + count_lossrate_interval))
        {
            //A
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
            //B
            if(difftime <= count_lossrate_interval)
            {
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
            else{
                //C

            }

        }
        //count loss rate
        if(difftime > count_lossrate_interval)
        {
            if(head->last_frame_timestamp > 0)
            {
                int sum_pkt = head->last_max_seqnum - head->last_min_seqnum;

                ret = 1 + (100 * (sum_pkt - head->last_pkt_num)) / sum_pkt;
                if(ret > 1)
                {
                    MYPRINT("RenewCountLossRateInfo: sum_pkt=%d \n", sum_pkt);
                    MYPRINT("RenewCountLossRateInfo: head->last_pkt_num=%d \n", head->last_pkt_num);
                    //MYPRINT("JitterPushData: head->pkt_num=%d \n", head->pkt_num);
#if 1
                    if(!obj->logfp)
                    {
                        char filename[256] = "lossrate_gxh_";
                        char ctmp[32] = "";
                        int fileidx = obj->sessionInfo->chanId;
                        sprintf(ctmp, "%d", fileidx);
                        strcat(filename, ctmp);
                        strcat(filename, ".txt");
                        obj->logfp = fopen(filename, "w");
                        if(obj->logfp)
                        {
                            char tmp[64];
	                        time_t t = time(0);
	                        strftime(tmp, 64, "%Y%m%d%H%M%S", localtime(&t));
	                        fprintf(obj->logfp, "%s \n", tmp);
                            fflush(obj->logfp);
                        }
                    }
                    if (obj->logfp)
                    {
                        char tmp[128] = "RenewCountLossRateInfo: ";
	                    time_t t = time(0);
	                    strftime(&tmp[strlen(tmp)], 64, "%Y%m%d%H%M%S", localtime(&t));
	                    fprintf(obj->logfp, "%s \n", tmp);
	                    fprintf(obj->logfp, "RenewCountLossRateInfo: obj->delay_time=%d ms \n", obj->delay_time);
                        //fprintf(obj->logfp, "RenewCountLossRateInfo: sum_pkt=%d \n", sum_pkt);
                        //fprintf(obj->logfp, "RenewCountLossRateInfo: head->last_pkt_num=%d \n", head->last_pkt_num);
                        fprintf(obj->logfp, "RenewCountLossRateInfo: ret=%d \n", ret);
                        fflush(obj->logfp);
                    }
#endif
                }
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
    else{
        //MYPRINT("RenewCountLossRateInfo: udata->head.rtx=%d \n", udata->head.rtx);
    }
    return ret;
}
int JitterPushData(SocketObj *sock, char *recv_buf, int recv_size, int seqnum, int64_t frame_timestamp)
{
    int ret = 0;
    int lossless = 0;
    int enable_netack = 0;
    pthread_mutex_lock(&sock->status_lock);
    SessionObj * session = (SessionObj *)sock->session;
    CallCodecVideo *obj = NULL;
    if(session)
    {
        obj = session->codec;
    }
    pthread_mutex_unlock(&sock->status_lock);
    if(obj)
    {
        CodecInfoObj *config = (CodecInfoObj *)&obj->codecInfo;
        lossless = config->lossless;
        enable_netack = config->enable_netack;
    }
    else{
        MYPRINT("JitterPushData: obj=%x \n", obj);
    }
    MYPRINT("JitterPushData: enable_netack=%d \n", enable_netack);
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
        head->loss_rate = 0;
        head->ref_idc = -1;
        head->next = NULL;  //头节点指针域置NULL
        head->tail = head;  // 开始时尾指针指向头节点；此时head->tail->next与head->next是同一地址
        sock->frameJitterHead = (void *)head;
    }
    head = (FrameJitterNode *)sock->frameJitterHead;
    int lossrate = 0;
#ifdef OPEN_LOSSNET
    if(!lossless)
    {
        lossrate = RenewCountLossRateInfo(obj, head, recv_buf, recv_size, seqnum, frame_timestamp);
        //
#if 0
        AVData *udata = (AVData *)recv_buf;
        int head_size = sizeof(AVData) - MAX_DATA_SIZE;
        int raw_size = udata->head.size - head_size;
        int lossrate2 = api_count_loss_rate2(obj->handle, udata->data, raw_size, 90);
        if(lossrate2 > 0)
        {
            //MYPRINT("JitterPushData: lossrate=%d, lossrate2=%d \n", lossrate, lossrate2);
            //MYPRINT("JitterPushData: lossrate2=%d \n", lossrate2);
        }
        lossrate = lossrate2;
#endif
        if(lossrate > 0)
        {
            obj->lossrate = lossrate;
            //MYPRINT("JitterPushData: lossrate=%d \n", lossrate);
        }
    }
#endif
    if(head->num > MAX_DELAY_PKT_NUM)
    {
        //free(recv_buf);
        MYPRINT("JitterPushData: skip head->num=%d \n", head->num);
        pthread_mutex_unlock(&sock->lock);
        return -2;
    }
    if(frame_timestamp <= head->last_frame_time)
    {
        int difftime = (int)(head->last_frame_time - frame_timestamp);
        //fec冗余包
        //MYPRINT("JitterPushData: difftime=%d \n", difftime);
        //MYPRINT("JitterPushData: frame_timestamp=%lld \n", frame_timestamp);
        MYPRINT("JitterPushData: head->last_frame_time=%lld \n", head->last_frame_time);
        //free(recv_buf);
        pthread_mutex_unlock(&sock->lock);
        return -1;
    }
    MYPRINT("PushData: head->num=%d \n", head->num);
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
                //MYPRINT("JitterPushData: thisSeqFrame->size=%d \n", thisSeqFrame->size);
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
                            MYPRINT("JitterPushData: udata->head.rtx=%d \n", udata->head.rtx);
#ifdef OPEN_DELAYNET
                            int delay_time = api_renew_delay_time(  obj->handle,
                                                                    (obj->sessionInfo->chanId + 1),
                                                                    udata->data, raw_size,
                                                                    obj->delay_time);
                            //MYPRINT("JitterPushData: delay_time=%d \n", delay_time);
                            if(delay_time > 0)
                            {
                                obj->delay_time = delay_time > MAX_DELAY_TIME ? MAX_DELAY_TIME : delay_time;
                                if(delay_time > 400 || delay_time < 0)
                                {
                                    MYPRINT("JitterPushData: delay_time=%d \n", delay_time);
                                }
                            }
#endif
                            //MYPRINT("JitterPushData: obj->lossrate=%d \n", obj->lossrate);
                            //MYPRINT("JitterPushData: enable_netack=%d \n", enable_netack);
#ifdef OPEN_NETACK
                            if(obj->netInfoObj && enable_netack)
                            {
                                MYPRINT("JitterPushData: obj->lossrate=%d \n", obj->lossrate);
                                push_net_info(obj->netInfoObj, kIsVideo,obj->sessionInfo->chanId, udata->data, raw_size, lossrate);
                            }
#endif
                        }
                        else{
                            MYPRINT2("JitterPushData: udata->head.rtx=%d \n", udata->head.rtx);
                        }
                    }
                    thisSeqFrame->size = recv_size;
                    q->pkt_sum_size += recv_size;
                    q->pkt_num++;
                }
                else{
                    //重复包
                    //free(recv_buf);
                    ret = -3;
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

        MYPRINT("PushData: renewflag=%d \n", renewflag);
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
                int factor = 1;//10;
                if(!pnew->max_data_size)
                {
                    MYPRINT("JitterPushData: pnew->max_data_size=%d \n", pnew->max_data_size);
                }
                int m = (pnew->frame_size % pnew->max_data_size) > 0;
                pnew->max_pkt_num = factor * (pnew->frame_size / pnew->max_data_size + m);
                pnew->rtp_pkt_num = pnew->max_pkt_num;
            }
            else{
                RtpInfo info = {0};
                info.raw_offset = -1;
                int head_size = sizeof(AVData) - MAX_DATA_SIZE;
                AVData *udata = (AVData *)recv_buf;
                int raw_size = udata->head.size - head_size;
                int is_hcsvc_rtp = GetRtpInfo((uint8_t*)udata->data, raw_size, &info, H264_PLT);
                if(!info.max_pkt_num)
                {
                    MYPRINT("error:JitterPushData: info.max_pkt_num=%d \n", info.max_pkt_num);
                    MYPRINT("error:JitterPushData: info.rtp_pkt_num=%d \n", info.rtp_pkt_num);
                    MYPRINT("error:JitterPushData: iis_hcsvc_rtp=%d \n", is_hcsvc_rtp);
                }
                pnew->rtp_pkt_num = info.rtp_pkt_num;
                pnew->max_pkt_num = info.max_pkt_num;
                pnew->start_seqnum = info.start_seqnum;
                pnew->ref_idx = info.ref_idx;
                pnew->ref_idc = info.ref_idc;
                //MYPRINT("JitterPushData: info.ref_idc=%d \n", info.ref_idc);
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
                    //MYPRINT("JitterPushData: obj->lossrate=%d \n", obj->lossrate);
#ifdef OPEN_NETACK
                    if(obj->netInfoObj && enable_netack)
                    {
                        push_net_info(obj->netInfoObj, kIsVideo,obj->sessionInfo->chanId, udata->data, raw_size, obj->lossrate);
                    }
#endif
                }
                else{
                    MYPRINT2("JitterPushData: udata->head.rtx=%d \n", udata->head.rtx);
                }
            }
            MYPRINT("JitterPushData: pnew->max_pkt_num=%d \n", pnew->max_pkt_num);
            pnew->seqFrame = calloc(1, pnew->max_pkt_num * sizeof(SeqFrame));
            pnew->has_rtx = 0;
            pnew->pkt_num = 0;
            pnew->min_seqnum = seqnum;
            pnew->max_seqnum = seqnum;
            pnew->frame_timestamp = frame_timestamp;
            //MYPRINT("JitterPushData: frame_timestamp=%lld \n", frame_timestamp);
            pnew->now_time = api_get_time_stamp_ll();
            //
            if(!pnew->max_pkt_num)
            {
                MYPRINT("video JitterPushData: pnew->max_pkt_num=%d \n", pnew->max_pkt_num);
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
                    //找到了时间戳更大的q
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
                //如果没找到时间戳更大的，则意味着pnew时间戳最大，且q=NULL
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
            MYPRINT("PushData: head->num=%d \n", head->num);
        }
    }

#endif
    pthread_mutex_unlock(&sock->lock);
    MYPRINT("PushData: ret=%d \n", ret);
    return ret;
}
int generate_nack(SocketObj *sock, CallCodecVideo *obj)
{
    int ret = 0;

    FrameJitterNode *head, *p, *q;
    head = (FrameJitterNode *)sock->frameJitterHead;
    q = head->next;
    if(q->has_rtx)
    {
        return 1;
    }
    if(!q->max_pkt_num)
    {
        MYPRINT("error:generate_nack: q->max_pkt_num=%d \n", q->max_pkt_num);
    }
    int rtp_pkt_num = q->rtp_pkt_num;
    int max_pkt_num = q->max_pkt_num;
    int start_seqnum = q->start_seqnum;//来自于rtp扩展数据
    int min_seqnum = q->min_seqnum;
    if(!max_pkt_num)
    {
        MYPRINT("generate_nack: max_pkt_num=%d \n", max_pkt_num);
    }
    int i = min_seqnum % max_pkt_num;
    SeqFrame *thisSeqFrame = &q->seqFrame[i];
    RtpInfo info = {0};
    info.raw_offset = -1;
    int head_size = sizeof(AVData) - MAX_DATA_SIZE;
    AVData *udata = (AVData *)thisSeqFrame->data;
    int raw_size = udata->head.size - head_size;
    int is_hcsvc_rtp = GetRtpInfo((uint8_t*)udata->data, raw_size, &info, H264_PLT);
    if(is_hcsvc_rtp)
    {
        if(1)//(info.refs == 1)
        {
            int diff_num = abs(q->ref_idx - head->ref_idx);
            if(diff_num != 1)
            {
                //loss frame
                MYPRINT("generate_nack: diff_num=%d \n", diff_num);
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
            MYPRINT("generate_nack: bidx=%d \n", bidx);
            MYPRINT("generate_nack: idx=%d \n", idx);
            MYPRINT("generate_nack: zero_sum=%d \n", zero_sum);
            MYPRINT("generate_nack: rtp_pkt_num32=%d \n", rtp_pkt_num32);
            MYPRINT("generate_nack: rtp_pkt_num128=%d \n", rtp_pkt_num128);
            if(bidx)
            {
                int jj = 0;
                for(int ii = 0; ii < bidx; ii++)
                {
                    NACK_LOSS nackLoss;
                    nackLoss.start_seqnum = useqnums[ii];
                    nackLoss.loss1 = 0xFFFFFFFF;
                    nackLoss.loss2 = 0xFFFFFFFF;
                    nackLoss.loss3 = 0xFFFFFFFF;
                    nackLoss.loss4 = 0xFFFFFFFF;
                    MYPRINT("generate_nack: idx=%d \n", idx);
                    if(jj < idx)
                    {
                        nackLoss.loss1 = urecvd[jj]; jj++;
                    }
                    MYPRINT("generate_nack: jj=%d \n", jj);
                    if(jj < idx)
                    {
                        nackLoss.loss2 = urecvd[jj]; jj++;
                    }
                    if(jj < idx)
                    {
                        nackLoss.loss3 = urecvd[jj]; jj++;
                    }
                    if(jj < idx)
                    {
                        nackLoss.loss4 = urecvd[jj]; jj++;
                    }
                    MYPRINT("generate_nack: jj=%d \n", jj);
                    nackLoss.send_time = q->frame_timestamp & 0xFFFFFFFF;
                    if(obj->sessionInfo && obj->netInfoObj)
                    {
                        nackLoss.chanId = obj->sessionInfo->chanId + 1;
                        push_rtx_info(obj->netInfoObj, &nackLoss);
                    }
                    MYPRINT("generate_nack: push_rtx_info end \n");
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
void * JitterPopData(SocketObj *sock)
{
    void *ret = NULL;
    int lossless = 0;
    int nack_time = MIN_RTT;
    int max_delay_time = nack_time << 1;

    pthread_mutex_lock(&sock->status_lock);
    SessionObj * session = (SessionObj *)sock->session;
    CallCodecVideo *obj = NULL;
    if(session)
    {
        obj = session->codec;
        if(obj)
        {
            CodecInfoObj *config = (CodecInfoObj *)&obj->codecInfo;
            lossless = config->lossless;
            int rtt = obj->delay_time << 1;//网络往返时间
            nack_time = rtt > MIN_RTT ? rtt : MIN_RTT;
            max_delay_time = nack_time << 1;//留出重传的时间;
            //需要进行播放平滑处理
            //MYPRINT("JitterPopData: max_delay_time=%d \n", max_delay_time);
        }
    }
    //max_delay_time = 100;//test
    pthread_mutex_unlock(&sock->status_lock);

    pthread_mutex_lock(&sock->lock);
    FrameJitterNode *head, *p, *q;
    head = (FrameJitterNode *)sock->frameJitterHead;
    if(head)
    {
        q = head->next;//1st frame
        if(q == NULL || q == head)
        {

        }
        else{
            int flag = 0;
            if(head->num > 0 || q->pkt_num >= q->rtp_pkt_num)
            {
                int64_t now_time = get_sys_time();
                int delay_time0 = now_time - head->last_frame_time;//q->frame_timestamp;
                p = head->tail;//end frame
                int delay_time = p->frame_timestamp - q->frame_timestamp;//保证有1帧以上的数据
                int is_overflow = head->num > 2;
                //与丢包率统计是否一致？
                is_overflow &= ((delay_time0 > max_delay_time && delay_time > nack_time) ||
                                delay_time0 > (max_delay_time << 1) ||
                                delay_time > max_delay_time);
                int is_rtx = head->num > 1;
                is_rtx &= (delay_time0 > nack_time && delay_time > 0) || delay_time > nack_time;

                if(q->pkt_num >= q->rtp_pkt_num && q->max_pkt_num <= 255)
                {
                    head->last_frame_time = q->frame_timestamp;
                    head->ref_idx = q->ref_idx;
                    head->num--;
                    flag = 1;
                }
                else if(q->max_pkt_num > 255 && (q->pkt_num - q->rtp_pkt_num) > 5)//???
                {
                    head->last_frame_time = q->frame_timestamp;
                    head->ref_idx = q->ref_idx;
                    head->num--;
                    flag = 1;
                }
                //if((delay_time0 > max_delay_time || delay_time > max_delay_time) && head->num > 2)
                //if((delay_time0 > max_delay_time || delay_time > max_delay_time) && head->num > 1)
                else if(is_overflow)
                {
#ifdef SHOW_DELAY
                    MYPRINT("[ JitterPopData: delay_time0=%d \n", delay_time0);
                    MYPRINT("JitterPopData: delay_time=%d \n", delay_time);
                    MYPRINT("JitterPopData: max_delay_time=%d \n", max_delay_time);
                    MYPRINT("JitterPopData: q->pkt_num=%d \n", q->pkt_num);
                    MYPRINT("JitterPopData: q->rtp_pkt_num=%d \n", q->rtp_pkt_num);
                    MYPRINT("JitterPopData: q->max_pkt_num=%d \n", q->max_pkt_num);
                    MYPRINT("JitterPopData: head->num=%d ] \n", head->num);

#endif
                    head->last_frame_time = q->frame_timestamp;
                    head->ref_idx = q->ref_idx;
                    head->num--;

                    q->loss_rate = (int)((1.0 - (float)q->pkt_num / (float)q->rtp_pkt_num) * 100);//for skip condition
                    int loss_rate = (int)((1.0 - (float)q->pkt_num / (float)q->max_pkt_num) * 100);
                    MYPRINT("JitterPopData: q->loss_rate=%d, loss_rate=%d \n", q->loss_rate, loss_rate);

                    flag = 1;
#ifdef SHOW_DELAY
                    if(!obj->logfp)
                    {
                        char filename[256] = "lossrate_gxh_";
                        char ctmp[32] = "";
                        int fileidx = obj->sessionInfo->chanId;
                        sprintf(ctmp, "%d", fileidx);
                        strcat(filename, ctmp);
                        strcat(filename, ".txt");
                        obj->logfp = fopen(filename, "w");
                        if(obj->logfp)
                        {
                            char tmp[64];
	                        time_t t = time(0);
	                        strftime(tmp, 64, "%Y%m%d%H%M%S", localtime(&t));
	                        fprintf(obj->logfp, "%s \n", tmp);
                            fflush(obj->logfp);
                        }
                    }
                    if (obj->logfp)
                    {
                        char tmp[128] = "JitterPopData: ";
	                    time_t t = time(0);
	                    strftime(&tmp[strlen(tmp)], 64, "%Y%m%d%H%M%S", localtime(&t));
	                    fprintf(obj->logfp, "%s \n", tmp);
	                    fprintf(obj->logfp, "JitterPopData: obj->delay_time=%d ms \n", obj->delay_time);
                        fprintf(obj->logfp,"JitterPopData: delay_time0=%d \n", delay_time0);
                        fprintf(obj->logfp,"JitterPopData: delay_time=%d \n", delay_time);
                        fprintf(obj->logfp,"JitterPopData: max_delay_time=%d \n", max_delay_time);
                        fprintf(obj->logfp,"JitterPopData: q->pkt_num=%d \n", q->pkt_num);
                        fprintf(obj->logfp,"JitterPopData: q->rtp_pkt_num=%d \n", q->rtp_pkt_num);
                        fprintf(obj->logfp,"JitterPopData: q->max_pkt_num=%d \n", q->max_pkt_num);
                        fprintf(obj->logfp,"JitterPopData: q->loss_rate=%d \n", q->loss_rate);
                        fprintf(obj->logfp,"JitterPopData: loss_rate=%d \n", loss_rate);
                        fprintf(obj->logfp,"JitterPopData: head->num=%d \n", head->num);
                        fflush(obj->logfp);
                    }
#endif
                }
                else if(is_rtx)
                {
                    //nack
                    if(!lossless && OPEN_RTX)
                    {

                        int ret2 = generate_nack(sock, obj);
                        if(!ret2)
                        {
                            MYPRINT2("JitterPopData: rtx: delay_time0=%d, delay_time=%d, nack_time=%d, head->num=%d \n", delay_time0, delay_time, nack_time, head->num);
                        }

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
                    int is_hcsvc_rtp = GetRtpInfo((uint8_t*)udata->data, raw_size, &info, H264_PLT);
                    if(!is_hcsvc_rtp)
                    {

                    }
#endif
                }
            }
            if(flag)
            {
                ret = q;
                //MYPRINT("JitterPopData: q->frame_timestamp=%lld \n", q->frame_timestamp);
                //MYPRINT("JitterPopData: q=%x \n", q);
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

static uint8_t * distill_frame(SocketObj *sock, FrameJitterNode *thisFrame, int *pwidth, int *pheight)
{
    uint8_t *ret = NULL;
    pthread_mutex_lock(&sock->status_lock);
    SessionObj * session = (SessionObj *)sock->session;
    CallCodecVideo *obj = session->codec;
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
            frame_size = (config->screen_width * config->screen_width * 3) >> 1;//test
            if(config->width > 0 && config->height > 0)
            {
                frame_size = (config->width * config->height * 3) >> 1;
            }
            //MYPRINT("distill_frame: config->width=%d, config->height=%d \n", config->width, config->height);
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
            int is_hcsvc_rtp = GetRtpInfo((uint8_t*)pktbuf, rtpSize[0], &info, H264_PLT);
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
                obj->json = api_renew_json_array(obj->json, "rtpSize", rtpSize, rtpNum);
                char *jsonstr = api_json2str(obj->json);
                //MYPRINT("distill_frame: 1: jsonstr=%s \n", jsonstr);
                int ret2 = api_rtp_packet2raw(  obj->handle,
                                                src,
                                                jsonstr,
                                                dst,
                                                obj->outparam);
                //MYPRINT("distill_frame: 1: ret2=%d \n", ret2);
                obj->json = api_delete_item(obj->json, "rtpSize");
                api_get_array_free(rtpSize);
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
                    ret2 = api_video_decode_one_frame(  obj->handle,
                                                        src,
                                                        jsonstr,
                                                        dst,
                                                        obj->outparam);
                    //MYPRINT("distill_frame: 3: ret2=%d \n", ret2);
                    obj->json = api_delete_item(obj->json, "insize");
                    api_json2str_free(jsonstr);
                    free(src);
                    if(ret2 <= 0)
                    {
                        free(dst);
                        dst = NULL;
                    }
                    else{
                        config->width = atoi(obj->outparam[1]);
                        config->height = atoi(obj->outparam[2]);
                        pwidth[0] = config->width;
                        pheight[0] = config->height;
                        //MYPRINT("distill_frame: config->width=%d, config->height=%d \n", config->width, config->height);
                    }
                    ret = dst;
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
static int is_skip(CallCodecVideo *obj, int loss_rate, int ref_idc, int threshold)
{

    if(obj->lossrate > 50)
    {
        if(loss_rate > 5 || obj->push_delay_time > threshold)
        {
            MYPRINT("is_skip: obj->lossrate=%d \n", obj->lossrate);
            MYPRINT("is_skip: loss_rate=%d \n", loss_rate);
            MYPRINT("is_skip: ref_idc=%d \n", ref_idc);
            MYPRINT("is_skip: obj->idr_status =%d \n", obj->idr_status );

            obj->idr_status = ref_idc + 1;//test
            if(ref_idc == 1)//  # P
            {
                obj->idr_status = ref_idc + 1;
            }
            else if(ref_idc == 2)//  # PB
            {
                obj->idr_status = ref_idc + 1;
            }
            else if(ref_idc == 3)//  # B
            {
                obj->idr_status = ref_idc + 1;
            }
        }
        else
        {
            if(ref_idc == 0)//  # I
            {
                obj->idr_status = 0;
            }
            else if((ref_idc + 1) < obj->idr_status)
            {
                obj->idr_status = 0;
            }
            else if (((ref_idc + 1) == obj->idr_status) &&
                ((ref_idc == 2) || (ref_idc == 3)))
            {
                //obj->idr_status = 0;
            }
        }
    }
    else{
        if(loss_rate > 5 || obj->push_delay_time > threshold)
        {
            if(ref_idc == 2)//  # PB
            {
                obj->idr_status = ref_idc + 1;
            }
            else if(ref_idc == 3)//  # B
            {
                obj->idr_status = ref_idc + 1;
            }
        }
        else
        {
            if(ref_idc == 0)//  # I
            {
                obj->idr_status = 0;
            }
            else if((ref_idc + 1) < obj->idr_status)
            {
                obj->idr_status = 0;
            }
            else if (((ref_idc + 1) == obj->idr_status) &&
                ((ref_idc == 2) || (ref_idc == 3)))
            {
                obj->idr_status = 0;
            }
        }
    }
    return obj->idr_status;
}
//纯数据处理，此处是瓶颈
int pkt_resort(SocketObj *obj, int *decodeNum)
{
    int ret = 0;
    pthread_mutex_lock(&obj->status_lock);
    SessionObj *session = (SessionObj *)obj->session;
    CallCodecVideo *codec = NULL;
    if(session)
    {
        codec = session->codec;
    }
    pthread_mutex_unlock(&obj->status_lock);
    DataNode *head = (DataNode *)PopDataAll(obj);

    struct sockaddr_in addr_client;
    if(head && head->num)
    {
        int pop_delay_time = 0;
#if 1
        DataNode *q;
        if(codec)
        {
            codec->push_delay_time = 0;
        }

        if(head->num > 10)
        {
            q = head->next;
            UserData *udata = (UserData *)q->data;
            if(udata->data0.head.datatype)
            {
                int64_t now_time0 = udata->data0.now_time;
                int64_t now_time = get_sys_time();
                pop_delay_time = (int)(now_time - now_time0);
                if(pop_delay_time > 1000)
                {
                    MYPRINT("pkt_resort: pop_delay_time=%d, head->num=%d \n", pop_delay_time, head->num);
                }
                if(codec)
                {
                    codec->push_delay_time = pop_delay_time;
                }
            }
        }
#endif
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
                int dataChanId = udata->data0.chanId;
                MYPRINT("pkt_resort: dataChanId=%d \n", dataChanId);
                //MYPRINT("pkt_resort: now_time=%lld \n", now_time);
                int seqnum = udata->data0.seqnum;
                int size = udata->data0.head.size;
                uint8_t *data = (uint8_t *)recv_buf;//udata->data0.data;
                //MYPRINT("pkt_resort: size=%d \n", size);
                int ret2 = JitterPushData(obj, data, size, seqnum, now_time);
                if(ret2 < 0)
                {
                    //MYPRINT("pkt_resort: ret2=%d \n", ret2);//old packet
                    free(q->data);
                }
                MYPRINT("pkt_resort: ret2=%d \n", ret2);
                FrameJitterNode *thisFrame = (FrameJitterNode *)JitterPopData(obj);
                if(thisFrame)
                {
                    int pkt_sum_size = thisFrame->pkt_sum_size;

                    if(session)
                    {
                        int skip = is_skip(codec, thisFrame->loss_rate, thisFrame->ref_idc, 1000);
                        if(skip)
                        {
                            MYPRINT("pkt_resort: codec->push_delay_time=%d \n", codec->push_delay_time);
                            for(int i = thisFrame->min_seqnum; i <= thisFrame->max_seqnum; i++)
                            {
                                int j = i % thisFrame->max_pkt_num;
                                SeqFrame *thisSeqFrame = &thisFrame->seqFrame[j];
                                if(thisSeqFrame && thisSeqFrame->data)
                                {
                                    free(thisSeqFrame->data);
                                }
                            }
                            free(thisFrame->seqFrame);
                            free(thisFrame);
                            thisFrame = NULL;
                        }
                        else
                        {
                            int chanId = session->sessionInfo->chanId;
                            MYPRINT("pkt_resort: thisFrame->frame_timestamp=%lld \n", thisFrame->frame_timestamp);
                            int64_t frame_timestamp = thisFrame->frame_timestamp;
                            int width = 0;
                            int height = 0;
                            uint8_t *frame_data = distill_frame(obj, thisFrame, &width, &height);
                            if(frame_data)
                            {
                                ret += pkt_sum_size;
                                decodeNum[0] += 1;
                                //MYPRINT("pkt_resort: width=%d, height=%d \n", width, height);
                                //MYPRINT("pkt_resort: session->render=%x \n", session->render);
                                if(session->render)
                                {
                                    int selfmode = session->sessionInfo->selfmode;
                                    int chanId = session->sessionInfo->chanId;
                                    int actor = session->sessionInfo->actor;
                                    if(selfmode)
                                    {
                                        chanId = (actor < DECACTOR) ? 1 : 0;
                                    }
                                    MYPRINT("pkt_resort: selfmode=%d, actor=%d, chanId=%d \n", selfmode, actor, chanId);
                                    FramePushData(session->render, frame_data, width, height, chanId, frame_timestamp, frame_timestamp);
                                    MYPRINT("pkt_resort: frame_timestamp=%lld \n", frame_timestamp);
                                }
                                else{
                                    //MYPRINT("pkt_resort: frame_data=%x \n", frame_data);
                                    free(frame_data);
                                    frame_data = NULL;
                                    //MYPRINT("pkt_resort: 2: frame_data=%x \n", frame_data);
                                }
                            }
                            else{
                                MYPRINT("pkt_resort: frame_data=%x, chanId=%d, dataChanId=%d \n", frame_data, chanId, dataChanId);
                            }
                        }
                    }
                    else{
                        MYPRINT("pkt_resort: session=%x \n", session);
                        if(thisFrame->seqFrame)
                        {
                            free(thisFrame->seqFrame);
                        }
                        free(thisFrame);
                    }
                }
                else{
                    //MYPRINT("pkt_resort: thisFrame=%x \n", thisFrame);
                }
            }
            else{
                //send_num = ProcessCmd(obj, send_buf, send_num, addr_client, sock_fd, len, now_time);
                int cmdtype = udata->data1.cmdtype;
                MYPRINT("pkt_resort: head->num=%d \n", head->num);
                MYPRINT("pkt_resort: q->size=%d \n", q->size);
                MYPRINT("pkt_resort: q=%x \n", q);
                MYPRINT("pkt_resort: cmdtype=%d \n", cmdtype);
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
int decode_frame_run(SocketObj *sock)
{
    int ret = 0;

    pthread_mutex_lock(&sock->status_lock);
    sock->send_status = 1;
    int status = sock->send_status;
    SessionObj * session = (SessionObj *)sock->session;
    CallCodecVideo *obj = session->codec;
    pthread_mutex_unlock(&sock->status_lock);

    int frame_num = 0;
    int64_t sumbytes = 0;
    int frame_size = (DEFAULT_WIDTH * DEFAULT_HEIGHT * 3) >> 1;
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
            if(framerate > 0)
                MYPRINT("decode_frame_run: framerate=%2.1f (fps) \n", framerate);
            if(bitrate > 0)
                MYPRINT("decode_frame_run: bitrate=%5.1f (Mbps) \n", bitrate);
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


char * get_cpx(int w, int h)
{
    char *ret = "superfast";
    if((w * h) <= 352 * 288)
    {
        ret = "medium";
        ret = "fast";
        ret = "veryfast";
    }
    else if((w * h) <= 704 * 576)
    {
        ret = "fast";
        ret = "veryfast";
    }
    else if((w * h) <= 1280 * 720)
    {
        ret = "veryfast";
    }
    else if((w * h) <= 1920 * 1080)
    {
        //#ret = "veryfast"
        ret = "superfast";
        //#ret = "ultrafast"
    }
    else
    {
        ret = "superfast";
    }
    return ret;

}
int video_set_capture(SocketObj *obj, CallCapture *capture, int capture_status)
{
    int ret = 0;
    pthread_mutex_lock(&obj->status_lock);
    if(capture_status < 0)
    {
        MYPRINT("video_set_capture: capture_status=%d \n", capture_status);
        obj->recv_status = 0;
        pthread_mutex_unlock(&obj->status_lock);
        return -1;
    }
    else if(!obj->session)
    {
        MYPRINT("video_set_capture: obj->session=%x \n",obj->session);
        //obj->recv_status = 0;
        pthread_mutex_unlock(&obj->status_lock);
        return -1;
    }
    else{
        SessionObj * session = (SessionObj *)obj->session;
        session->capture = capture;
        pthread_mutex_unlock(&obj->status_lock);
        MYPRINT("video_set_capture: obj=%x \n",obj);
        MYPRINT("video_set_capture: session=%x \n",session);
        MYPRINT("video_set_capture: capture=%x \n", capture);
    }
    return ret;
}
int video_set_render(SocketObj *obj, CallRender *render)
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
        session->render = render;
        MYPRINT("video_set_render: render=%x \n", render);
        if(session->sessionInfo)
        {
            int selfmode = session->sessionInfo->selfmode;
            int actor = session->sessionInfo->actor;
            int modeId = session->sessionInfo->modeId;
            int chanId = session->sessionInfo->chanId;
            int width = session->sessionInfo->screen_width;
            int height = session->sessionInfo->screen_height;
            //width = 1920;//test
            //height = 1080;
            if(selfmode)
            {
                chanId = (actor < DECACTOR) ? 1 : 0;
            }
            MYPRINT("video_set_render: selfmode=%d \n", selfmode);
            MYPRINT("video_set_render: actor=%d \n", actor);
            MYPRINT("video_set_render: chanId=%d \n", chanId);
            int num = 0;
            int maxLayerId = render->pMultLayer->maxLayerId;
            SDL_Rect rect = RectMap(&render->pMultLayer->pRect, modeId, chanId, width, height, &num ,&maxLayerId);
            //AddRect(render, rect, chanId);
            MYPRINT("video_set_render: maxLayerId=%d, modeId=%D \n", maxLayerId, modeId);
            if(render->pMultLayer->pRect)
            {
                render->pMultLayer->modeId = modeId;
                render->pMultLayer->ways = num;
                render->pMultLayer->maxLayerId = maxLayerId;
                MYPRINT("video_set_render: maxLayerId=%d \n", maxLayerId);
                MYPRINT("video_set_render: render->pMultLayer->maxLayerId=%d \n", render->pMultLayer->maxLayerId);
            }
        }
    }
    pthread_mutex_unlock(&obj->status_lock);

    return ret;
}
int video_set_netinfo(SocketObj *obj, NetInfoObj *netInfoObj)
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
        MYPRINT("video_set_netinfo: session->codec=%x \n", session->codec);
        if(session->codec)
        {
            MYPRINT("video_set_netinfo: session->codec->encoder=%x \n", session->codec->encoder);
            if(session->codec->encoder)
            {
                MYPRINT("video_set_netinfo: session->codec->encoder->selfChanId=%d \n", session->codec->encoder->selfChanId);
                AddEncoder(&netInfoObj->lock, &netInfoObj->encoderHead, session->codec->encoder);
            }
            session->codec->netInfoObj = netInfoObj;
            MYPRINT("video_set_netinfo: session->codec->netInfoObj=%x \n", session->codec->netInfoObj);
        }
    }
    pthread_mutex_unlock(&obj->status_lock);
    return ret;
}
int video_set_readstream(SocketObj *sock, char *handle)
{
    int ret = 0;
    pthread_mutex_lock(&sock->status_lock);
    SessionObj * session = (SessionObj *)sock->session;
    CallCodecVideo *obj = (CallCodecVideo *)session->codec;
    memcpy(session->handle, handle, 8);
    pthread_mutex_unlock(&sock->status_lock);
    return ret;
}
int video_set_stream(SocketObj *sock, char *handle, int is_encoder)
{
    int ret = 0;
    pthread_mutex_lock(&sock->status_lock);
    SessionObj * session = (SessionObj *)sock->session;
    CallCodecVideo *obj = (CallCodecVideo *)session->codec;// = obj;
    MYPRINT("video_set_stream: obj->handle=%x \n", obj->handle);
    MYPRINT("video_set_stream: handle=%x \n", handle);
    set_avstream2video(obj->handle, handle);
    pthread_mutex_unlock(&sock->status_lock);
    return ret;
}
int video_set_codec(SocketObj *sock, SessionInfoObj *sessionInfo, int is_encoder)
{
    int ret = 0;
    pthread_mutex_lock(&sock->status_lock);
    SessionObj * session = (SessionObj *)sock->session;
    CallCodecVideo *obj = (CallCodecVideo *)calloc(1, sizeof(CallCodecVideo));
    obj->sock = (void *)sock;
    //
    MYPRINT("video_set_codec: %d \n", is_encoder);
    pthread_mutex_init(&obj->lock,NULL);

    obj->sessionInfo = sessionInfo;

    char *params = "{\"print\":0}";
    obj->json = (cJSON *)api_str2json(params);

    CodecInfoObj *config = (CodecInfoObj *)&obj->codecInfo;
    config->lossless = LOSSLESS;
    if(is_encoder)
    {
        int w = sessionInfo->width;
        int h = sessionInfo->height;
        obj->frame_size = (w * h * 3) >>1;
        char *cpx = get_cpx(w, h);

        //int mb_w = (w >> 4) + ((w % 16) ? 1 : 0);
        //int mb_h = (h >> 4) + ((h % 16) ? 1 : 0);
        //int bmb_w = (mb_w >> 3) + ((mb_w % 8) ? 1 : 0);
        //int bmb_h = mb_h;
        //obj->bmb_size = bmb_w * bmb_h;

        x264_get_function((void **)&obj->x264_hnd);

        //x264_t *testx = NULL;
        //x264_encoder_close  ( testx );

        int codec_mode = 0;
        if(!strcmp(sessionInfo->scodec, "qsv264"))
        {
            codec_mode = 1;
        }
        else if(!strcmp(sessionInfo->scodec, "wz264"))
        {
            codec_mode = 2;
        }
        config->bwthreshold = sessionInfo->bwthreshold;
        config->loss_rate = sessionInfo->lossrate;
        config->codec_mode = codec_mode;
        config->gop_size = 50;
        config->refs = sessionInfo->refs;//16;//1;
        config->max_refs = 16;
        config->max_b_frames = 0;
        config->frame_rate = sessionInfo->framerate;//25;
        config->enable_fec = sessionInfo->fecenable;//1;
        //config->loss_rate = 0;//0.9;//0.05;
        config->code_rate = (1.0 - config->loss_rate);
        config->width = w;
        config->height = h;
        config->screen_width = sessionInfo->screen_width;
        config->screen_height = sessionInfo->screen_height;
        config->enable_netack = sessionInfo->enable_netack;
        config->bit_rate = sessionInfo->bitrate;//2*1024000;
        config->mtu_size = sessionInfo->mtu_size;//MTU_SIZE;

        MYPRINT("video_set_codec: config->bwthreshold=%2.3f \n", config->bwthreshold);
        MYPRINT("video_set_codec: config->loss_rate=%2.3f \n", config->loss_rate);

        config->qmin = 20;
        config->qmax = 30;//40;
        //3Mbps,1080P
        //config->qmin = 25;
        //config->qmax = 32;//40;

        config->qmin = 25;
        config->qmax = 31;//40;

        config->qmin = 25;
        config->qmax = 34;//40;

        //固定QP
        //config->qmin = 32;
        //config->qmax = 32;

        config->osd = sessionInfo->osd_enable;//1;
        config->adapt_cpu = 0;
        config->main_spatial_idx = 0;
        config->fec_level = sessionInfo->fec_level;//2;
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
        api_video_encode_open(obj->handle, jsonStr);

        for(int l = 0; l < 4; l++)
        {
            obj->outparam[l] = NULL;
        }
        obj->outbuffer = malloc(obj->frame_size * sizeof(uint8_t));
        obj->rtpbuffer = malloc(obj->frame_size * sizeof(uint8_t));

        //obj->nettime = 0;

        obj->encoder = calloc(1, sizeof(Encoder));
        obj->encoder->avtype = kIsVideo;
        obj->encoder->selfChanId = sessionInfo->chanId;
        MYPRINT("video_set_codec: sessionInfo->chanId=%d \n", sessionInfo->chanId);
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
        MYPRINT("video_set_codec: obj=%x \n", obj);
    }
    else{
        //obj->nettime = 0;
        int codec_mode = 0;
        if(!strcmp(sessionInfo->scodec, "qsv264"))
        {
            codec_mode = 1;
        }
        else if(!strcmp(sessionInfo->scodec, "wz264"))
        {
            //codec_mode = 2;
        }
        config->osd = sessionInfo->osd_enable;
        config->width = sessionInfo->width;
        config->height = sessionInfo->height;
        config->screen_width = sessionInfo->screen_width;
        config->screen_height = sessionInfo->screen_height;
        config->enable_netack = sessionInfo->enable_netack;

        obj->json = api_renew_json_int(obj->json , "codec_mode", codec_mode);//2
        obj->delay_time = 100;
        obj->json = api_renew_json_int(obj->json , "oneframe", 1);
        obj->json = api_renew_json_int(obj->json , "osd", config->osd);
        obj->json = api_renew_json_int(obj->json , "orgX", 0);
        obj->json = api_renew_json_int(obj->json , "orgY", 0);
        obj->json = api_renew_json_int(obj->json , "scale", 0);
        obj->json = api_renew_json_int(obj->json , "color", 4);
        obj->json = api_renew_json_int(obj->json , "adapt_cpu", config->adapt_cpu);
        char *jsonStr = cJSON_Print(obj->json );
        api_video_decode_open(obj->handle, jsonStr);
        for(int l = 0; l < 4; l++)
        {
            obj->outparam[l] = NULL;
        }
    }
    MYPRINT("video_set_codec: config->enable_netack=%d \n", config->enable_netack);
    //
    session->codec = obj;
    pthread_mutex_unlock(&sock->status_lock);
    MYPRINT("video_set_codec: ok \n");
    return ret;
}

void video_close_codec(SocketObj *sock, int is_encoder)
{
    pthread_mutex_lock(&sock->status_lock);
    SessionObj * session = (SessionObj *)sock->session;
    CallCodecVideo *obj = session->codec;
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
        //if(obj->img_convert_ctx)
        //{
        //    sws_freeContext(obj->img_convert_ctx);
        //    obj->img_convert_ctx = NULL;
        //}
        if(is_encoder)
        {
            api_video_encode_close(obj->handle);
        }
        else{
            api_video_decode_close(obj->handle);
        }
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
        //if(obj->sadHnd)
        //{
        //    I2SADHndClose(obj->sadHnd);
        //}
        if(obj->x264_hnd)
        {
            x264_hnd_free(obj->x264_hnd);
        }
        free(obj);
    }
    pthread_mutex_lock(&sock->status_lock);
    session->codec = NULL;
    pthread_mutex_unlock(&sock->status_lock);

}

//socket send
void * video_encode(SocketObj *obj)
{
    MYPRINT("video_encode: 0 \n");
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
        perror("video_encode: socket");
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
		perror("video_encode error:");
		return 0;
	}
	ret = setsockopt(obj->sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, len);
	if (ret == -1) {
	    perror("video_encode error:");
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
    MYPRINT("video_encode: 1 \n");
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
    MYPRINT("video_encode: obj->params=%s \n", obj->params);
    config->chanId = GetvalueInt(json, "chanId");
    config->actor = GetvalueInt(json, "actor");
    config->sessionId = GetvalueInt(json, "sessionId");
    config->idx = GetvalueInt(json, "idx");
    config->modeId = GetvalueInt(json, "modeId");
    config->avtype = GetvalueInt(json, "avtype");
    config->selfmode = GetvalueInt(json, "selfmode");
    config->testmode = GetvalueInt(json, "testmode");
    config->nettime = GetvalueInt(json, "nettime");
    config->width = GetvalueInt(json, "width");
    config->height = GetvalueInt(json, "height");
    config->screen_width = GetvalueInt(json, "screen_width");
    config->screen_height = GetvalueInt(json, "screen_height");
    config->enable_netack = GetvalueInt(json, "enable_netack");
    config->status = GetvalueInt(json, "status");
    ///config->streamName = GetvalueStr(json, "yuvfp");
    config->bitrate = GetvalueInt(json, "bitrate");
    config->mtu_size = GetvalueInt(json, "mtu_size");
    config->fecenable = GetvalueInt(json, "fecenable");
    config->fec_level = GetvalueInt(json, "fec_level");
    config->buffer_shift = GetvalueInt(json, "buffer_shift");
    config->denoise = GetvalueInt(json, "denoise");
    config->osd_enable = GetvalueInt(json, "osd_enable");
    config->framerate = GetvalueInt(json, "framerate");
    config->refs = GetvalueInt(json, "refs");
    config->bwthreshold = GetvalueFloat(json, "bwthreshold");
    config->lossrate = GetvalueFloat(json, "lossrate");
    MYPRINT("video_encode: config->bwthreshold=%2.3f \n", config->bwthreshold);
    MYPRINT("video_encode: config->lossrate=%2.3f \n", config->lossrate);
    MYPRINT("video_encode: config->fec_level=%d \n", config->fec_level);
    GetvalueStr(json, "scodec", config->scodec);
    GetvalueStr(json, "yuvfilename", config->yuvfilename);
    //MYPRINT("video_encode: config->streamName=%s \n", config->streamName);
    int w = config->width;
    int h = config->height;
    MYPRINT("video_encode: w=%d, h=%d \n", w, h);
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
        MYPRINT("error: video_encode register fail: ret=%d \n", ret);
        return 0;
    }
    //
    video_set_codec(obj, config, 1);
    //
    //pthread_t tid;
    //if(pthread_create(&tid, NULL, video_codec_enc_run, obj) < 0)
    //{
    //    MYPRINT("server_main: Create video_codec_enc_run failed!\n");
    //}

    StartSend(obj);
    pthread_t tid2;
    if(pthread_create(&tid2, NULL, client_send_run, obj) < 0)
    {
        MYPRINT("video_encode: Create client_send_run failed!\n");
        //exit(0);
    }

    encode_frame_run(obj);

    //StopRecv(obj);

    //char *p0;
    //if (pthread_join(tid, (void**)&p0))
    //{
    //    MYPRINT("video_encode: video_codec_enc_run thread is not exit...\n");
    //}
    //else{
    //    MYPRINT("video_encode: p0=%s \n", p0);
    //    free(p0);
    //}

    char *p1;
    if (pthread_join(tid2, (void**)&p1))
    {
        MYPRINT("video_encode: client_send_run thread is not exit...\n");
    }
    else{
        MYPRINT("video_encode: p1=%s \n", p1);
        free(p1);
    }
    video_close_codec(obj, 1);

#if defined (WIN32)
    closesocket(obj->sock_fd);
#elif defined(__linux__)
    close(obj->sock_fd);
#endif

    pthread_mutex_lock(&obj->status_lock);
    if(obj->session)
    {
        //SessionObj * session = (SessionObj *)obj->session;
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
    MYPRINT("video_encode: over: taskId=%d \n", taskId);
    return 0;
}

//socket recv
void * video_decode(SocketObj *obj)
{
    MYPRINT("video_decode: 0 \n");
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
        perror("video_decode: socket");
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
		perror("video_decode error:");
		return 0;
	}
	ret = setsockopt(obj->sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, len);
	if (ret == -1) {
	    perror("video_decode error:");
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
    config->width = 0;//GetvalueInt(json, "width");
    config->height = 0;//GetvalueInt(json, "height");
    config->status = GetvalueInt(json, "status");
    ///config->streamName = GetvalueStr(json, "yuvfp");
    config->bitrate = GetvalueInt(json, "bitrate");
    config->mtu_size = GetvalueInt(json, "mtu_size");
    config->fecenable = GetvalueInt(json, "fecenable");
    config->fec_level = GetvalueInt(json, "fec_level");
    config->buffer_shift = GetvalueInt(json, "buffer_shift");
    config->denoise = GetvalueInt(json, "denoise");
    config->osd_enable = GetvalueInt(json, "osd_enable");
    config->framerate = GetvalueInt(json, "framerate");
    config->refs = GetvalueInt(json, "refs");
    GetvalueStr(json, "scodec", config->scodec);
    config->enable_netack = GetvalueInt(json, "enable_netack");
    config->screen_width = GetvalueInt(json, "screen_width");
    config->screen_height = GetvalueInt(json, "screen_height");

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
        MYPRINT("error: video_decode register fail: ret=%d \n", ret);
        return 0;
    }
    //===
    MYPRINT("video_decode: 1 \n");
    video_set_codec(obj, config, 0);
    //
    StartRecv(obj);
    pthread_t tid;
    if(pthread_create(&tid, NULL, client_recv_run, obj) < 0)
    {
        MYPRINT("video_decode: Create client_recv_run failed!\n");
        //exit(0);
    }

    decode_frame_run(obj);

    //StopSend(obj);

    char *p0;
    if (pthread_join(tid, (void**)&p0))
    {
        MYPRINT("video_decode: client_recv_run thread is not exit...\n");
    }
    else{
        MYPRINT("video_decode: p0=%s \n", p0);
        free(p0);
    }
    video_close_codec(obj, 0);

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
    obj->status = -1;
    MYPRINT("video_decode: over: taskId=%d \n", taskId);
    return 0;
}
#include "inc.h"
#include "udpbase.h"

extern void * get_mcu(SocketObj *sock);
extern void McuPushData(McuObj *obj, uint8_t *data, int size, int64_t now_time, int avtype);
//=============================================player==================================
#if 0
void AudioFramePushData(CallPlayer *obj, uint8_t *data, int id, int64_t frame_timestamp, int64_t now_time)
{
    pthread_mutex_lock(&obj->lock);

    FrameNode *head,*pnew;
    if(!obj->frameListHead)
    {
        //head不存数据,head->next指向第一个数据,head->num保存总索引（也可以设计将第一个节点存入head）
        //注意链表元的2个组成部分，元和next指针，元控制的是整个链表元的地址，next指针则寄生在元下；
        //printf("PushData: create head: obj->data_list=%x \n", obj->data_list);
        head = (FrameNode *)calloc(1, sizeof(FrameNode));  //创建头节点。
        head->num = 0;
        head->next = NULL;  //头节点指针域置NULL
        head->tail = head;  // 开始时尾指针指向头节点；此时head->tail->next与head->next是同一地址
        obj->frameListHead = (void *)head;
    }
    head = (FrameNode *)obj->frameListHead;
    if(head->num > MAX_DELAY_FRAME_NUM)
    {
        free(data);
        printf("AudioFramePushData: skip head->num=%d \n", head->num);
    }
    else{
        pnew = (FrameNode *)calloc(1, sizeof(FrameNode));  //创建新节点
        pnew->data = (uint8_t *)data;
        pnew->idx = head->idx;
        pnew->id = id;
        pnew->frame_timestamp = frame_timestamp;
        pnew->now_time = now_time;
        pnew->next = NULL;   //新节点指针域置NULL
        head->tail->next = pnew;  //新节点插入到表尾；首个节点时，则意味着ead->next也指向了pnew
        head->tail = pnew;   //为指针指向当前的尾节点;元地址迁移，元的原先的next脱离了元，新地址的next成为元的next指针；
        head->idx++;
        head->num++;
        //printf("AudioFramePushData: head->num=%d \n", head->num);
    }
    pthread_mutex_unlock(&obj->lock);
}
#endif
static void * FramePopData(CallPlayer *obj)
{
    void *ret = NULL;
    pthread_mutex_lock(&obj->lock);
    FrameNode *head, *q;
    head = (FrameNode *)obj->frameListHead;
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
                //printf("AudioFramePopData: head->next is null \n");
                head->next = NULL;  //头节点指针域置NULL
                head->tail = head;
            }
        }
    }
    pthread_mutex_unlock(&obj->lock);
    return ret;
}
//每个通道保存的数据
static void FramePushData2(FrameBufferNode *obj, uint8_t *data, int size, int id, int64_t frame_timestamp, int64_t now_time)
{
    FrameNode *head,*pnew, *q;
    //printf("FramePushData2: size=%d \n", size);
    if(!obj->frameListHead)
    {
        //head不存数据,head->next指向第一个数据,head->num保存总索引（也可以设计将第一个节点存入head）
        //注意链表元的2个组成部分，元和next指针，元控制的是整个链表元的地址，next指针则寄生在元下；
        //printf("PushData: create head: obj->data_list=%x \n", obj->data_list);
        head = (FrameNode *)calloc(1, sizeof(FrameNode));  //创建头节点。
        head->num = 0;
        head->idx = 0;
        head->next = NULL;  //头节点指针域置NULL
        head->tail = head;  // 开始时尾指针指向头节点；此时head->tail->next与head->next是同一地址
        obj->frameListHead = (void *)head;
    }
    head = (FrameNode *)obj->frameListHead;
    //printf("FramePushData2: head->num=%d \n", head->num);
    if(head->num > MAX_DELAY_FRAME_NUM)
    {
        q = head->next;
        head->next = head->next->next;
        head->num--;
        if(head->next == NULL)
        {
            //printf("FramePopData: head->next is null \n");
            head->next = NULL;  //头节点指针域置NULL
            head->tail = head;
        }
        free(q->data);
        free(q);
        //free(data);
        printf("player: FramePushData2: skip head->num=%d \n", head->num);
    }
    {
        pnew = (FrameNode *)calloc(1, sizeof(FrameNode));  //创建新节点
        pnew->data = (uint8_t *)data;
        pnew->idx = head->idx;
        pnew->id = id;
        pnew->size = size;
        pnew->frame_timestamp = frame_timestamp;
        pnew->now_time = now_time;
        pnew->next = NULL;   //新节点指针域置NULL
        head->tail->next = pnew;  //新节点插入到表尾；首个节点时，则意味着ead->next也指向了pnew
        head->tail = pnew;   //为指针指向当前的尾节点;元地址迁移，元的原先的next脱离了元，新地址的next成为元的next指针；
        head->idx++;
        head->num++;
        //printf("FramePushData: head->num=%d \n", head->num);
    }
}
static void * FramePopData2(FrameBufferNode *obj)
{
    void *ret = NULL;
    FrameNode *head, *q;
    head = (FrameNode *)obj->frameListHead;
    //printf("FramePushData2: head->num=%d \n", head->num);
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
    return ret;
}
void AudioFrameBufferPushData(CallPlayer *obj, uint8_t *data, int size, int id, int64_t frame_timestamp, int64_t now_time)
{
    pthread_mutex_lock(&obj->lock);
    FrameBufferNode *head,*pnew, *q;
    if(!obj->frameBufferHead)
    {
        //head不存数据,head->next指向第一个数据,head->num保存总索引（也可以设计将第一个节点存入head）
        //注意链表元的2个组成部分，元和next指针，元控制的是整个链表元的地址，next指针则寄生在元下；
        //printf("PushData: create head: obj->data_list=%x \n", obj->data_list);
        head = (FrameBufferNode *)calloc(1, sizeof(FrameBufferNode));  //创建头节点。
        head->num = 0;
        head->idx = 0;
        head->next = NULL;  //头节点指针域置NULL
        head->tail = head;  // 开始时尾指针指向头节点；此时head->tail->next与head->next是同一地址
        obj->frameBufferHead = (void *)head;
    }
    head = (FrameBufferNode *)obj->frameBufferHead;
    //printf("FrameBufferPushData: head->num=%x \n", head->num);
#if 0
    if(head->num > 128)
    {
        q = head->next;
        //printf("FrameBufferPushData: q=%x \n", q);
        while(q)
        {
            if(q->id == id)
            {
                FrameNode * p = FramePopData2(q);
                if(p && p->data)
                {
                    free(p->data);
                    free(p);
                }
                break;
            }
            q = q->next;
        }
        //free(data);
        printf("FrameBufferPushData: skip head->num=%d \n", head->num);
    }
#endif
    {
        //printf("FrameBufferPushData: head->next=%x \n", head->next);
        q = head->next;
        int renewflag = 0;
        //printf("FrameBufferPushData: q=%x \n", q);
        while(q)
        {
            if(q->id == id)
            {
                FramePushData2(q, data, size, id, frame_timestamp, now_time);
                renewflag = 1;
                break;
            }
            q = q->next;
        }
        if(!renewflag)
        {
            pnew = (FrameBufferNode *)calloc(1, sizeof(FrameBufferNode));  //创建新节点
            //pnew->data = (uint8_t *)data;
            pnew->idx = head->idx;
            pnew->id = id;
            pnew->init_timestamp = 0;//  # 防止漂移
            pnew->init_start_time = 0;//  # 防止漂移
            pnew->base_timestamp = 0;
            pnew->base_start_time = 0;
            pnew->time_offset = 0;
            pnew->last_timestamp = 0;
            pnew->search_count = 0;
            pnew->audio_timestamp = 0;
            pnew->audio_start_time = 0;
            pnew->audio_frequence = 0;
            FramePushData2(pnew, data, size, id, frame_timestamp, now_time);
            pnew->next = NULL;   //新节点指针域置NULL
            head->tail->next = pnew;  //新节点插入到表尾；首个节点时，则意味着ead->next也指向了pnew
            head->tail = pnew;   //为指针指向当前的尾节点;元地址迁移，元的原先的next脱离了元，新地址的next成为元的next指针；
            head->idx++;
            head->num++;
        }
        //printf("FramePushData: head->num=%d \n", head->num);

    }
    pthread_mutex_unlock(&obj->lock);
}
static int FrameBufferPopDataMultWay(CallPlayer *obj, FrameNode *outnodes[])
{
    int i = 0;
    //printf("FrameBufferPopDataMultWay: obj=%x \n", obj);
    pthread_mutex_lock(&obj->lock);
    FrameBufferNode *head, *q;
    head = (FrameBufferNode *)obj->frameBufferHead;
    if(head)
    {
        //printf("FrameBufferPopDataMultWay: head->num=%d \n", head->num);
        q = head->next;
        while(q)
        {
            if(!q)
            {
                break;
            }
            //
            //printf("FrameBufferPopDataMultWay: start FramePopData2 \n");
            FrameNode * p = FramePopData2(q);
            //printf("FrameBufferPopDataMultWay: p=%x \n", p);
            if(p)
            {
                outnodes[i] = p;
                i++;
            }
            //
            q = q->next;
        }
    }
    pthread_mutex_unlock(&obj->lock);
    return i;
}
static void * FrameBufferPopData(CallPlayer *obj)
{
    void *ret = NULL;
    //pthread_mutex_lock(&obj->lock);
    FrameBufferNode *head, *q;
    head = (FrameBufferNode *)obj->frameBufferHead;
    if(head)
    {
        q = head->next;
        if(q == NULL || q == head)
        {

        }
        else{
            head->next = head->next->next;
            if(head->next == NULL)
            {
                head->next = NULL;  //头节点指针域置NULL
                head->tail = head;
            }
            ret = q;
        }
    }
    //pthread_mutex_unlock(&obj->lock);
    return ret;
}
static void * FrameBufferFinder(CallPlayer *obj, int id)
{
    void *ret = NULL;
    //pthread_mutex_lock(&obj->lock);
    FrameBufferNode *head, *q;
    head = (FrameBufferNode *)obj->frameBufferHead;
    if(head)
    {
        q = head->next;
        while(q)
        {
            if(q->id == id)
            {
                ret = q;
                break;
            }
            q = q->next;
        }
    }
    //pthread_mutex_unlock(&obj->lock);
    return ret;
}
int PlayerInit(CallPlayer *obj)
{
    int ret = 0;
    cJSON *json = NULL;//mystr2json(params);

    ret = api_player_init(obj->handle, obj->params);
    if(json)
    {
        deleteJson(json);
        json = NULL;
    }

    return ret;
}
int PlayerStop(CallPlayer *obj)
{
    int ret = 0;
    if(obj)
    {
        api_player_close(obj->handle);
    }
    return ret;
}
static void release_framenode(FrameNode *head)
{
    if(head && head->num)
    {
        do{
            FrameNode *q;
            q = head->next;
            if(q == NULL || q == head)
            {
                break;
            }
            else{
                head->next = head->next->next;
            }
            //
            if(q->data)
            {
                free(q->data);
            }
            free(q);   //释放节点i的内存单元
        }while(1);
        free(head);
    }
}
static void release_node(CallPlayer *obj)
{
    if(obj->rectHead)
    {
        RectNode *head = (RectNode *)obj->rectHead;
        if(head && head->num)
        {
            do{
                RectNode *q;
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
    }
    if(obj->frameListHead)
    {
        FrameNode *head = (FrameNode *)obj->frameListHead;
        release_framenode(head);
    }
    if(obj->frameBufferHead)
    {
        FrameBufferNode *head = (FrameBufferNode *)obj->frameBufferHead;
        if(head && head->num)
        {
            do{
                FrameBufferNode *q;
                q = head->next;
                if(q == NULL || q == head)
                {
                    break;
                }
                else{
                    head->next = head->next->next;
                }
                //
                release_framenode(q->frameListHead);
                free(q);   //释放节点i的内存单元
            }while(1);
            free(head);
        }
    }
}
void player_close(CallPlayer *obj)
{
    if(obj->json)
    {
        api_json_free(obj->json);
        obj->json = NULL;
    }
    release_node(obj);
}
int MixPlayerData(CallPlayer *obj, char *param_str, char *mix_buf[], int mix_num, int data_size)
{
    int ret = 0;
    //printf("MixPlayerData: param_str=%s \n", param_str);
    //printf("MixPlayerData: mix_num=%d \n", mix_num);
    //printf("MixPlayerData: data_size=%d \n", data_size);
    //printf("MixPlayerData: mix_buf[0]=%x \n", mix_buf[0]);
    ret = audio_play_frame_mix(obj->handle, param_str, mix_buf, data_size);
#if 1
    McuObj *mcu = (McuObj *)get_mcu((SocketObj *)(obj->sock));
    //printf("MixPlayerData: mcu=%x \n", mcu);
    if(mcu)
    {
        //uint8_t *data = malloc(obj->frame_size * sizeof(uint8_t));
        uint8_t *data = av_malloc(obj->frame_size * sizeof(uint8_t));
        //test
        memcpy(data, mix_buf[0], obj->frame_size);
        //
        int64_t now_time = get_sys_time();
        McuPushData(mcu, data, obj->frame_size, now_time, kIsAudio);
    }
#endif
    //printf("MixPlayerData: ret=%d \n", ret);
    return ret;
}
int set_mcu2player(SocketObj *sock, McuObj *mcu)
{
    int ret = 0;
    pthread_mutex_lock(&sock->status_lock);
    SessionObj * session = (SessionObj *)sock->session;
    session->mcu = mcu;
    pthread_mutex_unlock(&sock->status_lock);
    return ret;
}
int player_run(CallPlayer *obj)
{
    int ret = 0;
    //printf("player_run:obj=%x \n", obj);
    pthread_mutex_lock(&obj->status_lock);
    obj->recv_status = 1;
    int status = obj->recv_status;
    pthread_mutex_unlock(&obj->status_lock);
    while(status > 0)
    {
        //printf("player_run:status=%d \n", status);
        //play right now
#if 1
        FrameNode *outnodes[16] = {};
        uint8_t *mix_buf[16] = {};
        int mix_num = FrameBufferPopDataMultWay(obj, outnodes);
        //printf("player_run:mix_num=%d \n", mix_num);
        if(mix_num)
        {
            //printf("player_run:mix_num=%d \n", mix_num);
            int data_size = 0;
            for(int i = 0; i < mix_num; i++)
            {
                FrameNode *item = (FrameNode *)outnodes[i];
                if(item)
                {
                    //printf("Player_run: item->id=%d  \n", item->id);
                    //self.play_right_now3(first_frame[0], thisFrmBuf.id, show_flag)
                    int id = item->id;
                    uint8_t *data = item->data;
                    int64_t frame_timestamp = item->frame_timestamp;
                    int64_t now_time = item->now_time;
                    mix_buf[i] = data;
                    data_size = item->size;
                    //free(data);
                    free(item);
                }
            }
            obj->json = api_renew_json_int(obj->json , "mix_num", mix_num);
            char *jsonStr = cJSON_Print(obj->json);
            MixPlayerData(obj, jsonStr, mix_buf, mix_num, data_size);
            //printf("player_run:mix_num=%d \n", mix_num);
            api_json2str_free(jsonStr);
            for(int i = 0; i < mix_num; i++)
            {
                if(mix_buf[i])
                {
                    //printf("player_run:i=%d \n", i);
                    free(mix_buf[i]);
                    //printf("player_run:2: i=%d \n", i);
                }
            }
        }
        else{
            usleep(10000);//10ms
        }
#else
        //usleep(400000);//40ms
        obj->json = api_renew_json_int(obj->json , "mix_num", 1);
        char *jsonStr = cJSON_Print(obj->json);
        char testdata[8192] = {0};
        char *indata[1] = {testdata};
        printf("player_run:jsonStr=%s \n", jsonStr);
        audio_play_frame_mix(obj->handle, jsonStr, indata, 8192);
        //audio_play_frame(obj->handle, jsonStr, indata[0], 8192);
        api_json2str_free(jsonStr);
        usleep(40000);//40ms
#endif
        pthread_mutex_lock(&obj->status_lock);
        status = obj->recv_status;// & obj->send_status;
        pthread_mutex_unlock(&obj->status_lock);
        //printf("player_run: status=%d \n", status);
    }
    PlayerStop(obj);
    printf("player_run: over \n");
    //char *p = malloc(32);
    //strcpy(p,"player_run over");
    //pthread_exit((void*)p);
    return ret;
}

void * audio_player(SocketObj *sock)
{
    int taskId = sock->id;
    sock->status = 1;
    pthread_mutex_init(&sock->status_lock,NULL);
    SessionObj *session = (SessionObj *)calloc(1, sizeof(SessionObj));
    sock->session = (void *)session;
    CallPlayer *obj = (CallPlayer *)calloc(1, sizeof(CallPlayer));
    //sock->session = (void *)obj;
    obj->params = sock->params;
    //obj->json = (cJSON *)api_str2json(obj->params);
    int ret = PlayerInit(obj);
    printf("audio_player: ret=%x \n", ret);
    pthread_mutex_init(&obj->lock,NULL);
    pthread_mutex_init(&obj->status_lock,NULL);
    session->render = obj;
    obj->sock = (void *)sock;
    cJSON * json = (cJSON *)api_str2json(sock->params);
    int out_buffer_size = GetvalueInt(json, "out_buffer_size");
    int frame_size = GetvalueInt(json, "frame_size");
    obj->frame_size = out_buffer_size;

    player_run(obj);

    player_close(obj);
    pthread_mutex_destroy(&obj->lock);
    pthread_mutex_destroy(&obj->status_lock);
    free(obj);
    if(session)
    {
        free(session);
    }
    pthread_mutex_destroy(&sock->status_lock);
    sock->status = -1;
    printf("audio_player: over: taskId=%d \n", taskId);
    return 0;
}
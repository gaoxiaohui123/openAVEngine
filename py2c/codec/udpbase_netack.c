#include "inc.h"
#include "udpbase.h"

extern int GetRtxData(pthread_mutex_t *lock, NetInfoNode *netNode);

//opt:减少netinfo
int PushHasNet(HasNetNode **hasnetList, int chanId, int status, int is_rtx)
{
    int ret = 0;
    HasNetNode *head,*pnew, *q;
    head = *hasnetList;
    if(!(*hasnetList))
    {
        head = (HasNetNode *)calloc(1, sizeof(HasNetNode));
        head->idx = 0;
        head->num = 0;
        head->chanId = -1;
        head->status = -1;
        head->is_rtx = -1;
        head->next = NULL;
        head->tail = head;
        *hasnetList = head;
    }
    if(head->num > 0 && !is_rtx)
    {
        q = head->next;
        do{
            if(!q->next)
            {
                break;
            }
            if(q->chanId == chanId && q->status == status)
            {
                //opt: 减少netinfo数量；
                ret = 1;
            }
            q = q->next;
        }while(1);
    }
    if(!ret)
    {
        pnew = (HasNetNode *)calloc(1, sizeof(HasNetNode));  //创建新节点
        pnew->chanId = chanId;
        pnew->status = status;
        pnew->is_rtx = is_rtx;
        pnew->next = NULL;   //新节点指针域置NULL
        head->tail->next = pnew;  //新节点插入到表尾；首个节点时，则意味着ead->next也指向了pnew
        head->tail = pnew;   //为指针指向当前的尾节点;元地址迁移，元的原先的next脱离了元，新地址的next成为元的next指针；
        head->idx++;
        head->num++;
    }

    return ret;
}
void ReleaseHasNet(HasNetNode **hasnetList)
{
    HasNetNode *head = *hasnetList;
    if(head && head->num)
    {
        do{
            HasNetNode *q;
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
int AddRTT(RTTNode **rttDictList, cJSON *json)
{
    int ret = 0;
    RTTNode *head,*pnew, *q;
    if(!*rttDictList)
    {
        //head不存数据,head->next指向第一个数据,head->num保存总索引（也可以设计将第一个节点存入head）
        //注意链表元的2个组成部分，元和next指针，元控制的是整个链表元的地址，next指针则寄生在元下；
        //MYPRINT("PushData: create head: obj->data_list=%x \n", obj->data_list);
        head = (RTTNode *)malloc(sizeof(RTTNode));  //创建头节点。
        head->num = 0;
        head->idx = 0;
        head->next = NULL;  //头节点指针域置NULL
        head->tail = head;  // 开始时尾指针指向头节点；此时head->tail->next与head->next是同一地址
        *rttDictList = (RTTNode *)head;
    }
    head = (RTTNode *)(*rttDictList);
    if(head->num > 1000)
    {
        MYPRINT("AddRTT: head->num=%d \n", head->num);
    }
    pnew = (RTTNode *)malloc(sizeof(RTTNode));  //创建新节点
    pnew->json = json;
    pnew->idx = head->idx;
    pnew->next = NULL;   //新节点指针域置NULL
    head->tail->next = pnew;  //新节点插入到表尾；首个节点时，则意味着ead->next也指向了pnew
    head->tail = pnew;   //为指针指向当前的尾节点;元地址迁移，元的原先的next脱离了元，新地址的next成为元的next指针；
    head->idx++;
    head->num++;
    return ret;
}

int AddEncoder(pthread_mutex_t *lock, EncoderNode **encoderHead, Encoder *encoder)
{
    int ret = 0;
    MYPRINT("AddEncoder: encoderHead=%x \n", encoderHead);
    MYPRINT("AddEncoder: *encoderHead=%x \n", *encoderHead);
    pthread_mutex_lock(lock);
    EncoderNode *head,*pnew, *q;
    if(!*encoderHead)
    {
        //head不存数据,head->next指向第一个数据,head->num保存总索引（也可以设计将第一个节点存入head）
        //注意链表元的2个组成部分，元和next指针，元控制的是整个链表元的地址，next指针则寄生在元下；
        //MYPRINT("PushData: create head: obj->data_list=%x \n", obj->data_list);
        head = (EncoderNode *)malloc(sizeof(EncoderNode));  //创建头节点。
        head->num = 0;
        head->idx = 0;
        head->next = NULL;  //头节点指针域置NULL
        head->tail = head;  // 开始时尾指针指向头节点；此时head->tail->next与head->next是同一地址
        *encoderHead = (EncoderNode *)head;
    }
    MYPRINT("AddEncoder: *encoderHead=%x \n", *encoderHead);
    head = (EncoderNode *)(*encoderHead);
    pnew = (EncoderNode *)malloc(sizeof(EncoderNode));  //创建新节点
    pnew->encoder = encoder;
    pnew->idx = head->idx;
    pnew->next = NULL;   //新节点指针域置NULL
    head->tail->next = pnew;  //新节点插入到表尾；首个节点时，则意味着ead->next也指向了pnew
    head->tail = pnew;   //为指针指向当前的尾节点;元地址迁移，元的原先的next脱离了元，新地址的next成为元的next指针；
    head->idx++;
    head->num++;
    head->audio_num += encoder->avtype == kIsAudio;
    head->video_num += encoder->avtype == kIsVideo;
    pthread_mutex_unlock(lock);
    MYPRINT("AddEncoder: encoder=%x \n", encoder);
    MYPRINT("AddEncoder: encoder->codec=%x \n", encoder->codec);
    return ret;
}
int AddNetInfo(pthread_mutex_t *lock, NetInfoNode **netInfoHead, NetInfo *netinfo)
{
    int ret = 0;
    pthread_mutex_lock(lock);
    NetInfoNode *head,*pnew, *q;
    if(!*netInfoHead)
    {
        //head不存数据,head->next指向第一个数据,head->num保存总索引（也可以设计将第一个节点存入head）
        //注意链表元的2个组成部分，元和next指针，元控制的是整个链表元的地址，next指针则寄生在元下；
        //MYPRINT("PushData: create head: obj->data_list=%x \n", obj->data_list);
        head = (NetInfoNode *)malloc(sizeof(NetInfoNode));  //创建头节点。
        head->num = 0;
        head->idx = 0;
        head->next = NULL;  //头节点指针域置NULL
        head->tail = head;  // 开始时尾指针指向头节点；此时head->tail->next与head->next是同一地址
        *netInfoHead = (NetInfoNode *)head;
    }
    head = (NetInfoNode *)(*netInfoHead);
    if(head->num > 1000)
    {
        MYPRINT("AddNetInfo: head->num=%d \n", head->num);
    }
    int redundancy = 0;
#if 1
    if(head->num > 0)
    {
        q = head->next;
        while(q)
        {
            if(!netinfo->info_status)
            {
                if(q->netinfo.chanId == netinfo->chanId && q->netinfo.info_status == netinfo->info_status)
                {
                    q->netinfo = *netinfo;
                    redundancy = 1;
                }
            }
            else{
                if( q->netinfo.chanId == netinfo->chanId &&
                    q->netinfo.decodeId == netinfo->decodeId &&
                    q->netinfo.info_status == netinfo->info_status)
                {
                    q->netinfo = *netinfo;
                    redundancy = 1;
                }
            }
            if(redundancy)
            {
                break;
            }
            //if(q->chanId == chanId && q->status == status)
            q = q->next;
        }
    }
#endif
    if(!redundancy)
    {
        pnew = (NetInfoNode *)malloc(sizeof(NetInfoNode));  //创建新节点
        pnew->netinfo = *netinfo;
        pnew->idx = head->idx;
        pnew->next = NULL;   //新节点指针域置NULL
        head->tail->next = pnew;  //新节点插入到表尾；首个节点时，则意味着ead->next也指向了pnew
        head->tail = pnew;   //为指针指向当前的尾节点;元地址迁移，元的原先的next脱离了元，新地址的next成为元的next指针；
        head->idx++;//只增不减
        head->num++;//有增有减
    }

    pthread_mutex_unlock(lock);
    return ret;
}
int PopEncoder(pthread_mutex_t *lock, EncoderNode *encoderHead, Encoder *encoder)
{
    int ret = 0;
    pthread_mutex_lock(lock);
    EncoderNode *head, *p, *q;
    head = (EncoderNode *)encoderHead;

    if(encoder)
    {
        q = head->next;

        while(q)
        {
            if(q->encoder->selfChanId == encoder->selfChanId && q->encoder->avtype == encoder->avtype)
            {
                if(!p)
                {
                    head->num = 0;
                    head->audio_num = 0;
                    head->video_num = 0;
                    head->idx = 0;
                    head->next = NULL;  //头节点指针域置NULL
                    head->tail = head;
                }
                else{
                    p->next = q->next;//last node point to next node over current node
                    head->num -= 1;
                    head->audio_num -= q->encoder->avtype == kIsAudio;
                    head->video_num -= q->encoder->avtype == kIsVideo;
                }
                free(q);//delete node
                break;
            }
            p = q;
            q = q->next;
        }
    }
    pthread_mutex_unlock(lock);
    return ret;
}
EncoderNode * FindEncoder(pthread_mutex_t *lock, EncoderNode *encoderHead, int selfChanId, int avtype)
{
    EncoderNode * ret = NULL;
    pthread_mutex_lock(lock);
    EncoderNode *head, *p, *q;
    head = (EncoderNode *)encoderHead;
    //MYPRINT("FindEncoder: head=%x \n", head);
    q = head->next;
    while(q)
    {
        //MYPRINT("FindEncoder: q->encoder=%x \n", q->encoder);
        if(q->encoder->selfChanId == selfChanId && q->encoder->avtype == avtype)
        {
            ret = q;
            break;
        }
        p = q;
        q = q->next;
    }

    pthread_mutex_unlock(lock);
    return ret;
}
//called by encoder
void * PopNetInfo(pthread_mutex_t *lock, NetInfoNode *netInfoHead, NetInfo *netinfo)
{
    void *ret = NULL;
    pthread_mutex_lock(lock);
    NetInfoNode *head, *p, *q;
    head = (NetInfoNode *)netInfoHead;
    if(netinfo)
    {
    }
    else{
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
                //MYPRINT("FramePopData: head->next is null \n");
                head->next = NULL;  //头节点指针域置NULL
                head->tail = head;
            }
        }
    }
    pthread_mutex_unlock(lock);
    return ret;
}
void * PopNetInfoAll(pthread_mutex_t *lock, NetInfoNode **netInfoHead)
{
    //减少临界阻塞
    //当发包和收包速率相当时,累积包会以2倍的指数增长,即延迟会越来越大;
    //因此,实际的最大收包速率是其上限的二分之一;
    void *ret = NULL;
    pthread_mutex_lock(lock);
    ret = *netInfoHead;
    *netInfoHead = NULL;
    pthread_mutex_unlock(lock);
    return ret;
}

void loss_rate_info_init(LossRateInfo *loss_rate_info)
{
    loss_rate_info[0].loss_rate = -1;
    loss_rate_info[0].max_loss_rate = 0;
    loss_rate_info[0].last_max_loss_rate = 0;
    loss_rate_info[0].sum_loss_rate = 0;
    loss_rate_info[0].cnt_loss_rate = 0;
    loss_rate_info[0].cnt_max = 1;
    loss_rate_info[1].loss_rate = -1;
    loss_rate_info[1].max_loss_rate = 0;
    loss_rate_info[1].last_max_loss_rate = 0;
    loss_rate_info[1].sum_loss_rate = 0;
    loss_rate_info[1].cnt_loss_rate = 0;
    loss_rate_info[1].cnt_max = 10;
    loss_rate_info[2].loss_rate = -1;
    loss_rate_info[2].max_loss_rate = 0;
    loss_rate_info[2].last_max_loss_rate = 0;
    loss_rate_info[2].sum_loss_rate = 0;
    loss_rate_info[2].cnt_loss_rate = 0;
    loss_rate_info[2].cnt_max = 100;
}
// 与rtt不同，loss_rate更具突发性
static int add_loss_rate(LossRateInfo *loss_rate_info, int loss_rate)
{
    int ret = loss_rate;
    if(loss_rate >= 0)
    {
        for(int i = 0; i < 3; i++)
        {
            if(loss_rate_info[i].max_loss_rate < loss_rate)
            {
                loss_rate_info[i].max_loss_rate = loss_rate;
            }
            loss_rate_info[i].sum_loss_rate += loss_rate;
            loss_rate_info[i].cnt_loss_rate += 1;
            if(loss_rate_info[i].cnt_loss_rate >= loss_rate_info[i].cnt_max)
            {
                loss_rate_info[i].loss_rate = loss_rate_info[i].sum_loss_rate / loss_rate_info[i].cnt_max;
                loss_rate_info[i].last_max_loss_rate = loss_rate_info[i].max_loss_rate;
                loss_rate_info[i].max_loss_rate = loss_rate;
                loss_rate_info[i].sum_loss_rate = 0;
                loss_rate_info[i].cnt_loss_rate = 0;
            }
            //else{
            //    loss_rate_info[i].loss_rate = loss_rate_info[i].sum_loss_rate / loss_rate_info[i].cnt_loss_rate;
            //}
        }
        //if(loss_rate_info[1].loss_rate > 0)
        {
            //ret = loss_rate_info[1].max_loss_rate;
            if( loss_rate_info[1].last_max_loss_rate > 0 &&
                loss_rate < loss_rate_info[1].last_max_loss_rate)
            {
                ret = loss_rate_info[1].last_max_loss_rate;
            }
            if(ret < loss_rate_info[2].loss_rate)//瞬间波谷，容许在下个大周期更新；
            {
                ret = loss_rate_info[2].loss_rate;
            }
            if( ret > loss_rate_info[2].last_max_loss_rate &&
                loss_rate_info[2].last_max_loss_rate > 0)//瞬间峰值
            {
                //ret = loss_rate_info[2].last_max_loss_rate;
            }
            //持续偏大，必须快速更新
            //if(ret < loss_rate_info[1].loss_rate)
            //{
            //    ret = loss_rate_info[1].loss_rate;
            //}
        }
    }
    return ret;
}
static int add_delay(DelayInfo *delay_info, int delay)
{
    int ret = delay;
    if(delay >= 0)
    {
        for(int i = 0; i < 3; i++)
        {
            if(delay_info[i].max_delay < delay)
            {
                delay_info[i].max_delay = delay;
            }
            delay_info[i].sum_delay += delay;
            delay_info[i].cnt_delay += 1;
            if(delay_info[i].cnt_delay >= delay_info[i].cnt_max)
            {
                delay_info[i].delay = delay_info[i].sum_delay / delay_info[i].cnt_max;
                delay_info[i].last_max_delay = delay_info[i].max_delay;
                delay_info[i].max_delay = delay;
                delay_info[i].sum_delay = 0;
                delay_info[i].cnt_delay = 0;
            }
            //else{
            //    delay_info[i].delay = delay_info[i].sum_delay / delay_info[i].cnt_delay;
            //}
        }
        if(delay_info[1].delay > 0)
        {
            ret = delay_info[1].max_delay;
            if(delay_info[1].last_max_delay > 0)
            {
                ret = delay_info[1].last_max_delay;
            }
            if(ret < delay_info[2].delay)//瞬间波谷，容许在下个大周期更新；
            {
                ret = delay_info[2].delay;
            }
            //if(ret > delay_info[2].last_max_delay)//瞬间峰值
            //{
            //    ret = delay_info[2].last_max_delay;
            //}
            //持续偏大，必须快速更新
            //if(ret < delay_info[1].delay)
            //{
            //    ret = delay_info[1].delay;
            //}
        }
    }
    return ret;
}
int net_renew_delay_time(DelayInfo *delay_info, int selfChanId, uint8_t *buf, int size, int delay_time)
{
    int ret = 0;
    //if (id < MAX_OBJ_NUM)
    {
        NetInfo netInfo = {};
        int enable_nack = GetNetInfo((uint8_t*)buf, size, &netInfo, 0);//先更新延时信息，再更新其他net信息
        //MYPRINT("renew_delay_time: enable_nack= %d \n", enable_nack);
        if(enable_nack)
        {

#ifndef SELF_TEST_MODE
            if(netInfo.decodeId == selfChanId)
#endif
            {
                if(netInfo.info_status)
                {
                    int diff_time = (netInfo.rtt << 3);
                    //MYPRINT("video: api_renew_delay_time: diff_time= %d \n", diff_time);
                    ret = add_delay(delay_info, diff_time);
                }

            }
        }
    }
    return ret;
}
int is_skip(char *complete, int ref_idc, int loss_rate, int idr_status)
{
    //int idr_status = 0;
    if(loss_rate > 60)
    {
        if(strncmp(complete, "complete", strlen("complete")))
        {
            if(ref_idc == 1)//  # P
            {
                idr_status = ref_idc + 1;
            }
            else if(ref_idc == 2)//  # PB
            {
                idr_status = ref_idc + 1;
            }
            else if(ref_idc == 3)//  # B
            {
                idr_status = ref_idc + 1;
            }
        }
        else
        {
            if(ref_idc == 0)//  # I
            {
                idr_status = 0;
            }
            else if((ref_idc + 1) < idr_status)
            {
                idr_status = 0;
            }
            else if (((ref_idc + 1) == idr_status) &&
                ((ref_idc == 2) || (ref_idc == 3)))
            {
                idr_status = 0;
            }
        }
    }
    else{
        if(strncmp(complete, "complete", strlen("complete")))
        {
            if(ref_idc == 2)//  # PB
            {
                idr_status = ref_idc + 1;
            }
            else if(ref_idc == 3)//  # B
            {
                idr_status = ref_idc + 1;
            }
        }
        else
        {
            if(ref_idc == 0)//  # I
            {
                idr_status = 0;
            }
            else if((ref_idc + 1) < idr_status)
            {
                idr_status = 0;
            }
            else if (((ref_idc + 1) == idr_status) &&
                ((ref_idc == 2) || (ref_idc == 3)))
            {
                idr_status = 0;
            }
        }
    }
    return idr_status;
}
/*
int CheckNetInfo(NetInfo *netInfo, int loss_rate)
{
    int ret = 0;
    if(!netInfo->info_status)
    {
        //待反馈信息，非测试情况下，存在netInfo.chanId == selfCha
        if(loss_rate > 0)
        {
            netInfo->loss_rate = loss_rate;// + 1;
        }
        ret = 1;
    }
    else{
        //从中调出本端信息，其他的丢弃，此处获得的是终极信息
        pthread_mutex_t *lock;
        EncoderNode *encoderHead;
        int selfChanId = netInfo.chanId - 1;
        ret = FindEncoder(lock, encoderHead, selfChanId);//编码端id，也是对端信息持有者id
    }
    return ret;
}
*/
//decoder
int push_rtx_info(NetInfoObj *netInfoObj, NACK_LOSS *nackLoss)
{
    int ret = 0;
    pthread_mutex_t *lock = &netInfoObj->lock;
    NetInfoNode **netInfoHead = &netInfoObj->otherNetInfoHead;
    NetInfo netInfo = {};
    netInfo.is_rtx = 1;
    netInfo.nack.nack1 = *nackLoss;
    AddNetInfo(lock, netInfoHead, &netInfo);
    return ret;
}
int push_net_info(NetInfoObj *netInfoObj, int avtype, int chanId, uint8_t *data, int insize, int loss_rate)
{
    int ret = 0;
    //std::list<NetInfo> netInfoList;
    MYPRINT("push_net_info: netInfoObj->encoderHead=%x \n", netInfoObj->encoderHead);
    if(!netInfoObj->encoderHead)
    {
        return 0;
    }
    else if(!netInfoObj->encoderHead->num)
    {
        return 0;
    }
    else{
        if( (avtype == kIsVideo && !netInfoObj->encoderHead->video_num) ||
            (avtype == kIsAudio && !netInfoObj->encoderHead->audio_num))
        {
            return 0;
        }
        //MYPRINT("push_net_info: netInfoObj->encoderHead->num= %d \n", netInfoObj->encoderHead->num);
    }
    NetInfo netInfo = {};
    //先更新延时信息，再更新其他net信息
    int enable_nack = GetNetInfo((uint8_t*)data, insize, &netInfo, 1);//create netinfo
    MYPRINT("push_net_info: enable_nack= %d \n", enable_nack);
    if(enable_nack)
    {
        pthread_mutex_t *lock = NULL;
        NetInfoNode **netInfoHead = NULL;
        if(netInfo.is_rtx)
        {
            int selfChanId = netInfo.nack.nack1.chanId - 1;
            //MYPRINT("push_net_info: selfChanId=%d \n", selfChanId);
            if(netInfoObj->encoderHead)
            {
                EncoderNode *encoder = FindEncoder(&netInfoObj->lock, netInfoObj->encoderHead, selfChanId, avtype);//编码端id，也是对端信息持有者id
                if(encoder)
                {   //MYPRINT("push_net_info: encoder= %x \n", encoder);
                    CallCodecVideo *codec = (CallCodecVideo *)encoder->encoder->codec;
                    netInfoHead = &codec->selfNetInfoHead;
                    lock = &codec->lock;
                }
            }
        }
        else{
            //MYPRINT("push_net_info: netInfo.info_status= %d \n", netInfo.info_status);
            //bool net_status = CheckNetInfo(&netInfo, loss_rate);
            int selfChanId = netInfo.chanId - 1;
            if(netInfo.info_status == 1)
            {
                //MYPRINT("push_net_info: netInfo.info_status=%d, selfChanId=%d \n", netInfo.info_status, selfChanId);
                //selfChanId != chanId when not selfmode, because self encoder cannot be decoded by self
                //MYPRINT("push_net_info: netInfo.chanId= %d \n", netInfo.chanId);
                netInfo.decodeId = netInfo.chanId;
                //selfNetInfoHead//将最终结果返回给对应的编码器
                //MYPRINT("push_net_info: netInfo.chanId=%d, selfChanId=%d \n", netInfo.chanId, selfChanId);
                //MYPRINT("push_net_info: netInfoObj=%x \n", netInfoObj);
                //MYPRINT("push_net_info: netInfoObj->encoderHead=%x \n", netInfoObj->encoderHead);
                if(netInfoObj->encoderHead)
                {
                    EncoderNode *encoderNode = FindEncoder(&netInfoObj->lock, netInfoObj->encoderHead, selfChanId, avtype);//编码端id，也是对端信息持有者id
                    //MYPRINT("push_net_info: encoderNode=%x \n", encoderNode);
                    if(encoderNode)
                    {   //MYPRINT("push_net_info: encoder= %x \n", encoder);
                        CallCodecVideo *codec = (CallCodecVideo *)encoderNode->encoder->codec;
                        netInfoHead = &codec->selfNetInfoHead;
                        lock = &codec->lock;
                        //MYPRINT("push_net_info: 2: netInfo.info_status=%d, selfChanId=%d \n", netInfo.info_status, selfChanId);
                        //if(codec->selfNetInfoHead)
                        //{
                        //    MYPRINT("push_net_info: codec=%x \n", codec);
                        //    MYPRINT("push_net_info: codec->selfNetInfoHead=%x \n", codec->selfNetInfoHead);
                        //    MYPRINT("push_net_info: codec->selfNetInfoHead->num=%d \n", codec->selfNetInfoHead->num);
                        //}
                    }
                }
            }
            else{
                //待反馈信息，非测试情况下，存在netInfo.chanId == selfChanId
                if(loss_rate > 0)
                {
                    netInfo.loss_rate = loss_rate;// + 1;
                }
                //otherNetInfoHead//借由编码器进行发送;公共
                netInfoHead = &netInfoObj->otherNetInfoHead;
                lock = &netInfoObj->lock;
            }
        }
        //MYPRINT("push_net_info: netInfoHead=%x, lock=%x \n", netInfoHead, lock);
        if(netInfoHead && lock)
        {
            AddNetInfo(lock, netInfoHead, &netInfo);
        }
    }
    MYPRINT("push_net_info: ret= %d \n", ret);
    return ret;
}

int GetLossRate(CallCodecVideo *obj, int selfChanId, int *p_iloss_rate, float *p_floss_rate,
        float *p_fcode_rate, RTTNode **rttDictList, NetInfoNode **rtxDictHead)
{
    int ret = 0;
    //MYPRINT("GetLossRate: obj=%x \n", obj);
    //MYPRINT("GetLossRate: obj->selfNetInfoHead=%x \n", obj->selfNetInfoHead);
    //if(obj->selfNetInfoHead)
    //{
    //    MYPRINT("GetLossRate: obj->selfNetInfoHead->num=%d \n", obj->selfNetInfoHead->num);
    //}
    NetInfoNode *selfNetInfoHead = PopNetInfoAll(&obj->lock, &obj->selfNetInfoHead);
    //MYPRINT("GetLossRate: obj->selfNetInfoHead=%x \n", obj->selfNetInfoHead);
    if(selfNetInfoHead && selfNetInfoHead->num > 0)
    {
        //MYPRINT("GetLossRate: selfNetInfoHead->num=%d \n", selfNetInfoHead->num);
        int maxLossRate = 0;
        int last_rtt = -1;
        int count = 0;
        Encoder *encoder = obj->encoder;
        NetInfoNode *head = selfNetInfoHead;
        do{
            NetInfoNode *q;
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
            NetInfo *it = &q->netinfo;
            if(it->is_rtx)
            {
                MYPRINT("GetLossRate: it->is_rtx=%d \n", it->is_rtx);
                ///push_rtx_info(rtxDictHead, &it->nack.nack1);
                NetInfo netInfo = {};
                netInfo.is_rtx = 1;
                netInfo.nack.nack1 = it->nack.nack1;
                AddNetInfo(&obj->lock, rtxDictHead, &netInfo);
                MYPRINT("GetLossRate: rtxDictHead=%x \n", rtxDictHead);
            }
            else{
                //std::list<void *> rttDictList;
                int chanId = it->chanId;
                int st0 = it->st0;
                int rt0 = it->rt0;
                int st1 = it->st1;
                int rt1 = it->rt1;
                int rtt = (((rt1 - st0) - (st1 - rt0)) >> 1);
                //MYPRINT("GetLossRate: st0=%d \n", st0);
                //MYPRINT("GetLossRate: rt0=%d \n", rt0);
                //MYPRINT("GetLossRate: st1=%d \n", st1);
                //MYPRINT("GetLossRate: rt1=%d \n", rt1);
                //MYPRINT("GetLossRate: rtt=%d \n", rtt);
                int decodeId = it->decodeId;
                if(rtt > 1000)
                {
                    int dt0 = (st1 - rt0);
                    int dt1 = (rt1 - st0);
                    MYPRINT("GetLossRate: dt0=%d, dt1=%d, rtt=%d, chanId=%d, decodeId=%d \n", dt0, dt1, rtt, chanId, decodeId);
                }
                //MYPRINT("GetLossRate: decodeId=%d \n", decodeId);
                void *json = NULL;
                json = api_renew_json_int(json, "decodeId", decodeId);
                json = api_renew_json_int(json, "rtt", rtt);
                int loss_rate = it->loss_rate;
                //MYPRINT("GetLossRate: loss_rate=%d \n", loss_rate);
                //int selfChanId = (encoder->selfChanId + 1);
                //MYPRINT("gxh: GetLossRate: rtt=%d \n", rtt);
                //MYPRINT("gxh: GetLossRate: decodeId=%d \n", decodeId);
                //MYPRINT("GetLossRate: selfChanId=%d \n", selfChanId);
                //MYPRINT("GetLossRate: chanId=%d \n", chanId);
#ifndef SELF_TEST_MODE
                if(chanId == selfChanId)
#endif
                {
                    //rttDictList.push_back(json);//将获得的结果传送给解码端
                    AddRTT(rttDictList, json);//将获得的结果传送给解码端
                    //MYPRINT("GetLossRate: (*rttDictList)->num=%d \n", (*rttDictList)->num);
                    if(loss_rate > maxLossRate)
                    {
                        maxLossRate = loss_rate;
                        //MYPRINT("gxh: GetLossRate: maxLossRate=%d \n", maxLossRate);
                    }
                    if(last_rtt >= 0)
                    {
                        if(rtt > last_rtt)
                        {
                            count++;
                        }
                        else {
                            count = 0;
                        }
                    }
                    last_rtt = rtt;
                }
            }
            //free(q->data);
            free(q);   //释放节点i的内存单元
        }while(1);
        free(head);

        if(maxLossRate > 0)
        {
            //maxLossRate -= 1;
            encoder->selfMaxLossRate = maxLossRate;
            //MYPRINT("gxh: GetLossRate: maxLossRate=%d \n", maxLossRate);
        }
        //encoder->selfMaxLossRate = 86;//50;//20;//test
        if(encoder->selfMaxLossRate > 0)//
        {
            encoder->selfLossRate = encoder->selfMaxLossRate;

            //encoder->selfLossRate = 10;//test
            if(encoder->selfLossRate > 1 && encoder->selfLossRate < 10)
            {
                //encoder->selfLossRate = 10;//test
            }
            p_iloss_rate[0] = encoder->selfLossRate;
            //floss_rate = float(encoder->selfLossRate) / 100.0;
            float floss_rate = (float)(encoder->selfLossRate - 1) / 100.0;
            //MYPRINT("gxh: GetLossRate: floss_rate0=%2.2f \n", floss_rate0);
            //reset loss_rate/code_rate
            //
            float alfa = 0.8;//  # 0.5
            if(floss_rate > 0.7)
                alfa = 0.5;
            else if(floss_rate > 0.6)
                alfa = 0.6;
            else if(floss_rate > 0.5)
                alfa = 0.7;
            else if(floss_rate > 0.2)
                alfa = 0.8;
            else if(floss_rate > 0.1)
                alfa = 0.9;
            else
                alfa = 0.95;
            float fec_loss_rate = floss_rate;
            float fcode_rate = (1.0 - floss_rate);
            fcode_rate = fcode_rate * alfa;
            if(fcode_rate < 0.15)
                fcode_rate = 0.15;
            p_floss_rate[0] = floss_rate;
            p_fcode_rate[0] = fcode_rate;
        }
        //self.encode0.param.update({"loop_loss_rate": int(encoder->selfLossRate)})
    }
    return ret;
}
int AdaptBitRate(Encoder *encoder, int layerId, int loss_rate, int *p_bit_rate, int threshold)
{
    int ret = 0;
    {
        if(loss_rate > threshold)//10)
        //if(ref_idx == 0)
        {
            float floss_rate = (float)(loss_rate - 1) / 100.0;
            float alfa = 0.8;// 0.5
            if(floss_rate > 0.7)
            {
                alfa = 0.5;
            }
            else if(floss_rate > 0.6)
            {
                alfa = 0.6;
            }
            else if(floss_rate > 0.5)
            {
                alfa = 0.7;
            }
            else if(floss_rate > 0.2)
            {
                alfa = 0.8;
            }
            else if(floss_rate > 0.1)
            {
                alfa = 0.9;
            }
            else {
                alfa = 0.95;
            }
            float code_rate = (1.0 - floss_rate);
            code_rate = code_rate * alfa;
            code_rate = (float)((int)(code_rate * 100)) / 100.0;
            if(encoder->bitRate > (encoder->initBitRate[layerId] >> 1))
            {
                //只降码率
                encoder->bitRate = (int)(encoder->initBitRate[layerId] * code_rate);
            }
            else {
                encoder->bitRate = (int)(encoder->initBitRate[layerId] * code_rate);
                //encoder->bitRate = (int)(encoder->initBitRate[layerId] >> 1);
                //ret = 1;
            }
            //20,40,60,80
            //if(floss_rate > 0.30)
            //{
            //    encoder->bitRate = encoder->initBitRate[layerId] >> 1;
            //}
            //else
            //{
            //    encoder->bitRate = encoder->initBitRate[layerId];
            //}
            p_bit_rate[0] = encoder->bitRate;
            //self.encode0.param.update({"bit_rate": int(self.encode0.bit_rate)})
            //MYPRINT("gxh: GetBitRate: floss_rate=%2.2f \n", floss_rate);
            //MYPRINT("gxh: GetBitRate: ret=%d \n", ret);
        }
        else{
            if(loss_rate < 5)
            {
                //码率回调(调大码率)
                int diff = encoder->initBitRate[layerId] - encoder->bitRate;
                if(diff > 10 * 1024)//(encoder->bitRate < encoder->initBitRate[layerId])
                {
                    MYPRINT("gxh: GetBitRate: encoder->bitRate=%d \n", encoder->bitRate);
                    MYPRINT("gxh: GetBitRate: encoder->initBitRate[layerId]=%d \n", encoder->initBitRate[layerId]);
                    encoder->bitRate = encoder->bitRate + ((encoder->initBitRate[layerId] - encoder->bitRate) >> 1);
                    p_bit_rate[0] = encoder->bitRate;
                }
            }
        }
    }
    return ret;
}
int is_skip_frame(int refs, int ref_idx, int loss_rate, int threshold)
{
    int ret = 0;
    //[20, 40, 60, 70, 80]
    //return ret
    if(loss_rate > threshold)
    {
        if(refs < 2)
        {
        }
        else if(refs == 2)
        {
            if (ref_idx == 0)
            {
            }
            else if (ref_idx == refs)
            {
            }
            else if ((ref_idx & 1) == 1)
            {
                ret = 1;
            }
        }
        else
        {
            if(ref_idx == 0)// #I
            {
            }
            else if(ref_idx == refs)// #P
            {
            }
            else if(((ref_idx & 1) == 0) || (ref_idx == (refs - 1)) || (ref_idx == 1))//#B
            {
                //[1, 2,4,6,8,10,12,14,15]
                if(loss_rate > 40)
                {
                    ret = 1;//#2
                }
                else if(loss_rate > 30)
                {
                    if( ref_idx == 1  ||
                        ref_idx == 4  ||
                        ref_idx == 8  ||
                        ref_idx == 12 ||
                        ref_idx == 15)
                    {
                        ret = 1;
                    }
                }
                else if(loss_rate > 20)
                {
                    if( ref_idx == 2  ||
                        ref_idx == 6  ||
                        ref_idx == 10 ||
                        ref_idx == 14)
                    {
                        ret = 1;
                    }
                }
            }
            else if((ref_idx & 1) == 1)//#PB
            {
                //[3,5,7,9,11,13]
                if(loss_rate > 70)//[3,5,7,11,13]
                {
                    if( ref_idx == 3  ||
                        ref_idx == 5  ||
                        ref_idx == 7  ||
                        ref_idx == 11 ||
                        ref_idx == 13)
                    {
                        ret = 1;
                    }
                }
                else if(loss_rate > 60)//[3,5,9,11]
                {
                    if( ref_idx == 3  ||
                        ref_idx == 5  ||
                        ref_idx == 9  ||
                        ref_idx == 11)
                    {
                        ret = 1;
                    }
                }
                else if(loss_rate > 50)//[3,7,11]
                {
                    if( ref_idx == 3  ||
                        ref_idx == 7  ||
                        ref_idx == 11)
                    {
                        ret = 1;
                    }
                }
            }
        }
    }
    return ret;
}
int ResentByRtx(CallCodecVideo *obj, NetInfoNode *rtxDictHead)
{
    int ret = 0;
    do{
        NetInfoNode *thisNode = (NetInfoNode *)PopNetInfo(&obj->lock, rtxDictHead, NULL);
        if(!thisNode)
        {
            break;
        }
        GetRtxData(obj, thisNode);
        free(thisNode);
        //
    }while(1);

    return ret;
}
int32_t GetNetResult(CallCodecVideo *obj, int ref_idx)
{
    int ret = 0;

    if(obj->netInfoObj)
    {
        int is_skip = 0;
        int loop_loss_rate = 0;
        float floss_rate = 0;
        float code_rate = 0;
        int64_t now_time = api_get_time_stamp_ll();
        if(!obj->start_time)
        {
            obj->start_time = now_time;
        }
        int64_t difftime = (now_time - obj->start_time);
        int64_t long_timestamp = (difftime * 90000 / 1000) % LEFT_SHIFT32;
        uint32_t timestamp = (uint32_t)(long_timestamp);
        //std::list<NetInfo> otherNetInfoList;
        //std::list<NetInfo> selfNetInfoList;
        RTTNode *rttDictList = NULL;
        RTXNode *rtxDictList = NULL;
        NetInfoNode *rtxDictHead = NULL;
        int selfChanId = obj->sessionInfo->chanId;
        //gCodecInfo.pop_net_info(otherNetInfoList, -1);
        //MYPRINT("GetNetResult: selfChanId= %d \n", selfChanId);
        obj->json = api_renew_json_int(obj->json, "timestamp", timestamp);
        GetLossRate(obj, (selfChanId + 1), &loop_loss_rate, &floss_rate, &code_rate, &rttDictList, &rtxDictHead);
        if(rtxDictHead && OPEN_RTX)
        {
            MYPRINT("GetNetResult: rtxDictHead->num= %d \n", rtxDictHead->num);
            ResentByRtx(obj, rtxDictHead);
        }
        int bit_rate = 0;
        if(loop_loss_rate > 0)
        {
            //MYPRINT("GetNetResult: loop_loss_rate= %d \n", loop_loss_rate);
            api_add_loss_rate(obj->handle, (loop_loss_rate - 1));
            //MYPRINT("GetNetResult: floss_rate= %1.2f \n", floss_rate);
            //MYPRINT("GetNetResult: code_rate= %1.2f \n", code_rate);
#if 0
            is_skip = AdaptBitRate(obj->encoder, 0, (loop_loss_rate - 1), &bit_rate, 10);
#endif
        }


#if 1
        //int last_loss_rate = api_get_loss_rate(obj->handle, 2);
        int last_loss_rate = api_get_loss_rate(obj->handle, 3);
        is_skip = AdaptBitRate(obj->encoder, 0, last_loss_rate, &bit_rate, 10);
        //is_skip = AdaptBitRate(obj->encoder, 0, loop_loss_rate, &bit_rate, 40);
        if(is_skip)
        {
            //is_skip no used
            is_skip = is_skip_frame(obj->codecInfo.refs, ref_idx, (int)((floss_rate) * 100), 20);
        }
#endif
        obj->json = api_delete_item(obj->json, "rttInfo");

        if(rttDictList && rttDictList->num > 0)
        {
            //MYPRINT("GetNetResult: rttDictList->num= %d \n", rttDictList->num);
            HasNetNode *hasnetList = NULL;
            //PushHasNet(HasNetNode **hasnetList, int chanId, int status, int is_rtx)
            void *jsonArray = NULL;
            RTTNode *head = rttDictList;
            do{
                RTTNode *q;
                q = head->next;
                if(q == NULL || q == head)
                {
                    break;
                }
                else{
                    //cJSON *thisjson = &q->json;
                    cJSON *thisjson = q->json;
                    api_add_array2json(&obj->json, &jsonArray, thisjson, "rttInfo");
                    api_json_free(thisjson);
                    head->next = head->next->next;
                    if(head->next == NULL)
                    {
                        head->next = NULL;  //头节点指针域置NULL
                        head->tail = head;
                    }
                }
                //free(q->json);
                //api_json_free(q->json);//???
                free(q);   //释放节点i的内存单元
            }while(1);
            free(head);
        }
        obj->json = api_delete_item(obj->json, "netInfo");
        //if(obj->netInfoObj->otherNetInfoHead)
        //{
        //    MYPRINT("GetLossRate: obj->netInfoObj->otherNetInfoHead->num=%d \n", obj->netInfoObj->otherNetInfoHead->num);
        //}
        NetInfoNode * otherNetInfoList = PopNetInfoAll(&obj->netInfoObj->lock, &obj->netInfoObj->otherNetInfoHead);
        //MYPRINT("GetNetResult: obj->netInfoObj->otherNetInfoHead=%x \n", obj->netInfoObj->otherNetInfoHead);
        if(otherNetInfoList && otherNetInfoList->num > 0)
        {
            //MYPRINT("GetNetResult: otherNetInfoList->num= %d \n", otherNetInfoList->num);
            HasNetNode *hasnetList = NULL;
            //PushHasNet(HasNetNode **hasnetList, int chanId, int status, int is_rtx)
            void *jsonArray = NULL;
            NetInfoNode *head = otherNetInfoList;
            do{
                NetInfoNode *q;
                q = head->next;
                //MYPRINT("GetNetResult: q= %x \n", q);
                //MYPRINT("GetNetResult: head= %x \n", head);
                if(q == NULL || q == head)
                {
                    break;
                }
                else{
                    NetInfo *it = &q->netinfo;
                    if(it->is_rtx)
                    {
                        MYPRINT("GetNetResult: it->is_rtx= %d \n", it->is_rtx);
                        NACK_LOSS *nackLoss = &it->nack;
                        void *json = NULL;
                        json = api_renew_json_int(json, "send_time", nackLoss->send_time);
                        json = api_renew_json_int(json, "start_seqnum", nackLoss->start_seqnum);
                        json = api_renew_json_int(json, "chanId", nackLoss->chanId);
                        json = api_renew_json_int(json, "loss_type", nackLoss->loss_type);
                        json = api_renew_json_int(json, "loss0", nackLoss->loss0);
                        json = api_renew_json_int(json, "loss1", nackLoss->loss1);
                        json = api_renew_json_int(json, "loss2", nackLoss->loss2);
                        json = api_renew_json_int(json, "loss3", nackLoss->loss3);
                        json = api_renew_json_int(json, "loss4", nackLoss->loss4);
                        AddRTT(&rtxDictList, json);
                    }
                    else{
                        //MYPRINT("GetNetResult: it->chanId= %d \n", it->chanId);
                        api_add_netinfos2json(&obj->json, &jsonArray, it);
                    }
                    //MYPRINT("GetNetResult: jsonArray= %x \n", jsonArray);
                    head->next = head->next->next;
                    if(head->next == NULL)
                    {
                        head->next = NULL;  //头节点指针域置NULL
                        head->tail = head;
                    }
                }
                //free(q->data);
                free(q);   //释放节点i的内存单元
            }while(1);
            //MYPRINT("GetNetResult: free head 0 \n");
            free(head);
            //MYPRINT("GetNetResult: free head 1 \n");
        }
        obj->json = api_delete_item(obj->json, "rtxInfo");
        if(rtxDictList && rtxDictList->num > 0)
        {
            MYPRINT("GetNetResult: rtxDictList->num= %d \n", rtxDictList->num);
            void *jsonArray = NULL;
            RTXNode *head = rtxDictList;
            do{
                RTXNode *q;
                q = head->next;
                if(q == NULL || q == head)
                {
                    break;
                }
                else{
                    //cJSON *thisjson = &q->json;
                    MYPRINT("GetNetResult: q= %x \n", q);
                    cJSON *thisjson = q->json;
                    //char* teststr = api_json2str(thisjson);
                    //MYPRINT("GetNetResult: teststr= %s \n", teststr);
                    api_add_array2json(&obj->json, &jsonArray, thisjson, "rtxInfo");
                    api_json_free(thisjson);
                    head->next = head->next->next;
                    if(head->next == NULL)
                    {
                        head->next = NULL;  //头节点指针域置NULL
                        head->tail = head;
                    }
                }
                //free(q->data);
                //api_json_free(q->json);//???
                free(q);   //释放节点i的内存单元
            }while(1);
            free(head);
        }

        if(!obj->ssrc)
        {
            #define MAX_UINT    (((long long)1 << 32) - 1)
            unsigned int range = MAX_UINT - 1;
            obj->ssrc = (unsigned int)api_create_id(range) + 1;
        }
        //MYPRINT("GetNetResult: obj->ssrc=%u \n", obj->ssrc);
        //MYPRINT("GetNetResult: loop_loss_rate=%d \n", loop_loss_rate);
        obj->json = api_renew_json_int(obj->json, "ssrc", obj->ssrc);
        obj->json = api_renew_json_int(obj->json, "selfChanId", (selfChanId + 1));
        obj->json = api_renew_json_int(obj->json, "loop_loss_rate", loop_loss_rate);
        obj->json = api_renew_json_int(obj->json, "enable_fec", obj->codecInfo.enable_fec);
        if(loop_loss_rate > 0)
        {
            obj->loop_loss_rate = loop_loss_rate;
            if((floss_rate) * code_rate >= 0)
            {
                obj->codecInfo.loss_rate = floss_rate;
                obj->codecInfo.code_rate = code_rate;
                //MYPRINT("GetNetResult: floss_rate= %2.2f \n", floss_rate);
                //MYPRINT("GetNetResult: code_rate= %2.2f \n", code_rate);
            }
        }
#if 1
        if(loop_loss_rate > 0)
        {
            //MYPRINT("GetNetResult: loop_loss_rate= %d \n", loop_loss_rate);
        }
        //
        if(last_loss_rate > 0)
        {
            obj->json = api_renew_json_int(obj->json, "loop_loss_rate", last_loss_rate);
            floss_rate = ((float)last_loss_rate - 1) / 100.0;
            float alfa = 0.8;//  # 0.5
            if(floss_rate > 0.7)
                alfa = 0.5;
            else if(floss_rate > 0.6)
                alfa = 0.6;
            else if(floss_rate > 0.5)
                alfa = 0.7;
            else if(floss_rate > 0.2)
                alfa = 0.8;
            else if(floss_rate > 0.1)
                alfa = 0.9;
            else
                alfa = 0.95;
            code_rate = (1.0 - floss_rate);
            code_rate = code_rate * alfa;
            if(code_rate < 0.15)
                code_rate = 0.15;
            obj->codecInfo.loss_rate = floss_rate;
            obj->codecInfo.code_rate = code_rate;
            //MYPRINT("GetNetResult: floss_rate= %1.2f \n", floss_rate);
        }
#endif
        //config->code_rate = 0.5;//test
        if(bit_rate > 0)
        {
            //MYPRINT("gxh:HCSVCEncoderImpl::GetNetResult: bit_rate= %d \n", bit_rate);
            obj->json = api_renew_json_int(obj->json, "bit_rate", bit_rate);
        }
    }
    //MYPRINT("GetNetResult: ok \n");
    return is_skip;
}


void CreateNetInfoHnd(NetInfoObj **hnd)
{
    NetInfoObj *obj = (NetInfoObj *)calloc(1, sizeof(NetInfoObj));
    pthread_mutex_init(&obj->lock,NULL);
    *hnd = obj;
    return;
}
void CloseNetInfoHnd(NetInfoObj *obj)
{
    if(obj)
    {
        if(obj->otherNetInfoHead)
        {
            NetInfoNode *head = (NetInfoNode *)obj->otherNetInfoHead;
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
            obj->otherNetInfoHead = NULL;
        }
        if(obj->encoderHead)
        {
            EncoderNode *head = (EncoderNode *)obj->encoderHead;
            if(head && head->num)
            {
                do{
                    EncoderNode *q;
                    q = head->next;
                    if(q == NULL || q == head)
                    {
                        break;
                    }
                    else{
                        head->next = head->next->next;
                    }
                    //
                    //free(q);   //在encoder中释放
                }while(1);
                free(head);
            }
            obj->encoderHead = NULL;
        }
        pthread_mutex_destroy(&obj->lock);
        free(obj);
    }
}

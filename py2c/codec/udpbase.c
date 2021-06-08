
#include "udpbase.h"
#include "cJSON.h"

extern int GetvalueInt(cJSON *json, char *key);
extern char* GetvalueStr(cJSON *json, char *key, char *result);
int client_broadcast_get(SocketObj *obj, UserData *udata0);


static int glob_session_num = 100;
//static int glob_session_idx = 0;
static int64_t glob_paced_bitrate = PACED_BITRATE;
//static SessionInfo *GlobSessionList = NULL;//(SessionInfo *)calloc(1, glob_session_num * sizeof(SessionInfo));

//使用typedef目的一般有两个，一个是给变量一个易记且意义明确的新名字，
//另一个是简化一些比较复杂的类型声明。
struct sockInfo *create();   //创建链表
void insert(SockNode *head,SockNode *pnew,int i);   //插入链表
void pdelete(SockNode *head,int i);   //删除列表
void display(SockNode *head);   //输出链表
void Pfree(SockNode *head);    //销毁链表

struct sockInfo *sockNodeCreate()
{
    {
        //MYPRINT("PushData: create head: obj->data_list=%x \n", obj->data_list);
        SockNode *head = (SockNode *)malloc(sizeof(SockNode));  //创建头节点。
        head->num = 0;
        head->id = 0;
        head->next = NULL;  //头节点指针域置NULL
        head->tail = head;  // 开始时尾指针指向头节点
        return head;

    }
}
void sockNodeAdd(SockNode *head, SockNode *pnew)
{
    {
        pnew->id = head->num;
        pnew->next = NULL;   //新节点指针域置NULL
        head->tail->next = pnew;  //新节点插入到表尾
        head->tail = pnew;   //为指针指向当前的尾节点
        head->num++;
    }
}

struct sockInfo *create() {
    SockNode *head,*tail,*pnew;
    //int score;
    int id;
    head = (SockNode *)malloc(sizeof(SockNode));  //创建头节点。
    if (head == NULL) { //创建失败返回
        MYPRINT("创建失败！");
        return NULL;
    }
    head->next = NULL;  //头节点指针域置NULL
    tail = head;  // 开始时尾指针指向头节点
#if 0
    MYPRINT("输入学生成绩：");
    while (1) { //创建链表
        scanf("%d",&id);
        if (id<0) //成绩为负是退出循环
            break;
        pnew=(SockNode *)malloc(sizeof(SockNode));  //创建新节点
        if (pnew==NULL) { //创建失败返回
            MYPRINT("创建失败！");
            return NULL;
        }
        pnew->id = id;  //新节点数据域存放输入的成绩
        pnew->next = NULL;   //新节点指针域置NULL
        tail->next = pnew;  //新节点插入到表尾
        tail = pnew;   //为指针指向当前的尾节点
    }
#endif
    return head;  //返回创建链表的头指针
}
void insert(SockNode *head,SockNode *pnew,int i) {
    SockNode *p; //当前指针
    int j;

    p = head;
    for (j = 0; j < i && p != NULL; j++) //p指向要插入的第i个节点
        p = p->next;

    if(head->next == NULL)
    {
        MYPRINT("insert: 空节点 \n");
    }
    else if (p == NULL) { //节点i不存在
        MYPRINT("insert: 与插入的节点不存在！\n");
        return;
    }

    pnew->next = p->next;   //插入节点的指针域指向第i个节点的后继节点
    p->next = pnew;    //犟第i个节点的指针域指向插入的新节点
}
void addnode(SockNode *head, SockNode *pnew) {
    SockNode *p; //当前指针
    p = head;
    while(p->next){
        p = p->next;
    }
    pnew->next = p->next;
    p->next = pnew;

}
void *findnode(SockNode *head, struct sockaddr_in addr_client)
{
    void *ret = 0;
    SockNode *p;
    p = head;
    do{
        p = p->next;
        SockNode *info = (SockNode *)p;
        if(p)
        {
            struct sockaddr_in *paddr = (struct sockaddr_in *)&info->addr_client;
            int flag = (paddr->sin_port - addr_client.sin_port) | (paddr->sin_addr.s_addr - addr_client.sin_addr.s_addr);
            if(!flag)
            {
                ret = (void *)p;
                break;
            }
        }
    }while(p->next);
    return ret;
}
void deletenode(SockNode *head, int id)
{
    SockNode *p,*q;
    p = head;
    while(p->next){
        SockNode *info = (SockNode *)p->next;
        if(info->id == id)
        {
            break;
        }
        p = p->next;
    }
    if (p->next == NULL) { //表明链表中的节点不存在
        MYPRINT("deletenode: 不存在！\n");
        return;
    }
    q = p->next;  //q指向待删除的节点
    p->next = q->next;  //删除节点i，也可写成p->next=p->next->next
    free(q);   //释放节点i的内存单元
}
void pdelete(SockNode *head,int i) {
    SockNode *p,*q;
    int j;
    if (i == 0) //删除的是头指针，返回
        return;
    p = head;
    for (j = 1; j < i && p->next != NULL; j++)
        p = p->next;  //将p指向要删除的第i个节点的前驱节点
    if (p->next == NULL) { //表明链表中的节点不存在
        MYPRINT("pdelete: 不存在！\n");
        return;
    }
    q = p->next;  //q指向待删除的节点
    p->next = q->next;  //删除节点i，也可写成p->next=p->next->next
    free(q);   //释放节点i的内存单元
}
void display(SockNode *head) {
    SockNode *p;
    MYPRINT("display: id=");
    for (p = head->next; p != NULL; p = p->next)
        MYPRINT("%d ",p->id);
    MYPRINT("\n");
}
void pfree(SockNode *head) {
    SockNode *p,*q;

    p = head;
    while (p->next != NULL) { //每次删除头节点的后继节点
        q = p->next;
        p->next = q->next;
        free(q);
    }
    free(head);   //最后删除头节点
}
void Pfree(SockNode *head) {
    SockNode *p,*q;
    p = head;
    while (p->next != NULL) {
        q = p->next;
        p->next = q->next;
        free(q);
    }
    free(p);
}
int list_main() {
    struct sockInfo *head,*pnew;
    head = create();
    if (head == NULL)
        return 0;
    MYPRINT("输出创建的链表：");
    display(head);
    pnew=(SockNode *)malloc(sizeof(SockNode));
    if (pnew==NULL) {
        MYPRINT("创建失败！");
        return 0;
    }
    pnew->id = 88;
    //pnew->sock = NULL;
    insert(head,pnew, 0);   //将新节点插入节点3的后面
    MYPRINT("插入后的链表：");
    display(head);
    pdelete(head,0);   //删除节点3
    MYPRINT("删除后的链表：");
    display(head);
    Pfree(head);
    return 0;
}
//
int HBLoop(SocketObj *obj, int64_t now_time, int interval)
{
    int ret = 0;
    //self.lock.acquire()
    int64_t last_time = obj->last_send_time;//  # 避免意外包
    //self.lock.release()
    int difftime = (int)(now_time - last_time);
    //interval = HEARTBEAT_TIME
    //# wait_time = interval - diff #199, 200, 300
    //# wait_time = 1 if wait_time < 1 else wait_time
    //# 周期内无网络传输；
    //# 周期内有网络传输，且在100秒前;
    //# 100秒内已发生过网络传输，则取消当下发送；
    //# 心跳包最大间隔为为300s
    if(difftime > (interval >> 1))
    {
        SOCKFD sock_fd = obj->sock_fd;
        struct sockaddr_in addr_serv = obj->addr_serv;
        int len = sizeof(addr_serv);
        int send_num = 0;
        char send_buf[sizeof(UserData)];
        UserData *udata = (UserData *)send_buf;
        udata->data1.head.datatype = kCMD;
        int size = sizeof(CmdData);
        udata->data1.head.size = size;
        udata->data1.cmdtype = (CMDType)kHeartBeat;
        udata->data1.status = 1;
        udata->data1.chanId = obj->id;
        send_num = sendto(sock_fd, send_buf, size, 0, (struct sockaddr *)&addr_serv, len);//strlen(send_buf)
        if(send_num <= 0)
        {
            ret = -1;
            if(errno != EAGAIN && !send_num)
            {
                MYPRINT("HBLoop: send_num=%d \n", send_num);
                perror("HBLoop: sendto error:");
                MYPRINT("HBLoop: errno=%d \n", errno);
                ret = -2;
                //StopBroadCast(obj);
            }
        }
        obj->last_send_time = now_time;
        ret = 1;
    }
    return ret;
}
void StartBroadCast(SocketObj *obj)
{
    pthread_mutex_lock(&obj->status_lock);
    obj->broadcast_status = 1;
    pthread_mutex_unlock(&obj->status_lock);
}
void StartSend(SocketObj *obj)
{
    pthread_mutex_lock(&obj->status_lock);
    obj->send_status = 1;
    pthread_mutex_unlock(&obj->status_lock);
}
void StartRecv(SocketObj *obj)
{
    pthread_mutex_lock(&obj->status_lock);
    obj->recv_status = 1;
    pthread_mutex_unlock(&obj->status_lock);
}

void StopBroadCast(SocketObj *obj)
{
    pthread_mutex_lock(&obj->status_lock);
    if(obj->broadcast_status > 0)
    {
        obj->broadcast_status = 0;
    }
    pthread_mutex_unlock(&obj->status_lock);
}
void StopSend(SocketObj *obj)
{
    pthread_mutex_lock(&obj->status_lock);
    if(obj->send_status > 0)
    {
        obj->send_status = 0;
    }
    pthread_mutex_unlock(&obj->status_lock);
}
void StopRecv(SocketObj *obj)
{
    pthread_mutex_lock(&obj->status_lock);
    if(obj->recv_status > 0)
    {
        obj->recv_status = 0;
    }
    pthread_mutex_unlock(&obj->status_lock);
}
//
void ExitBroadCast(SocketObj *obj)
{
    pthread_mutex_lock(&obj->status_lock);
    obj->broadcast_status = -1;
    pthread_mutex_unlock(&obj->status_lock);
}
void ExitSend(SocketObj *obj)
{
    pthread_mutex_lock(&obj->status_lock);
    obj->send_status = -1;
    pthread_mutex_unlock(&obj->status_lock);
}
void ExitRecv(SocketObj *obj)
{
    pthread_mutex_lock(&obj->status_lock);
    obj->recv_status = -1;
    pthread_mutex_unlock(&obj->status_lock);
}
void WaitBroadCast(SocketObj *obj)
{
    int status;
    do{
        pthread_mutex_lock(&obj->status_lock);
        status = obj->broadcast_status;
        pthread_mutex_unlock(&obj->status_lock);
        if(status >= 0)
        {
            usleep(100000);//100ms
        }
    }while(status >= 0);
}
void WaitSend(SocketObj *obj)
{
    int status;
    do{
        pthread_mutex_lock(&obj->status_lock);
        status = obj->send_status;
        pthread_mutex_unlock(&obj->status_lock);
        if(status >= 0)
        {
            usleep(100000);//100ms
        }
    }while(status >= 0);
}
void WaitRecv(SocketObj *obj)
{
    int status;
    do{
        pthread_mutex_lock(&obj->status_lock);
        status = obj->recv_status;
        pthread_mutex_unlock(&obj->status_lock);
        if(status >= 0)
        {
            usleep(100000);//100ms
        }
    }while(status >= 0);
}

void SetPause(SocketObj *obj)
{
    pthread_mutex_lock(&obj->status_lock);
    if(obj->pause <= 0)
    {
        obj->pause = 1;
    }
    pthread_mutex_unlock(&obj->status_lock);
}
void ClearPause(SocketObj *obj)
{
    pthread_mutex_lock(&obj->status_lock);
    if(obj->pause > 0)
    {
        obj->pause = 0;
    }
    pthread_mutex_unlock(&obj->status_lock);
}
void WaitPause(SocketObj *obj)
{
    int status;
    do{
        pthread_mutex_lock(&obj->status_lock);
        if(obj->pause == 1)
        {
            obj->pause = 2;
        }
        status = obj->pause;
        pthread_mutex_unlock(&obj->status_lock);
        if(status > 0)
        {
            usleep(100000);//100ms
        }
    }while(status > 0);
}
void WaitPause2(SocketObj *obj)
{
    int status;
    do{
        pthread_mutex_lock(&obj->status_lock);
        status = obj->pause;
        pthread_mutex_unlock(&obj->status_lock);
        if(status != 2)
        {
            usleep(100000);//100ms
        }
    }while(status != 2);
}
void PacedSend(int64_t bitrate, int sendDataLen, int difftime, int64_t sumbytes)
{
    int usedtime = difftime * 1000;//us
    //int64_t bitrate = 1 * 1024 * 1024 * 1024;//bps
    int64_t sendbits = sendDataLen << 3;
    int64_t pretime = (sendbits * 1000000) / bitrate; //us
    int64_t sumbits = sumbytes << 3;
    int64_t sumpretime = (sumbits * 1000000) / bitrate;
    //MYPRINT("PacedSend: sumpretime=%lld \n", sumpretime);
    int tailtime = (int)(sumpretime - usedtime); // us
    //MYPRINT("PacedSend: tailtime=%d \n", tailtime);
    int waittime = (int)(tailtime + pretime); //us
    //MYPRINT("PacedSend: waittime=%d \n", waittime);
    //waittime = pretime #test
    if(waittime > 0)
    {
        //MYPRINT("PacedSend: waittime=%d (us) \n", waittime);
        usleep(waittime);
    }
}

int PushData(SocketObj *obj, char *recv_buf, int recv_num, struct sockaddr_in addr_client)
{
    int ret = 0;
    pthread_mutex_lock(&obj->lock);
    DataNode *head,*pnew, *q;
    if(!obj->data_list)
    {
        //head不存数据,head->next指向第一个数据,head->num保存总索引（也可以设计将第一个节点存入head）
        //注意链表元的2个组成部分，元和next指针，元控制的是整个链表元的地址，next指针则寄生在元下；
        //MYPRINT("PushData: create head: obj->data_list=%x \n", obj->data_list);
        head = (DataNode *)malloc(sizeof(DataNode));  //创建头节点。
        head->num = 0;
        head->next = NULL;  //头节点指针域置NULL
        head->tail = head;  // 开始时尾指针指向头节点；此时head->tail->next与head->next是同一地址
        obj->data_list = (void *)head;
    }
    head = (DataNode *)obj->data_list;
#ifdef CLIENT_MODE
    if(head->num > 100)
    {
        {
            UserData *udata = (UserData *)recv_buf;
            if(udata->data0.head.datatype)
            {
                q = head->next;
                UserData *udata0 = (UserData *)q->data;
                int64_t now_time0 = udata0->data0.now_time;//get_sys_time();
                int64_t now_time = udata->data0.now_time;
                int chanId = udata0->data0.chanId;
                int difftime = (int)(now_time - now_time0);
                if(difftime > 1000)
                {
                    MYPRINT("udpbase: PushData: difftime=%d, head->num=%d, chanId=%d \n", difftime, head->num, chanId);
                }
                ret = difftime;
            }
        }
    }
    if(head->num > MAX_DELAY_PKT_NUM)
    {
        free(recv_buf);
        MYPRINT("udpbase: PushData: skip head->num=%d \n", head->num);
    }
    else
#endif
    {
        pnew = (DataNode *)malloc(sizeof(DataNode));  //创建新节点
        pnew->addr_client = addr_client;
        pnew->data = (uint8_t *)recv_buf;//(uint8_t *)malloc(recv_num * sizeof(uint8_t));
        pnew->size = recv_num;
        pnew->idx = head->idx;
        pnew->now_time = get_sys_time();
        pnew->next = NULL;   //新节点指针域置NULL
        head->tail->next = pnew;  //新节点插入到表尾；首个节点时，则意味着ead->next也指向了pnew
        head->tail = pnew;   //为指针指向当前的尾节点;元地址迁移，元的原先的next脱离了元，新地址的next成为元的next指针；
        head->idx++;
        head->num++;
        //MYPRINT("PushData: head->num=%d \n", head->num);
    }
    pthread_mutex_unlock(&obj->lock);
    return ret;
}
void * PopData(SocketObj *obj)
{
    void *ret = NULL;
    //MYPRINT("PopData: obj=%x \n", obj);
    pthread_mutex_lock(&obj->lock);
    DataNode *head, *q;
    head = (DataNode *)obj->data_list;
    if(head)
    {
        q = head->next;
        if(q == NULL || q == head)
        {

        }
        else{
            ret = q;
#ifdef CLIENT_MODE
            if(head->num > 100)
            {
                UserData *udata = (UserData *)q->data;
                if(udata->data0.head.datatype)
                {
                    int64_t now_time0 = udata->data0.now_time;
                    int64_t now_time = get_sys_time();
                    int difftime = (int)(now_time - now_time0);
                    if(difftime > 1000)
                    {
                        MYPRINT("udpbase: PopData: difftime=%d, head->num=%d \n", difftime, head->num);
                    }
                }
            }
#endif
            //MYPRINT("PopData: q->data=%x \n", q->data);
            head->next = head->next->next;
            head->num--;
            if(head->next == NULL)
            {
                head->next = NULL;  //头节点指针域置NULL
                head->tail = head;
            }
        }
    }
    pthread_mutex_unlock(&obj->lock);
    //MYPRINT("PopData: ret=%x \n", ret);

    return ret;
}
void * PopDataAll(SocketObj *obj)
{
    //减少临界阻塞
    //当发包和收包速率相当时,累积包会以2倍的指数增长,即延迟会越来越大;
    //因此,实际的最大收包速率是其上限的二分之一;
    void *ret = NULL;
    pthread_mutex_lock(&obj->lock);
    ret = obj->data_list;
#ifdef CLIENT_MODE____
    DataNode *head, *q;
    head = (DataNode *)obj->data_list;
    if(head && head->num > 100)
    {
        q = head->next;
        UserData *udata = (UserData *)q->data;
        if(udata->data0.head.datatype)
        {
            int64_t now_time0 = udata->data0.now_time;
            int64_t now_time = get_sys_time();
            int difftime = (int)(now_time - now_time0);
            if(difftime > 1000)
            {
                MYPRINT("udpbase: PopDataAll: difftime=%d, head->num=%d \n", difftime, head->num);
            }
        }
    }
#endif
    obj->data_list = NULL;
    pthread_mutex_unlock(&obj->lock);
    return ret;
}
int AssignChanId(SessionInfo *ssinfo, uint32_t randId)
{
    int ret = -1;
    int64_t now_time = get_sys_time();
    for(int i = 0; i < MAX_CHAN_NUM; i++)
    {
        if(randId == ssinfo->randIdList[i])
        {
            //已经申请，并在确认中
            return -2 - i;
        }
    }
    for(int i = 0; i < MAX_CHAN_NUM; i++)
    {
        int chanId = ssinfo->chanIdList[i];
        if(chanId == -1)
        {
            ret = i;
            ssinfo->chanIdList[i] = -2 - i;//# 在确认中
            ssinfo->randIdList[chanId] = randId;
            break;
        }
    }
    if(ret < 0)
    {
        MYPRINT("error: AssignChanId: ret=%d \n", ret);
        return ret;
        //从失联中获取
        int64_t recv_time = now_time;//test
        int difftime = (int)(now_time - recv_time);
        if(difftime > (HEARTBEAT_TIME << 1))
        {
            int chanId = 0;//test
            if(chanId >= 0)
            {
                ret = chanId;
                ssinfo->chanIdList[chanId] = -2 - chanId;//# 在确认中
                ssinfo->randIdList[chanId] = randId;
            }
            //delete socket
        }
    }
    return ret;
}
int server_broadcast_run(SocketObj *obj)
{
    int ret = 0;
    pthread_mutex_lock(&obj->status_lock);
    obj->broadcast_status = 1;
    int status = obj->broadcast_status;
    pthread_mutex_unlock(&obj->status_lock);
    SOCKFD sock_fd = obj->sock_fd;
    struct sockaddr_in addr_serv = obj->addr_serv;

    struct sockaddr_in *addr_client;
    int len = sizeof(addr_serv);
    char send_buf[sizeof(UserData)] = "";
    int datatype = kCMD;
    UserData *udata = (UserData *)send_buf;
    udata->data1.head.datatype = datatype;
    int send_num = sizeof(CmdData);
    udata->data1.head.size = send_num;
    udata->data1.cmdtype = (CMDType)kBroadCast;
    udata->data1.status = 1;
    udata->data1.chanId = 0;
    udata->data1.avtype = 0;
    udata->data1.selfmode = 0;
    udata->data1.testmode = 1;
    udata->data1.modeId = 0;
    udata->data1.actor = 1;
    udata->data1.sessionId = 1;
    int interval = MAX_INTERVAL;//2000;//2s
    int64_t sendsize = 0;
    while(status > 0)
    {
        int64_t time0 = get_sys_time();
        for(int i = 0; i < obj->session_idx; i++)
        {
            SessionInfo *ssinfo0 = &obj->SessionList[i];
            udata->data1.sessionId = ssinfo0->sessionId;
            udata->data1.modeId = ssinfo0->modeId;
            udata->data1.selfmode = ssinfo0->selfmode;
            //udata->data1.width = ssinfo0->width;
            //udata->data1.height = ssinfo0->height;
            udata->data1.testmode = ssinfo0->testmode;
            udata->data1.status = ssinfo0->status;
            //udata->data1.nettime = ssinfo0->nettime;
            udata->data1.liveTime = ssinfo0->liveTime;
            //udata->data1.startTime = ssinfo0->startTime;
            //for(int j = 0; j < obj->session_idx; j++)
            //if(false)//test
            {
                //SessionInfo *ssinfo1 = &GlobSessionList[obj->session_idx];
                SockNode *head = obj->broadCastHead;
                SockNode *q;
                q = head->next;
                do{
                    addr_client = &q->addr_client;
                    //
                    send_num = sendto(sock_fd, send_buf, send_num, 0, (struct sockaddr *)addr_client, len);
                    if(send_num <= 0)
                    {
                        if(errno != EAGAIN && !send_num)
                        {
                            MYPRINT("server_broadcast_run: send_num=%d \n", send_num);
                            perror("server_broadcast_run: sendto error:");
                            MYPRINT("server_broadcast_run: errno=%d \n", errno);
                            ret = -2;
                            //StopBroadCast(obj);
                        }
                    }
                    else{
                        //MYPRINT("server_broadcast_run: send_num=%d \n", send_num);
                        sendsize += send_num;
                    }

                    //obj->pkt_send_num++;
                    if(!q->next)
                    {
                        break;
                    }
                    q = q->next;
                }while(1);
            }
        }
        int64_t now_time = get_sys_time();
        int difftime = (int)(now_time - time0);
        int wait_time = interval - difftime;
        if(wait_time > 0)
        {
            usleep(wait_time * 1000);
            float bitrate = (float)(((sendsize << 3) * 1.0) / (interval));
            time0 = now_time;
            sendsize = 0;
            if(bitrate > 0)
                MYPRINT2("server_broadcast_run: bitrate=%5.1f (Kbps) \n", bitrate);
        }
        pthread_mutex_lock(&obj->status_lock);
        status = (obj->broadcast_status > 0) & (obj->recv_status > 0); //obj->send_status ;
        pthread_mutex_unlock(&obj->status_lock);
    }
    ExitBroadCast(obj);
    MYPRINT("server_broadcast_run: over \n");

    char *p = malloc(32);
    strcpy(p,"server_broadcast_run exit");
    pthread_exit((void*)p);

    return ret;
}
int ProcessCmd(SocketObj *obj, char *send_buf, int send_num, struct sockaddr_in addr_client, SOCKFD sock_fd, int len, int64_t now_time)
{
    int ret = 1;
    struct sockInfo *pnew;
    CmdData *udata = (CmdData *)send_buf;
    int size = udata->head.size;
    int cmdtype = udata->cmdtype;
    MYPRINT("ProcessCmd: cmdtype=%d \n", cmdtype);
    switch(cmdtype)
    {
        case kBroadCastReg:
        {
            pnew = (SockNode *)malloc(sizeof(SockNode));
            if (pnew == NULL) {
                MYPRINT("创建失败！");
                ret = -2;
                //StopSend(obj);
            }
            pnew->id = (udata->sessionId << 16) | udata->chanId;
            //pnew->sock = (void *)paddr;
            pnew->addr_client = addr_client;
            pnew->startTime = now_time;
            pnew->status = 1;
            addnode(obj->broadCastHead, pnew);   //将新节点插入节点3的后面
            MYPRINT("插入后的链表：");
            display(obj->broadCastHead);
            //ack
            send_num = sendto(sock_fd, send_buf, send_num, 0, (struct sockaddr *)&addr_client, len);
            if(send_num <= 0)
            {
                if(errno != EAGAIN && !send_num)
                {
                    MYPRINT2("ProcessCmd: send_num=%d \n", send_num);
                    perror("ProcessCmd: sendto kBroadCastReg error:");
                    MYPRINT2("ProcessCmd: errno=%d \n", errno);
                    ret = -2;
                }
            }
            break;
        }
        case kGetSessionId:
        {
            SessionInfo *ssinfo = &obj->SessionList[obj->session_idx];
            if(udata->status == 1)
            {
                ssinfo->status = 1;
                //ssinfo->broadCast = addr_client;
                ssinfo->sessionId = obj->session_idx;//udata->sessionId;
                ssinfo->modeId = udata->modeId;
                ssinfo->selfmode = udata->selfmode;
                //ssinfo->width = udata->width;
                //ssinfo->height = udata->height;
                ssinfo->testmode = udata->testmode;
                //ssinfo->status;
                //ssinfo->nettime = udata->nettime;
                ssinfo->liveTime = udata->liveTime;
                ssinfo->startTime = now_time;
                ssinfo->randId = udata->randId;
                //
#if 0
                pnew = (SockNode *)malloc(sizeof(SockNode));
                if (pnew == NULL) {
                    MYPRINT("创建失败！");
                    ret = -2;
                    //StopSend(obj);
                }
                pnew->id = (udata->sessionId << 16) | udata->chanId;
                //pnew->sock = (void *)paddr;
                pnew->addr_client = addr_client;
                pnew->startTime = now_time;
                pnew->status = 1;
                addnode(ssinfo->broadCastHead, pnew);   //将新节点插入节点3的后面
                MYPRINT("插入后的链表：");
                display(ssinfo->broadCastHead);
#endif
                //ssinfo->decodecHead = create();
                ssinfo->decodecHead = sockNodeCreate();
                //反馈obj->session_idx
                udata->sessionId = obj->session_idx;
                for(int i = 0; i < MAX_CHAN_NUM; i++)
                {
                    ssinfo->chanIdList[i] = -1;
                }
                send_num = sendto(sock_fd, send_buf, send_num, 0, (struct sockaddr *)&addr_client, len);
                if(send_num <= 0)
                {
                    if(errno != EAGAIN && !send_num)
                    {
                        MYPRINT2("ProcessCmd: send_num=%d \n", send_num);
                        perror("ProcessCmd: sendto kGetSessionId error:");
                        MYPRINT2("ProcessCmd: errno=%d \n", errno);
                        ret = -2;
                    }
                }
                //丢包情况下，分辨重复申请: randId
                obj->session_idx++;
                //push to ack
            }
            else if(udata->status == 2)
            {
                ssinfo->status = 2;
                ssinfo->sessionId = udata->sessionId;
            }
            else if(udata->status == 0)
            {
                //delet session and all socket
            }
            break;
        }
        case kGetChanId:
        {
            int sessionId = udata->sessionId;
            if(sessionId < 0 || sessionId >= glob_session_num)
            {
                MYPRINT2("error: ProcessCmd: kGetChanId: sessionId=%d, obj->session_idx=%d \n", sessionId, obj->session_idx);
                break;
            }

            SessionInfo *ssinfo = &obj->SessionList[sessionId];
            if(ssinfo->status)
            {
                int chanId = udata->chanId;
                int actor = udata->actor;
                uint32_t randId = udata->randId;
                //MYPRINT("ProcessCmd: chanId=%d \n", chanId);
                //丢包情况下，分辨重复申请: randId
                if(chanId == -1)
                {
                    chanId = AssignChanId(ssinfo, randId);
                    udata->chanId = chanId;
                    send_num = sendto(sock_fd, send_buf, send_num, 0, (struct sockaddr *)&addr_client, len);
                    if(send_num <= 0)
                    {
                        if(errno != EAGAIN && !send_num)
                        {
                            MYPRINT2("ProcessCmd: send_num=%d \n", send_num);
                            perror("ProcessCmd: sendto kGetChanId error:");
                            MYPRINT2("ProcessCmd: errno=%d \n", errno);
                            ret = -2;
                        }
                    }
                }
                else{
                    if(chanId < 0 || chanId >= 128)
                    {
                        MYPRINT2("error: ProcessCmd: kGetChanId: chanId=%d \n", chanId);
                    }
                    ssinfo->chanIdList[chanId] = chanId;
                    ssinfo->randIdList[chanId] = randId;
                }
            }
            break;
        }
        case kReg:
        {
            int sessionId = udata->sessionId;
            ret = 0;
            if(sessionId < glob_session_num)//obj->session_idx
            {
                SessionInfo *ssinfo = &obj->SessionList[sessionId];
                MYPRINT2("ProcessCmd: sessionId=%d \n", sessionId);
                MYPRINT2("ProcessCmd: ssinfo->status=%d \n", ssinfo->status);
                if(ssinfo->status)
                {
                    int chanId = udata->chanId;
                    uint32_t randId = udata->randId;
                    int actor = udata->actor;
                    if(chanId < MAX_CHAN_NUM)
                    {
                        MYPRINT2("ProcessCmd: udata->actor=%d \n", udata->actor);
                        MYPRINT2("ProcessCmd: udata->chanId=%d \n", udata->chanId);
                        MYPRINT2("ProcessCmd: udata->sessionId=%d \n", udata->sessionId);

                        if(actor < DECACTOR)
                        {
                            ssinfo->chanIdList[chanId] = chanId;
                            //ssinfo->randIdList[chanId] = randId;
                            ssinfo->encodecClients[chanId].addr_client = addr_client;
                            ssinfo->encodecClients[chanId].startTime = now_time;
                            ssinfo->encodecClients[chanId].status = 1;
                            ssinfo->encodecClients[chanId].actor = actor;
                            ssinfo->encodecClients[chanId].chanId = chanId;
                        }
                        else if(actor == DECACTOR)
                        {
                            pnew = (SockNode *)malloc(sizeof(SockNode));
                            if (pnew == NULL) {
                                MYPRINT2("创建失败！");
                                ret = -2;
                                StopSend(obj);
                            }
                            pnew->id = (udata->sessionId << 16) | udata->chanId;
                            pnew->addr_client = addr_client;
                            pnew->startTime = now_time;
                            pnew->status = 1;
                            pnew->actor = actor;
                            pnew->chanId = chanId;
                            //addnode(ssinfo->decodecHead, pnew);   //将新节点插入节点3的后面
                            sockNodeAdd(ssinfo->decodecHead, pnew);
                            MYPRINT2("ProcessCmd: 插入后的链表：");
                            display(ssinfo->decodecHead);
                        }
                        //ack
                        send_num = sendto(sock_fd, send_buf, send_num, 0, (struct sockaddr *)&addr_client, len);
                        if(send_num <= 0)
                        {
                            if(errno != EAGAIN && !send_num)
                            {
                                MYPRINT2("ProcessCmd: send_num=%d \n", send_num);
                                perror("ProcessCmd: sendto kReg error:");
                                MYPRINT2("ProcessCmd: errno=%d \n", errno);
                                ret = -2;
                            }
                        }
                        ret = send_num;
                    }
                }
            }
            break;
        }
        case kHeartBeat:
        {
            send_num = sendto(sock_fd, send_buf, send_num, 0, (struct sockaddr *)&addr_client, len);
            if(send_num <= 0)
            {
                if(errno != EAGAIN && !send_num)
                {
                    MYPRINT2("ProcessCmd: send_num=%d \n", send_num);
                    perror("ProcessCmd: sendto kHeartBeat error:");
                    MYPRINT2("ProcessCmd: errno=%d \n", errno);
                    ret = -2;
                }
            }
            //#刷新时间
            break;
        }
        case kAsk:
        {
            //MYPRINT("ProcessCmd: kAsk \n");
            send_num = sendto(sock_fd, send_buf, send_num, 0, (struct sockaddr *)&addr_client, len);
            if(send_num <= 0)
            {
                if(errno != EAGAIN && !send_num)
                {
                    MYPRINT2("ProcessCmd: send_num=%d \n", send_num);
                    perror("ProcessCmd: sendto kHeartBeat error:");
                    MYPRINT2("ProcessCmd: errno=%d \n", errno);
                    ret = -2;
                }
            }
            //#刷新时间
            break;
        }
        case kRTT:
        {
            udata->rt0 = now_time;
            udata->st1 = get_sys_time();
            send_num = sendto(sock_fd, send_buf, send_num, 0, (struct sockaddr *)&addr_client, len);
            if(send_num <= 0)
            {
                if(errno != EAGAIN && !send_num)
                {
                    MYPRINT2("ProcessCmd: send_num=%d \n", send_num);
                    perror("ProcessCmd: sendto kRTT error:");
                    MYPRINT2("ProcessCmd: errno=%d \n", errno);
                    ret = -2;
                }
            }
            break;
        }
        case kBye:
        {
            int id = (udata->sessionId << 16) | udata->chanId;
            //deletenode(obj->sockHead, id);
            //MYPRINT("删除后的链表：");
            //display(obj->sockHead);
            //ack
            int sessionId = udata->sessionId;
            MYPRINT2("ProcessCmd: kBye: sessionId=%d \n", sessionId);
            if(sessionId >= 0 && sessionId < obj->session_idx)
            {
                SessionInfo *ssinfo = &obj->SessionList[sessionId];
                ssinfo->status = 0;
            }

            //udata->status = 0;
            //udata->cmdtype = kBroadCast;
            send_num = sendto(sock_fd, send_buf, send_num, 0, (struct sockaddr *)&addr_client, len);
            if(send_num <= 0)
            {
                if(errno != EAGAIN && !send_num)
                {
                    MYPRINT2("ProcessCmd: send_num=%d \n", send_num);
                    perror("ProcessCmd: sendto kBye error:");
                    MYPRINT2("ProcessCmd: errno=%d \n", errno);
                    ret = -2;
                }
            }
            break;
        }
        case kExit:
        {
            ret = 0;
            //ack
            send_num = sendto(sock_fd, send_buf, send_num, 0, (struct sockaddr *)&addr_client, len);
            if(send_num <= 0)
            {
                if(errno != EAGAIN && !send_num)
                {
                    MYPRINT2("ProcessCmd: send_num=%d \n", send_num);
                    perror("ProcessCmd: sendto kExit error:");
                    MYPRINT2("ProcessCmd: errno=%d \n", errno);
                    ret = -2;
                }
            }
            MYPRINT2("ProcessCmd: kExit \n");
            StopRecv(obj);
            //StopBroadCast(obj);//test
            //StopSend(obj);
            break;
        }
        default:
            break;
    }
    return ret;
}
int RelayData(SocketObj *obj, char *send_buf, int send_num, struct sockaddr_in addr_client, SOCKFD sock_fd, int len)
{
    int ret = 0;
    AVData *udata = (AVData *)send_buf;
    int size = udata->head.size;
    int seqnum = udata->seqnum;
#ifdef LOOPRELAY
    send_num = sendto(sock_fd, send_buf, send_num, 0, (struct sockaddr *)&addr_client, len);
    if(send_num <= 0)// || send_num != q->size)
    {
        if(errno != EAGAIN && !send_num)
        {
            MYPRINT2("RelayData: send_num=%d \n", send_num);
            perror("RelayData: sendto error:");
            MYPRINT2("RelayData: errno=%d \n", errno);
            ret = -2;
            //StopBroadCast(obj);
        }
    }
    ret += send_num;
#else
    //get sessionId,
    int sessionId = udata->sessionId;
    int chanId0 = udata->chanId;
    //MYPRINT2("RelayData: obj->session_idx=%d \n", obj->session_idx);
    if(sessionId >= obj->session_idx || sessionId < 0)
    {
        MYPRINT2("RelayData: obj->session_idx=%d \n", obj->session_idx);
        MYPRINT2("RelayData: sessionId=%d \n", sessionId);
        MYPRINT2("RelayData: chanId0=%d \n", chanId0);
        return ret;
    }
    SessionInfo *ssinfo = &obj->SessionList[sessionId];
    SockNode *head = ssinfo->decodecHead;
    if(head == NULL)
    {
        MYPRINT2("RelayData: head=%x \n", head);
    }
    //return ret;//test
    //else if(head->num == 0)
    //{
    //}
    int count = 0;
    SockNode *q = head->next;
    while(q){
        //MYPRINT("RelayData: q=%x \n", q);
        //MYPRINT("RelayData: q->chanId=%d \n", q->chanId);
        //MYPRINT("RelayData: q->actor=%d \n", q->actor);
        //MYPRINT("RelayData: 0: count=%d \n", count);
        if(ssinfo->selfmode || (chanId0 == q->chanId))
        {
            struct sockaddr_in addr_client2 = q->addr_client;
            //MYPRINT("RelayData: send_num=%d \n", send_num);
            //MYPRINT("RelayData: addr_client2.sin_port=%d \n", addr_client2.sin_port);
            send_num = sendto(sock_fd, send_buf, send_num, 0, (struct sockaddr *)&addr_client2, len);
            //MYPRINT("RelayData: send_num=%d \n", send_num);
            if(send_num <= 0)
            {
                if(errno != EAGAIN && !send_num)
                {
                    MYPRINT2("RelayData: send_num=%d \n", send_num);
                    perror("RelayData: sendto error:");
                    MYPRINT2("RelayData: errno=%d \n", errno);
                    ret = -2;
                    //StopBroadCast(obj);
                }
            }
            ret += send_num;
        }
        else{
            //MYPRINT("RelayData: chanId0=%d \n", chanId0);
            //MYPRINT("RelayData: q->chanId=%d \n", q->chanId);
        }

        q = q->next;
        count++;
    }
    //MYPRINT("RelayData: 1: count=%d \n", count);
    //MYPRINT("RelayData: end \n");
#endif
    return ret;
}
int server_recv_run(SocketObj *obj, SOCKFD sock_fd, struct sockaddr_in addr_serv)
{
    int ret = 0;
    pthread_mutex_lock(&obj->status_lock);
    obj->recv_status = 1;
    int status = obj->recv_status;
    pthread_mutex_unlock(&obj->status_lock);

    int recv_num;
    //char recv_buf[sizeof(UserData)];
    struct sockaddr_in addr_client;
    int len = sizeof(addr_serv);
    obj->start_time = get_sys_time();
    int64_t time0 = get_sys_time();
    int64_t recvsize = 0;
    int data_len = sizeof(UserData);
    char *recv_buf;
    //recv_buf = (char *)malloc(data_len);
    obj->pkt_recv_num = 0;
    while(status > 0)
    {
        int64_t now_time = get_sys_time();
        //MYPRINT("recv_run wait:\n");
        recv_buf = (char *)malloc(data_len);
        recv_num = recvfrom(sock_fd, recv_buf, data_len, 0, (struct sockaddr *)&addr_client, (socklen_t *)&len);
        if(recv_num <= 0)
        {
            if(errno != EAGAIN && !recv_num)
            {
                MYPRINT2("server_recv_run: recv_num=%d \n", recv_num);
                free(recv_buf);
                perror("server_recv_run: recvfrom error:");
                MYPRINT2("server_recv_run: errno=%d \n", errno);
                if(!recv_num)
                {
                    ret = -1;
                    break;
                }
            }

        }
        else{
            recvsize += recv_num;
            obj->pkt_recv_num++;
            //MYPRINT("server_main: recv_num=%d \n", recv_num);
            //MYPRINT("server_main: addr_client.sin_port=%d \n", addr_client.sin_port);
            //MYPRINT("server_main: addr_client.sin_addr.s_addr=%d \n", addr_client.sin_addr.s_addr);
            PushData(obj, recv_buf, recv_num, addr_client);
        }


        int difftime = (int)(now_time - time0);
        if(difftime > MAX_INTERVAL)
        {
            float bitrate = (float)(((recvsize << 3) * 1.0) / (difftime * 1000));
            //MYPRINT("server_main: recvsize=%lld \n", recvsize);
            time0 = now_time;
            recvsize = 0;
            if(bitrate > 0)
                MYPRINT2("server_recv_run: bitrate=%5.2f (Mbps) \n", bitrate);
        }
        pthread_mutex_lock(&obj->status_lock);
        status = obj->recv_status;
        pthread_mutex_unlock(&obj->status_lock);
    }
    ExitRecv(obj);
    StopSend(obj);
    StopBroadCast(obj);
    WaitSend(obj);
    WaitBroadCast(obj);
    MYPRINT2("server_recv_run: over \n");
    return ret;
}
int send_data(SocketObj *obj, DataNode *head, SOCKFD sock_fd, int len)
{
    int ret = 0;
    struct sockaddr_in addr_client;
    if(head && head->num)
    {
        //MYPRINT("send_data: head->num=%d \n", head->num);
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
            char *send_buf = (char *)q->data;
            int send_num = q->size;
            addr_client = q->addr_client;
#if 0
            //if(!q->data || !q->size || q->size > 1500 || q->idx >= (head->num - 1))
            if(q->idx >= (head->num - 1))
            {
                //MYPRINT("send_data: q->data=%x \n", q->data);
                //MYPRINT("send_data: q->size=%d \n", q->size);
                //MYPRINT("send_data: q->idx=%d \n", q->idx);
                MYPRINT("send_data: head->num=%d \n", head->num);
                MYPRINT("send_data: obj->pkt_send_num=%lld \n", obj->pkt_send_num);
                MYPRINT("send_data: obj->pkt_recv_num=%lld \n", obj->pkt_recv_num);
            }
#endif
            send_num = sendto(sock_fd, send_buf, send_num, 0, (struct sockaddr *)&addr_client, len);
            if(send_num <= 0)// || send_num != q->size)
            {
                if(errno != EAGAIN && !send_num)
                {
                    MYPRINT2("send_data: send_num=%d \n", send_num);
                    perror("send_data: sendto error:");
                    MYPRINT2("send_data: errno=%d \n", errno);
                    ret = -2;
                }
            }
            ret += send_num;
            obj->pkt_send_num++;
            free(q->data);
            free(q);   //释放节点i的内存单元
        }while(1);
        free(head);
    }
    else{
        //sleep()
        //MYPRINT("send_data: usleep \n");
        usleep(2000);//1ms
    }

    return ret;
}
int process_recv_data(SocketObj *obj, DataNode *head, SOCKFD sock_fd, int len, int64_t now_time)
{
    int ret = 0;
    struct sockaddr_in addr_client;
    if(head && head->num)
    {
        if(head->num > (MAX_DELAY_PKT_NUM - 10))
            MYPRINT2("process_recv_data: head->num=%d \n", head->num);
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
            char *send_buf = (char *)q->data;
            int send_num = q->size;
            addr_client = q->addr_client;
            now_time = q->now_time;
            //
            UserData *udata = (UserData *)send_buf;
            if(udata->data0.head.datatype == kDATA)
            {
                //MYPRINT("process_recv_data: q->idx=%d \n", q->idx);
                send_num = RelayData(obj, send_buf, send_num, addr_client, sock_fd, len);
            }
            else{
                //MYPRINT("process_recv_data: udata->data0.head.datatype=%d \n", udata->data0.head.datatype);
                send_num = ProcessCmd(obj, send_buf, send_num, addr_client, sock_fd, len, now_time);
            }

            ret += send_num;
            obj->pkt_send_num++;
            free(q->data);
            free(q);   //释放节点i的内存单元
        }while(1);
        free(head);
    }
    else{
        //sleep()
        //MYPRINT("process_recv_data: usleep \n");
        usleep(2000);//2ms
    }
    return ret;
}
int server_send_run(SocketObj *obj)
{
    int ret = 0;
    pthread_mutex_lock(&obj->status_lock);
    obj->send_status = 1;
    int status = obj->send_status;
    pthread_mutex_unlock(&obj->status_lock);
    SOCKFD sock_fd = obj->sock_fd;
    struct sockaddr_in addr_serv = obj->addr_serv;
    //struct sockInfo *head = NULL;

    int send_num = 10;
    char *send_buf;//[sizeof(UserData)] = "i am server!";
    struct sockaddr_in addr_client;
    int len = sizeof(addr_serv);
    obj->start_time = get_sys_time();
    int64_t time0 = get_sys_time();
    int64_t sendsize = 0;
    obj->pkt_send_num = 0;
    while(status > 0)
    {
        int64_t now_time = get_sys_time();
        DataNode *head = (DataNode *)PopDataAll(obj);
        //MYPRINT("server_send_run:  \n");
        //sendsize += send_data(obj, head, sock_fd, len);
        sendsize += process_recv_data(obj, head, sock_fd, len, now_time);
        int difftime = (int)(now_time - time0);
        if(difftime > MAX_INTERVAL)
        {
            float bitrate = (float)(((sendsize << 3) * 1.0) / (difftime * 1000));
            //MYPRINT("server_main: recvsize=%lld \n", recvsize);
            time0 = now_time;
            sendsize = 0;
            if(bitrate > 0)
                MYPRINT2("server_send_run: bitrate=%5.2f (Mbps) \n", bitrate);
        }
        pthread_mutex_lock(&obj->status_lock);
        status = obj->send_status;
        pthread_mutex_unlock(&obj->status_lock);
    }
    ExitSend(obj);
    MYPRINT2("server_send_run: over \n");

    char *p = malloc(32);
    strcpy(p,"server_send_run exit");
    pthread_exit((void*)p);

    return ret;
}
void * server_main(SocketObj *obj)
{
    int ret = 0;
    MYPRINT2("server_main: obj->params=%s \n", obj->params);
    obj->session_idx = 0;
    obj->SessionList = (SessionInfo *)calloc(1, glob_session_num * sizeof(SessionInfo));
    cJSON * json = (cJSON *)api_str2json(obj->params);
    obj->port = GetvalueInt(json, "port");
    GetvalueStr(json, "server_ip", obj->server_ip);
    obj->recv_status = 1;
    int taskId = obj->id;
    /* sock_fd --- socket文件描述符 创建udp套接字*/
    obj->sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(obj->sock_fd < 0)
    {
        perror("socket");
        return 0;
        //exit(1);
    }

    /* 将套接字和IP、端口绑定 */
    //struct sockaddr_in addr_serv;
    //int len;
    memset(&obj->addr_serv, 0, sizeof(struct sockaddr_in));  //每个字节都用0填充
    obj->addr_serv.sin_family = AF_INET;                       //使用IPV4地址
    obj->addr_serv.sin_port = htons(obj->port);                //端口
    /* INADDR_ANY表示不管是哪个网卡接收到数据，只要目的端口是obj->port，就会被该应用程序接收到 */
    obj->addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);  //自动获取IP地址
    //
#if 0
    int rv;
    struct addrinfo infotest;
    struct addrinfo hints, *servinfo;
    memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;//SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, addr_serv.sin_port, &hints, &servinfo)) != 0)
    {
#if defined (WIN32)
        fprintf(stderr, "getaddrinfo: WSAGetLastError=%d\n",  WSAGetLastError());
#elif defined(__linux__)
        fprintf(stderr, "getaddrinfo: gai_strerror=%s\n", gai_strerror(rv));
#endif
	}
#endif
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
		//error = errno;
		//while ((close(obj->sock_fd) == -1) && (errno == EINTR));
		//errno = error;
		return 0;
	}
	ret = setsockopt(obj->sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, len);
	if (ret == -1) {
	    return 0;
	}
    //
    len = sizeof(obj->addr_serv);
    /* 绑定socket */
    if(bind(obj->sock_fd, (struct sockaddr *)&obj->addr_serv, sizeof(obj->addr_serv)) < 0)
    {
        perror("server_main: bind error:");
        return 0;
        //exit(1);
    }
    pthread_mutex_init(&obj->lock,NULL);
    pthread_mutex_init(&obj->status_lock,NULL);
    //
    struct sockInfo *pnew;
    obj->broadCastHead = create();
    if (obj->broadCastHead == NULL)
        return 0;
    MYPRINT2("输出创建的链表：");
    display(obj->broadCastHead);
    //
    StartRecv(obj);
    //
    pthread_t tid;
    pthread_attr_t  attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,  PTHREAD_CREATE_DETACHED);
    //if(pthread_create(&tid, &attr, server_send_run, obj) < 0)
    if(pthread_create(&tid, NULL, server_send_run, obj) < 0)
    {
        MYPRINT2("server_main: Create server_send_run failed!\n");
        //perror("server_main: server_send_run thread error:");
    }
    pthread_t tid2;
    if(pthread_create(&tid2, NULL, server_broadcast_run, obj) < 0)
    {
        MYPRINT2("server_main: Create server_broadcast_run failed!\n");
        //perror("server_main: server_broadcast_run thread error:");
    }
    //
    //obj->recv_status = 1;
    //obj->send_status = 1;
    server_recv_run(obj, obj->sock_fd, obj->addr_serv);
    char *p0;
    if (pthread_join(tid, (void**)&p0))
    {
        MYPRINT2("server_main: server_send_run thread is not exit...\n");
    }
    else{
        MYPRINT2("server_main: p0=%s \n", p0);
        free(p0);
    }


    char *p1;
    if (pthread_join(tid2, (void**)&p1))
    {
        MYPRINT2("server_main: server_broadcast_run thread is not exit...\n");
    }
    else{
        MYPRINT2("server_main: p1=%s \n", p1);
        free(p1);
    }

    Pfree(obj->broadCastHead);
#if defined (WIN32)
    closesocket(obj->sock_fd);
#elif defined(__linux__)
    close(obj->sock_fd);
#endif
    pthread_mutex_destroy(&obj->lock);
    pthread_mutex_destroy(&obj->status_lock);
    MYPRINT2("server_main: over \n");
    return 0;
}
int send_data2server(SocketObj *obj, int64_t now_time)
{
    int ret = 0;
    SOCKFD sock_fd = obj->sock_fd;
    struct sockaddr_in addr_serv = obj->addr_serv;
    int len = sizeof(addr_serv);

    char send_buf[sizeof(UserData)] = "";//"hey, who are you?";
    int datatype = kDATA;
    UserData *udata = (UserData *)send_buf;
    int size = sizeof(AVData);
    int difftime = (int)(now_time - obj->start_time);
    if( difftime > TEST_TIME_LEN )
    //if(seqnum > 10)
    {
        MYPRINT("send_data2server: difftime=%d \n", difftime);
        datatype = kCMD;
        obj->send_status = 0;
    }
    else if(!obj->pkt_send_num)
    {
        datatype = kCMD;
    }
    else{
        datatype = kDATA;
    }
    if(datatype == kDATA)
    {
        udata->data0.head.datatype = datatype;
        size = sizeof(AVData);
        udata->data0.head.size = size;
        udata->data0.seqnum = obj->pkt_send_num;
    }
    else{
        udata->data1.head.datatype = datatype;
        size = sizeof(CmdData);
        udata->data1.head.size = size;
        if(!obj->pkt_send_num)
        {
            udata->data1.cmdtype = (CMDType)kReg;
            udata->data1.status = 1;
        }
        else{
            udata->data1.cmdtype = (CMDType)kExit;
            udata->data1.status = 0;
        }
        udata->data1.chanId = obj->id;
        udata->data1.avtype = 0;
        udata->data1.selfmode = 0;
        udata->data1.testmode = 1;
        udata->data1.modeId = 0;
        udata->data1.actor = 1;
        udata->data1.sessionId = 1;
    }
    ret = sendto(sock_fd, send_buf, size, 0, (struct sockaddr *)&addr_serv, len);//strlen(send_buf)
    if(ret <= 0)
    {
        if(errno != EAGAIN && !ret)
        {
            perror("send_data2server: sendto error:");
            MYPRINT("send_data2server: errno=%d \n", errno);
            ret = -1;
            //break;
        }
    }
    obj->last_send_time = now_time;
    //sendsize += send_num;
    obj->pkt_send_num++;
    return ret;
}

#ifndef LOOPRELAY
int client_send_run(SocketObj *obj)
{
    int ret = 0;
    pthread_mutex_lock(&obj->status_lock);
    obj->send_status = 1;
    int status = obj->send_status;
    pthread_mutex_unlock(&obj->status_lock);
    SOCKFD sock_fd = obj->sock_fd;
    struct sockaddr_in addr_serv = obj->addr_serv;
    int len = sizeof(addr_serv);
    obj->start_time = get_sys_time();
    int64_t time0 = get_sys_time();
    int64_t sendsize = 0;
    int pacedidx = 0;
    int pacedsumbytes = 0;
    int pacedstep = 1;//4;
    int64_t bitrate = obj->paced_bitrate ? obj->paced_bitrate : glob_paced_bitrate;

    while(status > 0)
    {
        int64_t now_time = get_sys_time();

        DataNode *head = (DataNode *)PopDataAll(obj);
        //MYPRINT("process_recv_data: head->num=%d \n", head->num);
        if(head && head->num)
        {
            //MYPRINT("client_send_run: head->num=%d \n", head->num);
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
                char *send_buf = (char *)q->data;
                int send_num = q->size;
                struct sockaddr_in addr_client = q->addr_client;
                //
                UserData *udata = (UserData *)send_buf;
                //
                int ret2 = sendto(sock_fd, send_buf, send_num, 0, (struct sockaddr *)&addr_client, len);//strlen(send_buf)
                //MYPRINT("client_send_run: ret2=%d \n", ret2);
                if(ret2 <= 0)
                {
                    if(errno != EAGAIN && !ret2)
                    {
                        perror("client_send_run: sendto error:");
                        MYPRINT("client_send_run: errno=%d \n", errno);
                        ret = -1;
                        StopSend(obj);
                    }
                }
                else{
                    now_time = get_sys_time();
                    obj->last_send_time = now_time;
                    int difftime = (int)(now_time - time0);
                    //
                    sendsize += ret2;
                    //MYPRINT("client_send_run: bitrate=%d \n", bitrate);
                    if(pacedstep > 1)
                    {
                        pacedsumbytes += ret2;
                        pacedidx++;
                        if(pacedidx > pacedstep)
                        {
                            PacedSend(bitrate, pacedsumbytes, difftime, sendsize);
                            pacedidx = 0;
                            pacedsumbytes = 0;
                        }
                    }
                    else{
                        PacedSend(bitrate, ret2, difftime, sendsize);
                    }
                    //MYPRINT("client_send_run: 2 \n");
                    //heart beat
                    int interval = HEARTBEAT_TIME;//300000;//30s
                    int ret2 = HBLoop(obj, now_time, interval);
                    if(ret2 == -2)
                    {
                        MYPRINT("client_send_run: HBLoop: ret2=%d \n", ret2);
                        StopSend(obj);
                    }
                    //MYPRINT("client_send_run: 3 \n");
                    //bit rate
                    if(difftime > MAX_INTERVAL)
                    {
                        float bitrate = (float)(((sendsize << 3) * 1.0) / (difftime * 1000));
                        //MYPRINT("server_main: recvsize=%lld \n", recvsize);
                        time0 = now_time;
                        sendsize = 0;
                        MYPRINT("client_send_run: bitrate=%5.2f (Mbps) \n", bitrate);
                    }
                    //
                    obj->pkt_send_num++;
                    //ret += ret2;
                }
                //
                free(q->data);
                free(q);   //释放节点i的内存单元
                //
                //
            }while(1);
            free(head);
        }
        else{
            //sleep()
            //MYPRINT("process_recv_data: usleep \n");
            usleep(2000);//1ms
        }

        pthread_mutex_lock(&obj->status_lock);
        status = obj->send_status;
        pthread_mutex_unlock(&obj->status_lock);
    }
    ExitSend(obj);
    StopRecv(obj);
    WaitRecv(obj);
    MYPRINT("client_send_run: over \n");

    char *p = malloc(32);
    strcpy(p,"client_send_run exit");
    pthread_exit((void*)p);

    return ret;
}
#else
int client_send_run(SocketObj *obj)
{
    int ret = 0;
    pthread_mutex_lock(&obj->status_lock);
    obj->send_status = 1;
    int status = obj->send_status;
    pthread_mutex_unlock(&obj->status_lock);
    obj->start_time = get_sys_time();
    int64_t time0 = get_sys_time();
    int64_t sendsize = 0;
    int pacedidx = 0;
    int pacedsumbytes = 0;
    int pacedstep = 1;//4;

    while(status > 0)
    {
        int64_t now_time = get_sys_time();

        ret = send_data2server(obj, now_time);
        //MYPRINT("client_send_run: ret=%d \n", ret);
        //paced send
        now_time = get_sys_time();
        int difftime = (int)(now_time - time0);
        int64_t bitrate = obj->paced_bitrate ? obj->paced_bitrate : glob_paced_bitrate;//PACED_BITRATE;//3 * 1024 * 1024 * 1024LL;//bps

        if(ret < 0)
        {
            MYPRINT("error: client_send_run: ret=%d \n", ret);
        }
        else{
            sendsize += ret;
            //MYPRINT("client_send_run: bitrate=%d \n", bitrate);
            if(pacedstep > 1)
            {
                pacedsumbytes += ret;
                pacedidx++;
                if(pacedidx > pacedstep)
                {
                    PacedSend(bitrate, pacedsumbytes, difftime, sendsize);
                    pacedidx = 0;
                    pacedsumbytes = 0;
                }
            }
            else{
                PacedSend(bitrate, ret, difftime, sendsize);
            }
            //heart beat
            int interval = HEARTBEAT_TIME;//300000;//30s
            int ret2 = HBLoop(obj, now_time, interval);
            if(ret2 == -2)
            {
                StopSend(obj);
            }
            //bit rate
            if(difftime > MAX_INTERVAL)
            {
                float bitrate = (float)(((sendsize << 3) * 1.0) / (difftime * 1000));
                //MYPRINT("server_main: recvsize=%lld \n", recvsize);
                time0 = now_time;
                sendsize = 0;
                MYPRINT("client_send_run: bitrate=%5.2f (Mbps) \n", bitrate);
            }
        }

        pthread_mutex_lock(&obj->status_lock);
        status = obj->send_status;
        pthread_mutex_unlock(&obj->status_lock);
    }
    ExitSend(obj);
    StopRecv(obj);
    WaitRecv(obj);
    MYPRINT("client_send_run: over \n");
    char *p = malloc(32);
    strcpy(p,"client_send_run exit");
    pthread_exit((void*)p);

    return ret;
}
#endif
int ClientProcessCmd(SocketObj *obj, char *recv_buf)
{
    int ret = 1;
    pthread_mutex_lock(&obj->lock);

    pthread_mutex_unlock(&obj->lock);
    return ret;
}
int client_recv_run(SocketObj *obj)
{
    int ret = 0;
    pthread_mutex_lock(&obj->status_lock);
    obj->recv_status = 1;
    int status = obj->recv_status;
    pthread_mutex_unlock(&obj->status_lock);
    SOCKFD sock_fd = obj->sock_fd;
    struct sockaddr_in addr_serv = obj->addr_serv;
    int len = sizeof(addr_serv);
    int recv_num;
    char *recv_buf = "";//[sizeof(UserData)] = "";//"hey, who are you?";
    int data_len = sizeof(UserData);
    //
    int datatype = kDATA;

    //obj->start_time = get_sys_time();
    int64_t time0 = get_sys_time();
    int64_t recvsize = 0;
    int series = 0;

    while(status)
    {
        int64_t now_time = get_sys_time();
        recv_buf = (char *)malloc(data_len);
        recv_num = recvfrom(sock_fd, recv_buf, data_len, 0, (struct sockaddr *)&addr_serv, (socklen_t *)&len);
        if(recv_num <= 0)
        {
            series++;
            if(errno != EAGAIN && !recv_num)
            {
                perror("client_recv_run: recvfrom error:");
                MYPRINT("client_recv_run: errno=%d \n", errno);
                if(!recv_num)
                {
                    ret = -1;
                    StopRecv(obj);
                }
            }
            else if(series > 60)
            {
                MYPRINT("client_recv_run: series=%d \n", series);
                UserData udata;
                udata.data1.head.datatype = kCMD;
                int size = sizeof(CmdData);
                udata.data1.head.size = size;
                udata.data1.cmdtype = (CMDType)kAsk;
                int asktimes = 0;
                int max_asktimes = 3;//连问3遍
                do{
                    int ret2 = client_broadcast_get(obj, &udata);
                    if(ret2 < 0)
                    {
                    }
                    else{
                        break;
                    }
                    asktimes++;
                }while(asktimes < max_asktimes);
                MYPRINT("client_recv_run: asktimes=%d \n", asktimes);
                if(asktimes >= max_asktimes)
                {
                    StopRecv(obj);
                    MYPRINT("client_recv_run: server closed \n");
                }
                else{
                    series = 0;
                }
            }
            free(recv_buf);
        }
        else{
            series = 0;
            recvsize += recv_num;
#ifdef LOOPRELAY
            UserData *udata = (UserData *)recv_buf;
            int datatype = udata->data0.head.datatype;
            if(datatype)
            {
                PushData(obj, recv_buf, recv_num, addr_serv);
            }
            else{
                //status = ClientProcessCmd(obj, recv_buf);
                MYPRINT("client_recv_run: datatype=%d \n", datatype);
            }
#else
            PushData(obj, recv_buf, recv_num, addr_serv);
            //MYPRINT("client_recv_run: recv_num=%d \n", recv_num);
#endif
        }

        int difftime = (int)(now_time - time0);
        if(difftime > MAX_INTERVAL)
        {
            float bitrate = (float)(((recvsize << 3) * 1.0) / (difftime * 1000));
            //MYPRINT("client_recv_run: recvsize=%lld \n", recvsize);
            time0 = now_time;
            recvsize = 0;
            MYPRINT("client_recv_run: bitrate=%5.2f (Mbps) \n", bitrate);
        }

        pthread_mutex_lock(&obj->status_lock);
        status = obj->recv_status;// & obj->send_status;
        pthread_mutex_unlock(&obj->status_lock);
    }
    ExitRecv(obj);
    MYPRINT("client_recv_run: over \n");
    char *p = malloc(32);
    strcpy(p,"client_recv_run exit");
    pthread_exit((void*)p);

    return ret;
}

void * client_main(SocketObj *obj)
{
    MYPRINT("client_main: 0 \n");
    int ret = 0;
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
        perror("socket");
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
		perror("client_main error:");
		return 0;
	}
	ret = setsockopt(obj->sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, len);
	if (ret == -1) {
	    perror("client_main error:");
	    return 0;
	}
    /* 设置address */
    //struct sockaddr_in addr_serv;
    memset(&obj->addr_serv, 0, sizeof(obj->addr_serv));
    obj->addr_serv.sin_family = AF_INET;
    obj->addr_serv.sin_addr.s_addr = inet_addr(obj->server_ip);
    obj->addr_serv.sin_port = htons(obj->port);
    //obj->recv_status = 1;
    //obj->send_status = 1;
    MYPRINT("client_main: 1 \n");
    pthread_mutex_init(&obj->lock,NULL);
    pthread_mutex_init(&obj->status_lock,NULL);
    //
    pthread_t tid;
    if(pthread_create(&tid, NULL, client_recv_run, obj) < 0)
    {
        MYPRINT("client_main: Create client_recv_run failed!\n");
        //exit(0);
    }

    client_send_run(obj);
    MYPRINT("client_main: client exit: taskId=%d \n", taskId);
    //StopRecv(obj);

    char *p0;
    if (pthread_join(tid, (void**)&p0))
    {
        MYPRINT("client_main: client_recv_run thread is not exit...\n");
    }
    else{
        MYPRINT("client_main: p0=%s \n", p0);
        free(p0);
    }

#if defined (WIN32)
    closesocket(obj->sock_fd);
#elif defined(__linux__)
    close(obj->sock_fd);
#endif
    pthread_mutex_destroy(&obj->lock);
    pthread_mutex_destroy(&obj->status_lock);
    MYPRINT("client_main: over \n");
    return 0;
}
int client_broadcast_cmd(SocketObj *obj, UserData *udata)
{
    int ret = 0;
    SOCKFD sock_fd = obj->sock_fd;
    struct sockaddr_in addr_serv = obj->addr_serv;
    int len = sizeof(addr_serv);
    char *send_buf = (char *)udata;
    //
    int64_t now_time = get_sys_time();
    int size = udata->data1.head.size;//sizeof(CmdData);
    ret = sendto(sock_fd, send_buf, size, 0, (struct sockaddr *)&addr_serv, len);//strlen(send_buf)
    if(ret <= 0)
    {
        if(errno != EAGAIN && !ret)
        {
            perror("client_broadcast_cmd: sendto error:");
            MYPRINT("client_broadcast_cmd: errno=%d \n", errno);
            ret = -1;
        }
    }
    obj->last_send_time = now_time;
    return ret;
}
//broadcast or normal client
int client_broadcast_get(SocketObj *obj, UserData *udata0)
{
    int ret = -(1 << 30);
    SOCKFD sock_fd = obj->sock_fd;
    struct sockaddr_in addr_serv = obj->addr_serv;
    int len = sizeof(addr_serv);
    int recv_num;
    char recv_buf[sizeof(UserData)] = "";//"hey, who are you?";
    int data_len = sizeof(UserData);
    int isok = 0;
    int times = 0;
    int net_times = 0;
    int net_n = 3;
    int sumdelay = 0;
    int sumrtt = 0;
    int send_cmd = 1;
    int cmd0 = udata0->data1.cmdtype;
    do{

        int ret2 = 1;
        if(send_cmd)
        {
            udata0->data1.st0 = get_sys_time();
            ret2 = client_broadcast_cmd(obj, udata0);
        }

        if(ret2 > 0)
        {
            recv_num = recvfrom(sock_fd, recv_buf, data_len, 0, (struct sockaddr *)&addr_serv, (socklen_t *)&len);
            if(recv_num <= 0)
            {
                if(errno != EAGAIN && !recv_num)
                {
                    perror("client_broadcast_get: recvfrom error:");
                    MYPRINT("client_broadcast_get: errno=%d \n", errno);
                    if(!recv_num)
                    {
                        ret = -(1 << 30);
                        break;
                    }
                }
                times++;
                send_cmd = 1;
            }
            UserData *udata = (UserData *)recv_buf;
            int datatype = udata->data1.head.datatype;
            int cmd = udata->data1.cmdtype;
            if(udata->data1.head.datatype == kCMD && cmd == cmd0)
            {
                send_cmd = 1;
                switch(cmd)
                {
                    case kExit:
                    case kBye:
                        obj->status = 0;
                        isok = 1;
                        ret = 1;
                        break;
                    case kBroadCastReg:
                        obj->status = 1;
                        isok = 1;
                        ret = 1;
                        break;
                    case kAsk:
                        isok = 1;
                        ret = 1;
                        MYPRINT("client_broadcast_get: kAsk: ret=%d \n", ret);
                        break;
                    case kReg:
                        obj->status = 1;
                        isok = 1;
                        ret = 1;
                        break;
                    case kGetSessionId:
                        //obj->status = 1;
                        ret = udata->data1.sessionId;
                        udata0->data1.sessionId = ret;
                        if(ret >= 0)
                        {
                            isok = 1;
                        }
                        else{
                            MYPRINT("client_broadcast_get: kGetSessionId: ret=%d \n", ret);
                        }
                        break;
                    case kGetChanId:
                        ret = udata->data1.chanId;
                        //MYPRINT("client_broadcast_get: ret=%d \n", ret);
                        udata0->data1.chanId = ret;
                        if(ret != -1)
                        {
                            isok = 1;
                        }
                        else{
                            MYPRINT("client_broadcast_get: kGetChanId: ret=%d \n", ret);
                        }
                        break;

                    case kRTT:
                    {
                        int64_t st0 = udata->data1.st0;
                        int64_t rt0 = udata->data1.rt0;
                        int64_t st1 = udata->data1.st1;
                        int64_t rt1 = get_sys_time();
                        int rtt = (int)(((rt1 - st0) - (st1 - rt0)) / 2);//  # ((rt0 - st0) + (rt1 - st1)) / 2 #
                        sumrtt += rtt;
                        //# delay0 = (st0 + rtt) - rt0
                        //# delay1 = (rt1 - rtt) - st1
                        //# delay = (delay0 + delay1) / 2
                        //st0 + rtt_a == rt0; rt1 - rtt_b == st1; rtt_a [=] rtt_b;
                        int delay = (int)(((rt1 + st0) - (st1 + rt0)) / 2);
                        sumdelay += delay;
                        net_times++;
                        if(net_times >= net_n)
                        {
                            ret = (sumdelay / net_n);//ms
                            isok = 1;
                        }
                        break;
                    }
                    case kHeartBeat:
                        ret = 1;
                        isok = 1;
                        break;
                    default:
                        MYPRINT("warning: client_broadcast_get: cmd=%d \n", cmd);
                        break;
                }
            }
            else if(datatype == kDATA)
            {
                MYPRINT("warning: client_broadcast_get: datatype=%d, cmd0=%d \n", datatype, cmd0);
                send_cmd = 0;
                //如何防止命令淹没在数据中？
                if(cmd0 == kAsk || cmd0 == kHeartBeat)
                {
                    ret = 1;
                    isok = 1;
                    break;
                }
            }
            else{
                //usleep(100000);//100ms
                MYPRINT("fail: client_broadcast_get: datatype=%d, cmd=%d, cmd0=%d \n", datatype, cmd, cmd0);
                if(cmd0 == kAsk || cmd0 == kHeartBeat)
                {
                    ret = 1;
                    isok = 1;
                    break;
                }
                send_cmd = 0;
                break;
            }
        }
        else{
            break;
        }
        if(times > 10)
        {
            break;
        }
    }while(!isok);
    MYPRINT("client_broadcast_get: times=%d \n", times);
    return ret;
}
int client_broadcast_run(SocketObj *obj)
{
    int ret = 0;
    pthread_mutex_lock(&obj->status_lock);
    obj->recv_status = 1;
    int status = obj->recv_status;
    pthread_mutex_unlock(&obj->status_lock);
    //ClearPause(obj);
    //SetPause(obj);

    SOCKFD sock_fd = obj->sock_fd;
    struct sockaddr_in addr_serv = obj->addr_serv;
    int len = sizeof(addr_serv);
    int recv_num;
    char *recv_buf = "";//[sizeof(UserData)] = "";//"hey, who are you?";
    int data_len = sizeof(UserData);
    //
    int datatype = kDATA;

    //obj->start_time = get_sys_time();
    int64_t time0 = get_sys_time();
    int64_t recvsize = 0;
    int series = 0;
    while(status)
    {
        int64_t now_time = get_sys_time();
        //wait pause
        WaitPause(obj);
        //防止同时执行recvfrom
        recv_buf = (char *)malloc(data_len);
        recv_num = recvfrom(sock_fd, recv_buf, data_len, 0, (struct sockaddr *)&addr_serv, (socklen_t *)&len);
        if(recv_num <= 0)
        {
            series++;
            if(errno != EAGAIN && !recv_num)
            {
                //如果出现EINTR即errno为4，错误描述Interrupted system call，操作也应该继续。
                //最后，如果recv的返回值为0，那表明连接已经断开，我们的接收操作也应该结束。
                perror("client_broadcast_run: recvfrom error:");
                MYPRINT("client_broadcast_run: errno=%d \n", errno);
                if(!recv_num)
                {
                    ret = -1;
                    StopRecv(obj);
                }

            }
            else if(series > 10)
            {
                //10 * 500 = 5s没有收到服务器消息，则发送问询，判断是否失联
                MYPRINT("client_broadcast_run: series=%d \n", series);
                UserData udata;
                udata.data1.head.datatype = kCMD;
                int size = sizeof(CmdData);
                udata.data1.head.size = size;
                udata.data1.cmdtype = (CMDType)kAsk;
                int asktimes = 0;
                int max_asktimes = 3;//连问3遍
                do{
                    int ret2 = client_broadcast_get(obj, &udata);
                    if(ret2 < 0)
                    {
                    }
                    else{
                        break;
                    }
                    asktimes++;
                }while(asktimes < max_asktimes);
                MYPRINT("client_broadcast_run: asktimes=%d \n", asktimes);
                if(asktimes >= max_asktimes)
                {
                    StopRecv(obj);
                    MYPRINT("client_broadcast_run: server closed ######################################### \n");
                }
                else{
                    series = 0;
                }
            }
            free(recv_buf);
        }
        else{
            series = 0;
            recvsize += recv_num;
            UserData *udata = (UserData *)recv_buf;
            if(udata->data1.head.datatype == kCMD)
            {
                int cmd = udata->data1.cmdtype;
                if(cmd == kBroadCast)
                {
                    MYPRINT("client_broadcast_run: cmd=%d \n", cmd);
                    MYPRINT("client_broadcast_run: recv_num=%d \n", recv_num);
                    int status = udata->data1.status;
                    int chanId = udata->data1.chanId;
                    int avtype = udata->data1.avtype;
                    int selfmode = udata->data1.selfmode;
                    int testmode = udata->data1.testmode;
                    int modeId = udata->data1.modeId;
                    int actor = udata->data1.actor;
                    int sessionId = udata->data1.sessionId;
                    MYPRINT("client_broadcast_run: status=%d, chanId=%d, sessionId=%d \n", status, chanId, sessionId);
                    MYPRINT("client_broadcast_run: avtype=%d, selfmode=%d, testmode=%d \n", avtype, selfmode, testmode);
                    MYPRINT("client_broadcast_run: modeId=%d, actor=%d, cmd=%d \n", modeId, actor, cmd);
                    PushData(obj, recv_buf, recv_num, addr_serv);
                }

            }
            else{
                free(recv_buf);
            }
        }

        //heart beat
        int interval = HEARTBEAT_TIME;//300000;//30s
        int ret2 = HBLoop(obj, now_time, interval);
        if(ret2 == -2)
        {
            StopRecv(obj);
        }
        //bit rate
        int difftime = (int)(now_time - time0);
        if(difftime > MAX_INTERVAL)
        {
            float bitrate = (float)(((recvsize << 3) * 1.0) / (difftime * 1000));
            //MYPRINT("server_main: recvsize=%lld \n", recvsize);
            time0 = now_time;
            recvsize = 0;
            if(bitrate > 0)
                MYPRINT("client_broadcast_run: bitrate=%5.2f (Mbps) \n", bitrate);
        }

        pthread_mutex_lock(&obj->status_lock);
        status = obj->recv_status;
        pthread_mutex_unlock(&obj->status_lock);
    }
    ExitRecv(obj);

    //char *p = malloc(32);
    //strcpy(p,"client_broadcast_run over");
    //pthread_exit((void*)p);

    return ret;
}
void * client_broadcast(SocketObj *obj)
{
    int ret = 0;
    cJSON * json = (cJSON *)api_str2json(obj->params);
    obj->port = GetvalueInt(json, "port");
    GetvalueStr(json, "server_ip", obj->server_ip);
    MYPRINT("client_broadcast: obj->port=%d \n", obj->port);
    MYPRINT("client_broadcast: obj->server_ip=%s \n", obj->server_ip);
    int taskId = obj->id;
    /* socket文件描述符 */
    SOCKFD sock_fd;
    /* 建立udp socket */
    obj->sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(obj->sock_fd < 0)
    {
        perror("socket");
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
		perror("client_broadcast error:");
		return 0;
	}
	ret = setsockopt(obj->sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, len);
	if (ret == -1) {
	    perror("client_broadcast error:");
	    return 0;
	}
    /* 设置address */
    //struct sockaddr_in addr_serv;
    memset(&obj->addr_serv, 0, sizeof(obj->addr_serv));
    obj->addr_serv.sin_family = AF_INET;
    //MYPRINT("client_broadcast: obj->server_ip=%s \n", obj->server_ip);
    obj->addr_serv.sin_addr.s_addr = inet_addr(obj->server_ip);
    obj->addr_serv.sin_port = htons(obj->port);
    //obj->recv_status = 1;
    //obj->send_status = 1;
    pthread_mutex_init(&obj->lock,NULL);
    pthread_mutex_init(&obj->status_lock,NULL);
    //
    sleep(1);
    UserData udata;
    udata.data1.head.datatype = kCMD;
    int size = sizeof(CmdData);
    udata.data1.head.size = size;
    udata.data1.cmdtype = (CMDType)kBroadCastReg;//kReg;//kExit;kGetSessionId,kGetChanId,kBye,kHeartBeat,kRTT,kBroadCast,kExit,
    udata.data1.status = 1;
    udata.data1.chanId = -1;//obj.id;
    udata.data1.avtype = 0;
    udata.data1.selfmode = 0;
    udata.data1.testmode = 0;
    udata.data1.modeId = 0;
    udata.data1.actor = DECACTOR + 1;
    udata.data1.sessionId = -1;
    ret = client_broadcast_get(obj, &udata);
    if(ret < 0)
    {
        MYPRINT("error: client_broadcast register fail: ret=%d \n", ret);
    }
    else{
#if 0
        MYPRINT("client_broadcast: kBroadCastReg: ret=%d \n", ret);
        udata.data1.st0 = get_sys_time();
        udata.data1.cmdtype = (CMDType)kRTT;
        ret = client_broadcast_get(obj, &udata);
        MYPRINT("client_broadcast: kRTT: ret=%d \n", ret);
        udata.data1.cmdtype = (CMDType)kGetSessionId;
        ret = client_broadcast_get(obj, &udata);
        MYPRINT("client_broadcast: kGetSessionId: ret=%d \n", ret);
        udata.data1.chanId = -1;
        udata.data1.randId = api_create_id((1 << 30));
        udata.data1.cmdtype = (CMDType)kGetChanId;
        ret = client_broadcast_get(obj, &udata);
        MYPRINT("client_broadcast: kGetChanId: ret=%d \n", ret);

        udata.data1.chanId = -1;
        udata.data1.randId = api_create_id((1 << 30));
        udata.data1.cmdtype = (CMDType)kGetChanId;
        ret = client_broadcast_get(obj, &udata);
        MYPRINT("client_broadcast: kGetChanId: ret=%d \n", ret);

        udata.data1.cmdtype = (CMDType)kReg;
        udata.data1.actor = 1;
        ret = client_broadcast_get(obj, &udata);
        MYPRINT("client_broadcast: kReg: ret=%d \n", ret);
        udata.data1.cmdtype = (CMDType)kReg;
        udata.data1.actor = DECACTOR;
        ret = client_broadcast_get(obj, &udata);
        MYPRINT("client_broadcast: kReg: ret=%d \n", ret);
        udata.data1.cmdtype = (CMDType)kBye;
        ret = client_broadcast_get(obj, &udata);
        MYPRINT("client_broadcast: kBye: ret=%d \n", ret);

        //udata.data1.cmdtype = (CMDType)kExit;
        //ret = client_broadcast_get(obj, &udata);
        //MYPRINT("client_broadcast: kExit: ret=%d \n", ret);

#else
        client_broadcast_run(obj);
#endif
    }
    MYPRINT("client_broadcast exit: taskId=%d \n", taskId);

#if defined (WIN32)
    closesocket(obj->sock_fd);
#elif defined(__linux__)
    close(obj->sock_fd);
#endif
    pthread_mutex_destroy(&obj->lock);
    pthread_mutex_destroy(&obj->status_lock);

    return 0;
}

//int pool_add_worker (void *(*process) (void *arg), void *arg);
void *thread_routine (void *arg);


//static CThread_pool *pool = NULL;
CThread_pool * pool_init (CThread_pool *pool, int max_thread_num)
{
    pool = (CThread_pool *) malloc (sizeof (CThread_pool));

    pthread_mutex_init (&(pool->queue_lock), NULL);
    pthread_cond_init (&(pool->queue_ready), NULL);

    pool->queue_head = NULL;

    pool->max_thread_num = max_thread_num;
    pool->cur_queue_size = 0;

    pool->shutdown = 0;

    pool->threadid = (pthread_t *) malloc (max_thread_num * sizeof (pthread_t));
    int i = 0;
    for (i = 0; i < max_thread_num; i++)
    {
        pthread_create (&(pool->threadid[i]), NULL, thread_routine, pool);
    }
    return pool;
}

/*向线程池中加入任务*/
int pool_add_worker (CThread_pool *pool, void *(*process) (void *arg), void *arg)
{
    //MYPRINT("pool_add_worker 0 \n");
    /*构造一个新任务*/
    CThread_worker *newworker = (CThread_worker *) malloc (sizeof (CThread_worker));
    newworker->process = process;
    newworker->arg = arg;
    newworker->next = NULL;/*别忘置空*/
    //MYPRINT("pool_add_worker 1 \n");

    pthread_mutex_lock (&(pool->queue_lock));
    /*将任务加入到等待队列中*/
    CThread_worker *member = pool->queue_head;
    if (member != NULL)
    {
        //MYPRINT("pool_add_worker 2 \n");
        while (member->next != NULL)
            member = member->next;
        member->next = newworker;
    }
    else
    {
        //MYPRINT("pool_add_worker 3 \n");
        pool->queue_head = newworker;
    }
    MYPRINT("pool_add_worker: pool->queue_head=%x \n", pool->queue_head);
    assert (pool->queue_head != NULL);

    pool->cur_queue_size++;
    MYPRINT("pool_add_worker: pool->cur_queue_size=%d \n", pool->cur_queue_size);
    pthread_mutex_unlock (&(pool->queue_lock));
    /*好了，等待队列中有任务了，唤醒一个等待线程；
    注意如果所有线程都在忙碌，这句没有任何作用*/
    pthread_cond_signal (&(pool->queue_ready));
    //MYPRINT("pool_add_worker over \n");
    return 0;
}

/*销毁线程池，等待队列中的任务不会再被执行，但是正在运行的线程会一直
把任务运行完后再退出*/
int pool_destroy (CThread_pool *pool)
{
    if (pool->shutdown)
        return -1;/*防止两次调用*/
    pool->shutdown = 1;
    MYPRINT("pool_destroy: pool->max_thread_num=%d \n", pool->max_thread_num);
    /*唤醒所有等待线程，线程池要销毁了*/
    pthread_cond_broadcast (&(pool->queue_ready));

    /*阻塞等待线程退出，否则就成僵尸了*/
    int i;
    for (i = 0; i < pool->max_thread_num; i++)
    {
        if(pool->threadid[i])
        {
            MYPRINT("pool_destroy: i=%d \n", i);
            pthread_join (pool->threadid[i], NULL);
        }
    }
    MYPRINT("pool_destroy: 1 \n");
    free (pool->threadid);

    /*销毁等待队列*/
    CThread_worker *head = NULL;
    while (pool->queue_head != NULL)
    {
        head = pool->queue_head;
        pool->queue_head = pool->queue_head->next;
        free (head);
    }
    /*条件变量和互斥量也别忘了销毁*/
    pthread_mutex_destroy(&(pool->queue_lock));
    pthread_cond_destroy(&(pool->queue_ready));

    free (pool);
    /*销毁后指针置空是个好习惯*/
    pool=NULL;
    return 0;
}
//停止线程池中的某一任务
int pool_delete(CThread_pool *pool, int id)
{
    MYPRINT("pool_delete 0 \n");
#if 0
    if(!pool->threadid[id])
    {
        return -1;
    }
    pthread_cond_broadcast (&(pool->queue_ready));
    pthread_join (pool->threadid[id], NULL);
    pool->threadid[id] = 0;
    MYPRINT("pool_delete 1 \n");
#endif
    pthread_mutex_lock (&(pool->queue_lock));
    CThread_worker *head = pool->queue_head;
    CThread_worker *p = head;
    CThread_worker *last = head;
    CThread_worker *q = NULL;
    MYPRINT("pool_delete: p=%x \n", p);
    do
    {
        SocketObj *obj = (SocketObj *)p->arg;
        MYPRINT("pool_delete: obj=%x \n", obj);
        MYPRINT("pool_delete: obj->id=%d \n", obj->id);
        if(obj->id == id)
        {
            q = p;
            if(last == head)
            {
                pool->queue_head = p->next;
                last->next = p->next;
                MYPRINT("pool_delete 2 \n");
            }
            else{
                last->next = p->next;
                MYPRINT("pool_delete 3 \n");
            }
            free(q);
            pool->cur_queue_size--;
            break;
        }
        //find task
        last = p;
        if(!p->next)
        {
            break;
        }
        p = p->next;
    }while(p);
    pthread_mutex_unlock (&(pool->queue_lock));
    MYPRINT("pool_delete over \n");
}
void * thread_routine (void *arg)
{
    //MYPRINT ("starting thread 0x%x\n", pthread_self ());
    CThread_pool *pool = (CThread_pool *)arg;
    while (1)
    {
        pthread_mutex_lock (&(pool->queue_lock));
        /*如果等待队列为0并且不销毁线程池，则处于阻塞状态; 注意
        pthread_cond_wait是一个原子操作，等待前会解锁，唤醒后会加锁*/
        while (pool->cur_queue_size == 0 && !pool->shutdown)
        {
            //MYPRINT ("thread_routine: thread 0x%x is waiting\n", pthread_self ());
            pthread_cond_wait (&(pool->queue_ready), &(pool->queue_lock));
        }

        /*线程池要销毁了*/
        if (pool->shutdown)
        {
            /*遇到break,continue,return等跳转语句，千万不要忘记先解锁*/
            pthread_mutex_unlock (&(pool->queue_lock));
            //MYPRINT ("thread_routine: thread 0x%x will exit\n", pthread_self ());
            pthread_exit (NULL);
        }

        //MYPRINT ("thread_routine: thread 0x%x is starting to work\n", pthread_self ());

        /*assert是调试的好帮手*/
        assert (pool->cur_queue_size != 0);
        assert (pool->queue_head != NULL);

        /*等待队列长度减去1，并取出链表中的头元素*/
        pool->cur_queue_size--;
        MYPRINT("thread_routine: pool->cur_queue_size=%d \n", pool->cur_queue_size);
        CThread_worker *worker = pool->queue_head;
        pool->queue_head = worker->next;
        pthread_mutex_unlock (&(pool->queue_lock));

        /*调用回调函数，执行任务*/
        (*(worker->process)) (worker->arg);
        free (worker);
        worker = NULL;
    }
    /*这一句应该是不可达的*/
    pthread_exit (NULL);
}

//    下面是测试代码

void * myprocess (void *arg)
{
    //int taskId = *(int *) arg;
    //MYPRINT ("threadid is 0x%x, working on task %d\n", pthread_self (),*(int *) arg);
    SocketObj *obj = (SocketObj *)arg;//calloc(1, sizeof(SocketObj));
    //obj->id = taskId;
    int taskId = obj->id;
    if(!taskId)
    {
        obj->type = 0;
        server_main(obj);
    }
    else if(taskId == -1)
    {
        obj->type = 2;
        client_broadcast(obj);
    }
    else{
        obj->type = 1;
        client_main(obj);

    }
    //sleep (1);/*休息一秒，延长任务的执行时间*/
    //MYPRINT("myprocess: obj=%x \n", obj);
    //MYPRINT("myprocess: obj->id=%d \n", obj->id);
    return NULL;//(void *)obj;
}

int AddTask(TaskPool *task, char *params, void *thread_fun, int taskId)
{
    int ret = 0;
    SocketObj *obj = calloc(1, sizeof(SocketObj));
    //obj->params = params;
    if(strlen(params) > 2048)
    {
        MYPRINT("error: AddTask:  strlen(params)=%d \n", strlen(params));
        return -1;
    }
    strcpy(obj->params, params);
    obj->id = taskId;
    //obj->init = 0;
    task->hnd[taskId] = obj;
    pool_add_worker(task->pool, thread_fun, obj);
    return ret;
}
int InitTask(TaskPool *task)
{
    int ret = 0;
    MYPRINT("InitTask 0 \n");
    //if(!GlobSessionList)
    //{
    //    GlobSessionList = (SessionInfo *)calloc(1, glob_session_num * sizeof(SessionInfo));
    //}
    task->pool = pool_init (task->pool, task->aliveTaskNum);/*线程池中最多三个活动线程*/
    /*连续向池中投入10个任务*/
    task->hnd = (SocketObj **) calloc(1, sizeof(SocketObj *) * task->taskNum);
    MYPRINT("InitTask 1 \n");
    int taskId = 0;
    SocketObj *obj = NULL;
    if(task->thread_fun)
    {
        for (int i = 0; i < task->taskNum; i++)
        {
#if 0
            obj = calloc(1, sizeof(SocketObj));
            obj->id = taskId;
            obj->init = 0;
            taskId++;
            task->hnd[i] = obj;
            //if(!i)
            if(false)
            {
                obj->type = 2;
                pool_add_worker(task->pool, client_broadcast, obj);
            }
            else{
                MYPRINT("InitTask 2 \n");
                obj->type = 1;
                pool_add_worker(task->pool, client_main, obj);
            }
#else
            if(task->thread_fun)
            {
                //pool_add_worker(task->pool, task->thread_fun, obj);
                char params[1024] = "";
                AddTask(task, params, task->thread_fun, taskId);
                taskId++;
            }
            else{
                MYPRINT("error: InitTask: task->thread_fun == 0 \n");
            }
#endif
        }
        MYPRINT("InitTask: wait... \n");
        /*等待所有任务完成*/
        sleep (1);
        for (int i = 0; i < task->taskNum; i++)
        {
            if(task->hnd[i])
            {
                MYPRINT("InitTask: task->hnd[i]->id=%d \n", task->hnd[i]->id);
                MYPRINT("InitTask: task->hnd[i]->send_status=%d \n", task->hnd[i]->send_status);
                MYPRINT("InitTask: task->hnd[i]->recv_status=%d \n", task->hnd[i]->recv_status);
            }
        }
    }

    return ret;
}

int CloseTask(TaskPool *task)
{
    int ret = 0;
    MYPRINT("CloseTask: pool_destroy:task->taskNum=%d \n", task->taskNum);
    /*销毁线程池*/
    pool_destroy (task->pool);
    //
    for (int i = 0; i < task->taskNum; i++)
    {
        SocketObj *obj = (SocketObj *)task->hnd[i];
        if(obj)
        {
            free(obj);
        }
    }
    free(task->hnd);
    //if(GlobSessionList)
    //{
    //    free(GlobSessionList);
    //    GlobSessionList = NULL;
    //}
    MYPRINT("CloseTask: over \n");
    return ret;
}


HCSVC_API
int pool_main()
{
    CThread_pool *pool = NULL;
    int aliveTaskNum = 10;//100;
    int taskNum = 2;//100;//3;//2;//3;//10;
    //
    //if(!GlobSessionList)
    //{
    //    GlobSessionList = (SessionInfo *)calloc(1, glob_session_num * sizeof(SessionInfo));
    //}
    if((taskNum - 1) > 0)
    {
        glob_paced_bitrate = glob_paced_bitrate / (taskNum - 1);
    }

    //
    pool = pool_init (pool, aliveTaskNum);/*线程池中最多三个活动线程*/

    /*连续向池中投入10个任务*/
    int *workingnum = (int *) malloc (sizeof(int) * taskNum);
    int i;
    SocketObj **hnd = (SocketObj **) calloc(1, sizeof(SocketObj *) * taskNum);
    //uint64_t *obj = (uint64_t *) calloc(1, sizeof(uint64_t) * taskNum);
    int taskId = 0;
    SocketObj *obj = NULL;
    for (i = 0; i < taskNum; i++)
    {
        //workingnum[i] = i;
        //if(!(i & 1))
        if(true)
        {
            obj = calloc(1, sizeof(SocketObj));
            obj->id = taskId;
            obj->init = 0;
            taskId++;
        }
        else{
            obj->init = 1;
        }
        hnd[i] = obj;
        //pool_add_worker (myprocess, &workingnum[i]);
        pool_add_worker(pool, myprocess, obj);
        //pool_add_worker(pool, server_main, obj);
        //MYPRINT("pool_main: i=%d, hnd[i]=%x \n", i, hnd[i]);
    }
    MYPRINT("pool_main: wait... \n");
    /*等待所有任务完成*/
    sleep (1);
    for (i = 0; i < taskNum; i++)
    {
        if(hnd[i])
        {
            MYPRINT("pool_main: hnd[i]->id=%d \n", hnd[i]->id);
            MYPRINT("pool_main: hnd[i]->send_status=%d \n", hnd[i]->send_status);
            MYPRINT("pool_main: hnd[i]->recv_status=%d \n", hnd[i]->recv_status);
        }
    }
    //
#if 0
    glob_paced_bitrate = glob_paced_bitrate / 1;
    char handle[8] = "";
    char params[1024] = "";
    api_taskpool_init(handle, params);

    api_taskpool_close(handle);
#endif
    //pool_delete(pool, 0);//!!!
    //
    MYPRINT("pool_main: pool_destroy \n");
    /*销毁线程池*/
    pool_destroy (pool);

    free (workingnum);
    MYPRINT("pool_main: over \n");
    return 0;
}
#if 1
sem_t can_add;//能够进行加法计算的信号量
sem_t can_mul;//能够进行输入的信号量
sem_t can_scanf;//能够进行乘法计算的信号量
int x,y;
void *thread_add(void *arg)//加法线程入口函数
{
    while(1)
    {
        sem_wait(&can_add);
        MYPRINT("%d+%d=%d\n",x,y,x+y);
        sem_post(&can_mul);
    }
}
void *thread_mul(void *arg)//乘法线程入口函数
{
    while(1)
    {
        sem_wait(&can_mul);
        MYPRINT("%d*%d=%d\n",x,y,x*y);
        sem_post(&can_scanf);
    }
}
int thread_main()
{
    pthread_t tid;
    int arg[2];
    //信号量初始化
    sem_init(&can_scanf,0,1);
    sem_init(&can_add,0,0);
    sem_init(&can_mul,0,0);
    if(pthread_create(&tid,NULL,thread_add,NULL)<0)
    {
        MYPRINT("Create thread_add failed!\n");
        exit(0);
    }
    if(pthread_create(&tid,NULL,thread_mul,NULL)<0)
    {
        MYPRINT("Create thread_mul failed!\n");
        exit(0);
    }
    while(1)
    {
        sem_wait(&can_scanf);//等待信号量置位并进行减一操作
        MYPRINT("Please input two integers:");
        scanf("%d%d",&x,&y);
        sem_post(&can_add);//信号量加一 操作
    }
}
#endif
//==================================================================================================
#if 0
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include "threadpool.h"
#define DEFAULT_TIME 10 // 领导定时检查队列、线程状态的时间间隔
#define MIN_WAIT_TASK_NUM 10 // 队列中等待的任务数>这个值，便会增加线程
#define DEFAULT_THREAD_VARY 10 //每次线程加减的数目
typedef struct
{
void *(*function)(void *);
void *arg;
} threadpool_task_t;
struct threadpool_t
{
pthread_mutex_t lock;// mutex for the taskpool
pthread_mutex_t thread_counter;//mutex for count the busy thread
pthread_cond_t queue_not_full;
pthread_cond_t queue_not_empty;//任务队列非空的信号
pthread_t *threads;//执行任务的线程
pthread_t adjust_tid;//负责管理线程数目的线程
threadpool_task_t *task_queue;//任务队列
int min_thr_num;
int max_thr_num;
int live_thr_num;
int busy_thr_num;
int wait_exit_thr_num;
int queue_front;
int queue_rear;
int queue_size;
int queue_max_size;
bool shutdown;
};
/**
 * @function void *threadpool_thread(void *threadpool)
 * @desc the worker thread
 * @param threadpool the pool which own the thread
 */
void *threadpool_thread(void *threadpool);
/**
 * @function void *adjust_thread(void *threadpool);
 * @desc manager thread
 * @param threadpool the threadpool
 */
void *adjust_thread(void *threadpool);
/**
 * check a thread is alive
 */
bool is_thread_alive(pthread_t tid);
int threadpool_free(threadpool_t *pool);
//创建线程池
threadpool_t *threadpool_create(int min_thr_num, int max_thr_num, int queue_max_size)
{
threadpool_t *pool = NULL;
 do{
 if((pool = (threadpool_t *)malloc(sizeof(threadpool_t))) == NULL)
 {
 MYPRINT("malloc threadpool fail");
 break;
 }
 pool->min_thr_num = min_thr_num;
 pool->max_thr_num = max_thr_num;
 pool->busy_thr_num = 0;
 pool->live_thr_num = min_thr_num;
 pool->queue_size = 0;
 pool->queue_max_size = queue_max_size;
 pool->queue_front = 0;
 pool->queue_rear = 0;
 pool->shutdown = false;
 pool->threads = (pthread_t *)malloc(sizeof(pthread_t)*max_thr_num);
 if (pool->threads == NULL)
 {
MYPRINT("malloc threads fail");
break;
 }
 memset(pool->threads, 0, sizeof(pool->threads));
 pool->task_queue = (threadpool_task_t *)malloc(sizeof(threadpool_task_t)*queue_max_size);
 if (pool->task_queue == NULL)
 {
MYPRINT("malloc task_queue fail");
break;
 }
 if (pthread_mutex_init(&(pool->lock), NULL) != 0
|| pthread_mutex_init(&(pool->thread_counter), NULL) != 0
|| pthread_cond_init(&(pool->queue_not_empty), NULL) != 0
|| pthread_cond_init(&(pool->queue_not_full), NULL) != 0)
 {
MYPRINT("init the lock or cond fail");
break;
 }
 /**
 * start work thread min_thr_num
 */
 for (int i = 0; i < min_thr_num; i++)
 {
//启动任务线程
pthread_create(&(pool->threads[i]), NULL, threadpool_thread, (void *)pool);
MYPRINT("start thread 0x%x...\n", pool->threads[i]);
 }
//启动管理线程
 pthread_create(&(pool->adjust_tid), NULL, adjust_thread, (void *)pool);
 return pool;
 }while(0);
 threadpool_free(pool);
return NULL;
}
//把任务添加到队列中
int threadpool_add(threadpool_t *pool, void*(*function)(void *arg), void *arg)
{
assert(pool != NULL);
assert(function != NULL);
assert(arg != NULL);
pthread_mutex_lock(&(pool->lock));
//队列满的时候，等待
while ((pool->queue_size == pool->queue_max_size) && (!pool->shutdown))
{
//queue full wait
pthread_cond_wait(&(pool->queue_not_full), &(pool->lock));
}
if (pool->shutdown)
{
pthread_mutex_unlock(&(pool->lock));
}
//如下是添加任务到队列，使用循环队列
if (pool->task_queue[pool->queue_rear].arg != NULL)
{
free(pool->task_queue[pool->queue_rear].arg);
pool->task_queue[pool->queue_rear].arg = NULL;
}
pool->task_queue[pool->queue_rear].function = function;
pool->task_queue[pool->queue_rear].arg = arg;
pool->queue_rear = (pool->queue_rear + 1)%pool->queue_max_size;
pool->queue_size++;
//每次加完任务，发个信号给线程
//若没有线程处于等待状态，此句则无效，但不影响
pthread_cond_signal(&(pool->queue_not_empty));
pthread_mutex_unlock(&(pool->lock));
return 0;
}
//线程执行任务
void *threadpool_thread(void *threadpool)
{
threadpool_t *pool = (threadpool_t *)threadpool;
threadpool_task_t task;
while(true)
{
/* Lock must be taken to wait on conditional variable */
pthread_mutex_lock(&(pool->lock));
//任务队列为空的时候，等待
while ((pool->queue_size == 0) && (!pool->shutdown))
{
MYPRINT("thread 0x%x is waiting\n", pthread_self());
pthread_cond_wait(&(pool->queue_not_empty), &(pool->lock));
//被唤醒后，判断是否是要退出的线程
if (pool->wait_exit_thr_num > 0)
{
pool->wait_exit_thr_num--;
if (pool->live_thr_num > pool->min_thr_num)
{
MYPRINT("thread 0x%x is exiting\n", pthread_self());
 pool->live_thr_num--;
 pthread_mutex_unlock(&(pool->lock));
 pthread_exit(NULL);
}
}
}
if (pool->shutdown)
{
pthread_mutex_unlock(&(pool->lock));
MYPRINT("thread 0x%x is exiting\n", pthread_self());
pthread_exit(NULL);
}
//get a task from queue
task.function = pool->task_queue[pool->queue_front].function;
task.arg = pool->task_queue[pool->queue_front].arg;
pool->queue_front = (pool->queue_front + 1)%pool->queue_max_size;
pool->queue_size--;
//now queue must be not full
pthread_cond_broadcast(&(pool->queue_not_full));
pthread_mutex_unlock(&(pool->lock));
// Get to work
MYPRINT("thread 0x%x start working\n", pthread_self());
pthread_mutex_lock(&(pool->thread_counter));
pool->busy_thr_num++;
pthread_mutex_unlock(&(pool->thread_counter));
(*(task.function))(task.arg);
// task run over
MYPRINT("thread 0x%x end working\n", pthread_self());
pthread_mutex_lock(&(pool->thread_counter));
pool->busy_thr_num--;
pthread_mutex_unlock(&(pool->thread_counter));
}
pthread_exit(NULL);
return (NULL);
}
//管理线程
void *adjust_thread(void *threadpool)
{
threadpool_t *pool = (threadpool_t *)threadpool;
while (!pool->shutdown)
{
sleep(DEFAULT_TIME);
pthread_mutex_lock(&(pool->lock));
int queue_size = pool->queue_size;
int live_thr_num = pool->live_thr_num;
pthread_mutex_unlock(&(pool->lock));
pthread_mutex_lock(&(pool->thread_counter));
int busy_thr_num = pool->busy_thr_num;
pthread_mutex_unlock(&(pool->thread_counter));
//任务多线程少，增加线程
if (queue_size >= MIN_WAIT_TASK_NUM
&& live_thr_num < pool->max_thr_num)
{
//need add thread
pthread_mutex_lock(&(pool->lock));
int add = 0;
for (int i = 0; i < pool->max_thr_num && add < DEFAULT_THREAD_VARY
&& pool->live_thr_num < pool->max_thr_num; i++)
{
if (pool->threads[i] == 0 || !is_thread_alive(pool->threads[i]))
{
pthread_create(&(pool->threads[i]), NULL, threadpool_thread, (void *)pool);
add++;
pool->live_thr_num++;
}
}
pthread_mutex_unlock(&(pool->lock));
}
//任务少线程多，减少线程
if ((busy_thr_num * 2) < live_thr_num
&& live_thr_num > pool->min_thr_num)
{
//need del thread
pthread_mutex_lock(&(pool->lock));
pool->wait_exit_thr_num = DEFAULT_THREAD_VARY;
pthread_mutex_unlock(&(pool->lock));
//wake up thread to exit
for (int i = 0; i < DEFAULT_THREAD_VARY; i++)
{
pthread_cond_signal(&(pool->queue_not_empty));
}
}
}
}
int threadpool_destroy(threadpool_t *pool)
{
if (pool == NULL)
{
return -1;
}
pool->shutdown = true;
//adjust_tid exit first
pthread_join(pool->adjust_tid, NULL);
// wake up the waiting thread
pthread_cond_broadcast(&(pool->queue_not_empty));
for (int i = 0; i < pool->min_thr_num; i++)
{
pthread_join(pool->threads[i], NULL);
}
threadpool_free(pool);
return 0;
}
int threadpool_free(threadpool_t *pool)
{
if (pool == NULL)
{
return -1;
}
if (pool->task_queue)
{
free(pool->task_queue);
}
if (pool->threads)
{
free(pool->threads);
pthread_mutex_lock(&(pool->lock));
pthread_mutex_destroy(&(pool->lock));
pthread_mutex_lock(&(pool->thread_counter));
pthread_mutex_destroy(&(pool->thread_counter));
pthread_cond_destroy(&(pool->queue_not_empty));
pthread_cond_destroy(&(pool->queue_not_full));
}
free(pool);
pool = NULL;
return 0;
}
int threadpool_all_threadnum(threadpool_t *pool)
{
int all_threadnum = -1;
pthread_mutex_lock(&(pool->lock));
all_threadnum = pool->live_thr_num;
pthread_mutex_unlock(&(pool->lock));
return all_threadnum;
}
int threadpool_busy_threadnum(threadpool_t *pool)
{
int busy_threadnum = -1;
pthread_mutex_lock(&(pool->thread_counter));
busy_threadnum = pool->busy_thr_num;
pthread_mutex_unlock(&(pool->thread_counter));
return busy_threadnum;
}
bool is_thread_alive(pthread_t tid)
{
int kill_rc = pthread_kill(tid, 0);
if (kill_rc == ESRCH)
{
return false;
}
return true;
}
// for test
//void *process(void *arg)
//{
//MYPRINT("thread 0x%x working on task %d\n ",pthread_self(),*(int *)arg);
//sleep(1);
//MYPRINT("task %d is end\n",*(int *)arg);
//return NULL;
//}
//int main()
//{
//threadpool_t *thp = threadpool_create(3,100,12);
//MYPRINT("pool inited");
//
//int *num = (int *)malloc(sizeof(int)*20);
//for (int i=0;i<10;i++)
//{
//num[i]=i;
//MYPRINT("add task %d\n",i);
//threadpool_add(thp,process,(void*)&num[i]);
//}
//sleep(10);
//threadpool_destroy(thp);
//}
#endif
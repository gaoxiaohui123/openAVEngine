
#include "hcsvc.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
//
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <assert.h>
//
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
//

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

extern int64_t get_sys_time();

//network congestion
//极限客户发送5Gbps,服务接收11Gbps
//#define MTU_SIZE 1500
#define PACED_BITRATE (int64_t)(2.5 * 1024 * 1024 * 1024) //安全的极限值2~2.5Gbps
#define PACED_BITRATE (int64_t)(2.0 * 1024 * 1024 * 1024)
#define MAX_SESSION_NUM 1000
static int glob_session_num = 100;
static int glob_session_idx = 0;
static int64_t glob_paced_bitrate = PACED_BITRATE;

typedef enum{
    kReg,
    kGetSessionId,
    kGetDeviceId,
    kBye,
    kHeart,
    kRTT,
    kExit,
}CMDType;
typedef struct{
    short datatype : 1;
    short size : 12;// >>1
}DataHeader;
typedef struct{
    DataHeader head;
    int seqnum;
    char data[MTU_SIZE];
}AVData;
typedef struct{
    //0 byte
   DataHeader head;
    //1 byte
    char avtype : 1;
    char selfmode : 1;
    char testmode : 1;
    char status : 1;
    char cmdtype : 4;
    //2 byte
    char chanId;
    //3 byte
    char modeId;
    //4 byte
    short actor;
    //6 byte
    short sessionId;
    //totle 8 bytes
    int liveTime;
    int64_t st0;
    int64_t rt0;
    int64_t st1;
}CmdData;

typedef union
{
    AVData data0;
    CmdData data1;
}UserData;

struct DataInfo{
    struct sockaddr_in addr_client;
    uint8_t *data;
    int size;
    int num;
    int idx;//test
    struct DataInfo *tail;
    struct DataInfo *next;
}DataInfo;
typedef struct DataInfo DataNode;

typedef struct{
    int sessionId;
    int modeId;
    int selfmode;
    int width;
    int height;
    int testmode;
    int status;
    int nettime;
    int liveTime;
    int startTime;
    int actors[64];
    int chanIdList[64];
}SessionInfo;
static SessionInfo *GlobSessionList = NULL;//(SessionInfo *)calloc(1, glob_session_num * sizeof(SessionInfo));

struct sockInfo {
    void *sock;
    int id;
    int actor;
    //int sessionId;
    int chanId;
    //int modeId;
    int avtype;
    //int selfmode;
    //int width;
    //int height;
    //int testmode;
    int status;
    int nettime;
    int liveTime;
    int startTime;
    //int actors[64];
    //int chanIdList[64];
    struct sockInfo *next;
};
typedef struct sockInfo NODE;  //typedef为C语言的关键字，作用是为一种数据类型定义一个新名字。

typedef struct{
    int id;
    int status;
    int init;
    int recv_status;
    int send_status;
    int sock_fd;
    int type;
    int64_t start_time;
    int64_t pkt_send_num;
    int64_t pkt_recv_num;
    struct sockaddr_in addr_serv;
    pthread_mutex_t lock;
    pthread_mutex_t status_lock;
    void *data_list;
    struct sockInfo *sockHead;
}SocketObj;
//使用typedef目的一般有两个，一个是给变量一个易记且意义明确的新名字，
//另一个是简化一些比较复杂的类型声明。
struct sockInfo *create();   //创建链表
void insert(NODE *head,NODE *pnew,int i);   //插入链表
void pdelete(NODE *head,int i);   //删除列表
void display(NODE *head);   //输出链表
void Pfree(NODE *head);    //销毁链表


struct sockInfo *create() {
    NODE *head,*tail,*pnew;
    //int score;
    int id;
    head = (NODE *)malloc(sizeof(NODE));  //创建头节点。
    if (head == NULL) { //创建失败返回
        printf("创建失败！");
        return NULL;
    }
    head->next = NULL;  //头节点指针域置NULL
    tail = head;  // 开始时尾指针指向头节点
#if 0
    printf("输入学生成绩：");
    while (1) { //创建链表
        scanf("%d",&id);
        if (id<0) //成绩为负是退出循环
            break;
        pnew=(NODE *)malloc(sizeof(NODE));  //创建新节点
        if (pnew==NULL) { //创建失败返回
            printf("创建失败！");
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
void insert(NODE *head,NODE *pnew,int i) {
    NODE *p; //当前指针
    int j;

    p = head;
    for (j = 0; j < i && p != NULL; j++) //p指向要插入的第i个节点
        p = p->next;

    if(head->next == NULL)
    {
        printf("insert: 空节点 \n");
    }
    else if (p == NULL) { //节点i不存在
        printf("insert: 与插入的节点不存在！\n");
        return;
    }

    pnew->next = p->next;   //插入节点的指针域指向第i个节点的后继节点
    p->next = pnew;    //犟第i个节点的指针域指向插入的新节点
}
void addnode(NODE *head, NODE *pnew) {
    NODE *p; //当前指针
    p = head;
    while(p->next){
        p = p->next;
    }
    pnew->next = p->next;
    p->next = pnew;
}
void *findnode(NODE *head, struct sockaddr_in addr_client)
{
    void *ret = 0;
    NODE *p;
    p = head;
    do{
        p = p->next;
        NODE *info = (NODE *)p;
        if(p)
        {
            struct sockaddr_in *paddr = (struct sockaddr_in *)info->sock;
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
void deletenode(NODE *head, int id)
{
    NODE *p,*q;
    p = head;
    while(p->next){
        NODE *info = (NODE *)p->next;
        if(info->id == id)
        {
            break;
        }
        p = p->next;
    }
    if (p->next == NULL) { //表明链表中的节点不存在
        printf("deletenode: 不存在！\n");
        return;
    }
    q = p->next;  //q指向待删除的节点
    p->next = q->next;  //删除节点i，也可写成p->next=p->next->next
    free(q);   //释放节点i的内存单元
}
void pdelete(NODE *head,int i) {
    NODE *p,*q;
    int j;
    if (i == 0) //删除的是头指针，返回
        return;
    p = head;
    for (j = 1; j < i && p->next != NULL; j++)
        p = p->next;  //将p指向要删除的第i个节点的前驱节点
    if (p->next == NULL) { //表明链表中的节点不存在
        printf("pdelete: 不存在！\n");
        return;
    }
    q = p->next;  //q指向待删除的节点
    p->next = q->next;  //删除节点i，也可写成p->next=p->next->next
    free(q);   //释放节点i的内存单元
}
void display(NODE *head) {
    NODE *p;
    printf("display: id=");
    for (p = head->next; p != NULL; p = p->next)
        printf("%d ",p->id);
    printf("\n");
}
void pfree(NODE *head) {
    NODE *p,*q;

    p = head;
    while (p->next != NULL) { //每次删除头节点的后继节点
        q = p->next;
        p->next = q->next;
        free(q);
    }
    free(head);   //最后删除头节点
}
void Pfree(NODE *head) {
    NODE *p,*q;
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
    printf("输出创建的链表：");
    display(head);
    pnew=(NODE *)malloc(sizeof(NODE));
    if (pnew==NULL) {
        printf("创建失败！");
        return 0;
    }
    pnew->id = 88;
    pnew->sock = NULL;
    insert(head,pnew, 0);   //将新节点插入节点3的后面
    printf("插入后的链表：");
    display(head);
    pdelete(head,0);   //删除节点3
    printf("删除后的链表：");
    display(head);
    Pfree(head);
    return 0;
}

//#include <stdio.h>
//#include <semaphore.h>
//#include <pthread.h>
//#include <stdlib.h>
sem_t can_add;//能够进行加法计算的信号量
sem_t can_mul;//能够进行输入的信号量
sem_t can_scanf;//能够进行乘法计算的信号量
int x,y;
void *thread_add(void *arg)//加法线程入口函数
{
    while(1)
    {
        sem_wait(&can_add);
        printf("%d+%d=%d\n",x,y,x+y);
        sem_post(&can_mul);
    }
}
void *thread_mul(void *arg)//乘法线程入口函数
{
    while(1)
    {
        sem_wait(&can_mul);
        printf("%d*%d=%d\n",x,y,x*y);
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
        printf("Create thread_add failed!\n");
        exit(0);
    }
    if(pthread_create(&tid,NULL,thread_mul,NULL)<0)
    {
        printf("Create thread_mul failed!\n");
        exit(0);
    }
    while(1)
    {
        sem_wait(&can_scanf);//等待信号量置位并进行减一操作
        printf("Please input two integers:");
        scanf("%d%d",&x,&y);
        sem_post(&can_add);//信号量加一 操作
    }
}
//
#define SERV_PORT   8000
#define DEST_PORT 8000
#define DSET_IP_ADDRESS  "127.0.0.1"

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
void StopSend(SocketObj *obj)
{
    pthread_mutex_lock(&obj->status_lock);
    obj->send_status = 0;
    pthread_mutex_unlock(&obj->status_lock);
}
void StopRecv(SocketObj *obj)
{
    pthread_mutex_lock(&obj->status_lock);
    obj->recv_status = 0;
    pthread_mutex_unlock(&obj->status_lock);
}
void WaitSend(SocketObj *obj)
{
    int status;
    do{
        pthread_mutex_lock(&obj->status_lock);
        status = obj->send_status;
        pthread_mutex_unlock(&obj->status_lock);
        if(status)
        {
            usleep(100000);//100ms
        }
    }while(status);
}
void WaitRecv(SocketObj *obj)
{
    int status;
    do{
        pthread_mutex_lock(&obj->status_lock);
        status = obj->recv_status;
        pthread_mutex_unlock(&obj->status_lock);
        if(status)
        {
            usleep(100000);//100ms
        }
    }while(status);
}
void PacedSend(int64_t bitrate, int sendDataLen, int difftime, int64_t sumbytes)
{
    int usedtime = difftime * 1000;//us
    //int64_t bitrate = 1 * 1024 * 1024 * 1024;//bps
    int64_t sendbits = sendDataLen << 3;
    int64_t pretime = (sendbits * 1000000) / bitrate; //us
    int64_t sumbits = sumbytes << 3;
    int64_t sumpretime = (sumbits * 1000000) / bitrate;
    //printf("PacedSend: sumpretime=%lld \n", sumpretime);
    int tailtime = (int)(sumpretime - usedtime); // us
    //printf("PacedSend: tailtime=%d \n", tailtime);
    int waittime = (int)(tailtime + pretime); //us
    //printf("PacedSend: waittime=%d \n", waittime);
    //waittime = pretime #test
    if(waittime > 0)
    {
        //printf("PacedSend: waittime=%d (us) \n", waittime);
        usleep(waittime);
    }
}

void PushData(SocketObj *obj, char *recv_buf, int recv_num, struct sockaddr_in addr_client)
{
    pthread_mutex_lock(&obj->lock);
#if 1
    DataNode *head,*pnew;
    if(!obj->data_list)
    {
        //printf("PushData: create head: obj->data_list=%x \n", obj->data_list);
        head = (DataNode *)malloc(sizeof(DataNode));  //创建头节点。
        head->num = 0;
        head->next = NULL;  //头节点指针域置NULL
        head->tail = head;  // 开始时尾指针指向头节点
        obj->data_list = (void *)head;
    }
    head = (DataNode *)obj->data_list;
    pnew = (DataNode *)malloc(sizeof(DataNode));  //创建新节点
    pnew->addr_client = addr_client;
    pnew->data = (uint8_t *)recv_buf;//(uint8_t *)malloc(recv_num * sizeof(uint8_t));
    pnew->size = recv_num;
    pnew->idx = head->num;
    pnew->next = NULL;   //新节点指针域置NULL
    head->tail->next = pnew;  //新节点插入到表尾
    head->tail = pnew;   //为指针指向当前的尾节点
    head->num++;
#endif
    pthread_mutex_unlock(&obj->lock);
}
void * PopData(SocketObj *obj)
{
    void *ret = NULL;
    pthread_mutex_lock(&obj->lock);

    pthread_mutex_unlock(&obj->lock);
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
    obj->data_list = NULL;
    pthread_mutex_unlock(&obj->lock);
    return ret;
}
int ProcessCmd(SocketObj *obj, char *recv_buf, int recv_num, struct sockaddr_in addr_client)
{
    int ret = 1;
    struct sockInfo *pnew;
    //if(1)
    {
        CmdData *udata = (CmdData *)recv_buf;
        int size = udata->head.size;
        printf("ProcessCmd: size=%d \n", size);
        int cmdtype = udata->cmdtype;
        if(cmdtype == (CMDType)kReg)
        {
            printf("ProcessCmd: cmdtype=%d \n", cmdtype);
            printf("ProcessCmd: udata->chanId=%d \n", udata->chanId);
            printf("ProcessCmd: udata->sessionId=%d \n", udata->sessionId);
            struct sockaddr_in *paddr = (struct sockaddr_in *)calloc(1, sizeof(struct sockaddr_in));
            paddr[0] = addr_client;
            printf("ProcessCmd: paddr->sin_port=%d \n", paddr->sin_port);
            printf("ProcessCmd: paddr->sin_addr.s_addr=%d \n", paddr->sin_addr.s_addr);
            pnew = (NODE *)malloc(sizeof(NODE));
            if (pnew == NULL) {
                printf("创建失败！");
                return 0;
            }
            pnew->id = (udata->sessionId << 16) | udata->chanId;
            pnew->sock = (void *)paddr;
            addnode(obj->sockHead, pnew);   //将新节点插入节点3的后面
            printf("插入后的链表：");
            display(obj->sockHead);
        }
        else if(cmdtype == (CMDType)kGetSessionId)
        {
            SessionInfo *ssinfo = &GlobSessionList[glob_session_idx];
            if(udata->status == 1)
            {
                ssinfo->status = 1;
                ssinfo->sessionId = udata->sessionId;
                ssinfo->modeId = udata->modeId;
                ssinfo->selfmode = udata->selfmode;
                //ssinfo->width = udata->width;
                //ssinfo->height = udata->height;
                ssinfo->testmode = udata->testmode;
                //ssinfo->status;
                //ssinfo->nettime = udata->nettime;
                ssinfo->liveTime = udata->liveTime;
                ssinfo->startTime = get_sys_time();
                //ssinfo->actors[64];
                //ssinfo->chanIdList[64];
                glob_session_idx++;
                //push to ack
            }

        }
        else if(cmdtype == (CMDType)kBye)
        {
            printf("ProcessCmd: cmdtype=%d \n", cmdtype);
            int id = (udata->sessionId << 16) | udata->chanId;
            deletenode(obj->sockHead, id);
            printf("删除后的链表：");
            display(obj->sockHead);
        }
        else if(cmdtype == (CMDType)kExit)
        {
            ret = 0;
        }
    }
    return ret;
}
int RelayData(SocketObj *obj, char *send_buf, int send_num, struct sockaddr_in addr_client, int sock_fd, int len)
{
    int ret = 0;
    AVData *udata = (AVData *)send_buf;
    int size = udata->head.size;
    int seqnum = udata->seqnum;
    //size <<= 1;
    //printf("ProcessData: AVData: size=%d, seqnum=%d \n", size, seqnum);
    NODE *info = (NODE *)findnode(obj->sockHead, addr_client);
    if(info)
    {
        //printf("ProcessData: AVData: info->id=%d \n", info->id);
    }
    send_num = sendto(sock_fd, send_buf, send_num, 0, (struct sockaddr *)&addr_client, len);
    if(send_num < 0)// || send_num != q->size)
    {
        printf("RelayData: send_num=%d \n", send_num);
        perror("RelayData: sendto error:");
        ret = -2;
        StopSend(obj);
    }
    ret += send_num;
    return ret;
}
int server_recv_run(SocketObj *obj, int sock_fd, struct sockaddr_in addr_serv)
{
    int ret = 0;
    obj->recv_status = 1;
    int status = obj->recv_status;
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
    while(status)
    {
        int64_t now_time = get_sys_time();
        //printf("recv_run wait:\n");
        recv_buf = (char *)malloc(data_len);
        recv_num = recvfrom(sock_fd, recv_buf, data_len, 0, (struct sockaddr *)&addr_client, (socklen_t *)&len);
        if(recv_num < 0)
        {
            free(recv_buf);
            perror("server_recv_run: recvfrom error:");
            ret = -1;
            break;
        }
        recvsize += recv_num;
        obj->pkt_recv_num++;
        //printf("server_main: recv_num=%d \n", recv_num);
        //printf("server_main: addr_client.sin_port=%d \n", addr_client.sin_port);
        //printf("server_main: addr_client.sin_addr.s_addr=%d \n", addr_client.sin_addr.s_addr);

        PushData(obj, recv_buf, recv_num, addr_client);

        int difftime = (int)(now_time - time0);
        if(difftime > 2000)
        {
            int bitrate = (int)(((recvsize << 3) * 1) / (difftime * 1000));
            //printf("server_main: recvsize=%lld \n", recvsize);
            time0 = now_time;
            recvsize = 0;
            printf("server_recv_run: bitrate=%d (Mbps) \n", bitrate);
        }
        pthread_mutex_lock(&obj->status_lock);
        status = obj->recv_status;
        pthread_mutex_unlock(&obj->status_lock);
    }
    StopRecv(obj);
    WaitSend(obj);
    return ret;
}
int send_data(SocketObj *obj, DataNode *head, int sock_fd, int len)
{
    int ret = 0;
    struct sockaddr_in addr_client;
    if(head && head->num)
    {
        //printf("server_send_run: head->num=%d \n", head->num);
        do{
            DataNode *q;
            q = head->next;
            if(q == NULL || q == head)
            {
                break;
            }
            else{
                head->next = head->next->next;
            }
            //
            char *send_buf = (char *)q->data;
            int send_num = q->size;
            addr_client = q->addr_client;
#if 0
            //if(!q->data || !q->size || q->size > 1500 || q->idx >= (head->num - 1))
            if(q->idx >= (head->num - 1))
            {
                //printf("server_send_run: q->data=%x \n", q->data);
                //printf("server_send_run: q->size=%d \n", q->size);
                //printf("server_send_run: q->idx=%d \n", q->idx);
                printf("server_send_run: head->num=%d \n", head->num);
                printf("server_send_run: obj->pkt_send_num=%lld \n", obj->pkt_send_num);
                printf("server_send_run: obj->pkt_recv_num=%lld \n", obj->pkt_recv_num);
            }
#endif
            send_num = sendto(sock_fd, send_buf, send_num, 0, (struct sockaddr *)&addr_client, len);
            if(send_num < 0)// || send_num != q->size)
            {
                printf("server_send_run: send_num=%d \n", send_num);
                perror("server_send_run: sendto error:");
                ret = -2;
                StopSend(obj);
                break;
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
        //printf("server_send_run: usleep \n");
        usleep(2000);//1ms
    }

    return ret;
}
int process_recv_data(SocketObj *obj, DataNode *head, int sock_fd, int len)
{
    int ret = 0;
    struct sockaddr_in addr_client;
    if(head && head->num)
    {
        //printf("server_send_run: head->num=%d \n", head->num);
        do{
            DataNode *q;
            q = head->next;
            if(q == NULL || q == head)
            {
                break;
            }
            else{
                head->next = head->next->next;
            }
            //
            char *send_buf = (char *)q->data;
            int send_num = q->size;
            addr_client = q->addr_client;
            //
            UserData *udata = (UserData *)send_buf;
            if(udata->data0.head.datatype)
            {
                RelayData(obj, send_buf, send_num, addr_client, sock_fd, len);
            }
            else{
                send_num = ProcessCmd(obj, send_buf, send_num, addr_client);
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
        //printf("server_send_run: usleep \n");
        usleep(2000);//1ms
    }
    return ret;
}
int server_send_run(SocketObj *obj)
{
    int ret = 0;
    obj->send_status = 1;
    int sock_fd = obj->sock_fd;
    struct sockaddr_in addr_serv = obj->addr_serv;
    //struct sockInfo *head = NULL;
    int status = obj->send_status;
    int send_num = 10;
    char *send_buf;//[sizeof(UserData)] = "i am server!";
    struct sockaddr_in addr_client;
    int len = sizeof(addr_serv);
    obj->start_time = get_sys_time();
    int64_t time0 = get_sys_time();
    int64_t sendsize = 0;
    obj->pkt_send_num = 0;
    while(status)
    {
        int64_t now_time = get_sys_time();
        DataNode *head = (DataNode *)PopDataAll(obj);
        //printf("server_send_run:  \n");
        //sendsize += send_data(obj, head, sock_fd, len);
        sendsize += process_recv_data(obj, head, sock_fd, len);
        int difftime = (int)(now_time - time0);
        if(difftime > 2000)
        {
            int bitrate = (int)(((sendsize << 3) * 1) / (difftime * 1000));
            //printf("server_main: recvsize=%lld \n", recvsize);
            time0 = now_time;
            sendsize = 0;
            printf("server_send_run: bitrate=%d (Mbps) \n", bitrate);
        }
        pthread_mutex_lock(&obj->status_lock);
        status = obj->send_status & obj->recv_status;
        pthread_mutex_unlock(&obj->status_lock);
    }
    StopSend(obj);
    return ret;
}
int server_main(SocketObj *obj)
{
    int ret = 0;
    if(obj->init)
    {
        obj->send_status = 1;
        ret = server_send_run(obj);
        return ret;
    }
    obj->recv_status = 1;
    int taskId = obj->id;
    /* sock_fd --- socket文件描述符 创建udp套接字*/
    obj->sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(obj->sock_fd < 0)
    {
        perror("socket");
        return -1;
        //exit(1);
    }

    /* 将套接字和IP、端口绑定 */
    //struct sockaddr_in addr_serv;
    //int len;
    memset(&obj->addr_serv, 0, sizeof(struct sockaddr_in));  //每个字节都用0填充
    obj->addr_serv.sin_family = AF_INET;                       //使用IPV4地址
    obj->addr_serv.sin_port = htons(SERV_PORT);                //端口
    /* INADDR_ANY表示不管是哪个网卡接收到数据，只要目的端口是SERV_PORT，就会被该应用程序接收到 */
    obj->addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);  //自动获取IP地址
    //
    //超时时间
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 500000;//500ms

	socklen_t len = sizeof(timeout);
	ret = setsockopt(obj->sock_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, len);
	if (ret == -1) {
		//error = errno;
		//while ((close(obj->sock_fd) == -1) && (errno == EINTR));
		//errno = error;
		return -2;
	}
	ret = setsockopt(obj->sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, len);
	if (ret == -1) {
	    return -3;
	}
    //
    len = sizeof(obj->addr_serv);
    /* 绑定socket */
    if(bind(obj->sock_fd, (struct sockaddr *)&obj->addr_serv, sizeof(obj->addr_serv)) < 0)
    {
        perror("bind error:");
        return -4;
        //exit(1);
    }
    pthread_mutex_init(&obj->lock,NULL);
    pthread_mutex_init(&obj->status_lock,NULL);
    //
    struct sockInfo *pnew;
    obj->sockHead = create();
    if (obj->sockHead == NULL)
        return 0;
    printf("输出创建的链表：");
    display(obj->sockHead);
    //
#if 0
    pnew=(NODE *)malloc(sizeof(NODE));
    if (pnew==NULL) {
        printf("创建失败！");
        return 0;
    }
    pnew->id = 88;
    insert(head,pnew, 0);   //将新节点插入节点3的后面
    printf("插入后的链表：");
    display(head);
    pdelete(head,1);   //删除节点3
    printf("删除后的链表：");
    display(head);
    Pfree(head);
#endif
    pthread_t tid;
    if(pthread_create(&tid, NULL, server_send_run, obj) < 0)
    {
        printf("Create server_send_run failed!\n");
        //exit(0);
    }
    //
    //obj->recv_status = 1;
    //obj->send_status = 1;
    server_recv_run(obj, obj->sock_fd, obj->addr_serv);
    StopSend(obj);
    if (pthread_join(tid, NULL))
    {
        printf("thread is not exit...\n");
    }
    Pfree(obj->sockHead);
    close(obj->sock_fd);
    pthread_mutex_destroy(&obj->lock);
    pthread_mutex_destroy(&obj->status_lock);
    printf("server_main: over \n");
    return 0;
}

int client_send_run(SocketObj *obj)
{
    int ret = 0;
    obj->send_status = 1;
    int sock_fd = obj->sock_fd;
    struct sockaddr_in addr_serv = obj->addr_serv;
    int len = sizeof(addr_serv);
    int send_num;
    char send_buf[sizeof(UserData)] = "";//"hey, who are you?";
    //
    int datatype = 1;
    //AVData *udata = (AVData *)send_buf;
    UserData *udata = (UserData *)send_buf;
    udata->data0.head.datatype = datatype;
    udata->data1.head.datatype = datatype;
    int size = sizeof(AVData);
    //int size = sizeof(CmdData);
    udata->data0.head.size = size;// >> 1;
    //char *text = (char *)&udata;//"hey, who are you?";
    //strcpy(send_buf, text);
    //memcpy(send_buf, text, size);
    //printf("client send: %s\n", send_buf);
    int seqnum = 0;
    int status = obj->send_status;
    obj->start_time = get_sys_time();
    int64_t time0 = get_sys_time();
    int64_t sendsize = 0;
    int pacedidx = 0;
    int pacedsumbytes = 0;
    int pacedstep = 1;//4;

    while(status)
    {
        int64_t now_time = get_sys_time();
        int difftime = (int)(now_time - obj->start_time);
        if(difftime > (60 * 1000))
        //if(seqnum > 10)
        {
            printf("client_send_run: difftime=%d \n", difftime);
            datatype = 0;
            obj->send_status = 0;
        }
        else if(!seqnum)
        {
            datatype = 0;
        }
        else{
            datatype = 1;
        }
        if(datatype)
        {
            udata->data0.head.datatype = datatype;
            size = sizeof(AVData);
            udata->data0.head.size = size;
            udata->data0.seqnum = seqnum;
        }
        else{
            udata->data1.head.datatype = datatype;
            size = sizeof(CmdData);
            udata->data1.head.size = size;
            if(!seqnum)
            {
                udata->data1.cmdtype = (CMDType)kReg;//kExit;
                udata->data1.status = 1;
            }
            else{
                udata->data1.cmdtype = (CMDType)kBye;//kExit;
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
        send_num = sendto(sock_fd, send_buf, size, 0, (struct sockaddr *)&addr_serv, len);//strlen(send_buf)
        if(send_num < 0)
        {
            printf("client_send_run: sendto error:size=%d \n", size);
            //exit(1);
            ret = -1;
            break;
        }
        sendsize += send_num;
        seqnum++;
        difftime = (int)(now_time - time0);

        int64_t bitrate = glob_paced_bitrate;//PACED_BITRATE;//3 * 1024 * 1024 * 1024LL;//bps
        if(pacedstep > 1)
        {
            pacedsumbytes += send_num;
            pacedidx++;
            if(pacedidx > pacedstep)
            {
                PacedSend(bitrate, pacedsumbytes, difftime, sendsize);

                pacedidx = 0;
                pacedsumbytes = 0;
            }
        }
        else{
            PacedSend(bitrate, send_num, difftime, sendsize);
        }

        if(difftime > 2000)
        {
            int bitrate = (int)(((sendsize << 3) * 1) / (difftime * 1000));
            //printf("server_main: recvsize=%lld \n", recvsize);
            time0 = now_time;
            sendsize = 0;
            printf("client_send_run: bitrate=%d (Mbps) \n", bitrate);
        }

        pthread_mutex_lock(&obj->status_lock);
        status = obj->send_status;
        pthread_mutex_unlock(&obj->status_lock);
    }
    StopSend(obj);
    WaitRecv(obj);
    return ret;
}

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
    obj->recv_status = 1;
    int sock_fd = obj->sock_fd;
    struct sockaddr_in addr_serv = obj->addr_serv;
    int len = sizeof(addr_serv);
    int recv_num;
    char recv_buf[sizeof(UserData)] = "";//"hey, who are you?";
    int data_len = sizeof(UserData);
    //
    int datatype = 1;
    int status = obj->recv_status;
    //obj->start_time = get_sys_time();
    int64_t time0 = get_sys_time();
    int64_t recvsize = 0;

    while(status)
    {
        int64_t now_time = get_sys_time();
        recv_num = recvfrom(sock_fd, recv_buf, data_len, 0, (struct sockaddr *)&addr_serv, (socklen_t *)&len);
        if(recv_num < 0)
        {
            perror("client_recv_run: recvfrom error:");
            //exit(1);
            ret = -1;
            break;
        }
        recvsize += recv_num;
#if 0
        UserData *udata = (UserData *)recv_buf;
        if(udata->data0.head.datatype)
        {
            PushData(obj, recv_buf, recv_num);
        }
        else{
            status = ClientProcessCmd(obj, recv_buf);
        }
#else
        ///PushData(obj, recv_buf, recv_num, addr_serv);
#endif
        int difftime = (int)(now_time - time0);
        if(difftime > 2000)
        {
            int bitrate = (int)(((recvsize << 3) * 1) / (difftime * 1000));
            //printf("server_main: recvsize=%lld \n", recvsize);
            time0 = now_time;
            recvsize = 0;
            printf("client_recv_run: bitrate=%d (Mbps) \n", bitrate);
        }

        pthread_mutex_lock(&obj->status_lock);
        status = obj->recv_status & obj->send_status;
        pthread_mutex_unlock(&obj->status_lock);
    }
    StopRecv(obj);
    return ret;
}

int client_main(SocketObj *obj)
{
    int ret = 0;
    if(obj->init)
    {
        ret = client_recv_run(obj);
        return ret;
    }
    int taskId = obj->id;
    /* socket文件描述符 */
    int sock_fd;

    /* 建立udp socket */
    obj->sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(obj->sock_fd < 0)
    {
        perror("socket");
        //exit(1);
        return -1;
    }
    //超时时间
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 500000;//500ms

	socklen_t len = sizeof(timeout);
	ret = setsockopt(obj->sock_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, len);
	if (ret == -1) {
		perror("client_main error:");
		return -2;
	}
	ret = setsockopt(obj->sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, len);
	if (ret == -1) {
	    perror("client_main error:");
	    return -3;
	}
    /* 设置address */
    //struct sockaddr_in addr_serv;
    memset(&obj->addr_serv, 0, sizeof(obj->addr_serv));
    obj->addr_serv.sin_family = AF_INET;
    obj->addr_serv.sin_addr.s_addr = inet_addr(DSET_IP_ADDRESS);
    obj->addr_serv.sin_port = htons(DEST_PORT);
    //obj->recv_status = 1;
    //obj->send_status = 1;
    pthread_mutex_init(&obj->lock,NULL);
    pthread_mutex_init(&obj->status_lock,NULL);
    //
    pthread_t tid;
    if(pthread_create(&tid, NULL, client_recv_run, obj) < 0)
    {
        printf("Create client_recv_run failed!\n");
        //exit(0);
    }

    client_send_run(obj);
    printf("client exit: taskId=%d \n", taskId);
    StopRecv(obj);
    if (pthread_join(tid, NULL))
    {
        printf("thread is not exit...\n");
    }

    close(obj->sock_fd);
    pthread_mutex_destroy(&obj->lock);
    pthread_mutex_destroy(&obj->status_lock);

    return 0;
}

/*
*线程池里所有运行和等待的任务都是一个CThread_worker
*由于所有任务都在链表里，所以是一个链表结构
*/
typedef struct worker
{
    /*回调函数，任务运行时会调用此函数，注意也可声明成其它形式*/
    void *(*process) (void *arg);
    void *arg;/*回调函数的参数*/
    struct worker *next;
} CThread_worker;

/*线程池结构*/
typedef struct
{
    pthread_mutex_t queue_lock;
    pthread_cond_t queue_ready;

    /*链表结构，线程池中所有等待任务*/
    CThread_worker *queue_head;

    /*是否销毁线程池*/
    int shutdown;
    pthread_t *threadid;
    /*线程池中允许的活动线程数目*/
    int max_thread_num;
    /*当前等待队列的任务数目*/
    int cur_queue_size;

} CThread_pool;


int pool_add_worker (void *(*process) (void *arg), void *arg);
void *thread_routine (void *arg);


static CThread_pool *pool = NULL;
void pool_init (int max_thread_num)
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
        pthread_create (&(pool->threadid[i]), NULL, thread_routine,NULL);
    }
}

/*向线程池中加入任务*/
int pool_add_worker (void *(*process) (void *arg), void *arg)
{
    /*构造一个新任务*/
    CThread_worker *newworker = (CThread_worker *) malloc (sizeof (CThread_worker));
    newworker->process = process;
    newworker->arg = arg;
    newworker->next = NULL;/*别忘置空*/

    pthread_mutex_lock (&(pool->queue_lock));
    /*将任务加入到等待队列中*/
    CThread_worker *member = pool->queue_head;
    if (member != NULL)
    {
        while (member->next != NULL)
            member = member->next;
        member->next = newworker;
    }
    else
    {
        pool->queue_head = newworker;
    }

    assert (pool->queue_head != NULL);

    pool->cur_queue_size++;
    pthread_mutex_unlock (&(pool->queue_lock));
    /*好了，等待队列中有任务了，唤醒一个等待线程；
    注意如果所有线程都在忙碌，这句没有任何作用*/
    pthread_cond_signal (&(pool->queue_ready));
    return 0;
}

/*销毁线程池，等待队列中的任务不会再被执行，但是正在运行的线程会一直
把任务运行完后再退出*/
int pool_destroy ()
{
    if (pool->shutdown)
        return -1;/*防止两次调用*/
    pool->shutdown = 1;

    /*唤醒所有等待线程，线程池要销毁了*/
    pthread_cond_broadcast (&(pool->queue_ready));

    /*阻塞等待线程退出，否则就成僵尸了*/
    int i;
    for (i = 0; i < pool->max_thread_num; i++)
        pthread_join (pool->threadid[i], NULL);
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

void * thread_routine (void *arg)
{
    printf ("starting thread 0x%x\n", pthread_self ());
    while (1)
    {
        pthread_mutex_lock (&(pool->queue_lock));
        /*如果等待队列为0并且不销毁线程池，则处于阻塞状态; 注意
        pthread_cond_wait是一个原子操作，等待前会解锁，唤醒后会加锁*/
        while (pool->cur_queue_size == 0 && !pool->shutdown)
        {
            printf ("thread 0x%x is waiting\n", pthread_self ());
            pthread_cond_wait (&(pool->queue_ready), &(pool->queue_lock));
        }

        /*线程池要销毁了*/
        if (pool->shutdown)
        {
            /*遇到break,continue,return等跳转语句，千万不要忘记先解锁*/
            pthread_mutex_unlock (&(pool->queue_lock));
            printf ("thread 0x%x will exit\n", pthread_self ());
            pthread_exit (NULL);
        }

        printf ("thread 0x%x is starting to work\n", pthread_self ());

        /*assert是调试的好帮手*/
        assert (pool->cur_queue_size != 0);
        assert (pool->queue_head != NULL);

        /*等待队列长度减去1，并取出链表中的头元素*/
        pool->cur_queue_size--;
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
    //printf ("threadid is 0x%x, working on task %d\n", pthread_self (),*(int *) arg);
    SocketObj *obj = (SocketObj *)arg;//calloc(1, sizeof(SocketObj));
    //obj->id = taskId;
    int taskId = obj->id;
    if(!taskId)
    {
        obj->type = 0;
        server_main(obj);
    }
    else{
        obj->type = 1;
        client_main(obj);
    }
    //sleep (1);/*休息一秒，延长任务的执行时间*/
    //printf("myprocess: obj=%x \n", obj);
    //printf("myprocess: obj->id=%d \n", obj->id);
    return NULL;//(void *)obj;
}

HCSVC_API
int pool_main()
{
    int aliveTaskNum = 100;
    int taskNum = 2;//100;//3;//2;//3;//10;
    //
    if(!GlobSessionList)
    {
        GlobSessionList = (SessionInfo *)calloc(1, glob_session_num * sizeof(SessionInfo));
    }
    glob_paced_bitrate = glob_paced_bitrate / (taskNum - 1);
    //
    pool_init (aliveTaskNum);/*线程池中最多三个活动线程*/

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
        pool_add_worker(myprocess, obj);
        //printf("pool_main: i=%d, hnd[i]=%x \n", i, hnd[i]);
    }
    printf("pool_main: wait... \n");
    /*等待所有任务完成*/
    sleep (1);
    for (i = 0; i < taskNum; i++)
    {
        if(hnd[i])
        {
            printf("pool_main: hnd[i]->id=%d \n", hnd[i]->id);
            printf("pool_main: hnd[i]->send_status=%d \n", hnd[i]->send_status);
            printf("pool_main: hnd[i]->recv_status=%d \n", hnd[i]->recv_status);
        }
    }
    printf("pool_main: pool_destroy \n");
    /*销毁线程池*/
    pool_destroy ();

    free (workingnum);
    printf("pool_main: over \n");
    return 0;
}
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
 printf("malloc threadpool fail");
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
printf("malloc threads fail");
break;
 }
 memset(pool->threads, 0, sizeof(pool->threads));
 pool->task_queue = (threadpool_task_t *)malloc(sizeof(threadpool_task_t)*queue_max_size);
 if (pool->task_queue == NULL)
 {
printf("malloc task_queue fail");
break;
 }
 if (pthread_mutex_init(&(pool->lock), NULL) != 0
|| pthread_mutex_init(&(pool->thread_counter), NULL) != 0
|| pthread_cond_init(&(pool->queue_not_empty), NULL) != 0
|| pthread_cond_init(&(pool->queue_not_full), NULL) != 0)
 {
printf("init the lock or cond fail");
break;
 }
 /**
 * start work thread min_thr_num
 */
 for (int i = 0; i < min_thr_num; i++)
 {
//启动任务线程
pthread_create(&(pool->threads[i]), NULL, threadpool_thread, (void *)pool);
printf("start thread 0x%x...\n", pool->threads[i]);
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
printf("thread 0x%x is waiting\n", pthread_self());
pthread_cond_wait(&(pool->queue_not_empty), &(pool->lock));
//被唤醒后，判断是否是要退出的线程
if (pool->wait_exit_thr_num > 0)
{
pool->wait_exit_thr_num--;
if (pool->live_thr_num > pool->min_thr_num)
{
printf("thread 0x%x is exiting\n", pthread_self());
 pool->live_thr_num--;
 pthread_mutex_unlock(&(pool->lock));
 pthread_exit(NULL);
}
}
}
if (pool->shutdown)
{
pthread_mutex_unlock(&(pool->lock));
printf("thread 0x%x is exiting\n", pthread_self());
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
printf("thread 0x%x start working\n", pthread_self());
pthread_mutex_lock(&(pool->thread_counter));
pool->busy_thr_num++;
pthread_mutex_unlock(&(pool->thread_counter));
(*(task.function))(task.arg);
// task run over
printf("thread 0x%x end working\n", pthread_self());
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
//printf("thread 0x%x working on task %d\n ",pthread_self(),*(int *)arg);
//sleep(1);
//printf("task %d is end\n",*(int *)arg);
//return NULL;
//}
//int main()
//{
//threadpool_t *thp = threadpool_create(3,100,12);
//printf("pool inited");
//
//int *num = (int *)malloc(sizeof(int)*20);
//for (int i=0;i<10;i++)
//{
//num[i]=i;
//printf("add task %d\n",i);
//threadpool_add(thp,process,(void*)&num[i]);
//}
//sleep(10);
//threadpool_destroy(thp);
//}
#endif
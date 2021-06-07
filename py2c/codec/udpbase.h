#ifndef UDPBASE_H
#define UDPBASE_H
#include "hcsvc.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#if defined (WIN32)
#include <winsock2.h>
#include<ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <pthread.h>
#include <assert.h>
//
#include <stdbool.h>
//

#if defined (WIN32)
#define SOCKFD SOCKET
#elif defined(__linux__)
#define SOCKFD int
#endif
extern int64_t get_sys_time();
extern int64_t get_sys_time0();

//network congestion
//极限客户发送5Gbps,服务接收11Gbps
#define MAX_DATA_SIZE 1500
#define PACED_BITRATE (int64_t)(2.5 * 1024 * 1024 * 1024) //安全的极限值2~2.5Gbps
//#define PACED_BITRATE (int64_t)(2.0 * 1024 * 1024 * 1024)
//#define PACED_BITRATE (int64_t)(1.0 * 1024 * 1024 * 1024)
//#define PACED_BITRATE (int64_t)(0.8 * 1024 * 1024 * 1024)
#define MAX_SESSION_NUM 1000
//#define HEARTBEAT_TIME  30000 //30s
#define HEARTBEAT_TIME  300000 //300s
#define MAX_CHAN_NUM 64
#define DECACTOR 128
//#define LOOPRELAY
#define TEST_TIME_LEN (12 * 60 * 60 * 1000) //12hours
#define MAX_ALIVE_TASK_NUM  128
#define MAX_TASK_NUM        256
#define MAX_DELAY_PKT_NUM (10 * 2048)//2048
//#define DEFAULT_WIDTH 1280
//#define DEFAULT_HEIGHT 720
#define DEFAULT_WIDTH 1920
#define DEFAULT_HEIGHT 1080
#define SHOW_DELAY 1
//#define SIMULATOR_DATA//VIRTUAL_CAPTURE
#define LOSSLESS 0
//#define SIMULATOR_LOSSRATE 0//7
#define COUNT_LOSSRATE_INTERAVL 400 //ms//同时也是最大乱序容限
#define OPEN_RTX 1
#define OPEN_LOSSNET 1
#define OPEN_DELAYNET 1
#define OPEN_NETACK 1
#define AUDIO_MAX_DELAY_PKT_NUM 16
#define MAX_DELAY_FRAME_NUM 25
#define MAX_INTERVAL 4000 //4s
#define MAX_INTERVAL_CLIENT 2000 //4s
#define MIN_RTT 100//50
//
#define CLIENT_MODE 1
//
//#define SERV_PORT   8000
//#define DEST_PORT 8000
//#define DSET_IP_ADDRESS  "127.0.0.1"

typedef enum{
    kCMD,
    kDATA,
}DataType;
typedef enum{
    kReg,
    kBroadCastReg,
    kGetSessionId,
    kGetChanId,
    kBye,
    kHeartBeat,
    kRTT,
    kAsk,
    kBroadCast,
    kExit,
}CMDType;
typedef struct{
    unsigned short datatype : 1;
    unsigned short size : 12;// >>1
    unsigned short codec : 1;//0:video;1:audio
    unsigned short rtx : 1;//重发包
    unsigned short fec : 1;
}DataHeader;
typedef struct{
    DataHeader head;
    int seqnum;
    int sessionId : 16;
    int chanId : 8;
    int64_t now_time;//test
    char data[MAX_DATA_SIZE];
}AVData;
typedef struct{
    //0 byte
   DataHeader head;
    //1 byte
    uint8_t avtype : 1;
    uint8_t selfmode : 1;
    uint8_t testmode : 1;
    uint8_t status : 1;
    uint8_t cmdtype : 4;
    //2 byte
    char chanId;
    //3 byte
    char modeId;
    //4 byte
    uint8_t actor;
    uint8_t sessionTextSize;
    //6 byte
    short sessionId;
    //totle 8 bytes
    uint32_t randId;
    //totle 12 bytes
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

struct RtxList{
    int num;//frame_num
    int idx;
    int id;
    int pkt_num;
    AVData *datalist;//calloc(1, max_pkt_num * sizeof(SeqFrame))
    int min_seqnum;
    int max_seqnum;
    int64_t frame_timestamp;
    int64_t now_time;
    int64_t last_frame_time;
    int frame_size;
    int pkt_sum_size;
    int max_data_size;
    int max_pkt_num;
    int rtp_pkt_num;
    int start_seqnum;
    int pict_type;
    int ref_idx;
    struct RtxList *tail;
    struct RtxList *next;
};
typedef struct RtxList RtxNode;

typedef struct{
    //uint8_t status;
    uint8_t *data;
    short size;
    short is_rtx;
}SeqFrame;
struct FrameJitterBuffer{
    int num;//frame_num
    int idx;
    int id;
    int pkt_num;
    SeqFrame *seqFrame;//calloc(1, max_pkt_num * sizeof(SeqFrame))
    int min_seqnum;
    int max_seqnum;
    int64_t frame_timestamp;
    int64_t now_time;
    int64_t last_frame_time;
    int frame_size;
    int pkt_sum_size;
    int max_data_size;
    int max_pkt_num;
    int rtp_pkt_num;
    int start_seqnum;
    int ref_idx;
    int has_rtx;
    int ref_idc;
    int loss_rate;
    //for count loss rate
    int last_min_seqnum;
    int last_max_seqnum;
    int last_pkt_num;
    int64_t last_frame_timestamp;

    struct FrameJitterBuffer *tail;
    struct FrameJitterBuffer *next;
};
typedef struct FrameJitterBuffer FrameJitterNode;

struct McuBuffer{
    int num;//frame_num
    int idx;
    uint8_t *data;
    int size;
    int64_t frame_timestamp;
    struct McuBuffer *tail;
    struct McuBuffer *next;
};
typedef struct McuBuffer McuNode;

struct DataInfo{
    struct sockaddr_in addr_client;
    uint8_t *data;
    int size;
    int num;
    int idx;//test
    int64_t now_time;
    struct DataInfo *tail;
    struct DataInfo *next;
};
typedef struct DataInfo DataNode;


struct sockInfo {
    //void *sock;//
    int num;
    struct sockaddr_in addr_client;
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
    struct sockInfo *tail;
    struct sockInfo *next;
};
typedef struct sockInfo SockNode;  //typedef为C语言的关键字，作用是为一种数据类型定义一个新名字。

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
    uint32_t randId;
    //struct sockaddr_in broadCast;
    struct sockInfo encodecClients[MAX_CHAN_NUM];
    char chanIdList[MAX_CHAN_NUM];
    uint32_t randIdList[MAX_CHAN_NUM];
    //struct sockInfo *broadCastHead;
    struct sockInfo *decodecHead;
}SessionInfo;

typedef struct{
    int session_idx;
    int port;
    char server_ip[64];
    int id;//not chanId
    int status;
    int init;
    int recv_status;
    int send_status;
    int broadcast_status;
    int pause;
    SOCKFD sock_fd;
    int type;
    int64_t start_time;
    int64_t last_send_time;
    int64_t pkt_send_num;
    int64_t pkt_recv_num;
    //test
    //int min_seqnum;
    //int max_seqnum;
    //
    int paced_bitrate;
    struct sockaddr_in addr_serv;
    pthread_mutex_t lock;
    pthread_mutex_t status_lock;
    void *data_list;
    //struct sockInfo *sockHead;
    FrameJitterNode *frameJitterHead;
    struct sockInfo *broadCastHead;
    void *session;
    char params[2048];
    SessionInfo *SessionList;
}SocketObj;


//
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

typedef struct{
    //cJSON *json;
    void *json;
    char *param;
    int print;
    int aliveTaskNum;
    int taskNum;
    SocketObj **hnd;
    CThread_pool *pool;
    void *thread_fun;
}TaskPool;

#endif
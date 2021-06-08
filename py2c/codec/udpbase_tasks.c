#include "inc.h"
#include "udpbase.h"

extern float GetvalueFloat(cJSON *json, char *key);
extern void * server_main(SocketObj *obj);
extern void * client_broadcast(SocketObj *obj);
extern void * client_main(SocketObj *obj);
//extern int InitTask(TaskPool *task);
extern int CloseTask(TaskPool *task);
extern int AddTask(TaskPool *task, char *params, void *thread_fun, int taskId);
extern void SetPause(SocketObj *obj);
extern void ClearPause(SocketObj *obj);
extern void WaitPause2(SocketObj *obj);
extern int client_broadcast_get(SocketObj *obj, UserData *udata0);
extern void StopSend(SocketObj *obj);
extern void StopRecv(SocketObj *obj);
//extern void WaitSend(SocketObj *obj);
//extern void WaitRecv(SocketObj *obj);
extern int WaitCapture(SocketObj *obj, int status, int avtype);
extern void WaitRender(SocketObj *obj);
//
extern void * video_encode(SocketObj *obj);
extern void * video_decode(SocketObj *obj);
extern void * video_capture(SocketObj *sock);
extern void * video_render(SocketObj *sock);
extern void * audio_encode(SocketObj *obj);
extern void * audio_decode(SocketObj *obj);
extern void * audio_capture(SocketObj *sock);
extern void * audio_player(SocketObj *sock);
extern void * mcu_task(SocketObj *sock);
extern void * read_stream(SocketObj *sock);


extern int video_set_capture(SocketObj *obj, CallCapture *capture, int capture_status);
extern int video_set_render(SocketObj *obj, CallRender *render);
extern void CreateNetInfoHnd(NetInfoObj **hnd);
extern void CloseNetInfoHnd(NetInfoObj *obj);
extern int video_set_netinfo(SocketObj *obj, NetInfoObj *netInfoObj);
extern int video_set_stream(SocketObj *sock, char *handle, int is_encoder);
extern int audio_set_stream(SocketObj *sock, char *handle, int is_encoder);
extern int video_set_readstream(SocketObj *sock, char *handle);
extern int audio_set_readstream(SocketObj *sock, char *handle);
extern int set_mcu2render(SocketObj *sock, McuObj *mcu);
extern int set_mcu2player(SocketObj *sock, McuObj *mcu);
extern void WaitAVStream(SocketObj *obj);

extern void * PopData(SocketObj *obj);

NetInfoObj *globalNetInfoObj = NULL;


/*
建立广播线程池;
建立会话线程池：
建立编解码线程；
建立数据发送/接收线程;
*/

int broadcast_test(SocketObj *obj, int selfmode, int modeId)
{
    int ret = 0;
    SetPause(obj);
    WaitPause2(obj);
    //usleep(500000);//500ms

    UserData udata;
    udata.data1.head.datatype = 0;
    int size = sizeof(CmdData);
    udata.data1.head.size = size;
    //
    udata.data1.status = 1;
    udata.data1.chanId = -1;//obj.id;
    udata.data1.avtype = 0;
    udata.data1.selfmode = selfmode;
    udata.data1.testmode = 0;
    udata.data1.modeId = modeId;
    udata.data1.actor = DECACTOR + 1;
    udata.data1.sessionId = -1;
    //

    udata.data1.cmdtype = (CMDType)kHeartBeat;
    ret = client_broadcast_get(obj, &udata);
    printf("client_broadcast: kHeartBeat: ret=%d \n", ret);

    udata.data1.st0 = get_sys_time();
    udata.data1.cmdtype = (CMDType)kRTT;
    ret = client_broadcast_get(obj, &udata);
    printf("client_broadcast: kRTT: ret=%d \n", ret);

    udata.data1.cmdtype = (CMDType)kGetSessionId;
    ret = client_broadcast_get(obj, &udata);
    printf("client_broadcast: kGetSessionId: ret=%d \n", ret);

    udata.data1.chanId = -1;
    udata.data1.randId = api_create_id((1 << 30));
    udata.data1.cmdtype = (CMDType)kGetChanId;
    ret = client_broadcast_get(obj, &udata);
    printf("client_broadcast: kGetChanId: ret=%d \n", ret);

    udata.data1.chanId = -1;
    udata.data1.randId = api_create_id((1 << 30));
    udata.data1.cmdtype = (CMDType)kGetChanId;
    ret = client_broadcast_get(obj, &udata);
    printf("client_broadcast: kGetChanId: ret=%d \n", ret);

    //udata.data1.cmdtype = (CMDType)kReg;
    //udata.data1.actor = 1;
    //ret = client_broadcast_get(obj, &udata);
    //printf("client_broadcast: kReg: ret=%d \n", ret);

    //udata.data1.cmdtype = (CMDType)kReg;
    //udata.data1.actor = DECACTOR;
    //ret = client_broadcast_get(obj, &udata);
    //printf("client_broadcast: kReg: ret=%d \n", ret);

    //udata.data1.cmdtype = (CMDType)kBye;
    //ret = client_broadcast_get(obj, &udata);
    //printf("client_broadcast: kBye: ret=%d \n", ret);
#if 0
    udata.data1.cmdtype = (CMDType)kExit;
    ret = client_broadcast_get(obj, &udata);
    printf("client_broadcast: kExit: ret=%d \n", ret);
    StopRecv(obj);
#endif
    //
    ClearPause(obj);

    return ret;
}
//=========================================================================================
HCSVC_API
int api_pool_start(char *handle, char *params)
{
    int ret = 0;
    cJSON *json = (cJSON *)api_str2json(params);
    int taskType = GetvalueInt(json, "taskType");

    api_taskpool_init(handle, params);

    if(!globalNetInfoObj && taskType)
    {
        CreateNetInfoHnd(&globalNetInfoObj);
    }
    return ret;
}
HCSVC_API
int api_start_broadcast(char *handle, char *params0)
{
    int ret = 0;
    printf("api_start_broadcast: params0=%s \n", params0);
    cJSON *json = (cJSON *)api_str2json(params0);
    int taskId = GetvalueInt(json, "taskId");
    printf("api_start_broadcast: taskId=%d \n", taskId);
    int port = GetvalueInt(json, "port");
    printf("api_start_broadcast: port=%d \n", port);
    char server_ip[64] = "";
    GetvalueStr(json, "server_ip", server_ip);

    long long *testp = (long long *)handle;
    TaskPool *task = (TaskPool *)testp[0];


    char params[1024] = "";

    strcpy(params, "{\"name\":\"broadcast\"}");
    json = (cJSON *)api_str2json(params);
    json = api_renew_json_int(json, "port", port);
    json = api_renew_json_str(json, "server_ip", server_ip);
    char *jsonStr = api_json2str(json);

    ret = api_taskpool_addtask(handle, jsonStr, client_broadcast, taskId);
    //NetInfoObj *globalNetInfoObj = NULL;
    //CreateNetInfoHnd(&globalNetInfoObj);
    printf("api_start_broadcast: port=%d \n", port);
    printf("api_start_broadcast: server_ip=%s \n", server_ip);

    ret = taskId;
    return ret;
}
HCSVC_API
SessInfoPointer api_get_sessionInfo(char *handle, int taskId)
{
    SessInfoPointer pRet = NULL;

    long long *testp = (long long *)handle;
    TaskPool *task = (TaskPool *)testp[0];
    SocketObj *obj = (SocketObj *)task->hnd[taskId];
    //printf("api_get_sessionInfo: taskId=%d \n", taskId);
#if 1
    DataNode *q = (DataNode *)PopData(obj);
    //printf("api_get_sessionInfo: q=%x \n", q);
    if(q)
    {
        UserData *udata = (UserData *)q->data;
        //printf("api_get_sessionInfo: udata=%x \n", udata);
        int datatype = udata->data1.head.datatype;
        //printf("api_get_sessionInfo: datatype=%d \n", datatype);
        if(datatype == kCMD)
        {
            int cmd = udata->data1.cmdtype;
            //printf("api_get_sessionInfo: cmd=%d \n", cmd);
            if(cmd == kBroadCast)
            {
                int status = udata->data1.status;
                int chanId = udata->data1.chanId;
                int avtype = udata->data1.avtype;
                int selfmode = udata->data1.selfmode;
                int testmode = udata->data1.testmode;
                int modeId = udata->data1.modeId;
                int actor = udata->data1.actor;
                int sessionId = udata->data1.sessionId;
                //printf("api_get_sessionInfo: selfmode=%d \n", selfmode);
                //printf("api_get_sessionInfo: status=%d \n", status);
                //printf("api_get_sessionInfo: modeId=%d \n", modeId);
                //printf("api_get_sessionInfo: avtype=%d \n", avtype);
                //printf("api_get_sessionInfo: actor=%d \n", actor);
                pRet = (SessInfoPointer)calloc(1, sizeof(SessInfoPointerTest));
                pRet->sessionId = sessionId;
                pRet->status = status;
	            pRet->modeId = modeId;
	            pRet->avtype = avtype;
            }
        }
        //
        free(q->data);
        free(q);   //释放节点i的内存单元
    }
#else
    DataNode *head = (DataNode *)PopDataAll(obj);
    if(head && head->num)
    {
        //printf("client_send_run: head->num=%d \n", head->num);
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
            UserData *udata = (UserData *)q->data;
            if(udata->data1.head.datatype == kCMD)
            {
                int cmd = udata->data1.cmdtype;
                if(cmd == kBroadCast)
                {
                    int status = udata->data1.status;
                    int chanId = udata->data1.chanId;
                    int avtype = udata->data1.avtype;
                    int selfmode = udata->data1.selfmode;
                    int testmode = udata->data1.testmode;
                    int modeId = udata->data1.modeId;
                    int actor = udata->data1.actor;
                    int sessionId = udata->data1.sessionId;
                    pRet = (SessInfoPointer)calloc(1, sizeof(SessInfoPointerTest));
                    pRet->sessionId = sessionId;
                    pRet->status = status;
	                pRet->modeId = modeId;
	                pRet->avtype = avtype;
                }
            }
            //
            free(q->data);
            free(q);   //释放节点i的内存单元
        }while(1);
        free(head);
    }
    else{
        usleep(2000);//1ms
    }
#endif
    return pRet;
}
HCSVC_API
void api_free(void *pRet)
{
    if(pRet)
    {
        free(pRet);
    }
}
HCSVC_API
int api_broadcast_cmd(char *handle, char *params)
{
    int ret = 0;
    cJSON *json = (cJSON *)api_str2json(params);
    int selfmode = GetvalueInt(json, "selfmode");
    int modeId = GetvalueInt(json, "modeId");
    int taskId = GetvalueInt(json, "taskId");
    int sessionId = GetvalueInt(json, "sessionId");
    int cmdtype = 0;
    char ctmp[32] = "";
    GetvalueStr(json, "cmdtype", ctmp);
    if(!strcmp(ctmp, "kGetSessionId"))
    {
        cmdtype = kGetSessionId;
    }
    else if(!strcmp(ctmp, "kGetChanId"))
    {
        cmdtype = kGetChanId;
    }
    else if(!strcmp(ctmp, "kRTT"))
    {
        cmdtype = kRTT;
    }
    else if(!strcmp(ctmp, "kBye"))
    {
        cmdtype = kBye;
    }
    else if(!strcmp(ctmp, "kExit"))
    {
        cmdtype = kExit;
    }

    long long *testp = (long long *)handle;
    TaskPool *task = (TaskPool *)testp[0];
    SocketObj *obj = (SocketObj *)task->hnd[taskId];

    printf("api_broadcast_cmd: SetPause \n");
    SetPause(obj);
    printf("api_broadcast_cmd: WaitPause2 \n");
    WaitPause2(obj);
    //usleep(500000);//500ms

    UserData udata;
    udata.data1.head.datatype = 0;
    int size = sizeof(CmdData);
    udata.data1.head.size = size;
    //
    udata.data1.status = 1;
    udata.data1.chanId = -1;//obj.id;
    udata.data1.avtype = 0;
    udata.data1.selfmode = selfmode;
    udata.data1.testmode = 0;
    udata.data1.modeId = modeId;
    udata.data1.actor = DECACTOR + 1;
    udata.data1.sessionId = sessionId;
    //
    udata.data1.chanId = -1;
    udata.data1.randId = api_create_id((1 << 30));
    udata.data1.cmdtype = (CMDType)cmdtype;
    ret = client_broadcast_get(obj, &udata);
    printf("api_broadcast_cmd: ctmp=%s, ret=%d \n", ctmp, ret);

    //udata.data1.cmdtype = (CMDType)kReg;
    //udata.data1.actor = 1;
    //ret = client_broadcast_get(obj, &udata);
    //printf("client_broadcast: kReg: ret=%d \n", ret);

    //udata.data1.cmdtype = (CMDType)kReg;
    //udata.data1.actor = DECACTOR;
    //ret = client_broadcast_get(obj, &udata);
    //printf("client_broadcast: kReg: ret=%d \n", ret);

    //udata.data1.cmdtype = (CMDType)kBye;
    //ret = client_broadcast_get(obj, &udata);
    //printf("client_broadcast: kBye: ret=%d \n", ret);
    //
    printf("api_broadcast_cmd: ClearPause \n");
    ClearPause(obj);
    printf("api_broadcast_cmd: ok \n");
    return ret;
}
HCSVC_API
int api_start_capture(char *handle, char *params0)
{
    int ret = 0;
    cJSON *json = (cJSON *)api_str2json(params0);
    int actor = GetvalueInt(json, "actor");
    int sessionId = GetvalueInt(json, "sessionId");
    int chanId = GetvalueInt(json, "chanId");
    int modeId = GetvalueInt(json, "modeId");
    int selfmode = GetvalueInt(json, "selfmode");
    int width = GetvalueInt(json, "width");
    int height = GetvalueInt(json, "height");
    int testmode = GetvalueInt(json, "testmode");
    int nettime = GetvalueInt(json, "nettime");
    int port = GetvalueInt(json, "port");
    char server_ip[64] = "";
    GetvalueStr(json, "server_ip", server_ip);
    int win_id = GetvalueInt(json, "win_id");
    int taskId = GetvalueInt(json, "taskId");
    //
    char yuvfilename[256] = "";
    char input_name[32] = "";
    char device_name[256] = "";
    char input_format[32] = "";
    GetvalueStr(json, "yuvfilename", yuvfilename);
    GetvalueStr(json, "input_name", input_name);
    GetvalueStr(json, "device_name", device_name);
    GetvalueStr(json, "input_format", input_format);
    int denoise = GetvalueInt(json, "denoise");
    int osd_enable = GetvalueInt(json, "osd_enable");
    int framerate = GetvalueInt(json, "framerate");

    long long *testp = (long long *)handle;
    TaskPool *task = (TaskPool *)testp[0];
    SocketObj *obj = (SocketObj *)task->hnd[taskId];
    char params[1024] = "";

    strcpy(params, "{\"name\":\"capture\"}");
    //CallCapture *capture = NULL;
    //CallRender *render = NULL;
    //strcpy(params, ctmp);
    json = (cJSON *)api_str2json(params);
    json = api_renew_json_int(json, "port", port);
    json = api_renew_json_str(json, "server_ip", server_ip);
#ifdef _WIN32
    json = api_renew_json_str(json, "input_name", "dshow");
    json = api_renew_json_str(json, "device_name", "USB  Live camera");
    //json = api_renew_json_str(json, "device_name", "Integrated Camera");
    json = api_renew_json_str(json, "input_format", "mjpeg");
#else
    json = api_renew_json_str(json, "input_name", "v4l2");
    //json = api_renew_json_str(json, "device_name", "/dev/video1");
    //json = api_renew_json_str(json, "input_format", "raw");//
    json = api_renew_json_str(json, "device_name", "/dev/video0");
    json = api_renew_json_str(json, "input_format", "mjpeg");
#endif
    json = api_renew_json_str(json, "input_name", input_name);
    json = api_renew_json_str(json, "device_name", device_name);
    json = api_renew_json_str(json, "input_format", input_format);

    json = api_renew_json_str(json, "pixformat", "AV_PIX_FMT_YUV420P");
    json = api_renew_json_str(json, "scale_type", "SWS_BILINEAR");
    json = api_renew_json_int(json, "cap_width", width);
    json = api_renew_json_int(json, "cap_height", height);
    json = api_renew_json_int(json, "framerate", framerate);
    json = api_renew_json_int(json, "max_buf_num", 4);
    json = api_renew_json_int(json, "width", width);
    json = api_renew_json_int(json, "height", height);
    json = api_renew_json_int(json, "ratio", 3);
    json = api_renew_json_int(json, "denoise", denoise);
    json = api_renew_json_int(json, "osd", osd_enable);
    json = api_renew_json_int(json, "orgX", 0);
    json = api_renew_json_int(json, "orgY", 0);
    json = api_renew_json_int(json, "scale", 0);
    json = api_renew_json_int(json, "color", 1);
    json = api_renew_json_int(json, "print", 0);
    char *jsonStr = api_json2str(json);
    printf("api_start_capture: jsonStr= %s \n", jsonStr);

    //taskId = 4;
    ret = api_taskpool_addtask(handle, jsonStr, video_capture, taskId);
    obj = (SocketObj *)task->hnd[taskId];
    //CallCapture *capture = (CallCapture *)obj->session;
    printf("api_start_capture: obj= %x \n", obj);
    if(obj)
    {
        ret = WaitCapture(obj, 1, kIsVideo);
        printf("api_start_capture: ret= %d \n", ret);
        if(!ret)
        {
            pthread_mutex_destroy(&obj->status_lock);
        }
    }
    return taskId;
}
HCSVC_API
int api_start_render(char *handle, char *params0)
{
    int ret = 0;
    cJSON *json = (cJSON *)api_str2json(params0);
    int actor = GetvalueInt(json, "actor");
    int sessionId = GetvalueInt(json, "sessionId");
    int chanId = GetvalueInt(json, "chanId");
    int modeId = GetvalueInt(json, "modeId");
    int selfmode = GetvalueInt(json, "selfmode");
    int width = GetvalueInt(json, "width");
    int height = GetvalueInt(json, "height");
    int screen_width = GetvalueInt(json, "screen_width");
    int screen_height = GetvalueInt(json, "screen_height");
    int testmode = GetvalueInt(json, "testmode");
    int nettime = GetvalueInt(json, "nettime");
    int port = GetvalueInt(json, "port");
    char server_ip[64] = "";
    GetvalueStr(json, "server_ip", server_ip);
    int win_id = GetvalueInt(json, "win_id");
    int taskId = GetvalueInt(json, "taskId");
    int osd_enable = GetvalueInt(json, "osd_enable");

    long long *testp = (long long *)handle;
    TaskPool *task = (TaskPool *)testp[0];
    SocketObj *obj = (SocketObj *)task->hnd[taskId];
    char params[1024] = "";

    strcpy(params, "{\"name\":\"render\"}");
    json = (cJSON *)api_str2json(params);
    json = api_renew_json_int(json, "port", port);
    json = api_renew_json_str(json, "server_ip", server_ip);
    json = api_renew_json_int(json, "screen_w", screen_width);
    json = api_renew_json_int(json, "screen_h", screen_height);
    json = api_renew_json_int(json, "pixel_w", width);
    json = api_renew_json_int(json, "pixel_h", height);
    json = api_renew_json_int(json, "bpp", 12);
    json = api_renew_json_int(json, "wait_time", 0);
    json = api_renew_json_str(json, "pixformat", "SDL_PIXELFORMAT_IYUV");
    json = api_renew_json_int(json, "win_id", win_id);
    json = api_renew_json_int(json, "osd", osd_enable);
    json = api_renew_json_int(json, "orgX", 0);
    json = api_renew_json_int(json, "orgY", 0);
    json = api_renew_json_int(json, "scale", 0);
    json = api_renew_json_int(json, "color", 4);
    json = api_renew_json_int(json, "print", 0);
    char *jsonStr = api_json2str(json);
    printf("api_start_render: jsonStr= %s \n", jsonStr);

    //taskId = 5;
    ret = api_taskpool_addtask(handle, jsonStr, video_render, taskId);
    obj = (SocketObj *)task->hnd[taskId];
    //render = (CallRender *)obj->session;
    printf("api_start_render: obj= %x \n", obj);
    if(obj)
    {
        WaitRender(obj);
    }
    return taskId;
}
HCSVC_API
int api_start_video(char *handle, char *params0)
{
    int ret = 0;
    cJSON *json = (cJSON *)api_str2json(params0);
    int actor = GetvalueInt(json, "actor");
    int sessionId = GetvalueInt(json, "sessionId");
    int chanId = GetvalueInt(json, "chanId");
    int modeId = GetvalueInt(json, "modeId");
    int selfmode = GetvalueInt(json, "selfmode");
    int width = GetvalueInt(json, "width");
    int height = GetvalueInt(json, "height");
    int screen_width = GetvalueInt(json, "screen_width");
    int screen_height = GetvalueInt(json, "screen_height");
    int enable_netack = GetvalueInt(json, "enable_netack");
    int testmode = GetvalueInt(json, "testmode");
    int nettime = GetvalueInt(json, "nettime");
    int port = GetvalueInt(json, "port");
    char server_ip[64] = "";
    GetvalueStr(json, "server_ip", server_ip);
    int idx = GetvalueInt(json, "idx");
    int win_id = GetvalueInt(json, "win_id");
    int isEncoder = GetvalueInt(json, "isEncoder");
    int taskId = GetvalueInt(json, "taskId");
    //
    int bitrate = GetvalueInt(json, "bitrate");
    int mtu_size = GetvalueInt(json, "mtu_size");
    int fecenable = GetvalueInt(json, "fecenable");
    int fec_level = GetvalueInt(json, "fec_level");
    int buffer_shift = GetvalueInt(json, "buffer_shift");
    int denoise = GetvalueInt(json, "denoise");
    int osd_enable = GetvalueInt(json, "osd_enable");
    int framerate = GetvalueInt(json, "framerate");
    int refs = GetvalueInt(json, "refs");
    float bwthreshold = GetvalueFloat(json, "bwthreshold");
    float lossrate = GetvalueFloat(json, "lossrate");
    printf("api_start_video: params0=%s \n", params0);
    printf("api_start_video: bwthreshold=%2.3f \n", bwthreshold);
    char scodec[32] = "";
    GetvalueStr(json, "scodec", scodec);
    char yuvfilename[256] = "";
    GetvalueStr(json, "yuvfilename", yuvfilename);

    long long *testp = (long long *)handle;
    TaskPool *task = (TaskPool *)testp[0];
    SocketObj *obj = (SocketObj *)task->hnd[taskId];
    char params[1024] = "";

    strcpy(params, "{\"name\":\"codec\"}");
    json = (cJSON *)api_str2json(params);
    //json = api_renew_json_str(json, "yuvfp", "");
    json = api_renew_json_int(json, "idx", idx);
    json = api_renew_json_int(json, "port", port);
    json = api_renew_json_str(json, "server_ip", server_ip);
    json = api_renew_json_int(json, "actor", actor);
    json = api_renew_json_int(json, "sessionId", sessionId);
    json = api_renew_json_int(json, "chanId", chanId);
    json = api_renew_json_int(json, "modeId", modeId);
    json = api_renew_json_int(json, "avtype", 0);
    json = api_renew_json_int(json, "selfmode", selfmode);
    json = api_renew_json_int(json, "testmode", testmode);
    json = api_renew_json_int(json, "nettime", nettime);
    json = api_renew_json_int(json, "width", width);
    json = api_renew_json_int(json, "height", height);
    json = api_renew_json_int(json, "screen_width", screen_width);
    json = api_renew_json_int(json, "screen_height", screen_height);
    json = api_renew_json_int(json, "enable_netack", enable_netack);
    json = api_renew_json_int(json, "status", 1);

    json = api_renew_json_str(json, "yuvfilename", yuvfilename);
    json = api_renew_json_str(json, "scodec", scodec);
    json = api_renew_json_int(json, "bitrate", bitrate);
    json = api_renew_json_int(json, "mtu_size", mtu_size);
    json = api_renew_json_int(json, "fecenable", fecenable);
    json = api_renew_json_int(json, "fec_level", fec_level);
    json = api_renew_json_int(json, "buffer_shift", buffer_shift);
    json = api_renew_json_int(json, "denoise", denoise);
    json = api_renew_json_int(json, "osd_enable", osd_enable);
    json = api_renew_json_int(json, "framerate", framerate);
    json = api_renew_json_int(json, "refs", refs);
    json = api_renew_json_float(json, "bwthreshold", bwthreshold);
    json = api_renew_json_float(json, "lossrate", lossrate);
    char *jsonStr = api_json2str(json);
    if(isEncoder)
    {
        ret = api_taskpool_addtask(handle, jsonStr, video_encode, taskId);//client_main
    }
    else{
        ret = api_taskpool_addtask(handle, jsonStr, video_decode, taskId);
    }

    return taskId;
}
HCSVC_API
int api_set4codec_video(char *handle, char *handle2, char *params)
{
    int ret = 0;
    cJSON *json = (cJSON *)api_str2json(params);
    int codecTaskId = GetvalueInt(json, "codecTaskId");
    int captureTaskId = GetvalueInt(json, "captureTaskId");
    int renderTaskId = GetvalueInt(json, "renderTaskId");
    int isEncoder = GetvalueInt(json, "isEncoder");

    long long *testp = (long long *)handle;
    TaskPool *task = (TaskPool *)testp[0];
    SocketObj *obj = (SocketObj *)task->hnd[codecTaskId];

    long long *testp2 = (long long *)handle2;
    TaskPool *task2 = (TaskPool *)testp2[0];
    SocketObj *obj2 = (SocketObj *)task2->hnd[codecTaskId];

    CallCapture *capture = NULL;
    CallRender *render = NULL;
    int capture_status = 0;

    int taskId = captureTaskId;
    if(taskId >= 0)
    {
        obj = (SocketObj *)task->hnd[taskId];
        printf("api_set4codec_video: captureTaskId= %d \n", captureTaskId);
        if(obj)
        {
            printf("api_set4codec_video: obj= %x \n", obj);
            capture_status = obj->status;
            SessionObj *session = (SessionObj *)obj->session;
            if(session)
            {
                capture = (CallCapture *)session->capture;
                printf("api_set4codec_video: capture= %x \n", capture);
            }
        }
    }
    else{
        printf("api_set4codec_video: captureTaskId= %d \n", captureTaskId);
    }

    taskId = renderTaskId;
    obj = (SocketObj *)task->hnd[taskId];
    printf("api_set4codec_video: renderTaskId= %d \n", renderTaskId);
    if(obj)
    {
        printf("api_set4codec_video: obj= %x \n", obj);
        SessionObj *session = (SessionObj *)obj->session;
        if(session)
        {
            render = (CallRender *)session->render;
            printf("api_set4codec_video: render= %x \n", render);
        }
    }

    taskId = codecTaskId;
    obj2 = (SocketObj *)task2->hnd[taskId];
    printf("api_set4codec_video: codecTaskId= %d \n", codecTaskId);
    if(obj2)
    {
        if(isEncoder)
        {
            ret = video_set_capture(obj2, capture, capture_status);
            if(ret < 0)
            {
                printf("api_set4codec_video: fail video_set_capture taskId= %d \n", taskId);
            }
        }

        ret = video_set_render(obj2, render);
        if(ret < 0)
        {
            printf("api_set4codec_video: fail video_set_render taskId= %d \n", taskId);
        }
        video_set_netinfo(obj2, globalNetInfoObj);
    }
    return ret;
}
HCSVC_API
int api_setstream2video(char *handle, char *handle2, char *params)
{
    int ret = 0;
    cJSON *json = (cJSON *)api_str2json(params);
    int codecTaskId = GetvalueInt(json, "codecTaskId");
    int captureTaskId = GetvalueInt(json, "captureTaskId");
    int renderTaskId = GetvalueInt(json, "renderTaskId");
    int isEncoder = GetvalueInt(json, "isEncoder");

    long long *testp = (long long *)handle;
    TaskPool *task = (TaskPool *)testp[0];
    SocketObj *obj = (SocketObj *)task->hnd[codecTaskId];

    int taskId = codecTaskId;
    obj = (SocketObj *)task->hnd[taskId];
    printf("api_setstream2video: codecTaskId= %d, isEncoder=%d \n", codecTaskId, isEncoder);
    if(obj)
    {
        video_set_stream(obj, handle2, isEncoder);
    }
    return ret;
}
HCSVC_API
int api_setreadstream2video(char *handle, char *handle2, char *params)
{
    int ret = 0;
    cJSON *json = (cJSON *)api_str2json(params);
    int codecTaskId = GetvalueInt(json, "codecTaskId");
    int rstreamTaskId = GetvalueInt(json, "rstreamTaskId");

    long long *testp = (long long *)handle;
    TaskPool *task = (TaskPool *)testp[0];
    SocketObj *obj = (SocketObj *)task->hnd[codecTaskId];

    SocketObj *obj2 = (SocketObj *)task->hnd[rstreamTaskId];
    printf("api_setreadstream2video: codecTaskId= %d, rstreamTaskId=%d \n", codecTaskId, rstreamTaskId);
    if(obj && obj2)
    {
        SessionObj *session = (SessionObj *)obj2->session;
        video_set_readstream(obj, session->handle);
    }
    return ret;
}
HCSVC_API
int api_start_acapture(char *handle, char *params0)
{
    int ret = 0;
    cJSON *json = (cJSON *)api_str2json(params0);
    int actor = GetvalueInt(json, "actor");
    int sessionId = GetvalueInt(json, "sessionId");
    int chanId = GetvalueInt(json, "chanId");
    int modeId = GetvalueInt(json, "modeId");
    int selfmode = GetvalueInt(json, "selfmode");
    int width = GetvalueInt(json, "width");
    int height = GetvalueInt(json, "height");
    int testmode = GetvalueInt(json, "testmode");
    int nettime = GetvalueInt(json, "nettime");
    int port = GetvalueInt(json, "port");
    char server_ip[64] = "";
    GetvalueStr(json, "server_ip", server_ip);
    int idx = GetvalueInt(json, "idx");
    int win_id = GetvalueInt(json, "win_id");
    int taskId = GetvalueInt(json, "taskId");
    //
    char input_name[32] = "";
    char device_name[256] = "";
    GetvalueStr(json, "input_name", input_name);
    GetvalueStr(json, "device_name", device_name);
    int denoise = GetvalueInt(json, "denoise");
    int out_nb_samples = GetvalueInt(json, "out_nb_samples");
    int in_channels = GetvalueInt(json, "in_channels");
    int in_sample_rate = GetvalueInt(json, "in_sample_rate");
    int out_channels = in_channels;
    int out_sample_rate = in_sample_rate;

    long long *testp = (long long *)handle;
    TaskPool *task = (TaskPool *)testp[0];
    SocketObj *obj = (SocketObj *)task->hnd[taskId];
    char params[1024] = "";

    strcpy(params, "{\"name\":\"acapture\"}");

    json = (cJSON *)api_str2json(params);
    json = api_renew_json_int(json, "port", port);
    json = api_renew_json_str(json, "server_ip", server_ip);
#ifdef _WIN32
    json = api_renew_json_str(json, "input_name", "dshow");
    json = api_renew_json_str(json, "device_name", "micro (USB Live Camera audio)");
#else
    json = api_renew_json_str(json, "input_name", "alsa");
    json = api_renew_json_str(json, "device_name", "default");
#endif
    json = api_renew_json_str(json, "input_name", input_name);
    json = api_renew_json_str(json, "device_name", device_name);

    json = api_renew_json_str(json, "in_sample_fmt", "AV_SAMPLE_FMT_S16");//
    json = api_renew_json_str(json, "out_sample_fmt", "AV_SAMPLE_FMT_S16");
    json = api_renew_json_str(json, "out_channel_layout", "AV_CH_LAYOUT_STEREO");
    json = api_renew_json_str(json, "filename", "./dukou_ref3.wav");
    json = api_renew_json_str(json, "pcmfile", "");
    json = api_renew_json_str(json, "capfile", "");
    json = api_renew_json_int(json, "in_channels", in_channels);
    json = api_renew_json_int(json, "out_channels", out_channels);
    json = api_renew_json_int(json, "in_sample_rate", in_sample_rate);
    json = api_renew_json_int(json, "out_sample_rate", out_sample_rate);
    json = api_renew_json_int(json, "out_nb_samples", 1024);//out_nb_samples
    json = api_renew_json_int(json, "out_buffer_size", 4096);
    json = api_renew_json_int(json, "frame_size", 4096);
    json = api_renew_json_int(json, "max_buf_num", 8);
    json = api_renew_json_int(json, "datatype", 2);
    json = api_renew_json_int(json, "process", 1);
    json = api_renew_json_int(json, "print", 0);
    char *jsonStr = api_json2str(json);
    printf("api_start_acapture: jsonStr= %s \n", jsonStr);

    ret = api_taskpool_addtask(handle, jsonStr, audio_capture, taskId);

    obj = (SocketObj *)task->hnd[taskId];
    if(obj)
    {
        ret = WaitCapture(obj, 1, kIsAudio);
        printf("api_start_acapture: ret= %d \n", ret);
        if(!ret)
        {
            pthread_mutex_destroy(&obj->status_lock);
        }
    }
    return taskId;
}
HCSVC_API
int api_start_player(char *handle, char *params0)
{
    int ret = 0;
    cJSON *json = (cJSON *)api_str2json(params0);
    int actor = GetvalueInt(json, "actor");
    int sessionId = GetvalueInt(json, "sessionId");
    int chanId = GetvalueInt(json, "chanId");
    int modeId = GetvalueInt(json, "modeId");
    int selfmode = GetvalueInt(json, "selfmode");
    int width = GetvalueInt(json, "width");
    int height = GetvalueInt(json, "height");
    int testmode = GetvalueInt(json, "testmode");
    int nettime = GetvalueInt(json, "nettime");
    int port = GetvalueInt(json, "port");
    char server_ip[64] = "";
    GetvalueStr(json, "server_ip", server_ip);
    int idx = GetvalueInt(json, "idx");
    int win_id = GetvalueInt(json, "win_id");
    int taskId = GetvalueInt(json, "taskId");

    long long *testp = (long long *)handle;
    TaskPool *task = (TaskPool *)testp[0];
    SocketObj *obj = (SocketObj *)task->hnd[taskId];
    char params[1024] = "";

    strcpy(params, "{\"name\":\"player\"}");

    strcpy(params, "{\"name\":\"player\"}");
    json = (cJSON *)api_str2json(params);
    json = api_renew_json_int(json, "port", port);
    json = api_renew_json_str(json, "server_ip", server_ip);
    json = api_renew_json_int(json, "mix_num", 2);
    json = api_renew_json_str(json, "format", "AUDIO_S16SYS");
    json = api_renew_json_int(json, "silence", 0);
    json = api_renew_json_int(json, "out_channels", 2);
    json = api_renew_json_int(json, "out_nb_samples", 2048);
    json = api_renew_json_str(json, "out_sample_fmt", "AV_SAMPLE_FMT_S16");
    json = api_renew_json_int(json, "out_sample_rate", 48000);
    json = api_renew_json_str(json, "out_channel_layout", "AV_CH_LAYOUT_STEREO");
    json = api_renew_json_int(json, "out_buffer_size", 8192);
    json = api_renew_json_int(json, "frame_size", 8192);
    json = api_renew_json_str(json, "pcmfile", "");
    json = api_renew_json_int(json, "sdl_status", 1);//0：纯音频模式；1：音视频
    json = api_renew_json_int(json, "print", 0);
    char *jsonStr = api_json2str(json);
    printf("api_start_player: jsonStr= %s \n", jsonStr);

    ret = api_taskpool_addtask(handle, jsonStr, audio_player, taskId);

    obj = (SocketObj *)task->hnd[taskId];
    if(obj)
    {
        WaitRender(obj);//mean waitplayer
    }
    return taskId;
}
HCSVC_API
int api_start_audio(char *handle, char *params0)
{
    int ret = 0;
    cJSON *json = (cJSON *)api_str2json(params0);
    int actor = GetvalueInt(json, "actor");
    int sessionId = GetvalueInt(json, "sessionId");
    int chanId = GetvalueInt(json, "chanId");
    int modeId = GetvalueInt(json, "modeId");
    int selfmode = GetvalueInt(json, "selfmode");
    //int width = GetvalueInt(json, "width");
    //int height = GetvalueInt(json, "height");
    int testmode = GetvalueInt(json, "testmode");
    int nettime = GetvalueInt(json, "nettime");
    int port = GetvalueInt(json, "port");
    char server_ip[64] = "";
    GetvalueStr(json, "server_ip", server_ip);
    int idx = GetvalueInt(json, "idx");
    int win_id = GetvalueInt(json, "win_id");
    int isEncoder = GetvalueInt(json, "isEncoder");
    int taskId = GetvalueInt(json, "taskId");
    //
    int bitrate = GetvalueInt(json, "bitrate");
    int mtu_size = GetvalueInt(json, "mtu_size");
    int fecenable = GetvalueInt(json, "fecenable");
    //int fec_level = GetvalueInt(json, "fec_level");
    int buffer_shift = GetvalueInt(json, "buffer_shift");
    int denoise = GetvalueInt(json, "denoise");
    int osd_enable = GetvalueInt(json, "osd_enable");
    int in_channels = GetvalueInt(json, "in_channels");
    int in_sample_rate = GetvalueInt(json, "in_sample_rate");
    int out_nb_samples = GetvalueInt(json, "out_nb_samples");
    int enable_netack = GetvalueInt(json, "enable_netack");
    float lossrate = GetvalueFloat(json, "lossrate");
    char scodec[32] = "";
    GetvalueStr(json, "scodec", scodec);
    char yuvfilename[256] = "";
    GetvalueStr(json, "yuvfilename", yuvfilename);

    long long *testp = (long long *)handle;
    TaskPool *task = (TaskPool *)testp[0];
    SocketObj *obj = (SocketObj *)task->hnd[taskId];
    char params[1024] = "";

    strcpy(params, "{\"name\":\"acodec\"}");
    json = (cJSON *)api_str2json(params);
    json = api_renew_json_str(json, "yuvfilename", yuvfilename);
    json = api_renew_json_int(json, "port", port);
    json = api_renew_json_str(json, "server_ip", server_ip);
    //json = api_renew_json_str(json, "yuvfp", "");
    json = api_renew_json_int(json, "actor", actor);
    json = api_renew_json_int(json, "sessionId", sessionId);
    json = api_renew_json_int(json, "chanId", chanId);
    json = api_renew_json_int(json, "modeId", modeId);
    json = api_renew_json_int(json, "avtype", 1);
    json = api_renew_json_int(json, "selfmode", selfmode);
    json = api_renew_json_int(json, "testmode", testmode);
    json = api_renew_json_int(json, "nettime", nettime);
    json = api_renew_json_int(json, "enable_netack", enable_netack);
    json = api_renew_json_int(json, "status", 1);
    json = api_renew_json_float(json, "lossrate", lossrate);
    char *jsonStr = api_json2str(json);
    if(isEncoder)
    {
        ret = api_taskpool_addtask(handle, jsonStr, audio_encode, taskId);//client_main
    }
    else{
        ret = api_taskpool_addtask(handle, jsonStr, audio_decode, taskId);
    }
    return taskId;
}
HCSVC_API
int api_set4codec_audio(char *handle, char *handle2, char *params)
{
    int ret = 0;
    cJSON *json = (cJSON *)api_str2json(params);
    int codecTaskId = GetvalueInt(json, "codecTaskId");
    int captureTaskId = GetvalueInt(json, "captureTaskId");
    int renderTaskId = GetvalueInt(json, "renderTaskId");
    int isEncoder = GetvalueInt(json, "isEncoder");

    long long *testp = (long long *)handle;
    TaskPool *task = (TaskPool *)testp[0];
    SocketObj *obj = (SocketObj *)task->hnd[codecTaskId];

    long long *testp2 = (long long *)handle2;
    TaskPool *task2 = (TaskPool *)testp2[0];
    SocketObj *obj2 = (SocketObj *)task2->hnd[codecTaskId];

    CallAudioCapture * acapture = NULL;
    CallPlayer *player = NULL;
    int capture_status = 0;

    int taskId = captureTaskId;
    if(taskId >= 0)
    {
        obj = (SocketObj *)task->hnd[taskId];
        if(obj)
        {
            printf("api_set4codec_audio: obj= %x \n", obj);
            capture_status = obj->status;
            SessionObj *session = (SessionObj *)obj->session;
            if(session)
            {
                acapture = (CallCapture *)session->capture;
                printf("api_set4codec_audio: acapture= %x \n", acapture);
            }
        }
    }

    taskId = renderTaskId;
    obj = (SocketObj *)task->hnd[taskId];
    if(obj)
    {
        printf("api_set4codec_audio: obj= %x \n", obj);
        SessionObj *session = (SessionObj *)obj->session;
        if(session)
        {
            player = (CallPlayer *)session->render;
            printf("api_set4codec_audio: player= %x \n", player);
        }
    }

    taskId = codecTaskId;
    obj2 = (SocketObj *)task2->hnd[taskId];
    if(obj2)
    {
        if(isEncoder)
        {
            ret = audio_set_capture(obj2, acapture, capture_status);
            if(ret < 0)
            {
                printf("api_set4codec_audio: fail audio_set_capture taskId= %d \n", taskId);
            }
            //
            ret = audio_set_player(obj2, player);
            if(ret < 0)
            {
                printf("api_set4codec_audio: fail audio_set_player taskId= %d \n", taskId);
            }
        }
        else{
            ret = audio_set_player(obj2, player);
            if(ret < 0)
            {
                printf("api_set4codec_audio: fail audio_set_player taskId= %d \n", taskId);
            }
        }
        //
        audio_set_netinfo(obj2, globalNetInfoObj);
    }

    return ret;
}
HCSVC_API
int api_setstream2audio(char *handle, char *handle2, char *params)
{
    int ret = 0;
    cJSON *json = (cJSON *)api_str2json(params);
    int codecTaskId = GetvalueInt(json, "codecTaskId");
    int captureTaskId = GetvalueInt(json, "captureTaskId");
    int renderTaskId = GetvalueInt(json, "renderTaskId");
    int isEncoder = GetvalueInt(json, "isEncoder");

    long long *testp = (long long *)handle;
    TaskPool *task = (TaskPool *)testp[0];
    SocketObj *obj = (SocketObj *)task->hnd[codecTaskId];

    int taskId = codecTaskId;
    obj = (SocketObj *)task->hnd[taskId];
    printf("api_setstream2audio: codecTaskId= %d, isEncoder=%d \n", codecTaskId, isEncoder);
    if(obj)
    {
        audio_set_stream(obj, handle2, isEncoder);
    }
    return ret;
}
HCSVC_API
int api_setreadstream2audio(char *handle, char *handle2, char *params)
{
    int ret = 0;
    cJSON *json = (cJSON *)api_str2json(params);
    int codecTaskId = GetvalueInt(json, "codecTaskId");
    int rstreamTaskId = GetvalueInt(json, "rstreamTaskId");

    long long *testp = (long long *)handle;
    TaskPool *task = (TaskPool *)testp[0];
    SocketObj *obj = (SocketObj *)task->hnd[codecTaskId];

    SocketObj *obj2 = (SocketObj *)task->hnd[rstreamTaskId];
    printf("api_setreadstream2audio: codecTaskId= %d, rstreamTaskId=%d \n", codecTaskId, rstreamTaskId);
    if(obj && obj2)
    {
        SessionObj *session = (SessionObj *)obj2->session;
        audio_set_readstream(obj, session->handle);
    }
    return ret;
}
HCSVC_API
int api_start_stream(char *handle, char *params0)
{
    int ret = 0;
    printf("api_start_stream:  \n");
    cJSON *json = (cJSON *)api_str2json(params0);

    int taskId = GetvalueInt(json, "taskId");
    char infile[256] = "";
    GetvalueStr(json, "infile", infile);
    char scodec[32] = "";
    GetvalueStr(json, "scodec", scodec);
    int width = GetvalueInt(json, "width");
    int height = GetvalueInt(json, "height");
    int bitrate = GetvalueInt(json, "bitrate");
    int mtu_size = GetvalueInt(json, "mtu_size");
    int denoise = GetvalueInt(json, "denoise");
    int osd_enable = GetvalueInt(json, "osd_enable");
    int framerate = GetvalueInt(json, "framerate");
    int refs = GetvalueInt(json, "refs");

    long long *testp = (long long *)handle;
    TaskPool *task = (TaskPool *)testp[0];
    SocketObj *obj = (SocketObj *)task->hnd[taskId];
    char params[1024] = "";

    strcpy(params, "{\"name\":\"readstream\"}");
    json = (cJSON *)api_str2json(params);
    json = api_renew_json_str(json, "infile", infile);
    json = api_renew_json_int(json, "width", width);
    json = api_renew_json_int(json, "height", height);
    json = api_renew_json_str(json, "scodec", scodec);
    json = api_renew_json_int(json, "bitrate", bitrate);
    json = api_renew_json_int(json, "mtu_size", mtu_size);
    json = api_renew_json_int(json, "denoise", denoise);
    json = api_renew_json_int(json, "osd_enable", osd_enable);
    json = api_renew_json_int(json, "framerate", framerate);
    json = api_renew_json_int(json, "refs", refs);
    char *jsonStr = api_json2str(json);
    ret = api_taskpool_addtask(handle, jsonStr, read_stream, taskId);

    obj = (SocketObj *)task->hnd[taskId];
    printf("api_start_stream: obj=%x \n", obj);
    if(obj)
    {
        WaitAVStream(obj);
        SessionObj *session = (SessionObj *)obj->session;
        if(session)
        {
            printf("api_start_stream: session->handle=%s \n", session->handle);
            long long *testp = (long long *)session->handle;
            AVStreamObj *stream = (AVStreamObj *)testp[0];
            printf("api_start_stream: stream=%x \n",stream);
        }
    }

    printf("api_start_stream: ok, taskId=%d \n", taskId);
    return taskId;
}

HCSVC_API
int api_start_mcu(char *handle, char *params0)
{
    int ret = 0;
    printf("api_start_mcu:  \n");
    cJSON *json = (cJSON *)api_str2json(params0);

    int taskId = GetvalueInt(json, "taskId");
    char outfile[256] = "";
    GetvalueStr(json, "outfile", outfile);
    char scodec[32] = "";
    GetvalueStr(json, "scodec", scodec);
    int width = GetvalueInt(json, "width");
    int height = GetvalueInt(json, "height");
    int bitrate = GetvalueInt(json, "bitrate");
    int mtu_size = GetvalueInt(json, "mtu_size");
    int denoise = GetvalueInt(json, "denoise");
    int osd_enable = GetvalueInt(json, "osd_enable");
    int framerate = GetvalueInt(json, "framerate");
    int refs = GetvalueInt(json, "refs");

    long long *testp = (long long *)handle;
    TaskPool *task = (TaskPool *)testp[0];
    SocketObj *obj = (SocketObj *)task->hnd[taskId];
    char params[1024] = "";

    strcpy(params, "{\"name\":\"mcu\"}");
    json = (cJSON *)api_str2json(params);
    json = api_renew_json_str(json, "outfile", outfile);
    json = api_renew_json_int(json, "width", width);
    json = api_renew_json_int(json, "height", height);
    json = api_renew_json_str(json, "scodec", scodec);
    json = api_renew_json_int(json, "bitrate", bitrate);
    json = api_renew_json_int(json, "mtu_size", mtu_size);
    json = api_renew_json_int(json, "denoise", denoise);
    json = api_renew_json_int(json, "osd_enable", osd_enable);
    json = api_renew_json_int(json, "framerate", framerate);
    json = api_renew_json_int(json, "refs", refs);
    char *jsonStr = api_json2str(json);
    ret = api_taskpool_addtask(handle, jsonStr, mcu_task, taskId);
    printf("api_start_mcu: ok, taskId=%d \n", taskId);
    return taskId;
}
HCSVC_API
int api_set4mcu(char *handle, char *handle2, char *params)
{
    int ret = 0;
    printf("api_set4mcu:  \n");
    cJSON *json = (cJSON *)api_str2json(params);
    int mcuTaskId = GetvalueInt(json, "mcuTaskId");
    int renderTaskId = GetvalueInt(json, "renderTaskId");
    int playerTaskId = GetvalueInt(json, "playerTaskId");

    long long *testp = (long long *)handle;
    TaskPool *task = (TaskPool *)testp[0];
    SocketObj *obj = (SocketObj *)task->hnd[mcuTaskId];

    long long *testp2 = (long long *)handle2;
    TaskPool *task2 = (TaskPool *)testp2[0];
    SocketObj *obj2 = (SocketObj *)task2->hnd[renderTaskId];

    long long *testp3 = (long long *)handle2;
    TaskPool *task3 = (TaskPool *)testp3[0];
    SocketObj *obj3 = (SocketObj *)task3->hnd[playerTaskId];

    McuObj *mcu = NULL;

    int taskId = mcuTaskId;
    if(taskId >= 0)
    {
        obj = (SocketObj *)task->hnd[taskId];
        printf("api_set4mcu: obj=%x \n", obj);
        if(obj)
        {
            SessionObj *session = (SessionObj *)obj->session;
            mcu = (McuObj *)session->mcu;
        }
    }
    printf("api_set4mcu: mcu=%x \n", mcu);

    taskId = renderTaskId;
    obj2 = (SocketObj *)task2->hnd[taskId];
    printf("api_set4mcu: obj2=%x \n", obj2);
    if(obj2 && mcu)
    {
        ret = set_mcu2render(obj2, mcu);
    }

    taskId = playerTaskId;
    obj3 = (SocketObj *)task2->hnd[taskId];
    printf("api_set4mcu: obj3=%x \n", obj3);
    if(obj3 && mcu)
    {
        ret = set_mcu2player(obj3, mcu);
    }
    printf("api_set4mcu: ok, ret=%d \n", ret);
    return ret;
}
int StopTask(SocketObj *sock)
{
    int ret = 0;
    if(sock->status < 0)
    {
        return ret;
    }
    StopBroadCast(sock);
    StopSend(sock);
    StopRecv(sock);
    //
    SessionObj *session = NULL;
    pthread_mutex_lock(&sock->status_lock);
    session = (SessionObj *)sock->session;
    pthread_mutex_unlock(&sock->status_lock);
    if(session)
    {
        if(session->capture)
        {
            CallCapture *obj = (CallCapture *)session->capture;
            pthread_mutex_lock(&obj->status_lock);
            if(obj->send_status > 0)
            {
                obj->send_status = 0;
            }
            if(obj->recv_status > 0)
            {
                obj->recv_status = 0;
            }
            pthread_mutex_unlock(&obj->status_lock);
        }
        if(session->render)
        {
            CallRender *obj = (CallRender *)session->render;
            pthread_mutex_lock(&obj->status_lock);
            if(obj->send_status > 0)
            {
                obj->send_status = 0;
            }
            if(obj->recv_status > 0)
            {
                obj->recv_status = 0;
            }
            pthread_mutex_unlock(&obj->status_lock);
            printf("StopTask: obj->recv_status=%d \n", obj->recv_status);
        }
        printf("StopTask: session->render=%x \n", session->render);
        if(session->mcu)
        {
            McuObj *obj = (McuObj *)session->mcu;
            pthread_mutex_lock(&obj->status_lock);
            if(obj->video_status > 0)
            {
                obj->video_status = 0;
            }
            if(obj->audio_status > 0)
            {
                obj->audio_status = 0;
            }
            pthread_mutex_unlock(&obj->status_lock);
        }
        if(strcmp(session->handle, ""))
        {
            api_avstream_status(session->handle, 2);
        }
    }
    printf("StopTask: sock->id=%d \n", sock->id);

    return ret;
}
HCSVC_API
int api_stop_task(char *handle, int taskId)
{
    int ret = 0;
    if(taskId >= 0)
    {
        long long *testp = (long long *)handle;
        TaskPool *task = (TaskPool *)testp[0];
        SocketObj *obj = (SocketObj *)task->hnd[taskId];
        char name[32] = "";
        cJSON * json = (cJSON *)api_str2json(obj->params);
        GetvalueStr(json, "name", name);
        printf("api_stop_task: name=%s \n",  name);

        if(obj)
        {
            StopTask(obj);
        }
    }
    else if(globalNetInfoObj)
    {
        CloseNetInfoHnd(globalNetInfoObj);
        globalNetInfoObj = NULL;
        //if(!globalNetInfoObj)
        //{
        //    CreateNetInfoHnd(&globalNetInfoObj);
        //}
    }

    return ret;
}

HCSVC_API
int api_pool_stop(char *handle)
{
    int ret = 0;
    printf("api_pool_stop: start \n");
    api_taskpool_close(handle);

    if(globalNetInfoObj)
    {
        CloseNetInfoHnd(globalNetInfoObj);
        globalNetInfoObj = NULL;
    }
    printf("api_pool_stop: over \n");
    return ret;
}
//===================================================================================
HCSVC_API
int pool_test(int port, int is_video)
{
    int ret = 0;
    char handle[8] = "";
    char params[2048] = "";
    char ctmp[128] = "{\"ratio\":3}";
    cJSON *json = NULL;
    char *jsonStr = NULL;
    int modeId = 2;//3;//2;//3;
    int selfmode = 1;//
    //int port = 10088;
    char *server_ip = "127.0.0.1";

    api_taskpool_init(handle, params);
    long long *testp = (long long *)handle;
    TaskPool *task = (TaskPool *)testp[0];

    strcpy(params, "{\"name\":\"media_server\"}");
    json = (cJSON *)api_str2json(params);
    json = api_renew_json_int(json, "port", port);
    json = api_renew_json_str(json, "server_ip", server_ip);

    jsonStr = api_json2str(json);
    int taskId = 0;
    ret = api_taskpool_addtask(handle, jsonStr, server_main, taskId);

    sleep(1);
#if 1
    strcpy(params, "{\"name\":\"client_broadcast\"}");
    json = (cJSON *)api_str2json(params);
    json = api_renew_json_int(json, "port", port);
    json = api_renew_json_str(json, "server_ip", server_ip);

    jsonStr = api_json2str(json);
    taskId = 1;
    ret = api_taskpool_addtask(handle, jsonStr, client_broadcast, taskId);
#endif
#if 1
    sleep(1);
    SocketObj *obj = (SocketObj *)task->hnd[taskId];
    broadcast_test(obj, selfmode, modeId);
#endif
    //NetInfoObj *globalNetInfoObj = NULL;
    CreateNetInfoHnd(&globalNetInfoObj);
    //api_ffmpeg_register();

//==========================video===================================
if(is_video)
{
    taskId = 2;
    strcpy(params, "{\"idx\":0}");
    json = (cJSON *)api_str2json(params);
    //json = api_renew_json_str(json, "yuvfp", "");
    json = api_renew_json_int(json, "port", port);
    json = api_renew_json_str(json, "server_ip", server_ip);
    json = api_renew_json_int(json, "actor", 1);
    json = api_renew_json_int(json, "sessionId", 0);
    json = api_renew_json_int(json, "chanId", 0);
    json = api_renew_json_int(json, "modeId", modeId);
    json = api_renew_json_int(json, "avtype", 0);
    json = api_renew_json_int(json, "selfmode", selfmode);
    json = api_renew_json_int(json, "testmode", 0);
    json = api_renew_json_int(json, "nettime", 0);
    json = api_renew_json_int(json, "width", DEFAULT_WIDTH);
    json = api_renew_json_int(json, "height", DEFAULT_HEIGHT);
    json = api_renew_json_int(json, "status", 1);
    jsonStr = api_json2str(json);
    ret = api_taskpool_addtask(handle, jsonStr, video_encode, taskId);//client_main

    //sleep(1);//???

    taskId = 3;
    strcpy(params, "{\"idx\":0}");
    json = (cJSON *)api_str2json(params);
    json = api_renew_json_int(json, "port", port);
    json = api_renew_json_str(json, "server_ip", server_ip);
    json = api_renew_json_int(json, "actor", DECACTOR);
    json = api_renew_json_int(json, "sessionId", 0);
    json = api_renew_json_int(json, "chanId", 0);
    json = api_renew_json_int(json, "modeId", modeId);
    json = api_renew_json_int(json, "avtype", 0);
    json = api_renew_json_int(json, "selfmode", selfmode);
    json = api_renew_json_int(json, "testmode", 0);
    json = api_renew_json_int(json, "nettime", 0);
    json = api_renew_json_int(json, "width", DEFAULT_WIDTH);
    json = api_renew_json_int(json, "height", DEFAULT_HEIGHT);
    json = api_renew_json_int(json, "status", 1);
    jsonStr = api_json2str(json);
    ret = api_taskpool_addtask(handle, jsonStr, video_decode, taskId);

    //sleep(1);//???

    CallCapture *capture = NULL;
    CallRender *render = NULL;
#ifndef SIMULATOR_DATA
    strcpy(params, ctmp);
    json = (cJSON *)api_str2json(params);
    json = api_renew_json_int(json, "port", port);
    json = api_renew_json_str(json, "server_ip", server_ip);
#ifdef _WIN32
    json = api_renew_json_str(json, "input_name", "dshow");
    json = api_renew_json_str(json, "device_name", "USB  Live camera");
    //json = api_renew_json_str(json, "device_name", "Integrated Camera");
    json = api_renew_json_str(json, "input_format", "mjpeg");
#else
    json = api_renew_json_str(json, "input_name", "v4l2");
    //json = api_renew_json_str(json, "device_name", "/dev/video1");
    //json = api_renew_json_str(json, "input_format", "raw");//
    json = api_renew_json_str(json, "device_name", "/dev/video0");
    json = api_renew_json_str(json, "input_format", "mjpeg");
#endif

    json = api_renew_json_str(json, "pixformat", "AV_PIX_FMT_YUV420P");
    json = api_renew_json_str(json, "scale_type", "SWS_BILINEAR");
    json = api_renew_json_int(json, "cap_width", DEFAULT_WIDTH);
    json = api_renew_json_int(json, "cap_height", DEFAULT_HEIGHT);
    json = api_renew_json_int(json, "framerate", 25);
    json = api_renew_json_int(json, "max_buf_num", 4);
    json = api_renew_json_int(json, "width", DEFAULT_WIDTH);
    json = api_renew_json_int(json, "height", DEFAULT_HEIGHT);
    json = api_renew_json_int(json, "ratio", 3);
    json = api_renew_json_int(json, "denoise", 2);
    json = api_renew_json_int(json, "osd", 1);
    json = api_renew_json_int(json, "orgX", 0);
    json = api_renew_json_int(json, "orgY", 0);
    json = api_renew_json_int(json, "scale", 0);
    json = api_renew_json_int(json, "color", 1);
    json = api_renew_json_int(json, "print", 0);
    jsonStr = api_json2str(json);
    printf("pool_test: jsonStr= %s \n", jsonStr);

    taskId = 4;
    ret = api_taskpool_addtask(handle, jsonStr, video_capture, taskId);
    //obj = (SocketObj *)task->hnd[taskId];
    //CallCapture *capture = (CallCapture *)obj->session;

#if 1
    strcpy(params, "{\"name\":\"render\"}");
    json = (cJSON *)api_str2json(params);
    json = api_renew_json_int(json, "port", port);
    json = api_renew_json_str(json, "server_ip", server_ip);
    json = api_renew_json_int(json, "screen_w", DEFAULT_WIDTH);
    json = api_renew_json_int(json, "screen_h", DEFAULT_HEIGHT);
    json = api_renew_json_int(json, "pixel_w", DEFAULT_WIDTH);
    json = api_renew_json_int(json, "pixel_h", DEFAULT_HEIGHT);
    json = api_renew_json_int(json, "bpp", 12);
    json = api_renew_json_int(json, "wait_time", 0);
    json = api_renew_json_str(json, "pixformat", "SDL_PIXELFORMAT_IYUV");
    json = api_renew_json_int(json, "win_id", 0);
    json = api_renew_json_int(json, "osd", 1);
    json = api_renew_json_int(json, "orgX", 0);
    json = api_renew_json_int(json, "orgY", 0);
    json = api_renew_json_int(json, "scale", 0);
    json = api_renew_json_int(json, "color", 4);
    json = api_renew_json_int(json, "print", 0);
    jsonStr = api_json2str(json);
    printf("pool_test: jsonStr= %s \n", jsonStr);

    taskId = 5;
    ret = api_taskpool_addtask(handle, jsonStr, video_render, taskId);
    //obj = (SocketObj *)task->hnd[taskId];
    //render = (CallRender *)obj->session;
#endif
    sleep(2);
    taskId = 4;
    obj = (SocketObj *)task->hnd[taskId];
    if(obj)
    {
        printf("pool_test: obj= %x \n", obj);
        SessionObj *session = (SessionObj *)obj->session;
        capture = (CallCapture *)session->capture;
        printf("pool_test: capture= %x \n", capture);
    }

    taskId = 5;
    obj = (SocketObj *)task->hnd[taskId];
    if(obj)
    {
        printf("pool_test: obj= %x \n", obj);
        SessionObj *session = (SessionObj *)obj->session;
        render = (CallRender *)session->render;
        printf("pool_test: render= %x \n", render);
    }

    taskId = 2;
    obj = (SocketObj *)task->hnd[taskId];
    if(obj)
    {
        video_set_capture(obj, capture, 0);
        video_set_render(obj, render);
        video_set_netinfo(obj, globalNetInfoObj);
    }


    taskId = 3;
    obj = (SocketObj *)task->hnd[taskId];
    if(obj)
    {
        video_set_render(obj, render);
        video_set_netinfo(obj, globalNetInfoObj);
    }
#endif//SIMULATOR_DATA
}//video

if(!is_video)
{

    CallAudioCapture * acapture = NULL;
    CallPlayer *player = NULL;
#if 1
    taskId = 6;
    strcpy(params, "{\"idx\":0}");
    json = (cJSON *)api_str2json(params);
    json = api_renew_json_int(json, "port", port);
    json = api_renew_json_str(json, "server_ip", server_ip);
    //json = api_renew_json_str(json, "yuvfp", "");
    json = api_renew_json_int(json, "actor", 1);
    json = api_renew_json_int(json, "sessionId", 0);
    json = api_renew_json_int(json, "chanId", 0);
    json = api_renew_json_int(json, "modeId", modeId);
    json = api_renew_json_int(json, "avtype", 0);
    json = api_renew_json_int(json, "selfmode", selfmode);
    json = api_renew_json_int(json, "testmode", 0);
    json = api_renew_json_int(json, "nettime", 0);
    json = api_renew_json_int(json, "status", 1);
    jsonStr = api_json2str(json);
    ret = api_taskpool_addtask(handle, jsonStr, audio_encode, taskId);//client_main

    //sleep(1);//???

    taskId = 7;
    strcpy(params, "{\"idx\":0}");
    json = (cJSON *)api_str2json(params);
    json = api_renew_json_int(json, "port", port);
    json = api_renew_json_str(json, "server_ip", server_ip);
    json = api_renew_json_int(json, "actor", DECACTOR);
    json = api_renew_json_int(json, "sessionId", 0);
    json = api_renew_json_int(json, "chanId", 0);
    json = api_renew_json_int(json, "modeId", modeId);
    json = api_renew_json_int(json, "avtype", 0);
    json = api_renew_json_int(json, "selfmode", selfmode);
    json = api_renew_json_int(json, "testmode", 0);
    json = api_renew_json_int(json, "nettime", 0);
    json = api_renew_json_int(json, "status", 1);
    //json = api_renew_json_str(json, "pcmfp", "");
    jsonStr = api_json2str(json);
    ret = api_taskpool_addtask(handle, jsonStr, audio_decode, taskId);
    //
    strcpy(params, ctmp);
    json = (cJSON *)api_str2json(params);
    json = api_renew_json_int(json, "port", port);
    json = api_renew_json_str(json, "server_ip", server_ip);
#ifdef _WIN32
    json = api_renew_json_str(json, "input_name", "dshow");
    json = api_renew_json_str(json, "device_name", "micro (USB Live Camera audio)");
#else
    json = api_renew_json_str(json, "input_name", "alsa");
    json = api_renew_json_str(json, "device_name", "default");
#endif
    json = api_renew_json_str(json, "in_sample_fmt", "AV_SAMPLE_FMT_S16");//
    json = api_renew_json_str(json, "out_sample_fmt", "AV_SAMPLE_FMT_S16");
    json = api_renew_json_str(json, "out_channel_layout", "AV_CH_LAYOUT_STEREO");
    json = api_renew_json_str(json, "filename", "./dukou_ref3.wav");
    json = api_renew_json_str(json, "pcmfile", "");
    json = api_renew_json_str(json, "capfile", "");
    json = api_renew_json_int(json, "in_channels", 2);
    json = api_renew_json_int(json, "out_channels", 2);
    json = api_renew_json_int(json, "in_sample_rate", 48000);
    json = api_renew_json_int(json, "out_sample_rate", 48000);
    json = api_renew_json_int(json, "out_nb_samples", 1024);
    json = api_renew_json_int(json, "out_buffer_size", 4096);
    json = api_renew_json_int(json, "frame_size", 4096);
    json = api_renew_json_int(json, "max_buf_num", 8);
    json = api_renew_json_int(json, "datatype", 2);
    json = api_renew_json_int(json, "process", 1);
    json = api_renew_json_int(json, "print", 0);
    jsonStr = api_json2str(json);
    printf("pool_test: jsonStr= %s \n", jsonStr);

    taskId = 8;
    ret = api_taskpool_addtask(handle, jsonStr, audio_capture, taskId);
#endif
    //sleep(1);

    strcpy(params, "{\"name\":\"player\"}");
    json = (cJSON *)api_str2json(params);
    json = api_renew_json_int(json, "port", port);
    json = api_renew_json_str(json, "server_ip", server_ip);
    json = api_renew_json_int(json, "mix_num", 2);
    json = api_renew_json_str(json, "format", "AUDIO_S16SYS");
    json = api_renew_json_int(json, "silence", 0);
    json = api_renew_json_int(json, "out_channels", 2);
    json = api_renew_json_int(json, "out_nb_samples", 2048);
    json = api_renew_json_str(json, "out_sample_fmt", "AV_SAMPLE_FMT_S16");
    json = api_renew_json_int(json, "out_sample_rate", 48000);
    json = api_renew_json_str(json, "out_channel_layout", "AV_CH_LAYOUT_STEREO");
    json = api_renew_json_int(json, "out_buffer_size", 8192);
    json = api_renew_json_int(json, "frame_size", 8192);
    json = api_renew_json_str(json, "pcmfile", "");
    json = api_renew_json_int(json, "sdl_status", 0);//0：纯音频模式；1：音视频
    json = api_renew_json_int(json, "print", 0);
    jsonStr = api_json2str(json);
    printf("pool_test: jsonStr= %s \n", jsonStr);

    taskId = 9;
    ret = api_taskpool_addtask(handle, jsonStr, audio_player, taskId);
    //===
    sleep(2);
    taskId = 8;
    obj = (SocketObj *)task->hnd[taskId];
    if(obj)
    {
        printf("pool_test: obj= %x \n", obj);
        SessionObj *session = (SessionObj *)obj->session;
        acapture = (CallCapture *)session->capture;
        printf("pool_test: acapture= %x \n", acapture);
    }

    taskId = 9;
    obj = (SocketObj *)task->hnd[taskId];
    if(obj)
    {
        printf("pool_test: obj= %x \n", obj);
        SessionObj *session = (SessionObj *)obj->session;
        player = (CallPlayer *)session->render;
        printf("pool_test: player= %x \n", player);
    }

    taskId = 6;
    obj = (SocketObj *)task->hnd[taskId];
    if(obj)
    {
        audio_set_capture(obj, acapture, 0);
        //audio_set_player(obj, player);
        audio_set_netinfo(obj, globalNetInfoObj);
    }


    taskId = 7;
    obj = (SocketObj *)task->hnd[taskId];
    if(obj)
    {
        audio_set_player(obj, player);
        audio_set_netinfo(obj, globalNetInfoObj);
    }
}//audio

    sleep (1);

    api_taskpool_close(handle);

    if(globalNetInfoObj)
    {
        CloseNetInfoHnd(globalNetInfoObj);
    }


    printf("pool_test: over \n");
    return ret;
}

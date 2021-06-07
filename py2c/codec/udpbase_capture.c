#include "inc.h"
#include "udpbase.h"

//============================================call capture======================================================

static void StopRecv(CallCapture *obj)
{
    pthread_mutex_lock(&obj->status_lock);
    if(obj->recv_status > 0)
    {
        obj->recv_status = 0;
    }
    pthread_mutex_unlock(&obj->status_lock);
}
static void ExitRecv(CallCapture *obj)
{
    pthread_mutex_lock(&obj->status_lock);
    obj->recv_status = -1;
    pthread_mutex_unlock(&obj->status_lock);
}
static void WaitRecv(CallCapture *obj)
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

int CaptureStop(CallCapture *obj)
{
    int ret = 0;
    if(obj)
    {
        api_capture_close(obj->handle);
    }
    return ret;
}
int read_frame_run(CallCapture *obj)
{
    int ret = 0;
    pthread_mutex_lock(&obj->status_lock);
    obj->recv_status = 1;
    int status = obj->recv_status;
    pthread_mutex_unlock(&obj->status_lock);
    int frame_num = 0;
    int pkt_send_num = 0;
    int64_t time0 = get_sys_time();
    obj->start_time = time0;
    while(status > 0)
    {
        int64_t now_time = get_sys_time();
        int difftime = (int)(now_time - obj->start_time);
        ret = api_capture_read_frame(obj->handle, 0);
        if(ret <= 0)
        {
            usleep(1000);
        }
        frame_num++;
        difftime = (int)(now_time - time0);
        if(difftime > 2000)
        {
            float framerate = (float)((frame_num * 1000.0) / difftime);
            time0 = now_time;
            frame_num = 0;
            printf("vodeo read_frame_run: framerate=%1.1f (fps) \n", framerate);
        }

        pthread_mutex_lock(&obj->status_lock);
        status = obj->recv_status;
        pthread_mutex_unlock(&obj->status_lock);
    }
    printf("read_frame_run: end \n");
    CaptureStop(obj);
    ExitRecv(obj);

    printf("read_frame_run: over \n");
    return ret;
}
void stop_capture_run(CallCapture *obj)
{
    printf("stop_capture_run: 0 \n");
    StopRecv(obj);
    printf("stop_capture_run: wait... \n");
    WaitRecv(obj);
    printf("stop_capture_run: over \n");
}
int read_frame(CallCapture *obj, char **outbuf)
{
    int ret = 0;
    pthread_mutex_lock(&obj->status_lock);
    int status = obj->recv_status;
    pthread_mutex_unlock(&obj->status_lock);
    if(status > 0)
    {
        ret = api_capture_read_frame3(obj->handle, outbuf);
    }
    return ret;
}

int CaptureInit(CallCapture *obj)
{
    int ret = 0;
    cJSON *json = NULL;//mystr2json(params);

    ret = api_capture_init(obj->handle, obj->params);
    if(json)
    {
        deleteJson(json);
        json = NULL;
    }

    return ret;
}
void * video_capture(SocketObj *sock)
{
    int taskId = sock->id;
    sock->status = 1;
    pthread_mutex_init(&sock->status_lock,NULL);
    SessionObj *session = (SessionObj *)calloc(1, sizeof(SessionObj));
    sock->session = (void *)session;
    CallCapture *obj = (CallCapture *)calloc(1, sizeof(CallCapture));
    session->capture = obj;
    obj->params = sock->params;

    int ret = CaptureInit(obj);
    if(ret < 0)
    {
        pthread_mutex_lock(&sock->status_lock);
        sock->status = -1;
        pthread_mutex_unlock(&sock->status_lock);
        free(obj);
        if(session)
        {
            free(session);
        }
        sock->session = NULL;
        //pthread_mutex_destroy(&sock->status_lock);
        return ret;
    }
    pthread_mutex_init(&obj->status_lock,NULL);
    read_frame_run(obj);
    pthread_mutex_destroy(&obj->status_lock);
    free(obj);
    if(session)
    {
        free(session);
    }
    pthread_mutex_destroy(&sock->status_lock);
    sock->status = -1;
    printf("video_capture: over: taskId=%d \n", taskId);
    return 0;
}

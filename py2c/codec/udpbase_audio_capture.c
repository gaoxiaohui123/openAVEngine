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

static int CaptureStop(CallCapture *obj)
{
    int ret = 0;
    if(obj)
    {
        api_audio_capture_close(obj->handle);
    }
    return ret;
}
static int read_frame_run(CallCapture *obj)
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
        ret = api_audio_capture_read_frame(obj->handle);
        if(ret <= 0)
        {
            usleep(10000);
        }
        frame_num++;
        difftime = (int)(now_time - time0);
        if(difftime > 2000)
        {
            float framerate = (float)((frame_num * 1000.0) / difftime);
            time0 = now_time;
            frame_num = 0;
            printf("audio read_frame_run: framerate=%1.1f (fps) \n", framerate);
        }

        pthread_mutex_lock(&obj->status_lock);
        status = obj->recv_status;
        pthread_mutex_unlock(&obj->status_lock);
    }
    printf("audio read_frame_run: end \n");
    CaptureStop(obj);
    ExitRecv(obj);

    printf("audio read_frame_run: over \n");
    return ret;
}
static void stop_capture_run(CallCapture *obj)
{
    printf("audio stop_capture_run: 0 \n");
    StopRecv(obj);
    printf("audio stop_capture_run: wait... \n");
    WaitRecv(obj);
}
int audio_read_frame(CallCapture *obj, char **p_outbuf)
{
    //printf("audio_read_frame: 0 \n");
    int ret = api_audio_capture_read_frame3(obj->handle, &obj->readbuf);
    if(ret > 0)
    {
        //printf("audio_read_frame: ret=%d \n", ret);
        memcpy(&obj->data_buf[obj->offset], obj->readbuf, ret);
        obj->offset += ret;
        //printf("audio_read_frame: obj->offset=%d \n", obj->offset);
        if(obj->offset >= obj->frame_size) //block_size:
        {
            //printf("audio_read_frame: obj->frame_size=%d \n", obj->frame_size);
            memcpy(obj->outbuf, obj->data_buf, obj->frame_size);
            *p_outbuf = obj->outbuf;
            ret = obj->frame_size;
            int tail = obj->offset - obj->frame_size;
            //printf("audio_read_frame: tail=%d \n", tail);
            if(tail > 0)
            {
                memmove(obj->data_buf, &obj->data_buf[obj->frame_size], tail);
            }
            obj->offset = tail;
        }
        else{
            ret = 0;
        }
    }
    return ret;
}

static int CaptureInit(CallCapture *obj)
{
    int ret = 0;
    cJSON *json = api_str2json(obj->params);

    obj->offset = 0;
    int frame_size = GetvalueInt(json, "frame_size");
    obj->frame_size = 8192;
    obj->outbuf = malloc(obj->frame_size * sizeof(char));
    obj->data_buf = malloc((obj->frame_size << 1) * sizeof(char));
    obj->readbuf = NULL;
    ret = api_audio_capture_init(obj->handle, obj->params);
    if(json)
    {
        deleteJson(json);
        json = NULL;
    }

    return ret;
}
static void CaptureClose(CallCapture *obj)
{
    if(obj->outbuf)
    {
        free(obj->outbuf);
        obj->outbuf = NULL;
    }
    if(obj->data_buf)
    {
        free(obj->data_buf);
        obj->data_buf = NULL;
    }
}
void * audio_capture(SocketObj *sock)
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
        CaptureClose(obj);
        free(obj);
        session->capture = NULL;
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
    CaptureClose(obj);
    free(obj);
    if(session)
    {
        free(session);
    }
    pthread_mutex_destroy(&sock->status_lock);
    sock->status = -1;
    printf("audio_capture: over: taskId=%d \n", taskId);
    return 0;
}

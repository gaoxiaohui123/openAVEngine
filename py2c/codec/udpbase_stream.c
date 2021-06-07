#include "inc.h"
#include "udpbase.h"

#if 1
void WaitAVStream(SocketObj *obj)
{
    int status = 0;
    do{
        pthread_mutex_lock(&obj->status_lock);
        status = obj->recv_status;
        pthread_mutex_unlock(&obj->status_lock);
        if(!status)
        {
            usleep(100000);//100ms
        }
    }while(!status);
}
void SetStartStream(SocketObj *obj)
{
    pthread_mutex_lock(&obj->status_lock);
    obj->recv_status = 1;
    pthread_mutex_unlock(&obj->status_lock);
}
#endif
void * read_stream(SocketObj *sock)
{
    int taskId = sock->id;
    sock->status = 1;
    printf("read_stream: taskId=%d \n", taskId);
    //pthread_mutex_init(&sock->lock,NULL);
    pthread_mutex_init(&sock->status_lock,NULL);
    SessionObj *session = (SessionObj *)calloc(1, sizeof(SessionObj));
    printf("read_stream: sock->params=%s \n", sock->params);
    int ret = api_avstream_init(session->handle, sock->params);
    sock->session = (void *)session;
    if(ret < 0)
    {
        printf("read_stream: failed! ret=%d \n", ret);
        if(session)
        {
            free(session);
            sock->session = NULL;
        }
        pthread_mutex_destroy(&sock->status_lock);
        sock->status = -1;
        return ret;
    }
    SetStartStream(sock);

    //WaitStart(sock);

    ret = api_avstream_loopread(session->handle);

    api_avstream_close(session->handle);

    if(sock->session)
    {
        SessionObj * session = (SessionObj *)sock->session;
        free(sock->session);
        sock->session = NULL;
    }

    //pthread_mutex_destroy(&sock->lock);
    pthread_mutex_destroy(&sock->status_lock);
    sock->status = -1;
    printf("read_stream: over: taskId=%d \n", taskId);
    return 0;
}

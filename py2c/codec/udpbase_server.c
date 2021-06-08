
#include "cJSON.h"
#include "udpbase.h"

extern void * server_main(SocketObj *obj);
extern int InitTask(TaskPool *task);
extern int CloseTask(TaskPool *task);
extern int AddTask(TaskPool *task, char *params, void *thread_fun, int taskId);
extern void StopBroadCast(SocketObj *obj);
extern void StopSend(SocketObj *obj);
extern void StopRecv(SocketObj *obj);
extern int GetvalueInt(cJSON *json, char *key);

HCSVC_API
int api_taskpool_handle(char *handle)
{
    long long ret = 0;
    if(handle)
    {
        TaskPool *obj = (TaskPool *)calloc(1, sizeof(TaskPool));
        ret = (long long)obj;
        int handle_size = sizeof(long long);
        memcpy(handle, &ret, handle_size);
        long long *testp = (long long *)handle;
        ret = (int)obj;
    }
    return (int)(ret & 0x7FFFFFFF);
}
HCSVC_API
void api_taskpool_close(char *handle)
{
    if(handle)
    {
        long long *testp = (long long *)handle;
        TaskPool *obj = (TaskPool *)testp[0];
        printf("api_taskpool_close \n");

        CloseTask(obj);
        if(obj->json)
        {
            api_json_free(obj->json);
            obj->json = NULL;
        }
        free(obj);
	    testp[0] = 0;
        printf("api_taskpool_close: ok \n");
    }
}
HCSVC_API
int api_taskpool_init(char *handle, char *params)
{
    int ret = 0;

    ret = api_taskpool_handle(handle);
    long long *testp = (long long *)handle;
    TaskPool *obj = (TaskPool *)testp[0];

    cJSON *json = (cJSON *)api_str2json(params);
    int aliveTaskNum = GetvalueInt(json, "aliveTaskNum");
    int taskNum = GetvalueInt(json, "taskNum");

    printf("api_taskpool_init: aliveTaskNum=%d \n", aliveTaskNum);
    printf("api_taskpool_init: taskNum=%d \n", taskNum);

    obj->aliveTaskNum = aliveTaskNum;
    obj->taskNum = taskNum;//2;
    //obj->thread_fun = client_main;
    //obj->thread_fun = client_broadcast;
    InitTask(obj);

	return ret;
}
HCSVC_API
int api_taskpool_addtask(char *handle, char *params, void *thread_fun, int taskId)
{
    int ret = 0;
    long long *testp = (long long *)handle;
    TaskPool *obj = (TaskPool *)testp[0];

    AddTask(obj, params, thread_fun, taskId);

    return ret;
}

HCSVC_API
int pool_start_server(char *handle, int taskId, int port, char *host)
{
    int ret = 0;
    char params[2048] = "";
    cJSON *json = NULL;
    char *jsonStr = NULL;

    long long *testp = (long long *)handle;
    TaskPool *task = (TaskPool *)testp[0];
    if(!task)
    {
        printf("pool_server: task=%x \n", task);
        strcpy(params, "{\"name\":\"media_server\"}");
        json = (cJSON *)api_str2json(params);
        json = api_renew_json_int(json, "aliveTaskNum", 2);
        json = api_renew_json_int(json, "taskNum", 2);
        jsonStr = api_json2str(json);
        api_taskpool_init(handle, jsonStr);
        task = (TaskPool *)testp[0];
    }

    strcpy(params, "{\"name\":\"media_server\"}");
    json = (cJSON *)api_str2json(params);
    json = api_renew_json_int(json, "port", port);
    json = api_renew_json_str(json, "server_ip", host);
    jsonStr = api_json2str(json);
    ret = api_taskpool_addtask(handle, jsonStr, server_main, taskId);

    printf("pool_server: port=%d \n", port);
    printf("pool_server: host=%s \n", host);

    return taskId;
}
HCSVC_API
int api_stop_server_task(char *handle, int taskId)
{
    int ret = 0;
    long long *testp = (long long *)handle;
    TaskPool *task = (TaskPool *)testp[0];
    SocketObj *obj = (SocketObj *)task->hnd[taskId];

    if(obj)
    {
        StopBroadCast(obj);
        StopSend(obj);
        StopRecv(obj);
    }

    return ret;
}
HCSVC_API
int pool_stop_server(char *handle)
{
    int ret = 0;

    api_taskpool_close(handle);

    printf("pool_stop_server: over \n");

    return ret;
}
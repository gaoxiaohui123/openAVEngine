

//#include "inc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "cJSON.h"
#include "hcsvc.h"
//修改对象的值
//cJSON_ReplaceItemInObject(item,"word",cJSON_CreateString("nihaoxiaobai"));
//cJSON_AddItemToObject(body, "Info", filter_root);

extern cJSON* renewJson(cJSON *json, char *key, int ivalue, char *cvalue, cJSON *subJson);

#if 0
cJSON* renewJson(cJSON *json, char *key, int ivalue, char *cvalue, cJSON *subJson)
{
    cJSON *ret  = json;
    if(NULL == json)
    {
        ret = cJSON_CreateObject(); //创建JSON对象

        if(NULL == ret)
        {
            //error happend here
            return NULL;
        }
    }
    if(NULL != key)
    {
        if(NULL != subJson)
        {
            cJSON *item = cJSON_GetObjectItem(ret, key);
            if(cJSON_IsArray(item) || cJSON_IsObject(item))
            {
                //cJSON_ReplaceItemInObject(ret, key, cJSON_CreateArray(subJson));
                //cJSON_ReplaceItemInObject(ret, key, cJSON_CreateObject(subJson));
                cJSON_ReplaceItemInObject(ret, key, subJson);
            }
            else{
                cJSON_AddItemToObject(ret, key, subJson);
            }
        }
        else if(NULL != cvalue)
        {
            cJSON *item = cJSON_GetObjectItem(ret, key);
            if(cJSON_IsString(item))
            {
                cJSON_ReplaceItemInObject(ret, key, cJSON_CreateString(cvalue));
                //cJSON_ReplaceItemInObject(ret, key, cvalue);
            }
            else{
                cJSON_AddStringToObject(ret, key, cvalue);
            }
        }
        else
        {
            cJSON *item = cJSON_GetObjectItem(ret, key);
            if(cJSON_IsNumber(item))
            {
                cJSON_ReplaceItemInObject(ret, key, cJSON_CreateNumber(ivalue));
                //cJSON_ReplaceItemInObject(ret, key, ivalue);
            }
            else{
                cJSON_AddNumberToObject(ret, key, ivalue);
            }
        }
    }
    else{
        cJSON_Delete(ret); //释放json对象
    }
    return ret;
}
#endif
cJSON* renewJsonArray(cJSON *json, char *key, int *value, int len)
{
    if(NULL == json)
    {
        json = cJSON_CreateObject(); //创建JSON对象

        if(NULL == json)
        {
            //error happend here
            return NULL;
        }
    }
    cJSON *array = cJSON_CreateArray();
    //
    json = api_delete_item(json, key);
    //
    cJSON_AddItemToObject(json, key, array);
    for(int i = 0; i < len; i++)
    {
        cJSON_AddItemToArray(array, cJSON_CreateNumber(value[i]));
    }
    return json;
}
cJSON* renewJsonArray1(cJSON *json, char *key, short *value, int len)
{
    if(NULL == json)
    {
        json = cJSON_CreateObject(); //创建JSON对象

        if(NULL == json)
        {
            //error happend here
            return NULL;
        }
    }
    cJSON *array = cJSON_CreateArray();
    //
    json = api_delete_item(json, key);
    //
    cJSON_AddItemToObject(json, key, array);
    for(int i = 0; i < len; i++)
    {
        cJSON_AddItemToArray(array, cJSON_CreateNumber(value[i]));
    }
    return json;
}
cJSON* renewJsonArray4(cJSON *json, char *key, int *value, int len)
{
    if(NULL == json)
    {
        json = cJSON_CreateObject(); //创建JSON对象

        if(NULL == json)
        {
            //error happend here
            return NULL;
        }
    }
    cJSON *array = cJSON_CreateArray();
    //
    json = api_delete_item(json, key);
    //
    cJSON_AddItemToObject(json, key, array);
    for(int i = 0; i < len; i++)
    {
        cJSON_AddItemToArray(array, cJSON_CreateNumber(value[i]));
    }
    return json;
}
#if 0
cJSON* renewJsonArray2(cJSON *json, char *key, short *value)
{
    if(NULL == json)
    {
        json = cJSON_CreateObject(); //创建JSON对象

        if(NULL == json)
        {
            //error happend here
            return NULL;
        }
    }
    cJSON *array = cJSON_CreateArray();
    cJSON_AddItemToObject(json, key, array);
    int i = 0;
    while(value[i] > 0)
    {
        cJSON_AddItemToArray(array, cJSON_CreateNumber(value[i]));
        i++;
    }
    return json;
}
#endif
cJSON* renewJsonArray3(cJSON **json, cJSON **array, char *key, cJSON *item)
{
    if(NULL == *json)
    {
        *json = cJSON_CreateObject(); //创建JSON对象

        if(NULL == *json)
        {
            //error happend here
            return NULL;
        }
        *array = cJSON_CreateArray();
        //
        *json = api_delete_item(*json, key);
        //
        cJSON_AddItemToObject(*json, key, *array);
    }
    cJSON *thisarray = cJSON_GetObjectItem(*json, key);
    if(thisarray)
    {
        *array = thisarray;
    }
    else if(NULL == *array)
    {
        *array = cJSON_CreateArray();
        //
        *json = api_delete_item(*json, key);
        //
        cJSON_AddItemToObject(*json, key, *array);
    }
    cJSON_AddItemToArray(*array, item);
    return array;
}
cJSON* renewJsonFloat(cJSON *json, char *key, float fvalue)
{
    cJSON *ret  = json;
    if(NULL == json)
    {
        ret = cJSON_CreateObject(); //创建JSON对象

        if(NULL == ret)
        {
            //error happend here
            return NULL;
        }
    }
    if(NULL != key)
    {
        {
            cJSON_AddNumberToObject(ret, key, fvalue);
        }
    }
    return ret;
}

#if 0
#define JSON_NAME_MAX_SIZE  128
enum
{
    JSON_OK = 0,
    JSON_ERROR,
    JSON_ERR_NO_SUCH_NODE,
    JSON_ERR_INVALID_NODE,
    JSON_ERR_NO_SUCH_ARRAY,
    JSON_ERR_INVALID_ARRAY_INDEX,
    JSON_ERR_UNMATCH_TYPE
};
void *jsonGetNode(void *json, const char *node, int *ret)
{
    cJSON *s;
    const char *subItem = NULL;
    const char *p;
    char *pe;
    char item[JSON_NAME_MAX_SIZE + 1];
    int len, index = -1;

    if ((node == NULL) || (*node == '\0'))
    {
        *ret = JSON_ERR_INVALID_NODE;
        return NULL;
    }

    /* split first node name */
    p = strchr(node, '.');

    if (p)
    {
        len = p - node;
        subItem = p + 1;
    }
    else
    {
        len = strlen(node);
    }

    if (!len || (len > sizeof(item) - 1))
    {
        *ret = JSON_ERR_INVALID_NODE;
        return NULL;
    }

    strncpy(item, node, len);
    item[len] = '\0';

    /* get array if it is */
    pe = strchr(item, '[');
    if (pe)
    {
        *pe = '\0';
        index = strtoul(pe + 1, NULL, 10);
    }

    s = cJSON_GetObjectItem(json, item);

    if (s && (index >= 0))
    {

        if (s->type != cJSON_Array)
        {
            *ret = JSON_ERR_NO_SUCH_ARRAY;
            return NULL;
        }

        if (index >= cJSON_GetArraySize(s))
        {
            *ret = JSON_ERR_INVALID_ARRAY_INDEX;
            return NULL;
        }

        s = cJSON_GetArrayItem(s, index);
    }

    if (s == NULL)
    {
        *ret = JSON_ERR_NO_SUCH_NODE;
        return NULL;
    }

    if (subItem)
    {
        return jsonGetNode(s, subItem, ret);
    }
    else
    {
        *ret = JSON_OK;
        return s;
    }
}

int jsonSetUint64(void *json, const char *node, unsigned long long value)
{
    int ret = JSON_OK;
    cJSON *d = node ? jsonGetNode(json, node, &ret) : json;

    if (ret != JSON_OK) return ret;

    if (!jsonTestNumber(d))
    {
        return JSON_ERR_UNMATCH_TYPE;
    }

    d->valueuint64 = value;

    return JSON_OK;
}
#endif

cJSON* deleteItem(cJSON *json, char *key)
{
    if(json && key)
    {

        cJSON *to_detach = cJSON_GetObjectItem(json, key);
        if(to_detach)
        {
#if 0
            cJSON_DetachItemFromObject(json, key);
            //printf("deleteItem: to_detach=%x \n", to_detach);
            cJSON_Delete(to_detach);
#else
            cJSON_DeleteItemFromObject(json, key);
#endif
        }
    }
    return json;
}
#if 0
int jsonGetUint64(void *json, const char *node, unsigned long long *value)
{
    int ret = JSON_OK;
    cJSON *d = node ? jsonGetNode(json, node, &ret) : json;

    if (ret != JSON_OK) return ret;
    if (!jsonTestNumber(d))
    {
        return JSON_ERR_UNMATCH_TYPE;
    }

    if (value)
    {
        *value = d->valueuint64;
    }
    return JSON_OK;
}
#endif


char* GetvalueStr(cJSON *json, char *key, char *result)
{
    cJSON *item = cJSON_GetObjectItem(json, key);
    if(cJSON_IsNull(item))
    {
        printf("GetvalueStr: null: key=%s \n", key);
    }
    else if(cJSON_IsString(item))
    {
        //printf("GetvalueStr: item->valuestring=%s \n", item->valuestring);
        if(result)
        {
            strcpy(result, item->valuestring);
        }
        return item->valuestring;//item为临时变量,返回后会被释放掉;
    }
    else{
        printf("GetvalueStr: 2: null: key=%s \n", key);
    }
    return "";
}

int *GetArrayValueInt(cJSON *json, char *key, int *arraySize)
{
    int *ret = NULL;
    cJSON *cjsonArr = cJSON_GetObjectItem(json, key);
    if( NULL != cjsonArr ){
        int i = 0;
        do
        {
            cJSON *cjsonTmp = cJSON_GetArrayItem(cjsonArr, i);
            if( NULL == cjsonTmp )
            {
                //printf("GetArrayValueInt: no member \n");
                break;
            }
            int num = cjsonTmp->valueint;
            //printf("GetArrayValueInt: num= %d \n", num);
            i++;
        }while(1);

        //int  array_size = cJSON_GetArraySize(cjsonArr);
        //nal_mem_num = array_size;
        int nal_mem_num = i;
        arraySize[0] = i;
        //printf("GetArrayValueInt: nal_mem_num= %d \n", nal_mem_num);
        if(i > 0)
        {
            ret = calloc(1, sizeof(int) * nal_mem_num);

            for( int i = 0 ; i < nal_mem_num ; i ++ ){
                cJSON * pSub = cJSON_GetArrayItem(cjsonArr, i);
                if(NULL == pSub ){ continue ; }
                //char * ivalue = pSub->valuestring ;
                int ivalue = pSub->valueint;
                ret[i] = ivalue;//可以不用傳入，通過擴展字段讀入：rtpSize[idx] = rtp_pkt_size;
                //rtpLen += ivalue;
            }
        }

    }
    return ret;
}
short *GetArrayValueShort(cJSON *json, char *key, int *arraySize)
{
    short *ret = NULL;
    cJSON *cjsonArr = cJSON_GetObjectItem(json, key);
    if( NULL != cjsonArr ){
        int i = 0;
        do
        {
            cJSON *cjsonTmp = cJSON_GetArrayItem(cjsonArr, i);
            if( NULL == cjsonTmp )
            {
                //printf("GetArrayValueInt: no member \n");
                break;
            }
            int num = cjsonTmp->valueint;
            //printf("GetArrayValueInt: num= %d \n", num);
            i++;
        }while(1);

        //int  array_size = cJSON_GetArraySize(cjsonArr);
        //nal_mem_num = array_size;
        int nal_mem_num = i;
        arraySize[0] = i;
        if(i > 0)
        {
            //printf("GetArrayValueInt: nal_mem_num= %d \n", nal_mem_num);
            ret = calloc(1, sizeof(short) * nal_mem_num);

            for( int i = 0 ; i < nal_mem_num ; i ++ ){
                cJSON * pSub = cJSON_GetArrayItem(cjsonArr, i);
                if(NULL == pSub ){ continue ; }
                //char * ivalue = pSub->valuestring ;
                int ivalue = pSub->valueint;
                ret[i] = (short)ivalue;//可以不用傳入，通過擴展字段讀入：rtpSize[idx] = rtp_pkt_size;
                //rtpLen += ivalue;
            }
        }
    }
    return ret;
}
long long *GetArrayObj(cJSON *json, char *key, int *arraySize)
{
    long long *ret = NULL;
    cJSON *cjsonArr = cJSON_GetObjectItem(json, key);
    if( NULL != cjsonArr ){
        int i = 0;
        do
        {
            cJSON *cjsonTmp = cJSON_GetArrayItem(cjsonArr, i);
            if( NULL == cjsonTmp )
            {
                //printf("GetArrayValueInt: no member \n");
                break;
            }
            int num = cjsonTmp->valueint;
            //printf("GetArrayValueInt: num= %d \n", num);
            i++;
        }while(1);

        //int  array_size = cJSON_GetArraySize(cjsonArr);
        //nal_mem_num = array_size;
        int nal_mem_num = i;
        arraySize[0] = i;
        if(i > 0)
        {
            //printf("GetArrayValueInt: nal_mem_num= %d \n", nal_mem_num);
            if(i > 0)
            {
                ret = calloc(1, sizeof(long long) * nal_mem_num);

                for( int i = 0 ; i < nal_mem_num ; i ++ ){
                    cJSON * pSub = cJSON_GetArrayItem(cjsonArr, i);
                    if(NULL == pSub ){ continue ; }
                    ret[i] = (long long)pSub;
                }
            }
        }
    }
    return ret;
}
//cjson 遍历
static void foreach()
{
    char *parmstr = "{\"video\":{\"mjpeg\":\"1x2\", \"raw\":\"3x4\"}}";
    cJSON *json = (cJSON *)api_str2json(parmstr);
    cJSON *item = json;
    do{
        if(cJSON_IsObject(item))
        {

        }
        if(!cJSON_IsNull(item))
        {
            int type = item->type;
            printf("api_get_dev_info: type=%d \n", type);
            char *key = item->string;

            if(key)
            {
                printf("api_get_dev_info: key=%s \n", key);
                cJSON *next = item;
                do{
                    next = next->next;
                    //printf("api_get_dev_info: next=%x \n", next);
                    if(!cJSON_IsNull(next) && !cJSON_IsInvalid(next) && next)
                    {
                        int type = next->type;
                        printf("api_get_dev_info: type=%d \n", type);
                        char *key = next->string;
                        printf("api_get_dev_info: key=%s \n", key);
                    }
                }while(!cJSON_IsNull(next) && !cJSON_IsInvalid(next) && next);
            }
        }
        item = item->child;

    }while(!cJSON_IsNull(item) && item);
}
static int example()
{
    char *parmstr = "{\"video\":\"mjpeg\", \"fb\":\"3x4\"}";
    cJSON *json = (cJSON *)api_str2json(parmstr);
    cJSON *item = json->child;
    cJSON *item2 = item->next;
    char *key = item2->string;
    printf("api_get_dev_info: key=%s \n", key);
    api_json_free(json);
    return 0;
}

HCSVC_API
void* api_renew_json_float(void *json, char *key, float fvalue)
{
    json = api_delete_item(json, key);
    return (void *)renewJsonFloat((cJSON *)json, key, fvalue);
}
HCSVC_API
void* api_renew_json_array(void *json, char *key, int *value, int len)
{
    json = api_delete_item(json, key);
    return (void *)renewJsonArray((cJSON *)json, key, value, len);
}
void* api_delete_item(void *json, char *key)
{
    return (void *)deleteItem((cJSON *)json, key);
}

HCSVC_API
int* api_get_array_int(char *parmstr, char *key, int *arraySize)
{
    int *ret = NULL;
    int chanNum = 0;
    cJSON *json = (cJSON *)api_str2json(parmstr);
    if(json)
    {
        ret = GetArrayValueInt(json, key, arraySize);
        api_json_free(json);
    }

    return ret;
}


#if 1
void *glob_json = NULL;
int64_t glob_idx = 0;
HCSVC_API
void api_mem_lead_cjson(int64_t start, int64_t loopn)//api_mem_leak_cjson
{
    printf("api_mem_lead_cjson: loopn=%lld \n", loopn);
    for(int64_t i = start; i < (start + loopn); i++)
    {
        if((glob_idx % 50) == 0)
        {
            glob_json = api_renew_json_int(glob_json, "refresh_idr", glob_idx / 50);
        }

        glob_json = api_renew_json_int(glob_json , "idx", glob_idx % (1 << 30));
        //glob_json = api_renew_json_int(glob_json , "idx", 500);
        glob_idx++;
        if((glob_idx % 1000) == 0)
        {
            printf("api_mem_lead_cjson: glob_json=%x \n", glob_json);
            printf("api_mem_lead_cjson: glob_idx=%lld \n", glob_idx);
            char* jsonstr = api_json2str(glob_json);
            printf("api_mem_lead_cjson: jsonstr=%s \n", jsonstr);
            api_json2str_free(jsonstr);
        }
    }
}
#endif
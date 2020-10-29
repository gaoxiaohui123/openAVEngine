

#include "inc.h"


//修改对象的值
//cJSON_ReplaceItemInObject(item,"word",cJSON_CreateString("nihaoxiaobai"));
//cJSON_AddItemToObject(body, "Info", filter_root);
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
    cJSON_AddItemToObject(json, key, array);
    for(int i = 0; i < len; i++)
    {
        cJSON_AddItemToArray(array, cJSON_CreateNumber(value[i]));
    }
    return json;
}
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
            cJSON *item = cJSON_GetObjectItem(ret, key);
            if(cJSON_IsNumber(item))
            {
                cJSON_ReplaceItemInObject(ret, key, cJSON_CreateNumber(fvalue));
                //cJSON_ReplaceItemInObject(ret, key, ivalue);
            }
            else{
                cJSON_AddNumberToObject(ret, key, fvalue);
            }
        }
    }
    return ret;
}
cJSON* renewJsonInt(cJSON *json, char *key, int ivalue)
{
    return renewJson(json, key, ivalue, NULL, NULL);
}
cJSON* renewJsonStr(cJSON *json, char *key, char *cvalue)
{
    return renewJson(json, key, 0, cvalue, NULL);
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
    cJSON_DetachItemFromObject(json, key);
    return json;
}

cJSON* deleteJson(cJSON *json)
{
    return renewJson(json, NULL, 0, NULL, NULL);
}
cJSON* mystr2json(char *text)
{
    char *out;
    cJSON *json;
    if (text == NULL || !strcmp(text, ""))
    {
        //char *text = "{\"size\" : 1024, \"data\" : \"this is string\"}";
        //char* text = "{\"name\":\"Messi\",\"age\":\"29\"}";
        char data[128] = "";
        for (int i = 0; i < 128; i++)
        {
            data[i] = (char)i;
        }
        //text = "{\"size\" : 1024, \"data\" : data}";
        text = "{\"size\" : 1024, \"data\" : \"this is default string for test cjosn\"}";
    }

    json = cJSON_Parse(text);
    if (!json) {
        printf("Error before: [%s]\n",cJSON_GetErrorPtr());
    } else {
        //将传入的JSON结构转化为字符串
        out=cJSON_Print(json);
        //cJSON_Delete(json);
        //printf("%s\n",out);
        free(out);
    }
    return json;
}
float GetvalueFloat(cJSON *json, char *key)
{
    cJSON *item = cJSON_GetObjectItem(json, key);
    if(cJSON_IsNull(item))
    {
    }
    else if(cJSON_IsNumber(item))
    {
        return item->valuedouble;
    }
    return 0;
}
int GetvalueInt(cJSON *json, char *key)
{
    cJSON *item = cJSON_GetObjectItem(json, key);
    if(cJSON_IsNull(item))
    {
    }
    else if(cJSON_IsNumber(item))
    {
        return item->valueint;
    }
    return 0;
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

char* GetvalueStr(cJSON *json, char *key)
{
    cJSON *item = cJSON_GetObjectItem(json, key);
    if(cJSON_IsNull(item))
    {
    }
    else if(cJSON_IsString(item))
    {
        return item->valuestring;
    }
    return "";
}
//cjson 遍历
static void foreach()
{
    char *parmstr = "{\"video\":{\"mjpeg\":\"1x2\", \"raw\":\"3x4\"}}";
    cJSON *json = mystr2json(parmstr);
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
    cJSON *json = mystr2json(parmstr);
    cJSON *item = json->child;
    cJSON *item2 = item->next;
    char *key = item2->string;
    printf("api_get_dev_info: key=%s \n", key);
    deleteJson(json);
    return 0;
}

HCSVC_API
void* api_renew_json_float(void *json, char *key, float fvalue)
{
    return (void *)renewJsonFloat((cJSON *)json, key, fvalue);
}
HCSVC_API
void* api_renew_json_array(void *json, char *key, int *value, int len)
{
    return (void *)renewJsonArray((cJSON *)json, key, value, len);
}
void* api_delete_item(void *json, char *key)
{
    return (void *)deleteItem((cJSON *)json, key);
}
HCSVC_API
void api_json_free(void *json)
{
    deleteJson((cJSON *)json);
}
#if 0
HCSVC_API
void* api_renew_json_int(void *json, char *key, int ivalue)
{
    return (void *)renewJson((cJSON *)json, key, ivalue, NULL, NULL);
}
HCSVC_API
void* api_renew_json_str(void *json, char *key, char *cvalue)
{
    return (void *)renewJson((cJSON *)json, key, 0, cvalue, NULL);
}

HCSVC_API
char* api_json2str(void *json)
{
    if(json)
    {
        return cJSON_Print((cJSON *)json);
    }
    return NULL;
}
HCSVC_API
void api_json2str_free(char *jsonstr)
{
    if(jsonstr)
    {
        free(jsonstr);
    }
}
#endif
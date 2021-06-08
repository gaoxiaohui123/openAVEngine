#include "inc.h"
#include "udpbase.h"

void * get_mcu(SocketObj *sock);
extern void McuPushData(McuObj *obj, uint8_t *data, int size, int64_t now_time, int avtype);
//=============================================render==================================

void FramePushData(CallRender *obj, uint8_t *data, int width, int height, int id, int64_t frame_timestamp, int64_t now_time)
{
    pthread_mutex_lock(&obj->lock);

    FrameNode *head,*pnew, *q;
    if(!obj->frameListHead)
    {
        //head不存数据,head->next指向第一个数据,head->num保存总索引（也可以设计将第一个节点存入head）
        //注意链表元的2个组成部分，元和next指针，元控制的是整个链表元的地址，next指针则寄生在元下；
        //printf("PushData: create head: obj->data_list=%x \n", obj->data_list);
        head = (FrameNode *)calloc(1, sizeof(FrameNode));  //创建头节点。
        head->num = 0;
        head->width = width;
        head->height = height;
        head->next = NULL;  //头节点指针域置NULL
        head->tail = head;  // 开始时尾指针指向头节点；此时head->tail->next与head->next是同一地址
        obj->frameListHead = (void *)head;
    }
    head = (FrameNode *)obj->frameListHead;
    if(head->num > MAX_DELAY_FRAME_NUM)
    {
        q = head->next;
        head->next = head->next->next;
        head->num--;
        if(head->next == NULL)
        {
            //printf("FramePopData: head->next is null \n");
            head->next = NULL;  //头节点指针域置NULL
            head->tail = head;
        }
        free(q->data);
        free(q);
        printf("render: FramePushData: skip head->num=%d, id=%d \n", head->num, id);
    }
    {
        pnew = (FrameNode *)calloc(1, sizeof(FrameNode));  //创建新节点
        pnew->data = (uint8_t *)data;
        pnew->width = width;
        pnew->height = height;
        pnew->idx = head->idx;
        pnew->id = id;
        pnew->frame_timestamp = frame_timestamp;
        pnew->now_time = now_time;
        pnew->next = NULL;   //新节点指针域置NULL
        head->tail->next = pnew;  //新节点插入到表尾；首个节点时，则意味着ead->next也指向了pnew
        head->tail = pnew;   //为指针指向当前的尾节点;元地址迁移，元的原先的next脱离了元，新地址的next成为元的next指针；
        head->idx++;
        head->num++;
        //printf("FramePushData: head->num=%d \n", head->num);
    }
    pthread_mutex_unlock(&obj->lock);
}

void * FramePopData(CallRender *obj)
{
    void *ret = NULL;

    pthread_mutex_lock(&obj->lock);
    FrameNode *head, *q;
    head = (FrameNode *)obj->frameListHead;
    if(head)
    {
        q = head->next;
        if(q == NULL || q == head)
        {

        }
        else{
            head->next = head->next->next;
            ret = q;
            //printf("FramePopData: q->frame_timestamp=%lld \n", q->frame_timestamp);
            head->num--;
            if(head->next == NULL)
            {
                //printf("FramePopData: head->next is null \n");
                head->next = NULL;  //头节点指针域置NULL
                head->tail = head;
            }
        }
    }
    pthread_mutex_unlock(&obj->lock);

    return ret;
}

void FramePushData2(FrameBufferNode *obj, uint8_t *data, int width, int height, int id, int64_t frame_timestamp, int64_t now_time)
{
    FrameNode *head,*pnew, *q;
    if(!obj->frameListHead)
    {
        //head不存数据,head->next指向第一个数据,head->num保存总索引（也可以设计将第一个节点存入head）
        //注意链表元的2个组成部分，元和next指针，元控制的是整个链表元的地址，next指针则寄生在元下；
        //printf("PushData: create head: obj->data_list=%x \n", obj->data_list);
        head = (FrameNode *)calloc(1, sizeof(FrameNode));  //创建头节点。
        head->num = 0;
        head->width = width;
        head->height = height;
        head->next = NULL;  //头节点指针域置NULL
        head->tail = head;  // 开始时尾指针指向头节点；此时head->tail->next与head->next是同一地址
        obj->frameListHead = (void *)head;
    }
    head = (FrameNode *)obj->frameListHead;
    if(head->num > MAX_DELAY_FRAME_NUM)
    {
        q = head->next;
        head->next = head->next->next;
        head->num--;
        if(head->next == NULL)
        {
            //printf("FramePopData: head->next is null \n");
            head->next = NULL;  //头节点指针域置NULL
            head->tail = head;
        }
        free(q->data);
        free(q);
        //free(data);
        printf("render: FramePushData: skip head->num=%d \n", head->num);
    }
    else{
        pnew = (FrameNode *)calloc(1, sizeof(FrameNode));  //创建新节点
        pnew->data = (uint8_t *)data;
        pnew->width = width;
        pnew->height = height;
        pnew->idx = head->idx;
        pnew->id = id;
        pnew->frame_timestamp = frame_timestamp;
        pnew->now_time = now_time;
        pnew->next = NULL;   //新节点指针域置NULL
        head->tail->next = pnew;  //新节点插入到表尾；首个节点时，则意味着ead->next也指向了pnew
        head->tail = pnew;   //为指针指向当前的尾节点;元地址迁移，元的原先的next脱离了元，新地址的next成为元的next指针；
        head->idx++;
        head->num++;
        //printf("FramePushData: head->num=%d \n", head->num);
    }
}
void * FramePopData2(FrameBufferNode *obj)
{
    void *ret = NULL;
    FrameNode *head, *q;
    head = (FrameNode *)obj->frameListHead;
    if(head)
    {
        q = head->next;
        if(q == NULL || q == head)
        {

        }
        else{
            head->next = head->next->next;
            ret = q;
            head->num--;
            if(head->next == NULL)
            {
                head->next = NULL;  //头节点指针域置NULL
                head->tail = head;
            }
        }
    }
    return ret;
}
FrameBufferNode * FrameBufferPushData(CallRender *obj, uint8_t *data, int width, int height, int id, int64_t frame_timestamp, int64_t now_time)
{
    //pthread_mutex_lock(&obj->lock);
    FrameBufferNode *ret = NULL;
    FrameBufferNode *head,*pnew, *q;
    if(!obj->frameBufferHead)
    {
        //head不存数据,head->next指向第一个数据,head->num保存总索引（也可以设计将第一个节点存入head）
        //注意链表元的2个组成部分，元和next指针，元控制的是整个链表元的地址，next指针则寄生在元下；
        //printf("PushData: create head: obj->data_list=%x \n", obj->data_list);
        head = (FrameBufferNode *)calloc(1, sizeof(FrameBufferNode));  //创建头节点。
        head->num = 0;
        //head->width = width;
        //head->height = height;
        head->idx = 0;
        head->next = NULL;  //头节点指针域置NULL
        head->tail = head;  // 开始时尾指针指向头节点；此时head->tail->next与head->next是同一地址
        obj->frameBufferHead = (void *)head;
    }
    head = (FrameBufferNode *)obj->frameBufferHead;
#if 0
    if(head->num > 128)
    {
        q = head->next;
        //printf("FrameBufferPushData: q=%x \n", q);
        while(q)
        {
            if(q->id == id)
            {
                FrameNode * p = FramePopData2(q);
                if(p && p->data)
                {
                    free(p->data);
                    free(p);
                }
                break;
            }
            q = q->next;
        }
        //free(data);
        printf("render: FrameBufferPushData: skip head->num=%d \n", head->num);
    }
#endif
    {
        q = head->next;
        int renewflag = 0;
        while(q)
        {
            if(q->id == id)
            {
                FramePushData2(q, data, width, height, id, frame_timestamp, now_time);
                renewflag = 1;
                ret = q;
                break;
            }
            q = q->next;
        }
        if(!renewflag)
        {
            pnew = (FrameBufferNode *)calloc(1, sizeof(FrameBufferNode));  //创建新节点
            //pnew->data = (uint8_t *)data;
            pnew->idx = head->idx;
            pnew->id = id;
            //printf("FramePopData: pnew->base_timestamp=%lld \n", pnew->base_timestamp);
            //printf("FramePopData: pnew->last_timestamp=%lld \n", pnew->last_timestamp);
            //pnew->init_timestamp = 0;//  # 防止漂移
            //pnew->init_start_time = 0;//  # 防止漂移
            //pnew->base_timestamp = 0;
            //pnew->base_start_time = 0;
            //pnew->time_offset = 0;
            //pnew->last_timestamp = 0;
            //pnew->search_count = 0;
            //pnew->audio_timestamp = 0;
            //pnew->audio_start_time = 0;
            //pnew->audio_frequence = 0;
            FramePushData2(pnew, data, width, height, id, frame_timestamp, now_time);
            pnew->next = NULL;   //新节点指针域置NULL
            head->tail->next = pnew;  //新节点插入到表尾；首个节点时，则意味着ead->next也指向了pnew
            head->tail = pnew;   //为指针指向当前的尾节点;元地址迁移，元的原先的next脱离了元，新地址的next成为元的next指针；
            head->idx++;
            head->num++;
            ret = pnew;
            printf("render: FramePushData: head->num=%d, id=%d \n", head->num, id);
        }


    }
    //pthread_mutex_unlock(&obj->lock);
    return ret;
}
void * FrameBufferPopData(CallRender *obj)
{
    void *ret = NULL;
    //pthread_mutex_lock(&obj->lock);
    FrameBufferNode *head, *q;
    head = (FrameBufferNode *)obj->frameBufferHead;
    if(head)
    {
        q = head->next;
        if(q == NULL || q == head)
        {

        }
        else{
            head->next = head->next->next;
            ret = q;
            head->num--;
            if(head->next == NULL)
            {
                head->next = NULL;  //头节点指针域置NULL
                head->tail = head;
            }
        }
    }
    //pthread_mutex_unlock(&obj->lock);
    return ret;
}
void * FrameBufferFinder(CallRender *obj, int id)
{
    void *ret = NULL;
    FrameBufferNode *head, *q;
    head = (FrameBufferNode *)obj->frameBufferHead;
    if(head)
    {
        q = head->next;
        while(q)
        {
            if(q->id == id)
            {
                ret = q;
                break;
            }
            q = q->next;
        }
    }
    return ret;
}
int RenderInit(CallRender *obj)
{
    int ret = 0;
    cJSON *json = NULL;

    ret = api_sdl_init(obj->handle, obj->params);
    if(json)
    {
        api_json_free(json);
        json = NULL;
    }

    return ret;
}
int RenderStop(CallRender *obj)
{
    int ret = 0;
    printf("RenderStop \n");
    if(obj)
    {
        api_sdl_stop(obj->handle);
    }
    printf("RenderStop over \n");
    return ret;
}
int is_mux(SDL_Rect * a, SDL_Rect *b)
{
    int x1 = a->x;
    int y1 = a->y;
    int w2 = a->w;
    int h2 = a->h;
    int x2 = x1 + a->w;
    int y2 = y1 + a->h;
    //
    int x3 = b->x;
    int y3 = b->y;
    int w4 = b->w;
    int h4 = b->h;
    int x4 = x3 + b->w;
    int y4 = y3 + b->h;
    // sum_area = 0
    int minx = x1 > x3 ? x1 : x3;//  # max(x1,x3)
    int miny = y1 > y3 ? y1 : y3;//  # max(y1,y3)
    int maxx = x2 < x4 ? x2 : x4;//  # min(x2,x4)
    int maxy = y2 < y4 ? y2 : y4;//  # min(y2,y4)
    //
    //# 最小的中的最大的还比最大的中的最小的要小就相离，否则相交
    if ((maxx < minx) || (maxy < miny))
    {
        //# 要判断是否相离，如果相离直接输出两个矩形面积的和
        return 0;
    }
    int t0 = abs((x1 - x2) * (y1 - y2));
    int t1 = abs((x3 - x4) * (y3 - y4));
    int t = t0 + t1;
    int s = abs(t);  //# 矩形相交的最大面积，也是矩形相离的面积
    int s0 = w2 * h2;
    int s1 = w4 * h4;
    float rat0 = (float)(s0) / s;
    float rat1 = (float)(s1) / s;
    int ret = 1 + (rat0 > rat1);//1:a被b遮挡；2：a遮挡b
    return ret;
}

int GetLayerId(SDL_Rect *a, int *pFatherId, int i, SDL_Rect *rec_tab, int num, int layerId)
{
    for(int j = 0; j < num; j++)
    {
        //优化：记录a的id，使得b不再与所有父调用的a进行比较
        if(pFatherId[j])
        {
            continue;
        }
        if(j != i)
        {
            SDL_Rect *b = &rec_tab[j];
            int mux = is_mux(a, b);
            if(mux == 2)
            {
                layerId++;
                pFatherId[i] = 1;
                //被a遮挡的b，是否也遮挡其他，如果是，层数继续增加
                layerId = GetLayerId(b, pFatherId, j, rec_tab, num, layerId);
            }
        }

    }
    return layerId;
}
int CreateMultRect(MultRect **pRect, SDL_Rect *rec_tab, int num)
{
    int ret = 0;
    *pRect = (MultRect *)calloc(num, sizeof(MultRect));
    for(int i = 0; i < num; i++)
    {
        (*pRect)[i].chanInfo.chanId = i;
        (*pRect)[i].layerId = 0;
        (*pRect)[i].rect = rec_tab[i];
    }

    for(int i = 0; i < num; i++)
    {
        SDL_Rect *a = &rec_tab[i];
        int layerId = 0;
        int *pFatherId = (int *)calloc(num, sizeof(int));
        layerId = GetLayerId(a, pFatherId, i, rec_tab, num, layerId);
        (*pRect)[i].layerId = layerId;
        free(pFatherId);
        if(layerId > ret)
        {
            ret = layerId;
        }
    }

    for(int i = 0; i < num; i++)
    {
        int layerId = (*pRect)[i].layerId;
        printf("CreateMultRect: i=%d, layerId=%d \n", i, layerId);
    }
    printf("CreateMultRect: ret=%d \n", ret);
    return ret;
}
SDL_Rect RectMap(MultRect **pRect, int modeId, int id, int w, int h, int *num, int *maxLayerId)
{
    SDL_Rect ret;
    int factor = 4;
    int w1 = (w / factor);
    int h1 = (h / factor);
    int w3 = (w / 3);
    int h3 = (h / 3);
    int w4 = w >> 2;
    int h4 = h >> 2;
    int w8 = w >> 3;
    int h8 = h >> 3;
    switch(modeId)
    {
        case 0:
        {
            SDL_Rect rec_tab[] = {
                {0, 0, w, h}
            };
            ret = rec_tab[id];
            num[0] = 1;
            if(!(*pRect))
            {
                maxLayerId[0] = CreateMultRect(pRect, rec_tab, num[0]);
            }
            break;
        }
        case 1:
        {
            SDL_Rect rec_tab[] = {
                {w4, 0, (w >> 1), (h >> 1)},
                {w4, (h >> 1), (w >> 1), (h >> 1)}
            };
            ret = rec_tab[id];
            num[0] = 2;
            if(!(*pRect))
            {
                maxLayerId[0] = CreateMultRect(pRect, rec_tab, num[0]);
            }
            break;
        }
        case 2:
        {
            SDL_Rect rec_tab[] = {
                {0, h4, (w >> 1), (h >> 1)},
                {(w >> 1), h4, (w >> 1), (h >> 1)}
            };
            ret = rec_tab[id];
            num[0] = 2;
            if(!(*pRect))
            {
                maxLayerId[0] = CreateMultRect(pRect, rec_tab, num[0]);
            }
            break;
        }
        case 3:
        {
            SDL_Rect rec_tab[] = {
                {0, 0, w, h},
                {0, 0, w1, h1}
            };
            ret = rec_tab[id];
            num[0] = 2;
            if(!(*pRect))
            {
                maxLayerId[0] = CreateMultRect(pRect, rec_tab, num[0]);
            }
            break;
        }
        case 4:
        {
            SDL_Rect rec_tab[] = {
                {0, 0, w, h},
                {(w - w1), 0, w1, h1}
            };
            ret = rec_tab[id];
            num[0] = 2;
            if(!(*pRect))
            {
                maxLayerId[0] = CreateMultRect(pRect, rec_tab, num[0]);
            }
            break;
        }
        case 5:
        {
            SDL_Rect rec_tab[] = {
                 {0, 0, w, h},
                 {0, (h - h1), w1, h1}
            };
            ret = rec_tab[id];
            num[0] = 2;
            if(!(*pRect))
            {
                maxLayerId[0] = CreateMultRect(pRect, rec_tab, num[0]);
            }
            break;
        }
        case 6:
        {
            SDL_Rect rec_tab[] = {
                {0, 0, w, h},
                {(w - w1), (h - h1), w1, h1}
            };
            ret = rec_tab[id];
            num[0] = 2;
            if(!(*pRect))
            {
                maxLayerId[0] = CreateMultRect(pRect, rec_tab, num[0]);
            }
            break;
        }
        case 7:
        {
            SDL_Rect rec_tab[] = {
                {w1, 0, (w - w1), h},
                {0, 0, w1, (h1 << 1)},
                {0, (h1 << 1), w1, (h1 << 1)}
            };
            ret = rec_tab[id];
            num[0] = 3;
            if(!(*pRect))
            {
                maxLayerId[0] = CreateMultRect(pRect, rec_tab, num[0]);
            }
            break;
        }
        case 8:
        {
            SDL_Rect rec_tab[] = {
                {0, 0, (w1 << 1), (h1 << 1)},
                {(w1 << 1), 0, (w1 << 1), (h1 << 1)},
                {0, (h1 << 1), (w1 << 1), (h1 << 1)},
                {(w1 << 1), (h1 << 1), (w1 << 1), (h1 << 1)}
            };
            ret = rec_tab[id];
            num[0] = 4;
            if(!(*pRect))
            {
                maxLayerId[0] = CreateMultRect(pRect, rec_tab, num[0]);
            }
            break;
        }
        case 9:
        {
            SDL_Rect rec_tab[] = {
                {w1, 0, (w - w1), h},
                {0, 0, w1, (h3)},
                {0, (h3), w1, (h3)},
                {0, (2 * h3), w1, (h3)}
            };
            ret = rec_tab[id];
            num[0] = 4;
            if(!(*pRect))
            {
                maxLayerId[0] = CreateMultRect(pRect, rec_tab, num[0]);
            }
            break;
        }
        case 10:
        {
            SDL_Rect rec_tab[] = {
                {0, 0, w, h},
                {0, 0, w1, h1},
                {0, h1, w1, h1},
                {0, (2 * h1), w1, h1},
                {0, (3 * h1), w1, h1}};
                ret = rec_tab[id];
                num[0] = sizeof(rec_tab) / sizeof(SDL_Rect);
                if(!(*pRect))
                {
                    maxLayerId[0] = CreateMultRect(pRect, rec_tab, num[0]);
                }
            break;
        }
        case 11:
        {
            SDL_Rect rec_tab[] = {
                {0, 0, w, h},
                {(w - w1), 0, w1, h1},
                {(w - w1), h1, w1, h1},
                {(w - w1), (2 * h1), w1, h1},
                {(w - w1), (3 * h1), w1, h1}};
                ret = rec_tab[id];
                num[0] = sizeof(rec_tab) / sizeof(SDL_Rect);
                if(!(*pRect))
                {
                    maxLayerId[0] = CreateMultRect(pRect, rec_tab, num[0]);
                }
            break;
        }
        case 12:
        {
            SDL_Rect rec_tab[] = {
                {0, 0, w, h},
                {0, (h - h1), w1, h1},
                {w1, (h - h1), w1, h1},
                {2 * w1, (h - h1), w1, h1},
                {3 * w1, (h - h1), w1, h1}};
                ret = rec_tab[id];
                num[0] = sizeof(rec_tab) / sizeof(SDL_Rect);
                if(!(*pRect))
                {
                    maxLayerId[0] = CreateMultRect(pRect, rec_tab, num[0]);
                }
            break;
        }
        case 13:
        {
            SDL_Rect rec_tab[] = {
                {0, 0, w, h},
                {0, 0, w1, h1},
                {w1, 0, w1, h1},
                {2 * w1, 0, w1, h1},
                {3 * w1, 0, w1, h1}};
                ret = rec_tab[id];
                num[0] = sizeof(rec_tab) / sizeof(SDL_Rect);
                if(!(*pRect))
                {
                    maxLayerId[0] = CreateMultRect(pRect, rec_tab, num[0]);
                }
            break;
        }
        case 14:
        {
            SDL_Rect rec_tab[] = {
                {0, 0, w, h},
                {0, 0, w1, h1},
                {0, h1, w1, h1},
                {0, (2 * h1), w1, h1},
                {0, (h - h1), w1, h1},
                {w1, (h - h1), w1, h1},
                {2 * w1, (h - h1), w1, h1},
                {3 * w1, (h - h1), w1, h1}};
                ret = rec_tab[id];
                num[0] = sizeof(rec_tab) / sizeof(SDL_Rect);
                if(!(*pRect))
                {
                    maxLayerId[0] = CreateMultRect(pRect, rec_tab, num[0]);
                }
            break;
        }
        case 15:
        {
            SDL_Rect rec_tab[] = {
                {0, 0, w, h},
                {(w - w1), 0, w1, h1},
                {(w - w1), h1, w1, h1},
                {(w - w1), (2 * h1), w1, h1},
                {0, (h - h1), w1, h1},
                {w1, (h - h1), w1, h1},
                {2 * w1, (h - h1), w1, h1},
                {3 * w1, (h - h1), w1, h1}};
                ret = rec_tab[id];
                num[0] = sizeof(rec_tab) / sizeof(SDL_Rect);
                if(!(*pRect))
                {
                    maxLayerId[0] = CreateMultRect(pRect, rec_tab, num[0]);
                }
            break;
        }
        case 16:
        {
            SDL_Rect rec_tab[] = {
                {0, 0, w, h},
                {0, 0, w1, h1},
                {w1, 0, w1, h1},
                {2 * w1, 0, w1, h1},
                {3 * w1, 0, w1, h1},
                {0, h1, w1, h1},
                {0, (2 * h1), w1, h1},
                {0, (3 * h1), w1, h1}};
                ret = rec_tab[id];
                num[0] = sizeof(rec_tab) / sizeof(SDL_Rect);
                if(!(*pRect))
                {
                    maxLayerId[0] = CreateMultRect(pRect, rec_tab, num[0]);
                }
            break;
        }
        case 17:
        {
            SDL_Rect rec_tab[] = {
                {0, 0, w, h},
                {0, 0, w1, h1},
                {w1, 0, w1, h1},
                {2 * w1, 0, w1, h1},
                {3 * w1, 0, w1, h1},
                {(w - w1), h1, w1, h1},
                {(w - w1), (2 * h1), w1, h1},
                {(w - w1), (3 * h1), w1, h1}};
                ret = rec_tab[id];
                num[0] = sizeof(rec_tab) / sizeof(SDL_Rect);
                if(!(*pRect))
                {
                    maxLayerId[0] = CreateMultRect(pRect, rec_tab, num[0]);
                }
            break;
        }
        case 18:
        {
            SDL_Rect rec_tab[] = {
                {     0,        0, w3, h3},
                {    w3,        0, w3, h3},
                {2 * w3,        0, w3, h3},
                {     0,    h3, w3, h3},
                {    w3,    h3, w3, h3},
                {2 * w3,    h3, w3, h3},
                {     0,2 * h3, w3, h3},
                {    w3,2 * h3, w3, h3},
                {2 * w3,2 * h3, w3, h3}};
                ret = rec_tab[id];
                num[0] = sizeof(rec_tab) / sizeof(SDL_Rect);
                if(!(*pRect))
                {
                    maxLayerId[0] = CreateMultRect(pRect, rec_tab, num[0]);
                }
            break;
        }
        case 19:
        {
            SDL_Rect rec_tab[] = {
                {     0, 0, w4, h4},
                {    w4, 0, w4, h4},
                {2 * w4, 0, w4, h4},
                {3 * w4, 0, w4, h4},
                {     0, h4, w4, h4},
                {    w4, h4, w4, h4},
                {2 * w4, h4, w4, h4},
                {3 * w4, h4, w4, h4},
                {     0, 2 * h4, w4, h4},
                {    w4, 2 * h4, w4, h4},
                {2 * w4, 2 * h4, w4, h4},
                {3 * w4, 2 * h4, w4, h4},
                {     0, 3 * h4, w4, h4},
                {    w4, 3 * h4, w4, h4},
                {2 * w4, 3 * h4, w4, h4},
                {3 * w4, 3 * h4, w4, h4}};
                ret = rec_tab[id];
                num[0] = sizeof(rec_tab) / sizeof(SDL_Rect);
                if(!(*pRect))
                {
                    maxLayerId[0] = CreateMultRect(pRect, rec_tab, num[0]);
                }
            break;
        }
        case 20:
        {
            SDL_Rect rec_tab[] = {
                {     0, 0, w,  h},
                {     0, 0, w8, h8},
                {1 * w8, 0, w8, h8},
                {2 * w8, 0, w8, h8},
                {3 * w8, 0, w8, h8},
                {4 * w8, 0, w8, h8},
                {5 * w8, 0, w8, h8},
                {6 * w8, 0, w8, h8},
                {7 * w8, 0, w8, h8},
                {7 * w8, 1 * h8, w8, h8},
                {7 * w8, 2 * h8, w8, h8},
                {7 * w8, 3 * h8, w8, h8},
                {7 * w8, 4 * h8, w8, h8},
                {7 * w8, 5 * h8, w8, h8},
                {7 * w8, 6 * h8, w8, h8},
                {7 * w8, 7 * h8, w8, h8}};
                ret = rec_tab[id];
                num[0] = sizeof(rec_tab) / sizeof(SDL_Rect);
                if(!(*pRect))
                {
                    maxLayerId[0] = CreateMultRect(pRect, rec_tab, num[0]);
                }
            break;
        }
        default:
            break;
    }
    return ret;
}
int AddRect(CallRender *obj, SDL_Rect rect, int id)
{
    int ret = 0;
    pthread_mutex_lock(&obj->lock);
    //cJSON *json = api_str2json(params);
    //if(json)
    {
        //SDL_Rect rect;
        //rect.x = GetvalueInt(json, "rect_x");
        //rect.y = GetvalueInt(json, "rect_y");
        //rect.w = GetvalueInt(json, "rect_w");
        //rect.h = GetvalueInt(json, "rect_h");
        //int id = GetvalueInt(json, "id");
        RectNode *head,*pnew, *q;
        if(!obj->rectHead)
        {
            //head不存数据,head->next指向第一个数据,head->num保存总索引（也可以设计将第一个节点存入head）
            //注意链表元的2个组成部分，元和next指针，元控制的是整个链表元的地址，next指针则寄生在元下；
            //printf("PushData: create head: obj->data_list=%x \n", obj->data_list);
            head = (RectNode *)calloc(1, sizeof(RectNode));  //创建头节点。
            head->num = 0;
            head->idx = 0;
            head->next = NULL;  //头节点指针域置NULL
            head->tail = head;  // 开始时尾指针指向头节点；此时head->tail->next与head->next是同一地址
            obj->rectHead = (void *)head;
        }
        head = (RectNode *)obj->rectHead;
        q = head->next;
        int renewflag = 0;
        while(q)
        {
            if(q->id == id)
            {
                q->rect = rect;//renew;
                renewflag = 1;
                break;
            }
            q = q->next;
        }
        if(!renewflag)
        {
            pnew = (RectNode *)calloc(1, sizeof(RectNode));  //创建新节点
            pnew->rect = rect;
            pnew->id = id;
            pnew->idx = head->idx;
            pnew->next = NULL;   //新节点指针域置NULL
            head->tail->next = pnew;  //新节点插入到表尾；首个节点时，则意味着ead->next也指向了pnew
            head->tail = pnew;   //为指针指向当前的尾节点;元地址迁移，元的原先的next脱离了元，新地址的next成为元的next指针；
            head->idx++;
            head->num++;
        }
        //api_json_free(json);
        //json = NULL;
    }
    pthread_mutex_unlock(&obj->lock);
    return ret;
}
SDL_Rect * GetRect(CallRender *obj, int id)
{
    SDL_Rect *ret = NULL;
    pthread_mutex_lock(&obj->lock);
    RectNode *head, *q;
    head = (RectNode *)obj->rectHead;
    q = head->next;
    while(q)
    {
        if(q->id == id)
        {
            ret = &q->rect;
            break;
        }
        q = q->next;
    }
    pthread_mutex_unlock(&obj->lock);
    return ret;
}
void PollRect(CallRender *obj)
{
    pthread_mutex_lock(&obj->lock);
    //MultRect *pRect = (MultRect *)calloc(num, sizeof(MultRect));
    printf("PollRect: obj->pMultLayer->ways=%d  \n", obj->pMultLayer->ways);
    MultRect Rect0;// = obj->pMultLayer->pRect[0];
    for(int i = 1; i < obj->pMultLayer->ways; i++)
    {
        MultRect *p0 = (MultRect *)&obj->pMultLayer->pRect[i];
        int I = (i + 1) % obj->pMultLayer->ways;
        MultRect *p1 = (MultRect *)&obj->pMultLayer->pRect[I];
        //char *data0 = p0->chanInfo.data;
        //char *data1 = p1->chanInfo.data;
        ChanInfo chanInfo0 = p0->chanInfo;
        ChanInfo chanInfo1 = p1->chanInfo;
        Rect0 = *p1;
        *p1 = *p0;
        *p0 = Rect0;
        p0->chanInfo = chanInfo0;
        p1->chanInfo = chanInfo1;
        //
#if 0
        if(!data0)
        {
            if(p0->chanInfo.data)
            {
                //该通道原先无数据，则交换后依然保持无数据
                free(p0->chanInfo.data);
                p0->chanInfo.data = NULL;
            }
        }
        if(!data1)
        {
            if(p1->chanInfo.data)
            {
                //该通道原先无数据，则交换后依然保持无数据
                free(p1->chanInfo.data);
                p1->chanInfo.data = NULL;
            }
        }
#endif
    }
    for(int i = 0; i < obj->pMultLayer->ways; i++)
    {
        MultRect *p0 = (MultRect *)&obj->pMultLayer->pRect[i];
        //各通道的数据均不是该通道该有的数据
#if 0
        for(int j = 0; j < obj->pMultLayer->ways; j++)
        {
            MultRect *p1 = (MultRect *)&obj->pMultLayer->pRect[j];
            if(i == p1->chanInfo.chanId)
            {
                ChanInfo chanInfo0 = p0->chanInfo;
                p0->chanInfo = p1->chanInfo;
                p1->chanInfo = chanInfo0;
            }
        }
#endif
#if 0
        MultRect *p0 = (MultRect *)&obj->pMultLayer->pRect[i];
        p0->chanInfo.chanId = i;
        p0->chanInfo.width = 0;
        p0->chanInfo.height = 0;
        if(p0->chanInfo.data)
        {
            free(p0->chanInfo.data);
            p0->chanInfo.data = NULL;
        }
#endif
        printf("PollRect: i=%d, p0->chanInfo.chanId=%d  \n", i, p0->chanInfo.chanId);
    }
    api_sdl_clear(obj->handle);
    //free(obj->pMultLayer->pRect);
    //obj->pMultLayer->pRect = pRect;
    pthread_mutex_unlock(&obj->lock);
}

void PollRect2(CallRender *obj)
{
    pthread_mutex_lock(&obj->lock);

    //MultRect *pRect = (MultRect *)calloc(num, sizeof(MultRect));
    printf("PollRect: obj->pMultLayer->ways=%d  \n", obj->pMultLayer->ways);
    SDL_Rect rect0;// = obj->pMultLayer->pRect[0];
    int layerId0;// = Rect0.chanInfo;
    for(int i = 1; i < obj->pMultLayer->ways; i++)
    {
        MultRect *p0 = (MultRect *)&obj->pMultLayer->pRect[i];
        int I = (i + 1) % obj->pMultLayer->ways;
        MultRect *p1 = (MultRect *)&obj->pMultLayer->pRect[I];
        //
        rect0 = p1->rect;
        p1->rect = p0->rect;
        p0->rect = rect0;
        //
        layerId0 = p1->layerId;
        p1->layerId = p0->layerId;
        p0->layerId = layerId0;
    }

    for(int i = 0; i < obj->pMultLayer->ways; i++)
    {
        MultRect *p0 = (MultRect *)&obj->pMultLayer->pRect[i];
        printf("PollRect: i=%d, p0->chanInfo.chanId=%d  \n", i, p0->chanInfo.chanId);
    }
    //free(obj->pMultLayer->pRect);
    //obj->pMultLayer->pRect = pRect;
    pthread_mutex_unlock(&obj->lock);
}

HCSVC_API
int api_poll_rect(char *handle, int taskId)
{
    int ret = 0;
    long long *testp = (long long *)handle;
    TaskPool *task = (TaskPool *)testp[0];
    SocketObj *sock = (SocketObj *)task->hnd[taskId];
    if(sock)
    {
        SessionObj *session = (SessionObj *)sock->session;
        if(session)
        {
            CallRender *obj = (CallRender *)session->render;
            PollRect(obj);
        }
    }
    return ret;
}
HCSVC_API
int api_reset_rect(char *handle, int taskId, int modeId, int w, int h)
{
    int ret = 0;
    long long *testp = (long long *)handle;
    TaskPool *task = (TaskPool *)testp[0];
    SocketObj *sock = (SocketObj *)task->hnd[taskId];
    if(sock)
    {
        SessionObj *session = (SessionObj *)sock->session;
        if(session)
        {
            CallRender *obj = (CallRender *)session->render;
            pthread_mutex_lock(&obj->lock);
#if 0
            if(obj->rectHead)
            {
                RectNode *head = (RectNode *)obj->rectHead;
                if(head && head->num)
                {
                    do{
                        RectNode *q;
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
                        free(q);   //释放节点i的内存单元
                    }while(1);
                    free(head);
                }
                obj->rectHead = NULL;
            }
#endif
            printf("api_reset_rect: free bj->pMultLayer->pRect \n");
            if(obj->pMultLayer->pRect)
            {
                for(int i = 0; i < obj->pMultLayer->ways; i++)
                {
                    MultRect *p = (MultRect *)&obj->pMultLayer->pRect[i];
                    if(p)
                    {
                        if(p->chanInfo.data)
                        {
                            free(p->chanInfo.data);
                            p->chanInfo.data = NULL;
                        }
                    }
                }
                free(obj->pMultLayer->pRect);
                obj->pMultLayer->pRect = NULL;
            }
            printf("api_reset_rect: RectMap \n");
            int chanId = 0;
            int num = 0;
            int maxLayerId = 0;
            //do{
            //    SDL_Rect rect = RectMap(&obj->pMultLayer->pRect, modeId, chanId, w, h, &num, &maxLayerId);
            //    printf("api_reset_rect: num=%d \n", num);
            //    AddRect(obj, rect, chanId);
            //    chanId++;
            //}while(chanId < num);
            SDL_Rect rect = RectMap(&obj->pMultLayer->pRect, modeId, chanId, w, h, &num, &maxLayerId);

            if(obj->pMultLayer->pRect)
            {
                obj->pMultLayer->modeId = modeId;
                obj->pMultLayer->ways = num;
                obj->pMultLayer->maxLayerId = maxLayerId;
                //obj->pMultLayer->pRect = (MultRect *)calloc(num, sizeof(MultRect));
                printf("api_reset_rect: obj->pMultLayer->maxLayerId=%d \n", obj->pMultLayer->maxLayerId);
            }
            pthread_mutex_unlock(&obj->lock);
            printf("api_reset_rect: ok \n");

        }
    }
    return ret;
}

int MultLayerRenderData(CallRender *obj, char *data, int width0, int height0, int id, int64_t now_time)
{
    int ret = 0;
    //1)当有遮挡发生时，取出被遮挡的历史数据，按层重新绘制;
    //2)不分辨，全路重画;
    if (api_sdl_status(obj->handle) == 0)
    {
        pthread_mutex_lock(&obj->lock);

        MultRect *pRect = obj->pMultLayer->pRect;
        if(pRect[id].chanInfo.data)
        {
            free(pRect[id].chanInfo.data);
        }
        pRect[id].chanInfo.data = data;
        pRect[id].chanInfo.width = width0;
        pRect[id].chanInfo.height = height0;
        int show_flag = 0;
        if(!obj->pMultLayer->last_frame_time_stamp)
        {
            obj->pMultLayer->last_frame_time_stamp = now_time;
        }
        int difftime = (int)(now_time - obj->pMultLayer->last_frame_time_stamp);
        //printf("MultLayerRenderData: id=%d, width=%d, height=%d \n", id, width, height);
        //printf("MultLayerRenderData: difftime=%d \n", difftime);
        //printf("MultLayerRenderData: obj->pMultLayer->modeId=%d \n", obj->pMultLayer->modeId);
        //printf("MultLayerRenderData: obj->pMultLayer->maxLayerId=%d \n", obj->pMultLayer->maxLayerId);
        //printf("MultLayerRenderData: obj->pMultLayer->ways=%d \n", obj->pMultLayer->ways);
        if(difftime >= 20)// || !obj->pMultLayer->last_frame_time_stamp)
        {
            int show_flag = 1;
            for(int i = obj->pMultLayer->maxLayerId; i >= 0 ; i--)
            {
                for(int j = 0;  j < obj->pMultLayer->ways; j++)
                {
                    int layerId = pRect[j].layerId;
                    if(layerId == i)
                    {
                        MultRect *p =  &pRect[j];
                        if(p->chanInfo.data)
                        {

                            int width = p->chanInfo.width;
                            int height = p->chanInfo.height;
                            if(i != obj->pMultLayer->maxLayerId)
                            {
                                show_flag = 0;
                            }
                            ret = api_render_data(obj->handle, p->chanInfo.data, (void *)&p->rect, show_flag, width, height);
                            show_flag = 0;
                        }
                    }
                }
            }
            pthread_mutex_unlock(&obj->lock);
            //
            show_flag = 2;
#if 1
            McuObj *mcu = (McuObj *)get_mcu((SocketObj *)(obj->sock));
            //printf("MultLayerRenderData: mcu=%x \n", mcu);
            if(mcu)
            {
                CallCodecVideo *codec = (CallCodecVideo *)mcu->video;
                CodecInfoObj *config = (CodecInfoObj *)&codec->codecInfo;
                int width = config->width;
                int height = config->height;
                //uint8_t *data = malloc(obj->frame_size * sizeof(uint8_t));
                uint8_t *data = av_malloc(obj->frame_size * sizeof(uint8_t));

                api_render_data(obj->handle, data, NULL, show_flag, width, height);

                McuPushData(mcu, data, obj->frame_size, now_time, kIsVideo);
            }
            else{
                api_render_data(obj->handle, NULL, NULL, show_flag, 0, 0);
            }
#else
            api_render_data(obj->handle, NULL, NULL, show_flag, 0, 0);
#endif
            //show_flag = 1;
            obj->pMultLayer->last_frame_time_stamp = now_time;
            pthread_mutex_lock(&obj->lock);
        }
        //
        pthread_mutex_unlock(&obj->lock);
    }
    return ret;
}
#if 0
int RenderData(CallRender *obj, char *data, int id, int show_flag)
{
    int ret = 0;
    if (api_sdl_status(obj->handle) == 0)
    {
        SDL_Rect *rect = GetRect(obj, id);
        if(rect)
        {
            int data_stride = 0;
            ret = api_render_data(obj->handle, data, (void *)rect, show_flag, width, height);
        }
    }
    return ret;
}
#endif
//注意：避免将编码抖动保留到播放中
int render_run(CallRender *obj)
{
    int ret = 0;
    pthread_mutex_lock(&obj->status_lock);
    obj->recv_status = 1;
    int status = obj->recv_status;
    pthread_mutex_unlock(&obj->status_lock);
    while(status > 0)
    {
        int64_t now_time = get_sys_time();//api_get_time_stamp_ll();
#ifdef OPEN_PACED_DISPLAY
        //item = self.PopQueue()
        FrameNode *item = (FrameNode *)FramePopData(obj);
        if(item)
        {
            int64_t recv_time = item->frame_timestamp;

            //printf("render_run: FrameBufferFinder: item->id=%d \n", item->id);
            FrameBufferNode *thisFrmBuf = (FrameBufferNode *)FrameBufferFinder(obj, item->id);
            //printf("render_run: thisFrmBuf=%x \n", thisFrmBuf);
            if(!thisFrmBuf)
            {
                //thisFrmBuf = FrameBuffer(id)
                //self.FrameList.append(thisFrmBuf)
                thisFrmBuf = FrameBufferPushData(obj, item->data, item->width, item->height, item->id, recv_time, now_time);
                //printf("render_run: thisFrmBuf=%x \n", thisFrmBuf);
            }
            if(thisFrmBuf->last_timestamp && recv_time < thisFrmBuf->last_timestamp)
            {
                //thisFrmBuf->time_offset += (1L << 32) - 1;
                printf("warning: render_run: thisFrmBuf->last_timestamp=%lld \n", thisFrmBuf->last_timestamp);
                printf("warning: render_run: recv_time=%lld \n", recv_time);
            }
            if(thisFrmBuf->base_timestamp == 0)
            {
                thisFrmBuf->base_timestamp = recv_time;
                thisFrmBuf->init_timestamp = recv_time;
            }
            else
            {
            }
            if(thisFrmBuf->base_start_time == 0)
            {
                //printf("render_run: thisFrmBuf->base_start_time=%d \n", thisFrmBuf->base_start_time);
                thisFrmBuf->base_start_time = now_time;
                thisFrmBuf->init_start_time = now_time;
                //self.play_right_now3(data, id, show_flag)
                //int show_flag = 1;
                //RenderData(obj, item->data, item->id, show_flag);
                //free(item->data);
                //free(item);
            }
            else
            {
                //int difftime = (int)(recv_time + thisFrmBuf->time_offset - thisFrmBuf->base_timestamp) / 90;//  # (ms)
                int difftime = (int)(recv_time + thisFrmBuf->time_offset - thisFrmBuf->base_timestamp);
                int64_t pre_play_time = thisFrmBuf->base_start_time + difftime;
                int delay = (int)(now_time - thisFrmBuf->base_start_time) - difftime;  //# (ms)
                // 理想状态delay == 0
                // 延迟到达delay > 0
                // 早到delay < 0
                if(delay >= 0)
                {
#if 0
                    if(thisFrmBuf->frameListHead->num == 0)
                    {
                        //if ((delay) > 100) and False:
                        //    thisFrmBuf.base_timestamp = recvTime
                        //    thisFrmBuf.base_start_time = now_time

                        //# show right now ???
                        //self.play_right_now3(data, id, show_flag)
                        int show_flag = 1;
                        MultLayerRenderData(obj, item->data, item->width, item->height, item->id, now_time);
                        //free(item->data);
                        //free(item);
                    }
                    else{
                        //thisFrmBuf.framelist.append((data, recvTime, now_time))
                        FramePushData2(thisFrmBuf, item->data, item->width, item->height, item->id, recv_time, now_time);
                    }
#else
                    FramePushData2(thisFrmBuf, item->data, item->width, item->height, item->id, recv_time, now_time);
                    FrameNode *item2 = (FrameNode *)FramePopData2(thisFrmBuf);
                    if(item2)
                    {
                        int show_flag = 1;
                        MultLayerRenderData(obj, item2->data, item2->width, item2->height, item2->id, now_time);
                        //free(item2->data);
                        free(item2);
                    }
#endif
                }
                else
                {
                    if(thisFrmBuf->frameListHead->num > 0)
                    {
                        //printf("render_run: thisFrmBuf->frameListHead->num=%d \n", thisFrmBuf->frameListHead->num);
                        FrameNode *first_frame = thisFrmBuf->frameListHead->next;
                        //printf("render_run: first_frame=%x \n", first_frame);
                        int64_t recvTime0 = first_frame->frame_timestamp;
                        //int difftime = (int)(recvTime0 + thisFrmBuf->time_offset - thisFrmBuf->base_timestamp) / 90;//  # (ms)
                        int difftime = (int)(recvTime0 + thisFrmBuf->time_offset - thisFrmBuf->base_timestamp);
                        int64_t pre_play_time0 = thisFrmBuf->base_start_time + difftime;//  # (ms)
                        int delay0 = (int)(now_time - pre_play_time0);
                        //printf("render_run: delay0=%d \n", delay0);
                        if(delay0 >= 0)
                        {
                            //self.play_right_now3(first_frame[0], id, show_flag)
                            //del thisFrmBuf.framelist[0]
                            FrameNode *item2 = (FrameNode *)FramePopData2(thisFrmBuf);
                            if(item2)
                            {
                                int show_flag = 1;
                                MultLayerRenderData(obj, item2->data, item2->width, item2->height, item2->id, now_time);
                                //free(item2->data);
                                free(item2);
                            }

                        }
                        //thisFrmBuf.framelist.append((data, recvTime, now_time))
                        FramePushData2(thisFrmBuf, item->data, item->width, item->height, item->id, recv_time, now_time);
                        //printf("render_run: thisFrmBuf->frameListHead->num=%d \n", thisFrmBuf->frameListHead->num);
                    }
                    else{
                        if ((-delay) > 100)
                        {
                            thisFrmBuf->base_timestamp = recv_time;
                            thisFrmBuf->base_start_time = now_time;
                            thisFrmBuf->search_count += 1;

                            //self.play_right_now3(data, id, show_flag)
                            int show_flag = 1;
                            MultLayerRenderData(obj, item->data, item->width, item->height, item->id, now_time);
                            //free(item->data);
                            //free(item);
                        }
                        else{
                            //thisFrmBuf.framelist.append((data, recvTime, now_time))
                            FramePushData2(thisFrmBuf, item->data, item->width, item->height, item->id, recv_time, now_time);
                        }

                    }
                }
                int n = thisFrmBuf->frameListHead->num;
                if(n > 10){
                    MYPRINT("render_run: n=%d \n", n);
                    FrameNode *first_frame = thisFrmBuf->frameListHead->next;
                    FrameNode *last_frame = thisFrmBuf->frameListHead->tail;
                    int difftime0 = (int)(last_frame->frame_timestamp - first_frame->frame_timestamp);
                    int difftime1 = (int)(last_frame->now_time - first_frame->now_time);//now_time1 - now_time0
                    if(difftime0 > 100 || difftime1 > 100)
                    {
                        //print("ShowThread: run: (id, n, difftime0, difftime1)= ",(id, n, difftime0, difftime1))
                    }
                }
            }
            thisFrmBuf->last_timestamp = recv_time;
            //printf("render_run 1 \n");
            free(item);
            //printf("render_run 2 \n");
        }
        else{
            int flag = 0;
            // 循环检测，不另外使用定时器
            FrameBufferNode *head, *q;
            head = (FrameBufferNode *)obj->frameBufferHead;
            //printf("render_run: head=%x \n", head);
            if(head)
            {
                q = head->next;
                int renewflag = 0;
                //for thisFrmBuf in self.FrameList:
                //    n = len(thisFrmBuf.framelist)
                //    if n > 0:
                while(q)
                {
                    FrameBufferNode *thisFrmBuf = q;
                    //printf("render_run: q->frameListHead=%x \n", thisFrmBuf->frameListHead);
                    int n = thisFrmBuf->frameListHead->num;
                    if(n)
                    {
                        //printf("render_run: n=%d \n", n);
                        FrameNode *first_frame = thisFrmBuf->frameListHead->next;
                        FrameNode *last_frame = thisFrmBuf->frameListHead->tail;
                        //printf("render_run: first_frame=%x \n", first_frame);
                        //printf("render_run: last_frame=%x \n", last_frame);
                        int64_t recvTime0 = first_frame->frame_timestamp;
                        int64_t recvTime1 = last_frame->frame_timestamp;
                        int64_t saveTime0 = first_frame->now_time;
                        int64_t saveTime1 = last_frame->now_time;
                        //int difftime0 = (int)(recvTime0 + thisFrmBuf->time_offset - thisFrmBuf->base_timestamp) / 90;//  # (ms)
                        int difftime0 = (int)(recvTime0 + thisFrmBuf->time_offset - thisFrmBuf->base_timestamp);
                        int64_t pre_play_time0 = thisFrmBuf->base_start_time + difftime0;//  # (ms)
                        //int difftime1 = (int)(recvTime1 + thisFrmBuf->time_offset - thisFrmBuf->base_timestamp) / 90;//  # (ms)
                        int difftime1 = (int)(recvTime1 + thisFrmBuf->time_offset - thisFrmBuf->base_timestamp);
                        int64_t pre_play_time1 = thisFrmBuf->base_start_time + difftime1;//  # (ms)
                        int64_t now_time_ms = now_time;//time.time() * 1000
                        //# pre_play_time1 = pre_play_time1 if pre_play_time1 > now_time_ms else now_time_ms
                        int strideTime = (int)(pre_play_time1 - pre_play_time0);//  # 分配播放跨度
                        int interval = strideTime / n;
                        int delay0 = (pre_play_time0 - interval) - now_time_ms;//

                        //printf("render_run: delay0=%d \n", delay0);
                        if(delay0 <= 0)//  # 过点了
                        {
                            //self.play_right_now3(first_frame[0], thisFrmBuf.id, show_flag)
                            //del thisFrmBuf.framelist[0]
                            //printf("render_run: delay0=%d \n", delay0);
                            FrameNode *item2 = (FrameNode *)FramePopData2(thisFrmBuf);
                            flag = 1;
                            if(item2)
                            {
                                int show_flag = 1;
                                MultLayerRenderData(obj, item2->data, item2->width, item2->height, item2->id, now_time);
                                //free(item2->data);
                                free(item2);
                            }
                        }
                    }
                    q = q->next;
                }
            }
            if(!flag)
            {
                //int64_t time1 = get_sys_time();
                usleep(10000);//10ms
                //int64_t time2 = get_sys_time();
                //int difftime = (int)(time2 - time1);
                //printf("render_run: difftime=%d (ms) \n", difftime);
            }
        }
#else
        //show right now
        FrameNode *item = (FrameNode *)FramePopData(obj);
        if(item)
        {
            //int64_t time2 = get_sys_time();
            //int difftime = (int)(time2 - now_time);
            //printf("render_run: 1: difftime=%d (ms) \n", difftime);
            //printf("render_run: item->id=%d  \n", item->id);
            //self.play_right_now3(first_frame[0], thisFrmBuf.id, show_flag)
            int id = item->id;
            uint8_t *data = item->data;
            //int64_t frame_timestamp = item->frame_timestamp;
            //int64_t now_time = item->now_time;
            //int show_flag = 1;
            //RenderData(obj, data, id, show_flag);
            MultLayerRenderData(obj, data, item->width, item->height, id, now_time);
            //free(data);
            free(item);
            //time2 = get_sys_time();
            //difftime = (int)(time2 - now_time);
            //printf("render_run: 2: difftime=%d (ms) \n", difftime);
        }
        else{
            int64_t time1 = get_sys_time();
            usleep(10000);//10ms
            int64_t time2 = get_sys_time();
            int difftime = (int)(time2 - time1);
            //printf("render_run: difftime=%d (ms) \n", difftime);
        }
#endif
        pthread_mutex_lock(&obj->status_lock);
        status = obj->recv_status;// & obj->send_status;
        pthread_mutex_unlock(&obj->status_lock);
    }
    RenderStop(obj);
    printf("render_run: over \n");
    char *p = malloc(32);
    strcpy(p,"render_run over");
    pthread_exit((void*)p);

    return ret;
}
void release_framenode(FrameNode *head)
{
    if(head && head->num)
    {
        do{
            FrameNode *q;
            q = head->next;
            if(q == NULL || q == head)
            {
                break;
            }
            else{
                head->next = head->next->next;
            }
            //
            if(q->data)
            {
                free(q->data);
            }
            free(q);   //释放节点i的内存单元
        }while(1);
        free(head);
    }
}
void release_node(CallRender *obj)
{
    if(obj->rectHead)
    {
        RectNode *head = (RectNode *)obj->rectHead;
        if(head && head->num)
        {
            do{
                RectNode *q;
                q = head->next;
                if(q == NULL || q == head)
                {
                    break;
                }
                else{
                    head->next = head->next->next;
                }
                //
                free(q);   //释放节点i的内存单元
            }while(1);
            free(head);
        }
    }
    if(obj->frameListHead)
    {
        FrameNode *head = (FrameNode *)obj->frameListHead;
        release_framenode(head);
    }
    if(obj->frameBufferHead)
    {
        FrameBufferNode *head = (FrameBufferNode *)obj->frameBufferHead;
        if(head && head->num)
        {
            do{
                FrameBufferNode *q;
                q = head->next;
                if(q == NULL || q == head)
                {
                    break;
                }
                else{
                    head->next = head->next->next;
                }
                //
                release_framenode(q->frameListHead);
                free(q);   //释放节点i的内存单元
            }while(1);
            free(head);
        }
    }
}

int set_mcu2render(SocketObj *sock, McuObj *mcu)
{
    int ret = 0;
    pthread_mutex_lock(&sock->status_lock);
    SessionObj * session = (SessionObj *)sock->session;
    session->mcu = mcu;
    pthread_mutex_unlock(&sock->status_lock);
    return ret;
}
void * get_mcu(SocketObj *sock)
{
    void *ret = NULL;
    pthread_mutex_lock(&sock->status_lock);
    SessionObj * session = (SessionObj *)sock->session;
    if(session)
    {
        ret = (void *)session->mcu;
    }
    pthread_mutex_unlock(&sock->status_lock);
    return ret;
}
void * video_render(SocketObj *sock)
{
    int taskId = sock->id;
    sock->status = 1;
    pthread_mutex_init(&sock->status_lock,NULL);
    SessionObj *session = (SessionObj *)calloc(1, sizeof(SessionObj));
    sock->session = (void *)session;
    CallRender *obj = (CallRender *)calloc(1, sizeof(CallRender));
    session->render = obj;
    obj->sock = (void *)sock;
    obj->params = sock->params;
    obj->pMultLayer = (MultLayer *)calloc(1, sizeof(MultLayer));
    obj->pMultLayer->last_frame_time_stamp = 0;
    obj->pMultLayer->pRect = NULL;
    cJSON * json = (cJSON *)api_str2json(sock->params);
    int w = GetvalueInt(json, "pixel_w");
    int h = GetvalueInt(json, "pixel_h");
    obj->frame_size = (w * h * 3) >> 1;

    RenderInit(obj);
    pthread_mutex_init(&obj->lock,NULL);
    pthread_mutex_init(&obj->status_lock,NULL);

    pthread_t tid;
    if(pthread_create(&tid, NULL, render_run, obj) < 0)
    {
        printf("video_render: Create render_run failed!\n");
    }

    api_sdl_show_run(obj->handle);

    pthread_mutex_lock(&obj->status_lock);
    obj->recv_status = 0;
    pthread_mutex_unlock(&obj->status_lock);

    char *p0;
    if (pthread_join(tid, (void**)&p0))
    {
        printf("video_render: render_run thread is not exit...\n");
    }
    else{
        printf("video_render: p0=%s \n", p0);
        free(p0);
    }

    api_sdl_close(obj->handle);

    printf("video_render: free obj->pMultLayer \n");
    if(obj->pMultLayer)
    {
        if(obj->pMultLayer->pRect)
        {
            for(int i = 0; i < obj->pMultLayer->ways; i++)
            {
                MultRect *p = (MultRect *)&obj->pMultLayer->pRect[i];
                if(p)
                {
                    if(p->chanInfo.data)
                    {
                        free(p->chanInfo.data);
                    }
                }
            }
            free(obj->pMultLayer->pRect);
        }
        free(obj->pMultLayer);
    }
    printf("video_render: free obj->pMultLayer ok \n");
    release_node(obj);
    pthread_mutex_destroy(&obj->lock);
    pthread_mutex_destroy(&obj->status_lock);
    free(obj);
    if(session)
    {
        free(session);
    }
    pthread_mutex_destroy(&sock->status_lock);
    sock->status = -1;
    printf("video_render: over: taskId=%d \n", taskId);
    return 0;
}
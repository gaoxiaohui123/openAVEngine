
#include "inc.h"

extern int GetvalueInt(cJSON *json, char *key);
extern char* GetvalueStr(cJSON *json, char *key, char *result);
extern int64_t get_sys_time();
int glob_sld_status = 0;
FILE *yuvfp = NULL;

int test_sdl_0(char * show_buffer, int width, int height, int delay)
{
	//------------SDL----------------
	int screen_w,screen_h;
	SDL_Window *screen;
	SDL_Renderer* sdlRenderer;
	SDL_Texture* sdlTexture;
	SDL_Rect sdlRect;
	
 
    	printf("create screen \n");
	//SDL 2.0 Support for multiple windows
	screen_w = 640;//1280;
	screen_h = 480;//720;
	screen = SDL_CreateWindow("Simplest ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screen_w, screen_h, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);//

	if(!screen) {
		printf("SDL: could not create window - exiting:%s\n",SDL_GetError());
		return -1;
	}
	printf("screen=%x \n", screen);
	sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
	//IYUV: Y + U + V  (3 planes)
	//YV12: Y + V + U  (3 planes)
	sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,width,height);

	sdlRect.x=0;
	sdlRect.y=0;
	sdlRect.w=width;
	sdlRect.h=height;
	//------------SDL End------------
	//Event Loop

	
	//SDL---------------------------
	SDL_UpdateTexture( sdlTexture, NULL, show_buffer, width );
	SDL_RenderClear( sdlRenderer );
	//SDL_RenderCopy( sdlRenderer, sdlTexture, &sdlRect, &sdlRect );
	SDL_RenderCopy( sdlRenderer, sdlTexture, NULL, NULL);
	SDL_RenderPresent( sdlRenderer );
	//SDL End-----------------------
	SDL_Delay(delay);

	SDL_Quit();
	
	return 0;
}
int test_sdl_1(char * show_buffer, int width, int height, int delay)
{
	//------------SDL----------------
	int screen_w,screen_h;
	SDL_Window *screen;
	SDL_Renderer* sdlRenderer;
	SDL_Texture* sdlTexture;
	SDL_Rect sdlRect;
	
 
    	printf("create screen \n");
	//SDL 2.0 Support for multiple windows
	screen_w = 1280;
	screen_h = 720;
	screen = SDL_CreateWindow("Simplest ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screen_w, screen_h, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);//

	if(!screen) {
		printf("SDL: could not create window - exiting:%s\n",SDL_GetError());
		return -1;
	}
	printf("screen=%x \n", screen);
	sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
	//IYUV: Y + U + V  (3 planes)
	//YV12: Y + V + U  (3 planes)
	sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,640,480);

	sdlRect.x=0;
	sdlRect.y=0;
	sdlRect.w=width >> 1;
	sdlRect.h=height >> 1;
	//------------SDL End------------
	//Event Loop

	
	//SDL---------------------------
	SDL_UpdateTexture( sdlTexture, NULL, show_buffer, width );
	SDL_RenderClear( sdlRenderer );
	SDL_RenderCopy( sdlRenderer, sdlTexture, &sdlRect, &sdlRect );
	//SDL_RenderCopy( sdlRenderer, sdlTexture, NULL, &sdlRect );
	//SDL_RenderCopy( sdlRenderer, sdlTexture, NULL, NULL);
	SDL_RenderPresent( sdlRenderer );
	//SDL End-----------------------
	SDL_Delay(delay);

	SDL_Quit();
	
	return 0;
}
HCSVC_API
int api_test_sdl(char * show_buffer, int width, int height, int delay)
{
	return test_sdl_1(show_buffer, width, height, delay);
}


#if 1
//int screen_w=500,screen_h=500;
//const int pixel_w=320,pixel_h=240;

//const int bpp=12;
//static unsigned char *show_buffer = (unsigned char *)calloc(1, pixel_w*pixel_h*bpp/8);
//Refresh Event
#define REFRESH_EVENT  (SDL_USEREVENT + 1)
//Break
#define BREAK_EVENT  (SDL_USEREVENT + 2)

//int thread_exit=0;
//int thread_pause=0;
#endif

typedef struct
{
    int Obj_id;
    SDL_Window *screen;
    SDL_Surface *screen2;
    SDL_Renderer* sdlRenderer;
    SDL_Texture* sdlTexture;
    SDL_Event event;
    SDL_Rect rect;
    SDL_Thread *refresh_thread;
    pthread_mutex_t mutex;
    cJSON *json;
    char *param;
    char name[64];
    int pixformat;
    int screen_w;
    int screen_h;
    int pixel_w;
    int pixel_h;
    float scalex;
    float scaley;
    int bpp;
    int wait_time;
    int frame_size;
    int thread_exit;
    int thread_pause;
    int win_id;
    //
    char *osd_handle;
    int handle_size;
    int osd_enable;
	int orgX;
	int orgY;
	int scale;
	int color;
	int64_t start_time;
    int64_t frame_num;
    int64_t last_frame_num;
	int interval;
    int frame_rate;
    int print;
    unsigned char *show_buffer;
}SDLObj;
//定时刷新模式，未启用
int refresh_video(void *handle){
    if(!handle)
    {
        return -1;
    }
    long long *testp = (long long *)handle;
    SDLObj *obj = (SDLObj *)testp[0];
    printf("refresh_video: obj->Obj_id= %x \n", obj->Obj_id);
    int wait_time = obj->wait_time;
    pthread_mutex_lock(&obj->mutex);
    obj->thread_exit=0;
    while (obj->thread_exit == 0)
    {
        if(obj->thread_pause==0)
        {
            pthread_mutex_unlock(&obj->mutex);
            SDL_Event event;
            event.type = REFRESH_EVENT;
            SDL_PushEvent(&event);
            pthread_mutex_lock(&obj->mutex);
        }
        pthread_mutex_unlock(&obj->mutex);
        SDL_Delay(wait_time);
        pthread_mutex_lock(&obj->mutex);
    }
    obj->thread_exit = 0;
    pthread_mutex_unlock(&obj->mutex);
    //Break
    SDL_Event event;
    event.type = BREAK_EVENT;
    SDL_PushEvent(&event);
    return 0;
}
HCSVC_API
int api_create_sdl_handle(char *handle)
{
    long long ret = 0;
    if(handle)
    {
        SDLObj *obj = (SDLObj *)calloc(1, sizeof(SDLObj));
        //CodecObj *obj = (CodecObj *)&global_codec_objs[id];
        ret = (long long)obj;
        //handle = (void *)obj;
        int handle_size = sizeof(long long);
        printf("api_create_sdl_handle: handle_size= %d \n", handle_size);
        printf("api_create_sdl_handle: obj= %x \n", obj);
        printf("api_create_sdl_handle: ret= %x \n", ret);
        memcpy(handle, &ret, handle_size);
        long long *testp = (long long *)handle;
        printf("api_create_sdl_handle: testp[0]= %x \n", testp[0]);
        int id = (int)obj;//long long ???
        obj->Obj_id = id;
        ret = id;
    }
    return (int)ret;
}
#if 0
HCSVC_API
int split_screen2(char *handle, char *show_buffer, SDL_Rect rect, int width, int show_flag)
{
    int ret = 0;
    int w = 1920;
    int h = 1080;
    if(handle)
    {
        int64_t time0 = get_sys_time();
        long long *testp = (long long *)handle;
        SDLObj *obj = (SDLObj *)testp[0];
        //
        SDL_Surface *father_surface = SDL_GetWindowSurface(obj->screen);
        SDL_Surface* pic = SDL_LoadBMP("1.bmp");//SDL_SetVideoMode(w, h, 0, 0);
        //
        if ( pic == NULL ) {
            printf("无法加载 %s\n", SDL_GetError());
            return -1;
        }
#if 0
        SDL_Surface *pic0;
        /* Blit到屏幕surface。onto the screen surface.
            这时不能锁住surface。
        */
        SDL_Rect dest;
        dest.x = x;
        dest.y = y;
        dest.w = image->w;
        dest.h = image->h;
        SDL_BlitSurface(pic, NULL, pic0, &dest);

        /* 刷新屏幕的变化部分 */
        SDL_UpdateRects(pic0, 1, &dest);
#endif
        //
        SDL_Texture* texture = SDL_CreateTextureFromSurface(obj->sdlRenderer, pic);
        int64_t time1 = get_sys_time();
	    int difftime = (int)(time1 - time0);
	    //if(difftime > 20)
	    {
	        printf("split_screen: SDL_CreateTextureFromSurface: difftime= %d \n", difftime);
	    }
	    SDL_RenderCopy( obj->sdlRenderer, texture, NULL, &rect);
	    if(show_flag)
        {
            //显示
            SDL_RenderPresent( obj->sdlRenderer );
	        //SDL_Delay(1000);
	        time1 = get_sys_time();
	        difftime = (int)(time1 - time0);
	        //if(difftime > 20)
	        {
	            printf("split_screen: SDL_RenderPresent: show_flag=%d, difftime= %d \n", show_flag, difftime);
	        }
        }

	    //SDL_FreeSurface(pic);
    }

    return ret;
}
#endif

int split_screen(char *handle, char *show_buffer, SDL_Rect rect, int width, int height,int show_flag)
{
    //注意： obj->sdlTexture在初始化时与obj->pixel_w进行了绑定
    if(handle)
    {
        int64_t time0 = get_sys_time();
        long long *testp = (long long *)handle;
        SDLObj *obj = (SDLObj *)testp[0];
        //printf("split_screen \n");
        //memcpy(obj->show_buffer, show_buffer, obj->frame_size);
        //SDL_RenderClear( obj->sdlRenderer );
        if(!obj->start_time)
        {
            obj->start_time = api_get_time_stamp_ll();
            obj->last_frame_num = obj->frame_num;
            obj->interval = 1500;//1s
        }
        int64_t now = api_get_time_stamp_ll();
        int difftime = (int)(now - obj->start_time);
        //printf("osd_process: 0 \n");
        if(difftime > obj->interval)
        {
            float sum_frame_num = (float)(obj->frame_num - obj->last_frame_num);
            obj->frame_rate = (int)((sum_frame_num * 1000.0) / (float)difftime + 0.5);

            obj->start_time = now;
            obj->last_frame_num = obj->frame_num;
        }
        if(show_flag == 1)
        {
	        if(obj->osd_enable)
	        {
	            //printf("split_screen: width=%d, height=%d \n", width, height);
	            if(!obj->osd_handle)
	            {
	                obj->handle_size = 8;
	                obj->osd_handle = (char *)calloc(1, obj->handle_size * sizeof(char));
	                //obj->pCodecCtx->width//obj->pCodecCtx->height
                    //obj->json = api_renew_json_int(obj->json, "width", obj->pixel_w);
                    //obj->json = api_renew_json_int(obj->json, "height", obj->pixel_h);
                    obj->json = api_renew_json_int(obj->json, "width", width);
                    obj->json = api_renew_json_int(obj->json, "height", height);

                    obj->json = api_renew_json_int(obj->json, "orgX", obj->orgX);
                    obj->json = api_renew_json_int(obj->json, "orgY", obj->orgY);
                    obj->json = api_renew_json_int(obj->json, "scale", obj->scale);
                    obj->json = api_renew_json_int(obj->json, "color", obj->color);
                    obj->json = api_renew_json_str(obj->json, "src_pix_fmt", "AV_PIX_FMT_YUV420P");

                    char* jsonstr2 = api_json2str(obj->json);
                    int ret2 = api_simple_osd_init(obj->osd_handle, jsonstr2);
                    api_json2str_free(jsonstr2);
	            }
	            int framerate = obj->frame_rate;
	            //确保show_buffer内存大小与（obj->pixel_w, obj->pixel_h）一致
                if(framerate > 0)
                {

                    int orgX = 0;
                    int orgY = height - (height / 16);
                    orgY = (height / 8);
                    char context[255] = "";
                    obj->json = api_renew_json_int(obj->json, "width", width);
                    obj->json = api_renew_json_int(obj->json, "height", height);
                    obj->json = api_renew_json_int(obj->json, "orgX", orgX);
                    obj->json = api_renew_json_int(obj->json, "orgY", orgY);
                    //sprintf(&context[strlen(context)], " %d", obj->pixel_w);
                    //sprintf(&context[strlen(context)], "x%d", obj->pixel_h);
                    sprintf(&context[strlen(context)], " %d", width);
                    sprintf(&context[strlen(context)], "x%d", height);
                    sprintf(&context[strlen(context)], " %2d", framerate);
                    //strcat(context, "  ");
                    //strcat(context, obj->input_format);
                    obj->json = api_renew_json_str(obj->json, "context", context);
                    char* jsonstr2 = api_json2str(obj->json);
                    int ret2 = api_simple_osd_process(  obj->osd_handle,
                                                        (char *)show_buffer,
                                                        jsonstr2);
                    api_json2str_free(jsonstr2);
                    //printf("api_capture_read_frame2: ret2= %d \n", ret2);
                }
	        }

	        //obj->frame_num++;
        }
#if 0
        int pitch = SDL_BYTESPERPIXEL(obj->pixformat) * width;
        //printf("split_screen: width=%d, pitch=%d, obj->pixel_w=%d \n", width, pitch, obj->pixel_w);
        //SDL_SetTextureBlendMode(obj->sdlTexture, SDL_BLENDMODE_BLEND);
        //SDL_UpdateTexture( obj->sdlTexture, &rect, show_buffer, width);
        SDL_UpdateTexture( obj->sdlTexture, NULL, show_buffer, pitch);//wxh整个纹理刷新
#elif(0)
        //目前只支持固定区域rect0,不支持可变区域
        SDL_Rect rect0;
        rect0.x = 0;
        rect0.y = 0;
        rect0.w = width;//obj->pixel_w;
        rect0.h = height;//obj->pixel_h;
        //rect0.w = 640;//test
        //rect0.h = 480;//test
        int pitch = SDL_BYTESPERPIXEL(obj->pixformat) * width;//一行像素数据的字节数
        //printf("split_screen: width=%d, pitch=%d, obj->pixel_w=%d \n", width, pitch, obj->pixel_w);
        SDL_UpdateTexture( obj->sdlTexture, &rect0, show_buffer, pitch);//rect0区域局部纹理刷新
#else
        int w = width;//obj->pixel_w;
        int h = height;//obj->pixel_h;
        //int frame_size = w * h * obj->bpp / 8;
        //SDL_Rect *rect2 = &rect;//NULL;
        SDL_Rect rect0;
        rect0.x = 0;
        rect0.y = 0;
        rect0.w = width;
        rect0.h = height;
        char *src_y = show_buffer;
        char *src_u = &show_buffer[w * h];
        char *src_v = &show_buffer[w * h + (w >> 1) * (h >> 1)];
        int linesize[3] = { width,
                            width >> 1,
                            width >> 1};
        SDL_UpdateYUVTexture(   obj->sdlTexture, &rect0,
                                src_y,
                                linesize[0],
                                src_u,
                                linesize[1],
                                src_v,
                                linesize[2]);
#endif
        //SDL_RenderCopy( sdlRenderer, sdlTexture, &rect, &rect );
        int64_t time1 = get_sys_time();
	    difftime = (int)(time1 - time0);
	    if(obj->print)//(difftime > 20)
	    {
	        printf("split_screen: SDL_UpdateTexture: show_flag=%d, difftime= %d \n", show_flag, difftime);
	    }
	    float scalex = obj->scalex;
	    float scaley = obj->scaley;
	    if(scalex != 1.0 || scaley != 1.0)
	    {

	        int x = rect.x;
	        int y = rect.y;
	        int w = rect.w;
	        int h = rect.h;
	        rect.x = (int)(x * scalex);
	        rect.y = (int)(y * scaley);
	        rect.w = (int)(w * scalex);
	        rect.h = (int)(h * scaley);
	    }
#if 0
        int w = obj->pixel_w;
        int h = obj->pixel_h;
        SDL_Rect rect3;
        rect3.x = 0;
        rect3.y = 0;
        rect3.w = w;
        rect3.h = h;
        SDL_RenderCopy( obj->sdlRenderer, obj->sdlTexture, NULL, &rect3);
#elif(1)

        SDL_RenderCopy( obj->sdlRenderer, obj->sdlTexture, &rect0, &rect);//将rect0的纹理拷贝到rect区域(缩小)
#else
        //SDL_SetRenderDrawBlendMode(obj->sdlRenderer, SDL_BLENDMODE_BLEND);
        SDL_RenderCopy( obj->sdlRenderer, obj->sdlTexture, NULL, &rect);//将wxh的纹理拷贝到rect区域(缩小)
#endif

        time1 = get_sys_time();
	    difftime = (int)(time1 - time0);
	    if(obj->print)//(difftime > 20)
	    {
	        printf("split_screen: SDL_RenderCopy: show_flag=%d, difftime= %d \n", show_flag, difftime);
	    }

#if 0
        if(obj->thread_pause == 0)
        {
            SDL_Event event;
            event.type = REFRESH_EVENT;
            SDL_PushEvent(&event);
        }
#else
        if(show_flag == 2)
        {

#if 0
            int w = obj->pixel_w;
            int h = obj->pixel_h;
            SDL_Rect rect3;
            rect3.x = 0;
            rect3.y = 0;
            rect3.w = w;
            rect3.h = h;
            SDL_RenderReadPixels(   obj->sdlRenderer,
                                    NULL,//&rect3,
                                    obj->pixformat,
                                    show_buffer,
                                    width);
            if(!yuvfp)
            {
                yuvfp = fopen("out.yuv", "wb");
            }
            if(yuvfp)
            {
                int frame_size = (w * h * 3) >> 1;
                fwrite(show_buffer, 1, frame_size, yuvfp);
            }
#endif
            //显示
            SDL_RenderPresent( obj->sdlRenderer );

            //SDL_Delay(1);
	        time1 = get_sys_time();
	        difftime = (int)(time1 - time0);
	        if(obj->print)//(difftime > 20)
	        {
	            printf("split_screen: SDL_RenderPresent: show_flag=%d, difftime= %d \n", show_flag, difftime);
	        }
            //SDL_RenderClear( obj->sdlRenderer );
            obj->frame_num++;
        }

#endif

    }
    return 0;
}

HCSVC_API
int api_split_screen(char *handle, char * show_buffer, char *param, int width, int height)
{
    SDL_Rect rect;
    cJSON *json = (cJSON *)api_str2json(param);

    int show_flag = GetvalueInt(json, "show_flag");
    if(show_flag == 2 && handle)
    {
        long long *testp = (long long *)handle;
        SDLObj *obj = (SDLObj *)testp[0];

        SDL_RenderPresent( obj->sdlRenderer );
        obj->frame_num++;
        return 0;
    }
    rect.x = GetvalueInt(json, "rect_x");
    rect.y = GetvalueInt(json, "rect_y");
    rect.w = GetvalueInt(json, "rect_w");
    rect.h = GetvalueInt(json, "rect_h");
    int ret = split_screen(handle, show_buffer, rect, width, height, show_flag);
    api_json_free(json);
    return ret;
}
#if 0
static int adapt_scale(struct SwsContext **ctx, uint8_t *src, int src_w, int src_h, uint8_t *dst, int dst_w, int dst_h)
{
    int ret = 0;
    if(!*ctx)
    {
        *ctx = sws_getContext(  src_w,
	                            src_h,
	                            AV_PIX_FMT_YUV420P,
	                            dst_w,
	                            dst_h,
	                            AV_PIX_FMT_YUV420P,
	                            SWS_FAST_BILINEAR,
	                            NULL, NULL, NULL);
    }
    if(*ctx)
    {
        AVFrame src_frame = {};//注意：初始化至关重要，否则会导致异常崩溃
        src_frame.data[0] = src;
        src_frame.data[1] = &src[size];
        src_frame.data[2] = &src[size + (size >> 2)];
        src_frame.linesize[0] = src_w;
        src_frame.linesize[1] = src_w >> 1;
        src_frame.linesize[2] = src_w >> 1;
        src_frame.width = src_w;
	    src_frame.height = src_h;
        src_frame.format = AV_PIX_FMT_YUV420P;


        AVFrame dst_frame = {};//注意：初始化至关重要，否则会导致异常崩溃
        dst_frame.data[0] = dst;
        dst_frame.data[1] = &dst[size4];
        dst_frame.data[2] = &dst[size4 + (size4 >> 2)];
        dst_frame.linesize[0] = dst_w;
        dst_frame.linesize[1] = dst_w >> 1;
        dst_frame.linesize[2] = dst_w >> 1;
        dst_frame.width = dst_w;
	    dst_frame.height = dst_h;
        dst_frame.format = AV_PIX_FMT_YUV420P;
        sws_scale(  *ctx,
	                src_frame.data,
			        src_frame.linesize, 0,
			        src_h,
			        dst_frame.data,
			        dst_frame.linesize);
    }
    return ret;
}
#endif
HCSVC_API
int api_render_data(char *handle, char *data, void *rect, int show_flag, int width, int height)
{
    long long *testp = (long long *)handle;
    SDLObj *obj = (SDLObj *)testp[0];

    //if(width == 640)
    //    return 0;
    //printf("api_render_data: width=%d, obj->pixel_w= %d \n", width, obj->pixel_w);
    if(show_flag == 2)
    {
        //SDL_RenderPresent( obj->sdlRenderer );
#if 1
        if(data)
        {
            //printf("api_render_data: obj->screen_w=%d, obj->pixel_w= %d \n", obj->screen_w, obj->pixel_w);
            //printf("api_render_data: width=%d, obj->pixel_w=%d, obj->screen_w=%d \n", width, obj->pixel_w, obj->screen_w);
            int w0 = 0;
            int h0 = 0;
            SDL_GetWindowSize(obj->screen, &w0, &h0);
            //printf("api_render_data: w0=%d, h0= %d \n", w0, h0);
            uint32_t pixformat = SDL_GetWindowPixelFormat(obj->screen);
            //printf("api_render_data: pixformat=%d, SDL_PIXELFORMAT_IYUV=%d, obj->pixformat= %d \n", pixformat, SDL_PIXELFORMAT_IYUV, obj->pixformat);
            int pitch0 = SDL_BYTESPERPIXEL(pixformat) * w0;
            //printf("api_render_data: pitch0=%d \n", pitch0);

            int w = width;//obj->pixel_w;//mcu_width
            int h = height;//obj->pixel_h;//mcu_height

            int pitch = SDL_BYTESPERPIXEL(obj->pixformat) * w;
            //pitch = SDL_BYTESPERPIXEL(obj->pixformat) * w0;
            //printf("api_render_data: pitch=%d \n", pitch);
            h0 = (h0 >> 1) << 1;
            SDL_Rect rect3;
            rect3.x = 0;
            rect3.y = 0;
            rect3.w = w0;//w0;//w;
            rect3.h = h0;//h0;//h;
#if 1
            int dy = 0, dx = 0;
            if(h0 < h)
            {
                dy = (h - h0);
            }
            if(w0 < w)
            {
                dx = (w - w0);
            }
            int offsety = dy >> 1;
            int offsetx = dx >> 1;
            if(dy || dx)
            {
                int size = w * h;//dst
                uint8_t *dsty = data;
                uint8_t *dstu = &data[size];
                uint8_t *dstv = &data[size + (size >> 2)];
                memset(dsty, 0, size);
                memset(dstu, 128, size >> 1);//uv
            }

#endif
            //不会进行拉伸
            //以pitch为跨度采集
            //但高度是h0,要适配h，y与uv的位置必需调整
            int ret = SDL_RenderReadPixels( obj->sdlRenderer,
                                            &rect3,
                                            obj->pixformat,
                                            &data[offsety * w + offsetx],
                                            //&data[offsetx],
                                            //data,
                                            pitch);
            if(dy)
            {
                int size0 = w * h0;//src
                int size = w * h;//dst
                int offsety2 = dy >> 2;
                int offsetx2 = dx >> 2;
                int w2 = w >> 1;
                int h2 = h >> 1;
                uint8_t *sy = data;
                uint8_t *su = &data[size0];
                uint8_t *sv = &data[size0 + (size0 >> 2)];
                uint8_t *dsty = data;
                uint8_t *dstu = &data[size];
                uint8_t *dstv = &data[size + (size >> 2)];

                memmove(&dstv[offsety2 * w2 + offsetx2], &sv[offsety * w + offsetx], (size0 >> 2));//先移动v分量
                memmove(&dstu[offsety2 * w2 + offsetx2], &su[offsety * w + offsetx], (size0 >> 2));
                //以黑色填充底边
                memset(&dsty[(h - offsety) * w], 0, (offsety * w));
                memset(&dstu[(h2 - offsety2) * w2], 128, (offsety2 * w2));
                memset(&dstv[(h2 - offsety2) * w2], 128, (offsety2 * w2));
            }
#if 0

            if(dy || dx)//dx暂未处理
            {

                int size0 = w0 * h0;//src
                int size = w * h;//dst
                uint8_t *sy = data;
                uint8_t *su = &data[size0];
                uint8_t *sv = &data[size0 + (size0 >> 2)];
                uint8_t *dy = data;
                uint8_t *du = &data[size];
                uint8_t *dv = &data[size + (size >> 2)];
                int w2 = w0 >> 1;
                int w3 = w >> 1;
                int offsetx3 = offsetx >> 1;
                //move v
                for(int i = h - 1; i >= (h - offsety); i -= 2)
                {
                    int i3 = i >> 1;
                    memset(&dv[i3 * w3], 128, w3);
                }
                for(int i = (h - offsety - 1); i >= offsety; i -= 2)
                {
                    int i3 = i >> 1;
                    int i2 = ((i - offsety) >> 1);
                    memcpy(&dv[i3 * w3 + offsetx3], &sv[i2 * w2], w2);
                }
                for(int i = (offsety - 1); i >= 0; i -= 2)
                {
                    int i3 = i >> 1;
                    memset(&dv[i3 * w3 + offsetx3], 128, w3);
                }
                //move u
                for(int i = h - 1; i >= (h - offsety); i -= 2)
                {
                    int i3 = i >> 1;
                    memset(&du[i3 * w3], 128, w3);
                }
                for(int i = (h - offsety - 1); i >= offsety; i -= 2)
                {
                    int i3 = i >> 1;
                    int i2 = ((i - offsety) >> 1);
                    memcpy(&du[i3 * w3 + offsetx3], &su[i2 * w2], w2);
                }
                for(int i = (offsety - 1); i >= 0; i -= 2)
                {
                    int i3 = i >> 1;
                    memset(&du[i3 * w3], 128, w3);
                }
                //move y
                for(int i = h - 1; i >= (h - offsety); i -= 1)
                {
                    memset(&dy[i * w], 0, w);
                }
                for(int i = (h - offsety - 1); i >= offsety; i -= 1)
                {
                    memcpy(&dy[i * w + offsetx], &sy[(i - offsety) * w0], w0);
                }
                for(int i = (offsety - 1); i >= 0; i -= 1)
                {
                    memset(&dy[i * w], 0, w);
                }
            }
#endif
            if(ret != 0)
            {
                fprintf(stderr, "api_render_data: %s\n", av_err2str(ret));
            }
            if(!yuvfp && false)
            {
                yuvfp = fopen("/home/gxh/works/out.yuv", "wb");
            }
            if(yuvfp)
            {
                //int frame_size = (w0 * h0 * 3) >> 1;
                int frame_size = (w * h * 3) >> 1;
                fwrite(data, 1, frame_size, yuvfp);
            }
        }
#endif
        SDL_RenderPresent( obj->sdlRenderer );
        obj->frame_num++;
        return 0;
    }

    int ret = split_screen(handle, data, *(SDL_Rect *)rect, width, height, show_flag);
    return ret;
}

HCSVC_API
int api_sdl_status(char *handle)
{
    int ret = -1;
    if(handle)
    {
        long long *testp = (long long *)handle;
        SDLObj *obj = (SDLObj *)testp[0];
        pthread_mutex_lock(&obj->mutex);
        ret = obj->thread_exit;
        pthread_mutex_unlock(&obj->mutex);
    }
    return ret;
}
HCSVC_API
void api_sdl_clear(char *handle)
{
    if(handle)
    {
        long long *testp = (long long *)handle;
        SDLObj *obj = (SDLObj *)testp[0];
        SDL_RenderClear( obj->sdlRenderer );
    }
}
HCSVC_API
void api_sdl_stop(char *handle)
{
    if(handle)
    {
        long long *testp = (long long *)handle;
        SDLObj *obj = (SDLObj *)testp[0];
        SDL_Event event;
        event.type = BREAK_EVENT;
        SDL_PushEvent(&event);
    }
}


HCSVC_API
void api_sdl_push_event(int id)
{
    SDL_Event event;
    event.type = REFRESH_EVENT;

    switch(id){
        case 0  :
            event.type = REFRESH_EVENT;
            break;
        case 1  :
            event.type = SDL_WINDOWEVENT;
            break;
        case 2  :
            event.type = SDL_KEYDOWN;
            break;
        case 3  :
            event.type = SDL_QUIT;
            break;
        case 4  :
            event.type = BREAK_EVENT;
            break;
        default :
            event.type = REFRESH_EVENT;
    }

    SDL_PushEvent(&event);
}
void PrintWinEvent(const SDL_Event * event)
{
    if (event->type == SDL_WINDOWEVENT) {
        switch (event->window.event) {
        case SDL_WINDOWEVENT_SHOWN:
            SDL_Log("Window %d shown", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_HIDDEN:
            SDL_Log("Window %d hidden", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_EXPOSED:
            SDL_Log("Window %d exposed", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_MOVED:
            SDL_Log("Window %d moved to %d,%d",
                    event->window.windowID, event->window.data1,
                    event->window.data2);
            break;
        case SDL_WINDOWEVENT_RESIZED:
            SDL_Log("Window %d resized to %dx%d",
                    event->window.windowID, event->window.data1,
                    event->window.data2);
            break;
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            SDL_Log("Window %d size changed to %dx%d",
                    event->window.windowID, event->window.data1,
                    event->window.data2);
            break;
        case SDL_WINDOWEVENT_MINIMIZED:
            SDL_Log("Window %d minimized", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_MAXIMIZED:
            SDL_Log("Window %d maximized", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_RESTORED:
            SDL_Log("Window %d restored", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_ENTER:
            SDL_Log("Mouse entered window %d",
                    event->window.windowID);
            break;
        case SDL_WINDOWEVENT_LEAVE:
            SDL_Log("Mouse left window %d", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            SDL_Log("Window %d gained keyboard focus",
                    event->window.windowID);
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            SDL_Log("Window %d lost keyboard focus",
                    event->window.windowID);
            break;
        case SDL_WINDOWEVENT_CLOSE:
            SDL_Log("Window %d closed", event->window.windowID);
            break;
#if SDL_VERSION_ATLEAST(2, 0, 5)
        case SDL_WINDOWEVENT_TAKE_FOCUS:
            SDL_Log("Window %d is offered a focus", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_HIT_TEST:
            SDL_Log("Window %d has a special hit test", event->window.windowID);
            break;
#endif
        default:
            SDL_Log("Window %d got unknown event %d",
                    event->window.windowID, event->window.event);
            break;
        }
    }
}
HCSVC_API
void api_sdl_show_run(char *handle)
{
    if(handle)
    {
        long long *testp = (long long *)handle;
        SDLObj *obj = (SDLObj *)testp[0];
        printf("sdl_show_run: obj->Obj_id= %x \n", obj->Obj_id);
        int frame_size = obj->pixel_w * obj->pixel_h * obj->bpp / 8;
        printf("sdl_show_run: frame_size= %d \n", frame_size);
        //
        SDL_Event event;
        while(1){
            //Wait
            //printf("sdl_show_run: wait 0 \n");
            SDL_WaitEvent(&event);
            //printf("sdl_show_run: wait 1 \n");
            if(event.type==REFRESH_EVENT){
                //显示
                printf("sdl_show_run: wait REFRESH_EVENT \n");
                SDL_RenderPresent( obj->sdlRenderer );
		        printf("api_sdl_show_run: REFRESH_EVENT \n");
                //延时20ms
                //SDL_Delay(20);
	        //}else if(event.type == SDL_ResizeEvent){
		        //printf("api_sdl_show_run: SDL_ResizeEvent \n");
            }else if(event.type == SDL_WINDOWEVENT){
                //If Resize
		        printf("api_sdl_show_run: SDL_WINDOWEVENT: event.type=%d \n", event.type);
                //PrintWinEvent(&event);
                int w = obj->screen_w;
                int h = obj->screen_h;
                SDL_GetWindowSize(obj->screen,&w, &h);
                if(w != obj->screen_w || h != obj->screen_h)
                {
                    obj->scalex = (float)w / (float)obj->screen_w;
                    obj->scaley = (float)h / (float)obj->screen_h;
                    printf("api_sdl_show_run: SDL_WINDOWEVENT: w=%d \n", w);
		            printf("api_sdl_show_run: SDL_WINDOWEVENT: h=%d \n", h);
                    printf("api_sdl_show_run: SDL_WINDOWEVENT: obj->screen_w=%d \n", obj->screen_w);
		            printf("api_sdl_show_run: SDL_WINDOWEVENT: obj->screen_h=%d \n", obj->screen_h);
                }

		        //SDL_Delay(1000);
           }else if(event.type == SDL_KEYDOWN){
                //Pause
		        printf("api_sdl_show_run: SDL_KEYDOWN \n");
                if(event.key.keysym.sym == SDLK_SPACE)
                {
                    if(obj->refresh_thread)//定时刷新模式
                    {
                        pthread_mutex_lock(&obj->mutex);
                        obj->thread_pause = !obj->thread_pause;
                        pthread_mutex_unlock(&obj->mutex);
                    }
                }

           }else if(event.type == SDL_QUIT){
		        printf("api_sdl_show_run: SDL_QUIT \n");
                if(obj->refresh_thread)//定时刷新模式
                {
                    pthread_mutex_lock(&obj->mutex);
                    obj->thread_exit = 1;
                    pthread_mutex_unlock(&obj->mutex);
                }
                else{
                    pthread_mutex_lock(&obj->mutex);
                    obj->thread_exit = 1;
                    pthread_mutex_unlock(&obj->mutex);
                    printf("api_sdl_show_run: SDL_QUIT: break \n");
                    break;
                }

            }else if(event.type == BREAK_EVENT){
                pthread_mutex_lock(&obj->mutex);
                obj->thread_exit = 1;
                pthread_mutex_unlock(&obj->mutex);
		        printf("api_sdl_show_run: BREAK_EVENT \n");
                break;
            }
            else{
                if(event.type == SDL_MOUSEMOTION){

                }
                else{
                    printf("api_sdl_show_run: other event: event.type=%d \n", event.type);
                }
            }
        }
        if(obj->win_id)
        {
            printf("api_sdl_show_run: sdlRenderer \n");
            if(obj->sdlRenderer)
                SDL_DestroyRenderer(obj->sdlRenderer);
            printf("api_sdl_show_run: SDL_DestroyWindow \n");
            if(obj->screen)
                SDL_DestroyWindow(obj->screen);
        }
    }
    printf("api_sdl_show_run: SDL_Quit \n");
    SDL_Quit();
    glob_sld_status = 2;
    printf("api_sdl_show_run: quit ok \n");
}
HCSVC_API
void api_sdl_close(char *handle)
{
    if(handle)
    {
        long long *testp = (long long *)handle;
        SDLObj *obj = (SDLObj *)testp[0];
        printf("api_sdl_close: obj->Obj_id= %x \n", obj->Obj_id);

        //SDL_Quit();
        if(obj->sdlTexture)
        {
            SDL_DestroyTexture(obj->sdlTexture);
        }
        if(obj->show_buffer)
        {
            free(obj->show_buffer);
        }
        printf("api_sdl_close: free(obj->show_buffer) ok \n");
        if(obj->json)
        {
            api_json_free(obj->json);
            obj->json = NULL;
        }
        printf("api_sdl_close: api_json_free(obj->json) ok \n");
        if(obj->osd_handle)
        {
            api_simple_osd_close(obj->osd_handle);
            free(obj->osd_handle);
            obj->osd_handle = NULL;
        }
        pthread_mutex_destroy(&obj->mutex);
        printf("api_sdl_close: free(obj->osd_handle) ok \n");
        free(obj);
        testp[0] = 0;
        printf("api_sdl_close: ok \n");
    }
}
HCSVC_API
int api_av_sdl_init()
{
#ifdef _WIN32
    //char *cmd = "set SDL_AUDIODRIVER=directsound";
    //system(cmd);
    SetEnvironmentVariableA("SDL_AUDIODRIVER", "directsound");
#endif
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        printf( "api_sdl_init: Could not initialize SDL - %s\n", SDL_GetError());
        return -1;
    }
    glob_sld_status = 1;
    return 0;
}
HCSVC_API
int api_sdl_init(char *handle, char *param)
{
    int ret = 0;

    uint32_t pixformat = 0;
    //IYUV: Y + U + V  (3 planes)
    //YV12: Y + V + U  (3 planes)
    pixformat = SDL_PIXELFORMAT_IYUV; //SDL_PIXELFORMAT_YV12

    //char handle[8] = "";
    ret = api_create_sdl_handle(handle);
    long long *testp = (long long *)handle;
    SDLObj *obj = (SDLObj *)testp[0];
    printf("api_sdl_init: param= %s \n", param);
    obj->json = (cJSON *)api_str2json(param);
    printf("api_sdl_init: obj->json= %x \n", obj->json);
    obj->param = param;

    obj->osd_enable = GetvalueInt(obj->json, "osd");
    printf("api_sdl_init: obj->osd_enable=%d \n", obj->osd_enable);
    if(obj->osd_enable)
    {
        obj->orgX = GetvalueInt(obj->json, "orgX");
        obj->orgY = GetvalueInt(obj->json, "orgY");
        obj->scale = GetvalueInt(obj->json, "scale");
        obj->color = GetvalueInt(obj->json, "color");
        char src_pix_fmt[64] = "";
        GetvalueStr(obj->json, "src_pix_fmt", src_pix_fmt);
	    if (strcmp(src_pix_fmt, ""))
	    {
            if (!strcmp(src_pix_fmt, "AV_PIX_FMT_YUV420P"))
            {
                //obj->src_pix_fmt = AV_PIX_FMT_YUV420P;
            }
        }
    }

    obj->print = GetvalueInt(obj->json, "print");
    obj->win_id = GetvalueInt(obj->json, "win_id");
    obj->scalex = 1.0;
    obj->scaley = 1.0;
    obj->screen_w = GetvalueInt(obj->json, "screen_w");
    printf("api_sdl_init: obj->screen_w= %d \n", obj->screen_w);
    obj->screen_h = GetvalueInt(obj->json, "screen_h");
    printf("api_sdl_init: obj->screen_h= %d \n", obj->screen_h);
    obj->pixel_w = GetvalueInt(obj->json, "pixel_w");
    printf("api_sdl_init: obj->pixel_w= %d \n", obj->pixel_w);
    obj->pixel_h = GetvalueInt(obj->json, "pixel_h");
    printf("api_sdl_init: obj->pixel_h= %d \n", obj->pixel_h);
    obj->bpp = GetvalueInt(obj->json, "bpp");
    printf("api_sdl_init: obj->bpp= %d \n", obj->bpp);
    obj->wait_time = GetvalueInt(obj->json, "wait_time");
    printf("api_sdl_init: obj->wait_time= %d \n", obj->wait_time);
    char cpixformat[64] = "";
    GetvalueStr(obj->json, "pixformat", cpixformat);
	if (!strcmp(cpixformat, "SDL_PIXELFORMAT_IYUV"))
	{
       obj->pixformat =
       pixformat = SDL_PIXELFORMAT_IYUV;
	}
	else if (!strcmp(cpixformat, "SDL_PIXELFORMAT_YV12"))
	{
       obj->pixformat =
       pixformat = SDL_PIXELFORMAT_YV12;
	}
	printf("api_sdl_init: pixformat= %d \n", pixformat);
	char name[64] = "";
	GetvalueStr(obj->json, "name", name);
    printf("api_sdl_init: name= %s \n", name);

    int frame_size = obj->pixel_w * obj->pixel_h * obj->bpp / 8;
    obj->frame_size = frame_size;
    //unsigned char show_buffer[frame_size];
    obj->show_buffer = calloc(1, frame_size * sizeof(char));
    pthread_mutex_init(&obj->mutex,NULL);
    printf("api_sdl_init: obj->show_buffer= %x \n", obj->show_buffer);
     //------------SDL初始化--------
#if 0
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        printf( "api_sdl_init: Could not initialize SDL - %s\n", SDL_GetError());
        return -1;
    }
#endif
    printf("api_sdl_init: SDL_Init ok \n");
    SDL_Window *screen = NULL;
    if(obj->win_id)
    {
        printf("api_sdl_init: obj->win_id= %d \n", obj->win_id);
        screen = SDL_CreateWindowFrom(obj->win_id);
        printf("api_sdl_init: screen= %x \n", screen);
    }
    else{
        //SDL 2.0 Support for multiple windows
        screen = SDL_CreateWindow(name,
                          SDL_WINDOWPOS_UNDEFINED,
                          SDL_WINDOWPOS_UNDEFINED,
                          obj->screen_w, obj->screen_h,
                          SDL_WINDOW_RESIZABLE/* SDL_WINDOW_HIDDEN*/| SDL_WINDOW_OPENGL);
    }


    if(!screen) {
        printf("SDL: could not set video mode - exiting\n");
        return -1;
    }
    SDL_Renderer* sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
    SDL_Texture* sdlTexture = SDL_CreateTexture(
        sdlRenderer,
        obj->pixformat,//SDL_PIXELFORMAT_ARGB8888//SDL_PIXELFORMAT_RGBA8888
        //SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        obj->pixel_w,
        obj->pixel_h);

    SDL_Rect rect;
    SDL_Thread *refresh_thread = NULL;
    if(obj->wait_time > 0)//定时刷新模式
    {
        refresh_thread = SDL_CreateThread(refresh_video,"TestThread",(void *)handle);
    }
    SDL_Event event;

    obj->screen = screen;
    obj->sdlRenderer = sdlRenderer;
    obj->sdlTexture = sdlTexture;
    obj->refresh_thread = refresh_thread;
#if 0
    api_sdl_show_run(handle);
    api_sdl_close(handle);

#endif
    return ret;
}


//===============================================

#if 0
int main(int argc, char **argv)
{
    const char *output_type;

    /* register all the codecs */
    avcodec_register_all();

    if (argc < 2) {
        printf("usage: %s output_type\n"
               "API example program to decode/encode a media stream with libavcodec.\n"
               "This program generates a synthetic stream and encodes it to a file\n"
               "named test.h264, test.mp2 or test.mpg depending on output_type.\n"
               "The encoded stream is then decoded and written to a raw data output.\n"
               "output_type must be chosen between 'h264', 'mp2', 'mpg'.\n",
               argv[0]);
        return 1;
    }
    output_type = argv[1];

    if (!strcmp(output_type, "h264")) {
        video_encode_example("test.h264", AV_CODEC_ID_H264);
    } else if (!strcmp(output_type, "mp2")) {
        audio_encode_example("test.mp2");
        audio_decode_example("test.pcm", "test.mp2");
    } else if (!strcmp(output_type, "mpg")) {
        video_encode_example("test.mpg", AV_CODEC_ID_MPEG1VIDEO);
        video_decode_example("test%02d.pgm", "test.mpg");
    } else {
        fprintf(stderr, "Invalid output type '%s', choose between 'h264', 'mp2', or 'mpg'\n",
                output_type);
        return 1;
    }

    return 0;
}
#endif

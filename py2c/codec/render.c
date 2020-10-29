
#include "inc.h"

extern cJSON* mystr2json(char *text);
extern int GetvalueInt(cJSON *json, char *key);
extern char* GetvalueStr(cJSON *json, char *key);
extern cJSON* deleteJson(cJSON *json);
extern int64_t get_sys_time();

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
    cJSON *json;
    char *param;
    char *name;
    int pixformat;
    int screen_w;
    int screen_h;
    int pixel_w;
    int pixel_h;
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
    int frame_rate;
    int print;
    unsigned char *show_buffer;
}SDLObj;

int refresh_video(void *handle){
    if(!handle)
    {
        return -1;
    }
    long long *testp = (long long *)handle;
    SDLObj *obj = (SDLObj *)testp[0];
    printf("refresh_video: obj->Obj_id= %x \n", obj->Obj_id);
    int wait_time = obj->wait_time;
    obj->thread_exit=0;
    while (obj->thread_exit == 0)
    {
        if(obj->thread_pause==0)
        {
            SDL_Event event;
            event.type = REFRESH_EVENT;
            SDL_PushEvent(&event);
        }
        SDL_Delay(wait_time);
    }
    obj->thread_exit = 0;
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

#if 1

int split_screen(char *handle, char *show_buffer, SDL_Rect rect, int width, int show_flag)
{
    if(handle)
    {
        int64_t time0 = get_sys_time();
        long long *testp = (long long *)handle;
        SDLObj *obj = (SDLObj *)testp[0];
        //printf("split_screen \n");
        //memcpy(obj->show_buffer, show_buffer, obj->frame_size);
        //SDL_RenderClear( obj->sdlRenderer );
        //if(show_flag)
        {
            int step = 25;//50;
    	    if(!obj->start_time)
            {
                obj->start_time = api_get_time_stamp_ll();
            }
            int64_t now = api_get_time_stamp_ll();
            int difftime = (int)(now - obj->start_time);

            if(difftime > 1000)
            {
                int sum_time = (difftime / 1000);//s
                if((obj->frame_num % step) == (step - 1))
                {
                    obj->frame_rate = (int)((float)step / (float)sum_time);
                    //int bits_rate = bits_rate_ >> 10;
                    //
                    obj->start_time = now;
                    //sum_bits_ = 0;
                }
            }
	        if(obj->osd_enable)
	        {
	            if(!obj->osd_handle)
	            {
	                obj->handle_size = 8;
	                obj->osd_handle = (char *)calloc(1, obj->handle_size * sizeof(char));
	                //obj->pCodecCtx->width//obj->pCodecCtx->height
                    obj->json = api_renew_json_int(obj->json, "width", obj->pixel_w);
                    obj->json = api_renew_json_int(obj->json, "height", obj->pixel_h);
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
                if(framerate > 0)
                {
                    int orgX = 0;
                    int orgY = obj->pixel_h - (obj->pixel_h / 16);
                    orgY = (obj->pixel_h / 8);
                    char context[255] = "";
                    obj->json = api_renew_json_int(obj->json, "orgX", orgX);
                    obj->json = api_renew_json_int(obj->json, "orgY", orgY);
                    sprintf(&context[strlen(context)], "  %d", obj->pixel_w);
                    sprintf(&context[strlen(context)], "   %d", obj->pixel_h);
                    sprintf(&context[strlen(context)], "   %2d", framerate);
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

	        obj->frame_num++;
        }
#if 1
        SDL_UpdateTexture( obj->sdlTexture, NULL, show_buffer, width);
#else
        int w = obj->pixel_w;
        int h = obj->pixel_h;
        int frame_size = w * h * obj->bpp / 8;
        SDL_Rect *rect2 = NULL;
        char *src_y = show_buffer;
        char *src_u = &show_buffer[w * h];
        char *src_v = &show_buffer[w * h + (w >> 1) * (h >> 1)];
        int linesize[3] = { width,
                            width >> 1,
                            width >> 1};
        SDL_UpdateYUVTexture(   obj->sdlTexture, rect2,
                                src_y,
                                linesize[0],
                                src_u,
                                linesize[1],
                                src_v,
                                linesize[2]);
#endif
        //SDL_RenderCopy( sdlRenderer, sdlTexture, &rect, &rect );
        int64_t time1 = get_sys_time();
	    int difftime = (int)(time1 - time0);
	    if(obj->print)//(difftime > 20)
	    {
	        printf("split_screen: SDL_UpdateTexture: show_flag=%d, difftime= %d \n", show_flag, difftime);
	    }
        SDL_RenderCopy( obj->sdlRenderer, obj->sdlTexture, NULL, &rect);

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
        if(show_flag)
        {
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
        }

#endif

    }
    return 0;
}

HCSVC_API
int api_split_screen(char *handle, char * show_buffer, char *param, int width)
{
    SDL_Rect rect;
    cJSON *json = mystr2json(param);
    rect.x = GetvalueInt(json, "rect_x");
    rect.y = GetvalueInt(json, "rect_y");
    rect.w = GetvalueInt(json, "rect_w");
    rect.h = GetvalueInt(json, "rect_h");
    int show_flag = GetvalueInt(json, "show_flag");
    int ret = split_screen(handle, show_buffer, rect, width, show_flag);
    deleteJson(json);
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
        ret = obj->thread_exit;
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
#endif

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
		        //printf("api_sdl_show_run: SDL_WINDOWEVENT: obj->screen_w=%d \n", obj->screen_w);
		        //printf("api_sdl_show_run: SDL_WINDOWEVENT: obj->screen_h=%d \n", obj->screen_h);
                SDL_GetWindowSize(obj->screen,&obj->screen_w,&obj->screen_h);
		        //SDL_Delay(1000);
           }else if(event.type == SDL_KEYDOWN){
                //Pause
		        printf("api_sdl_show_run: SDL_KEYDOWN \n");
                if(event.key.keysym.sym == SDLK_SPACE)
                {
                    if(obj->refresh_thread)
                    {
                        obj->thread_pause = !obj->thread_pause;
                    }
                }

           }else if(event.type == SDL_QUIT){
		        printf("api_sdl_show_run: SDL_QUIT \n");
                if(obj->refresh_thread)
                {
                    obj->thread_exit = 1;
                }
                else{
                    obj->thread_exit = 1;
                    printf("api_sdl_show_run: SDL_QUIT: break \n");
                    break;
                }

            }else if(event.type == BREAK_EVENT){
                obj->thread_exit = 1;
		        printf("api_sdl_show_run: BREAK_EVENT \n");
                break;
            }
        }
        if(obj->win_id)
        {
            SDL_DestroyRenderer(obj->sdlRenderer);
            SDL_DestroyWindow(obj->screen);
        }
    }

    SDL_Quit();
    printf("api_sdl_show_run: quit \n");
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
        SDL_DestroyTexture(obj->sdlTexture);
        if(obj->show_buffer)
        {
            free(obj->show_buffer);
        }
        if(obj->json)
        {
            api_json_free(obj->json);
            obj->json = NULL;
        }
        if(obj->osd_handle)
        {
            api_simple_osd_close(obj->osd_handle);
            free(obj->osd_handle);
            obj->osd_handle = NULL;
        }
        free(obj);
        testp[0] = 0;
        printf("api_sdl_close: ok \n");
    }

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
    obj->json = mystr2json(param);
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
        char *src_pix_fmt = GetvalueStr(obj->json, "src_pix_fmt");
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
    char *cpixformat = GetvalueStr(obj->json, "pixformat");
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
	char *name = GetvalueStr(obj->json, "name");
    printf("api_sdl_init: name= %s \n", name);

    int frame_size = obj->pixel_w * obj->pixel_h * obj->bpp / 8;
    obj->frame_size = frame_size;
    //unsigned char show_buffer[frame_size];
    obj->show_buffer = calloc(1, frame_size * sizeof(char));
    printf("api_sdl_init: obj->show_buffer= %x \n", obj->show_buffer);
     //------------SDL初始化--------
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        printf( "Could not initialize SDL - %s\n", SDL_GetError());
        return -1;
    }
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
    if(obj->wait_time > 0)
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

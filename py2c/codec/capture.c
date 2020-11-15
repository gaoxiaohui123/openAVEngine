

#include "inc.h"
//#include "libavutil/log2_tab.c" //ff_log2_tab变量定义在log2_tab.c文件中，libavformat模块有引用到

//#ifdef linux
//#include <pthread.h>
//#endif

#if 1
#ifdef linux

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/fb.h>
#include <linux/videodev2.h>

#endif
#endif

extern cJSON* mystr2json(char *text);
extern int GetvalueInt(cJSON *json, char *key);
extern char* GetvalueStr(cJSON *json, char *key);
extern cJSON* deleteJson(cJSON *json);
extern int64_t get_sys_time();

#if 1

#if 1
//#define linux

//Show Device
void show_dshow_device(){
	AVFormatContext *pFormatCtx = avformat_alloc_context();
	AVDictionary* options = NULL;
	av_dict_set(&options,"list_devices","true",0);
	AVInputFormat *iformat = av_find_input_format("dshow");
	printf("Device Info=============\n");
	avformat_open_input(&pFormatCtx,"video=dummy",iformat,&options);
	printf("========================\n");
}

//Show Device Option
void show_dshow_device_option(){
	AVFormatContext *pFormatCtx = avformat_alloc_context();
	AVDictionary* options = NULL;
	av_dict_set(&options,"list_options","true",0);
	AVInputFormat *iformat = av_find_input_format("dshow");
	printf("Device Option Info======\n");
	avformat_open_input(&pFormatCtx,"video=Integrated Camera",iformat,&options);
	printf("========================\n");
}

//Show VFW Device
void show_vfw_device(){
	AVFormatContext *pFormatCtx = avformat_alloc_context();
	AVInputFormat *iformat = av_find_input_format("vfwcap");
	printf("VFW Device Info======\n");
	avformat_open_input(&pFormatCtx,"list",iformat,NULL);
	printf("=====================\n");
}

//Show Device
void show_avfoundation_device(){
	AVFormatContext *pFormatCtx = avformat_alloc_context();
	AVDictionary* options = NULL;
	av_dict_set(&options,"list_devices","true",0);
	AVInputFormat *iformat = av_find_input_format("avfoundation");
	printf("AVFoundation Device Info=============\n");
	avformat_open_input(&pFormatCtx,"",iformat,&options);
	printf("========================\n");
}
//linux
void show_linux_device(){
	AVFormatContext *pFormatCtx = avformat_alloc_context();
	AVDictionary* options = NULL;
	//av_dict_set(&options,"list_devices","true",0);
	AVInputFormat *iformat = av_find_input_format("video4linux2");
	printf("AVFoundation Device Info=============\n");
	avformat_open_input(&pFormatCtx,"/dev/video0",iformat,&options);
	//avformat_open_input(&pFormatCtx,"",iformat,&options);
	printf("========================\n");
}
#endif

#if 1
char* input_name= "video4linux2";
char* file_name = "/dev/video0";
//char* out_file  = "test.jpeg";
char* out_file  = "yuv420.yuv";

void captureOneFrame(void){
    AVFormatContext *fmtCtx = NULL;
    AVPacket *packet;
    AVInputFormat *inputFmt;
    FILE *fp;
	int ret;


    inputFmt = av_find_input_format (input_name);

    if (inputFmt == NULL)    {
        printf("can not find_input_format\n");
        return;
    }

    if (avformat_open_input ( &fmtCtx, file_name, inputFmt, NULL) < 0){
        printf("can not open_input_file\n");         return;
    }
	/* print device information*/
	av_dump_format(fmtCtx, 0, file_name, 0);

    packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    av_read_frame(fmtCtx, packet);
    printf("data length = %d\n",packet->size);

    fp = fopen(out_file, "wb");
    if (fp < 0)    {
        printf("open frame data file failed\n");
        return ;
    }

    fwrite(packet->data, 1, packet->size, fp);

    fclose(fp);
    av_free_packet(packet);
    avformat_close_input(&fmtCtx);
 }

/*=============================================================================
 * #     FileName: read_device.c
 * #         Desc: use ffmpeg read a frame data from v4l2, and convert
 * #			   the output data format
 * #       Author: licaibiao
 * #   LastChange: 2017-03-28
 * =============================================================================*/

void captureOneFrame2(void){
    AVFormatContext *fmtCtx = NULL;
    AVInputFormat 	*inputFmt;
    AVPacket 		*packet;
	AVCodecContext	*pCodecCtx;
	AVCodec 		*pCodec;
	struct SwsContext *sws_ctx;
    FILE *fp;
	int i;
	int ret;
	int videoindex;

	enum AVPixelFormat dst_pix_fmt = AV_PIX_FMT_YUV420P;
    const char *dst_size = NULL;
    const char *src_size = NULL;
    uint8_t *src_data[4];
    uint8_t *dst_data[4];
    int src_linesize[4];
    int dst_linesize[4];
    int src_bufsize;
    int dst_bufsize;
    int src_w ;
    int src_h ;
    int dst_w = 1280;
    int dst_h = 720;

    fp = fopen(out_file, "wb");
    if (fp < 0)    {
        printf("open frame data file failed\n");
        return ;
    }

    inputFmt = av_find_input_format (input_name);

    if (inputFmt == NULL)    {
        printf("can not find_input_format\n");
        return;
    }

    if (avformat_open_input ( &fmtCtx, file_name, inputFmt, NULL) < 0){
        printf("can not open_input_file\n");         return;
    }

	av_dump_format(fmtCtx, 0, file_name, 0);

	videoindex= -1;
	for(i=0; i<fmtCtx->nb_streams; i++)
		if(fmtCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
			videoindex=i;
			break;
		}
	if(videoindex==-1){
		printf("Didn't find a video stream.\n");
		return -1;
	}

	pCodecCtx = fmtCtx->streams[videoindex]->codec;
	pCodec    = avcodec_find_decoder(pCodecCtx->codec_id);

    printf("picture width   =  %d \n", pCodecCtx->width);
    printf("picture height  =  %d \n", pCodecCtx->height);
    printf("Pixel   Format  =  %d \n", pCodecCtx->pix_fmt);

	sws_ctx = sws_getContext( pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, dst_w, dst_h, dst_pix_fmt,
                             SWS_BILINEAR, NULL, NULL, NULL);

	src_bufsize = av_image_alloc(src_data, src_linesize, pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 16);
	dst_bufsize = av_image_alloc(dst_data, dst_linesize, dst_w, dst_h, dst_pix_fmt, 1);

	packet = (AVPacket *)av_malloc(sizeof(AVPacket));

	int loop = 100;
	while(loop--){
    	av_read_frame(fmtCtx, packet);
		memcpy(src_data[0], packet->data, packet->size);
		sws_scale(sws_ctx, src_data,  src_linesize, 0, pCodecCtx->height, dst_data, dst_linesize);
    	fwrite(dst_data[0], 1, dst_bufsize, fp);
	}

    fclose(fp);
    av_free_packet(packet);
	av_freep(&dst_data[0]);
	sws_freeContext(sws_ctx);
    avformat_close_input(&fmtCtx);
 }

int video_capture();
#endif

HCSVC_API
void api_show_device()
{
    av_log_set_level(AV_LOG_DEBUG);//( AV_LOG_DEBUG );
    av_register_all();
    avcodec_register_all();
    avformat_network_init();

    avdevice_register_all();

#ifdef _WIN32
    //Show Dshow Device
	show_dshow_device();
	//Show Device Options
	show_dshow_device_option();
	//Show VFW Options
	show_vfw_device();
#elif defined linux
    ///show_linux_device();
#elif defined macOS
    show_avfoundation_device();
#endif
    //captureOneFrame();
    //captureOneFrame2();
    video_capture();
    //av_log_set_level(AV_LOG_QUIET);
}


#if 0
//#include "SDL/SDL.h"
/**
 * 最简单的基于FFmpeg的AVDevice例子（读取摄像头）
 * Simplest FFmpeg Device (Read Camera)
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本程序实现了本地摄像头数据的获取解码和显示。是基于FFmpeg
 * 的libavdevice类库最简单的例子。通过该例子，可以学习FFmpeg中
 * libavdevice类库的使用方法。
 * 本程序在Windows下可以使用2种方式读取摄像头数据：
 *  1.VFW: Video for Windows 屏幕捕捉设备。注意输入URL是设备的序号，
 *          从0至9。
 *  2.dshow: 使用Directshow。注意作者机器上的摄像头设备名称是
 *         “Integrated Camera”，使用的时候需要改成自己电脑上摄像头设
 *          备的名称。
 * 在Linux下可以使用video4linux2读取摄像头设备。
 * 在MacOS下可以使用avfoundation读取摄像头设备。
 *
 * This software read data from Computer's Camera and play it.
 * It's the simplest example about usage of FFmpeg's libavdevice Library.
 * It's suiltable for the beginner of FFmpeg.
 * This software support 2 methods to read camera in Microsoft Windows:
 *  1.gdigrab: VfW (Video for Windows) capture input device.
 *             The filename passed as input is the capture driver number,
 *             ranging from 0 to 9.
 *  2.dshow: Use Directshow. Camera's name in author's computer is
 *             "Integrated Camera".
 * It use video4linux2 to read Camera in Linux.
 * It use avfoundation to read Camera in MacOS.
 *
 */


#include <stdio.h>

#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
//Windows
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "SDL/SDL.h"
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
//#include <SDL/SDL.h>
#ifdef __cplusplus
};
#endif
#endif

//Output YUV420P
#define OUTPUT_YUV420P 0
//'1' Use Dshow
//'0' Use VFW
#define USE_DSHOW 0


//Refresh Event
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)

#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)

int thread_exit=0;

int sfp_refresh_thread(void *opaque)
{
	thread_exit=0;
	while (!thread_exit) {
		SDL_Event event;
		event.type = SFM_REFRESH_EVENT;
		SDL_PushEvent(&event);
		SDL_Delay(40);
	}
	thread_exit=0;
	//Break
	SDL_Event event;
	event.type = SFM_BREAK_EVENT;
	SDL_PushEvent(&event);
    printf("sfp_refresh_thread: exit");
	return 0;
}

int video_capture()
{

	AVFormatContext	*pFormatCtx;
	int				i, videoindex;
	AVCodecContext	*pCodecCtx;
	AVCodec			*pCodec;

	av_register_all();
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();

	//Open File
	//char filepath[]="src01_480x272_22.h265";
	//avformat_open_input(&pFormatCtx,filepath,NULL,NULL)

	//Register Device
	avdevice_register_all();

//Windows
#ifdef _WIN32

	//Show Dshow Device
	show_dshow_device();
	//Show Device Options
	show_dshow_device_option();
    //Show VFW Options
    show_vfw_device();

#if USE_DSHOW
	AVInputFormat *ifmt=av_find_input_format("dshow");
	//Set own video device's name
	if(avformat_open_input(&pFormatCtx,"video=Integrated Camera",ifmt,NULL)!=0){
		printf("Couldn't open input stream.\n");
		return -1;
	}
#else
	AVInputFormat *ifmt=av_find_input_format("vfwcap");
	if(avformat_open_input(&pFormatCtx,"0",ifmt,NULL)!=0){
		printf("Couldn't open input stream.\n");
		return -1;
	}
#endif
#elif defined linux
    //Linux
	AVInputFormat *ifmt=av_find_input_format("video4linux2");
	if(avformat_open_input(&pFormatCtx,"/dev/video0",ifmt,NULL)!=0){
		printf("Couldn't open input stream.\n");
		return -1;
	}
#else
    show_avfoundation_device();
    //Mac
    AVInputFormat *ifmt=av_find_input_format("avfoundation");
    //Avfoundation
    //[video]:[audio]
    if(avformat_open_input(&pFormatCtx,"0",ifmt,NULL)!=0){
        printf("Couldn't open input stream.\n");
        return -1;
    }
#endif


	if(avformat_find_stream_info(pFormatCtx,NULL)<0)
	{
		printf("Couldn't find stream information.\n");
		return -1;
	}
	videoindex=-1;
	for(i=0; i<pFormatCtx->nb_streams; i++)
		if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
		{
			videoindex=i;
			break;
		}
	if(videoindex==-1)
	{
		printf("Couldn't find a video stream.\n");
		return -1;
	}
	pCodecCtx=pFormatCtx->streams[videoindex]->codec;
	pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
	if(pCodec==NULL)
	{
		printf("Codec not found.\n");
		return -1;
	}
	if(avcodec_open2(pCodecCtx, pCodec,NULL)<0)
	{
		printf("Could not open codec.\n");
		return -1;
	}
	AVFrame	*pFrame,*pFrameYUV;
	pFrame=av_frame_alloc();
	pFrameYUV=av_frame_alloc();
	//unsigned char *out_buffer=(unsigned char *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
	//avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
	//SDL----------------------------
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		printf( "Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}
	int screen_w=0,screen_h=0;
	SDL_Surface *screen;
	screen_w = pCodecCtx->width;
	screen_h = pCodecCtx->height;
	screen = SDL_SetVideoMode(screen_w, screen_h, 0,0);

	if(!screen) {
		printf("SDL: could not set video mode - exiting:%s\n",SDL_GetError());
		return -1;
	}
	SDL_Overlay *bmp;
	bmp = SDL_CreateYUVOverlay(pCodecCtx->width, pCodecCtx->height,SDL_YV12_OVERLAY, screen);
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = screen_w;
	rect.h = screen_h;
	//SDL End------------------------
	int ret, got_picture;

	AVPacket *packet=(AVPacket *)av_malloc(sizeof(AVPacket));

#if OUTPUT_YUV420P
    FILE *fp_yuv=fopen("output.yuv","wb+");
#endif

	struct SwsContext *img_convert_ctx;
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

	printf("video_capture: pCodecCtx->pix_fmt= %d \n", pCodecCtx->pix_fmt);
	//------------------------------
	SDL_Thread *video_tid = SDL_CreateThread(sfp_refresh_thread,NULL);
	//
	SDL_WM_SetCaption("Simplest FFmpeg Read Camera",NULL);
	//Event Loop
	SDL_Event event;

	for (;;) {
		//Wait
		SDL_WaitEvent(&event);
		if(event.type==SFM_REFRESH_EVENT){
			//------------------------------
			int64_t time0 = get_sys_time();
			ret = av_read_frame(pFormatCtx, packet);
			int64_t time1 = get_sys_time();
	        int difftime = (int)(time1 - time0);
	        printf("video_capture: difftime= %d \n", difftime);
			if(ret>=0){
				if(packet->stream_index==videoindex){
					ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
					if(ret < 0){
						printf("Decode Error.\n");
						return -1;
					}
					if(got_picture){
						SDL_LockYUVOverlay(bmp);
						pFrameYUV->data[0]=bmp->pixels[0];
						pFrameYUV->data[1]=bmp->pixels[2];
						pFrameYUV->data[2]=bmp->pixels[1];
						pFrameYUV->linesize[0]=bmp->pitches[0];
						pFrameYUV->linesize[1]=bmp->pitches[2];
						pFrameYUV->linesize[2]=bmp->pitches[1];
						sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);

#if OUTPUT_YUV420P
						int y_size=pCodecCtx->width*pCodecCtx->height;
						fwrite(pFrameYUV->data[0],1,y_size,fp_yuv);    //Y
						fwrite(pFrameYUV->data[1],1,y_size/4,fp_yuv);  //U
						fwrite(pFrameYUV->data[2],1,y_size/4,fp_yuv);  //V
#endif

						SDL_UnlockYUVOverlay(bmp);

						SDL_DisplayYUVOverlay(bmp, &rect);

					}
				}
				av_free_packet(packet);
			}else{
				//Exit Thread
				thread_exit=1;
			}
		}else if(event.type==SDL_QUIT){
			thread_exit=1;
		}else if(event.type==SFM_BREAK_EVENT){
			break;
		}

	}


	sws_freeContext(img_convert_ctx);

#if OUTPUT_YUV420P
    fclose(fp_yuv);
#endif

	SDL_Quit();

	//av_free(out_buffer);
	av_free(pFrameYUV);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);

	return 0;
}
#else
/**
 * 最简单的基于FFmpeg的视频播放器2(SDL升级版)
 * Simplest FFmpeg Player 2(SDL Update)
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 第2版使用SDL2.0取代了第一版中的SDL1.2
 * Version 2 use SDL 2.0 instead of SDL 1.2 in version 1.
 *
 * 本程序实现了视频文件的解码和显示(支持HEVC，H.264，MPEG2等)。
 * 是最简单的FFmpeg视频解码方面的教程。
 * 通过学习本例子可以了解FFmpeg的解码流程。
 * 本版本中使用SDL消息机制刷新视频画面。
 * This software is a simplest video player based on FFmpeg.
 * Suitable for beginner of FFmpeg.
 *
 * 备注:
 * 标准版在播放视频的时候，画面显示使用延时40ms的方式。这么做有两个后果：
 * （1）SDL弹出的窗口无法移动，一直显示是忙碌状态
 * （2）画面显示并不是严格的40ms一帧，因为还没有考虑解码的时间。
 * SU（SDL Update）版在视频解码的过程中，不再使用延时40ms的方式，而是创建了
 * 一个线程，每隔40ms发送一个自定义的消息，告知主函数进行解码显示。这样做之后：
 * （1）SDL弹出的窗口可以移动了
 * （2）画面显示是严格的40ms一帧
 * Remark:
 * Standard Version use's SDL_Delay() to control video's frame rate, it has 2
 * disadvantages:
 * (1)SDL's Screen can't be moved and always "Busy".
 * (2)Frame rate can't be accurate because it doesn't consider the time consumed
 * by avcodec_decode_video2()
 * SU（SDL Update）Version solved 2 problems above. It create a thread to send SDL
 * Event every 40ms to tell the main loop to decode and show video frames.
 */

#include <stdio.h>

#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
//Windows
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "SDL2/SDL.h"
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <SDL2/SDL.h>
#ifdef __cplusplus
};
#endif
#endif

//Refresh Event
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)

#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)

int thread_exit=0;
int thread_pause=0;

int sfp_refresh_thread(void *opaque){
	thread_exit=0;
	thread_pause=0;

	while (!thread_exit) {
		if(!thread_pause){
			SDL_Event event;
			event.type = SFM_REFRESH_EVENT;
			SDL_PushEvent(&event);
		}
		SDL_Delay(40);
	}
	printf("sfp_refresh_thread: thread_exit=%d \n", thread_exit);
	thread_exit=0;
	thread_pause=0;
	//Break
	SDL_Event event;
	event.type = SFM_BREAK_EVENT;
	SDL_PushEvent(&event);

	return 0;
}

int video_capture()
{

	AVFormatContext	*pFormatCtx = NULL;
	int				i, videoindex;
	AVCodecContext	*pCodecCtx;
	AVCodec			*pCodec;
	AVFrame	*pFrame,*pFrameYUV;
	unsigned char *out_buffer;
	AVPacket *packet;
	int ret, got_picture;

	//------------SDL----------------
	int screen_w,screen_h;
	SDL_Window *screen;
	SDL_Renderer* sdlRenderer;
	SDL_Texture* sdlTexture;
	SDL_Rect sdlRect;
	SDL_Thread *video_tid;
	SDL_Event event;

	struct SwsContext *img_convert_ctx;

	//char filepath[]="bigbuckbunny_480x272.h265";
	//char filepath[]="Titanic.ts";

	//av_register_all();
	//avformat_network_init();
    printf("video_capture 0 \n");


    AVInputFormat *inputFmt;
    inputFmt = av_find_input_format (input_name);

    if (inputFmt == NULL)    {
        printf("can not find_input_format\n");
        return;
    }
    printf("video_capture 1 \n");
    AVDictionary* options = NULL;
    //av_dict_set(&options,"list_formats","all",0);
    //av_dict_set(&options,"list_formats","raw",0);
	//av_dict_set(&options,"video_size","1280x720",0);
	av_dict_set(&options,"video_size","320x180",0);
	av_dict_set(&options,"framerate","25",0);
    if (avformat_open_input ( &pFormatCtx, file_name, inputFmt, &options) < 0){
        printf("can not open_input_file\n");         return -1;
    }
    printf("video_capture 2 \n");
	//pFormatCtx = avformat_alloc_context();

	//if(avformat_open_input(&pFormatCtx,filepath,NULL,NULL)!=0){
	//	printf("Couldn't open input stream.\n");
	//	return -1;
	//}
	if(avformat_find_stream_info(pFormatCtx,NULL)<0){
		printf("Couldn't find stream information.\n");
		return -1;
	}
	videoindex=-1;
	for(i=0; i<pFormatCtx->nb_streams; i++)
		if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
			videoindex=i;
			break;
		}
	if(videoindex==-1){
		printf("Didn't find a video stream.\n");
		return -1;
	}
	printf("video_capture 3 \n");
	pCodecCtx=pFormatCtx->streams[videoindex]->codec;
	pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
	if(pCodec==NULL){
		printf("Codec not found.\n");
		return -1;
	}
	if(avcodec_open2(pCodecCtx, pCodec,NULL)<0){
		printf("Could not open codec.\n");
		return -1;
	}
	pFrame=av_frame_alloc();
	pFrameYUV=av_frame_alloc();

	out_buffer=(unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P,  pCodecCtx->width, pCodecCtx->height,1));
	av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize,out_buffer,
		AV_PIX_FMT_YUV420P,pCodecCtx->width, pCodecCtx->height,1);
    printf("video_capture 4 \n");
	//Output Info-----------------------------
	//printf("---------------- File Information ---------------\n");
	//av_dump_format(pFormatCtx,0,filepath,0);
	//printf("-------------------------------------------------\n");
    printf("video_capture: pCodecCtx->pix_fmt= %d \n", pCodecCtx->pix_fmt);
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		printf( "Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}
    printf("create screen \n");
	//SDL 2.0 Support for multiple windows
	screen_w = pCodecCtx->width;
	screen_h = pCodecCtx->height;
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
	sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,pCodecCtx->width,pCodecCtx->height);

	sdlRect.x=0;
	sdlRect.y=0;
	sdlRect.w=screen_w;
	sdlRect.h=screen_h;

	packet=(AVPacket *)av_malloc(sizeof(AVPacket));

	video_tid = SDL_CreateThread(sfp_refresh_thread,NULL,NULL);
	//------------SDL End------------
	//Event Loop

	for (;;) {
		//Wait
		SDL_WaitEvent(&event);
		if(event.type==SFM_REFRESH_EVENT){
		    int64_t time0 = get_sys_time();
			while(1){
				if(av_read_frame(pFormatCtx, packet)<0)
					thread_exit=1;

				if(packet->stream_index==videoindex)
					break;
			}
			int64_t time1 = get_sys_time();
	        int difftime = (int)(time1 - time0);
	        printf("video_capture: difftime= %d \n", difftime);
			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
			if(ret < 0){
				printf("Decode Error.\n");
				return -1;
			}
			if(got_picture){
				sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
				//SDL---------------------------
				SDL_UpdateTexture( sdlTexture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0] );
				SDL_RenderClear( sdlRenderer );
				//SDL_RenderCopy( sdlRenderer, sdlTexture, &sdlRect, &sdlRect );
				SDL_RenderCopy( sdlRenderer, sdlTexture, NULL, NULL);
				SDL_RenderPresent( sdlRenderer );
				//SDL End-----------------------
			}
			av_free_packet(packet);
		}else if(event.type == SDL_WINDOWEVENT){
		    printf("video_capture: SDL_WINDOWEVENT \n");
            //If Resize
            SDL_GetWindowSize(screen,&screen_w,&screen_h);
		}else if(event.type==SDL_KEYDOWN){
			//Pause
			printf("video_capture: SDL_KEYDOWN \n");
			if(event.key.keysym.sym==SDLK_SPACE)
				thread_pause=!thread_pause;
		}else if(event.type==SDL_QUIT){
		    printf("video_capture: SDL_QUIT \n");
			thread_exit = 1;
		}else if(event.type==SFM_BREAK_EVENT){
		    printf("video_capture: SFM_BREAK_EVENT \n");
			break;
		}

	}

	sws_freeContext(img_convert_ctx);

	SDL_Quit();
	//--------------
	av_frame_free(&pFrameYUV);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);

	return 0;
}
#endif

#endif


void get_screen_wxh(int *width, int *height)
{
#ifdef linux
    char cmd= "sudo chmod 0777 /dev/fb0"; //chmod 0777 /dev/fb0
    system(cmd);
    int fd;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    int screensize = 0;
    fd = open("/dev/fb0",O_RDWR);
    /*取得屏幕相关参数*/
    ioctl(fd, FBIOGET_FSCREENINFO, &finfo);
    ioctl(fd, FBIOGET_VSCREENINFO, &vinfo);
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

    printf("get_screen_wxh: %d*%d\n",vinfo.xres,vinfo.yres);
    width[0] = vinfo.xres;
    height[0] = vinfo.yres;
    close(fd);
#endif
}

HCSVC_API
int api_list_devices(char *input_name0, char *device_name0)
{
    int ret = 0;
#if 0

    int fd;
    struct v4l2_capability cap;
    struct v4l2_fmtdesc fmtdesc;
    struct v4l2_format fmt;
    struct v4l2_requestbuffers reqbuf;
    struct v4l2_buffer buf;

    fd = open(device_name0,O_RDWR,0);
    if (fd < 0) {
		printf("Open %s failed \n");
	}
    ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
	if (ret < 0) {
		printf("VIDIOC_QUERYCAP failed (%d)\n", ret);
	}
	printf("------------VIDIOC_QUERYCAP-----------\n");
	printf("Capability Informations:\n");
	printf(" driver: %s\n", cap.driver);
	printf(" card: %s\n", cap.card);
	printf(" bus_info: %s\n", cap.bus_info);
	printf(" version: %08X\n", cap.version);
	printf(" capabilities: %08X\n\n", cap.capabilities);
    memset(&fmtdesc,0,sizeof(fmtdesc));
	fmtdesc.index=0;
	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret=ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc);
	while (ret != 0)
	{
		fmtdesc.index++;
		ret=ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc);
	}
	printf("--------VIDIOC_ENUM_FMT---------\n");
	printf("get the format what the device support\n");
	printf("pixelformat = %c%c%c%c, description = %s \n",fmtdesc.pixelformat & 0xFF, (fmtdesc.pixelformat >> 8) & 0xFF, (fmtdesc.pixelformat >> 16) & 0xFF,(fmtdesc.pixelformat >> 24) & 0xFF, fmtdesc.description);
    return ret;

#endif

#if 0
    char cmd= "sudo chmod 0777 /dev/fb0"; //chmod 0777 /dev/fb0
    system(cmd);
    int fd;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    int screensize = 0;
    fd = open("/dev/fb0",O_RDWR);
    /*取得屏幕相关参数*/
    ioctl(fd, FBIOGET_FSCREENINFO, &finfo);
    ioctl(fd, FBIOGET_VSCREENINFO, &vinfo);
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

    printf("api_list_devices: %d*%d\n",vinfo.xres,vinfo.yres);
    close(fd);
#endif
    av_register_all();
	avformat_network_init();
    avdevice_register_all();

    AVFormatContext	*pFormatCtx = NULL;
    AVInputFormat *inputFmt;
    int64_t time0 = get_sys_time();
    inputFmt = av_find_input_format (input_name0);
    //AVInputFormat *iformat = av_find_input_format("dshow");

    if (inputFmt == NULL)    {
        printf("api_list_devices: can not find_input_format: input_name0=%s \n", input_name0);
        return -1;
    }
    int64_t time1 = get_sys_time();
    int difftime = (int)(time1 - time0);
    printf("api_list_devices: av_find_input_format: difftime=%d \n", difftime);

    printf("api_list_devices 1 \n");
    AVDictionary* options = NULL;
    av_dict_set(&options,"list_formats","all",0);
    //av_opt_set_int(&options,"list_formats",1,0);
    //av_dict_set(&options,"list_formats","raw",0);
	//av_dict_set(&options,"video_size","1280x720",0);
	//av_dict_set(&options,"video_size","320x180",0);
	//av_dict_set(&options,"framerate","25",0);
	av_dict_set(&options, "list_options", "true", 0);


	time0 = get_sys_time();
    if (avformat_open_input ( &pFormatCtx, device_name0, inputFmt, &options) < 0){
        printf("api_list_devices: can not open_input_file: device_name0=%s \n", device_name0);
        ret = -1;
    }
    time1 = get_sys_time();
    difftime = (int)(time1 - time0);
    printf("api_list_devices: avformat_open_input: difftime=%d \n", difftime);
    AVDictionaryEntry *t = NULL;
    while((t = av_dict_get(options, "", t, AV_DICT_IGNORE_SUFFIX))){
        av_log(NULL, AV_LOG_DEBUG, "%s", "%s", t->key, t->value);
    }
    return ret;
}

HCSVC_API
int api_create_capture_handle(char *handle)
{
    long long ret = 0;
    if(handle)
    {
        CaptureObj *obj = (CaptureObj *)calloc(1, sizeof(CaptureObj));
        //CodecObj *obj = (CodecObj *)&global_codec_objs[id];
        ret = (long long)obj;
        //handle = (void *)obj;
        int handle_size = sizeof(long long);
        printf("api_create_capture_handle: handle_size= %d \n", handle_size);
        printf("api_create_capture_handle: obj= %x \n", obj);
        printf("api_create_capture_handle: ret= %x \n", ret);
        memcpy(handle, &ret, handle_size);
        long long *testp = (long long *)handle;
        printf("api_create_capture_handle: testp[0]= %x \n", testp[0]);
        int id = (int)obj;//long long ???
        obj->Obj_id = id;
        ret = id;
    }
    return (int)ret;
}
//linux: "/dev/video0"; windows: "video=USB 2.0 CAMERA"
HCSVC_API
int api_capture_init(char *handle, char *param)
{
    int ret = 0;
    int got_picture = 0;
    AVPacket *packet;

    //char handle[8] = "";
    ret = api_create_capture_handle(handle);
    long long *testp = (long long *)handle;
    CaptureObj *obj = (CaptureObj *)testp[0];
    obj->json = mystr2json(param);
    obj->param = param;

    obj->osd_enable = GetvalueInt(obj->json, "osd");
    printf("api_capture_init: obj->osd_enable=%d \n", obj->osd_enable);
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
    obj->ratio = GetvalueInt(obj->json, "ratio");
    printf("api_capture_init: obj->ratio=%d \n", obj->ratio);
    obj->width = GetvalueInt(obj->json, "width");
    printf("api_capture_init: obj->width=%d \n", obj->width);
    obj->height = GetvalueInt(obj->json, "height");
    printf("api_capture_init: obj->height=%d \n", obj->height);
    //
    obj->cap_width = GetvalueInt(obj->json, "cap_width");
    printf("api_capture_init: obj->cap_width=%d \n", obj->cap_width);
    obj->cap_height = GetvalueInt(obj->json, "cap_height");
    printf("api_capture_init: obj->cap_height=%d \n", obj->cap_height);
    obj->framerate = GetvalueInt(obj->json, "framerate");
    printf("api_capture_init: obj->framerate=%d \n", obj->framerate);
    obj->input_format = GetvalueStr(obj->json, "input_format");
    printf("api_capture_init: obj->input_format=%s \n", obj->input_format);
    //
    obj->input_name = GetvalueStr(obj->json, "input_name");
    printf("api_capture_init: obj->input_name=%s \n", obj->input_name);
    obj->device_name = GetvalueStr(obj->json, "device_name");
    printf("api_capture_init: obj->device_name=%s \n", obj->device_name);

    obj->denoise = GetvalueInt(obj->json, "denoise");
    if(obj->denoise)
    {
        obj->img_hnd = (char *)calloc(1, 8 * sizeof(char));

        //OpenH264Version *pVersion;
        //WelsGetCodecVersionEx (pVersion);
        //WelsGxhTest();
        if(obj->denoise == 1)
        {
            ret = I2CreateVideoDenoise(obj->img_hnd);
        }
        else{
            ret = I2CreateVideoDenoise2(obj->img_hnd);
        }

        //ISVCEncoder *encoder = NULL;
        //WelsDestroySVCEncoder(encoder);
    }

    char *cscale_type = GetvalueStr(obj->json, "scale_type");
    printf("api_capture_init: cscale_type=%s \n", cscale_type);
    if (!strcmp(cscale_type, "SWS_BICUBIC"))
	{
       obj->scale_type = SWS_BICUBIC;
	}
	else if (!strcmp(cscale_type, "SWS_BILINEAR"))
	{
       obj->scale_type = SWS_BILINEAR;
	}
    char *cpixformat = GetvalueStr(obj->json, "pixformat");
	if (!strcmp(cpixformat, "AV_PIX_FMT_YUV420P"))
	{
       obj->pixformat = AV_PIX_FMT_YUV420P;
	}
	else if (!strcmp(cpixformat, "SDL_PIXELFORMAT_YV12"))
	{
       obj->pixformat = SDL_PIXELFORMAT_YV12;
	}
    printf("api_capture_init: obj->pixformat=%d \n", obj->pixformat);

    obj->max_buf_num = GetvalueInt(obj->json, "max_buf_num");
    printf("api_capture_init: obj->max_buf_num=%d \n", obj->max_buf_num);
    obj->frame_size = (3 * obj->width * obj->height) >> 1;
    int cap_frame_size = obj->cap_width * obj->cap_height * 4;//3;
    if(obj->max_buf_num > 0)
    {
        int frame_size = cap_frame_size;
	    obj->head_size = sizeof(int64_t) + sizeof(int64_t);//aline: 16
        frame_size += obj->head_size;//sizeof(int64_t);

        obj->cap_buf = (char **)calloc(1, obj->max_buf_num * sizeof(char **));
        for(int i = 0; i < obj->max_buf_num; i++)
        {
            obj->cap_buf[i] = (char *)av_malloc(frame_size);
            printf("api_capture_init: obj->cap_buf[i]=%x, i=%d \n", obj->cap_buf[i], i);
        }
        obj->tmp_buf = (char *)av_malloc(cap_frame_size);//max is 4:4:4
        pthread_mutex_init(&obj->mutex,NULL);

    }

	av_register_all();
	avformat_network_init();
    avdevice_register_all();

    int64_t time0 = get_sys_time();
    obj->inputFmt = av_find_input_format (obj->input_name);
    if (obj->inputFmt == NULL)    {
        printf("api_capture_init: can not find_input_format: obj->input_name=%s \n", obj->input_name);
        return -1;
    }
    int64_t time1 = get_sys_time();
    int difftime = (int)(time1 - time0);
    printf("api_capture_init: difftime=%d \n", difftime);

    printf("api_capture_init: 1 \n");
    if(!strcmp(obj->input_name, "x11grab"))
    {
        int screen_width = 0;
        int screen_height = 0;
        get_screen_wxh(&screen_width, &screen_height);
        if(screen_width * screen_height > 0)
        {
            obj->cap_width = screen_width;
            obj->cap_height = screen_height;
        }
    }
    AVDictionary* options = NULL;
    char text[16] = "";
    char ctmp[16] = "";
    //av_dict_set(&options,"list_formats","all",0);
    //av_dict_set(&options,"list_formats","raw",0);
    sprintf(text, "%d", obj->cap_width);
    strcat(text, "x");
    sprintf(ctmp, "%d", obj->cap_height);
    strcat(text, ctmp);
    av_dict_set(&options, "video_size", text, AV_DICT_MATCH_CASE);
	//av_dict_set(&options,"video_size","320x180",0);
	sprintf(text, "%d", obj->framerate);
	if(obj->framerate > 0)
	{
	    av_dict_set(&options, "framerate", text, AV_DICT_MATCH_CASE);
	}

	if(!strcmp(obj->input_format, "mjpeg"))
	{
	    av_dict_set(&options, "input_format", obj->input_format, AV_DICT_MATCH_CASE);
	}
    time0 = get_sys_time();
    if (avformat_open_input ( &obj->pFormatCtx, obj->device_name, obj->inputFmt, &options) < 0){
        printf("api_capture_init: can not find_input_format: obj->input_name=%s \n", obj->input_name);
        printf("api_capture_init: can not open_input_file: obj->device_name=%s \n", obj->device_name);
        printf("api_capture_init: can not open_input_file: obj->input_format=%s \n", obj->input_format);
        return -1;
    }
    time1 = get_sys_time();
    difftime = (int)(time1 - time0);
    printf("api_capture_init: avformat_open_input: difftime=%d \n", difftime);

    AVDictionaryEntry *t = NULL;
    t = av_dict_get(options, "framerate", NULL, AV_DICT_IGNORE_SUFFIX);
    if(t)
    {
        av_log(NULL, AV_LOG_DEBUG, "framerate: %s", t->value);
    }

    t = av_dict_get(options, "video_size", NULL, AV_DICT_IGNORE_SUFFIX);
    if(t)
    {
        av_log(NULL, AV_LOG_DEBUG, "video_size: %s", t->value);
    }

    t = av_dict_get(options, "list_formats", NULL, AV_DICT_IGNORE_SUFFIX);
    if(t)
    {
        av_log(NULL, AV_LOG_DEBUG, "list_formats: %s", t->value);
    }

    while((t = av_dict_get(options, "", t, AV_DICT_IGNORE_SUFFIX))){
        av_log(NULL, AV_LOG_DEBUG, "%s", "%s", t->key, t->value);
    }
    while((t = av_dict_get(obj->pFormatCtx->metadata, "", t, AV_DICT_IGNORE_SUFFIX))){
        //av_log(NULL, AV_LOG_DEBUG, "%s", "%s", t->key, t->value);
        printf("api_capture_init: %s, %s \n", t->key, t->value);
    }
    printf("api_capture_init 2 \n");
	//pFormatCtx = avformat_alloc_context();

	//if(avformat_open_input(&pFormatCtx,filepath,NULL,NULL)!=0){
	//	printf("Couldn't open input stream.\n");
	//	return -1;
	//}
	if(avformat_find_stream_info(obj->pFormatCtx,NULL) < 0){
		printf("Couldn't find stream information.\n");
		return -1;
	}

	//obj->videoindex = av_finde_best_stream(obj->pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &decodec, 0);
	obj->videoindex = -1;
	for(int i = 0; i < obj->pFormatCtx->nb_streams; i++)
		if(obj->pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
			obj->videoindex = i;
			break;
		}
	if(obj->videoindex == -1){
		printf("Didn't find a video stream.\n");
		return -1;
	}
	printf("api_capture_init 3 \n");
	obj->pCodecCtx = obj->pFormatCtx->streams[obj->videoindex]->codec;
	obj->pCodec = avcodec_find_decoder(obj->pCodecCtx->codec_id);
	if(obj->pCodec == NULL){
		printf("Codec not found.\n");
		return -1;
	}
	if(avcodec_open2(obj->pCodecCtx, obj->pCodec,NULL) < 0){
		printf("Could not open codec.\n");
		return -1;
	}
	printf("api_capture_init: obj->pCodecCtx->width=%d \n", obj->pCodecCtx->width);
	printf("api_capture_init: obj->pCodecCtx->height=%d \n", obj->pCodecCtx->height);
	int num = obj->pCodecCtx->framerate.num;
	int den = obj->pCodecCtx->framerate.den;
	if(num * den > 0)
	{
	    int framerate = num / den;
	    printf("api_capture_init: framerate=%d \n", framerate);
	}
	printf("api_capture_init: obj->pFormatCtx->iformat->name=%s \n", obj->pFormatCtx->iformat->name);
	printf("api_capture_init: obj->pFormatCtx->duration=%f \n", obj->pFormatCtx->duration / 1000000);
	obj->pFrame = av_frame_alloc();
	obj->pFrameYUV = av_frame_alloc();
    int n = 2;//预留余量
    int aline_size = 64;//32;

	int flag = 0;
	if(!flag)
	{
	    obj->scale_buf_size = n * av_image_get_buffer_size(obj->pixformat,  obj->width, obj->height,aline_size);
	    obj->out_buffer = (unsigned char *)av_malloc(obj->scale_buf_size);
	    av_image_fill_arrays(   obj->pFrameYUV->data,
	                            obj->pFrameYUV->linesize,
	                            obj->out_buffer,
		                        obj->pixformat,
		                        obj->width,
		                        obj->height,
		                        (aline_size >> 1));
	}
	else{
	    obj->scale_buf_size = av_image_get_buffer_size(obj->pixformat,  obj->width, obj->height,aline_size);
	    int ret2 = av_image_alloc(  obj->pFrameYUV->data,
                                    obj->pFrameYUV->linesize,
                                    obj->width,
		                            obj->height,
                                    obj->pixformat,
                                    aline_size);
	}


    printf("api_capture_init 4 \n");
	//Output Info-----------------------------
	//printf("---------------- File Information ---------------\n");
	//av_dump_format(pFormatCtx,0,filepath,0);
	//printf("-------------------------------------------------\n");
    printf("api_capture_init ok \n");
	return (ret & 0x7FFFFFFF);
}
HCSVC_API
void api_capture_close(char *handle)
{
    if(handle)
    {
        long long *testp = (long long *)handle;
        CaptureObj *obj = (CaptureObj *)testp[0];
        printf("api_capture_close: obj->Obj_id= %x \n", obj->Obj_id);

        sws_freeContext(obj->img_convert_ctx);
	    av_frame_free(&obj->pFrameYUV);
	    av_frame_free(&obj->pFrame);
	    avcodec_close(obj->pCodecCtx);
	    avformat_close_input(&obj->pFormatCtx);
	    int frame_size = (3 * obj->width * obj->height) >> 1;

        if(obj->cap_buf)
        {
            for(int i = 0; i < obj->max_buf_num; i++)
            {
                if(obj->cap_buf[i])
                {
                    av_free(obj->cap_buf[i]);
                    obj->cap_buf[i] = NULL;
                }
            }
            free(obj->cap_buf);
            obj->cap_buf = NULL;
            printf("api_capture_close: obj->cap_buf freed \n");
        }
        if(obj->tmp_buf)
        {
            av_free(obj->tmp_buf);
            obj->tmp_buf = NULL;
        }
        if(obj->img_hnd)
        {
            if(obj->denoise == 1)
            {
                I2VideoDenoiseClose(obj->img_hnd);
            }
            else{
                I2VideoDenoiseClose2(obj->img_hnd);
            }
             free(obj->img_hnd);
        }
        pthread_mutex_destroy(&obj->mutex);
        if(obj->scale_handle)
        {
            //
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
        printf("api_capture_close: ok \n");
    }

}
void get_ratio_wxh(int width, int height, int *dst_w, int *dst_h)
{
    int w = dst_w[0];
    int h = dst_h[0];
    int w2 = width * h / height;
    int h2 = w * height / width;
    //就小
    if(w2 <= w)
    {
        dst_w[0] = w2;
    }
    else{
        dst_h[0] = h2;
    }
}
void get_ratio_wxh2(int width, int height, int *dst_w, int *dst_h)
{
    int w = dst_w[0];
    int h = dst_h[0];
    int w2 = (width * h) / height;//h不变，求同ratio下的w
    int h2 = (w * height) / width;//w不变，求同ratio下的w
    int aline_bits = 6;//5;
    int aline_size_half = (1 << (aline_bits - 1));
    //printf("get_ratio_wxh2: w2=%d, h2=%d \n", w2, h2);
    h2 = ((h2 + aline_size_half) >> aline_bits) << aline_bits;
    w2 = ((w2 + aline_size_half) >> aline_bits) << aline_bits;
    //printf("get_ratio_wxh2: w2=%d, h2=%d \n", w2, h2);
    //就大
    if(h2 > h)
    {
        dst_h[0] = h2;//crop h, w不变
    }
    else{
        dst_w[0] = w2;//crop w, h不变
    }
}
int crop_frame(char *handle, AVFrame *pFrame, char *outbuffer)
{
    long long *testp = (long long *)handle;
    CaptureObj *obj = (CaptureObj *)testp[0];
    int frame_size = (3 * obj->width * obj->height) >> 1;
	int ret = frame_size;
    //
    int linesize[3] = { pFrame->linesize[0],
                        pFrame->linesize[1],
                        pFrame->linesize[2]};
    int w = obj->width;
    int h = obj->height;
    int w2 = w >> 1;
    int start_x = (pFrame->width - w) >> 1;
    int start_y = (pFrame->height - h) >> 1;
    char *src_y = &pFrame->data[0][start_x + start_y * linesize[0]];
    char *src_u = &pFrame->data[1][(start_x >> 1) + (start_y >> 1) * linesize[1]];
    char *src_v = &pFrame->data[2][(start_x >> 1) + (start_y >> 1) * linesize[2]];
    char *dst_y = &outbuffer[0];
    char *dst_u = &outbuffer[w * h];
    char *dst_v = &outbuffer[w * h + ((w >> 1) * (h >> 1))];

    //if(obj->max_buf_num > 0)
    //{
    //    pthread_mutex_lock(&obj->mutex);
    //}
    int diff = obj->in_idx - obj->out_idx;

    if(diff >= obj->max_buf_num || diff < 0)
    {
        printf("warning: crop_frame: diff=%d \n", diff);
    }
    for(int i = 0; i < h; i++)
    {
        memcpy(&dst_y[i * w], &src_y[i * linesize[0]], w);
        if((i & 1) == 0)
        {
            int I = i >> 1;
            memcpy(&dst_u[I * w2], &src_u[I * linesize[1]], w2);
            memcpy(&dst_v[I * w2], &src_v[I * linesize[2]], w2);
        }
    }
    //if(obj->max_buf_num > 0)
    //{
    //    pthread_mutex_unlock(&obj->mutex);
    //}

    return ret;
}

int adapt_scale(char *handle, char *outbuffer)
{
    int ret = 0;
    long long *testp = (long long *)handle;
    CaptureObj *obj = (CaptureObj *)testp[0];
    if(obj)
    {
        int linesize[3] = { obj->pFrameYUV->linesize[0],
	                        obj->pFrameYUV->linesize[1],
	                        obj->pFrameYUV->linesize[2]};
	    int w = obj->width;
	    int h = obj->height;
	    if(obj->ratio)
	    {
	        get_ratio_wxh(obj->pCodecCtx->width, obj->pCodecCtx->height, &w, &h);
	        linesize[0] = w;
	        linesize[1] = w >> 1;
	        linesize[2] = w >> 1;
	        //printf("adapt_scale: w=%d, h=%d \n", w, h);

	    }
	    if((obj->scale_width != w) || (obj->scale_height != h))
	    {
	        int frame_size = (3 * w * h) >> 1;
	        if(frame_size > obj->scale_buf_size)
	        {
	            printf("error: adapt_scale: frame_size=%d, obj->scale_buf_size=%d \n", frame_size, obj->scale_buf_size);
	            return ret;
	        }
#if 0
	        int ret2 = 0;
	        if(!obj->scale_handle && false)
	        {
	            obj->handle_size = 8;
	            obj->scale_handle = (char *)calloc(1, obj->handle_size * sizeof(char));
	            char ctmp[16] = "";
	            char param[1024] = "{";
	            strcat(param, "src_w:");
	            sprintf(ctmp, "%d", obj->pCodecCtx->width);
	            strcat(param, ctmp);
	            strcat(param, ",");

	            strcat(param, "src_h:");
	            sprintf(ctmp, "%d", obj->pCodecCtx->height);
	            strcat(param, ctmp);
	            strcat(param, ",");

	            strcat(param, "src_pix_fmt:");
	            sprintf(ctmp, "%d", obj->pCodecCtx->pix_fmt);
	            strcat(param, ctmp);
	            strcat(param, ",");

	            strcat(param, "dst_pix_fmt:");
	            sprintf(ctmp, "%d", obj->pixformat);
	            strcat(param, ctmp);
	            strcat(param, ",");

	            strcat(param, "scale_mode:");
	            sprintf(ctmp, "%d", obj->scale_type);
	            strcat(param, ctmp);
	            //strcat(param, ",");

	            strcat(param, "}");
	            printf("adapt_scale: param=%s \n", param);
	            ret2 = api_scale_init(obj->scale_handle, obj->param);
	            //
	            long long *testp = (long long *)handle;
                ScaleObj *scale = (ScaleObj *)testp[0];
	            scale->src_data[0] = obj->pFrame->data[0];
                scale->src_data[1] = obj->pFrame->data[1];
                scale->src_data[2] = obj->pFrame->data[2];
                scale->src_linesize[0] = obj->pFrame->linesize[0];
                scale->src_linesize[1] = obj->pFrame->linesize[1];
                scale->src_linesize[2] = obj->pFrame->linesize[2];
                //
                scale->dst_linesize[0] = linesize[0];
                scale->dst_linesize[1] = linesize[1];
                scale->dst_linesize[2] = linesize[2];

                scale->dst_data[0] = obj->pFrameYUV->data[0];
                scale->dst_data[1] = obj->pFrameYUV->data[1];
                scale->dst_data[2] = obj->pFrameYUV->data[2];
                //
                scale->scale_mode = obj->scale_type;
	        }
#endif
	        if(obj->img_convert_ctx)
	        {
	            sws_freeContext(obj->img_convert_ctx);
	            obj->img_convert_ctx = NULL;
	        }
	    }
        if(!obj->img_convert_ctx)
        {
            printf("adapt_scale: obj->pFrame->data[0]=%x \n", obj->pFrame->data[0]);
	        printf("adapt_scale: obj->pFrameYUV->data[0]=%x \n", obj->pFrameYUV->data[0]);
            obj->img_convert_ctx = sws_getContext(  obj->pCodecCtx->width,
	                                                obj->pCodecCtx->height,
	                                                obj->pCodecCtx->pix_fmt,
		                                            w,
		                                            h,
		                                            obj->pixformat,
		                                            obj->scale_type,
		                                            NULL, NULL, NULL);
		    printf("adapt_scale: w=%d, h=%d \n", w, h);
		    obj->scale_width = w;
	        obj->scale_height = h;
        }
        if(obj->img_convert_ctx)
        {
            sws_scale(  obj->img_convert_ctx,
		                (const unsigned char* const*)obj->pFrame->data,
				        obj->pFrame->linesize, 0,
				        obj->pCodecCtx->height,
				        obj->pFrameYUV->data,
				        linesize);
        }

        //if(obj->max_buf_num > 0)
		//{
		//    pthread_mutex_lock(&obj->mutex);
		//}
        int diff = obj->in_idx - obj->out_idx;

        if(diff >= obj->max_buf_num || diff < 0)
        {
            printf("warning: adapt_scale: diff=%d \n", diff);
        }
	    int stride = obj->pFrameYUV->linesize[0];
		char *dst_y = &outbuffer[0];
		char *dst_u = &outbuffer[obj->width * obj->height];
		char *dst_v = &outbuffer[obj->width * obj->height + ((obj->width >> 1) * (obj->height >> 1))];
		if((obj->width == stride) && (obj->width == w) && (obj->height == h))
		{
		    memcpy(dst_y, obj->pFrameYUV->data[0], w * h);
		    memcpy(dst_u, obj->pFrameYUV->data[1], ((w >> 1) * (h >> 1)));
		    memcpy(dst_v, obj->pFrameYUV->data[2], ((w >> 1) * (h >> 1)));

		    int frame_size = (3 * w * h) >> 1;
		    ret = frame_size;
		}
		else if(w * h < obj->width * obj->height)
		{
		    int frame_size = (3 * w * h) >> 1;
		    ret = frame_size;
		    frame_size = (3 * obj->width * obj->height) >> 1;

		    memset(outbuffer, 0, obj->width * obj->height);
		    memset(&outbuffer[obj->width * obj->height], 128, obj->width * (obj->height >> 1));
		    if(w == obj->width)
		    {
		        int start_y = (obj->height - h) >> 1;
		        char *dst_y2 = &dst_y[start_y * obj->width];
		        char *dst_u2 = &dst_u[(start_y >> 1) * (obj->width >> 1)];
		        char *dst_v2 = &dst_v[(start_y >> 1) * (obj->width >> 1)];

		        memcpy(dst_y2, obj->pFrameYUV->data[0], w * h);
		        memcpy(dst_u2, obj->pFrameYUV->data[1], ((w >> 1) * (h >> 1)));
		        memcpy(dst_v2, obj->pFrameYUV->data[2], ((w >> 1) * (h >> 1)));
		    }
		    else{
		        int start_x = (obj->width - w) >> 1;
		        char *dst_y2 = &dst_y[start_x];
		        char *dst_u2 = &dst_u[(start_x >> 1)];
		        char *dst_v2 = &dst_v[(start_x >> 1)];
		        int w2 = w >> 1;
		        int width = obj->width;
		        int width2 = width >> 1;

		        for(int i = 0; i < h; i++)
		        {
		            memcpy(&dst_y2[i * width], &obj->pFrameYUV->data[0][i * w], w);
		            if((i & 1) == 0)
		            {
		                int I = i >> 1;
		                memcpy(&dst_u2[I * width2], &obj->pFrameYUV->data[1][I * w2], w2);
		                memcpy(&dst_v2[I * width2], &obj->pFrameYUV->data[2][I * w2], w2);
		            }
		        }
		    }

		}
		else{
		    printf("adapt_scale: w=%d, obj->width=%d, stride=%d \n", w, obj->width, stride);
		}
		//if(obj->max_buf_num > 0)
		//{
		//    pthread_mutex_unlock(&obj->mutex);
		//}
	}
    return ret;
}
int adapt_scale2(char *handle, char *outbuffer)
{
    int ret = 0;
    long long *testp = (long long *)handle;
    CaptureObj *obj = (CaptureObj *)testp[0];
    if(obj)
    {
        int linesize[3] = { obj->pFrameYUV->linesize[0],
	                        obj->pFrameYUV->linesize[1],
	                        obj->pFrameYUV->linesize[2]};
	    int w = obj->width;
	    int h = obj->height;
	    if(obj->ratio)
	    {
	        get_ratio_wxh2(obj->pCodecCtx->width, obj->pCodecCtx->height, &w, &h);
	        linesize[0] = w;
	        linesize[1] = w >> 1;
	        linesize[2] = w >> 1;
	        //printf("adapt_scale2: w=%d, h=%d \n", w, h);
	        obj->pFrameYUV->data[0] = obj->out_buffer;
	        obj->pFrameYUV->data[1] = &obj->out_buffer[w * h];
	        obj->pFrameYUV->data[2] = &obj->out_buffer[w * h + ((w >> 1) * (h >> 1))];
	        obj->pFrameYUV->linesize[0] = w;
	        obj->pFrameYUV->linesize[1] = w >> 1;
	        obj->pFrameYUV->linesize[2] = w >> 1;
	        obj->pFrameYUV->width = w;
	        obj->pFrameYUV->height = h;
	        linesize[0] = w;
	        linesize[1] = w >> 1;
	        linesize[2] = w >> 1;
	    }

	    if((obj->scale_width != w) || (obj->scale_height != h))
	    {
	        int frame_size = (3 * w * h) >> 1;
	        if(frame_size > obj->scale_buf_size)
	        {
	            printf("error: adapt_scale2: frame_size=%d, obj->scale_buf_size=%d \n", frame_size, obj->scale_buf_size);
	            return ret;
	        }
	        if(obj->img_convert_ctx)
	        {
	            sws_freeContext(obj->img_convert_ctx);
	            obj->img_convert_ctx = NULL;
	        }
	    }
        if(!obj->img_convert_ctx)
        {
            printf("adapt_scale2: obj->pFrame->data[0]=%x \n", obj->pFrame->data[0]);
	        printf("adapt_scale2: obj->pFrameYUV->data[0]=%x \n", obj->pFrameYUV->data[0]);
	        printf("adapt_scale2: obj->pCodecCtx->pix_fmt=%d \n", obj->pCodecCtx->pix_fmt);

            obj->img_convert_ctx = sws_getContext(  obj->pCodecCtx->width,
	                                                obj->pCodecCtx->height,
	                                                obj->pCodecCtx->pix_fmt,
		                                            w,
		                                            h,
		                                            obj->pixformat,
		                                            obj->scale_type,
		                                            NULL, NULL, NULL);
		    printf("adapt_scale2: w=%d, h=%d \n", w, h);
		    obj->scale_width = w;
	        obj->scale_height = h;
        }
	    sws_scale(  obj->img_convert_ctx,
		            (const unsigned char* const*)obj->pFrame->data,
				    obj->pFrame->linesize, 0,
				    obj->pCodecCtx->height,
				    obj->pFrameYUV->data,
				    linesize);
        ret = crop_frame(handle, obj->pFrameYUV, outbuffer);
        //printf("adapt_scale2: ret=%d \n", ret);
	}
    return ret;
}
HCSVC_API
int api_capture_read_frame(char *handle, char *outbuf)
{
    int ret = 0;

    AVPacket *packet;
    char *outbuffer = outbuf;

    long long *testp = (long long *)handle;
    CaptureObj *obj = (CaptureObj *)testp[0];
    obj->ratio = GetvalueInt(obj->json, "ratio");
    //printf("api_capture_read_frame: obj->ratio=%d \n", obj->ratio);
    obj->print = GetvalueInt(obj->json, "print");
    //printf("api_capture_read_frame: obj->print=%d \n", obj->print);

	packet=(AVPacket *)av_malloc(sizeof(AVPacket));
    int64_t time0 = get_sys_time();
    while(1){
        ret = av_read_frame(obj->pFormatCtx, packet);
	    if(ret < 0)
	    {
	        obj->cap_read_error_cnt++;
	        return -1;
	    }

	    if(packet->stream_index==obj->videoindex)
	    {
	        break;
	    }
	}
	int64_t time1 = get_sys_time();
	int difftime = (int)(time1 - time0);
	if(obj->print)
	{
	    printf("api_capture_read_frame: difftime0= %d \n", difftime);
	    //printf("api_capture_read_frame: packet->stream_index= %d \n", packet->stream_index);
	    //printf("api_capture_read_frame: packet->flags= %d \n", packet->flags);
	    //printf("api_capture_read_frame: packet->pts= %d \n", packet->pts);
        //printf("api_capture_read_frame: obj->pCodecCtx->pix_fmt= %d \n", obj->pCodecCtx->pix_fmt);
	}


	if(ret >= 0)
    {
        if(obj->max_buf_num > 0)
		{
		    pthread_mutex_lock(&obj->mutex);
		}
		//
		int64_t ll_difftime = (int)(get_sys_time() - obj->cap_start_time_stamp);

		int flag = 1;

		if(obj->framerate > 0)
		{
		    int target_framerate = obj->framerate;
		    float interval = 1000.0 / target_framerate;
            //obj->in_idx * interval >= ll_difftime
            int64_t target_difftime = (int64_t)(obj->in_idx * 1000) / target_framerate;

            //printf("api_capture_read_frame: target_framerate= %d \n", target_framerate);
            //printf("api_capture_read_frame: target_difftime= %lld \n", target_difftime);
            flag = (target_difftime <= ll_difftime) | (!obj->cap_start_time_stamp);
		}

        if(flag)
		{
		    if((obj->max_buf_num > 0))
            {
                int I = obj->in_idx % obj->max_buf_num;
                int *p0 = (int *)obj->cap_buf[I];
                int64_t *p1 = (int64_t *)&obj->cap_buf[I][sizeof(int64_t)];
                char *data = &obj->cap_buf[I][obj->head_size];
                p0[0] = packet->size;
                p1[0] = get_sys_time();
                ret = packet->size;
                memcpy(data, packet->data, ret);
            }


            if(!obj->cap_start_time_stamp)
            {
                obj->cap_start_time_stamp = time0;
            }
            else if(ll_difftime > 1000)
            {
                //obj->cap_framerate = (int)(obj->in_idx / (ll_difftime / 1000));
                //if(obj->print)
                if((obj->in_idx % 300) == 1)
                {
                    //printf("api_capture_read_frame: obj->cap_framerate= %d fps \n", obj->cap_framerate);
	                //printf("api_capture_read_frame: obj->cap_read_error_cnt= %d \n", obj->cap_read_error_cnt);
                }

            }
            obj->in_idx++;
            //printf("api_capture_read_frame: obj->in_idx= %d \n", obj->in_idx);
		}

        if(obj->max_buf_num > 0)
		{
		    pthread_mutex_unlock(&obj->mutex);
		}
    }
    //printf("api_capture_read_frame: obj->cap_read_error_cnt=%d \n", obj->cap_read_error_cnt);
	//printf("api_capture_read_frame: ret=%d \n", ret);//
	//printf("api_capture_read_frame: packet->duration=%d \n", packet->duration);
	av_free_packet(packet);
	return ret;
}
HCSVC_API
int api_capture_read_frame2(char *handle, char *outbuf)
{
    int ret = 0;
    int got_picture = 0;
    long long *testp = (long long *)handle;
    CaptureObj *obj = (CaptureObj *)testp[0];
    //printf("api_capture_read_frame2: start \n");
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.flags = 1;
    pkt.stream_index = 0;//obj->videoindex;
    int64_t time0 = get_sys_time();

	if(obj->max_buf_num > 0)
    {
        pthread_mutex_lock(&obj->mutex);
    }
    int in_idx = obj->in_idx;
    if(!obj->last_time_stamp && (in_idx > 1))
    {
        obj->out_idx = in_idx - 1;
    }
    //printf("api_capture_read_frame2: obj->out_idx=%d \n", obj->out_idx);
    //printf("api_capture_read_frame2: in_idx=%d \n", in_idx);
    if(obj->out_idx < in_idx)
	{
	    int64_t time_stamp = 0;
	    char *data = NULL;
	    int size = 0;
	    int I0 = in_idx % obj->max_buf_num;
	    int diff = in_idx - obj->out_idx;
        if(diff > (obj->max_buf_num - 1))
        {
            //has overwrite
            obj->out_idx = in_idx - (obj->max_buf_num - 1);// + (obj->max_buf_num >> 1);
        }
        do
        {
            int I = obj->out_idx % obj->max_buf_num;
            int *p0 = (int *)obj->cap_buf[I];
            int64_t *p1 = (int64_t *)&obj->cap_buf[I][sizeof(int64_t)];
            data = &obj->cap_buf[I][obj->head_size];
            size = p0[0];
            time_stamp = p1[0];
            diff = in_idx - obj->out_idx;
            if(diff >= obj->max_buf_num || diff < 0)
            {
                printf("warning: api_capture_read_frame2: in_idx=%d, diff= %d \n", in_idx, diff);
            }
            int step = diff >> 1;
            step = step < 1 ? 1 : step;
            obj->out_idx += step;

            //if(diff > (obj->max_buf_num >> 1))
            //{
            //    obj->out_idx += (obj->max_buf_num >> 1);
            //}
            //else{
            //    obj->out_idx++;
            //}

            if(time_stamp > obj->last_time_stamp)
            {
                p0[0] = 0;
                ret = size;//obj->frame_size;
                break;
            }
        }while(obj->out_idx < in_idx);
        //printf("api_capture_read_frame2: ret=%d \n", ret);
        if(ret > 0)
        {
            memcpy(obj->tmp_buf, data, ret);
            obj->last_time_stamp = time_stamp;
            pkt.data = obj->tmp_buf;
            pkt.size = ret;
            //printf("api_capture_read_frame2: obj->out_idx= %d, in_idx=%d \n", obj->out_idx, in_idx);
        }
	}

    if(obj->max_buf_num > 0)
    {
        pthread_mutex_unlock(&obj->mutex);
    }
    if(ret <= 0)
    {
        av_free_packet(&pkt);
        return ret;
    }
	//
    //printf("api_capture_read_frame2: decode: pkt.size=%d \n", pkt.size);
	//printf("api_capture_read_frame: read ret= %d \n", ret);
	ret = avcodec_decode_video2(obj->pCodecCtx, obj->pFrame, &got_picture, &pkt);
	//printf("api_capture_read_frame: ret= %d \n", ret);
	if(ret < 0){
	    printf("api_capture_read_frame: Decode Error.\n");
	    obj->cap_read_error_cnt++;
		return -1;
	}
	//printf("api_capture_read_frame2: decode: ret=%d \n", ret);
	int64_t time1 = get_sys_time();
	int difftime = (int)(time1 - time0);
	if(obj->print)
	    printf("api_capture_read_frame2: difftime1= %d \n", difftime);
	//printf("api_capture_read_frame2: obj->pFrame->width=%d \n", obj->pFrame->width);
	//printf("api_capture_read_frame2: obj->pFrame->height=%d \n", obj->pFrame->height);
	if(got_picture){
	    int flag = obj->pCodecCtx->pix_fmt == obj->pixformat;
	    //printf("api_capture_read_frame: obj->pCodecCtx->pix_fmt=%d \n", obj->pCodecCtx->pix_fmt);

	    flag &= (obj->width <= obj->pFrame->width);
	    flag &= (obj->height <= obj->pFrame->height);
	    flag &= (obj->ratio == 2);
	    if(flag)
	    {
	        //crop
            ret = crop_frame(handle, obj->pFrame, outbuf);
            //
	        //av_free_packet(packet);
	        return ret;
	    }
	    if(obj->ratio <= 1)
	    {
	        ret = adapt_scale(handle, outbuf);
	    }
	    else{
	        ret = adapt_scale2(handle, outbuf);
	    }
	    if(obj->img_hnd && true)
	    {
	        time0 = get_sys_time();
	        int width = obj->width;
	        int height = obj->height;
	        char *src[3];
	        int w = width;
	        int h = height;
	        int linesize[3];
	        linesize[0] = w;
	        linesize[1] = w >> 1;
	        linesize[2] = w >> 1;
	        src[0] = &outbuf[0];
            src[1] = &outbuf[w * h];
            src[2] = &outbuf[w * h + ((w >> 1) * (h >> 1))];

            if(obj->denoise == 1)
            {
                int ret2 = I2VideoDenoise(obj->img_hnd, src, linesize, width, height);
            }
	        else{
	            int ret2 = I2VideoDenoise2(obj->img_hnd, src, linesize, width, height);
	        }
	        //printf("api_capture_read_frame: ret2=%d \n", ret2);
	        time1 = get_sys_time();
	        difftime = (int)(time1 - time0);
	        if(obj->print)
	            printf("I2VideoDenoise: difftime= %d \n", difftime);
	    }

	}
	else{
	    ret = 0;
	}
	//
	int step = 25;//50;
	if(!obj->start_time)
    {
        obj->start_time = api_get_time_stamp_ll();
    }
    int64_t now = api_get_time_stamp_ll();
    difftime = (int)(now - obj->start_time);

    if(difftime > 1000)
    {
        float sum_time = (difftime / 1000.0);//s
        if((obj->frame_num % step) == (step - 1))
        {
            obj->cap_framerate = (int)((float)step / (float)sum_time);
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
            obj->json = api_renew_json_int(obj->json, "width", obj->width);
            obj->json = api_renew_json_int(obj->json, "height", obj->height);
            obj->json = api_renew_json_int(obj->json, "orgX", obj->orgX);
            obj->json = api_renew_json_int(obj->json, "orgY", obj->orgY);
            obj->json = api_renew_json_int(obj->json, "scale", obj->scale);
            obj->json = api_renew_json_int(obj->json, "color", obj->color);
            obj->json = api_renew_json_str(obj->json, "src_pix_fmt", "AV_PIX_FMT_YUV420P");

            char* jsonstr2 = api_json2str(obj->json);
            int ret2 = api_simple_osd_init(obj->osd_handle, jsonstr2);
            api_json2str_free(jsonstr2);
	    }
	    int framerate = obj->cap_framerate;
        if(framerate > 0)
        {
            int orgX = 0;
            int orgY = obj->height - (obj->height / 16);
            orgY = (obj->height / 16);
            char context[255] = "";
            obj->json = api_renew_json_int(obj->json, "orgX", orgX);
            obj->json = api_renew_json_int(obj->json, "orgY", orgY);
            sprintf(&context[strlen(context)], "  %d", obj->width);
            sprintf(&context[strlen(context)], "   %d", obj->height);
            sprintf(&context[strlen(context)], "   %2d", framerate);
            strcat(context, "  ");
            strcat(context, obj->input_format);
            obj->json = api_renew_json_str(obj->json, "context", context);
            char* jsonstr2 = api_json2str(obj->json);
            int ret2 = api_simple_osd_process(  obj->osd_handle,
                                                (char *)outbuf,
                                                jsonstr2);
            api_json2str_free(jsonstr2);
            //printf("api_capture_read_frame2: ret2= %d \n", ret2);
      }
	}
    av_free_packet(&pkt);
	obj->frame_num++;
    return ret;
}

#include "inc.h"



int FFGetFormat(char *cformat)
{
	if(!strcmp(cformat, "AV_PIX_FMT_YUV420P"))
	{
		return AV_PIX_FMT_YUV420P;
	}
	else if(!strcmp(cformat, "AV_PIX_FMT_NV12"))
	{
		return AV_PIX_FMT_NV12;
	}
	else if(!strcmp(cformat, "AV_SAMPLE_FMT_S16"))
	{
		return AV_SAMPLE_FMT_S16;
	}
	else if(!strcmp(cformat, "AV_SAMPLE_FMT_FLTP"))
	{
		return AV_SAMPLE_FMT_FLTP;
	}
	return 0;
}

void *FFScaleCreate(void *handle)
{
	if(handle == NULL)
	{
		FFScale *h = (FFScale *)malloc(sizeof(FFScale));
		h->sws_ctx = NULL;
		handle = (void *)h;
	}
	
	return (void *)handle;
}
void FFScaleClose(void *handle)
{
	if(handle)
	{		
		FFScale *h = (FFScale *)handle;
		if(h->sws_ctx)
		{
			sws_freeContext(h->sws_ctx);
			h->sws_ctx = NULL;
		}
		free(h);
	}
}
int FFScaleParams(void *scaleHandle, int format,unsigned char *y, unsigned char *u, unsigned char *v, int width, int stride,int height, int flag)
{
	FFScale *scale = (FFScale *)scaleHandle;
	if(scaleHandle == NULL)
		return -1;
	if(!flag)
	{
		/*if(scale->srcFrame == NULL)
			return -1;*/
		scale->srcFrame.format = format;
		scale->srcFrame.data[0] = y;
		scale->srcFrame.data[1] = u;
		scale->srcFrame.data[2] = v;
		scale->srcFrame.linesize[0] = stride;
		scale->srcFrame.linesize[1] = stride >> (format == AV_PIX_FMT_YUV420P);
		if(format == AV_PIX_FMT_YUV420P)
			scale->srcFrame.linesize[2] = stride >> (format == AV_PIX_FMT_YUV420P);
		else
			scale->srcFrame.linesize[2] = 0;
		scale->srcFrame.width = width;
		scale->srcFrame.height = height;
	}
	else
	{
		/*if(scale->dstFrame == NULL)
			return -1;*/
		scale->dstFrame.format = format;
		scale->dstFrame.data[0] = y;
		scale->dstFrame.data[1] = u;
		scale->dstFrame.data[2] = v;
		scale->dstFrame.linesize[0] = stride;
		scale->dstFrame.linesize[1] = stride >> (format == AV_PIX_FMT_YUV420P);
		if(format == AV_PIX_FMT_YUV420P)
			scale->dstFrame.linesize[2] = stride >> (format == AV_PIX_FMT_YUV420P);
		else
			scale->dstFrame.linesize[2] = 0;
		scale->dstFrame.width = width;
		scale->dstFrame.height = height;
	}
	return 0;
}
int FFScaleInit(void *scaleHandle)//void &sws_ctx)
{
	FFScale *scale = (FFScale *)scaleHandle;

	if(scale->sws_ctx == NULL)
	{
		//struct SwsContext *sws = scale->sws_ctx;//(struct SwsContext *)sws_ctx;
		int src_width = scale->srcFrame.width;
		int src_height = scale->srcFrame.height;
		int src_format = scale->srcFrame.format;
		int dst_width = scale->dstFrame.width;
		int dst_height = scale->dstFrame.height;
		int dst_format = scale->dstFrame.format;
		int mode = (src_width * src_height < dst_width * dst_height) ? SWS_FAST_BILINEAR : SWS_POINT;

		scale->sws_ctx = sws_getContext(src_width, src_height, src_format,//AV_PIX_FMT_YUV420P,
			dst_width, dst_height, dst_format,//AV_PIX_FMT_YUV420P,
			mode, NULL, NULL, NULL);//SWS_POINT

	}
	if(scale->sws_ctx == NULL)
		return -1;
	return 0;
}
int FFScaleFrame(void *scaleHandle)
{
	int ret = 0;
	FFScale *scale = (FFScale *)scaleHandle;
	struct SwsContext *sws = scale->sws_ctx;
	if(!scale->dstFrame.data[2])
	{
		return -1;
	}
	ret = 
	sws_scale(	sws, 
				(const uint8_t **)scale->srcFrame.data, 
				scale->srcFrame.linesize,
				0, 
				scale->srcFrame.height, 
				scale->dstFrame.data, 
				scale->dstFrame.linesize);
	return ret;
}



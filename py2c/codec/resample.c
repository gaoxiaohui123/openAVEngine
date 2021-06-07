#include "inc.h"

void *ResampleCreate(void *handle)
{
	if(handle == NULL)
	{
		FFResample *h = (FFResample *)malloc(sizeof(FFResample));
		h->swr_ctx = NULL;
		handle = (void *)h;
	}
	
	return (void *)handle;
}
void ResampleClose(void *handle)
{
	if(handle)
	{		
		FFResample *h = (FFResample *)handle;
		if(h->swr_ctx)
		{
			swr_free(&h->swr_ctx);
			h->swr_ctx = NULL;
		}
		free(h);
	}
}
int ResampleParams(void *resampleHandle, unsigned char *data[3], int format, int channels, int nb_samples,int sample_rate, int flag)
{
	FFResample *resample = (FFResample *)resampleHandle;
	if(resampleHandle == NULL)
		return -1;
	if(!flag)
	{
	    for(int i = 0; i < channels; i++)
	    {
	        resample->srcFrame.data[i] = data[i];
	    }
		resample->srcFrame.channels = channels;
		resample->srcFrame.nb_samples = nb_samples;
		resample->srcFrame.sample_rate = sample_rate;
		resample->srcFrame.format = format;
	}
	else
	{
	    for(int i = 0; i < channels; i++)
	    {
	        resample->dstFrame.data[i] = data[i];
	    }
		resample->dstFrame.channels = channels;
		resample->dstFrame.nb_samples = nb_samples;
		resample->dstFrame.sample_rate = sample_rate;
		resample->dstFrame.format = format;
	}

	
	return 0;
}
int ResampleInit(void *resampleHandle)//void &sws_ctx)
{
	FFResample *resample = (FFResample *)resampleHandle;
	int ret = 0;
	if(resample->swr_ctx == NULL)
	{
		//enum AVSampleFormat sample_fmt;
		resample->swr_ctx = swr_alloc();
		if(resample->swr_ctx == NULL)
			return -1;

		/* set options */
		av_opt_set_int       (resample->swr_ctx, "in_channel_count",   resample->srcFrame.channels,       0);
		av_opt_set_int       (resample->swr_ctx, "in_sample_rate",     resample->srcFrame.sample_rate,    0);
		av_opt_set_sample_fmt(resample->swr_ctx, "in_sample_fmt",      (enum AVSampleFormat)resample->srcFrame.format, 0);
		av_opt_set_int       (resample->swr_ctx, "out_channel_count",  resample->dstFrame.channels,       0);
		av_opt_set_int       (resample->swr_ctx, "out_sample_rate",    resample->dstFrame.sample_rate,    0);
		av_opt_set_sample_fmt(resample->swr_ctx, "out_sample_fmt",     (enum AVSampleFormat)resample->dstFrame.format,     0);

		//
		if ((ret = swr_init(resample->swr_ctx)) < 0) {
			return -1;
		}
#if 0
		ret = av_rescale_rnd(swr_get_delay(resample->swr_ctx, resample->srcFrame.sample_rate) + resample->srcFrame.nb_samples,
			resample->dstFrame.sample_rate,
			resample->srcFrame.sample_rate,
			AV_ROUND_UP
			);
		if (ret > 0)
		{
			resample->dstFrame.nb_samples = ret;
		}
#endif
	}
	
	return ret;
}
int ResampleFrame(void *resampleHandle)
{
	int ret = 0;
	FFResample *resample = (FFResample *)resampleHandle;
	struct SwrContext *swr = resample->swr_ctx;
#if 0
	dst_nb_samples = av_rescale_rnd(swr_get_delay(audio->swr_ctx, c->sample_rate) + frame->nb_samples,
		c->sample_rate, c->sample_rate, AV_ROUND_UP);
#endif
    //printf("ResampleFrame: swr=%x \n", swr);
    //printf("ResampleFrame: resample->dstFrame.data[0]=%x \n", resample->dstFrame.data[0]);
    //printf("ResampleFrame: resample->srcFrame.data[0]=%x \n", resample->srcFrame.data[0]);
    //printf("ResampleFrame: resample->dstFrame.nb_samples=%d \n", resample->dstFrame.nb_samples);
    //printf("ResampleFrame: resample->srcFrame.nb_samples=%d \n", resample->srcFrame.nb_samples);
	ret = swr_convert(	swr,
						resample->dstFrame.data, resample->dstFrame.nb_samples,
						(const uint8_t **)resample->srcFrame.data, resample->srcFrame.nb_samples);

	return ret;
}



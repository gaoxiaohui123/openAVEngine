

//HCSVC_API
int api_create_video_denoise_handle(char *handle);
//HCSVC_API
int api_video_denoise_close(char *handle);
//HCSVC_API
int api_video_denoise(char *handle, unsigned char *data[3], int linesize[3], int width, int height);

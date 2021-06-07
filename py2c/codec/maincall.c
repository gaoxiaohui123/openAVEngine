/***gcc -o libpycall.so -shared -fPIC pycall.c*/
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

#include "../webrtc/hcsvc.h"
void main()
{
    printf("main: start \n");
#if 0
    {
        char buf[1024]={0};
        int read_size = 1024;
        char *cmd = "ffmpeg -list_devices true -f dshow -i dummy";
        int ret = api_get_cmd(cmd, buf, read_size);
        if(ret > 0)
        {
            printf("main: buf= %s \n", buf);
        }
    }
    return;
#endif
    {
        char *outparam[4];
        char buf[256] ;
        outparam[0] = buf;
        api_get_time(outparam);
        printf("main: buf= %s \n", buf);
        return;
    }

    api_register_test();

    //api_show_device();
    char *cmd = "select_video";
    char buf2[10240] = "";
    memset(buf2, 0, 1024 * sizeof(char));
    int ret = api_get_dev_info(cmd, buf2);
    if(ret > 0)
    {
        printf("main: buf= %s \n", buf2);
    }

    char *cmd2 = "select_audio";
    memset(buf2, 0, 1024 * sizeof(char));
    ret = api_get_dev_info(cmd2, buf2);
    if(ret > 0)
    {
        printf("main: buf= %s \n", buf2);
    }

    printf("main: end \n");
}
//gcc -o libpycall.so -shared -fPIC pycall.c
//注意：链接库的顺序极其重要，顺序不当，会报各种链接错误，即便动态库编译通过，调用时也会报错
//gcc -o decoding_encoding.so -shared -fPIC -Wl,-Bsymbolic decoding_encoding.c -I/data/home/gxh/works/ffmpeg-linux/ffmpeg-2.8.7 -L/data/home/gxh/works/ffmpeg-linux/mylib -L/usr/lib -lavformat -lavdevice -lavcodec -lavutil -lswresample -lswscale -lavfilter -laacplus -lx264 -lcjson -lpthread -lm
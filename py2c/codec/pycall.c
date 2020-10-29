/***gcc -o libpycall.so -shared -fPIC pycall.c*/
#include <stdio.h>
#include <stdlib.h>
int foo(int a, int b)
{
  printf("you input %d and %d\n", a, b);
  return a+b;
}
char * ret_str(void)
{
    const char *ret = "string come from c code";
    return (char *)ret;
}

//gcc -o libpycall.so -shared -fPIC pycall.c
//注意：链接库的顺序极其重要，顺序不当，会报各种链接错误，即便动态库编译通过，调用时也会报错
//gcc -o decoding_encoding.so -shared -fPIC -Wl,-Bsymbolic decoding_encoding.c -I/data/home/gxh/works/ffmpeg-linux/ffmpeg-2.8.7 -L/data/home/gxh/works/ffmpeg-linux/mylib -L/usr/lib -lavformat -lavdevice -lavcodec -lavutil -lswresample -lswscale -lavfilter -laacplus -lx264 -lcjson -lpthread -lm
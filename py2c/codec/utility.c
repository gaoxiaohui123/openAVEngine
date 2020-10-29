#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef linux
#include <unistd.h>
#include <linux/fb.h>
#endif

#include "inc.h"

extern cJSON* mystr2json(char *text);
extern int GetvalueInt(cJSON *json, char *key);
extern char* GetvalueStr(cJSON *json, char *key);
extern cJSON* renewJson(cJSON *json, char *key, int ivalue, char *cvalue, cJSON *subJson);
extern cJSON* deleteJson(cJSON *json);

typedef struct {
    char            riffType[4];    //4byte,资源交换文件标志:RIFF   
    unsigned int    riffSize;       //4byte,从下个地址到文件结尾的总字节数  
    char            wavType[4]; //4byte,wav文件标志:WAVE    
    char            formatType[4];  //4byte,波形文件标志:FMT(最后一位空格符)    
    unsigned int    formatSize;     //4byte,音频属性(compressionCode,numChannels,sampleRate,bytesPerSecond,blockAlign,bitsPerSample)所占字节数
    unsigned short  compressionCode;//2byte,格式种类(1-线性pcm-WAVE_FORMAT_PCM,WAVEFORMAT_ADPCM)
    unsigned short  numChannels;    //2byte,通道数
    unsigned int    sampleRate;     //4byte,采样率
    unsigned int    bytesPerSecond; //4byte,传输速率
    unsigned short  blockAlign;     //2byte,数据块的对齐，即DATA数据块长度
    unsigned short  bitsPerSample;  //2byte,采样精度-PCM位宽
    char            dataType[4];    //4byte,数据标志:data
    unsigned int    dataSize;       //4byte,从下个地址到文件结尾的总字节数，即除了wav header以外的pcm data length
} head_data_t;

int pcmAddWavHeader(FILE *fp, int channels, int bits, int sample_rate, int len) {
    head_data_t pcm2wavHEAD;
    if (NULL == fp) {
        printf("Input file ptr is null.\n"); 
        return -1; 
    }   
    memcpy(pcm2wavHEAD.riffType, "RIFF", strlen("RIFF"));
    memcpy(pcm2wavHEAD.wavType, "WAVE", strlen("WAVE"));
    pcm2wavHEAD.riffSize = 36 + len;
    pcm2wavHEAD.sampleRate = sample_rate;
    pcm2wavHEAD.bitsPerSample = bits;
    memcpy(pcm2wavHEAD.formatType, "fmt ", strlen("fmt "));
    pcm2wavHEAD.formatSize = 16;
    pcm2wavHEAD.numChannels = channels;
    pcm2wavHEAD.blockAlign = channels * bits / 8;
    pcm2wavHEAD.compressionCode = 1;
    pcm2wavHEAD.bytesPerSecond = pcm2wavHEAD.sampleRate * pcm2wavHEAD.blockAlign;
    memcpy(pcm2wavHEAD.dataType, "data", strlen("data"));
    pcm2wavHEAD.dataSize = len;
    fseek(fp, 0, SEEK_SET);
    fwrite(&pcm2wavHEAD, 44, 1, fp);
    return 0;
}

HCSVC_API
int api_pcm2wav(char *srcfile, char *dstfile, int channels, int bits, int sample_rate)
{
    int total_data = 0;
    FILE *orifp = fopen(srcfile,"rb");
    if(NULL == orifp){
        printf("OPEN FILE FAIL\n");
        return -1;
    }
    FILE *tarfp = fopen(dstfile,"wb");
    if(NULL == tarfp){
        printf("OPEN FILE FAIL\n");
        return -1;
    }
    fseek(orifp, 0,SEEK_END);
    total_data = ftell(orifp);
    char *mempcm;
    mempcm = (char *)malloc(total_data);
    rewind(orifp);
    int lenn = fread(mempcm,sizeof(char),total_data,orifp);
    pcmAddWavHeader(tarfp, channels, bits, sample_rate, total_data);
    fseek(tarfp,44,SEEK_SET);
    fwrite(mempcm,total_data,1,tarfp);
    free(mempcm);
    mempcm = NULL;
    fclose(orifp);
    fclose(tarfp);
    orifp = NULL;
    tarfp = NULL;
}
//pcmAddWavHeader(tarfp, channel, bit, sample_rate, total_data);
//fseek(tarfp,44,SEEK_SET);
//��ȡ�ַ�����ַ���
static char *mystrcat(char *szPath, char tok)
{
	char* pPos = NULL;
	pPos = strrchr(szPath, tok);//'__');//����һ���ַ�����һ���ַ�����"���һ��"���ֵ�λ��
	if (pPos == NULL)
		return NULL;
	*pPos = '\0';
	return ++pPos;
}
//��ȡ�ַ�ǰ���ַ���
static char *mystrcat2(char *szPath, char tok)
{
	char* pPos = NULL;
	pPos = strrchr(szPath, tok);//'__');//����һ���ַ�����һ���ַ�����"���һ��"���ֵ�λ��
	if (pPos == NULL)
		return NULL;//szPath;
	*pPos = '\0';
	return szPath;
}
HCSVC_API
int *api_get_array_by_str(char *text, char tok, int *num)
{
    int *ret = NULL;
    int i = 0;
	char *tmp = NULL;
	char *pPos = NULL;
	char *text2 = text;
    if(strlen(text) > 0)
    {
        int mem_num = (strlen(text) >> 1);
        mem_num = mem_num ? mem_num : 1;
        ret = malloc(mem_num * sizeof(int));
    }
    else{
        return ret;
    }
	do
	{
		pPos = strchr(text2, tok);
		if (pPos)
		{
		    char ctmp[32] = "";
			strncpy(ctmp, text2, (int)(pPos - text2));
			text2 = ++pPos;
			ret[i] = atoi(ctmp);
			i++;
		}
		else
		{
		    char ctmp[32] = "";
			strcpy(ctmp, text2);
			ret[i] = atoi(ctmp);
		}
	} while (pPos);

	if(i)
	{
	    num[0] = i + 1;
	}
	else{
	    if(strlen(text) > 0)
	    {
	        ret[0] = atoi(text);
	        num[0] = 1;
	    }
	    else{
	        num[0] = 0;
	        free(ret);
	        ret = NULL;
	    }

	}
	return ret;
}
HCSVC_API
void api_get_array_free(int *pktSize)
{
    if(pktSize)
    {
        free(pktSize);
    }
}

int executeCMD(const char *cmd, char *result, int read_size)
{
    //char buf_ps[512];
    //char ps[512]={0};
    int ret = 0;
    FILE *fp;
    //strcpy(ps, cmd);
    if((fp = popen(cmd, "r")) != NULL)
    {
         //ret = fread(result, read_size, 1, fp);
         ret = fread(result, 1, read_size, fp);
         //printf("executeCMD: ret=%d \n", ret);
         //if(fgets(result, read_size, fp) != NULL)
         //{
         //   ret = strlen(result);
         //   printf("executeCMD: 2: ret=%d \n", ret);
         //}
        //while(fgets(buf_ps, read_size, fp) != NULL)
        //{
        //   strcat(result, buf_ps);
        //   ret += strlen(result);
        //   if(strlen(result) >= read_size)
        //       break;
        //}
        pclose(fp);
        fp = NULL;
    }
    else
    {
        printf("executeCMD: popen %s error\n", cmd);
    }
    return ret;
}
char *findstr(char *src, char *key)
{
    char *p = strstr(src, key);
    return p;
}
HCSVC_API
int api_get_cmd(char *cmd, char *buf, int read_size)
{
    int ret = 0;
#if 0
    system(cmd);
#elif(1)
    memset(buf, 0, read_size * sizeof(char));
    ret = executeCMD(cmd, buf, read_size);
    //
    char *p = findstr(buf, "Compressed:");
    if(p)
    {
        //printf("api_get_cmd: p=%s \n", p);
    }
    else{
        //printf("api_get_cmd: p=%x \n", p);
    }
#else
    int fd[2];
    int backfd;
    pipe(fd);
    backfd = dup(STDOUT_FILENO);//备份标准输出，用于恢复
    dup2(fd[1],STDOUT_FILENO);  //将标准输出重定向到fd[1]
    system(cmd);
    //printf("api_get_cmd: after cmd \n");
    ret = read(fd[0], buf, read_size);
    printf("api_get_cmd: ret=%d \n", ret);
    dup2(backfd,STDOUT_FILENO);  //恢复标准输出
    printf("api_get_cmd: buf=%s \n", buf);  //上面不恢复，则此处的执行结果无法再屏幕上打印
#endif
    return ret;
}
int get_resolution(char *key, char *buf, char *result)
{
    int ret = 0;
    char *p = NULL;
    char key2[16] = "raw";
    //memset(key2, 0, 16 * sizeof(char));
    p = buf;

    p = findstr(p, key);
    if(p)
    {
        char *p1 = p;//&p[strlen(key)];
        char *p2 = NULL;
        char tmp[16] = "";
        do{

            p2 = findstr(p1, ":");
            if(p2)
            {
                int n = (int)((long long)p2 - (long long)p1);
                memset(tmp, 0, 16 * sizeof(char));
                //printf("api_get_dev_info: n=%d \n", n);
                if(n < 16)
                {
                    strncpy(tmp, p1, n);
                    //printf("get_resolution: tmp=%s \n", tmp);
                    if(strcmp(key, "Raw") && strcmp(tmp, "Compressed") && !strcmp(key2, "raw"))
                    {
                        strcpy(key2, tmp);
                        //printf("get_resolution: key2=%s \n", key2);
                    }
                }

                //printf("get_resolution: key2=%s \n", key2);

                p2++;
                p1 = p2;
            }
        }while(p2);
        if(p1)
        {
            char *p2 = findstr(buf, "\n");
            if(p2)
            {
                p2[0] = ' ';//python的json中'\n'为非法字符
            }
            printf("get_resolution: p1=%s \n", p1);
            if(strlen(result) > 0)
            {
                strcat(result, ",");
            }
            strcat(result, "\"");
            strcat(result, key2);
            //strcat(result, key);
            strcat(result, "\"");
            strcat(result, ":\"");
            strcat(result, p1);
            strcat(result, "\"");
            ret = strlen(result);
            //printf("get_resolution: key2=%s \n", key2);
        }
    }


    return ret;
}
int get_video_option(char *cmd, char *buf, char *result)
{
    int ret = 0;
    char *p0 = findstr(buf, "Compressed");
    char *p1 = findstr(buf, "Raw");
    char *p2 = NULL;
    char tmp[1024] = "";
    char tmp2[1024] = "";
    char tmp3[1024] = "";
    int n = 0;
    printf("get_video_option: start \n");
    if(p0 && p1)
    {
        n = (int)((long long)p1 - (long long)p0);

        if(n > 0)
        {
            strncpy(tmp, p0, n);
            p0 = tmp;
            p2 = findstr(p0, "[");
            if(p2)
            {
                n = (int)((long long)p2 - (long long)p0);
                if(n > 0)
                {
                    strncpy(tmp2, p0, n);
                    p0 = tmp2;
                }
            }
            //
            p2 = findstr(p1, "/dev");
            if(p2)
            {
                n = (int)((long long)p2 - (long long)p1);
                if(n > 0)
                {
                    strncpy(tmp3, p1, n);
                    p1 = tmp3;
                }
            }
        }
        else{
            strncpy(tmp, p1, -n);
            p1 = tmp;

            p2 = findstr(p1, "[");
            if(p2)
            {
                n = (int)((long long)p2 - (long long)p1);
                if(n > 0)
                {
                    strncpy(tmp2, p1, n);
                    p1 = tmp2;
                }
            }
            //
            p2 = findstr(p0, "/dev");
            if(p2)
            {
                n = (int)((long long)p2 - (long long)p0);
                if(n > 0)
                {
                    strncpy(tmp3, p0, n);
                    p0 = tmp3;
                }
            }
        }
        ret = get_resolution("Compressed", p0, result);
        ret = get_resolution("Raw", p1, result);
    }
    else{
        if(p0)
        {
            p2 = findstr(p0, "/dev");
            if(p2)
            {
                n = (int)((long long)p2 - (long long)p0);
                if(n > 0)
                {
                    strncpy(tmp3, p0, n);
                    p0 = tmp3;
                }
            }
            ret = get_resolution("Compressed", p0, result);
        }
        if(p1)
        {
            p2 = findstr(p1, "/dev");
            if(p2)
            {
                n = (int)((long long)p2 - (long long)p1);
                if(n > 0)
                {
                    strncpy(tmp3, p1, n);
                    p1 = tmp3;
                }
            }
            ret = get_resolution("Raw", p1, result);
        }
    }
    //printf("get_video_option: result=%s \n", result);
    printf("get_video_option: end \n");
    return ret;
}
int get_video_info(char *cmd, char *buf)
{
    int ret = 0;
    char result[2048] = "";
    char cmd2[1024] = "nohup ffmpeg -f v4l2 -list_formats all -i /dev/video0";
    int read_size = 1024;
    char *p = NULL;

    strcpy(cmd2, "ls /dev | grep video");
    memset(buf, 0, read_size * sizeof(char));
    read_size = 1024;
    ret = executeCMD(cmd2, buf, read_size);
    if(ret > 0)
    {   printf("api_get_dev_info: buf=%s \n", buf);
        p = buf;
        do{
            p = findstr(p, "video");
            if(p)
            {
                char tmp[16] = "";
                //printf("get_video_info: p=%s \n", p);
                char *p2 = findstr(p, "\n");
                if(p2)
                {
                    int n = (int)((long long)p2 - (long long)p);
                    //printf("api_get_dev_info: n=%d \n", n);
                    if(n > strlen("video"))
                    {
                        strncpy(tmp, p, n);
                    }
                    else{
                        strcmp(tmp, p);
                    }

                }
                else{
                    strcmp(tmp, p);
                }
                if(!strncmp(tmp, "video", strlen("video")))
                {
                    printf("get_video_info: tmp=%s \n", tmp);
                    strcpy(cmd2, "nohup ffmpeg -f v4l2 -list_formats all -i /dev/");
                    strcat(cmd2, tmp);
                    printf("api_get_dev_info: cmd2=%s \n", cmd2);
                    read_size = 10240;
                    char buf2[10240] = "";
                    memset(buf2, 0, read_size * sizeof(char));
                    ret = executeCMD(cmd2, buf2, read_size);
                    if(ret > 0)
                    {
                        printf("api_get_dev_info: buf=%s \n", buf);
                        char result2[2048] = "";
                        ret = get_video_option("", buf2, result2);
                        if(strcmp(result2, "") && ret > 0)
                        {
                            if(strlen(result) > 0)
                            {
                                strcat(result, ",");
                            }
                            //strcat(result, "{");
                            strcat(result, "\"");
                            strcat(result, tmp);
                            strcat(result, "\":{");
                            strcat(result, result2);
                            strcat(result, "}");
                            printf("get_video_info: result=%s \n", result);

                        }
                    }
                }
                p += strlen("video");
            }

        }while(p);
    }
    ret = strlen(result);
    if(ret > 0)
    {
        strcpy(buf, result);
    }
    return ret;
}
int get_screen_resolution(char *cmd, int *width, int *height)
{
    int ret = 0;
    //char *cmd = "nohup chmod 0777 /dev/fb0";
    char outbuf[1024] = "";
    ret = api_get_cmd(cmd, outbuf, 1024);
    int fd;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    int screensize = 0;
    fd = open("/dev/fb0",O_RDWR);
    /*取得屏幕相关参数*/
    ioctl(fd, FBIOGET_FSCREENINFO, &finfo);
    ioctl(fd, FBIOGET_VSCREENINFO, &vinfo);
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

    //printf("get_screen_resolution: %d*%d\n",vinfo.xres, vinfo.yres);
    width[0] = vinfo.xres;
    height[0] = vinfo.yres;
    close(fd);
    return ret;
}
int get_screen_info(char *cmd, char *buf)
{
    int ret = 0;
    char result[2048] = "";
    char *p = NULL;
    char *p2 = NULL;
    char cmd2[1024] = "ls /dev | grep fb";
    int read_size = 1024;
    ret = executeCMD(cmd2, buf, read_size);
    if(ret > 0)
    {
        p = buf;
        do{
            p = findstr(p, "fb");
            if(p)
            {
                char tmp[16] = "";
                printf("get_screen_info: p=%s \n", p);
                char *p2 = findstr(p, "\n");
                if(p2)
                {
                    int n = (int)((long long)p2 - (long long)p);
                    printf("get_screen_info: n=%d \n", n);
                    if(n > strlen("fb"))
                    {
                        strncpy(tmp, p, n);
                    }
                    else{
                        strcmp(tmp, p);
                    }
                }
                else{
                    strcmp(tmp, p);
                }
                if(!strncmp(tmp, "fb", strlen("fb")))
                {
                    printf("get_screen_info: tmp=%s \n", tmp);
                    int width = 0;
                    int height = 0;

                    strcpy(cmd2, "nohup chmod 0777 /dev/");
                    strcat(cmd2, tmp);
                    printf("get_screen_info: cmd2=%s \n", cmd2);
                    ret = get_screen_resolution(cmd2, &width, &height);
                    printf("get_screen_info: width=%d \n", width);
                    printf("get_screen_info: height=%d \n", height);
                    char ctmp[16] = "";
                    sprintf(ctmp, "%dx%d", width, height);
                    if(strlen(result) > 0)
                    {
                        strcat(result, ",");
                    }
                    //strcat(result, "{");
                    strcat(result, "\"");
                    strcat(result, tmp);
                    strcat(result, "\":\"");
                    strcat(result, ctmp);
                    strcat(result, "\"");
                    //strcat(result, "}");
                    printf("get_screen_info: result=%s \n", result);
                }
                p += strlen("fb");
            }

        }while(p);
    }
    ret = strlen(result);
    if(ret > 0)
    {
        strcpy(buf, result);
    }
    return ret;
}
int get_record_info(char *cmd, char *buf)
{
    int ret = 0;
    char result[2048] = "";
    char *p = NULL;
    char *p1 = NULL;
    char *p2 = NULL;
    char cmd2[1024] = "arecord -l | grep card";
    int read_size = 1024;
    ret = executeCMD(cmd2, buf, read_size);
    if(ret > 0)
    {
        p = buf;
        do{
            p = findstr(p, "card");
            if(p)
            {
                char tmp[16] = "";
                char tmp2[16] = "";
                //printf("get_record_info: p=%s \n", p);
                char *p2 = findstr(p, "\n");
                if(p2)
                {
                    int n = (int)((long long)p2 - (long long)p);
                    //printf("get_record_info: n=%d \n", n);
                    if(n > strlen("card"))
                    {
                        strncpy(tmp, p, n);
                    }
                    else{
                        strcmp(tmp, p);
                    }

                }
                else{
                    strcmp(tmp, p);
                }
                if(!strncmp(tmp, "card", strlen("card")))
                {
                    //printf("get_record_info: tmp=%s \n", tmp);
                    p2  = findstr(tmp, ":");
                    if(p2)
                    {
                        int n = (int)((long long)p2 - (long long)tmp);
                        memset(tmp2, 0, 16 * sizeof(char));
                        strncpy(tmp2, tmp, n);
                        //printf("get_record_info: tmp2=%s \n", tmp2);
                        p1 = p2;
                        p2  = findstr(p1, "device");
                        if(p2)
                        {
                            p1 = p2;
                            p2  = findstr(p2, ":");
                            if(p2)
                            {
                                n = (int)((long long)p2 - (long long)p1);
                                if(strlen(result) > 0)
                                {
                                    strcat(result, ",");
                                }
                                //strcat(result, "{");
                                strcat(result, "\"");
                                strcat(result, tmp2);
                                strcat(result, "\":\"");


                                memset(tmp2, 0, 16 * sizeof(char));
                                strncpy(tmp2, p1, n);
                                //printf("get_record_info: tmp2=%s \n", tmp2);
                                strcat(result, tmp2);
                                strcat(result, "\"");

                                //printf("get_record_info: result=%s \n", result);
                            }
                        }
                    }

                }
                p += strlen("card");
            }
        }while(p);
    }
    ret = strlen(result);
    if(ret > 0)
    {
        strcpy(buf, result);
    }
    return ret;
}
int restart_pulseaudio()
{
    int ret = 0;
    int read_size = 1024;
    char buf[10240] = "";
    char cmd[1024] = "ps -A|grep pulseaudio";
    ret = executeCMD(cmd, buf, read_size);

    if(ret > 0)
    {
        memset(buf, 0, read_size);
        strcpy(cmd, "pulseaudio -k");
        ret = executeCMD(cmd, buf, read_size);
        do
        {
            strcpy(cmd, "ps -A|grep pulseaudio");
            ret = executeCMD(cmd, buf, read_size);
            if(ret > 0)
            {
                memset(buf, 0, read_size);
                sleep(1);
            }
        }while(ret > 0);


    }
    strcpy(cmd, "pulseaudio --start");
    ret = executeCMD(cmd, buf, read_size);
    int count = 0;
    do{
        strcpy(cmd, "ps -A|grep pulseaudio");
        ret = executeCMD(cmd, buf, read_size);
        if(!ret)
        {
            sleep(1);
            count++;
        }
        if(count > 10)
        {
            printf("restart_pulseaudio fail ! \n");
        }
    }while(!ret);


    if(ret > 0)
    {
        printf("restart_pulseaudio ok \n");
        //sleep(1);
    }
    return ret;
}
int try_record(char *key)
{
    int ret = 0;
    int read_size = 1024;
    char buf[10240] = "";
    char *p = NULL;
    char cmd[1024] = "ps -A|grep pulseaudio";
#if 0
    ret = executeCMD(cmd, buf, read_size);

    if(ret > 0)
    {
#if 1
        strcpy(cmd, "pulseaudio -k");
        ret = executeCMD(cmd, buf, read_size);
#else
        char tmp[16] = "";
        p = findstr(buf, "?");
        if(p)
        {
            int n = (int)((long long)p - (long long)buf);
            strncpy(tmp, buf, n);
            int pid = atoi(tmp);
            printf("try_record: pid=%d \n", pid);
            strcpy(cmd, "kill -9 ");
            strcat(cmd, tmp);
            strcpy(buf, "");
            ret = executeCMD(cmd, buf, read_size);

        }
#endif

    }
    else{
        printf("try_record: no 'pulseaudio' run \n");
    }

    //restart pulse audio:
    //strcpy(cmd, "pulseaudio -k");
    //ret = executeCMD(cmd, buf, read_size);
    strcpy(cmd, "pulseaudio --start");
    ret = executeCMD(cmd, buf, read_size);
    strcpy(cmd, "ps -A|grep pulseaudio");
    ret = executeCMD(cmd, buf, read_size);
#else
    ret = restart_pulseaudio();
#endif
    if(ret > 0)
    {
#if 0
        char ch;
        printf("please enter any key \n");
        ch = getchar();
        putchar(ch);
        ch = getchar();
        putchar(ch);
#else
        //usleep(2000 * 1000);//100ms
        //sleep(1);
#endif
        strcpy(cmd, "../ffmpeg -y -f alsa -i hw:");
        strcat(cmd, key);
        strcat(cmd, " -t 3 out.wav");
        printf("try_record: cmd=%s \n", cmd);
        strcpy(buf, "");
        ret = executeCMD(cmd, buf, read_size);
        if(ret > 0)
        {
            //printf("try_record: buf=%s \n", buf);
        }
    }
    //printf("try_record: end \n");
    ret = 0;
    return ret;
}
int try_records(char *inbuf)
{
    int ret = 0;
    char *p = inbuf;
    char *p1 = NULL;
    char *p2 = NULL;

    do{
        p = findstr(p, "card");
        if(p)
        {
            char key[16] = "";
            p1 = findstr(p, " ");
            p2 = findstr(p, "\"");
            if(p1 && p2)
            {
                int n = (int)((long long)p2 - (long long)p1);
                strncpy(key, &p1[1], n - 1);
                p1 = findstr(p, "device");
                if(p1)
                {
                    p1 = findstr(p1, " ");
                    p2 = findstr(p1, "\"");
                    if(p1 && p2)
                    {
                        n = (int)((long long)p2 - (long long)p1);
                        char tmp[16] = "";
                        strncpy(tmp, &p1[1], n - 1);
                        strcat(key, ",");
                        strcat(key, tmp);
                        printf("try_records: key=%s \n", key);
                        //try_record(key);
                    }
                }

            }
            p += strlen("card");
        }
    }while(p);
    //printf("try_records: end \n");
    return ret;
}
int foreach_video(char *result)
{
    cJSON *json = mystr2json(result);
    cJSON *item = json;
    //cJSON *json2 = cJSON_CreateObject();
    char ret[1024] = "";
    do{
        if(!cJSON_IsNull(item))
        {
            int type = item->type;
            //printf("foreach_video: type=%d \n", type);
            char *key = item->string;

            if(key)
            {

                if(strlen(ret))
                {
                }
                printf("foreach_video: key=%s \n", key);
                if(cJSON_IsString(item))
                {
                    char *value = item->valuestring;
                    printf("foreach_video: value=%s \n", value);
                }

                cJSON *next = item;
                do{
                    next = next->next;
                    //printf("foreach_video: next=%x \n", next);
                    if(!cJSON_IsNull(next) && !cJSON_IsInvalid(next) && next)
                    {
                        int type = next->type;
                        //printf("foreach_video: type=%d \n", type);
                        char *key2 = next->string;
                        if(key2)
                        {

                            printf("foreach_video: key2=%s \n", key2);
                            if(cJSON_IsString(next))
                            {
                                char *value2 = next->valuestring;
                                printf("foreach_video: value2=%s \n", value2);
                            }
                            if(!strncmp(key, "video", strlen("video")))
                            {
                            }
                            else{
                            }
                        }

                    }
                }while(!cJSON_IsNull(next) && !cJSON_IsInvalid(next) && next);
            }
        }
        item = item->child;

    }while(!cJSON_IsNull(item) && item);

    deleteJson(json);
    //deleteJson(json2);

    return strlen(ret);
}
int foreach_record(char *result)
{
    cJSON *json = mystr2json(result);
    cJSON *item = json;
    char ret[1024] = "";
    do{
        if(!cJSON_IsNull(item))
        {
            int type = item->type;
            //printf("foreach_record: type=%d \n", type);
            char *key = item->string;

            if(key)
            {
                printf("foreach_record: key=%s \n", key);
                cJSON *next = item;
                do{
                    next = next->next;
                    //printf("foreach_record: next=%x \n", next);
                    if(!cJSON_IsNull(next) && !cJSON_IsInvalid(next) && next)
                    {
                        int type = next->type;
                        //printf("foreach_record: type=%d \n", type);
                        char *key = next->string;
                        printf("foreach_record: key=%s \n", key);
                    }
                }while(!cJSON_IsNull(next) && !cJSON_IsInvalid(next) && next);
            }
        }
        item = item->child;

    }while(!cJSON_IsNull(item) && item);
    deleteJson(json);
    return strlen(ret);
}
HCSVC_API
int api_get_dev_info(char *cmd, char *buf)
{
    int ret = 0;

    if(!strncmp(cmd, "select_video", strlen("select_video")))
    {
        char result[1024] = "";
        char result2[1024] = "";
        ret = get_video_info(cmd, result);
        ret += get_screen_info(cmd, result2);
        if(ret > 0)
        {
            strcat(buf, "{");
            if(strlen(result) > 0)
            {
                strcat(buf, result);
            }
            if(strlen(result2) > 0)
            {
                if(strlen(buf) > 0)
                {
                    strcat(buf, ",");
                }
                strcat(buf, result2);
            }
            strcat(buf, "}");
            ret = strlen(buf);

            if(!strcmp(cmd, "select_video_capture"))
            {
                foreach_video(buf);
            }
            else if(!strcmp(cmd, "select_video_params"))
            {
                foreach_video(buf);
            }

        }
    }
    else if(!strncmp(cmd, "select_audio", strlen("select_audio")))
    {
        char result[1024] = "";
        ret = get_record_info(cmd, result);
        if(ret > 0)
        {
            strcat(buf, "{");
            strcat(buf, result);
            strcat(buf, "}");
            ret = strlen(buf);
            try_records(result);
            foreach_record(buf);
            if(!strcmp(cmd, "select_audio_recorder"))
            {
            }
            else if(!strcmp(cmd, "select_audio_params"))
            {
            }
            else if(!strcmp(cmd, "select_audio_player"))
            {
            }
        }
    }
    //ret = 0;
    return ret;
}
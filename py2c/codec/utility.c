#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>

#ifdef linux
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <linux/videodev2.h>
#include <sys/time.h>
#include<signal.h>
#endif

#include "inc.h"

pthread_mutex_t * glob_lock = NULL;
static int gssrc = 0;
extern int GetvalueInt(cJSON *json, char *key);
extern int64_t get_sys_time();
extern int64_t get_sys_time2();

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

typedef struct CPU_PACKED         //定义一个cpu occupy的结构体
{
    char name[20];             //定义一个char类型的数组名name有20个元素
    unsigned int user;        //定义一个无符号的int类型的user
    unsigned int nice;        //定义一个无符号的int类型的nice
    unsigned int system;    //定义一个无符号的int类型的system
    unsigned int idle;         //定义一个无符号的int类型的idle
    unsigned int iowait;
    unsigned int irq;
    unsigned int softirq;
    int64_t time;
}CPU_OCCUPY;
typedef struct MEM_PACKED         //定义一个mem occupy的结构体
{
        char name[20];      //定义一个char类型的数组名name有20个元素
        unsigned long total;
        char name2[20];
}MEM_OCCUPY;

typedef struct MEM_PACK         //定义一个mem occupy的结构体
{
        double total,used_rate;
}MEM_PACK;

typedef struct DEV_MEM         //定义一个mem occupy的结构体
{
        double total,used_rate;
}DEV_MEM;

double cal_cpuoccupy (CPU_OCCUPY *o, CPU_OCCUPY *n)
{
    double od, nd;
    double id, sd;
    double cpu_use ;

    od = (double) (o->user + o->nice + o->system +o->idle+o->softirq+o->iowait+o->irq);//第一次(用户+优先级+系统+空闲)的时间再赋给od
    nd = (double) (n->user + n->nice + n->system +n->idle+n->softirq+n->iowait+n->irq);//第二次(用户+优先级+系统+空闲)的时间再赋给od

    id = (double) (n->idle);    //用户第一次和第二次的时间之差再赋给id
    sd = (double) (o->idle) ;    //系统第一次和第二次的时间之差再赋给sd
    if((nd-od) != 0)
    cpu_use =100.0- ((id-sd))/(nd-od)*100.00; //((用户+系统)乖100)除(第一次和第二次的时间差)再赋给g_cpu_used
    else cpu_use = 0;
    return cpu_use;
}

void get_cpuoccupy (CPU_OCCUPY *cpust)
{
    FILE *fd;
    int n;
    char buff[256];
    CPU_OCCUPY *cpu_occupy;
    cpu_occupy=cpust;

    fd = fopen ("/proc/stat", "r");
    fgets (buff, sizeof(buff), fd);

    sscanf (buff, "%s %u %u %u %u %u %u %u", cpu_occupy->name, &cpu_occupy->user, &cpu_occupy->nice,&cpu_occupy->system, &cpu_occupy->idle ,&cpu_occupy->iowait,&cpu_occupy->irq,&cpu_occupy->softirq);

    fclose(fd);
}

double getCpuRate()
{
    CPU_OCCUPY cpu_stat1;
    CPU_OCCUPY cpu_stat2;
    double cpu;
    get_cpuoccupy((CPU_OCCUPY *)&cpu_stat1);
    sleep(1);

    //第二次获取cpu使用情况
    get_cpuoccupy((CPU_OCCUPY *)&cpu_stat2);

    //计算cpu使用率
    cpu = cal_cpuoccupy ((CPU_OCCUPY *)&cpu_stat1, (CPU_OCCUPY *)&cpu_stat2);

    return cpu;
}
HCSVC_API
int api_getCpuRate(void **pcpu_stat, int cpurate, int threshold)
{
    int ret = -1;
    int64_t now_time = get_sys_time();
    if(pcpu_stat && (*pcpu_stat == NULL))
    {
        *pcpu_stat = (void *)calloc(1, sizeof(CPU_OCCUPY));
        if(*pcpu_stat)
        {
            get_cpuoccupy((CPU_OCCUPY *)(*pcpu_stat));
            ((CPU_OCCUPY *)(*pcpu_stat))->time = now_time;
        }
        return ret;
    }
    else{
        int64_t last_time = ((CPU_OCCUPY *)(*pcpu_stat))->time;
        int difftime = (int)(now_time - last_time);
        if(difftime > threshold)
        {
            CPU_OCCUPY cpu_stat2;
            cpu_stat2.time = now_time;
            get_cpuoccupy((CPU_OCCUPY *)&cpu_stat2);
            ret = (int)cal_cpuoccupy((CPU_OCCUPY *)(*pcpu_stat), (CPU_OCCUPY *)&cpu_stat2);
            if(cpurate > 0)
            {
                ret = (ret + cpurate) >> 1;
            }
            ((CPU_OCCUPY *)(*pcpu_stat))[0] = cpu_stat2;
        }
    }
    return ret;
}
MEM_PACK *get_memoccupy ()    // get RAM message
{
    FILE *fd;
    int n;
    double mem_total,mem_used_rate;;
    char buff[256];
    MEM_OCCUPY *m=(MEM_OCCUPY *)malloc(sizeof(MEM_OCCUPY));
    MEM_PACK *p=(MEM_PACK *)malloc(sizeof(MEM_PACK));
    fd = fopen ("/proc/meminfo", "r");

    fgets (buff, sizeof(buff), fd);
    sscanf (buff, "%s %lu %s\n", m->name, &m->total, m->name2);
    mem_total=m->total;
    fgets (buff, sizeof(buff), fd);
    sscanf (buff, "%s %lu %s\n", m->name, &m->total, m->name2);
    mem_used_rate=(1-m->total/mem_total)*100;
    mem_total=mem_total/(1024*1024);
    p->total=mem_total;
    p->used_rate=mem_used_rate;
    fclose(fd);     //关闭文件fd
    free(m);
    return p ;
}
DEV_MEM *get_devmem()        // get hard disk meeeage
{
        FILE * fp;
        int h=0;
        char buffer[80],a[80],d[80],e[80],f[80],buf[256];
        double c,b;
        fp=popen("df","r");
        fgets(buf,256,fp);
        double dev_total=0,dev_used=0;
        DEV_MEM  *dev=(DEV_MEM *)malloc(sizeof(DEV_MEM));
        while(6==fscanf(fp,"%s %lf %lf %s %s %s",a,&b,&c,d,e,f))
        {
                dev_total+=b;
                dev_used+=c;
        }
        dev->total=dev_total/1024/1024;;
        dev->used_rate=dev_used/dev_total*100;
        pclose(fp);
        return dev;
}
HCSVC_API
int api_get_cpu_info(char *outparam[])
{
    char *p = outparam[0];
    double cpurate = getCpuRate();
    MEM_PACK *mempack = get_memoccupy();
    DEV_MEM *devmem = get_devmem();
    int memsize = (int)(mempack->total);
    int memrate = (int)(mempack->used_rate);
    int devmemsize = (int)(devmem->total);
    int devmemrate = (int)(devmem->used_rate);
    int icpurate = (int)(cpurate);
    sprintf(p, "%d", icpurate);
    strcat(p, ";");
    sprintf(&p[strlen(p)], "%d", memrate);
    strcat(p, ";");
    sprintf(&p[strlen(p)], "%d", devmemrate);
    free(mempack);
    free(devmem);
    return 0;
}
HCSVC_API
int api_get_cpu_info2(int *icpurate, int *memrate, int *devmemrate)
{
    //if(!glob_lock)
    //{
    //    glob_lock = calloc(1, sizeof(pthread_mutex_t));
    //    pthread_mutex_init(glob_lock, NULL);
    //}
    //pthread_mutex_lock(glob_lock);
    double cpurate = getCpuRate();
#if 1
    MEM_PACK *mempack = get_memoccupy();
    DEV_MEM *devmem = get_devmem();
    int memsize = (int)(mempack->total);
    memrate[0] = (int)(mempack->used_rate);
    int devmemsize = (int)(devmem->total);
    devmemrate[0] = (int)(devmem->used_rate);
    icpurate[0] = (int)(cpurate);
    free(mempack);
    free(devmem);
#else
    icpurate[0] = (int)(cpurate);
#endif
    //pthread_mutex_unlock(glob_lock);
    return 0;
}
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
int api_clear_yuv420p(char *data, int w, int h)
{
    int size0 = w * h;
    int size1 = (w >> 1) * (h >> 1);
    char *srcY = (char *)data;
    char *srcU = (char *)&data[size0];
    char *srcV = (char *)&data[size0 + size1];
    memset(srcY, 0, size0);
    memset(srcU, 128, size1);
    memset(srcV, 128, size1);
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
HCSVC_API
int* api_array_alloc(int num)
{
    int *ret = calloc(1, sizeof(int) * num);
    return ret;
}
//代码转换:从一种编码转为另一种编码
int code_convert(char *from_charset,char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen)
{
    iconv_t cd;
    int rc;
    char **pin = &inbuf;
    char **pout = &outbuf;

    cd = iconv_open(to_charset,from_charset);
    if (cd==0) return -1;
    memset(outbuf,0,outlen);
    if (iconv(cd,pin,&inlen,pout,&outlen)==-1) return -1;
    iconv_close(cd);
    return 0;
}
//UNICODE码转为GB2312码
int u2g(char *inbuf,int inlen,char *outbuf,int outlen)
{
return code_convert("utf-8","gb2312",inbuf,inlen,outbuf,outlen);
}
//GB2312码转为UNICODE码
int g2u(char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
return code_convert("gb2312","utf-8",inbuf,inlen,outbuf,outlen);
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
#ifdef _WIN32______
        while (fgets(result, 255, fp) != NULL) {
            printf("executeCMD: %s", result);
        }
        ret = strlen(result);
#else
        char tmpbuf[4096];
         ret = fread(tmpbuf, 1, read_size, fp);
         if(ret > 0)
         {
            ///ret = g2u(tmpbuf, ret, result, 4096);
            memcpy(result, tmpbuf, ret);
         }
#endif
        printf("executeCMD: cmd=%s \n", cmd);
        printf("executeCMD: ret=%d \n", ret);
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
#ifdef linux
enum VideoType {
  kUnknown,
  kI420,
  kIYUV,
  kRGB24,
  kABGR,
  kARGB,
  kARGB4444,
  kRGB565,
  kARGB1555,
  kYUY2,
  kYV12,
  kUYVY,
  kMJPEG,
  kNV21,
  kNV12,
  kBGRA,
};
typedef struct{
  int32_t width;
  int32_t height;
  int32_t maxFPS;
  //VideoType videoType;
  int32_t videoType;
  bool interlaced;
}VideoCaptureCapability;

int32_t FillCapabilities(int fd, char *result) {
    int ret = 0;
    int count = 0;
    // set image format
    int totalFmts = 4;
    unsigned int videoFormats[] = {V4L2_PIX_FMT_MJPEG, V4L2_PIX_FMT_YUV420,
                                    V4L2_PIX_FMT_YUYV, V4L2_PIX_FMT_UYVY };
//  int sizes = 13;
//  unsigned int size[][2] = {{128, 96},   {160, 120},  {176, 144},  {320, 240},
//                            {352, 288},  {640, 480},  {704, 576},  {800, 600},
//                            {960, 720},  {1280, 720}, {1024, 768}, {1440, 1080},
//                            {1920, 1080}};
    struct v4l2_frmsizeenum frmsize;
    memset(&frmsize, 0, sizeof(frmsize));
    struct v4l2_frmivalenum frmival;
    memset(&frmival, 0, sizeof(frmival));

    struct v4l2_fmtdesc fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.index = 0;
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int count2 = 0;
    while (ioctl(fd, VIDIOC_ENUM_FMT, &fmt) == 0)
    {
        for (int fmts = 0; fmts < totalFmts; fmts++)
        {
            if(fmt.pixelformat == videoFormats[fmts])
            {
                memset(&frmsize, 0, sizeof(frmsize));
                frmsize.pixel_format = fmt.pixelformat;
                frmsize.index = 0;
                count = 0;
                while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0)
                {
                    if(frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
                    {
                        if(!count)
                        {
                            if(count2)
                            {
                                strcat(result, ",");
                            }
                            if(fmts)
                            {
                                strcat(result, "\"raw\":\"");
                            }
                            else{
                                strcat(result, "\"mjpeg\":\"");
                            }

                        }
                        VideoCaptureCapability cap;
                        cap.width = frmsize.discrete.width;
                        cap.height = frmsize.discrete.height;
                        if (fmt.pixelformat == V4L2_PIX_FMT_YUYV) {
                            cap.videoType = kYUY2;
                        } else if (fmt.pixelformat == V4L2_PIX_FMT_YUV420) {
                            cap.videoType = kI420;
                        } else if (fmt.pixelformat == V4L2_PIX_FMT_MJPEG) {
                            cap.videoType = kMJPEG;
                        } else if (fmt.pixelformat == V4L2_PIX_FMT_UYVY) {
                            cap.videoType = kUYVY;
                        }

                        memset(&frmival, 0, sizeof(frmival));
                        frmival.index = 0;
                        frmival.pixel_format = fmt.pixelformat;
                        frmival.width = frmsize.discrete.width;
                        frmival.height = frmsize.discrete.height;
                        if(ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) == 0)
                        {
                            if (frmival.type == V4L2_FRMIVAL_TYPE_DISCRETE)
                            {
                                if(frmival.discrete.numerator > 0 && frmival.discrete.denominator > 0)
                                {
                                    cap.maxFPS = frmival.discrete.denominator / frmival.discrete.numerator;
                                }
                            }
                        }
                        else{
                            // get fps of current camera mode
                            // V4l2 does not have a stable method of knowing so we just guess.
                            if (cap.width >= 800 && cap.videoType != kMJPEG) {
                                cap.maxFPS = 15;
                            } else {
                                cap.maxFPS = 30;
                            }
                        }
                        //_captureCapabilities.push_back(cap);
                        if(count)
                        {
                            strcat(result, " ");
                        }
                        char tmp[128] = "";
                        sprintf(tmp, "%d", cap.width);

                        strcat(result, tmp);
                        strcat(result, "x");
                        sprintf(tmp, "%d", cap.height);
                        strcat(result, tmp);
                        ///strcat(result, "x");
                        ///sprintf(tmp, "%d", cap.maxFPS);
                        ///strcat(result, tmp);
                        printf("FillCapabilities:cap.width=%d, cap.height=%d \n", cap.width, cap.height);
                        printf("FillCapabilities:cap.maxFPS=%d, cap.videoType=%d \n", cap.maxFPS, cap.videoType);
                        ret = cap.videoType;
                        count++;
                    }
                    frmsize.index++;
                }
                if(count)
                {
                    strcat(result, "\"");
                    count2++;
                }
            }
        }
        fmt.index++;
    }
    return ret;
}
int show_device_linux(int deviceNumber, char* deviceNameUTF8, char* deviceUniqueIdUTF8, char *result)
{
    int ret = 0;
    uint32_t deviceNameLength = 256;
    uint32_t deviceUniqueIdUTF8Length = 256;
    //uint32_t count = 0;
    char device[20];
    int fd = -1;
    sprintf(device, "/dev/video%d", deviceNumber);
    if ((fd = open(device, O_RDONLY)) != -1)
    {
        char tmp[16] = "";
        sprintf(tmp, "video%d", deviceNumber);
        if(strlen(result) > 0)
        {
            //strcat(result, ",");
        }
        else{
            //strcat(result, "{");
        }
        strcat(result, "\"");
        strcat(result, tmp);
        strcat(result, "\":{");
        printf("show_device_linux: device=%s \n", device);
    }
    else
    {
        close(fd);
        return -1;
    }
    struct v4l2_capability cap;
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0)
    {
        close(fd);
        return -2;
    }
    ret = FillCapabilities(fd, result);
    close(fd);
    if(ret <= 0)
    {
        return ret;
    }
    strcat(result, "}");
    char cameraName[64];
    memset(deviceNameUTF8, 0, deviceNameLength);
    memcpy(cameraName, cap.card, sizeof(cap.card));

    if (deviceNameLength >= strlen(cameraName)) {
        memcpy(deviceNameUTF8, cameraName, strlen(cameraName));
    } else {
        return -3;
    }

    if (cap.bus_info[0] != 0)  // may not available in all drivers
    {
        // copy device id
        if (deviceUniqueIdUTF8Length >= strlen((const char*)cap.bus_info))
        {
            memset(deviceUniqueIdUTF8, 0, deviceUniqueIdUTF8Length);
            memcpy(deviceUniqueIdUTF8, cap.bus_info,strlen((const char*)cap.bus_info));
        }
        else {
            return -4;
        }
    }
    else{
        return -5;
    }
    return ret;
}
#include <xcb/xcb.h>
//fb0是终端的SVGA FrameBuffer驱动程序。所以获得的是终端的数据。
//XCB是X-Window C Binding，是一个用于和X交互的C语言API。你应该安装libxcb的开发库和头文件。比如fedora要安装libxcb-devel这样的包。
//这样在gcc编译的时候指定--libs xcb就可以了
int get_screen_resolution2(int *width, int *height)
{
        /* Open the connection to the X server. Use the DISPLAY environment variable */

        int i, screenNum;
        xcb_connection_t *connection = xcb_connect (NULL, &screenNum);
        /* Get the screen whose number is screenNum */
        const xcb_setup_t *setup = xcb_get_setup (connection);
        xcb_screen_iterator_t iter = xcb_setup_roots_iterator (setup);
        // we want the screen at index screenNum of the iterator
        for (i = 0; i < screenNum; ++i) {
            xcb_screen_next (&iter);
        }
        xcb_screen_t *screen = iter.data;
        /* report */
        //printf ("\n");
        //printf ("Informations of screen %ld:\n", screen->root);
        //printf ("  width.........: %d\n", screen->width_in_pixels);
        //printf ("  height........: %d\n", screen->height_in_pixels);
        //printf ("  white pixel...: %ld\n", screen->white_pixel);
        //printf ("  black pixel...: %ld\n", screen->black_pixel);
        //printf ("\n");
        width[0] = screen->width_in_pixels;
        height[0] = screen->height_in_pixels;
        return 0;
}
int show_desktop_linux(int deviceNumber, int *width, int *height)
{
    int ret = 0;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    int screensize = 0;
    char device[20];
    int fd = -1;
    char cmd[256] = "nohup sudo chmod 0777 /dev/fb0";
    char outbuf[1024] = "";
    sprintf(cmd, "chmod 0777 /dev/fb%d", deviceNumber);
    ret = api_get_cmd(cmd, outbuf, 1024);
    sprintf(device, "/dev/fb%d", deviceNumber);
    if ((fd = open(device, O_RDONLY)) != -1)
    {
        printf("show_desktop_linux: device=%s \n", device);
    }
    else
    {
        close(fd);
        printf("show_desktop_linux: fail device=%s \n", device);
        get_screen_resolution2(width, height);
        return -1;
    }
    /*取得屏幕相关参数*/
    ioctl(fd, FBIOGET_FSCREENINFO, &finfo);
    ioctl(fd, FBIOGET_VSCREENINFO, &vinfo);
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

    //printf("get_screen_resolution: %d*%d\n",vinfo.xres, vinfo.yres);
    width[0] = vinfo.xres;
    height[0] = vinfo.yres;
    close(fd);
    printf("show_desktop_linux: device=%s, ret=%d \n", device, ret);
    return ret;
}
HCSVC_API
int api_list_device(char *buf)
//int api_list_device()
{
    int ret = 0;
    int width = 0;
    int height = 0;

    for (int n = 0; n < 20; n++)
    {
        char result[1024] = "";
        char deviceNameUTF8[256] = "";
        char deviceUniqueIdUTF8[256] = "";
        ret = show_device_linux(n, deviceNameUTF8, deviceUniqueIdUTF8, result);
        if(ret > 0)
        {
            printf("api_list_device:ret=%d \n", ret);
            printf("api_list_device:deviceNameUTF8=%s \n", deviceNameUTF8);
            printf("api_list_device:deviceUniqueIdUTF8=%s \n", deviceUniqueIdUTF8);
            //printf("api_list_device:result=%s \n", result);
            if(strlen(buf) > 0)
            {
                strcat(buf, ",");
            }
            if(!strlen(buf))
            {
                strcat(buf, "{");
            }
            strcat(buf, result);
        }
        //printf("api_list_device:result=%s \n", result);
    }
    for (int n = 0; n < 4; n++)
    {
        if(n < 4 && ((width | height) == 0))
        {
            char result[1024] = "";
            ret = show_desktop_linux(n, &width, &height);
            if(ret >= 0|| (width * height > 0))
            {
                strcat(result, ",\"fb0\":\"");
                char tmp[16];
                sprintf(tmp, "%d", width);
                strcat(result, tmp);
                strcat(result, "x");
                sprintf(tmp, "%d", height);
                strcat(result, tmp);
                strcat(result, "\"}");
                printf("api_list_device:width=%d, height=%d \n", width, height);
                strcat(buf, result);
            }
        }
    }
    printf("api_list_device:2: buf=%s \n", buf);
    ret = strlen(buf);
    return ret;
}
#endif
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
//枚举指定类型的所有采集设备的名称
//ENUMDEVICE_API HRESULT EnumDevice(CAPTURE_DEVICE_TYPE type, char * deviceList[], int nListLen, int & iNumCapDevices);
//ffmpeg -list_options true -f dshow -i video="Integrated Camera"
int get_av_info_win(char *buf, char *key0, char *key1)
{
    int ret = 0;
    char result[4096] = "";
    //char cmd2[4096] = "start /b ffmpeg -list_devices true -f dshow -i dummy";
    char cmd2[4096] = "start /b ffmpeg -list_devices true -f dshow -i dummy 2>&1";
    int read_size = 4096;
    char *p = NULL;
    char buf2[4096] = "";
    printf("get_av_info_win: cmd2=%s \n", cmd2);
    executeCMD("chcp 65001", buf2, 1024);// chcp 936
    memset(buf2, 0, read_size * sizeof(char));
    ret = executeCMD(cmd2, buf2, read_size);
    if(ret > 0)
    {
        printf("get_av_info_win: buf2=%s \n", buf2);
        int headsize = strlen("[dshow @ 031082a0]");
        char *p0 = findstr(buf2, key0);//"DirectShow video devices");
        char *p1 = findstr(buf2, key1);//"DirectShow audio devices");
        char *p2 = NULL;
        int idx = 0;
        int flag = 0;
        p = &p0[-headsize - 4];
        do{
            p = findstr(p, "[dshow @");
            if(p)
            {
                if(!idx)
                {
                    //printf("get_av_info_win: p=%s \n", p);
                }
                if(p2)
                {
                    int n = (int)((long long)p - (long long)p2);
                    //printf("get_av_info_win: n=%d \n", n);
                    if(n > 2){
                        if((idx & 1))
                        {
                            if(flag)
                            {
                                strcat(buf, ",");
                            }
                            strncpy(&buf[strlen(buf)], &p2[headsize + 2], n - headsize - 3);
                            //printf("get_av_info_win: buf=%s \n", buf);
                            flag += 1;
                        }
                        idx++;
                    }
                }
                p2 = p;
                p += headsize;
            }
            if((&p[headsize + 4] > p1) && p1 && (p1 > p0))
            {
                break;
            }
        }while(p);
        ///strcpy(buf, result);
    }
    ret = strlen(buf);
    return ret;
}
int get_video_info_win(char *buf)
{
    char *key0 = "DirectShow video devices";
    char *key1 = "DirectShow audio devices";
    char *tmp = "{\"windev\":{\"videodev\":[";
    strcpy(buf, tmp);
    int ret = get_av_info_win(buf, key0, key1);
    if(ret > 0)
    {
        char *tmp = "]}}";
        strcpy(&buf[strlen(buf)], tmp);
    }
    return ret;
}
int get_audio_info_win(char *buf)
{
    char *key0 = "DirectShow video devices";
    char *key1 = "DirectShow audio devices";
    char *tmp = "{\"windev\":{\"audiodev\":[";
    strcpy(buf, tmp);
    int ret = get_av_info_win(&buf[strlen(buf)], key1, key0);
    if(ret > 0)
    {
        char *tmp = "]}}";
        strcpy(&buf[strlen(buf)], tmp);
    }
    return ret;
}
//get info by ffmpeg
//ffmpeg -list_devices true -f dshow -i dummy
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
#ifdef linux
    //char *cmd = "nohup chmod 0777 /dev/fb0";
    char outbuf[1024] = "";
    ret = api_get_cmd(cmd, outbuf, 1024);
    int fd;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    int screensize = 0;
    fd = open("/dev/fb0",O_RDONLY);
    /*取得屏幕相关参数*/
    ioctl(fd, FBIOGET_FSCREENINFO, &finfo);
    ioctl(fd, FBIOGET_VSCREENINFO, &vinfo);
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

    //printf("get_screen_resolution: %d*%d\n",vinfo.xres, vinfo.yres);
    width[0] = vinfo.xres;
    height[0] = vinfo.yres;
    close(fd);
#endif
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
    //printf("get_record_info: result=%s \n", result);
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
#ifdef _WIN32
    printf("restart_pulseaudio: windows \n");
#else
    ret = executeCMD(cmd, buf, read_size);

    if(ret > 0)
    {
        memset(buf, 0, read_size);
        strcpy(cmd, "pulseaudio -k");
        ret = executeCMD(cmd, buf, read_size);
#if 0
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
#endif
    }

#if 0
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
#endif

    if(ret > 0)
    {
        printf("restart_pulseaudio ok \n");
        //sleep(1);
    }
#endif
    return ret;
}
int try_record(char *key)
{
    int ret = 0;
    int read_size = 1024;
    char buf[10240] = "";
    char *p = NULL;
    char cmd[1024] = "ps -A|grep pulseaudio";

    ret = restart_pulseaudio();

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
    cJSON *json = (cJSON *)api_str2json(result);
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

    api_json_free(json);
    //api_json_free(json2);

    return strlen(ret);
}
int foreach_record(char *result)
{
    cJSON *json = (cJSON *)api_str2json(result);
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
    api_json_free(json);
    return strlen(ret);
}
HCSVC_API
int api_get_dev_info(char *cmd, char *buf)
{
    int ret = 0;
    //printf("api_get_dev_info: cmd=%s \n", cmd);
    if(!strncmp(cmd, "select_video", strlen("select_video")))
    {
        char result[1024] = "";
        char result2[1024] = "";
#ifdef _WIN32
        ret = get_video_info_win(buf);
        if(ret > 0)
        {
            ret = strlen(buf);
        }
        return ret;
#elif defined(linux)
        ret = api_list_device(buf);
        if(ret > 0)
        {
            ret = strlen(buf);
        }
        return ret;
#endif
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
            printf("api_get_dev_info: buf=%s \n", buf);
        }

    }
    else if(!strncmp(cmd, "select_audio", strlen("select_audio")))
    {
        char result[1024] = "";
#ifdef _WIN32
        ret = get_audio_info_win(buf);
        if(ret > 0)
        {
            ret = strlen(buf);
        }
        return ret;
#endif
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
HCSVC_API
#if 1
unsigned int api_create_id(unsigned int range)
{
    int ret = 0;
    unsigned int seed = (unsigned int)get_sys_time2();
    srand(seed);
    ret = rand() % range;
    return ret;
}
#else
unsigned int api_create_id(unsigned int range)
{
    int ret = 0;
    if(!gssrc)
    {
        unsigned int seed = (unsigned int)get_sys_time();
        srand(seed);
        gssrc = rand() % range;
        gssrc++;
    }
    ret = gssrc;
    gssrc++;
    return ret;
}
#endif
/*
 * Copyright (c) 2020
 * Created by gxh_20200602
 */


#include "inc.h"

extern int GetvalueInt(cJSON *json, char *key);
extern FILE * global_logfp;
//Normal: I/P=5/1, P/B=3/1

float get_compress_factor(void *hnd)
{
    CodecObj *obj = (CodecObj *)hnd;
    cJSON *json = (cJSON *)obj->param;
    int width = GetvalueInt(json, "width");//352;
    int height = GetvalueInt(json, "height");//288;
    int qmin = GetvalueInt(json, "qmin");
    int qmax = GetvalueInt(json, "qmax");
    int bit_rate = GetvalueInt(json, "bit_rate");
    int fps = GetvalueInt(json, "frame_rate");
    int frame_size = (((width * height) * 3) >> 1);
    int bytes_per_frame = (bit_rate >> 3) / fps;
    float factor = (float)frame_size / (float)bytes_per_frame;
    //printf("get_I_qp: factor= %f \n", factor);

    return factor;
}
int get_I_qp(void *hnd)
{
    CodecObj *obj = (CodecObj *)hnd;
    cJSON *json = (cJSON *)obj->param;

    int qmin = GetvalueInt(json, "qmin");
    int qmax = GetvalueInt(json, "qmax");

    float factor = get_compress_factor(hnd);
    float base_factor = 100.0;
    int base_qp = 24;
    int qp_step = 6;
    //printf("get_I_qp: factor= %f \n", factor);
    int qp = base_qp + (int)((factor / base_factor - 1.0) * qp_step);

    //printf("get_I_qp: qp= %d \n", qp);
    qp = qp < qmin ? qmin : qp > qmax ? qmax : qp;

    return qp;
}
int rate_control16(void *hnd)
{
    CodecObj *obj = (CodecObj *)hnd;
    cJSON *json = (cJSON *)obj->param;
    int ret = -1;
    int width = GetvalueInt(json, "width");//352;
    int height = GetvalueInt(json, "height");//288;
    int frame_idx = GetvalueInt(json, "frame_idx");
    int ref_idx = GetvalueInt(json, "ref_idx");
    int gop_size = GetvalueInt(json, "gop_size");
    int refs = GetvalueInt(json, "refs");
    int bit_rate = GetvalueInt(json, "bit_rate");
    int fps = GetvalueInt(json, "frame_rate");
    int qmin = GetvalueInt(json, "qmin");
    int qmax = GetvalueInt(json, "qmax");
    int refs_all[5];//1,2,4,8,16
    int bytes_per_frame = (bit_rate >> 3) / fps;
    int bytes_P;//10%
    int bytes_I;//10%
    int bytes_FP0[8];//0,2,4,6,8,10,12,15
    int bytes_FP1[8];//1,3,5,7,9,11,13,14
    float k = (float)gop_size / (float)fps;
    int bytes_target_per_gop = (int)((bit_rate >> 3) * k);
    float k1 = 0.1;
    int bytes_target_I = (int)(bytes_target_per_gop * k1);
    float k2 = 0.1;
    int bytes_target_P = (int)(bytes_target_per_gop * k2);
    float k3 = (1.0 - (k1 + k2));
    int bytes_target_FP = (int)(bytes_target_per_gop * k3);
    float k4 = 4.0;
    int bytes_target_FP0 = (int)((float)bytes_target_FP * 3.0 / k4);
    int bytes_target_FP1 = (int)((float)bytes_target_FP * 1.0 / k4);
    int p_num = gop_size / refs;
    int fp0_num = (gop_size / refs) * 6;//此系數會影響碼率的控制精度；
    int fp1_num = gop_size - (1 + p_num + fp0_num);
    int factor = (int)get_compress_factor(hnd);//100
    int bytes_overflow = (((width * height) * 3) >> 1) / factor;//100;
    int bytes_overflow2 = bytes_overflow >> 1;

    //printf("rate_control16: fp0_num= %d \n", fp0_num);
    //printf("rate_control16: fp1_num= %d \n", fp1_num);

    //printf("rate_control16: bit_rate= %d kbps\n", (bit_rate / 1024));
    //printf("rate_control16: bytes_target_I= %d \n", bytes_target_I);
    //printf("rate_control16: bytes_target_P= %d \n", bytes_target_P);
    //printf("rate_control16: bytes_target_FP0= %d \n", bytes_target_FP0);
    //printf("rate_control16: bytes_target_FP1= %d \n", bytes_target_FP1);
    //printf("rate_control16: frame_idx= %d \n", frame_idx);
    //printf("rate_control16: ref_idx= %d \n", ref_idx);
    if (ref_idx == 0)
    {
        //I
        //printf("rate_control16: I \n");
        //printf("rate_control16: bytes_target_I= %d \n", bytes_target_I);

        //
        obj->brctrl.bytes_sum = 0;
        //obj->brctrl.base_qp = 26;
        int qp = get_I_qp(hnd);
        obj->brctrl.base_qp = qp + 3;
        obj->brctrl.qp = qp;
        //obj->brctrl.qp = obj->brctrl.base_qp;//26;//23;
        //printf("rate_control16: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
        //
        obj->brctrl.bytes_target_sum = bytes_target_I;
        obj->brctrl.bytes_target_last = bytes_target_I;
    }
    else if(ref_idx == refs)
    {
        //P
        //printf("rate_control16: P \n");
        //printf("rate_control16: bytes_target_P= %d \n", (bytes_target_P / p_num));
        //according to last info
        int diff = obj->brctrl.bytes_sum - obj->brctrl.bytes_target_sum;
        int diff2 = obj->brctrl.bytes_last - obj->brctrl.bytes_target_last;
        float k = 0.2;
        //int bytes_overflow = (int)((float)obj->brctrl.bytes_target_sum * k);
        //printf("rate_control16: 0: bytes_overflow= %d \n", bytes_overflow);
        //printf("rate_control16: 0: diff= %d \n", diff);
        //printf("rate_control16: 0: diff2= %d \n", diff2);
        if(global_logfp)
        {
            fprintf(global_logfp,"rate_control16: 0: bytes_overflow= %d \n", bytes_overflow);
            fprintf(global_logfp,"rate_control16: 0: diff= %d \n", diff);
            fflush(global_logfp);
        }
        //
        //if ((diff > bytes_overflow) && (diff2 > -bytes_overflow2))
        if((diff2 > bytes_overflow))
        {
            //
            obj->brctrl.base_qp += 1;
            //obj->brctrl.qp += 1;
        }
        else if ((diff < -bytes_overflow) && (diff2 < bytes_overflow2))
        {
            //
            obj->brctrl.base_qp -= 1;
            //obj->brctrl.qp -= 1;
        }
        int qp = obj->brctrl.base_qp;
        qp = qp < qmin ? qmin : qp > qmax ? qmax : qp;
        obj->brctrl.base_qp = qp;
        obj->brctrl.qp = obj->brctrl.base_qp;
        //
        obj->brctrl.bytes_target_last = (bytes_target_P / p_num);
        obj->brctrl.bytes_target_sum += (bytes_target_P / p_num);
        //printf("rate_control16: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
    }
    else if (((ref_idx & 1) == 0) || (ref_idx == (refs - 1)) || (ref_idx == 1))
    {
        //[1, 2,4,6,8,10,12,14,15]
        //printf("rate_control16: FP1 \n");
        //printf("rate_control16: bytes_target_FP1= %d \n", (bytes_target_FP1 / fp1_num));
        //
        int diff = obj->brctrl.bytes_sum - obj->brctrl.bytes_target_sum;
        int diff2 = obj->brctrl.bytes_last - obj->brctrl.bytes_target_last;
        float k = 0.2;
        //int bytes_overflow = (int)((float)obj->brctrl.bytes_target_sum * k);
        //printf("rate_control16: 1: bytes_overflow= %d \n", bytes_overflow);
        //printf("rate_control16: 1: diff= %d \n", diff);
        //printf("rate_control16: 1: diff2= %d \n", diff2);
        if(global_logfp)
        {
            fprintf(global_logfp,"rate_control16: 1: bytes_overflow= %d \n", bytes_overflow);
            fprintf(global_logfp,"rate_control16: 1: diff= %d \n", diff);
            fflush(global_logfp);
        }
        //
        //if ((diff > bytes_overflow) && (diff2 > -bytes_overflow2))
        if((diff2 > bytes_overflow))
        {
            //
            obj->brctrl.base_qp += 1;
            //obj->brctrl.qp += 1;
        }
        else if ((diff < -bytes_overflow) && (diff2 < bytes_overflow2))
        {
            //
            obj->brctrl.base_qp -= 1;
            //obj->brctrl.qp -= 1;
        }
        int qp = obj->brctrl.base_qp;
        qp = qp < qmin ? qmin : qp > qmax ? qmax : qp;
        obj->brctrl.base_qp = qp;
        obj->brctrl.qp = obj->brctrl.base_qp + 5;
        //obj->brctrl.qp = obj->brctrl.base_qp + 3;
        //obj->brctrl.qp = obj->brctrl.base_qp;
        //
        obj->brctrl.bytes_target_last = (bytes_target_FP1 / fp1_num);
        obj->brctrl.bytes_target_sum += (bytes_target_FP1 / fp1_num);
        //printf("rate_control16: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
    }
    else if ((ref_idx & 1) == 1)
    {
        //[3,5,7,9,11,13]
        //printf("rate_control16: FP0 \n");
        //printf("rate_control16: bytes_target_FP0= %d \n", (bytes_target_FP0 / fp0_num));
        //
        int diff = obj->brctrl.bytes_sum - obj->brctrl.bytes_target_sum;
        int diff2 = obj->brctrl.bytes_last - obj->brctrl.bytes_target_last;
        float k = 0.2;
        //int bytes_overflow = (int)((float)obj->brctrl.bytes_target_sum * k);
        //printf("rate_control16: 2: bytes_overflow= %d \n", bytes_overflow);
        //printf("rate_control16: 2: diff= %d \n", diff);
        //printf("rate_control16: 2: diff2= %d \n", diff2);
        if(global_logfp)
        {
            fprintf(global_logfp,"rate_control16: 2: bytes_overflow= %d \n", bytes_overflow);
            fprintf(global_logfp,"rate_control16: 2: diff= %d \n", diff);
            fflush(global_logfp);
        }
        //
        //if ((diff > bytes_overflow) && (diff2 > -bytes_overflow2))
        if((diff2 > bytes_overflow))
        {
            //
            obj->brctrl.base_qp += 1;
            //obj->brctrl.qp += 1;
        }
        else if ((diff < -bytes_overflow) && (diff2 < bytes_overflow2))
        {
            //
            obj->brctrl.base_qp -= 1;
            //obj->brctrl.qp -= 1;
        }
        int qp = obj->brctrl.base_qp;
        qp = qp < qmin ? qmin : qp > qmax ? qmax : qp;
        obj->brctrl.base_qp = qp;
        obj->brctrl.qp = obj->brctrl.base_qp + 2;
        //obj->brctrl.qp = obj->brctrl.base_qp;
        //
        obj->brctrl.bytes_target_last = (bytes_target_FP0 / fp0_num);
        obj->brctrl.bytes_target_sum += (bytes_target_FP0 / fp0_num);
        //printf("rate_control16: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
    }
    //obj->brctrl.qp = 26;//test
    ret = obj->brctrl.qp;
    ///printf("rate_control16: obj->brctrl.base_qp= %d \n", obj->brctrl.base_qp);
    ///printf("rate_control16: obj->brctrl.qp= %d \n", obj->brctrl.qp);
    if(global_logfp)
    {
        fprintf(global_logfp,"rate_control16: obj->brctrl.base_qp= %d \n", obj->brctrl.base_qp);
        fprintf(global_logfp,"rate_control16: obj->brctrl.qp= %d \n", obj->brctrl.qp);
        fflush(global_logfp);
    }
    return ret;
}
int rate_control8(void *hnd)
{
    CodecObj *obj = (CodecObj *)hnd;
    cJSON *json = (cJSON *)obj->param;
    int ret = -1;
    int width = GetvalueInt(json, "width");//352;
    int height = GetvalueInt(json, "height");//288;
    int frame_idx = GetvalueInt(json, "frame_idx");
    int ref_idx = GetvalueInt(json, "ref_idx");
    int gop_size = GetvalueInt(json, "gop_size");
    int refs = GetvalueInt(json, "refs");
    int bit_rate = GetvalueInt(json, "bit_rate");
    int fps = GetvalueInt(json, "frame_rate");
    int qmin = GetvalueInt(json, "qmin");
    int qmax = GetvalueInt(json, "qmax");
    int refs_all[5];//1,2,4,8,16
    int bytes_per_frame = (bit_rate >> 3) / fps;
    int bytes_P;//10%
    int bytes_I;//10%
    int bytes_FP0[8];//0,2,4,6,8//,10,12,15
    int bytes_FP1[8];//1,3,5,7,//9,11,13,14
    float k = (float)gop_size / (float)fps;
    int bytes_target_per_gop = (int)((bit_rate >> 3) * k);
    float k1 = 0.1;
    int bytes_target_I = (int)(bytes_target_per_gop * k1);
    float k2 = 0.15; //0.1;
    int bytes_target_P = (int)(bytes_target_per_gop * k2);
    float k3 = (1.0 - (k1 + k2));
    int bytes_target_FP = (int)(bytes_target_per_gop * k3);
    float k4 = 4.0;
    int bytes_target_FP0 = (int)((float)bytes_target_FP * 3.0 / k4);
    int bytes_target_FP1 = (int)((float)bytes_target_FP * 1.0 / k4);
    int p_num = gop_size / refs;
    int fp0_num = (gop_size / refs) * 2;//5;//6;
    int fp1_num = gop_size - (1 + p_num + fp0_num);

    int factor = (int)get_compress_factor(hnd);//100
    int bytes_overflow = (((width * height) * 3) >> 1) / factor;//100;
    //int bytes_overflow = (((width * height) * 3) >> 1) / 100;
    int bytes_overflow2 = bytes_overflow >> 1;

    //printf("rate_control8: fp0_num= %d \n", fp0_num);
    //printf("rate_control8: fp1_num= %d \n", fp1_num);

    //printf("rate_control8: bit_rate= %d kbps\n", (bit_rate / 1024));
    //printf("rate_control8: bytes_target_I= %d \n", bytes_target_I);
    //printf("rate_control8: bytes_target_P= %d \n", bytes_target_P);
    //printf("rate_control8: bytes_target_FP0= %d \n", bytes_target_FP0);
    //printf("rate_control8: bytes_target_FP1= %d \n", bytes_target_FP1);
    //printf("rate_control8: frame_idx= %d \n", frame_idx);
    //printf("rate_control8: ref_idx= %d \n", ref_idx);
    if (ref_idx == 0)
    {
        //I
        //printf("rate_control8: I \n");
        printf("rate_control8: bytes_target_I= %d \n", bytes_target_I);

        //
        obj->brctrl.bytes_sum = 0;
        //obj->brctrl.base_qp = 26;
        //obj->brctrl.qp = obj->brctrl.base_qp;//26;//23;
        int qp = get_I_qp(hnd);
        obj->brctrl.base_qp = qp + 3;
        obj->brctrl.qp = qp;
        //printf("rate_control8: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
        //
        obj->brctrl.bytes_target_sum = bytes_target_I;
        obj->brctrl.bytes_target_last = bytes_target_I;
    }
    else if(ref_idx == refs)
    {
        //P
        //printf("rate_control8: P \n");
        printf("rate_control8: bytes_target_P= %d \n", (bytes_target_P / p_num));
        //according to last info
        int diff = obj->brctrl.bytes_sum - obj->brctrl.bytes_target_sum;
        int diff2 = obj->brctrl.bytes_last - obj->brctrl.bytes_target_last;
        float k = 0.1;
        int bytes_overflow3 = (int)((float)obj->brctrl.bytes_target_sum * k);
        printf("rate_control8: bytes_overflow= %d \n", bytes_overflow);
        printf("rate_control8: diff= %d \n", diff);
        //
        //if ((diff > bytes_overflow) && (diff2 > -bytes_overflow2))
        //if((diff2 > bytes_overflow) || (diff > bytes_overflow3))
        if((diff2 > bytes_overflow))
        {
            //
            obj->brctrl.base_qp += 1;
            //obj->brctrl.qp += 1;
        }
        else if ((diff < -bytes_overflow) && (diff2 < bytes_overflow2))
        {
            //
            obj->brctrl.base_qp -= 1;
            //obj->brctrl.qp -= 1;
        }
        int qp = obj->brctrl.base_qp;
        qp = qp < qmin ? qmin : qp > qmax ? qmax : qp;
        obj->brctrl.base_qp = qp;
        obj->brctrl.qp = obj->brctrl.base_qp;
        //
        obj->brctrl.bytes_target_last = (bytes_target_P / p_num);
        obj->brctrl.bytes_target_sum += (bytes_target_P / p_num);
        //printf("rate_control8: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
    }
    else if (((ref_idx & 1) == 0) || (ref_idx == (refs - 1)) || (ref_idx == 1))
    {
        //[1, 2,4,6,8,10,12,14,15]
        //printf("rate_control8: FP1 \n");
        printf("rate_control8: bytes_target_FP1= %d \n", (bytes_target_FP1 / fp1_num));
        //
        int diff = obj->brctrl.bytes_sum - obj->brctrl.bytes_target_sum;
        int diff2 = obj->brctrl.bytes_last - obj->brctrl.bytes_target_last;
        float k = 0.1;
        int bytes_overflow3 = (int)((float)obj->brctrl.bytes_target_sum * k);
        printf("rate_control8: bytes_overflow= %d \n", bytes_overflow);
        printf("rate_control8: diff= %d \n", diff);
        //
        //if ((diff > bytes_overflow) && (diff2 > -bytes_overflow2))
        //if((diff2 > bytes_overflow) || (diff > bytes_overflow3))
        if((diff2 > bytes_overflow))
        {
            //
            obj->brctrl.base_qp += 1;
            //obj->brctrl.qp += 1;
        }
        else if ((diff < -bytes_overflow) && (diff2 < bytes_overflow2))
        {
            //
            obj->brctrl.base_qp -= 1;
            //obj->brctrl.qp -= 1;
        }
        int qp = obj->brctrl.base_qp;
        qp = qp < qmin ? qmin : qp > qmax ? qmax : qp;
        obj->brctrl.base_qp = qp;
        obj->brctrl.qp = obj->brctrl.base_qp + 5;
        //
        obj->brctrl.bytes_target_last = (bytes_target_FP1 / fp1_num);
        obj->brctrl.bytes_target_sum += (bytes_target_FP1 / fp1_num);
        //printf("rate_control8: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
    }
    else if ((ref_idx & 1) == 1)
    {
        //[3,5,7,9,11,13]
        //printf("rate_control8: FP0 \n");
        printf("rate_control8: bytes_target_FP0= %d \n", (bytes_target_FP0 / fp0_num));
        //
        int diff = obj->brctrl.bytes_sum - obj->brctrl.bytes_target_sum;
        int diff2 = obj->brctrl.bytes_last - obj->brctrl.bytes_target_last;
        float k = 0.1;
        int bytes_overflow3 = (int)((float)obj->brctrl.bytes_target_sum * k);
        printf("rate_control8: bytes_overflow= %d \n", bytes_overflow);
        printf("rate_control8: diff= %d \n", diff);
        //
        //if ((diff > bytes_overflow) && (diff2 > -bytes_overflow2))
        //if((diff2 > bytes_overflow) || (diff > bytes_overflow3))
        if((diff2 > bytes_overflow))
        {
            //
            obj->brctrl.base_qp += 1;
            //obj->brctrl.qp += 1;
        }
        else if ((diff < -bytes_overflow) && (diff2 < bytes_overflow2))
        {
            //
            obj->brctrl.base_qp -= 1;
            //obj->brctrl.qp -= 1;
        }
        int qp = obj->brctrl.base_qp;
        qp = qp < qmin ? qmin : qp > qmax ? qmax : qp;
        obj->brctrl.base_qp = qp;
        obj->brctrl.qp = obj->brctrl.base_qp + 2;
        //
        obj->brctrl.bytes_target_last = (bytes_target_FP0 / fp0_num);
        obj->brctrl.bytes_target_sum += (bytes_target_FP0 / fp0_num);
        //printf("rate_control8: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
    }
    //obj->brctrl.qp = 26;//test
    ret = obj->brctrl.qp;
    printf("rate_control8: obj->brctrl.base_qp= %d \n", obj->brctrl.base_qp);
    printf("rate_control8: obj->brctrl.qp= %d \n", obj->brctrl.qp);
    return ret;
}
int rate_control4(void *hnd)
{
    CodecObj *obj = (CodecObj *)hnd;
    cJSON *json = (cJSON *)obj->param;
    int ret = -1;
    int width = GetvalueInt(json, "width");//352;
    int height = GetvalueInt(json, "height");//288;
    int frame_idx = GetvalueInt(json, "frame_idx");
    int ref_idx = GetvalueInt(json, "ref_idx");
    int gop_size = GetvalueInt(json, "gop_size");
    int refs = GetvalueInt(json, "refs");
    int bit_rate = GetvalueInt(json, "bit_rate");
    int fps = GetvalueInt(json, "frame_rate");
    int qmin = GetvalueInt(json, "qmin");
    int qmax = GetvalueInt(json, "qmax");
    int refs_all[5];//1,2,4,8,16
    int bytes_per_frame = (bit_rate >> 3) / fps;
    int bytes_P;//10%
    int bytes_I;//10%
    int bytes_FP0[8];//0,2,4,6,8,10,12,15
    int bytes_FP1[8];//1,3,5,7,9,11,13,14
    float k = (float)gop_size / (float)fps;
    int bytes_target_per_gop = (int)((bit_rate >> 3) * k);
    float k1 = 0.1;
    int bytes_target_I = (int)(bytes_target_per_gop * k1);
    float k2 = 0.2;
    int bytes_target_P = (int)(bytes_target_per_gop * k2);
    float k3 = (1.0 - (k1 + k2));
    int bytes_target_FP = (int)(bytes_target_per_gop * k3);
    float k4 = 4.0;
    int bytes_target_FP0 = (int)((float)bytes_target_FP * 3.0 / k4);
    int bytes_target_FP1 = (int)((float)bytes_target_FP * 1.0 / k4);
    int p_num = gop_size / refs;
    int fp0_num = (gop_size / refs) * 1;
    int fp1_num = gop_size - (1 + p_num + fp0_num);
    int factor = (int)get_compress_factor(hnd);//100
    int bytes_overflow = (((width * height) * 3) >> 1) / factor;//100;
    //int bytes_overflow = (((width * height) * 3) >> 1) / 100;
    int bytes_overflow2 = bytes_overflow >> 1;

    //printf("rate_control4: fp0_num= %d \n", fp0_num);
    //printf("rate_control4: fp1_num= %d \n", fp1_num);

    //printf("rate_control4: bit_rate= %d kbps\n", (bit_rate / 1024));
    //printf("rate_control4: bytes_target_I= %d \n", bytes_target_I);
    //printf("rate_control4: bytes_target_P= %d \n", bytes_target_P);
    //printf("rate_control4: bytes_target_FP0= %d \n", bytes_target_FP0);
    //printf("rate_control4: bytes_target_FP1= %d \n", bytes_target_FP1);
    //printf("rate_control4: frame_idx= %d \n", frame_idx);
    //printf("rate_control4: ref_idx= %d \n", ref_idx);
    if (ref_idx == 0)
    {
        //I
        //printf("rate_control4: I \n");
        //printf("rate_control4: bytes_target_I= %d \n", bytes_target_I);

        //
        obj->brctrl.bytes_sum = 0;
        //obj->brctrl.base_qp = 26;
        //obj->brctrl.qp = obj->brctrl.base_qp;//26;//23;
        int qp = get_I_qp(hnd);
        obj->brctrl.base_qp = qp + 3;
        obj->brctrl.qp = qp;
        //printf("rate_control4: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
        //
        obj->brctrl.bytes_target_sum = bytes_target_I;
        obj->brctrl.bytes_target_last = bytes_target_I;
    }
    else if(ref_idx == refs)
    {
        //P
        //printf("rate_control4: P \n");
        //printf("rate_control4: bytes_target_P= %d \n", (bytes_target_P / p_num));
        //according to last info
        int diff = obj->brctrl.bytes_sum - obj->brctrl.bytes_target_sum;
        int diff2 = obj->brctrl.bytes_last - obj->brctrl.bytes_target_last;
        float k = 0.2;
        //int bytes_overflow = (int)((float)obj->brctrl.bytes_target_sum * k);
        //printf("rate_control4: bytes_overflow= %d \n", bytes_overflow);
        //printf("rate_control4: diff= %d \n", diff);
        if(global_logfp)
        {
            fprintf(global_logfp,"rate_control4: 0: bytes_overflow= %d \n", bytes_overflow);
            fprintf(global_logfp,"rate_control4: 0: diff= %d \n", diff);
            fflush(global_logfp);
        }
        //
        //if ((diff > bytes_overflow) && (diff2 > -bytes_overflow2))
        if((diff2 > bytes_overflow))
        {
            //
            obj->brctrl.base_qp += 1;
            //obj->brctrl.qp += 1;
        }
        else if ((diff < -bytes_overflow) && (diff2 < bytes_overflow2))
        {
            //
            obj->brctrl.base_qp -= 1;
            //obj->brctrl.qp -= 1;
        }
        int qp = obj->brctrl.base_qp;
        qp = qp < qmin ? qmin : qp > qmax ? qmax : qp;
        obj->brctrl.base_qp = qp;
        obj->brctrl.qp = obj->brctrl.base_qp;
        //
        obj->brctrl.bytes_target_last = (bytes_target_P / p_num);
        obj->brctrl.bytes_target_sum += (bytes_target_P / p_num);
        //printf("rate_control4: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
    }
    else if (((ref_idx & 1) == 0) || ((ref_idx == (refs - 1)) && (refs > 4)) || (ref_idx == 1))
    {
        //[1, 2,4,6,8,10,12,14,15]
        //printf("rate_control4: FP1 \n");
        //printf("rate_control4: bytes_target_FP1= %d \n", (bytes_target_FP1 / fp1_num));
        //
        int diff = obj->brctrl.bytes_sum - obj->brctrl.bytes_target_sum;
        int diff2 = obj->brctrl.bytes_last - obj->brctrl.bytes_target_last;
        float k = 0.2;
        //int bytes_overflow = (int)((float)obj->brctrl.bytes_target_sum * k);
        //printf("rate_control4: bytes_overflow= %d \n", bytes_overflow);
        //printf("rate_control4: diff= %d \n", diff);
        if(global_logfp)
        {
            fprintf(global_logfp,"rate_control4: 1: bytes_overflow= %d \n", bytes_overflow);
            fprintf(global_logfp,"rate_control4: 1: diff= %d \n", diff);
            fflush(global_logfp);
        }
        //
        //if ((diff > bytes_overflow) && (diff2 > -bytes_overflow2))
        if((diff2 > bytes_overflow))
        {
            //
            obj->brctrl.base_qp += 1;
            //obj->brctrl.qp += 1;
        }
        else if ((diff < -bytes_overflow) && (diff2 < bytes_overflow2))
        {
            //
            obj->brctrl.base_qp -= 1;
            //obj->brctrl.qp -= 1;
        }
        int qp = obj->brctrl.base_qp;
        qp = qp < qmin ? qmin : qp > qmax ? qmax : qp;
        obj->brctrl.base_qp = qp;
        obj->brctrl.qp = obj->brctrl.base_qp + 5;
        //
        obj->brctrl.bytes_target_last = (bytes_target_FP1 / fp1_num);
        obj->brctrl.bytes_target_sum += (bytes_target_FP1 / fp1_num);
        //printf("rate_control4: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
    }
    else if ((ref_idx & 1) == 1)
    {
        //[3,5,7,9,11,13]
        //printf("rate_control4: FP0 \n");
        //printf("rate_control4: bytes_target_FP0= %d \n", (bytes_target_FP0 / fp0_num));
        //
        int diff = obj->brctrl.bytes_sum - obj->brctrl.bytes_target_sum;
        int diff2 = obj->brctrl.bytes_last - obj->brctrl.bytes_target_last;
        float k = 0.2;
        //int bytes_overflow = (int)((float)obj->brctrl.bytes_target_sum * k);
        //printf("rate_control4: bytes_overflow= %d \n", bytes_overflow);
        //printf("rate_control4: diff= %d \n", diff);
        if(global_logfp)
        {
            fprintf(global_logfp,"rate_control4: 2: bytes_overflow= %d \n", bytes_overflow);
            fprintf(global_logfp,"rate_control4: 2: diff= %d \n", diff);
            fflush(global_logfp);
        }
        //
        //if ((diff > bytes_overflow) && (diff2 > -bytes_overflow2))
        if((diff2 > bytes_overflow))
        {
            //
            obj->brctrl.base_qp += 1;
            //obj->brctrl.qp += 1;
        }
        else if ((diff < -bytes_overflow) && (diff2 < bytes_overflow2))
        {
            //
            obj->brctrl.base_qp -= 1;
            //obj->brctrl.qp -= 1;
        }
        int qp = obj->brctrl.base_qp;
        qp = qp < qmin ? qmin : qp > qmax ? qmax : qp;
        obj->brctrl.base_qp = qp;
        obj->brctrl.qp = obj->brctrl.base_qp + 2;
        //
        obj->brctrl.bytes_target_last = (bytes_target_FP0 / fp0_num);
        obj->brctrl.bytes_target_sum += (bytes_target_FP0 / fp0_num);
        //printf("rate_control4: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
    }
    //obj->brctrl.qp = 26;//test
    ret = obj->brctrl.qp;
    //printf("rate_control4: obj->brctrl.base_qp= %d \n", obj->brctrl.base_qp);
    //printf("rate_control4: obj->brctrl.qp= %d \n", obj->brctrl.qp);
    if(global_logfp)
    {
        fprintf(global_logfp,"rate_control4: obj->brctrl.base_qp= %d \n", obj->brctrl.base_qp);
        fprintf(global_logfp,"rate_control4: obj->brctrl.qp= %d \n", obj->brctrl.qp);
        fflush(global_logfp);
    }
    return ret;
}
int rate_control2(void *hnd)
{
    CodecObj *obj = (CodecObj *)hnd;
    cJSON *json = (cJSON *)obj->param;
    int ret = -1;
    int width = GetvalueInt(json, "width");//352;
    int height = GetvalueInt(json, "height");//288;
    int frame_idx = GetvalueInt(json, "frame_idx");
    int ref_idx = GetvalueInt(json, "ref_idx");
    int gop_size = GetvalueInt(json, "gop_size");
    int refs = GetvalueInt(json, "refs");
    int bit_rate = GetvalueInt(json, "bit_rate");
    int fps = GetvalueInt(json, "frame_rate");
    int qmin = GetvalueInt(json, "qmin");
    int qmax = GetvalueInt(json, "qmax");
    int refs_all[5];//1,2,4,8,16
    int bytes_per_frame = (bit_rate >> 3) / fps;
    int bytes_P;//10%
    int bytes_I;//10%
    int bytes_FP0[8];//0,2,4,6,8,10,12,15
    int bytes_FP1[8];//1,3,5,7,9,11,13,14
    float k = (float)gop_size / (float)fps;
    int bytes_target_per_gop = (int)((bit_rate >> 3) * k);
    float k1 = 0.1;
    int bytes_target_I = (int)(bytes_target_per_gop * k1);
    float k2 = 0.25;
    int bytes_target_P = (int)(bytes_target_per_gop * k2);
    float k3 = (1.0 - (k1 + k2));
    int bytes_target_FP = (int)(bytes_target_per_gop * k3);
    float k4 = 4.0;
    int bytes_target_FP0 = bytes_target_FP;//(int)((float)bytes_target_FP * 3.0 / k4);
    int bytes_target_FP1 = (int)((float)bytes_target_FP * 1.0 / k4);
    int p_num = gop_size / refs;
    int fp0_num = (gop_size / refs) * 1;
    int fp1_num = gop_size - (1 + p_num + fp0_num);

    int factor = (int)get_compress_factor(hnd);//100
    int bytes_overflow = (((width * height) * 3) >> 1) / factor;//100;
    //int bytes_overflow = (((width * height) * 3) >> 1) / 100;
    int bytes_overflow2 = bytes_overflow >> 1;

    //printf("rate_control2: fp0_num= %d \n", fp0_num);
    //printf("rate_control2: fp1_num= %d \n", fp1_num);

    //printf("rate_control2: bit_rate= %d kbps\n", (bit_rate / 1024));
    //printf("rate_control2: bytes_target_I= %d \n", bytes_target_I);
    //printf("rate_control2: bytes_target_P= %d \n", bytes_target_P);
    //printf("rate_control2: bytes_target_FP0= %d \n", bytes_target_FP0);
    //printf("rate_control2: bytes_target_FP1= %d \n", bytes_target_FP1);
    //printf("rate_control2: frame_idx= %d \n", frame_idx);
    //printf("rate_control2: ref_idx= %d \n", ref_idx);
    if (ref_idx == 0)
    {
        //I
        //printf("rate_control2: I \n");
        //printf("rate_control2: bytes_target_I= %d \n", bytes_target_I);

        //
        obj->brctrl.bytes_sum = 0;
        //obj->brctrl.base_qp = 26;
        //obj->brctrl.qp = obj->brctrl.base_qp;//26;//23;
        int qp = get_I_qp(hnd);
        obj->brctrl.base_qp = qp + 3;
        obj->brctrl.qp = qp;
        //printf("rate_control2: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
        //
        obj->brctrl.bytes_target_sum = bytes_target_I;
        obj->brctrl.bytes_target_last = bytes_target_I;
    }
    else if(ref_idx == refs)
    {
        //P
        //printf("rate_control2: P \n");
        //printf("rate_control2: bytes_target_P= %d \n", (bytes_target_P / p_num));
        //according to last info
        int diff = obj->brctrl.bytes_sum - obj->brctrl.bytes_target_sum;
        int diff2 = obj->brctrl.bytes_last - obj->brctrl.bytes_target_last;
        float k = 0.2;
        //int bytes_overflow = (int)((float)obj->brctrl.bytes_target_sum * k);
        //printf("rate_control2: bytes_overflow= %d \n", bytes_overflow);
        //printf("rate_control2: diff= %d \n", diff);
        //
        //if ((diff > bytes_overflow) && (diff2 > -bytes_overflow2))
        if((diff2 > bytes_overflow))
        {
            //
            obj->brctrl.base_qp += 1;
            //obj->brctrl.qp += 1;
        }
        else if ((diff < -bytes_overflow) && (diff2 < bytes_overflow2))
        {
            //
            obj->brctrl.base_qp -= 1;
            //obj->brctrl.qp -= 1;
        }
        int qp = obj->brctrl.base_qp;
        qp = qp < qmin ? qmin : qp > qmax ? qmax : qp;
        obj->brctrl.base_qp = qp;
        obj->brctrl.qp = obj->brctrl.base_qp;
        //
        obj->brctrl.bytes_target_last = (bytes_target_P / p_num);
        obj->brctrl.bytes_target_sum += (bytes_target_P / p_num);
        //printf("rate_control2: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
    }

    else if ((ref_idx & 1) == 1)
    {
        //[3,5,7,9,11,13]
        //printf("rate_control2: FP0 \n");
        //printf("rate_control2: bytes_target_FP0= %d \n", (bytes_target_FP0 / fp0_num));
        //
        int diff = obj->brctrl.bytes_sum - obj->brctrl.bytes_target_sum;
        int diff2 = obj->brctrl.bytes_last - obj->brctrl.bytes_target_last;
        float k = 0.2;
        //int bytes_overflow = (int)((float)obj->brctrl.bytes_target_sum * k);
        //printf("rate_control2: bytes_overflow= %d \n", bytes_overflow);
        //printf("rate_control2: diff= %d \n", diff);
        //
        //if ((diff > bytes_overflow) && (diff2 > -bytes_overflow2))
        if((diff2 > bytes_overflow))
        {
            //
            obj->brctrl.base_qp += 1;
            //obj->brctrl.qp += 1;
        }
        else if ((diff < -bytes_overflow) && (diff2 < bytes_overflow2))
        {
            //
            obj->brctrl.base_qp -= 1;
            //obj->brctrl.qp -= 1;
        }
        int qp = obj->brctrl.base_qp;
        qp = qp < qmin ? qmin : qp > qmax ? qmax : qp;
        obj->brctrl.base_qp = qp;
        obj->brctrl.qp = obj->brctrl.base_qp + 2;
        //
        obj->brctrl.bytes_target_last = (bytes_target_FP0 / fp0_num);
        obj->brctrl.bytes_target_sum += (bytes_target_FP0 / fp0_num);
        //printf("rate_control2: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
    }
    //obj->brctrl.qp = 26;//test
    ret = obj->brctrl.qp;
    //printf("rate_control2: obj->brctrl.base_qp= %d \n", obj->brctrl.base_qp);
    //printf("rate_control2: obj->brctrl.qp= %d \n", obj->brctrl.qp);
    return ret;
}
int rate_control1(void *hnd)
{
    CodecObj *obj = (CodecObj *)hnd;
    cJSON *json = (cJSON *)obj->param;
    int ret = -1;
    int width = GetvalueInt(json, "width");//352;
    int height = GetvalueInt(json, "height");//288;
    int frame_idx = GetvalueInt(json, "frame_idx");
    //int pict_type = GetvalueInt(json, "pict_type");
    int gop_size = GetvalueInt(json, "gop_size");
    int refs = GetvalueInt(json, "refs");
    int bit_rate = GetvalueInt(json, "bit_rate");
    int fps = GetvalueInt(json, "frame_rate");
    int qmin = GetvalueInt(json, "qmin");
    int qmax = GetvalueInt(json, "qmax");
    int max_b_frames = GetvalueInt(json, "max_b_frames");
    int refs_all[5];//1,2,4,8,16
    int bytes_per_frame = (bit_rate >> 3) / fps;
    int bytes_P;//10%
    int bytes_I;//10%
    int bytes_FP0[8];//0,2,4,6,8,10,12,15
    int bytes_FP1[8];//1,3,5,7,9,11,13,14
    float k = (float)gop_size / (float)fps;
    int bytes_target_per_gop = (int)((bit_rate >> 3) * k);
    float k1 = 0.1;
    int bytes_target_I = (int)(bytes_target_per_gop * k1);
    //P/B = 3/1
    //x + max_b_frames * x / 3 = 0.9
    //(1 + max_b_frames / 3) * x = 0.9
    float k2 = 0.9;//0.9 / (1.0 + max_b_frames / 3.0);
    int bytes_target_P = (int)(bytes_target_per_gop * k2);
    float k3 = (1.0 - (k1 + k2));
    int bytes_target_B = (int)(bytes_target_per_gop * k3);
    int p_num = gop_size / refs - 1;
    int b_num = 0;
    int bytes_overflow = (((width * height) * 3) >> 1) / 100;
    int bytes_overflow2 = bytes_overflow >> 1;
    int pict_type = 0;
    if(max_b_frames && false)
    {
        k2 = 0.9 / (1.0 + max_b_frames / 3.0);
        bytes_target_P = (int)(bytes_target_per_gop * k2);
        k3 = (1.0 - (k1 + k2));
        bytes_target_B = (int)(bytes_target_per_gop * k3);
        pict_type = GetvalueInt(json, "pict_type");
        p_num = p_num / (max_b_frames + 1) + 1;
        b_num = gop_size - 1 - p_num;
        //return ret;
    }

    ///printf("rate_control1: bit_rate= %d kbps\n", (bit_rate / 1024));
    //printf("rate_control1: bytes_target_I= %d \n", bytes_target_I);
    //printf("rate_control1: bytes_target_P= %d \n", bytes_target_P);
    //printf("rate_control1: frame_idx= %d \n", frame_idx);
    //printf("rate_control1: ref_idx= %d \n", ref_idx);
    if (pict_type == 0 && frame_idx == 0)
    {
        //I
        //printf("rate_control1: I \n");
        //printf("rate_control1: bytes_target_I= %d \n", bytes_target_I);

        //
        obj->brctrl.bytes_sum = 0;
        //obj->brctrl.base_qp = 26;
        int qp = get_I_qp(hnd);
        obj->brctrl.base_qp = qp + 3;
        obj->brctrl.qp = qp;
        //printf("rate_control1: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
        //
        obj->brctrl.bytes_target_sum = bytes_target_I;
        obj->brctrl.bytes_target_last = bytes_target_I;
    }
    else //if(ref_idx == refs)
    {
        //B
        if((pict_type == 3) && false)
        {
            int diff = obj->brctrl.bytes_sum - obj->brctrl.bytes_target_sum;
            int diff2 = obj->brctrl.bytes_last - obj->brctrl.bytes_target_last;
            //float k = 0.2;
            //int bytes_overflow = (int)((float)obj->brctrl.bytes_target_sum * k);
            printf("rate_control2: bytes_overflow= %d \n", bytes_overflow);
            printf("rate_control2: diff= %d \n", diff);
            //
            //if ((diff > bytes_overflow) && (diff2 > -bytes_overflow2))
            if((diff2 > bytes_overflow))
            {
                //
                obj->brctrl.base_qp += 1;
                //obj->brctrl.qp += 1;
            }
            else if ((diff < -bytes_overflow) && (diff2 < bytes_overflow2))
            {
                //
                obj->brctrl.base_qp -= 1;
                //obj->brctrl.qp -= 1;
            }
            int qp = obj->brctrl.base_qp;
            qp = qp < qmin ? qmin : qp > qmax ? qmax : qp;
            obj->brctrl.base_qp = qp;
            obj->brctrl.qp = obj->brctrl.base_qp + 2;
            //
            obj->brctrl.bytes_target_last = (bytes_target_B / b_num);
            obj->brctrl.bytes_target_sum += (bytes_target_B / b_num);
        }
        else{
            //P
            //printf("rate_control1: P \n");
            //printf("rate_control1: bytes_target_P= %d \n", (bytes_target_P / p_num));
            //according to last info
            int diff = obj->brctrl.bytes_sum - obj->brctrl.bytes_target_sum;
            int diff2 = obj->brctrl.bytes_last - obj->brctrl.bytes_target_last;
            float k = 0.1;
            int bytes_overflow3 = (int)((float)obj->brctrl.bytes_target_sum * k);
            //printf("rate_control1: bytes_overflow= %d \n", bytes_overflow);
            //printf("rate_control1: diff= %d \n", diff);
            //
            //if ((diff > bytes_overflow) && (diff2 > -bytes_overflow2))
            //if((diff2 > bytes_overflow))
            if((diff2 > bytes_overflow) || (diff > bytes_overflow3))
            {
                //
                obj->brctrl.base_qp += 1;
                //obj->brctrl.qp += 1;
            }
            else if ((diff < -bytes_overflow) && (diff2 < bytes_overflow2))
            {
                //
                obj->brctrl.base_qp -= 1;
                //obj->brctrl.qp -= 1;
            }
            int qp = obj->brctrl.base_qp;
            qp = qp < qmin ? qmin : qp > qmax ? qmax : qp;
            obj->brctrl.base_qp = qp;
            obj->brctrl.qp = obj->brctrl.base_qp;
            //
            obj->brctrl.bytes_target_last = (bytes_target_P / p_num);
            obj->brctrl.bytes_target_sum += (bytes_target_P / p_num);
        }

        //printf("rate_control1: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
    }
    //obj->brctrl.qp = obj->brctrl.base_qp;
    ret = obj->brctrl.qp;
    ///printf("rate_control1: obj->brctrl.base_qp= %d \n", obj->brctrl.base_qp);
    return ret;
}
int rate_control(void *hnd)
{
    CodecObj *obj = (CodecObj *)hnd;
    cJSON *json = (cJSON *)obj->param;
    int ret = 0;
    int ref_idx = GetvalueInt(json, "ref_idx");
    int gop_size = GetvalueInt(json, "gop_size");
    int refs = GetvalueInt(json, "refs");
    int bit_rate = GetvalueInt(json, "bit_rate");
    int fps = GetvalueInt(json, "frame_rate");
    int refs_all[5];//1,2,4,8,16
    int bytes_per_frame = (bit_rate >> 3) / fps;
    int bytes_P;//10%
    int bytes_I;//10%
    int bytes_FP0[8];//0,2,4,6,8,10,12,15
    int bytes_FP1[8];//1,3,5,7,9,11,13
    //printf("rate_control: refs= %d \n", refs);
    if(refs == 16)
        ret = rate_control16(hnd);
    else if(refs == 8)
        ret = rate_control8(hnd);
    else if(refs == 4)
        ret = rate_control4(hnd);
    else if(refs == 2)
        ret = rate_control2(hnd);
    else if(refs == 1)
        ret = rate_control1(hnd);
    return ret;
}

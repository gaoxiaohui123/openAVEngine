/*
 * Copyright (c) 2020
 * Created by gxh_20200602
 */

#include "hcsvc.h"
#include "inc.h"

extern int GetvalueInt(cJSON *json, char *key);
//extern FILE * global_logfp;
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
    //MYPRINT("get_I_qp: factor= %f \n", factor);

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
    //MYPRINT("get_I_qp: factor= %f \n", factor);
    int qp = base_qp + (int)((factor / base_factor - 1.0) * qp_step);

    //MYPRINT("get_I_qp: qp= %d \n", qp);
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
    int pict_type = GetvalueInt(json, "pict_type");
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

    //MYPRINT("rate_control16: fp0_num= %d \n", fp0_num);
    //MYPRINT("rate_control16: fp1_num= %d \n", fp1_num);

    //MYPRINT("rate_control16: bit_rate= %d kbps\n", (bit_rate / 1024));
    //MYPRINT("rate_control16: bytes_target_I= %d \n", bytes_target_I);
    //MYPRINT("rate_control16: bytes_target_P= %d \n", bytes_target_P);
    //MYPRINT("rate_control16: bytes_target_FP0= %d \n", bytes_target_FP0);
    //MYPRINT("rate_control16: bytes_target_FP1= %d \n", bytes_target_FP1);
    //MYPRINT("rate_control16: frame_idx= %d \n", frame_idx);
    //MYPRINT("rate_control16: ref_idx= %d \n", ref_idx);
    //MYPRINT("rate_control16: pict_type= %d \n", pict_type);
    if (ref_idx == 0 && (pict_type != 2))
    {
        //I
        //MYPRINT("rate_control16: I \n");
        //MYPRINT("rate_control16: bytes_target_I= %d \n", bytes_target_I);

        //
        obj->brctrl.bytes_sum = 0;
        //obj->brctrl.base_qp = 26;
        int qp = get_I_qp(hnd);
        obj->brctrl.base_qp = qp + 3;
        obj->brctrl.qp = qp;
        //obj->brctrl.qp = obj->brctrl.base_qp;//26;//23;
        //MYPRINT("rate_control16: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
        //
        obj->brctrl.bytes_target_sum = bytes_target_I;
        obj->brctrl.bytes_target_last = bytes_target_I;
    }
    else if(ref_idx == refs || pict_type == 2)
    {
        //P
        //MYPRINT("rate_control16: P \n");
        //MYPRINT("rate_control16: bytes_target_P= %d \n", (bytes_target_P / p_num));
        //according to last info
        int diff = obj->brctrl.bytes_sum - obj->brctrl.bytes_target_sum;
        int diff2 = obj->brctrl.bytes_last - obj->brctrl.bytes_target_last;
        float k = 0.2;
        //int bytes_overflow = (int)((float)obj->brctrl.bytes_target_sum * k);
        //MYPRINT("rate_control16: 0: bytes_overflow= %d \n", bytes_overflow);
        //MYPRINT("rate_control16: 0: diff= %d \n", diff);
        //MYPRINT("rate_control16: 0: diff2= %d \n", diff2);
        //if(global_logfp)
        //{
        //    fprintf(global_logfp,"rate_control16: 0: bytes_overflow= %d \n", bytes_overflow);
        //    fprintf(global_logfp,"rate_control16: 0: diff= %d \n", diff);
        //    fflush(global_logfp);
        //}
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
        //MYPRINT("rate_control16: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
    }
    else if (((ref_idx & 1) == 0) || (ref_idx == (refs - 1)) || (ref_idx == 1))
    {
        //[1, 2,4,6,8,10,12,14,15]
        //MYPRINT("rate_control16: FP1 \n");
        //MYPRINT("rate_control16: bytes_target_FP1= %d \n", (bytes_target_FP1 / fp1_num));
        //
        int diff = obj->brctrl.bytes_sum - obj->brctrl.bytes_target_sum;
        int diff2 = obj->brctrl.bytes_last - obj->brctrl.bytes_target_last;
        float k = 0.2;
        //int bytes_overflow = (int)((float)obj->brctrl.bytes_target_sum * k);
        //MYPRINT("rate_control16: 1: bytes_overflow= %d \n", bytes_overflow);
        //MYPRINT("rate_control16: 1: diff= %d \n", diff);
        //MYPRINT("rate_control16: 1: diff2= %d \n", diff2);
        //if(global_logfp)
        //{
        //    fprintf(global_logfp,"rate_control16: 1: bytes_overflow= %d \n", bytes_overflow);
        //    fprintf(global_logfp,"rate_control16: 1: diff= %d \n", diff);
        //    fflush(global_logfp);
        //}
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
        //MYPRINT("rate_control16: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
    }
    else if ((ref_idx & 1) == 1)
    {
        //[3,5,7,9,11,13]
        //MYPRINT("rate_control16: FP0 \n");
        //MYPRINT("rate_control16: bytes_target_FP0= %d \n", (bytes_target_FP0 / fp0_num));
        //
        int diff = obj->brctrl.bytes_sum - obj->brctrl.bytes_target_sum;
        int diff2 = obj->brctrl.bytes_last - obj->brctrl.bytes_target_last;
        float k = 0.2;
        //int bytes_overflow = (int)((float)obj->brctrl.bytes_target_sum * k);
        //MYPRINT("rate_control16: 2: bytes_overflow= %d \n", bytes_overflow);
        //MYPRINT("rate_control16: 2: diff= %d \n", diff);
        //MYPRINT("rate_control16: 2: diff2= %d \n", diff2);
        //if(global_logfp)
        //{
        //    fprintf(global_logfp,"rate_control16: 2: bytes_overflow= %d \n", bytes_overflow);
        //    fprintf(global_logfp,"rate_control16: 2: diff= %d \n", diff);
        //    fflush(global_logfp);
        //}
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
        //MYPRINT("rate_control16: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
    }
    //obj->brctrl.qp = 26;//test
    ret = obj->brctrl.qp;
    ///MYPRINT("rate_control16: obj->brctrl.base_qp= %d \n", obj->brctrl.base_qp);
    ///MYPRINT("rate_control16: obj->brctrl.qp= %d \n", obj->brctrl.qp);
    //if(global_logfp)
    //{
    //    fprintf(global_logfp,"rate_control16: obj->brctrl.base_qp= %d \n", obj->brctrl.base_qp);
    //    fprintf(global_logfp,"rate_control16: obj->brctrl.qp= %d \n", obj->brctrl.qp);
    //    fflush(global_logfp);
    //}
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
    int pict_type = GetvalueInt(json, "pict_type");
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

    //MYPRINT("rate_control8: fp0_num= %d \n", fp0_num);
    //MYPRINT("rate_control8: fp1_num= %d \n", fp1_num);

    //MYPRINT("rate_control8: bit_rate= %d kbps\n", (bit_rate / 1024));
    //MYPRINT("rate_control8: bytes_target_I= %d \n", bytes_target_I);
    //MYPRINT("rate_control8: bytes_target_P= %d \n", bytes_target_P);
    //MYPRINT("rate_control8: bytes_target_FP0= %d \n", bytes_target_FP0);
    //MYPRINT("rate_control8: bytes_target_FP1= %d \n", bytes_target_FP1);
    //MYPRINT("rate_control8: frame_idx= %d \n", frame_idx);
    //MYPRINT("rate_control8: ref_idx= %d \n", ref_idx);
    if (ref_idx == 0 && pict_type != 2)
    {
        //I
        //MYPRINT("rate_control8: I \n");
        MYPRINT("rate_control8: bytes_target_I= %d \n", bytes_target_I);

        //
        obj->brctrl.bytes_sum = 0;
        //obj->brctrl.base_qp = 26;
        //obj->brctrl.qp = obj->brctrl.base_qp;//26;//23;
        int qp = get_I_qp(hnd);
        obj->brctrl.base_qp = qp + 3;
        obj->brctrl.qp = qp;
        //MYPRINT("rate_control8: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
        //
        obj->brctrl.bytes_target_sum = bytes_target_I;
        obj->brctrl.bytes_target_last = bytes_target_I;
    }
    else if(ref_idx == refs || pict_type == 2)
    {
        //P
        //MYPRINT("rate_control8: P \n");
        MYPRINT("rate_control8: bytes_target_P= %d \n", (bytes_target_P / p_num));
        //according to last info
        int diff = obj->brctrl.bytes_sum - obj->brctrl.bytes_target_sum;
        int diff2 = obj->brctrl.bytes_last - obj->brctrl.bytes_target_last;
        float k = 0.1;
        int bytes_overflow3 = (int)((float)obj->brctrl.bytes_target_sum * k);
        MYPRINT("rate_control8: bytes_overflow= %d \n", bytes_overflow);
        MYPRINT("rate_control8: diff= %d \n", diff);
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
        //MYPRINT("rate_control8: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
    }
    else if (((ref_idx & 1) == 0) || (ref_idx == (refs - 1)) || (ref_idx == 1))
    {
        //[1, 2,4,6,8,10,12,14,15]
        //MYPRINT("rate_control8: FP1 \n");
        MYPRINT("rate_control8: bytes_target_FP1= %d \n", (bytes_target_FP1 / fp1_num));
        //
        int diff = obj->brctrl.bytes_sum - obj->brctrl.bytes_target_sum;
        int diff2 = obj->brctrl.bytes_last - obj->brctrl.bytes_target_last;
        float k = 0.1;
        int bytes_overflow3 = (int)((float)obj->brctrl.bytes_target_sum * k);
        MYPRINT("rate_control8: bytes_overflow= %d \n", bytes_overflow);
        MYPRINT("rate_control8: diff= %d \n", diff);
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
        //MYPRINT("rate_control8: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
    }
    else if ((ref_idx & 1) == 1)
    {
        //[3,5,7,9,11,13]
        //MYPRINT("rate_control8: FP0 \n");
        MYPRINT("rate_control8: bytes_target_FP0= %d \n", (bytes_target_FP0 / fp0_num));
        //
        int diff = obj->brctrl.bytes_sum - obj->brctrl.bytes_target_sum;
        int diff2 = obj->brctrl.bytes_last - obj->brctrl.bytes_target_last;
        float k = 0.1;
        int bytes_overflow3 = (int)((float)obj->brctrl.bytes_target_sum * k);
        MYPRINT("rate_control8: bytes_overflow= %d \n", bytes_overflow);
        MYPRINT("rate_control8: diff= %d \n", diff);
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
        //MYPRINT("rate_control8: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
    }
    //obj->brctrl.qp = 26;//test
    ret = obj->brctrl.qp;
    MYPRINT("rate_control8: obj->brctrl.base_qp= %d \n", obj->brctrl.base_qp);
    MYPRINT("rate_control8: obj->brctrl.qp= %d \n", obj->brctrl.qp);
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
    int pict_type = GetvalueInt(json, "pict_type");
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

    //MYPRINT("rate_control4: fp0_num= %d \n", fp0_num);
    //MYPRINT("rate_control4: fp1_num= %d \n", fp1_num);

    //MYPRINT("rate_control4: bit_rate= %d kbps\n", (bit_rate / 1024));
    //MYPRINT("rate_control4: bytes_target_I= %d \n", bytes_target_I);
    //MYPRINT("rate_control4: bytes_target_P= %d \n", bytes_target_P);
    //MYPRINT("rate_control4: bytes_target_FP0= %d \n", bytes_target_FP0);
    //MYPRINT("rate_control4: bytes_target_FP1= %d \n", bytes_target_FP1);
    //MYPRINT("rate_control4: frame_idx= %d \n", frame_idx);
    //MYPRINT("rate_control4: ref_idx= %d \n", ref_idx);
    if (ref_idx == 0 && pict_type != 2)
    {
        //I
        //MYPRINT("rate_control4: I \n");
        //MYPRINT("rate_control4: bytes_target_I= %d \n", bytes_target_I);

        //
        obj->brctrl.bytes_sum = 0;
        //obj->brctrl.base_qp = 26;
        //obj->brctrl.qp = obj->brctrl.base_qp;//26;//23;
        int qp = get_I_qp(hnd);
        obj->brctrl.base_qp = qp + 3;
        obj->brctrl.qp = qp;
        //MYPRINT("rate_control4: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
        //
        obj->brctrl.bytes_target_sum = bytes_target_I;
        obj->brctrl.bytes_target_last = bytes_target_I;
    }
    else if(ref_idx == refs || pict_type == 2)
    {
        //P
        //MYPRINT("rate_control4: P \n");
        //MYPRINT("rate_control4: bytes_target_P= %d \n", (bytes_target_P / p_num));
        //according to last info
        int diff = obj->brctrl.bytes_sum - obj->brctrl.bytes_target_sum;
        int diff2 = obj->brctrl.bytes_last - obj->brctrl.bytes_target_last;
        float k = 0.2;
        //int bytes_overflow = (int)((float)obj->brctrl.bytes_target_sum * k);
        //MYPRINT("rate_control4: bytes_overflow= %d \n", bytes_overflow);
        //MYPRINT("rate_control4: diff= %d \n", diff);
        //if(global_logfp)
        //{
        //    fprintf(global_logfp,"rate_control4: 0: bytes_overflow= %d \n", bytes_overflow);
        //    fprintf(global_logfp,"rate_control4: 0: diff= %d \n", diff);
        //    fflush(global_logfp);
        //}
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
        //MYPRINT("rate_control4: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
    }
    else if (((ref_idx & 1) == 0) || ((ref_idx == (refs - 1)) && (refs > 4)) || (ref_idx == 1))
    {
        //[1, 2,4,6,8,10,12,14,15]
        //MYPRINT("rate_control4: FP1 \n");
        //MYPRINT("rate_control4: bytes_target_FP1= %d \n", (bytes_target_FP1 / fp1_num));
        //
        int diff = obj->brctrl.bytes_sum - obj->brctrl.bytes_target_sum;
        int diff2 = obj->brctrl.bytes_last - obj->brctrl.bytes_target_last;
        float k = 0.2;
        //int bytes_overflow = (int)((float)obj->brctrl.bytes_target_sum * k);
        //MYPRINT("rate_control4: bytes_overflow= %d \n", bytes_overflow);
        //MYPRINT("rate_control4: diff= %d \n", diff);
        //if(global_logfp)
        //{
        //    fprintf(global_logfp,"rate_control4: 1: bytes_overflow= %d \n", bytes_overflow);
        //    fprintf(global_logfp,"rate_control4: 1: diff= %d \n", diff);
        //    fflush(global_logfp);
        //}
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
        //MYPRINT("rate_control4: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
    }
    else if ((ref_idx & 1) == 1)
    {
        //[3,5,7,9,11,13]
        //MYPRINT("rate_control4: FP0 \n");
        //MYPRINT("rate_control4: bytes_target_FP0= %d \n", (bytes_target_FP0 / fp0_num));
        //
        int diff = obj->brctrl.bytes_sum - obj->brctrl.bytes_target_sum;
        int diff2 = obj->brctrl.bytes_last - obj->brctrl.bytes_target_last;
        float k = 0.2;
        //int bytes_overflow = (int)((float)obj->brctrl.bytes_target_sum * k);
        //MYPRINT("rate_control4: bytes_overflow= %d \n", bytes_overflow);
        //MYPRINT("rate_control4: diff= %d \n", diff);
        //if(global_logfp)
        //{
        //    fprintf(global_logfp,"rate_control4: 2: bytes_overflow= %d \n", bytes_overflow);
        //    fprintf(global_logfp,"rate_control4: 2: diff= %d \n", diff);
        //    fflush(global_logfp);
        //}
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
        //MYPRINT("rate_control4: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
    }
    //obj->brctrl.qp = 26;//test
    ret = obj->brctrl.qp;
    //MYPRINT("rate_control4: obj->brctrl.base_qp= %d \n", obj->brctrl.base_qp);
    //MYPRINT("rate_control4: obj->brctrl.qp= %d \n", obj->brctrl.qp);
    //if(global_logfp)
    //{
    //    fprintf(global_logfp,"rate_control4: obj->brctrl.base_qp= %d \n", obj->brctrl.base_qp);
    //    fprintf(global_logfp,"rate_control4: obj->brctrl.qp= %d \n", obj->brctrl.qp);
    //    fflush(global_logfp);
    //}
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
    int pict_type = GetvalueInt(json, "pict_type");
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

    //MYPRINT("rate_control2: fp0_num= %d \n", fp0_num);
    //MYPRINT("rate_control2: fp1_num= %d \n", fp1_num);

    //MYPRINT("rate_control2: bit_rate= %d kbps\n", (bit_rate / 1024));
    //MYPRINT("rate_control2: bytes_target_I= %d \n", bytes_target_I);
    //MYPRINT("rate_control2: bytes_target_P= %d \n", bytes_target_P);
    //MYPRINT("rate_control2: bytes_target_FP0= %d \n", bytes_target_FP0);
    //MYPRINT("rate_control2: bytes_target_FP1= %d \n", bytes_target_FP1);
    //MYPRINT("rate_control2: frame_idx= %d \n", frame_idx);
    //MYPRINT("rate_control2: ref_idx= %d \n", ref_idx);
    if (ref_idx == 0 && pict_type != 2)
    {
        //I
        //MYPRINT("rate_control2: I \n");
        //MYPRINT("rate_control2: bytes_target_I= %d \n", bytes_target_I);

        //
        obj->brctrl.bytes_sum = 0;
        //obj->brctrl.base_qp = 26;
        //obj->brctrl.qp = obj->brctrl.base_qp;//26;//23;
        int qp = get_I_qp(hnd);
        obj->brctrl.base_qp = qp + 3;
        obj->brctrl.qp = qp;
        //MYPRINT("rate_control2: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
        //
        obj->brctrl.bytes_target_sum = bytes_target_I;
        obj->brctrl.bytes_target_last = bytes_target_I;
    }
    else if(ref_idx == refs || pict_type == 2)
    {
        //P
        //MYPRINT("rate_control2: P \n");
        //MYPRINT("rate_control2: bytes_target_P= %d \n", (bytes_target_P / p_num));
        //according to last info
        int diff = obj->brctrl.bytes_sum - obj->brctrl.bytes_target_sum;
        int diff2 = obj->brctrl.bytes_last - obj->brctrl.bytes_target_last;
        float k = 0.2;
        //int bytes_overflow = (int)((float)obj->brctrl.bytes_target_sum * k);
        //MYPRINT("rate_control2: bytes_overflow= %d \n", bytes_overflow);
        //MYPRINT("rate_control2: diff= %d \n", diff);
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
        //MYPRINT("rate_control2: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
    }
    else if ((ref_idx & 1) == 1)
    {
        //[3,5,7,9,11,13]
        //MYPRINT("rate_control2: FP0 \n");
        //MYPRINT("rate_control2: bytes_target_FP0= %d \n", (bytes_target_FP0 / fp0_num));
        //
        int diff = obj->brctrl.bytes_sum - obj->brctrl.bytes_target_sum;
        int diff2 = obj->brctrl.bytes_last - obj->brctrl.bytes_target_last;
        float k = 0.2;
        //int bytes_overflow = (int)((float)obj->brctrl.bytes_target_sum * k);
        //MYPRINT("rate_control2: bytes_overflow= %d \n", bytes_overflow);
        //MYPRINT("rate_control2: diff= %d \n", diff);
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
        //MYPRINT("rate_control2: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
    }
    //obj->brctrl.qp = 26;//test
    ret = obj->brctrl.qp;
    //MYPRINT("rate_control2: obj->brctrl.base_qp= %d \n", obj->brctrl.base_qp);
    //MYPRINT("rate_control2: obj->brctrl.qp= %d \n", obj->brctrl.qp);
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
    pict_type = GetvalueInt(json, "pict_type");
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

    ///MYPRINT("rate_control1: bit_rate= %d kbps\n", (bit_rate / 1024));
    //MYPRINT("rate_control1: bytes_target_I= %d \n", bytes_target_I);
    //MYPRINT("rate_control1: bytes_target_P= %d \n", bytes_target_P);
    //MYPRINT("rate_control1: frame_idx= %d \n", frame_idx);
    //MYPRINT("rate_control1: ref_idx= %d \n", ref_idx);
    if (frame_idx == 0 && pict_type != 2)
    {
        //I
        //MYPRINT("rate_control1: I \n");
        //MYPRINT("rate_control1: bytes_target_I= %d \n", bytes_target_I);

        //
        obj->brctrl.bytes_sum = 0;
        //obj->brctrl.base_qp = 26;
        int qp = get_I_qp(hnd);
        obj->brctrl.base_qp = qp + 3;
        obj->brctrl.qp = qp;
        //MYPRINT("rate_control1: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
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
            MYPRINT("rate_control2: bytes_overflow= %d \n", bytes_overflow);
            MYPRINT("rate_control2: diff= %d \n", diff);
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
            //MYPRINT("rate_control1: P \n");
            //MYPRINT("rate_control1: bytes_target_P= %d \n", (bytes_target_P / p_num));
            //according to last info
            int diff = obj->brctrl.bytes_sum - obj->brctrl.bytes_target_sum;//长期
            int diff2 = obj->brctrl.bytes_last - obj->brctrl.bytes_target_last;//即时
            float k = 0.1;
            int bytes_overflow3 = (int)((float)obj->brctrl.bytes_target_sum * k);
            //MYPRINT("rate_control1: bytes_overflow= %d \n", bytes_overflow);
            //MYPRINT("rate_control1: diff= %d \n", diff);
            //
            //if ((diff > bytes_overflow) && (diff2 > -bytes_overflow2))
            //if((diff2 > bytes_overflow))
            if((diff2 > bytes_overflow) || (diff > bytes_overflow3))
            //if((diff2 > bytes_overflow) && (diff > bytes_overflow3))
            {
                //MYPRINT("rate_control1: diff= %d \n", diff);
                //MYPRINT("rate_control1: bytes_overflow3= %d \n", bytes_overflow3);
                //MYPRINT("rate_control1: diff2= %d \n", diff2);
                //MYPRINT("rate_control1: bytes_overflow= %d \n", bytes_overflow);
                //MYPRINT("rate_control1: obj->brctrl.base_qp= %d \n", obj->brctrl.base_qp);
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

        //MYPRINT("rate_control1: obj->brctrl.bytes_target_sum = %d \n", obj->brctrl.bytes_target_sum );
    }
    //obj->brctrl.qp = obj->brctrl.base_qp;
    ret = obj->brctrl.qp;
#if 0
    obj->lastQp[obj->qp_idx] = ret;
    obj->qp_idx++;
    if(obj->qp_idx >= MAX_QP_NUM)
    {
        obj->qp_idx = 0;
    }
#endif
    ///MYPRINT("rate_control1: obj->brctrl.base_qp= %d \n", obj->brctrl.base_qp);
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
    int skip_frame = GetvalueInt(json, "skip_frame");
    int qmin = GetvalueInt(json, "qmin");
    int qmax = GetvalueInt(json, "qmax");
    int refs_all[5];//1,2,4,8,16
    int bytes_per_frame = (bit_rate >> 3) / fps;
    int bytes_P;//10%
    int bytes_I;//10%
    int bytes_FP0[8];//0,2,4,6,8,10,12,15
    int bytes_FP1[8];//1,3,5,7,9,11,13
    //MYPRINT("rate_control: refs= %d \n", refs);
    if(obj->brc_pause)
    {
    }
    else{
        if(obj->last_skip_frame && !skip_frame)
        {
            obj->brctrl.base_qp = qmin + ((qmax - qmin) >> 1);//30
        }
        if(!skip_frame)
        {
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
        }
    }

#if 1
    obj->lastQp[obj->qp_idx] = obj->brctrl.qp;
    obj->qp_idx++;
    if(obj->qp_idx >= MAX_QP_NUM)
    {
        obj->qp_idx = 0;
    }
#endif
    obj->last_skip_frame = skip_frame;
    return ret;
}
//设计的目标：
//带宽不足下，主动丢帧，并保证平缓不流畅（即：无卡顿，帧间隔拉大但等距）;
//帧率不足，视觉效果上表现为不流畅，但未必不平缓;
int is_bw_overflow(char *handle, int cur_bitrate, int cur_framerate, float threshold, int difftime, int basetime)
{
    int ret = 0;
    long long *testp = (long long *)handle;
    CodecObj *obj = (CodecObj *)testp[0];
    if(obj)
    {
        cJSON *json = (cJSON *)obj->param;
        //float threshold = 0.25;
        //int skip_frame = 0;//GetvalueInt(json, "skip_frame");
        int qmin = obj->c->qmin;//GetvalueInt(json, "qmin");
        int qmax = obj->c->qmax;//GetvalueInt(json, "qmax");
        int bit_rate = obj->c->bit_rate;
        int target_framerate = obj->target_framerate;
        int frame_rate = obj->new_framerate;//
        int diff = cur_bitrate - obj->c->bit_rate;
        int frame_overflow = cur_framerate - frame_rate;//当前帧率减目标帧率
        if(obj->c->bit_rate <= 0)
        {
            MYPRINT("error: is_bw_overflow: obj->c->bit_rate=%d \n", obj->c->bit_rate);
            return ret;
        }
        float rate = (float)diff / (float)obj->c->bit_rate;
        int pre_framerate = target_framerate;
        if(rate > (threshold * 5))
        {
            pre_framerate = 1;//target_framerate >> 5;
        }
        else if(rate > (threshold * 4))
        {
            pre_framerate = target_framerate >> 4;
        }
        else if(rate > (threshold * 3))
        {
            pre_framerate = target_framerate >> 3;
        }
        else if(rate > (threshold * 2))
        {
            pre_framerate = target_framerate >> 2;
        }
        else if(rate > (threshold * 1))
        {
            pre_framerate = target_framerate >> 1;
        }
        //
        int pre_framerate2 = target_framerate;
        //MYPRINT("is_bw_overflow: difftime=%d, basetime=%d \n", difftime, basetime);
        if(difftime > (basetime * 5))
        {
            pre_framerate2 = 1;//target_framerate >> 5;
        }
        else if(difftime > (basetime * 4))
        {
            pre_framerate2 = target_framerate >> 4;
        }
        else if(difftime > (basetime * 3))
        {
            pre_framerate2 = target_framerate >> 3;
        }
        else if(difftime > (basetime * 2))
        {
            pre_framerate2 = target_framerate >> 2;
        }
        else if(difftime > (basetime * 1))
        {
            pre_framerate2 = target_framerate >> 1;
        }
        pre_framerate = pre_framerate < pre_framerate2 ? pre_framerate : pre_framerate2;
        //MYPRINT("is_bw_overflow: pre_framerate=%d \n", pre_framerate);
        //MYPRINT("is_bw_overflow: threshold=%1.2f \n", threshold);
        //MYPRINT("is_bw_overflow: cur_framerate=%d, cur_bitrate=%d \n", cur_framerate, cur_bitrate);
        //MYPRINT("is_bw_overflow: frame_rate=%d, cur_bitrate=%d, bit_rate=%d, rate=%1.2f \n", frame_rate, cur_bitrate, bit_rate, rate);
        //MYPRINT("is_bw_overflow: qmin=%d, qmax=%d, skip_frame=%d \n", qmin, qmax, skip_frame);
        obj->brc_pause = 0;
        if(rate > 0.05 || difftime > basetime)
        {
            if(frame_overflow >= 1 && (frame_rate <= pre_framerate))
            //if(frame_overflow >= 1)
            {
                //帧率溢出，超出了设定帧率，跳过此帧
                obj->brc_pause = 1;//码率控制暂停
                //帧率存在收敛过程，防止频繁跳帧
                ret = 1;
                obj->skip_num++;
                if(obj->skip_num >= obj->skip_freq)
                {
                    ret = 0;
                    obj->skip_num = 0;
                }
                //MYPRINT("is_bw_overflow: ret=%d, frame_overflow=%d, obj->skip_freq=%d \n", ret, frame_overflow, obj->skip_freq);
            }
            else if(rate > (threshold * 5) || difftime > (basetime * 5))
            {

                //if(frame_rate > pre_framerate)//防止频繁skip
                if(frame_rate != pre_framerate)
                {
                    MYPRINT("is_bw_overflow: cur_framerate=%d, frame_rate=%d, pre_framerate=%d, threshold=%1.2f \n", cur_framerate, frame_rate, pre_framerate, threshold);
                    MYPRINT("is_bw_overflow: bit_rate=%d, cur_bitrate=%d, diff=%d, rate=%1.2f \n", bit_rate, cur_bitrate, diff, rate);
                    obj->brctrl.base_qp = qmax;
                    obj->brctrl.qp = qmax;
                    obj->brc_pause = 1;//码率控制暂停
                    for(int i = 0; i < MAX_QP_NUM; i++)
                    {
                        obj->lastQp[i] = qmax;
                    }
                    obj->skip_num = 1;
                    obj->skip_freq = 32;
                    frame_rate = pre_framerate;//target_framerate >> 4;
                    frame_rate = frame_rate > 0 ? frame_rate : 1;
                    obj->new_framerate = frame_rate;
                    ret = 1;
                    //MYPRINT("is_bw_overflow: frame_rate=%d \n", frame_rate);
                }
            }
            else if(rate > (threshold * 4) || difftime > (basetime * 4))
            {

                //if(frame_rate > pre_framerate)//防止频繁skip
                if(frame_rate != pre_framerate)
                {
                    MYPRINT("is_bw_overflow: cur_framerate=%d, frame_rate=%d, pre_framerate=%d, threshold=%1.2f \n", cur_framerate, frame_rate, pre_framerate, threshold);
                    MYPRINT("is_bw_overflow: bit_rate=%d, cur_bitrate=%d, diff=%d, rate=%1.2f \n", bit_rate, cur_bitrate, diff, rate);
                    obj->brctrl.base_qp = qmax;
                    obj->brctrl.qp = qmax;
                    obj->brc_pause = 1;//码率控制暂停
                    for(int i = 0; i < MAX_QP_NUM; i++)
                    {
                        obj->lastQp[i] = qmax;
                    }
                    obj->skip_num = 1;
                    obj->skip_freq = 16;
                    frame_rate = pre_framerate;//target_framerate >> 4;
                    frame_rate = frame_rate > 0 ? frame_rate : 1;
                    obj->new_framerate = frame_rate;
                    ret = 1;
                    //MYPRINT("is_bw_overflow: frame_rate=%d \n", frame_rate);
                }
            }
            else if(rate > (threshold * 3) || difftime > (basetime * 3))
            {
                //if(frame_rate > pre_framerate)//防止频繁skip
                if(frame_rate != pre_framerate)
                {
                    MYPRINT("is_bw_overflow: cur_framerate=%d, frame_rate=%d, pre_framerate=%d, threshold=%1.2f \n", cur_framerate, frame_rate, pre_framerate, threshold);
                    MYPRINT("is_bw_overflow: bit_rate=%d, cur_bitrate=%d, diff=%d, rate=%1.2f \n", bit_rate, cur_bitrate, diff, rate);
                    obj->brctrl.base_qp = qmax;
                    obj->brctrl.qp = qmax;
                    obj->brc_pause = 1;//码率控制暂停
                    for(int i = 0; i < MAX_QP_NUM; i++)
                    {
                        obj->lastQp[i] = qmax;
                    }
                    obj->skip_num = 1;
                    obj->skip_freq = 8;
                    frame_rate = pre_framerate;//target_framerate >> 3;
                    frame_rate = frame_rate > 0 ? frame_rate : 1;
                    obj->new_framerate = frame_rate;
                    ret = 1;
                    //MYPRINT("is_bw_overflow: frame_rate=%d \n", frame_rate);
                }
            }
            else if(rate > (threshold * 2) || difftime > (basetime * 2))
            {
                //if(frame_rate > pre_framerate)//防止频繁skip
                if(frame_rate != pre_framerate)
                {
                    MYPRINT("is_bw_overflow: cur_framerate=%d, frame_rate=%d, pre_framerate=%d, threshold=%1.2f \n", cur_framerate, frame_rate, pre_framerate, threshold);
                    MYPRINT("is_bw_overflow: bit_rate=%d, cur_bitrate=%d, diff=%d, rate=%1.2f \n", bit_rate, cur_bitrate, diff, rate);
                    obj->brctrl.base_qp = qmax;
                    obj->brctrl.qp = qmax;
                    obj->brc_pause = 1;//码率控制暂停
                    for(int i = 0; i < MAX_QP_NUM; i++)
                    {
                        obj->lastQp[i] = qmax;
                    }
                    obj->skip_num = 1;
                    obj->skip_freq = 4;
                    frame_rate = pre_framerate;//target_framerate >> 2;
                    frame_rate = frame_rate > 0 ? frame_rate : 1;
                    obj->new_framerate = frame_rate;
                    ret = 1;
                    //MYPRINT("is_bw_overflow: frame_rate=%d \n", frame_rate);
                }
            }
            else if(rate > threshold || difftime > (basetime * 1))
            {
                //if(frame_rate > pre_framerate)//防止频繁skip
                if(frame_rate != pre_framerate)
                {
                    MYPRINT("is_bw_overflow: cur_framerate=%d, frame_rate=%d, pre_framerate=%d, threshold=%1.2f \n", cur_framerate, frame_rate, pre_framerate, threshold);
                    MYPRINT("is_bw_overflow: bit_rate=%d, cur_bitrate=%d, diff=%d, rate=%1.2f \n", bit_rate, cur_bitrate, diff, rate);
                    ret = 1;
                    //注意：只有恢复到正常帧率，qp才可以回调
                    for(int i = 0; i < MAX_QP_NUM; i++)
                    {
                        obj->lastQp[i] = qmax;//test
                        //
                        //int qp = obj->lastQp[i];
                        //if(qp < qmax)
                        //{
                        //    ret = 0;
                        //}
                    }
                    //if(ret)// && rate < 0.5)
                    {
                        //MYPRINT("is_bw_overflow: cur_bitrate=%d, bit_rate=%d, rate=%1.2f \n", cur_bitrate, bit_rate, rate);
                        obj->lastQp[obj->qp_idx % MAX_QP_NUM] = 0;
                        obj->brc_pause = 1;//码率控制暂停
                        obj->skip_num = 1;
                        obj->skip_freq = 2;
                        frame_rate = pre_framerate;//target_framerate >> 1;
                        frame_rate = frame_rate > 0 ? frame_rate : 1;
                        obj->new_framerate = frame_rate;
                        //MYPRINT("is_bw_overflow: frame_rate=%d \n", frame_rate);
                    }
                }
            }
        }
        else{
            if(!obj->brc_pause)
            {
                //帧率回调
                if(rate < -threshold && difftime <= (basetime >> 1))//亏了0.25
                {
                    if(frame_rate == (target_framerate >> 5))
                    {
                        if(frame_overflow >= 0)
                        {
                            //帧率供给足够状态:
                            //帧率不能处于亏欠状态，即帧率供给是够的
                            //防止在不恰当时机回调
                            int target_framerate2 = target_framerate >> 4;
                            target_framerate2 = target_framerate2 > 0 ? target_framerate2 : 1;
                            obj->new_framerate = target_framerate2;
                            obj->skip_freq = 16;
                            obj->adjust_framerate = 0;
                            //MYPRINT("is_bw_overflow: obj->new_framerate=%d,rate=%1.2f \n", obj->new_framerate, rate);
                        }
                        else{
                            //帧率亏欠状态:
                            //1）可能处于上一波调整过程中
                            //2）帧率由于视频采集，编码时长等原因导致供给不足，错过码率控制恢复时机
                            obj->adjust_framerate++;
                            if(obj->adjust_framerate > cur_framerate)
                            {
                                int target_framerate2 = target_framerate >> 4;
                                target_framerate2 = target_framerate2 > 0 ? target_framerate2 : 1;
                                obj->new_framerate = target_framerate2;
                                obj->skip_freq = 16;
                                obj->adjust_framerate = 0;
                            }
                            //MYPRINT("is_bw_overflow: 1: obj->adjust_framerate=%d \n", obj->adjust_framerate);
                        }
                    }
                    else if(frame_rate == (target_framerate >> 4))
                    {
                        if(frame_overflow >= 0)
                        {
                            //帧率供给足够状态:
                            //帧率不能处于亏欠状态，即帧率供给是够的
                            //防止在不恰当时机回调
                            int target_framerate2 = target_framerate >> 3;
                            target_framerate2 = target_framerate2 > 0 ? target_framerate2 : 1;
                            obj->new_framerate = target_framerate2;
                            obj->skip_freq = 8;
                            obj->adjust_framerate = 0;
                            //MYPRINT("is_bw_overflow: obj->new_framerate=%d,rate=%1.2f \n", obj->new_framerate, rate);
                        }
                        else{
                            //帧率亏欠状态:
                            //1）可能处于上一波调整过程中
                            //2）帧率由于视频采集，编码时长等原因导致供给不足，错过码率控制恢复时机
                            obj->adjust_framerate++;
                            if(obj->adjust_framerate > cur_framerate)
                            {
                                int target_framerate2 = target_framerate >> 3;
                                target_framerate2 = target_framerate2 > 0 ? target_framerate2 : 1;
                                obj->new_framerate = target_framerate2;
                                obj->skip_freq = 8;
                                obj->adjust_framerate = 0;
                            }
                            //MYPRINT("is_bw_overflow: 1: obj->adjust_framerate=%d \n", obj->adjust_framerate);
                        }
                    }
                    else if(frame_rate == (target_framerate >> 3))
                    {
                        if(frame_overflow >= 0)
                        {
                            //帧率供给足够状态:
                            //帧率不能处于亏欠状态，即帧率供给是够的
                            //防止在不恰当时机回调
                            int target_framerate2 = target_framerate >> 2;
                            target_framerate2 = target_framerate2 > 0 ? target_framerate2 : 1;
                            obj->new_framerate = target_framerate2;
                            obj->skip_freq = 4;
                            obj->adjust_framerate = 0;
                            //MYPRINT("is_bw_overflow: obj->new_framerate=%d,rate=%1.2f \n", obj->new_framerate, rate);
                        }
                        else{
                            //帧率亏欠状态:
                            //1）可能处于上一波调整过程中
                            //2）帧率由于视频采集，编码时长等原因导致供给不足，错过码率控制恢复时机
                            obj->adjust_framerate++;
                            if(obj->adjust_framerate > cur_framerate)
                            {
                                int target_framerate2 = target_framerate >> 2;
                                target_framerate2 = target_framerate2 > 0 ? target_framerate2 : 1;
                                obj->new_framerate = target_framerate2;
                                obj->skip_freq = 4;
                                obj->adjust_framerate = 0;
                            }
                            //MYPRINT("is_bw_overflow: 1: obj->adjust_framerate=%d \n", obj->adjust_framerate);
                        }
                    }
                    else if(frame_rate == (target_framerate >> 2))
                    {
                        if(frame_overflow >= 0)
                        {
                            //帧率供给足够状态:
                            //帧率不能处于亏欠状态，即帧率供给是够的
                            //防止在不恰当时机回调
                            int target_framerate2 = target_framerate >> 1;
                            target_framerate2 = target_framerate2 > 0 ? target_framerate2 : 1;
                            obj->new_framerate = target_framerate2;
                            obj->skip_freq = 2;
                            obj->adjust_framerate = 0;
                            //MYPRINT("is_bw_overflow: obj->new_framerate=%d,rate=%1.2f \n", obj->new_framerate, rate);
                        }
                        else{
                            //帧率亏欠状态:
                            //1）可能处于上一波调整过程中
                            //2）帧率由于视频采集，编码时长等原因导致供给不足，错过码率控制恢复时机
                            obj->adjust_framerate++;
                            if(obj->adjust_framerate > cur_framerate)
                            {
                                int target_framerate2 = target_framerate >> 1;
                                target_framerate2 = target_framerate2 > 0 ? target_framerate2 : 1;
                                obj->new_framerate = target_framerate2;
                                obj->skip_freq = 2;
                                obj->adjust_framerate = 0;
                            }
                            //MYPRINT("is_bw_overflow: 1: obj->adjust_framerate=%d \n", obj->adjust_framerate);
                        }
                    }
                    else if(frame_rate == (target_framerate >> 1))
                    {
                        if(frame_overflow >= 0)
                        {
                            //帧率供给足够状态:
                            //帧率不能处于亏欠状态，即帧率供给是够的
                            //防止在不恰当时机回调
                            obj->new_framerate = target_framerate;
                            obj->skip_freq = 0;
                            obj->brc_pause = 0;
                            obj->adjust_framerate = 0;
                            //MYPRINT("is_bw_overflow: obj->new_framerate=%d,rate=%1.2f \n", obj->new_framerate, rate);
                        }
                        else{
                            //帧率亏欠状态:
                            //1）可能处于上一波调整过程中
                            //2）帧率由于视频采集，编码时长等原因导致供给不足，错过码率控制恢复时机
                            obj->adjust_framerate++;
                            if(obj->adjust_framerate > cur_framerate)
                            {
                                obj->new_framerate = target_framerate;
                                obj->skip_freq = 0;
                                obj->brc_pause = 0;
                                obj->adjust_framerate = 0;
                            }
                            //MYPRINT("is_bw_overflow: 2: obj->adjust_framerate=%d \n", obj->adjust_framerate);
                        }
                    }
                    else{
                    }

                }
                else if(rate < -(2 * threshold))//亏了0.5
                {
                    obj->new_framerate = target_framerate;
                    obj->skip_freq = 0;
                    obj->brc_pause = 0;
                    obj->adjust_framerate = 0;
                    //MYPRINT("is_bw_overflow: obj->new_framerate=%d,rate=%1.2f \n", obj->new_framerate, rate);
                }
            }
        }
    }

    return ret;
}
char *renew_frame(PreProcObj *obj, char *new_frame, int idx)
{
    char *old_frame = obj->last_frame[idx];
    obj->last_frame[idx] = new_frame;//obj->last_frame[idx - 1];
    obj->skip_frame_idx = idx;
    idx++;
    if(idx >= MAX_SKIP_FRAME_NUM || !old_frame)
    {
        return old_frame;
    }
    old_frame = renew_frame(obj, old_frame, idx);
    return old_frame;
}
//注意：
//摄像头下开启，是因为存在噪声，噪声这种有害的干扰信息，会极大的占用码字，
//在静帧下，开启静帧检测，实际是以帧为单元，“过滤”这种噪声；
//桌面共享下，因不存在噪声干扰，静止宏块能被压缩算法(包括x264)准确检查出,
//基本不会占用码字，因此，无需开启静帧检测，并能获得更好的效果；
int is_static_frame(PreProcObj *obj, int w, int h, int frame_idx, char *outbuf)
{
    int ret = 0;
    //CodecInfoObj *config = (CodecInfoObj *)&obj->codecInfo;

    //int w = config->width;
    //int h = config->height;
    int size = w * h;

#if 0
    int m = 0;//16x16->64*64
    int w4 = w >> m;
    int h4 = h >> m;
    int size4 = w4 * h4;
    char *outbuf2 = (char *)av_malloc((size4 * 3) >> 1);
    memcpy(outbuf2, outbuf, ((size4 * 3) >> 1));
#else
    int m = 2;//16x16->64*64
    int w4 = w >> m;
    int h4 = h >> m;
    int nsize = 5;//4;
    int size4 = w4 * h4;
    int n = (w4 >> nsize) * (h4 >> nsize);
    //MYPRINT("is_static_frame: w4=%d, h4=%d, n=%d \n", w4, h4, n);
    if(n < 1)
    {
        return ret;
    }
    int fsize4 = (w4 * h4) + (((w4 >> 1) * (h4 >> 1)) << 1);
    int fsize4_2 = (size4 * 3) >> 1;
    //MYPRINT("is_static_frame: fsize4=%d, fsize4_2=%d \n", fsize4, fsize4_2);
    char *outbuf2 = (char *)av_malloc((size4 * 3) >> 1);
    if(!obj->img_convert_ctx)
    {
        obj->img_convert_ctx = sws_getContext(  w,
	                                            h,
	                                            AV_PIX_FMT_YUV420P,
	                                            w4,
	                                            h4,
	                                            AV_PIX_FMT_YUV420P,
	                                            SWS_FAST_BILINEAR,
	                                            NULL, NULL, NULL);
    }
    if(obj->img_convert_ctx)
    {
        AVFrame src_frame = {};//注意：初始化至关重要，否则会导致异常崩溃
        src_frame.data[0] = outbuf;
        src_frame.data[1] = &outbuf[w * h];
        src_frame.data[2] = &outbuf[w * h + (w >> 1) * (h >> 1)];
        src_frame.linesize[0] = w;
        src_frame.linesize[1] = w >> 1;
        src_frame.linesize[2] = w >> 1;
        src_frame.width = w;
	    src_frame.height = h;
        src_frame.format = AV_PIX_FMT_YUV420P;


        AVFrame dst_frame = {};//注意：初始化至关重要，否则会导致异常崩溃
        dst_frame.data[0] = outbuf2;
        dst_frame.data[1] = &outbuf2[w4 * h4];
        dst_frame.data[2] = &outbuf2[w4 * h4 + (w4 >> 1) * (h4 >> 1)];
        dst_frame.linesize[0] = w4;
        dst_frame.linesize[1] = w4 >> 1;
        dst_frame.linesize[2] = w4 >> 1;
        dst_frame.width = w4;
	    dst_frame.height = h4;
        dst_frame.format = AV_PIX_FMT_YUV420P;
        sws_scale(  obj->img_convert_ctx,
	                src_frame.data,
			        src_frame.linesize, 0,
			        h,
			        dst_frame.data,
			        dst_frame.linesize);
    }

#endif
    if(obj->skip_frame_idx)
    {
        int nsize = 4;
        int msize = nsize * nsize;//16;
        w = w4;
        h = h4;
        size = size4;
        int thread0 = 700;//600;//500;
        int thread1 = 600;//500;//400;
        int thread2 = 500;//400;
        //
        uint8_t *y0 = &outbuf2[0];
        uint8_t *u0 = &outbuf2[size];
        uint8_t *v0 = &outbuf2[size + (size >> 2)];
        uint8_t *y1 = &obj->last_frame[0][0];
        uint8_t *u1 = &obj->last_frame[0][size];
        uint8_t *v1 = &obj->last_frame[0][size + (size >> 2)];
        uint8_t *y2 = &obj->last_frame[obj->skip_frame_idx][0];
        uint8_t *u2 = &obj->last_frame[obj->skip_frame_idx][size];
        uint8_t *v2 = &obj->last_frame[obj->skip_frame_idx][size + (size >> 2)];
        int64_t sumy0 = 0;
        int64_t sumy1 = 0;
        //int n = ((w - msize) >> nsize) * ((h - msize) >> nsize);
        int n = (w >> nsize) * (h >> nsize);
        //
        int mb_w = (w >> 4) + ((w % 16) ? 1 : 0);
        int mb_h = (h >> 4) + ((h % 16) ? 1 : 0);
        int bmb_w = (mb_w >> 3) + ((mb_w % 8) ? 1 : 0);
        int bmb_h = mb_h;
        //
        //MYPRINT("is_static_frame: 1: n=%d \n", n);
        for(int i = 0; i < h; i += msize)
        {
            for(int j = 0; j < w; j += msize)
            {
                if((j + msize) > w || (i + msize) > h)
                {
                    break;
                }
                //MYPRINT("is_static_frame: i=%d, j=%d \n", i, j);
                int sad = 0;
                sad = I2SADnxm(obj->sadHnd, 16, 16, &y0[i * w + j], w, &y1[i * w + j], w);
                //int sad2 = x264_pixel_sa8d_16x16_gxh(obj->x264_hnd, &y0[i * w + j], w, &y1[i * w + j], w);
                //MYPRINT("is_static_frame: sad=%d, sad2=%d, w=%d \n", sad, sad2, w);
                if(sad > (thread1 * 4))
                {
                    //MYPRINT("is_static_frame: 0: sad=%d, i=%d, j=%d \n", sad, i, j);
                    goto save_frame;//
                    //continue;
                }
                sumy0 += sad;
                sad = I2SADnxm(obj->sadHnd, 16, 16, &y0[i * w + j], w, &y2[i * w + j], w);
                //sad = x264_pixel_sa8d_16x16_gxh(obj->x264_hnd, &y0[i * w + j], w, &y2[i * w + j], w);
                if(sad > (thread0 * 4))
                {
                    //MYPRINT("is_static_frame: 1: sad=%d, i=%d, j=%d \n", sad, i, j);
                    goto save_frame;//
                    //continue;
                }
                sumy1 += sad;
#if 0
                int bmb_xy = i * bmb_w + (j >> 3);
                int bmb_bit = (j % 8);
                uint8_t value = obj->skip_mb[bmb_xy];
                value |= 1 << bmb_bit;
                obj->skip_mb[bmb_xy] = value;
#endif
            }
        }
        int avgsady0 = (int)(sumy0 / n);
        int avgsady1 = (int)(sumy1 / n);

        w = w >> 1;
        h = h >> 1;
        int64_t sumu0 = 0;
        int64_t sumu1 = 0;
        int64_t sumv0 = 0;
        int64_t sumv1 = 0;
        //n = ((w - msize) >> nsize) * ((h - msize) >> nsize);
        n = (w >> nsize) * (h >> nsize);
        //MYPRINT("is_static_frame: 2: n=%d \n", n);
        for(int i = 0; i < h; i += msize)
        {
            for(int j = 0; j < w; j += msize)
            {
                if((j + msize) > w || (i + msize) > h)
                {
                    break;
                }
                //MYPRINT("is_static_frame: i=%d, j=%d \n", i, j);
                int sad = 0;
                sad = I2SADnxm(obj->sadHnd, 16, 16, &u0[i * w + j], w, &u1[i * w + j], w);
                //int sad2 = x264_pixel_sa8d_16x16_gxh(obj->x264_hnd, &u0[i * w + j], w, &u1[i * w + j], w);
                //MYPRINT("is_static_frame: sad=%d, sad2=%d, w=%d \n", sad, sad2, w);
                if(sad > (thread2 << 1))
                {
                    //MYPRINT("is_static_frame: 2: sad=%d, i=%d, j=%d \n", sad, i, j);
                    goto save_frame;//continue;
                }
                sumu0 += sad;
                sad = I2SADnxm(obj->sadHnd, 16, 16, &u0[i * w + j], w, &u2[i * w + j], w);
                //sad = x264_pixel_sa8d_16x16_gxh(obj->x264_hnd, &u0[i * w + j], w, &u2[i * w + j], w);
                if(sad > (thread2 << 1))
                {
                    //MYPRINT("is_static_frame: 3: sad=%d, i=%d, j=%d \n", sad, i, j);
                    goto save_frame;//continue;
                }
                sumu1 += sad;

                sad = I2SADnxm(obj->sadHnd, 16, 16, &v0[i * w + j], w, &v1[i * w + j], w);
                //sad = x264_pixel_sa8d_16x16_gxh(obj->x264_hnd, &v0[i * w + j], w, &v1[i * w + j], w);
                if(sad > (thread2 << 1))
                {
                    //MYPRINT("is_static_frame: 4: sad=%d, i=%d, j=%d \n", sad, i, j);
                    goto save_frame;//continue;
                }
                sumv0 += sad;
                sad = I2SADnxm(obj->sadHnd, 16, 16, &v0[i * w + j], w, &v2[i * w + j], w);
                //sad = x264_pixel_sa8d_16x16_gxh(obj->x264_hnd, &v0[i * w + j], w, &v2[i * w + j], w);
                if(sad > (thread2 << 1))
                {
                    //MYPRINT("is_static_frame: 5: sad=%d, i=%d, j=%d \n", sad, i, j);
                    goto save_frame;//continue;
                }
                sumv1 += sad;

            }
        }
        int avgsadu0 = (int)(sumu0 / n);
        int avgsadu1 = (int)(sumu1 / n);
        int avgsadv0 = (int)(sumv0 / n);
        int avgsadv1 = (int)(sumv1 / n);

        ret |= avgsady0 < thread1 && avgsady1 < thread0 && avgsadu0 < thread2 && avgsadu1 < thread2 && avgsadv0 < thread2 && avgsadv1 < thread2;
        if(!ret && false)
        {
            if(!(avgsady0 < thread1))
                MYPRINT("is_static_frame: avgsady0=%d \n", avgsady0);
            if(!(avgsady1 < thread0))
                MYPRINT("is_static_frame: avgsady1=%d \n", avgsady1);
            if(!(avgsadu0 < thread2))
                MYPRINT("is_static_frame: avgsadu0=%d \n", avgsadu0);
            if(!(avgsadu1 < thread2))
                MYPRINT("is_static_frame: avgsadu1=%d \n", avgsadu1);
            if(!(avgsadv0 < thread2))
                MYPRINT("is_static_frame: avgsadv0=%d \n", avgsadv0);
            if(!(avgsadv1 < thread2))
                MYPRINT("is_static_frame: avgsadv1=%d \n", avgsadv1);
        }
        if(ret && false)
        {
            MYPRINT("is_static_frame: ret=%d \n", ret);
        }
    }
save_frame:
    if(obj->last_frame[0])
    {
        //MYPRINT("is_static_frame: obj->last_frame[0]=%x \n", obj->last_frame[0]);
        if(!(frame_idx % 25))
        {
            char *old_frame = renew_frame(obj, obj->last_frame[0], 1);
            //MYPRINT("is_static_frame: old_frame=%x \n", old_frame);
            if(old_frame)
            {
                av_free(old_frame);
            }
            //obj->last_frame[1] = obj->last_frame[0];
        }
        else{
            av_free(obj->last_frame[0]);
        }
    }
    obj->last_frame[0] = outbuf2;

    return ret;
}
int br_push_data(BitRateNode **brhead, int size, int new_frame, int64_t now_time)
{
    int ret = 0;
    BitRateNode *head,*pnew, *q;
    if(!*brhead)
    {
        //head不存数据,head->next指向第一个数据,head->num保存总索引（也可以设计将第一个节点存入head）
        //注意链表元的2个组成部分，元和next指针，元控制的是整个链表元的地址，next指针则寄生在元下；
        //MYPRINT("PushData: create head: obj->data_list=%x \n", obj->data_list);
        head = (BitRateNode *)calloc(1, sizeof(BitRateNode));  //创建头节点。
        head->num = 0;
        head->cur_bitrate = 0;
        head->cur_framerate = 0;
        head->frame_idx = 0;
        head->next = NULL;  //头节点指针域置NULL
        head->tail = head;  // 开始时尾指针指向头节点；此时head->tail->next与head->next是同一地址
        *brhead = (void *)head;
    }
    head = (BitRateNode *)(*brhead);
    if(head->num > 1)
    {
        BitRateNode *first_node = head->next;
        BitRateNode *last_node = head->tail;
        int difftime = (int)(last_node->frame_timestamp - first_node->frame_timestamp);
        int framenum = (int)(last_node->frame_idx - first_node->frame_idx);
        if(difftime >= 1000)
        {
            //
            int sumbits = 0;
            q = head->next;
            do{
                if(!q->next)
                {
                    break;
                }
                sumbits += q->size;
                q = q->next;
            }while(1);
            ret = sumbits;
            head->cur_bitrate = (int)((float)ret / ((float)difftime / 1000.0) + 0.5);
            head->cur_framerate = (int)((float)framenum / ((float)difftime / 1000.0) + 0.5);
            //
            q = head->next;
            head->next = head->next->next;
            head->num--;
            if(head->next == NULL)
            {
                head->next = NULL;  //头节点指针域置NULL
                head->tail = head;
            }
            free(q);
        }
    }
    {
        pnew = (BitRateNode *)calloc(1, sizeof(BitRateNode));  //创建新节点
        pnew->size = size;
        pnew->frame_idx = head->frame_idx;
        pnew->idx = head->idx;
        pnew->frame_timestamp = now_time;
        pnew->next = NULL;   //新节点指针域置NULL
        head->tail->next = pnew;  //新节点插入到表尾；首个节点时，则意味着ead->next也指向了pnew
        head->tail = pnew;   //为指针指向当前的尾节点;元地址迁移，元的原先的next脱离了元，新地址的next成为元的next指针；
        head->frame_idx += new_frame;
        head->idx++;
        head->num++;
    }
    return ret;
}
HCSVC_API
int api_br_push_data(char *handle, int size, int new_frame, int64_t now_time)
{
    long long *testp = (long long *)handle;
    CodecObj *codec = (CodecObj *)testp[0];
    int ret = br_push_data(&codec->ppObj->brhead, size, new_frame, now_time);
    return ret;
}
void release_brnode(BitRateNode *head)
{
    if(head && head->num)
    {
        do{
            BitRateNode *q;
            q = head->next;
            if(q == NULL || q == head)
            {
                break;
            }
            else{
                head->next = head->next->next;
            }
            free(q);   //释放节点i的内存单元
        }while(1);
        free(head);
    }
}
HCSVC_API
int api_control_max_bitrate(char *handle, int frame_idx, uint8_t *frame_data, float threshold)
{
    int ret = 0;

    long long *testp = (long long *)handle;
    CodecObj *codec = (CodecObj *)testp[0];

    PreProcObj *obj = codec->ppObj;

    int w = codec->width;
    int h = codec->height;
    //MYPRINT("control_max_bitrate: w=%d, h=%d, obj->bmb_size=%d \n", w, h, obj->bmb_size);
    memset(obj->skip_mb, 0, obj->bmb_size);
    int is_static = 0;
    if(w * h >= 640 * 360)
    {
        //因为需要1/4下采样，分辨率太小，容易误判
        is_static = is_static_frame(obj, w, h, frame_idx, frame_data);
        if(is_static)
        {
            //obj->json = api_renew_json_int(obj->json, "skip_frame", 1);
            ret = 1;
            //MYPRINT("control_max_bitrate: 1: is_static=%d \n", is_static);
        }
    }

    if(!is_static)
    {
        int cur_bitrate = 0;
        int cur_framerate = 0;
        if(obj->brhead)
        {
            if(obj->brhead->cur_bitrate)
            {
                cur_bitrate = obj->brhead->cur_bitrate;
            }
            if(obj->brhead->cur_framerate)
            {
                cur_framerate = obj->brhead->cur_framerate;
            }
            int is_overflow = is_bw_overflow(handle, cur_bitrate, cur_framerate, threshold,  0, 0);
            if(is_overflow)
            {
                //MYPRINT("control_max_bitrate: is_overflow=%d \n", is_overflow);
                //return ret;
                //int skip_frame = 1;
                //obj->json = api_renew_json_int(obj->json, "skip_frame", skip_frame);
                memset(obj->skip_mb, 255, obj->bmb_size);
                ret = 2;
            }
        }

    }
    api_video_set_skip_mb(handle, obj->skip_mb, obj->bmb_size);
    if(is_static)
    {
        //MYPRINT("control_max_bitrate: 2: ret=%d \n", ret);
    }
    return ret;
}
HCSVC_API
int api_control_max_bitrate2(char *handle, int frame_idx, uint8_t *frame_data, float threshold, int difftime, int basetime)
{
    int ret = 0;

    long long *testp = (long long *)handle;
    CodecObj *codec = (CodecObj *)testp[0];

    PreProcObj *obj = codec->ppObj;

    int w = codec->width;
    int h = codec->height;
    //MYPRINT("control_max_bitrate: w=%d, h=%d, obj->bmb_size=%d \n", w, h, obj->bmb_size);
    memset(obj->skip_mb, 0, obj->bmb_size);
    int is_static = 0;
    if(w * h >= 640 * 480)
    {
        //因为需要1/4下采样，分辨率太小，容易误判
        is_static = is_static_frame(obj, w, h, frame_idx, frame_data);
        if(is_static)
        {
            //obj->json = api_renew_json_int(obj->json, "skip_frame", 1);
            ret = 1;
            //MYPRINT("control_max_bitrate: 1: is_static=%d \n", is_static);
        }
    }

    if(!is_static)
    {
        int cur_bitrate = 0;
        int cur_framerate = 0;
        if(obj->brhead)
        {
            if(obj->brhead->cur_bitrate)
            {
                cur_bitrate = obj->brhead->cur_bitrate;
            }
            if(obj->brhead->cur_framerate)
            {
                cur_framerate = obj->brhead->cur_framerate;
            }
            int is_overflow = is_bw_overflow(handle, cur_bitrate, cur_framerate, threshold, difftime, basetime);
            if(is_overflow)
            {
                //MYPRINT("control_max_bitrate: is_overflow=%d \n", is_overflow);
                //return ret;
                //int skip_frame = 1;
                //obj->json = api_renew_json_int(obj->json, "skip_frame", skip_frame);
                memset(obj->skip_mb, 255, obj->bmb_size);
                ret = 2;
            }
        }

    }
    api_video_set_skip_mb(handle, obj->skip_mb, obj->bmb_size);
    if(is_static)
    {
        //MYPRINT("control_max_bitrate: 2: ret=%d \n", ret);
    }
    return ret;
}
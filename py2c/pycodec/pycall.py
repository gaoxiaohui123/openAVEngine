# -*- coding: utf-8 -*


import sys
import os
import shutil
import time
import cv2

#import matplotlib.pyplot as plt # plt 用于显示图片
#from matplotlib import pyplot as plt
#import matplotlib.image as mpimg # mpimg 用于读取图片
import numpy as np
#from PIL import Image
import math

import ctypes
from ctypes import *
import json


(WIDTH, HEIGHT) = (352, 288)
src_dir = r'/data/home/gxh/works/datashare/for_ENC/352X288/foreman_cif.yuv'

sys.path.append(".")

class PyCall(object):
    def __init__(self):
        ll = ctypes.cdll.LoadLibrary
        try:
            lib = ll("./libpycall.so")
        except:
            lib = ll("../py2c/libpycall.so")
        ret = lib.foo(1, 3)
        print("ret", ret)

        lib.ret_str.restype = c_char_p
        s = lib.ret_str()
        print("len(s)= ", len(s))
        print("s", s)
        ###
        print("load my dll")
        try:
            self.lib = ll("./decoding_encoding.so")
        except:
            self.lib = ll("../py2c/decoding_encoding.so")
        filename = 'test-0.h264'
        codec_id = -1
        ##self.lib.APIEncodecTest(filename, codec_id)
        self.lib.mystr2json("")
        #self.lib.api_video_encode_one_frame.argtypes = [c_char_p]
        #self.lib.api_video_encode_one_frame.restype = c_char_p
        h264filename = r'/data/home/gxh/works/datashare/for_ENC/stream720p/FVDO_Girl_720p.264'
        h264filename = r'/data/home/gxh/works/datashare/jy/mytest/test_py.h264'
        yuvfilename = r'/data/home/gxh/works/datashare/jy/mytest/out.yuv'
        yuvfilename = 'out.yuv'
        h264filename = "test_py.h264"
        #self.lib.APIDecodecTest(yuvfilename, h264filename)

    def psnr1(self, img1, img2):
        #PSNR = 10*log10(255^2/MSE); MSE = ((E(P0(i,j) - P1(i,j)))^2)/n
        mse = np.mean((img1 / 1.0 - img2 / 1.0) ** 2)

        if mse < 1.0e-10:
            return 100
        return 10 * math.log10(255.0 ** 2 / mse)

    def psnr2(self, img1, img2):
        mse = np.mean((img1 / 255. - img2 / 255.) ** 2)

        if mse < 1.0e-10:
            return 100
        PIXEL_MAX = 1
        return 20 * math.log10(PIXEL_MAX / math.sqrt(mse))
    def get_yuv_psnr(self, infile, outfile, W, H):
        frame_size = (W * H * 3) / 2
        fp0 = open(infile, 'rb')
        fp1 = open(outfile, 'rb')
        sum_psnr = 0
        sum_psnrY = 0
        sum_psnrU = 0
        sum_psnrV = 0
        i = 0
        while True:
            data0 = fp0.read(frame_size)
            data1 = fp1.read(frame_size)
            if len(data0) > 0 and len(data1) > 0:
                frame0 = np.frombuffer(data0, dtype=np.uint8)
                yuv_I420_0 = frame0.reshape((H * 3) / 2, W)
                img0 = cv2.cvtColor(yuv_I420_0, cv2.COLOR_YUV2BGR_I420)

                frame1 = np.frombuffer(data1, dtype=np.uint8)
                yuv_I420_1 = frame1.reshape((H * 3) / 2, W)
                img1 = cv2.cvtColor(yuv_I420_1, cv2.COLOR_YUV2BGR_I420)
                psnr = self.psnr1(img0, img1)
                ###
                size0 = W*H
                size1 = size0 + ((W*H) >> 2)
                size2 = size1 + ((W*H) >> 2)
                y0 = data0[:size0]
                v0 = data0[size0:size1]
                u0 = data0[size1:size2]
                y1 = data1[:size0]
                v1 = data1[size0:size1]
                u1 = data1[size1:size2]
                frameY0 = np.frombuffer(y0, dtype=np.uint8)
                frameU0 = np.frombuffer(u0, dtype=np.uint8)
                frameV0 = np.frombuffer(v0, dtype=np.uint8)
                frameY1 = np.frombuffer(y1, dtype=np.uint8)
                frameU1 = np.frombuffer(u1, dtype=np.uint8)
                frameV1 = np.frombuffer(v1, dtype=np.uint8)
                psnrY = self.psnr1(frameY0, frameY1)
                psnrU = self.psnr1(frameU0, frameU1)
                psnrV = self.psnr1(frameV0, frameV1)
                print("(psnrY,psnrU,psnrV)= ", (psnrY,psnrU,psnrV))
                sum_psnrY += psnrY
                sum_psnrU += psnrU
                sum_psnrV += psnrV
                psnr = self.psnr1(frame0, frame1)
                print("(i,psnr)= ", (i,psnr))
                sum_psnr += psnr
                i += 1
            else:
                break
        avg_psnrY = sum_psnrY / i
        avg_psnrU = sum_psnrU / i
        avg_psnrV = sum_psnrV / i
        avg_psnr = sum_psnr / i
        return (avg_psnr, avg_psnrY, avg_psnrU, avg_psnrV)
    def interlace2frame_2(self, frame0, frame1):
        h,w,ch = frame0.shape
        (y, u, v) = cv2.split(frame0)
        print("u.shape= ", u.shape)
        #print("type(frame0)= ", type(frame0))
        #print("y= ", y)
        cv2.imshow('y', y)
        cv2.imshow('u', u)
        cv2.imshow('v', v)
        cv2.waitKey(0)
        return frame0

    def interlace2frame(self, data0, data1, width, height):
        data = ""
        for i in range(0, height, 1):
            data += data0[(i + 0) * width: (i + 1) * width]
            data += data1[(i + 0) * width: (i + 1) * width]
        return data
    def interlaceYUV2frame(self, data0, data1, w, h):
        size0 = w * h
        size1 = size0 + ((w * h) >> 2)
        size2 = size1 + ((w * h) >> 2)
        y0 = data0[:size0]
        u0 = data0[size0:size1]
        v0 = data0[size1:size2]
        y1 = data1[:size0]
        u1 = data1[size0:size1]
        v1 = data1[size1:size2]
        y = self.interlace2frame(y0, y1, w, h)
        u = self.interlace2frame(u0, u1, w >> 1, h >> 1)
        v = self.interlace2frame(v0, v1, w >> 1, h >> 1)
        data = y + u + v
        return data
    def interlace(self, data, width, height):
        (data0, data1) = ("", "")
        for i in range(0, height, 2):
            data0 += data[(i+0)*width : (i+1)*width]
            data1 += data[(i+1)*width : (i+2)*width]
        return (data0, data1)
    def interlaceYUV(self,data, w,h):
        size0 = w * h
        size1 = size0 + ((w * h) >> 2)
        size2 = size1 + ((w * h) >> 2)
        y = data[:size0]
        u = data[size0:size1]
        v = data[size1:size2]
        (y0, y1) = self.interlace(y, w, h)
        (u0, u1) = self.interlace(u, w >> 1, h >> 1)
        (v0, v1) = self.interlace(v, w >> 1, h >> 1)
        data0 = y0 + u0 + v0
        data1 = y1 + u1 + v1
        return (data0, data1)
    def test_interlace(self, yuvfilename, w, h):
        frame_size = (w * h * 3) / 2
        fp = open(yuvfilename, 'rb')
        i = 0
        while True:
            data = fp.read(frame_size)
            if len(data) > 0:
                start_time = time.time()
                (data0, data1) = self.interlaceYUV(data, w, h)
                difftime = time.time() - start_time
                print('{} test_interlace time: {:.3f}ms'.format(time.ctime(), difftime * 1000))
                frame0 = np.frombuffer(data0, dtype=np.uint8)
                yuv_I420_0 = frame0.reshape((h * 3) / 4, w)
                img0 = cv2.cvtColor(yuv_I420_0, cv2.COLOR_YUV2BGR_I420)
                frame1 = np.frombuffer(data1, dtype=np.uint8)
                yuv_I420_1 = frame1.reshape((h * 3) / 4, w)
                img1 = cv2.cvtColor(yuv_I420_1, cv2.COLOR_YUV2BGR_I420)
                frame = np.vstack([img0, img1])  # vstack #hstack
                ###
                data2 = self.interlaceYUV2frame(data0, data1, w, h >> 1)
                frame2 = np.frombuffer(data2, dtype=np.uint8)
                yuv_I420_2 = frame2.reshape((h * 3) / 2, w)
                frame3 = cv2.cvtColor(yuv_I420_2, cv2.COLOR_YUV2BGR_I420)
                showflag = True  # False
                if showflag:
                    cv2.imshow("frame", frame)
                    #cv2.imshow("frame3", yuv_I420_2)
                    cv2.imshow("frame3", frame3)
                    cv2.waitKey(40)
                print("i= ", i)
                #break
            else:
                break
    def setparam(self, id):
        param = {}
        self.mtu_size = 0  # 1400#0 #1400
        width = WIDTH
        height = HEIGHT
        param.update({"width": width})
        param.update({"height": height})
        param.update({"bit_rate": 400000})
        param.update({"gop_size": 50})
        param.update({"preset": "superfast"})  # "slow"
        # param.update({"preset": "slow"})
        param.update({"tune": "zerolatency"})
        if self.mtu_size > 0:
            param.update({"slice-max-size": str(self.mtu_size)})
        param.update({"tff": 0})
        param.update({"bff": 0})
        param.update({"qmin": 26})  # 20
        param.update({"qmax": 26})  # 40
        param.update({"max_b_frames": 0})
        param.update({"coder_type": 0})  # cavlc/cabac
        param.update({"refs": 1})
        param.update({"frame_rate": 25})
        param.update({"thread_type": 0})
        param.update({"thread_count": 0})
        frame_size = (width * height * 3) / 2
        outbuf = create_string_buffer(frame_size)
        array_type = c_char_p * 4
        outparam = array_type()
        return (param, outbuf, outparam)
    def test_encode(self, yuvfilename, h264filename):
        sizelist = []
        #yuvfilename = '/data/home/gxh/works/datashare/for_ENC/352X288/foreman_cif.yuv'
        #h264filename = "test_py.h264"
        try:
            fout = open(h264filename, "wb")
        except:
            print("open file fail !")
            return
        print("start run test_encode")
        obj_id = 0
        (width, height) = (WIDTH, HEIGHT)
        (param, outbuf, outparam) = self.setparam(obj_id)

        frame_size = (width * height * 3) / 2
        #outbuf = bytearray(frame_size)
        #content = content.ljust(frame_size) #填充空格
        #buffers.value = content #将数据写入
        with open(yuvfilename, 'rb') as fp:
            i = 0
            total_bytes = 0
            key_bytes = 0
            while True:
                data = fp.read(frame_size)
                if len(data) > 0:
                    param.update({"frame_idx": i})
                    param_str = json.dumps(param, encoding='utf-8', ensure_ascii=False, indent=4, sort_keys=True)
                    ###
                    outparam[0] = "test1"
                    ###
                    ret = self.lib.api_video_encode_one_frame(obj_id, data, param_str, outbuf, outparam)
                    print("outparam[0]= ", outparam[0])
                    #print("len(s)= ", len(s))
                    print("ret= ", ret)
                    if ret > 0:
                        sizelist.append(ret)
                        total_bytes += ret
                        if outparam[0] in ["PIC_TYPE_KEYFRAME"]:
                            key_bytes += ret
                        #print("outbuf.raw=", outbuf.raw)
                        data2 = outbuf.raw[:ret]
                        #print("data2= ", data2)
                        print("len(data2)= ", len(data2))
                        fout.write(data2)
                        fout.flush()
                    i += 1
                else:
                    print("len(data)= ", len(data))
                    break
        fp.close()
        fout.close()
        print("End run dll")
        print('***finish***')
        return sizelist
    def setparam2(self, id):
        param = {}
        width = WIDTH
        height = HEIGHT
        param.update({"oneframe": 1})
        frame_size = (width * height * 3) / 2
        outbuf = create_string_buffer(frame_size)
        array_type = c_char_p * 4
        outparam = array_type()
        return (param, outbuf, outparam)
    def test_decode(self, yuvfilename, h264filename, sizelist):
        #yuvfilename = 'out.yuv'
        #h264filename = "test_py.h264"
        try:
            fout = open(yuvfilename, "wb")
        except:
            print("open file fail !")
            return
        print("start run test_decode")
        obj_id = 1
        (param, outbuf, outparam) = self.setparam2(obj_id)
        frame_idx = 0
        with open(h264filename, 'rb') as fp:
            for i in range(len(sizelist)):
                frame_size = sizelist[i]
                data = fp.read(frame_size)
                if len(data) > 0:
                    #print("len(data)= ", len(data))
                    #print("frame_size= ", frame_size)
                    param.update({"insize": frame_size})
                    param_str = json.dumps(param, encoding='utf-8', ensure_ascii=False, indent=4, sort_keys=True)
                    ###
                    outparam[0] = "test1"
                    #buf = bytearray(frame_size + 64)
                    #buf[:frame_size] = data
                    #buf[frame_size:] = 0
                    ###
                    ret = self.lib.api_video_decode_one_frame(obj_id, data, param_str, outbuf, outparam)
                    print("outparam[0]= ", outparam[0])
                    print("frame_idx= ", frame_idx)
                    data2 = outbuf.raw[:ret]
                    fout.write(data2)
                    fout.flush()
                    frame_idx += 1
    def setparam3(self, id):
        param = {}
        width = WIDTH
        height = HEIGHT
        w = width << 1
        h = height << 1
        w = width >> 1
        h = height >> 1
        dst_size = ((w * h) * 3) >> 1
        param.update({"src_w": width})
        param.update({"src_h": height})
        param.update({"dst_size": str(w)+"x"+str(h)})
        param.update({"src_pix_fmt": "AV_PIX_FMT_YUV420P"})
        param.update({"dst_pix_fmt": "AV_PIX_FMT_YUV420P"})
        param.update({"scale_mode": "SWS_BILINEAR"})

        outbuf = create_string_buffer(dst_size)
        array_type = c_char_p * 4
        outparam = array_type()
        return (param, outbuf, outparam)

    def test_scale(self, yuvfilename, width, height):
        obj_id = 1
        (param, outbuf, outparam) = self.setparam3(obj_id)
        frame_size = (width * height * 3) / 2
        frame_idx = 0
        with open(yuvfilename, 'rb') as fp:
            while True:
                data = fp.read(frame_size)
                if len(data) > 0:

                    param.update({"insize": frame_size})
                    param_str = json.dumps(param, encoding='utf-8', ensure_ascii=False, indent=4, sort_keys=True)
                    ###
                    outparam[0] = "test1"
                    ###
                    start_time = time.time()
                    ret = self.lib.api_ff_scale(obj_id, data, param_str, outbuf, outparam)
                    difftime = time.time() - start_time
                    print('{} test_scale time: {:.3f}ms'.format(time.ctime(), difftime * 1000))
                    print("outparam[0]= ", outparam[0])
                    ###
                    frame = np.frombuffer(outbuf, dtype=np.uint8)
                    w = width << 1
                    h = height << 1
                    w = width >> 1
                    h = height >> 1
                    oneframe_I420 = frame.reshape((h * 3) / 2, w)
                    oneframe_RGB = cv2.cvtColor(oneframe_I420, cv2.COLOR_YUV2BGR_I420)
                    cv2.imshow("oneframe_RGB", oneframe_RGB)
                    cv2.waitKey(40)
                    ###
                    #break
                else:
                    break
    def test_call(self):
        yuvfilename0 = '/data/home/gxh/works/datashare/for_ENC/352X288/foreman_cif.yuv'
        h264filename = "test_py.h264"
        sizelist = self.test_encode(yuvfilename0, h264filename)
        yuvfilename1 = 'out.yuv'
        h264filename = "test_py.h264"
        self.test_decode(yuvfilename1, h264filename, sizelist)
        (avg_psnr, avg_psnrY, avg_psnrU, avg_psnrV) = self.get_yuv_psnr(yuvfilename0, yuvfilename1, WIDTH, HEIGHT)
        print("(avg_psnr, avg_psnrY, avg_psnrU, avg_psnrV)=", (avg_psnr, avg_psnrY, avg_psnrU, avg_psnrV))
    def test_call2(self):
        yuvfilename = 'out.yuv'
        #call.test_interlace(yuvfilename, WIDTH, HEIGHT)
        call.test_scale(yuvfilename, WIDTH, HEIGHT)

if __name__ == '__main__':
    print('Start pycall.')
    call = PyCall()
    call.test_call2()

    print('End pycall.')
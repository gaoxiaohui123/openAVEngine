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
from callvideoencode import CallVideoEncode
from callvideodecode import CallVideoDecode
from callvideointerlace import CallVideoInterlace
from callvideopsnr import CallVideoPsnr
from callvideoscale import CallVideoScale
import loadlib

#(WIDTH, HEIGHT) = (352, 288)
#src_dir = r'/data/home/gxh/works/datashare/for_ENC/352X288/foreman_cif.yuv'

sys.path.append(".")

class BCodecCore(object):
    def __init__(self):
        self.videoencode0 = CallVideoEncode(0)
        self.videodecode0 = CallVideoDecode(1)
        self.videoencode1 = CallVideoEncode(2)
        self.videodecode1 = CallVideoDecode(3)
        self.videoencodebase = CallVideoEncode(4)
        self.videodecodebase = CallVideoDecode(5)
        self.videoencodebase2 = CallVideoEncode(8)
        self.videodecodebase2 = CallVideoDecode(9)
        self.interlace = CallVideoInterlace()
        self.psnr = CallVideoPsnr()
        self.scale_down = CallVideoScale(6)
        self.scale_up = CallVideoScale(7)
    def FrameRateScable(self):
        return
    def ResolutionScable(self):
        return
    def QualityScable(self):
        return
    def FrameCodecTest(self):
        sizelist0 = []
        sizelist1 = []
        sum_psnr0 = 0
        sum_psnr1 = 0
        sum_size0 = 0
        sum_size1 = 0
        yuvfilename = loadlib.yuvfilename #'/data/home/gxh/works/datashare/for_ENC/352X288/foreman_cif.yuv'
        (w, h) = (loadlib.WIDTH, loadlib.HEIGHT)
        frame_size = (w * h * 3) / 2
        fp = open(yuvfilename, 'rb')
        i = 0
        while True:
            data = fp.read(frame_size)
            if len(data) > 0:
                start_time = time.time()
                ###inderlace
                (data0, data1) = self.interlace.interlaceYUV(data, w, h)
                difftime = time.time() - start_time
                showflag = False
                if showflag:
                    frame0 = np.frombuffer(data0, dtype=np.uint8)
                    yuv_I420_0 = frame0.reshape((h * 3) / 4, w)
                    img0 = cv2.cvtColor(yuv_I420_0, cv2.COLOR_YUV2BGR_I420)
                    frame1 = np.frombuffer(data1, dtype=np.uint8)
                    yuv_I420_1 = frame1.reshape((h * 3) / 4, w)
                    img1 = cv2.cvtColor(yuv_I420_1, cv2.COLOR_YUV2BGR_I420)
                    frame = np.vstack([img0, img1])  # vstack #hstack
                    cv2.imshow("frame", frame)
                    #cv2.waitKey(40)
                (data_0, data_1) = ("", "")
                ###encode and deocde
                data_ = data
                (ret, outbuf, outparam) = self.videoencodebase.encodeframe(data, i)
                if ret > 0 and True:
                    sum_size0 += ret
                    data_ = outbuf.raw[:ret]
                    (ret_, outbuf_, outparam_) = self.videodecodebase.decodeframe(data_, ret)
                    #print("outparam_[1]= ", outparam_[1])
                    #print("outparam_[2]= ", outparam_[2])
                    data_ = outbuf_.raw[:ret_]
                ###
                self.videoencode0.resetresolution(w, h >> 1)
                self.videoencode1.resetresolution(w, h >> 1)
                (ret0, outbuf0, outparam0) = self.videoencode0.encodeframe(data0, i)
                (ret1, outbuf1, outparam1) = self.videoencode1.encodeframe(data1, i)
                if ret0 > 0 and True:
                    sum_size1 += ret0
                    sizelist0.append(ret0)
                    data_0 = outbuf0.raw[:ret0]
                    (ret_0, outbuf_0, outparam_0) = self.videodecode0.decodeframe(data_0, ret0)
                    #print("outparam_0[1]= ", outparam_0[1])
                    #print("outparam_0[2]= ", outparam_0[2])
                    data_0 = outbuf_0.raw[:ret_0]
                    data0 = data_0
                if ret1 > 0 and True:
                    sum_size1 += ret1
                    sizelist1.append(ret1)
                    data_1 = outbuf1.raw[:ret1]
                    (ret_1, outbuf_1, outparam_1) = self.videodecode1.decodeframe(data_1, ret1)
                    #print("outparam_1[1]= ", outparam_1[1])
                    #print("outparam_1[2]= ", outparam_1[2])
                    data_1 = outbuf_1.raw[:ret_1]
                    data1 = data_1
                ###anti-interlace
                #(data0, data1) = (data_0, data_1)
                data2 = self.interlace.interlaceYUV2frame(data0, data1, w, h >> 1)
                ###psnr
                (psnr0, psnrY0, psnrU0, psnrV0) = self.psnr.frame_yuv_psnr(data, data_, w, h)
                (psnr1, psnrY1, psnrU1, psnrV1) = self.psnr.frame_yuv_psnr(data, data2, w, h)
                sum_psnr0 += psnr0
                sum_psnr1 += psnr1
                print("(psnr0, psnr1)=", (psnr0, psnr1))
                print("(sum_size0, sum_size1)=", (sum_size0, sum_size1))
                ###
                showflag = True
                if showflag:
                    frame2 = np.frombuffer(data_, dtype=np.uint8)
                    yuv_I420_2 = frame2.reshape((h * 3) / 2, w)
                    frame3 = cv2.cvtColor(yuv_I420_2, cv2.COLOR_YUV2BGR_I420)

                    frame4 = np.frombuffer(data2, dtype=np.uint8)
                    yuv_I420_ = frame4.reshape((h * 3) / 2, w)
                    frame5 = cv2.cvtColor(yuv_I420_, cv2.COLOR_YUV2BGR_I420)
                    frame6 = np.hstack([frame3, frame5])  # vstack #hstack
                    cv2.imshow("frame6", frame6)
                    cv2.waitKey(1)
                i += 1
                ###
                #difftime = time.time() - start_time
                print('{} FrameCodecTest time: {:.3f}ms'.format(time.ctime(), difftime * 1000))
            else:
                break
        avg_psnr0 = sum_psnr0 / i
        avg_psnr1 = sum_psnr1 / i
        print("(avg_psnr0, avg_psnr1)=", (avg_psnr0, avg_psnr1))
        print("(sum_size0, sum_size1)=", (sum_size0 / 1024, sum_size1 / 1024))
    def MultResolutionCodecTest(self):
        sizelist0 = []
        sizelist1 = []
        sum_psnr0 = 0
        sum_psnr1 = 0
        sum_psnr2 = 0
        sum_size0 = 0
        sum_size1 = 0
        sum_size2 = 0
        ###
        cv2.namedWindow("frame", cv2.WINDOW_NORMAL)
        cv2.moveWindow("frame", 10, 10)
        if False:
            h264filename = "test_py.h264"
            try:
                fout = open(h264filename, "wb")
            except:
                print("open file fail !")
                return
        ###
        yuvfilename = loadlib.yuvfilename#'/data/home/gxh/works/datashare/for_ENC/352X288/foreman_cif.yuv'
        (w, h) = (loadlib.WIDTH, loadlib.HEIGHT)
        frame_size = (w * h * 3) / 2
        fp = open(yuvfilename, 'rb')
        i = 0
        while True:
            data = fp.read(frame_size)
            if len(data) > 0:
                start_time = time.time()
                ###downsample
                (ret1, outbuf1, outparam1) = self.scale_down.yuv_down_sample(data, w, h)
                if ret1 <= 0:
                    print("MultResolutionCodecTest: yuv_down_sample error")
                    continue
                data1 = outbuf1.raw[:ret1]
                #
                #data1 = self.scale_down.yuv_down_sample2(data, w, h)
                #
                #difftime = time.time() - start_time
                showflag = False
                if showflag:
                    (w2, h2) = (w >> 1, h >> 1)
                    frame1 = np.frombuffer(data1, dtype=np.uint8)
                    yuv_I420_1 = frame1.reshape((h2 * 3) / 2, w2)
                    img1 = cv2.cvtColor(yuv_I420_1, cv2.COLOR_YUV2BGR_I420)
                    cv2.imshow("img1", img1)
                    cv2.waitKey(0)
                (data_0, data_1) = ("", "")
                ###encode and deocde
                data_ = data
                (ret, outbuf, outparam) = self.videoencodebase.encodeframe(data, i)
                #print("ret= ", ret)
                if ret > 0 and True:
                    sum_size0 += ret
                    data_ = outbuf.raw[:ret]
                    (ret_, outbuf_, outparam_) = self.videodecodebase.decodeframe(data_, ret)
                    #print("outparam_[1]= ", outparam_[1])
                    #print("outparam_[2]= ", outparam_[2])
                    data_ = outbuf_.raw[:ret_]
                #
                data_2 = data
                qp = 26+6+3 #+ 4

                self.videoencodebase2.resetqp(qp)
                (ret, outbuf, outparam) = self.videoencodebase2.encodeframe(data, i)
                # print("ret= ", ret)
                if ret > 0 and True:
                    sum_size2 += ret
                    data_2 = outbuf.raw[:ret]
                    (ret_, outbuf_, outparam_) = self.videodecodebase2.decodeframe(data_2, ret)
                    # print("outparam_[1]= ", outparam_[1])
                    # print("outparam_[2]= ", outparam_[2])
                    data_2 = outbuf_.raw[:ret_]
                ###
                self.videoencode1.resetresolution(w >> 1, h >> 1)
                #qp = 14#20
                #self.videoencode1.resetqp(qp)
                (ret1, outbuf1, outparam1) = self.videoencode1.encodeframe(data1, i)
                #print("ret1= ", ret1)
                if ret1 > 0 and True:
                    sum_size1 += ret1
                    sizelist1.append(ret1)
                    data_1 = outbuf1.raw[:ret1]

                    #fout.write(data_1)
                    #fout.flush()

                    (ret_1, outbuf_1, outparam_1) = self.videodecode1.decodeframe(data_1, ret1)
                    #print("ret_1= ", ret_1)
                    #print("outparam_1[1]= ", outparam_1[1])
                    #print("outparam_1[2]= ", outparam_1[2])
                    data_1 = outbuf_1.raw[:ret_1]
                    data1 = data_1
                ###upsample
                (ret2, outbuf2, outparam2) = self.scale_up.yuv_up_sample(data1, w >> 1, h >> 1)
                print("ret2= ", ret2)
                data2 = outbuf2.raw[:ret2]
                ###psnr
                (psnr0, psnrY0, psnrU0, psnrV0) = self.psnr.frame_yuv_psnr(data, data_, w, h)
                (psnr1, psnrY1, psnrU1, psnrV1) = self.psnr.frame_yuv_psnr(data, data2, w, h)
                (psnr2, psnrY2, psnrU2, psnrV2) = self.psnr.frame_yuv_psnr(data, data_2, w, h)
                sum_psnr0 += psnr0
                sum_psnr1 += psnr1
                sum_psnr2 += psnr2
                print("(psnr0, psnr1)=", (psnr0, psnr1))
                print("(sum_size0, sum_size1)=", (sum_size0, sum_size1))
                ###
                showflag = True
                if showflag:
                    frame2 = np.frombuffer(data_, dtype=np.uint8)
                    yuv_I420_2 = frame2.reshape((h * 3) / 2, w)
                    frame3 = cv2.cvtColor(yuv_I420_2, cv2.COLOR_YUV2BGR_I420)

                    frame7 = np.frombuffer(data_2, dtype=np.uint8)
                    yuv_I420_3 = frame7.reshape((h * 3) / 2, w)
                    frame8 = cv2.cvtColor(yuv_I420_3, cv2.COLOR_YUV2BGR_I420)

                    #frame7 = np.frombuffer(data1, dtype=np.uint8)
                    #yuv_I420_3 = frame7.reshape(((h >> 1) * 3) / 2, w >> 1)
                    #frame8 = cv2.cvtColor(yuv_I420_3, cv2.COLOR_YUV2BGR_I420)

                    frame4 = np.frombuffer(data2, dtype=np.uint8)
                    yuv_I420_ = frame4.reshape((h * 3) / 2, w)
                    frame5 = cv2.cvtColor(yuv_I420_, cv2.COLOR_YUV2BGR_I420)
                    frame6 = np.hstack([frame3, frame5, frame8])  # vstack #hstack
                    #cv2.imshow("frame8", frame8)
                    cv2.imshow("frame", frame6)
                    cv2.waitKey(40)
                i += 1
                ###
                #difftime = time.time() - start_time
                #print('{} FrameCodecTest time: {:.3f}ms'.format(time.ctime(), difftime * 1000))
            else:
                break
        avg_psnr0 = sum_psnr0 / i
        avg_psnr1 = sum_psnr1 / i
        avg_psnr2 = sum_psnr2 / i
        #print("(avg_psnr0, avg_psnr1)=", (avg_psnr0, avg_psnr1))
        #print("(sum_size0, sum_size1)=", (sum_size0 / 1024, sum_size1 / 1024))
        print("(avg_psnr0, sum_size0)=", (avg_psnr0, sum_size0 / 1024))
        print("(avg_psnr1, sum_size1)=", (avg_psnr1, sum_size1 / 1024))
        print("(avg_psnr2, sum_size2)=", (avg_psnr2, sum_size2 / 1024))
        if showflag:
            cv2.waitKey(0)
    def test_call(self):
        yuvfilename0 = loadlib.yuvfilename #'/data/home/gxh/works/datashare/for_ENC/352X288/foreman_cif.yuv'
        h264filename = "test_py.h264"
        sizelist = self.videoencode.test_encode(yuvfilename0, h264filename)
        yuvfilename1 = 'out.yuv'
        h264filename = "test_py.h264"
        self.videodecode.test_decode(yuvfilename1, h264filename, sizelist)
        (avg_psnr, avg_psnrY, avg_psnrU, avg_psnrV) = self.psnr.get_yuv_psnr(yuvfilename0, yuvfilename1, WIDTH, HEIGHT)
        print("(avg_psnr, avg_psnrY, avg_psnrU, avg_psnrV)=", (avg_psnr, avg_psnrY, avg_psnrU, avg_psnrV))
    def test_call2(self):
        yuvfilename = 'out.yuv'
        self.interlace.test_interlace(yuvfilename, loadlib.WIDTH, loadlib.HEIGHT)
        self.scale.test_scale(yuvfilename, loadlib.WIDTH, loadlib.HEIGHT)

if __name__ == '__main__':
    print('Start pycall.')
    call = BCodecCore()
    #call.test_call()
    #call.test_call2()
    #call.FrameCodecTest()
    call.MultResolutionCodecTest()

    print('End pycall.')
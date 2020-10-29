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
import loadlib

#(WIDTH, HEIGHT) = (352, 288)
#src_dir = r'/data/home/gxh/works/datashare/for_ENC/352X288/foreman_cif.yuv'

sys.path.append(".")

class CallVideoInterlace(object):
    def __init__(self):
        self.load = loadlib.gload


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
    def test_call2(self):
        yuvfilename = 'out.yuv'
        self.test_interlace(yuvfilename, loadlib.WIDTH, loadlib.HEIGHT)


if __name__ == '__main__':
    print('Start pycall.')
    call = CallVideoInterlace()
    call.test_call2()

    print('End pycall.')
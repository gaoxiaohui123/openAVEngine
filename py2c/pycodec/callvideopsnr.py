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

(WIDTH, HEIGHT) = (352, 288)
src_dir = r'/data/home/gxh/works/datashare/for_ENC/352X288/foreman_cif.yuv'

sys.path.append(".")

class CallVideoPsnr(object):
    def __init__(self):
        self.load = loadlib.gload

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
    def frame_yuv_psnr(self, data0, data1, W, H):
        (psnr, psnrY, psnrU, psnrV) = (0, 0, 0, 0)
        if len(data0) > 0 and len(data1) > 0:
            frame0 = np.frombuffer(data0, dtype=np.uint8)
            frame1 = np.frombuffer(data1, dtype=np.uint8)
            ###
            size0 = W * H
            size1 = size0 + ((W * H) >> 2)
            size2 = size1 + ((W * H) >> 2)
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
            #print("(psnrY,psnrU,psnrV)= ", (psnrY, psnrU, psnrV))
            psnr = self.psnr1(frame0, frame1)

        return (psnr, psnrY, psnrU, psnrV)
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

    def test_call(self):
        yuvfilename0 = '/data/home/gxh/works/datashare/for_ENC/352X288/foreman_cif.yuv'
        h264filename = "test_py.h264"

        yuvfilename1 = 'out.yuv'
        h264filename = "test_py.h264"
        (avg_psnr, avg_psnrY, avg_psnrU, avg_psnrV) = self.get_yuv_psnr(yuvfilename0, yuvfilename1, WIDTH, HEIGHT)
        print("(avg_psnr, avg_psnrY, avg_psnrU, avg_psnrV)=", (avg_psnr, avg_psnrY, avg_psnrU, avg_psnrV))


if __name__ == '__main__':
    print('Start pycall.')
    call = CallVideoPsnr()
    call.test_call()

    print('End pycall.')
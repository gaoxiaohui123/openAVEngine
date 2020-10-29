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

class PostProcess():
    def __init__(self, mode):
        self.mode = mode
        #创建CLAHE对象
        self.clahe = cv2.createCLAHE(clipLimit=2.0,tileGridSize=(8,8))
    def DoProcess(self, img):
        if self.mode == 0:
            #彩色图均衡化,需要分解通道,对每一个通道进行均衡化
            (b,g,r) = cv2.split(img)
            #限制对比度的自适应直方图均衡化
            bH = self.clahe.apply(b)
            gH = self.clahe.apply(g)
            rH = self.clahe.apply(r)
            #合并每一个通道
            img = cv2.merge((bH,gH,rH))
        elif self.mode == 1:
            img = cv2.GaussianBlur(img,(3,3),0)
            #img = cv2.GaussianBlur(img, (3, 3), 1.8)
            #img = cv2.GaussianBlur(img,(7,7),0)
            #img = cv2.GaussianBlur(img,(15,15),0)
        elif self.mode == 2:
            img = cv2.bilateralFilter(img,40,75,75)
        elif self.mode == 3:
            img = cv2.fastNlMeansDenoisingColored(img, None, 10, 10, 7, 21)
        elif self.mode == 4:
            canny = cv2.Canny(img,30,200)
            img = cv2.merge((canny, canny, canny))
            #print("canny.shape= ", canny.shape)
        elif self.mode == 5:
            a = 1.1#1#2

            O = img * float(a)

            O[O > 255] = 255

            O = np.round(O)

            img = O.astype(np.uint8)

        return img
    def yuv2img(self, data0, w, h):
        frame0 = np.frombuffer(data0, dtype=np.uint8)
        yuv_I420 = frame0.reshape((h * 3) / 2, w)
        img0 = cv2.cvtColor(yuv_I420, cv2.COLOR_YUV2BGR_I420)
        return img0
    def img2yuv(self, img):
        (h, w) = (img.shape[0], img.shape[1])
        img_yuv = cv2.cvtColor(img, cv2.COLOR_BGR2YUV_I420)
        return (img_yuv , w, h)
    def denoise(self, data, w, h):
        frame = self.yuv2img(data, w, h)
        self.mode = 1
        frame = self.DoProcess(frame)
        (yuv, w2, h2) = self.img2yuv(frame)
        return (yuv.data, w2, h2)
    def testProcess(self):
        import loadlib
        yuvfilename = loadlib.yuvfilename  # '/data/home/gxh/works/datashare/for_ENC/352X288/foreman_cif.yuv'
        (w, h) = (loadlib.WIDTH, loadlib.HEIGHT)
        frame_size = (w * h * 3) / 2
        fp = open(yuvfilename, 'rb')
        i = 0
        while True:
            data = fp.read(frame_size)
            if len(data) > 0:
                start_time = time.time()
                frame = self.yuv2img(data, w, h)

                flag = True
                if flag:
                    frame2 = self.DoProcess(frame)
                    (yuv, w2, h2) = self.img2yuv(frame2)
                    print("len(yuv.data)= ", len(yuv.data))
                    #print("len(yuv[1])= ", len(yuv[1]))
                    frame = self.yuv2img(yuv, w, h)
                difftime = time.time() - start_time
                print('{} testProcess time: {:.3f}ms'.format(time.ctime(), difftime * 1000))
                showflag = True  # False
                if showflag:
                    cv2.imshow("frame", frame)
                    cv2.waitKey(40)
if __name__ == '__main__':
    print('Start .')
    call = PostProcess(1)
    call.testProcess()
    print('End .')
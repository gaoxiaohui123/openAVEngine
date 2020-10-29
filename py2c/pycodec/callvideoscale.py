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
from loadlib import LoadLib
import loadlib

#(WIDTH, HEIGHT) = (352, 288)
#src_dir = r'/data/home/gxh/works/datashare/for_ENC/352X288/foreman_cif.yuv'

sys.path.append(".")

class CallVideoScale(object):
    def __init__(self, id):
        self.load = loadlib.gload
        self.obj_id = id
        (self.param, self.outbuf, self.outparam) = self.setparam3()
    def setparam3(self):
        param = {}
        width = loadlib.WIDTH
        height = loadlib.HEIGHT
        w = width << 1
        h = height << 1
        #w = width >> 1
        #h = height >> 1
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
    def yuv_down_sample2(self,data, w, h):
        y = data[:w*h]
        u = data[w*h:(w*h+((w*h) >> 2))]
        v = data[(w*h+((w*h) >> 2)):(w*h+((w*h) >> 1))]
        y = np.frombuffer(y, dtype=np.uint8)
        y = y.reshape(h, w)
        u = np.frombuffer(u, dtype=np.uint8)
        u = u.reshape(h >> 1, w >> 1)
        v = np.frombuffer(v, dtype=np.uint8)
        v = v.reshape(h >> 1, w >> 1)

        y = cv2.resize(y, (w >> 1, h >> 1))
        u = cv2.resize(u, (w >> 2, h >> 2))
        v = cv2.resize(v, (w >> 2, h >> 2))
        buf = y.tobytes() + u.tobytes() + v.tobytes()
        print("yuv_down_sample2: len(buf)= ", len(buf))
        return buf
    def yuv_up_sample(self, data, w, h):
        ret = 0
        frame_size = (w * h * 3) / 2
        if len(data) > 0:
            self.param.update({"insize": frame_size})
            (w2, h2) = (w << 1, h << 1)
            self.param.update({"src_w": w})
            self.param.update({"src_h": h})
            self.param.update({"dst_size": str(w2) + "x" + str(h2)})
            ##self.param.update({"scale_mode": "SWS_SPLINE"})
            param_str = json.dumps(self.param, encoding='utf-8', ensure_ascii=False, indent=4, sort_keys=True)
            ###
            self.outparam[0] = "test1"
            ###
            start_time = time.time()
            ret = self.load.lib.api_ff_scale(self.obj_id, data, param_str, self.outbuf, self.outparam)
        return (ret, self.outbuf, self.outparam)
    def yuv_down_sample(self, data, w, h):
        ret = 0
        frame_size = (w * h * 3) / 2
        if len(data) > 0:
            self.param.update({"insize": frame_size})
            #self.param.update({"scale_mode": "SWS_FAST_BILINEAR"})
            #self.param.update({"scale_mode": "SWS_BICUBIC"})
            (w2, h2) = (w >> 1, h >> 1)
            self.param.update({"dst_size": str(w2) + "x" + str(h2)})
            param_str = json.dumps(self.param, encoding='utf-8', ensure_ascii=False, indent=4, sort_keys=True)
            ###
            self.outparam[0] = "test1"
            ###
            start_time = time.time()
            ret = self.load.lib.api_ff_scale(self.obj_id, data, param_str, self.outbuf, self.outparam)
        return (ret, self.outbuf, self.outparam)
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
                    ret = self.load.lib.api_ff_scale(obj_id, data, param_str, outbuf, outparam)
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
    def test_call2(self):
        yuvfilename = 'out.yuv'
        #call.test_interlace(yuvfilename, WIDTH, HEIGHT)
        self.test_scale(yuvfilename, loadlib.WIDTH, loadlib.HEIGHT)

if __name__ == '__main__':
    print('Start pycall.')
    call = CallVideoScale(0)
    call.test_call2()

    print('End pycall.')
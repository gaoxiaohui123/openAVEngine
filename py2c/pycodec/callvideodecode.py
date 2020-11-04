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

class CallVideoDecode(object):
    def __init__(self, id, width, height):
        self.load = loadlib.gload
        self.width = width
        self.height = height
        self.obj_id = id
        self.handle_size = 8
        self.handle = create_string_buffer(self.handle_size)
        self.min_distance = 2
        self.delay_time = 100
        self.buf_size = 1024
        #
        self.osd = 1
        self.orgX = 0
        self.orgY = 0
        self.scale = 0
        self.color = 4
        (self.param, self.outbuf, self.outparam) = self.setparam2()
        self.param.update({"osd": self.osd})
        self.param.update({"orgX": self.orgX})
        self.param.update({"orgY": self.orgY})
        self.param.update({"scale": self.scale})
        self.param.update({"color": self.color})
        start_time = time.time()
        ret = self.load.lib.api_initobj()
        param_str = json.dumps(self.param, encoding='utf-8', ensure_ascii=False, sort_keys=True)
        ret = self.load.lib.api_video_decode_open(self.handle, param_str)
        print("CallVideoDecode: init: open ret= ", ret)
        print("CallVideoDecode: init: open self.handle= ", self.handle)

        difftime = time.time() - start_time
        print('{} CallVideoDecode: init time: {:.3f}ms'.format(time.ctime(), difftime * 1000))

    def setparam2(self):
        param = {}
        #self.width = loadlib.WIDTH
        #self.height = loadlib.HEIGHT
        param.update({"oneframe": 1})
        self.frame_size = (self.width * self.height * 3) / 2
        outbuf = create_string_buffer(self.frame_size)
        array_type = c_char_p * 4
        outparam = array_type()
        return (param, outbuf, outparam)

    def decodeframe(self, data, insize):
        ret = 0
        if len(data) > 0:
            self.param.update({"insize": insize})
            param_str = json.dumps(self.param, encoding='utf-8', ensure_ascii=False, indent=4, sort_keys=True)
            ###
            self.outparam[0] = "test1"
            # buf = bytearray(frame_size + 64)
            # buf[:frame_size] = data
            # buf[frame_size:] = 0
            ###
            ret = self.load.lib.api_video_decode_one_frame(self.obj_id, data, param_str, self.outbuf, self.outparam)
            #print("encodeframe: self.obj_id= ", self.obj_id)
            #print("outparam[0]= ", self.outparam[0])
        return (ret, self.outbuf, self.outparam)
    def test_decode(self, yuvfilename, h264filename, sizelist):
        #yuvfilename = 'out.yuv'
        #h264filename = "test_py.h264"
        try:
            fout = open(yuvfilename, "wb")
        except:
            print("open file fail !")
            return
        print("start run test_decode")

        (param, outbuf, outparam) = self.setparam2()
        param.update({"insize": 0})
        start_time = time.time()
        param_str = json.dumps(param, encoding='utf-8', ensure_ascii=False, indent=4, sort_keys=True)
        self.load.lib.decode_open2(self.obj_id, param_str)
        difftime = time.time() - start_time
        print('{} test_decode:decode_open2: time: {:.3f}ms'.format(time.ctime(), difftime * 1000))


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
                    start_time = time.time()
                    ret = self.load.lib.api_video_decode_one_frame(self.obj_id, data, param_str, outbuf, outparam)
                    difftime = time.time() - start_time
                    print('{} test_decode time: {:.3f}ms'.format(time.ctime(), difftime * 1000))
                    print("outparam[0]= ", outparam[0])
                    print("ret= ", ret)
                    print("frame_idx= ", frame_idx)
                    data2 = outbuf.raw[:ret]
                    fout.write(data2)
                    fout.flush()
                    frame_idx += 1
    def test_decode2(self, yuvfilename, streamlist0, streamlist1, rtplist0):
        #yuvfilename = 'out.yuv'
        #h264filename = "test_py.h264"
        try:
            fout = open(yuvfilename, "wb")
        except:
            print("open file fail !")
            return
        print("start run test_decode")

        (param, outbuf, outparam) = self.setparam2()
        param.update({"insize": 0})
        start_time = time.time()
        param_str = json.dumps(param, encoding='utf-8', ensure_ascii=False, indent=4, sort_keys=True)
        self.load.lib.decode_open2(self.obj_id, param_str)
        difftime = time.time() - start_time
        print('{} test_decode:decode_open2: time: {:.3f}ms'.format(time.ctime(), difftime * 1000))

        frame_idx = 0
        for i in range(len(streamlist0)):
            data = streamlist0[i]
            ###lost frame
            #if i in [1,2,3,4]:
            #    data = streamlist1[i]
            frame_size = len(data)
            if True:
                if len(data) > 0:
                    #print("len(data)= ", len(data))
                    #print("frame_size= ", frame_size)
                    ##param.update({"insize": frame_size})
                    param_str = json.dumps(param, encoding='utf-8', ensure_ascii=False, indent=4, sort_keys=True)
                    ###
                    outparam[0] = ""
                    outparam[1] = ""
                    #buf = bytearray(frame_size + 64)
                    #buf[:frame_size] = data
                    #buf[frame_size:] = 0
                    ###test rtp
                    (data3, rtpSize) = rtplist0[i]
                    sizelist = []
                    for packet_size in rtpSize:
                        sizelist.append(int(packet_size))
                    param.update({"rtpSize": sizelist})
                    param.update({"mtu_size": 1400})
                    param_str = json.dumps(param, encoding='utf-8', ensure_ascii=False, sort_keys=True)
                    #print("param_str= ", param_str)
                    ret = self.load.lib.api_rtp_packet2raw(self.obj_id, data3, param_str, outbuf, outparam)
                    data4 = outbuf.raw[:ret]
                    param.update({"insize": ret})
                    ##print("test_decode2: (frame_size, ret)=", (frame_size, ret))
                    param_str = json.dumps(param, encoding='utf-8', ensure_ascii=False, sort_keys=True)
                    ###
                    start_time = time.time()
                    ret = self.load.lib.api_video_decode_one_frame(self.obj_id, data4, param_str, outbuf, outparam)
                    #ret = self.load.lib.api_video_decode_one_frame(self.obj_id, data, param_str, outbuf, outparam)
                    difftime = time.time() - start_time
                    print('{} test_decode time: {:.3f}ms'.format(time.ctime(), difftime * 1000))
                    print("outparam[0]= ", outparam[0])
                    print("ret= ", ret)
                    print("frame_idx= ", frame_idx)
                    data2 = outbuf.raw[:ret]
                    fout.write(data2)
                    fout.flush()
                    frame_idx += 1
                    #break

    def test_call(self):
        yuvfilename0 = '/data/home/gxh/works/datashare/for_ENC/352X288/foreman_cif.yuv'
        h264filename = "test_py.h264"
        sizelist = []
        #sizelist = self.test_encode(yuvfilename0, h264filename)
        yuvfilename1 = 'out.yuv'
        h264filename = "test_py.h264"
        self.test_decode(yuvfilename1, h264filename, sizelist)

if __name__ == '__main__':
    print('Start pycall.')
    call = CallVideoDecode(1)
    call.test_call()

    print('End pycall.')
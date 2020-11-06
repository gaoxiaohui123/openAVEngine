# -*- coding: utf-8 -*


import sys
import os
import shutil
import time
#import cv2

#import matplotlib.pyplot as plt # plt 用于显示图片
#from matplotlib import pyplot as plt
#import matplotlib.image as mpimg # mpimg 用于读取图片
import numpy as np
#from PIL import Image
import math
import threading

import ctypes
from ctypes import *
from ctypes import c_longlong as ll
import json
import errno

yuvdir = '/home/gxh/works/datashare/'
inputflag = 3#1#7#4#1#3#1#5#3#2#5#3#1#6#1#3#6#
yuvfilename = yuvdir + 'for_ENC/352X288/foreman_cif.yuv'
(WIDTH, HEIGHT) = (352, 288)

if inputflag == 1:
    yuvfilename = yuvdir + 'InToTree_1920x1080.yuv'
    (WIDTH, HEIGHT) = (1920, 1080)
elif inputflag == 2:
    yuvfilename = yuvdir + 'for_ENC/704X576/plane_704x576.yuv'
    (WIDTH, HEIGHT) = (704, 576)
elif inputflag == 3:
    yuvfilename = yuvdir + 'ConferenceMotion_1280_720_50.yuv'
    (WIDTH, HEIGHT) = (1280, 720)
elif inputflag == 4:
    yuvfilename = yuvdir + 'FourPeople_1280x720_30.yuv'
    (WIDTH, HEIGHT) = (1280, 720)
elif inputflag == 5:
    yuvfilename = yuvdir + "road_640x480.yuv"
    (WIDTH, HEIGHT) = (640, 480)
elif inputflag == 6:
    yuvfilename = yuvdir + "foreman_320x240.yuv"
    (WIDTH, HEIGHT) = (320, 240)
elif inputflag == 7:
    yuvfilename = yuvdir + "foreman_320x240.yuv"
    (WIDTH, HEIGHT) = (360, 240)

h264filename = r'../../mytest/test_py.h264'
sys.path.append(".")

gload = None

class TestThread(threading.Thread):
    def __init__(self, actor, host, port):
        threading.Thread.__init__(self)
        self.actor = actor
        self.host = host
        self.port = port

        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True

        array_type = c_char_p * 4
        self.outparam = array_type()

        self.obj_id = 0
        self.seqnum = 0

    def setparam(self):
        self.param = {}
        self.symbol_size = 1024 #1500 #1024
        #loss_rate = (n - k) / n
        #loss_rate = 1 - (k / n)
        #loss_rate = 1 - code_rate
        #code_rate = (k / n)
        #n = k / code_rate
        self.k = 4 #8 #100 #8 #100
        self.lost_rate = 0.3
        self.code_rate = (1 - self.lost_rate) #0.667		# k/n = 2/3 means we add 50% of repair symbols
        self.total_size = self.symbol_size * self.k * 3
        self.param.update({"symbol_size": self.symbol_size})
        self.param.update({"k": self.k})
        self.param.update({"loss_rate": self.lost_rate})
        self.param.update({"code_rate": self.code_rate})
        self.outbuf = create_string_buffer(self.total_size)
        array_type = c_char_p * 4
        self.outparam = array_type()

    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals

    def run(self):
        count = 0
        while self.__running.isSet():
            self.__flag.wait()   # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            if self.actor == 0:
                print("api_fec_server_main start")
                gload.lib.api_fec_server_main()
                print("api_fec_server_main end")
            elif self.actor == 1:
                print("api_fec_client_main start")
                gload.lib.api_fec_client_main()
                print("api_fec_client_main end")
            else:
                start_time = time.time()
                self.setparam()
                param_str = json.dumps(self.param, encoding='utf-8', ensure_ascii=False,sort_keys=True)
                data = ""
                ret = gload.lib.api_fec_encode(self.obj_id, data, param_str, self.outbuf, self.outparam)
                if ret > 0:
                    print("encode raw size= ", ret)
                    self.seqnum = int(self.outparam[1])
                    pktSize = self.outparam[0].split(",")
                    print("pktSize= ", pktSize)
                    print("len(pktSize)= ", len(pktSize))
                    data2 = self.outbuf.raw[:ret]
                    ###
                    sizelist = []
                    for packet_size in pktSize:
                        sizelist.append(int(packet_size))
                    self.param.update({"inSize": sizelist})
                    param_str = json.dumps(self.param, encoding='utf-8', ensure_ascii=False, sort_keys=True)
                    ret = gload.lib.api_fec_decode(self.obj_id + 1, data2, param_str, self.outbuf, self.outparam)
                    if ret > 0:
                        print("decode raw size= ", ret)
                        pktSize = self.outparam[0].split(",")
                        print("pktSize= ", pktSize)
                        print("len(pktSize)= ", len(pktSize))
                        data3 = self.outbuf.raw[:ret]
                difftime = time.time() - start_time
                print('{} fec encode&decode time: {:.3f}ms'.format(time.ctime(), difftime * 1000))
            print("count= ", count)
            count += 1
            if count > 0:
                break
            time.sleep(0.01)
        print("run exit")


class LoadLib(object):
    def __init__(self):
        ll = ctypes.cdll.LoadLibrary
        #try:
        #    lib = ll("../libpycall.so")
        #except:
        #    lib = ll("../py2c/libpycall.so")
        #ret = lib.foo(1, 3)
        #print("ret", ret)

        #lib.ret_str.restype = c_char_p
        #s = lib.ret_str()
        #print("len(s)= ", len(s))
        #print("s", s)
        ###
        print("load my dll")
        thispath = os.path.abspath('./loadlib.py')
        print("thispath= ", thispath)
        try:
            self.lib = ll("../libhcsvcapi.so")
        except IOError, error: #python2
        #except IOError as error:  # python3
            self.lib = ll("../py2c/libhcsvcapi.so")
        ###
        print("load my dll2")
        try:
            self.lib2 = ll("../libhcsvc.so")
        except:
            self.lib2 = ll("../py2c/libhcsvc.so")
        ret = self.lib2.hcsvc_dll_init("../libhcsvcapi.so")
        print("hcsvc_dll_init: ret= ", ret)
        ###
        filename = 'test-0.h264'
        codec_id = -1
        ##self.lib.APIEncodecTest(filename, codec_id)
        #self.lib.mystr2json("")
        #self.lib.api_video_encode_one_frame.argtypes = [c_char_p]
        #self.lib.api_video_encode_one_frame.restype = c_char_p
        h264filename = r'/data/home/gxh/works/datashare/for_ENC/stream720p/FVDO_Girl_720p.264'
        h264filename = r'/data/home/gxh/works/datashare/jy/mytest/test_py.h264'
        yuvfilename = r'/data/home/gxh/works/datashare/jy/mytest/out.yuv'
        yuvfilename = 'out.yuv'
        h264filename = "test_py.h264"
        #self.lib.APIDecodecTest(yuvfilename, h264filename)
        #self.lib.api_fec_main()
        print("LoadLib ok")
    def test_fec_server(self):
        self.lib.api_fec_server_main();
    def test_fec_client(self):
        self.lib.api_fec_client_main();

    def dlltest_init(self):
        frame_size = (WIDTH * HEIGHT * 3) / 2
        self.outbuf = create_string_buffer(frame_size)
        array_type = c_char_p * 4
        self.outparam = array_type()
        self.obj_id = 0
        self.seqnum = 0  # can be changed by fec
        self.enable_fec = 0#1  # 0#1
        self.refresh_idr = 1
        self.lost_rate = 0.3
        self.code_rate = (1 - self.lost_rate)
        (self.width, self.height) = (WIDTH, HEIGHT)
        self.max_refs = 16
        self.mtu_size = 0#1400 #0 #1400
        self.frame_rate = 25
        self.gop_size = self.frame_rate << 1  # 50
        self.qp = 26
        self.refs = 16#1#16
        self.bit_rate = 700 * 1024 #300000 #640 * 1024  # 524288
        self.param = {}
        ###
        self.param.update({"seqnum": self.seqnum})
        self.param.update({"enable_fec": self.enable_fec})
        self.param.update({"refresh_idr": self.refresh_idr})
        self.param.update({"width": self.width})
        self.param.update({"height": self.height})
        self.param.update({"bit_rate": self.bit_rate})  # 400000
        self.param.update({"gop_size": self.gop_size})
        self.param.update({"preset": "superfast"})  # "superfast" "slow" fast/faster/verfast/superfast/ultrafas
        # param.update({"preset": "slow"})
        self.param.update({"tune": "zerolatency"})
        if self.mtu_size > 0:
            self.param.update({"slice-max-size": str(self.mtu_size)})
            self.param.update({"mtu_size": self.mtu_size})
        self.param.update({"tff": 0})
        self.param.update({"bff": 0})
        self.param.update({"qmin": 20})  # 20
        self.param.update({"qmax": 48})  # 40
        self.param.update({"qp": self.qp})  # 20
        self.param.update({"max_b_frames": 0})
        self.param.update({"coder_type": 2})  # 1:cavlc/2:cabac
        self.param.update({"refs": self.refs})
        self.param.update({"frame_rate": self.frame_rate})
        self.param.update({"thread_type": 1})  # FF_THREAD_FRAME: 1 #FF_THREAD_SLICE: 2
        self.param.update({"thread_count": 1})

        param_str = json.dumps(self.param, encoding='utf-8', ensure_ascii=False, sort_keys=True)
        self.lib2.i2_video_encode_open(self.obj_id, param_str)
        ##
    def dlltest(self):
        frame_size = (WIDTH * HEIGHT * 3) / 2
        self.dlltest_init()
        h264filename = "../../mytest/test_py.h264"
        try:
            fout = open(h264filename, "wb")
        except:
            print("open file fail !")
            return
        ###
        with open(yuvfilename, 'rb') as fp:
            i = 0
            while True:
                data = fp.read(frame_size)
                if len(data) > 0:
                    if i % self.gop_size:
                        I = (i % self.gop_size)
                        j = (I - 1) % (self.refs)
                        # I P0 P1 P2 P0 P1 P2 (ref=3)
                        ref_idx = j + 1
                        if I >= (int(self.gop_size / self.max_refs) * self.max_refs):
                            ref_idx = 1
                        elif (ref_idx & 1) == 0 and ref_idx != self.refs and self.refs > 2:
                            ref_idx = 1
                        elif ref_idx == (self.refs - 1) and (self.refs > 4):
                            ref_idx = 1
                        ref_idx0 = ref_idx
                        #ref_idx = 1
                    else:
                        ref_idx = 0
                        ref_idx0 = ref_idx
                        if i > 0:
                            self.refresh_idr = 0
                            self.param.update({"refresh_idr": self.refresh_idr})
                    self.param.update({"frame_idx": i})
                    self.param.update({"seqnum": self.seqnum})
                    if ((self.refs & 1) == 0) and True:
                        self.param.update({"ref_idx": ref_idx})
                    param_str = json.dumps(self.param, encoding='utf-8', ensure_ascii=False, indent=4, sort_keys=True)
                    ###
                    self.outparam[0] = "test1"
                    ###
                    start_time = time.time()
                    ret = self.lib2.i2_video_encode_one_frame(self.obj_id, data, param_str, self.outbuf, self.outparam)
                    difftime = time.time() - start_time
                    print('{} test_encode one frame time: {:.3f}ms'.format(time.ctime(), difftime * 1000))
                    print("outparam[0]= ", self.outparam[0])
                    # print("len(s)= ", len(s))
                    print("ret= ", ret)
                    if ret > 0:
                        data2 = self.outbuf.raw[:ret]
                        rtpSize = self.outparam[1].split(",")
                        print("Encoder: rtpSize= ", rtpSize)
                        print("Encoder: len(rtpSize)= ", len(rtpSize))
                        sizelist = []
                        for packet_size in rtpSize:
                            sizelist.append(int(packet_size))
                        #self.param.update({"inSize": sizelist})
                        fout.write(data2)
                        fout.flush()
                    i += 1
                else:
                    break
        fp.close()
        fout.close()

gload = LoadLib()
#gload.dlltest()

def TestApi():
    if False:
        task = TestThread(3, "", 8888)
        task.start()
    if True:
        task0 = TestThread(0, "", 8888)
        task1 = TestThread(1, "", 8888)
        task0.start()
        task1.start()
        while idx >= 0 and idx < 4:
            try:
                idx = int(raw_input('please input to exit(eg: 0 ): '))
            except:
                idx = int(input('please input to exit(eg: 0 ): '))
            print("idx= ", idx)
        print("main: start stop...")
        task0.stop()
        task1.stop()
def TestFec():
    idx = int(raw_input('please input to exit(eg: 0 ): '))
    while idx >= 0:
        if idx == 0:
            print("client: idx=", idx)
            gload.test_fec_server()
        else:
            print("client: idx=", idx)
            ret = gload.test_fec_client()
            if ret < 0:
                print("client: ret=", ret)
        idx = int(raw_input('please input to exit(eg: 0 ): '))
def TestDevice():
    gload.lib.api_show_device()
def TestAudio():
    gload.lib.audio_capture()
def TestGetTime():
    handle_size = 8
    ll_handle = create_string_buffer(handle_size)
    ll_handle = (c_char * handle_size)()
    array_type = c_char_p * 1
    ctime = array_type()
    ret = gload.lib.api_get_time2(ll_handle, ctime)
    print("TestGetTime: ret= ", ret)
    time0 = long(ctime[0])
    #print("TestGetTime: s= ", s)
    time.sleep(1)
    ret = gload.lib.api_get_time2(ll_handle, ctime)
    print("TestGetTime: ret= ", ret)
    time1 = long(ctime[0])
    difftime = int(time1 - time0)
    print("TestGetTime: difftime= ", difftime)
    return
    #"1234567890123456"指向某一固定地址，多线程会出现访问竞争
    array_type = c_char_p * 1
    outparam = array_type()
    tmpbuf =  (c_ubyte * 16)()
    tmpbuf = bytearray(16)
    tmpbuf = "1234567890123456"
    outparam[0] = c_char_p(tmpbuf)
    ret = gload.lib.api_get_time(outparam)
    print("TestGetTime: ret= ", ret)
    time0 = long(outparam[0])
    print("TestGetTime: time0= ", time0)
    time.sleep(1)
    gload.lib.api_get_time(outparam)
    time1 = long(outparam[0])
    print("TestGetTime: time1= ", time1)
    difftime = int(time1 - time0)
    print("TestGetTime: difftime= ", difftime)
def TestGetTime2():
    #"1234567890123456"指向某一固定地址，多线程会出现访问竞争
    array_type = c_char_p * 1
    outparam = array_type()
    tmpbuf =  (c_ubyte * 16)()
    tmpbuf = bytearray(16)
    tmpbuf = "1234567890123456"
    databuf = create_string_buffer(16)
    tmpbuf = databuf.raw
    outparam[0] = c_char_p(tmpbuf)
    ret = gload.lib.api_get_time(outparam)
    print("TestGetTime: ret= ", ret)
    time0 = long(outparam[0])
    print("TestGetTime: time0= ", time0)
    time.sleep(1)
    gload.lib.api_get_time(outparam)
    time1 = long(outparam[0])
    print("TestGetTime: time1= ", time1)
    difftime = int(time1 - time0)
    print("TestGetTime: difftime= ", difftime)
def TestPcm2Wav():
    srcfile = "/home/gxh/works/aac_decode_0.pcm"
    dstfile = "/home/gxh/works/aac_decode_0.wav"

    srcfile = "/home/gxh/works/cap_1.pcm"
    dstfile = "/home/gxh/works/cap_1.wav"

    srcfile = "/home/gxh/works/cap_resample_1.pcm"
    dstfile = "/home/gxh/works/cap_resample_1.wav"

    srcfile = "/home/gxh/works/play_0.pcm"
    dstfile = "/home/gxh/works/play_0.wav"

    channels = 2
    bits = 16
    sample_rate = 48000
    ret = gload.lib.api_pcm2wav(srcfile, dstfile, channels, bits, sample_rate)
def TestCmd():
    read_size = 10240
    cmd = "ls /dev | grep video"
    cmd = "ls /dev | grep fb"
    cmd = "arecord -l | grep card"
    cmd = "arecord -l | grep device"
    cmd = "nohup ffmpeg -f v4l2 -list_formats all -i /dev/video0"
    cmd = "nohup chmod 0777 /dev/fb0"
    cmd = "ps -A|grep pulseaudio"
    cmd = "./ffmpeg -y -f alsa -i hw:0,0 -t 30 out.wav"
    outbuf = create_string_buffer(read_size)

    #ret = gload.lib.api_get_cmd(cmd, outbuf, read_size)
    #return

    cmd = "select_video_capture"
    #cmd = "select_audio_recorder"
    ret = gload.lib.api_get_dev_info(cmd, outbuf)
    if ret > 0:
        data = outbuf.raw
        data2 = data[0:ret]
        #print("outbuf= ", outbuf.raw)
        print("data2= ", data2)
        print("ret= ", ret)

        try:
            result = json.loads(data2, encoding='utf-8')
        except:
            result = json.loads(data2)
        else:
            infolist = []
            #for key, value in result.items():
            for key in result.keys():
                #print("key=", key)
                value = result[key]
                valuelist = []
                if isinstance(value, dict):
                    keys = value.keys()
                    #print("keys=", keys)
                    for key2 in keys:
                        value2 = value[key2]
                        rlist2 = []
                        rlist = value2.split(' ')
                        for thisdata in rlist:
                            if "x" in thisdata:
                                items = thisdata.split('x')
                                value2 = (int(items[0]), int(items[1]))
                                rlist2.append(value2)

                        key2 = key2.replace(" ", "")
                        valuelist.append((key2, rlist2))
                else:
                    value = value.replace(" ", "")
                    items = value.split('x')
                    value2 = (int(items[0]), int(items[1]))
                    valuelist.append(value2)

                key = key.replace(" ", "")
                infolist.append((key, valuelist))
            for item in infolist:
                if "video" in item[0]:
                    if len(item[1]) > 0:
                        for item2 in item[1]:
                            if "raw" in item2[0]:
                                i = infolist.index(item)
                                j = item[1].index(item2)
                                #del(infolist[i][j])
                                del (item[1][j])

            print("infolist=", infolist)

    #return

    outbuf = create_string_buffer(read_size)
    cmd = "select_audio_recorder"
    ret = gload.lib.api_get_dev_info(cmd, outbuf)
    if ret > 0:
        data = outbuf.raw
        data2 = data[0:ret]
        # print("outbuf= ", outbuf.raw)
        print("data2= ", data2)
        print("ret= ", ret)
        try:
            result = json.loads(data2, encoding='utf-8')
        except:
            result = json.loads(data2)
        else:
            infolist = []
            for key, value in result.items():
                if ("card" in key) and ("device" in value):
                    key = key.replace("card", "")
                    value = value.replace("device", "")
                    key = key.replace(" ", "")
                    value = value.replace(" ", "")
                    infolist.append("hw:" + key + "," + value)
            print("infolist=", infolist)

if __name__ == '__main__':
    print('Start pycall.')
    #TestApi()
    #TestDevice()
    TestGetTime2()
    #TestAudio()
    ##TestPcm2Wav()
    #TestCmd()
    print('End pycall.')



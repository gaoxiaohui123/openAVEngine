# -*- coding: utf-8 -*


import sys
import os
import io
import shutil
import time

# import matplotlib.pyplot as plt # plt 用于显示图片
# from matplotlib import pyplot as plt
# import matplotlib.image as mpimg # mpimg 用于读取图片
#import numpy as np
# from PIL import Image
import math
import threading
import queue
import inspect
import platform


import ctypes
from ctypes import *
from ctypes import c_longlong as ll
import json
import errno
import struct
import binascii
import numpy as np
#import cv2
import random

#sys.stdout = io.TextIOWrapper(sys.stdout.buffer,encoding='utf-8')
MAX_RESORT_FRAME_NUM = 3#10#20
CONFIG_FILENAME = "config-16w.json"

yuvdir = '/home/gxh/works/datashare/'
if (platform.system() == 'Windows'):
    yuvdir = r'C:\\Users\\86139\\winshare'
inputflag = 1#6  # 4#2#1
yuvfilename = yuvdir + 'foreman_cif.yuv'
(WIDTH, HEIGHT) = (352, 288)

if inputflag == 1:
    yuvfilename = os.path.join(yuvdir, 'InToTree_1920x1080.yuv')
    #yuvfilename = r'C:\\Users\\86139\\winshare\\InToTree_1920x1080.yuv'
    (WIDTH, HEIGHT) = (1920, 1080)
elif inputflag == 2:
    yuvfilename = os.path.join(yuvdir, 'ConferenceMotion_1280_720_50.yuv')
    (WIDTH, HEIGHT) = (1280, 720)
elif inputflag == 3:
    yuvfilename = os.path.join(yuvdir, 'FourPeople_1280x720_30.yuv')
    (WIDTH, HEIGHT) = (1280, 720)
elif inputflag == 4:
    yuvfilename = os.path.join(yuvdir, 'plane_704x576.yuv')
    (WIDTH, HEIGHT) = (704, 576)
elif inputflag == 5:
    yuvfilename = os.path.join(yuvdir, "road_640x480.yuv")
    (WIDTH, HEIGHT) = (640, 480)
elif inputflag == 6:
    yuvfilename = os.path.join(yuvdir, "foreman_cif.yuv")
    (WIDTH, HEIGHT) = (352, 288)
elif inputflag == 7:
    yuvfilename = os.path.join(yuvdir, "foreman_320x240.yuv")
    (WIDTH, HEIGHT) = (320, 240)

h264filename = r'../../mytest/test_py.h264'

outfilename = r'/home/gxh/works/out.yuv'
streamfilename = r'/home/gxh/works/out.264'
if (platform.system() == 'Windows'):
    outfilename = r'C:\works\test\out.yuv'
    streamfilename = r'C:\works\test\out.264'
encodec_name = "h264_vaapi"
decodec_name = "h264_qsv"

if len(sys.argv) > 1:
    streamfilename = r'/home/gxh/works/out.265'
    encodec_name = "hevc_vaapi"
    decodec_name = "hevc_qsv"


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
        self.symbol_size = 1024  # 1500 #1024
        # loss_rate = (n - k) / n
        # loss_rate = 1 - (k / n)
        # loss_rate = 1 - code_rate
        # code_rate = (k / n)
        # n = k / code_rate
        self.k = 4  # 8 #100 #8 #100
        self.lost_rate = 0.3
        self.code_rate = (1 - self.lost_rate)  # 0.667		# k/n = 2/3 means we add 50% of repair symbols
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
            self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
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
                param_str = json.dumps(self.param, encoding='utf-8', ensure_ascii=False, sort_keys=True)
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

class StructPointer(ctypes.Structure):
    _fields_ = [("name", ctypes.c_char * 32),
                ("complete", ctypes.c_char_p * MAX_RESORT_FRAME_NUM),
                ("frame_num", ctypes.c_int),
                ("loss_rate", ctypes.c_int),
                ("frame_timestamp", ctypes.c_longlong * MAX_RESORT_FRAME_NUM),
                ("frame_size", ctypes.c_int * MAX_RESORT_FRAME_NUM),
                ("pkt_num", ctypes.c_int * MAX_RESORT_FRAME_NUM),
                ("rtp_size", ctypes.c_short * (MAX_RESORT_FRAME_NUM * (1 << 14)))]

class SessInfoPointer(ctypes.Structure):
    _fields_ = [("note", ctypes.c_char * 256),
                ("sessionId", ctypes.c_int),
                ("status", ctypes.c_int),
                ("modeId", ctypes.c_int),
                ("avtype", ctypes.c_int)]

class LoadLib(object):
    def __init__(self):
        ll = ctypes.cdll.LoadLibrary
        #ll = ctypes.CDLL
        if False:
            try:
                lib = ll("./libpycall.dll")
            # except IOError, error: #python2
            except IOError as error:  # python3
                print("LoadLib: error=", error)
            else:
                print("LoadLib: libpycall.dll is loaded ##############")
                #ret = lib.foo(1, 3)
                #print("ret", ret)

        # lib.ret_str.restype = c_char_p
        # s = lib.ret_str()
        # print("len(s)= ", len(s))
        # print("s", s)
        ###
        #import pip._internal
        #print(pip._internal.pep425tags.get_supported())
        print("executable path= ", os.path.dirname(sys.executable))
        print("中文乱码")
        print("load my dll")
        #print("sys.path= ", sys.path)
        thispath = os.path.abspath('./loadlib.py')
        print("thispath= ", thispath)
        current_file_name = inspect.getfile(inspect.currentframe())
        print("current_file_name= ", current_file_name)
        current_working_dir = os.getcwd()
        print("current_working_dir=", current_working_dir)
        print(platform.system())
        dllfile = os.path.join(current_file_name + "/../", "libhcsvcapi.so")
        if (platform.system() == 'Windows'):
            print('Windows系统')
            #ll = ctypes.windll.LoadLibrary
            #ll = ctypes.WinDLL
            dllfile = os.path.join(current_working_dir, "libhcsvcapi.dll")
            #dllfile = r"d:/msys64/home/gxh/works/huichang/svc_codec/bizconf_svc_codec/py2c/pycodec/libhcsvcapi.dll"
            if False:
                file1 = "c:/WINDOWS/SYSTEM32/ntdll.dll"
                if os.path.exists(file1):
                    print("file1= ", file1)
                    try:
                        self.lib = ll(file1)
                    # except IOError, error: #python2
                    except IOError as error:  # python3
                        print("LoadLib: error=", error)
                    else:
                        print("load ok: ", file1)
                file2 = "c:/WINDOWS/SYSTEM32/KERNEL32.dll"
                if os.path.exists(file2):
                    print("file2= ", file2)
                    try:
                        self.lib = ll(file2)
                    # except IOError, error: #python2
                    except IOError as error:  # python3
                        print("LoadLib: error=", error)
                    else:
                        print("load ok: ", file2)
                file3 = "c:/WINDOWS/SYSTEM32/KERNELBASE.dll"
                if os.path.exists(file3):
                    print("file3= ", file3)
                    try:
                        self.lib = ll(file3)
                    # except IOError, error: #python2
                    except IOError as error:  # python3
                        print("LoadLib: error=", error)
                    else:
                        print("load ok: ", file3)
                cmd = "cp " + file1 + " " + current_working_dir + "/"
                print("cmd= ", cmd)
                os.system(cmd)
                cmd = "cp " + file2 + " " + current_working_dir + "/"
                os.system(cmd)
                cmd = "cp " + file3 + " " + current_working_dir + "/"
                os.system(cmd)
                sys.path.append(current_working_dir)
        elif(platform.system() == 'Linux'):
            print('Linux系统')
            dllfile = os.path.join(current_working_dir, "libhcsvcapi.so")
        else:
            print('其他')

        print("dllfile= ", dllfile)
        is_exists = os.path.exists(dllfile)
        print("is_exists= ", is_exists)
        if is_exists:
            try:
                self.lib = ll(dllfile)
                # self.lib = ll("./libhcsvcapi.dll")
                ##self.lib = ll("./libhcsvcapi.so")
            #except IOError, error: #python2
            except IOError as error:  # python3
                print("LoadLib: error=", error)
                print("load failed !!!")
                return
        self.lib.api_resort_packet_all.restype = ctypes.POINTER(StructPointer)
        self.lib.api_get_sessionInfo.restype = ctypes.POINTER(SessInfoPointer)
        self.lib.api_ffmpeg_register()

        print("load sucess !!!")
        ###
        # print("load my dll2")
        # try:
        #    self.lib2 = ll("../libhcsvc.so")
        # except:
        #    self.lib2 = ll("../py2c/libhcsvc.so")
        # ret = self.lib2.hcsvc_dll_init("../libhcsvcapi.so")
        # print("hcsvc_dll_init: ret= ", ret)
        ###
        filename = 'test-0.h264'
        codec_id = -1
        ##self.lib.APIEncodecTest(filename, codec_id)
        # self.lib.api_video_encode_one_frame.argtypes = [c_char_p]
        # self.lib.api_video_encode_one_frame.restype = c_char_p
        h264filename = r'/data/home/gxh/works/datashare/for_ENC/stream720p/FVDO_Girl_720p.264'
        h264filename = r'/data/home/gxh/works/datashare/jy/mytest/test_py.h264'
        yuvfilename = r'/data/home/gxh/works/datashare/jy/mytest/out.yuv'
        yuvfilename = 'out.yuv'
        h264filename = "test_py.h264"
        # self.lib.APIDecodecTest(yuvfilename, h264filename)
        # self.lib.api_fec_main()
        print("LoadLib ok")

    def test_fec_server(self):
        self.lib.api_fec_server_main();

    def test_fec_client(self):
        self.lib.api_fec_client_main();

    def dlltest_init(self):
        print("load my dll2")
        try:
            self.lib2 = ll("../libhcsvc.so")
        except:
            self.lib2 = ll("../py2c/libhcsvc.so")
        ret = self.lib2.hcsvc_dll_init("../libhcsvcapi.so")
        print("dlltest_init: ret= ", ret)
        frame_size = (WIDTH * HEIGHT * 3) / 2
        self.outbuf = create_string_buffer(frame_size)
        array_type = c_char_p * 4
        self.outparam = array_type()
        self.obj_id = 0
        self.seqnum = 0  # can be changed by fec
        self.enable_fec = 0  # 1  # 0#1
        self.refresh_idr = 1
        self.lost_rate = 0.3
        self.code_rate = (1 - self.lost_rate)
        (self.width, self.height) = (WIDTH, HEIGHT)
        self.max_refs = 16
        self.mtu_size = 0  # 1400 #0 #1400
        self.frame_rate = 25
        self.gop_size = self.frame_rate << 1  # 50
        self.qp = 26
        self.refs = 16  # 1#16
        self.bit_rate = 700 * 1024  # 300000 #640 * 1024  # 524288
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
                        # ref_idx = 1
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
                        # self.param.update({"inSize": sizelist})
                        fout.write(data2)
                        fout.flush()
                    i += 1
                else:
                    break
        fp.close()
        fout.close()


gload = LoadLib()


# gload.dlltest()

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
    gload.lib.api_list_device()


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
    time0 = int(ctime[0])
    # print("TestGetTime: s= ", s)
    time.sleep(1)
    ret = gload.lib.api_get_time2(ll_handle, ctime)
    print("TestGetTime: ret= ", ret)
    time1 = int(ctime[0])
    difftime = int(time1 - time0)
    print("TestGetTime: difftime= ", difftime)
    return
    # "1234567890123456"指向某一固定地址，多线程会出现访问竞争
    array_type = c_char_p * 1
    outparam = array_type()
    tmpbuf = (c_ubyte * 16)()
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
    # "1234567890123456"指向某一固定地址，多线程会出现访问竞争
    #gload.lib.api_get_time_stamp_ll()
    array_type = c_char_p * 1
    outparam = array_type()
    tmpbuf = (c_ubyte * 16)()
    tmpbuf = bytearray(16)
    tmpbuf = "1234567890123456"
    databuf = create_string_buffer(16)
    tmpbuf = databuf.raw
    outparam[0] = c_char_p(tmpbuf)
    ret = gload.lib.api_get_time(outparam)
    print("TestGetTime: ret= ", ret)
    if sys.version_info >= (3, 0):
        time0 = int(outparam[0])
    else:
        time0 = long(outparam[0])
    print("TestGetTime: time0= ", time0)
    time.sleep(1)
    gload.lib.api_get_time(outparam)
    if sys.version_info >= (3, 0):
        time1 = int(outparam[0])
    else:
        time1 = long(outparam[0])
    print("TestGetTime: time1= ", time1)
    difftime = int(time1 - time0)
    print("TestGetTime: difftime= ", difftime)


def TestGetCpuInfo():
    array_type = c_char_p * 1
    outparam = array_type()
    databuf = create_string_buffer(1024)
    tmpbuf = databuf.raw
    outparam[0] = c_char_p(tmpbuf)
    ret = gload.lib.api_get_cpu_info(outparam)
    infolist = outparam[0].split(b';')
    print("TeseGetCpuInfo: infolist=", infolist)
    print("TeseGetCpuInfo: outparam[0]=", outparam[0])


def TestWriteStr():
    t0 = time.time()
    gload.lib.api_test_write_str(1024)
    t1 = time.time()
    difftime = (t1 - t0) * 100
    print("TestWriteStr: difftime= ", difftime)


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

    # ret = gload.lib.api_get_cmd(cmd, outbuf, read_size)
    # return

    cmd = "select_video_capture"
    # cmd = "select_audio_recorder"
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
            # for key, value in result.items():
            for key in result.keys():
                # print("key=", key)
                value = result[key]
                valuelist = []
                if isinstance(value, dict):
                    keys = value.keys()
                    # print("keys=", keys)
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
                                # del(infolist[i][j])
                                del (item[1][j])

            print("infolist=", infolist)

    # return

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
def TestDict():
    tmpDict = {}
    sessionId = 1
    actor = 2
    data1 = {"test1": "a", "test2": "b"}
    data2 = {"test1": "c", "test2": "d"}
    tmpDict[sessionId] = {actor: data1}
    #tmpDict.update({sessionId: {actor: data1}})
    #tmpDict.update({sessionId: {actor: data2}})
    #tmpDict[sessionId] = {actor: data1}
    tmpDict.update({"key0":"value0"})
    tmpDict.update({"key1": "value1"}) #key不同，追加
    tmpDict.update({"key0": "value2"}) #key相同，覆盖
    tmpDict[3] = {1234:0}
    tmpDict[3].update({2345:1})
    tmpDict[3].update({4567: 2})
    print("TestDict: tmpDict=", tmpDict)
    pairs = tmpDict.get(3)
    for key,value in pairs.items():
        print((key, value))
def TestCreateId():
    rangesize = 170
    retList = []
    count = 0
    for i in range(rangesize):
        id = gload.lib.api_create_id(rangesize)
        if id in retList:
            print("TestCreateId: (i,id)=", (i,id))
            count += 1
        else:
            retList.append(id)
        print("TestCreateId: count=", count)
        time.sleep(0.1)
def TestGetList():
    testlist = []
    testlist.append(1)
    testlist.append(2)
    testlist.append(3)
    ret = testlist
    #del testlist
    testlist = []
    print("ret= ", ret)
    print("testlist= ", testlist)
def TestQueu():
    q = queue.PriorityQueue(10)
    data = (1,2,3)
    q.put(data)
    q.put(data)
    q.put(data)
    size = q.qsize()
    print(size)
    isfull = q.full()
    print(isfull)
    ret = []
    ret.append(q.get())
    ret.append(q.get())
    ret.append(q.get())
    if not q.empty():
        ret.append(q.get())
    q.join() #等待空
    #q.task_done()
    print(ret)
class CmdHead(object):
    def __init__(self):
        self.idx = random.randint(0, (1 << 15))
        idinfo = 0
        seqnum = 0
        timestamp = time.time()
        values = (idinfo, seqnum, timestamp)
        self.headstruct = struct.Struct('2Id')
        self.headbuf = ctypes.create_string_buffer(self.headstruct.size)
        print("CmdHead: self.headstruct.size= ", self.headstruct.size)
        self.headstruct.pack_into(self.headbuf, 0, *values)
    def packobj(self, data):
        idinfo = data[0]
        seqnum = data[1]
        timestamp = data[2]
        values = (idinfo, seqnum, timestamp)
        self.headstruct.pack_into(self.headbuf, 0, *values)
        return self.headbuf.raw
    def packdata(self, data):
        ret = self.headbuf.raw + data
        return ret
    def unpackdata(self, data):
        unpacked = self.headstruct.unpack_from(data, 0)
        data2 = data[self.headstruct.size:]
        ret = (unpacked, data2)
        return ret
    def show_yuv(self, data, w, h):
        import cv2
        ret = 1
        if True:  # id in [20]:
            frame0 = np.frombuffer(data, dtype=np.uint8)
            yuv_I420_0 = frame0.reshape(int((h * 3) >> 1), w)
            frame = cv2.cvtColor(yuv_I420_0, cv2.COLOR_YUV2BGR_I420)
            cv2.imshow("show_yuv" + str(self.idx), frame)
            #cv2.waitKey(1)
            keydown = cv2.waitKey(1) & 0xFF
            if keydown == ord('q'):
                ret = 0
            elif keydown == ord('b'):
                ret = -1
        return ret
class Render(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.idx = random.randint(0, (1 << 15))
        self.lock = threading.Lock()
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
        self.start_time = 0
        self.frame_num = 0
        self.fps = 25
        self.imgDict = {}
        self.imgList = []
        self.w = 1280
        self.h = 720
    def show_yuv(self, data, w, h):
        import cv2
        ret = 1
        if True:  # id in [20]:
            frame0 = np.frombuffer(data, dtype=np.uint8)
            yuv_I420_0 = frame0.reshape(int((h * 3) >> 1), w)
            frame = cv2.cvtColor(yuv_I420_0, cv2.COLOR_YUV2BGR_I420)
            cv2.imshow("show_yuv" + str(self.idx), frame)
            #cv2.waitKey(1)
            keydown = cv2.waitKey(1) & 0xFF
            if keydown == ord('q'):
                ret = 0
            elif keydown == ord('b'):
                ret = -1
        return ret
    def yuv2bgr(self, data, w, h):
        import cv2
        frame0 = np.frombuffer(data, dtype=np.uint8)
        yuv_I420_0 = frame0.reshape(int((h * 3) >> 1), w)
        frame = cv2.cvtColor(yuv_I420_0, cv2.COLOR_YUV2BGR_I420)
        return frame
    def multshow(self, imglist):
        import cv2
        ret = 1
        n = len(imglist)
        m = int(math.log(n, 2) + 0.99)
        print("multshow: m= ", m)
        k = 0
        imgs = None
        imgs1 = []
        (h, w, _) = imglist[0][0].shape
        blank = np.zeros((h, w, 3), np.uint8)
        for i in range(m):
            imgs0 = []
            for j in range(m):
                if k < n:
                    imgs0.append(imglist[k][0])
                else:
                    imgs0.append(blank)
                k += 1
            print("multshow: len(imgs0)= ", len(imgs0))
            imgs = np.hstack(imgs0)
            imgs1.append(imgs)
        imgs = np.vstack(imgs1)
        frame = cv2.resize(imgs, (self.w, self.h), interpolation=cv2.INTER_CUBIC)
        cv2.imshow("multshow", frame)
        # cv2.waitKey(1)
        keydown = cv2.waitKey(0) & 0xFF
        if keydown == ord('q'):
            ret = 0
        elif keydown == ord('b'):
            ret = -1
        return ret
    def data2show(self):
        imglist = self.PopFrame()
        imgs = []
        for (data, w, h) in imglist:
            frame = self.yuv2bgr(data, w, h)
            imgs.append((frame, w, h))
        ret = self.multshow(imgs)
        return ret
    def PushFrame(self, idx, data, w, h):
        self.lock.acquire()
        if self.imgDict.get(idx) == None:
            self.imgDict.update({idx:[(data, w, h)]})
        else:
            self.imgDict[idx].append((data, w, h))
        self.lock.release()
    def PopFrame(self):
        ret = []
        self.lock.acquire()
        size = len(self.imgDict.keys())
        if size > len(self.imgList):
            self.imgList = [None for x in range(size)]
        i = 0
        for key,value in self.imgDict.items():
            if len(value) > 0:
                self.imgList[i] = value[0]
                del value[0]
            i += 1
        ret = self.imgList
        self.lock.release()
        return ret
    def run(self):
        interval = int(1000 / self.fps)
        while self.__running.isSet():
            self.__flag.wait()
            now_time = time.time()
            ###
            ###
            self.frame_num += 1
            if self.start_time == 0:
                self.start_time = now_time
            difftime = now_time - self.start_time
            difftime = int(1000 * difftime)
            sumtime = interval * self.frame_num
            wait_time = sumtime - difftime
            if wait_time > 0:
                wait_time = float(wait_time / 1000.0)
                time.sleep(wait_time)
            else:
                pass

    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals


    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞
def TestStruct():
    seqnum = 0
    timestamp = time.time()
    values = (seqnum, timestamp)
    s = struct.Struct('Id')
    s2 = struct.Struct('1400B')
    print("s.size= ", s.size)
    print("s2.size= ", s2.size)
    #prebuffer = ctypes.create_string_buffer(s.size + s2.size)
    prebuffer = ctypes.create_string_buffer(s.size)
    print('Before :', binascii.hexlify(prebuffer))
    ###
    cmdheader = CmdHead()
    render = Render()
    ###
    (w, h) = (WIDTH, HEIGHT)
    filename = yuvfilename
    fp = open(filename, 'rb')
    if fp != None:
        i = 0
        framesize = (WIDTH * HEIGHT * 3) >> 1
        while True:
            data = fp.read(framesize)
            if len(data) != framesize:
                break
            if False:
                idx = i % 15
                if i > 0 and idx == 0:
                    ret = render.data2show()
                    if ret == 0:
                        break
                render.PushFrame(idx, data, w, h)

                i += 1
                continue
            ###
            idinfo = ((1 << 8) | 8)
            seqnum = 1
            timestamp = time.time()
            values = (idinfo, seqnum, timestamp)
            cmdheader.packobj(values)
            data2 = cmdheader.packdata(data)
            (r1, r2) = cmdheader.unpackdata(data2)
            print("r1= ", r1)
            print("len(r2)= ", len(r2))
            ret = cmdheader. show_yuv(r2, WIDTH, HEIGHT)
            if ret == 0:
                break
            time.sleep(1)
            continue
            s.pack_into(prebuffer, 0, *values)
            print("len(prebuffer)= ", len(prebuffer))
            #print("prebuffer.raw= ", prebuffer.raw)
            data2 = prebuffer.raw + data
            print ("len(data2)= ", len(data2))
            unpacked = s.unpack_from(data2, 0)
            #a = binascii.hexlify(prebuffer)
            #print("len(a)= ", len(a))
            #values2 = (data)
            #s2.pack_into(prebuffer, s.size, *values2)
            #unpacked = s.unpack_from(prebuffer, 0)
            print('After unpack:', unpacked)
            print("data2= ", data2)
            #unpacked2 = s2.unpack_from(prebuffer, s.size)
            break
    #a = s.pack_into(prebuffer, 0, *values)
    #print("a= ", a)
    #print('After pack:', binascii.hexlify(prebuffer))
    #unpacked = s.unpack_from(prebuffer, 0)
    #print('After unpack:', unpacked)
def TestUDP():
    #gload.lib.pool_main()
    #gload.lib.pool_test(10088, 0)
    gload.lib.pool_test(10089, 1)
def TestSWEncode():
    start_time = time.time()
    #frame_num = 100 * 50
    frame_num = 250
    streamfilename2 = ""
    gload.lib.api_sw_encode_test(   yuvfilename.encode('utf-8'),
                                    streamfilename.encode('utf-8'),
                                    int(WIDTH), int(HEIGHT),
                                    -100,
                                    frame_num)
    end_time = time.time()
    difftime = float((end_time - start_time) * 1000)
    avgtime0 = difftime / frame_num
    print("api_sw_encode_test: avgtime0(ms)= ", avgtime0)
    #gload.lib.api_vaapi_decode_test()
def TestHWEncode():
    start_time = time.time()
    frame_num = 100 * 50
    streamfilename2 = ""
    ret = gload.lib.api_vaapi_encode_test(  yuvfilename.encode('utf-8'),
                                            streamfilename2.encode('utf-8'),
                                            int(WIDTH), int(HEIGHT),
                                            decodec_name.encode('utf-8'),
                                            frame_num)
    end_time = time.time()
    difftime = float((end_time - start_time) * 1000)
    if ret > 0:
        avgtime0 = difftime / frame_num
        print("api_vaapi_encode_test: avgtime0(ms)= ", avgtime0)
    else:
        return

    return

    start_time = time.time()
    ret = gload.lib.api_vaapi_decode_test(  streamfilename.encode('utf-8'),
                                            outfilename.encode('utf-8'),
                                            decodec_name.encode('utf-8'),
                                            0)
    end_time = time.time()
    difftime = float((end_time - start_time) * 1000)
    avgtime1 = difftime / ret
    print("api_vaapi_encode_test: avgtime0(ms)= ", avgtime0)
    print("api_vaapi_decode_test: avgtime1(ms)= ", avgtime1)
def TestCJSON():
    from ctypes import c_longlong as ll
    loopn = (int(1) << 10)
    print("loopn= ", loopn)
    offset = 0
    for i in range(200):
        gload.lib.api_mem_lead_cjson(ll(offset), ll(loopn))
        print("offset= ", offset)
        time.sleep(1)
        offset += loopn
if __name__ == '__main__':
    print('Start pycall.')
    # TestApi()
    # TestDevice()
    #TestGetTime2()
    # TestGetCpuInfo()
    #TestWriteStr()
    # TestAudio()
    ##TestPcm2Wav()
    # TestCmd()
    #TestDict()
    #TestCreateId()
    #TestGetList()
    #TestQueu()
    #TestStruct()
    #datalis = [1,2,3]
    #del datalis[0]
    #del datalis[0]
    #del datalis[0]
    #print(datalis)
    #TestDevice()
    #TestUDP()
    #TestSWEncode()
    TestCJSON()
    #TestHWEncode()
    print('End pycall.')

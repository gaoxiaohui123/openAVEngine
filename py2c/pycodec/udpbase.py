#!/usr/bin/env python
# -*- coding:utf8 -*-

import sys

# for python2
if sys.version_info >= (3, 0):
    pass
else:
    try:
        print (sys.getdefaultencoding())
        reload(sys)
        sys.setdefaultencoding('utf-8')
        print (sys.getdefaultencoding())
    except:
        pass

import os
import socket
import threading
#import queue
import time
import signal
import json
#from ctypes import *
import ctypes
from ctypes import *
import struct
import binascii
from ctypes import c_longlong as ll
import random
import numpy as np
import errno
import platform
#import cv2
import math
import copy
from multiprocessing import  Process
from multiprocessing import Pool
import loadserver
if sys.version > '3':
    import queue
else:
    import Queue as queue

if (platform.system() == 'Windows'):
    w32 = ctypes.windll.kernel32
else:
    w32 = None
    #w32 = ctypes.cdll.kernel32



THREAD_SET_INFORMATION = 0x20
THREAD_PRIORITY_ABOVE_NORMAL = 1

#设计要求：
#在无复杂业务逻辑下，极限的数据吞吐量，丢包率为零，延时接近零，抖动、乱序尽量小

# 关于线程run的运行频率：
#与客户端线程数不对等，导致访问频率减缓，其影响大于共享数据访问冲突
#提高某线程的访问频率，可以尝试将其他线程的run调用频率降低
#在相同线程优先级下，注意次要线程对重要线程的控制权的抢占；
#先在相同优先级下做到最优，再根据重要性，调整优先级；
#注意push与pop在各自时间间隙的抢占节奏,减少对更重要一方操作的阻塞；
#结论:
# 线程个数的影响大于临界区;
# 不同PC的I/O吞吐能力不同,导致udp数据吞吐量也不同;
# 线程个数增加,数据吞吐量降低;
#note:
#localhost:
# DATA_SIZE=800, 历史峰值为1446Mbps #430Mbps, 丢包率为1.8%, delay为29
# DATA_SIZE=1500, 历史峰值为2500Mbps #700Mbps, 丢包率为0.6%, delay为24
# DATA_SIZE=64k, 历史峰值为2000Mbps, 丢包率为0, delay为2
# 标准的以太网IP报文大小是：1500 bytes，不包含以太网头和FCS的18 bytes（6+6+2+4），
# 如果包含以太网头和FCS，则为1518 bytes
# network card (1 client):
# mtusize = 1400, 历史峰值为900Mbps, rate = 90%
#example:
#python udpbase.py 0 0 2 1 10.184.101.78 1108
#python udptest.py 1 0 1 2 10.184.101.78

#SOCK_STREAM： 提供面向连接的稳定数据传输，即TCP协议。
#OOB： 在所有数据传送前必须使用connect()来建立连接状态。
#SOCK_DGRAM： 使用不连续不可靠的数据包连接。
#SOCK_SEQPACKET： 提供连续可靠的数据包连接。
#SOCK_RAW： 提供原始网络协议存取。
#SOCK_RDM： 提供可靠的数据包连接。
#SOCK_PACKET： 与网络驱动程序直接通信。
#SO_SNDBUF，设置发送缓冲区的大小。其上限为256 * (sizeof(struct sk_buff) + 256)，下限为2048字节。
#SO_RCVBUF，设置接收缓冲区的大小。上下限分别是：256 * (sizeof(struct sk_buff) + 256)和256字节。
#每个UDP socket都有一个接收缓冲区，没有发送缓冲区，从概念上来说就是只要有数据就发，不管对方是否可以正确接收

CMD_SIZE = 256
DATA_SIZE = 1500 #800 #1500 #64 * 1024 #1500
AUDIO_DATA_SIZE = 800
#DATA_SIZE = 64 * 1024
CMD_SIZE = DATA_SIZE #test

SO_SNDBUF = 64 * 1024
SO_RCVBUF = 64 * 1024
LongMaxBufLen = 30 * 1024 #每帧最大1024个包，最多保存30帧
ShortMaxBufLen = 100 #4 * 1024 #每帧最大1024个包，最大延时4帧间隔
TEST_LOSSRATE = False
PACE_BITRATE = 0
#PACE_BITRATE = 2*1024*1024 #固定码率发送
PACE_BITRATE = 0 #512*1024
FIX_CMD_SIZE = 160
MULT_THREAD = False
MULT_SERVER = False
INTERVAL = 2000000
INTERVAL = 10000000
SHOWFPS = 0.1
RESULTSHOW = 0#1 #0
HEARTBEAT_TIME = 30 #200s
DEFAULT_MODE = 21 #8
LIVETIME = 24 * 60 * 60 #(s) one hour
#LIVETIME = 1 * 60
DECACTOR = 128
BCACTOR = 256
FRAMESKIP = True #False
NETINFO = True #False
RESORTTYPE = 1#0
ADAPTBW = 1#0
LONGLOSS = 1

h264filename = r'/home/gxh/works/datashare/for_ENC/stream720p/FVDO_Girl_720p.264'
yuvfilename = r'/home/gxh/works/datashare/foreman_cif.yuv'
(WIDTH, HEIGHT) = (352, 288)
global_cvshow_status = False
global_render = None

h264filename = ""
yuvfilename = ""

#支持4 * 16 (* 16)并发 bitrate=512kbps



#["ack_cmd", "relay_data", "send_data"]


global_port = 8097 #8888
global_host = 'localhost'
global_actor = 0
global_baseidx = 0
global_speakernum = 1
global_procnum = 1
#global_host = '172.16.0.17'
#global_host = '111.230.226.17'
if len(sys.argv) > 6:
    global_port = int(sys.argv[6])
if len(sys.argv) > 5:
    global_host = sys.argv[5]
if len(sys.argv) > 4:
    global_speakernum = int(sys.argv[4])
if len(sys.argv) > 3:
    global_procnum = int(sys.argv[3])
if len(sys.argv) > 2:
    global_baseidx = int(sys.argv[2])
if len(sys.argv) > 1:
    global_actor = int(sys.argv[1])

def json2str(jsonobj):
    if sys.version_info >= (3, 0):
        json_str = json.dumps(jsonobj, ensure_ascii=False, sort_keys=False).encode('utf-8')
    else:
        json_str = json.dumps(jsonobj, encoding='utf-8', ensure_ascii=False, sort_keys=False)

    return json_str
def char2long(x):
    ret = 0
    if sys.version_info >= (3, 0):
        ret = int(x)
    else:
        try:  # Add these 3 lines
            ret = long(x)
        except: # ValueError:
            print("Something went wrong {!r}".format(x))
    return ret

def str2json(json_str):
    outjson = None
    try:
        if sys.version_info >= (3, 0):
            outjson = json.loads(json_str.decode())
        else:
            outjson = json.loads(json_str, encoding='utf-8')
    except:
        try:
            outjson = json.loads(json_str)
        except:
            print("str2json: fail: json_str=", json_str)
            print("str2json: fail: len(json_str)=", len(json_str))
        else:
            pass
    else:
        pass
    return outjson
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
        self.fps = SHOWFPS#25
        self.imgDict = {}
        self.imgList = []
        self.w = 1280
        self.h = 720
        self.blank = None
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
        if n <= 0:
            return ret
        #print("multshow: n= ", n)
        m = int(math.sqrt(n) + 0.99)
        #print("multshow: m= ", m)
        k = 0
        imgs = None
        imgs1 = []
        (h, w, _) = imglist[0][0].shape
        if self.blank == None:
            self.blank = np.zeros((h, w, 3), np.uint8)
        blank = self.blank
        for i in range(m):
            imgs0 = []
            for j in range(m):
                if k < n:
                    imgs0.append(imglist[k][0])
                else:
                    imgs0.append(blank)
                k += 1
            #print("multshow: len(imgs0)= ", len(imgs0))
            imgs = np.hstack(imgs0)
            imgs1.append(imgs)
        imgs = np.vstack(imgs1)
        frame = cv2.resize(imgs, (self.w, self.h), interpolation=cv2.INTER_CUBIC)
        cv2.imshow("multshow", frame)
        # cv2.waitKey(1)
        keydown = cv2.waitKey(1) & 0xFF
        if keydown == ord('q'):
            ret = 0
        elif keydown == ord('b'):
            ret = -1
        return ret
    def data2show(self):
        imglist = self.PopFrame()
        imgs = []
        for thisdata in imglist:
            if thisdata != None:
                (data, w, h) = thisdata
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
            if len(value) > 4:
                self.imgDict[key] = []
            i += 1
        ret = self.imgList
        self.lock.release()
        return ret
    def run(self):
        print("Render: run: ")
        interval = int(1000.0 / self.fps)
        while self.__running.isSet():
            self.__flag.wait()
            now_time = time.time()
            ###
            ret = self.data2show()
            #print("Render: run: ret=", ret)
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

if RESULTSHOW:
    global_render = Render()


class CmdHead(object):
    def __init__(self):
        self.idx = random.randint(0,(1 << 15))
        self.show_name = ""
        print("CmdHead: self.idx= ", self.idx)
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
        #print("packdata: len(ret)= ", len(ret))
        return ret
    def unpackdata(self, data):
        unpacked = self.headstruct.unpack_from(data, 0)
        data2 = data[self.headstruct.size:]
        #print("unpackdata: len(data2)= ", len(data2))
        ret = (unpacked, data2)
        return ret
    def show_init(self):
        import cv2
        self.show_name = "show_yuv" + str(self.idx)
        cv2.namedWindow(self.show_name, cv2.WINDOW_NORMAL)
    def show_yuv(self, data, w, h):
        import cv2
        global global_cvshow_status
        if global_cvshow_status == False:
            global_cvshow_status = True
            if self.show_name == "":
                self.show_init()
        ret = 1
        if self.show_name != "":
            #print("CmdHead: show_yuv: len(data)= ", len(data))
            frame0 = np.frombuffer(data, dtype=np.uint8)
            yuv_I420_0 = frame0.reshape(int((h * 3) >> 1), w)
            frame = cv2.cvtColor(yuv_I420_0, cv2.COLOR_YUV2BGR_I420)
            #print("CmdHead: show_yuv: frame.shape= ", frame.shape)
            if self.show_name == "":
                self.show_init()
            cv2.imshow(self.show_name, frame)
            #cv2.waitKey(1)
            keydown = cv2.waitKey(1) & 0xFF
            #time.sleep(0.001)
            if keydown == ord('q'):
                ret = 0
            elif keydown == ord('b'):
                ret = -1
        return ret
class CmdData(object):
    def __init__(self):
        self.cmdtype = ""
        self.actor = -1
        self.sessionId = -1
        self.chanId = -1000
        self.modeId = -1
        self.selfmode = 0
        self.liveTime = LIVETIME
        self.seqnum = -1
        self.now_time = 0
        self.st0 = 0
        self.status = 1
        self.note = ""
        self.data = None
        self.cmdDict = {}
    def SetCmd(self):
        cmd = {}
        # cmd = {"actor": self.actor, "sessionId": self.sessionId, "chanId": self.chanId, "seqnum": self.send_packet_num,
        #       "time": now_time, "data": self.testdata}
        cmd.update({"cmd": self.cmdtype})
        cmd.update({"actor": self.actor})
        cmd.update({"sessionId": self.sessionId})
        cmd.update({"chanId": self.chanId})
        cmd.update({"modeId": self.modeId})
        cmd.update({"selfmode": self.selfmode})
        cmd.update({"liveTime": self.liveTime})
        cmd.update({"seqnum": self.seqnum})
        cmd.update({"time": self.now_time})
        cmd.update({"st0": self.now_time})
        cmd.update({"status": self.status})
        cmd.update({"note": self.note})
        cmd.update({"data": self.data})
        self.cmdDict = cmd
        return cmd
    def GetCmd(self, jsondata):
        self.cmdtype = jsondata.get("cmd")
        self.actor = jsondata.get("actor")
        self.sessionId = jsondata.get("sessionId")
        self.chanId = jsondata.get("chanId")
        self.modeId = jsondata.get("modeId")
        self.selfmode = jsondata.get("selfmode")
        self.liveTime = jsondata.get("liveTime")
        self.seqnum = jsondata.get("seqnum")
        self.now_time = jsondata.get("time")
        self.st0 = jsondata.get("st0")
        self.status = jsondata.get("status")
        self.note = jsondata.get("note")
        self.data = jsondata.get("data")
class DataManager(object):
    def __init__(self, revbufsize):
        self.revbufsize = revbufsize
        self.lock = threading.Lock()
        self.DataList = []
        self.q = queue.PriorityQueue(1024)
        self.offset = 0
        self.readable = threading.Event()  # 用于暂停线程的标识
        self.readable.clear()  # 设置为True
    def __del__(self):
        print("DataManager del")
        #self.readable.set()
    def PopQueue(self):
        ret = []
        self.lock.acquire()
        #self.readable.clear()
        n = len(self.DataList)
        if n > 0:
            self.offset -= len(self.DataList[0])
            ret.append(self.DataList[0])
            del(self.DataList[0])
        self.lock.release()
        return ret
    def PopQueueAll(self):
        self.lock.acquire()
        self.offset = 0
        #self.readable.clear()
        ret = self.DataList
        self.DataList = []
        self.lock.release()
        return ret
    def PopQueueGroup(self, m):
        ret = []
        self.lock.acquire()
        #self.readable.clear()
        n = len(self.DataList)
        for i in range(n):
            self.offset -= len(self.DataList[0])
            ret.append(self.DataList[0])
            del(self.DataList[0])
            if i > m:
                break
        self.lock.release()
        return ret
    def PushQueue(self, revcData, remoteHost, remotePort, recvTime):
        #self.__flag.wait()
        #self.__flag.clear()
        self.lock.acquire()
        ##self.offset += len(revcData)
        data = (revcData, remoteHost, remotePort, recvTime)
        self.DataList.append(data)
        #print("DataManager:PushQueue: len(self.DataList)= ", len(self.DataList))
        n = len(self.DataList)
        if n > 1000:
            (revcData0, remoteHost0, remotePort0, recvTime0) = self.DataList[0]
            (revcData1, remoteHost1, remotePort1, recvTime1) = self.DataList[n - 1]
            delay = int((recvTime1 - recvTime0) * 1000)
            ##print("PushQueue: (n, delay)= ", (n, delay))
            self.DataList = [] #test
        self.lock.release()
        #self.readable.set() #会减缓push节奏
    def PushQueue1(self, data):
        self.lock.acquire()
        self.DataList.append(data)
        n = len(self.DataList)
        if n > 1000:
            self.DataList = [] #test
        self.lock.release()

    def PopQueueAll2(self):
        ret = []
        self.lock.acquire()
        while not self.q.empty():
            ret.append(self.q.get())
        #self.q.task_done()
        self.lock.release()
        return ret
    def PushQueue2(self, revcData, remoteHost, remotePort, recvTime):
        self.lock.acquire()
        data = (revcData, remoteHost, remotePort, recvTime)
        self.q.put(data)
        self.lock.release()

def HeartBeat(obj):
    #print("HeartBeat start")
    obj.lock.acquire()
    last_time = obj.last_snd_time  # 避免意外包
    obj.lock.release()
    now = time.time()
    diff = int(now - last_time)
    interval = HEARTBEAT_TIME
    # wait_time = interval - diff #199, 200, 300
    # wait_time = 1 if wait_time < 1 else wait_time
    # 周期内无网络传输；
    # 周期内有网络传输，且在100秒前;
    # 100秒内已发生过网络传输，则取消当下发送；
    # 心跳包最大间隔为为300s
    #print("HeartBeat start: last_time= ", last_time)
    #print("HeartBeat start: now= ", now)
    #print("HeartBeat start: diff= ", diff)
    if diff > (interval >> 1):
        obj.sendheartbeat()
        print("HeartBeat: obj.chanId= ", obj.chanId)

    obj.heartbeat = threading.Timer(interval, HeartBeat, [obj])
    obj.heartbeat.setDaemon(True)
    obj.heartbeat.start()
    # print("HeartBeat ok")
    return
class SendThread(threading.Thread):
    def __init__(self, client, testmode, attribute):
        threading.Thread.__init__(self)
        print("SendThread init 0")
        self.attribute = attribute # "ack_cmd": 0,"relay_data":1,"send_data":2
        self.client = client
        self.sessionId = client.sessionId
        self.actor = client.actor
        self.chanId = client.chanId
        self.idx = (self.sessionId << 16) + self.chanId
        self.idinfo = (self.sessionId << 8) | (self.chanId & 0xFF)
        self.sock = client.sock
        self.host = client.host
        self.port = client.port
        self.revbufsize = client.revbufsize
        self.lock = client.lock
        print("SendThread init 1")
        self.dead = threading.Event()
        self.dead.set()
        print("SendThread init 2")
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
        ###
        print("SendThread init 3")
        self.start_time = 0
        self.last_snd_time = 0
        self.times = 0
        self.send_packet_num = 0
        self.offset = 0
        self.bitrate = PACE_BITRATE
        self.interval = INTERVAL
        self.data_master = None
        if testmode == False:
            self.data_master = DataManager(self.revbufsize)
        self.testdata = ""
        self.str_cmd = None
        self.cmddata = CmdData()
        self.cmdheader = CmdHead()
        self.imglist = []
        self.framidx = 0
        self.init()
        cmd = {"cmd": "heartbeat", "actor": self.actor, "sessionId": self.sessionId}
        self.hbCmd = json2str(cmd)
        self.heartbeat = None
        print("SendThread init ok")
    def __del__(self):
        print("SendThread del")
    def init(self):
        #print("EchoClientThread: init: 0")
        n = DATA_SIZE - FIX_CMD_SIZE
        #print("EchoClientThread: init: n= ", n)
        for i in range(n):
            #print("EchoClientThread: init i= ", i)
            value = i % 10
            #print("EchoClientThread: init: value= ", value)
            #self.data[i] = value
            self.testdata += str(value)
        self.cmddata.cmdtype = "reg"
        self.cmddata.actor = self.actor
        self.cmddata.sessionId = self.sessionId
        self.cmddata.chanId = self.chanId
        self.cmddata.modeId = 0
        self.cmddata.selfmode = 0
        self.cmddata.seqnum = self.send_packet_num
        self.cmddata.now_time = 0
        self.cmddata.data = self.testdata
        self.cmddata.SetCmd()
        ###
        if h264filename != "":
            filename = h264filename
        elif yuvfilename != "":
            filename = yuvfilename
        else:
            return
        self.framesize = 1400
        imgsize = ((WIDTH * HEIGHT * 3) >> 1)
        #coresize = 11 * 9 * 3
        n = imgsize / self.framesize
        print("SendThread:init: n= ", n)
        if not os.path.exists(filename):
            return
        fp = open(filename, 'rb')
        if fp == None:
            print("SendThread:init:fail: filename=", filename)
            return
        count = 0
        while True:
            data = fp.read(self.framesize)
            if len(data) != self.framesize:
                break
            if count > (50 * n):
                break
            self.imglist.append(data)
            #print("SendThread:init: count=", count)
            count += 1
        print("SendThread:init:ok: filename=", filename)

    def hbcycle(self, now_time):
        # print("HeartBeat start")
        self.lock.acquire()
        last_time = self.last_snd_time  # 避免意外包
        self.lock.release()
        #now = time.time()
        diff = int(now_time - last_time)
        interval = HEARTBEAT_TIME
        # wait_time = interval - diff #199, 200, 300
        # wait_time = 1 if wait_time < 1 else wait_time
        # 周期内无网络传输；
        # 周期内有网络传输，且在100秒前;
        # 100秒内已发生过网络传输，则取消当下发送；
        # 心跳包最大间隔为为300s
        # print("HeartBeat start: last_time= ", last_time)
        # print("HeartBeat start: now= ", now)
        # print("HeartBeat start: diff= ", diff)
        if diff > (interval >> 1):
            self.sendheartbeat()
            #print("hbcycle: self.chanId= ", self.chanId)

        #self.heartbeat = threading.Timer(interval, self.hbcycle, [self])
        #self.heartbeat.setDaemon(True)
        #self.heartbeat.start()
        # print("HeartBeat ok")
        return
    def startheartbeat(self):
        self.heartbeat = threading.Timer(HEARTBEAT_TIME, self.hbcycle, [self])
        self.heartbeat.setDaemon(True)
        self.heartbeat.start()  # 开启定时器
    def sendheartbeat(self):
        now_time = time.time()
        try:
            self.lock.acquire()
            sendDataLen = self.sock.sendto(self.hbCmd, (self.host, self.port))
            self.last_snd_time = now_time
            self.lock.release()
        # except IOError, error:
        except IOError as error:  # python3
            print("run error: ", error)
            self.stop()
        else:
            pass
    def packdata(self, data, seqnum, now_time):
        values = (self.idinfo, seqnum, now_time)
        self.cmdheader.packobj(values)
        str_cmd = self.cmdheader.packdata(data)
        return str_cmd
    def createdata(self, now_time):
        if self.str_cmd == None and False:
            #"actor": self.actor, "sessionId": self.sessionId, "chanId": self.id
            #cmd = {"actor": self.actor, "sessionId": self.sessionId, "chanId": self.chanId, "seqnum": self.send_packet_num, "time": now_time, "data": self.testdata}
            self.cmddata.cmdDict.update({"seqnum": self.send_packet_num})
            self.cmddata.cmdDict.update({"time": now_time})
            cmd = self.cmddata.cmdDict
            str_cmd = json2str(cmd)
            ##self.str_cmd = str_cmd
        elif True:
            values = (self.idinfo, self.send_packet_num, now_time)
            self.cmdheader.packobj(values)
            data = self.imglist[self.framidx]
            str_cmd = self.cmdheader.packdata(data)
            #print("SendThread:createdata:len(str_cmd)= ", len(str_cmd))
            self.framidx += 1
            if self.framidx >= len(self.imglist):
                self.framidx = 0
        else:
            data = ""
            # data = str(self.idx) + ","
            # data = str(self.idx) + "," + str(self.send_packet_num) + ","
            # data = str(self.idx) + "," + str(self.send_packet_num) + "," + str(now_time) + ","
            data = str(self.send_packet_num) + "," + str(now_time) + ","
            # inow_time = int(now_time * 1000 * 1000 * 1000)
            # data = str(self.idx) + "," + str(self.send_packet_num) + "," + str(inow_time) + ","
            data += self.testdata
            str_cmd = data.encode('utf-8')

            # str_cmd = self.str_cmd
        return str_cmd
    def getinfo(self, now_time, sendDataLen):
        self.times += 1
        self.offset += sendDataLen
        #self.send_packet_num += (sendDataLen > 0)
        self.send_packet_num += 1
        difftime = int((now_time - self.start_time) * 1000000)
        if difftime > self.interval and self.start_time > 0:
            if self.client.actor != DECACTOR:
                self.hbcycle(now_time)
                pass
            freq = int((self.times * 1000000) / difftime)
            print("SendThread: run: access freq= ", freq)
            bitrate = ((self.offset) << 3)  #
            bitrate = int(1000000 * bitrate / difftime)
            # / (1024 * 1024)
            bitrate >>= 10
            # bitrate /= (1024 * 1024)
            if bitrate > 0:
                sessionId = self.sessionId #(self.idx >> 16)
                chanId = self.chanId #(self.idx & 0xFFFF)
                if bitrate > 10240:
                    bitrate = (bitrate + 512) >> 10
                    print("SendThread: run: sendto: bitrate(Mbps)= ", (sessionId, chanId, bitrate))
                else:
                    print("SendThread: run: sendto: bitrate(Kbps)= ", (sessionId, chanId, bitrate))
            self.start_time = now_time
            #self.send_packet_num = 0
            self.times = 0
            self.offset = 0
        return difftime
    def setbitrate(self, bitrate):
        self.bitrate = bitrate
    def pacesend(self, sendDataLen, difftime):
        ret = 0
        if sendDataLen > 0:
            usedtime = difftime
            interval = self.interval
            bitrate = self.bitrate  # /1000000 /us
            sendbits = sendDataLen << 3
            pretime = (sendbits * 1000000) / bitrate
            sumbits = self.offset << 3
            sumpretime = (sumbits * 1000000) / bitrate
            tailtime = (sumpretime - usedtime)
            waittime = (tailtime + pretime)
            #waittime = pretime #test
            if waittime > 0:
                wait_time = waittime / 1000000.0
                #print("pacesend: wait_time= ", wait_time)
                time.sleep(wait_time)
        return ret
    def fullspeed(self, count):
        # self.__flag.wait()  #会延缓发送频率
        sendDataLen = 0
        difftime = 0
        now_time = time.time()  # 会影响发包效率
        # print("EchoClientThread: run")
        try:
            # self.renewdata()
            # now_time = time.time()#会影响发包效率
            (str_cmd, host, port) = (None, self.host, self.port)
            if self.data_master == None:
                str_cmd = self.createdata(now_time)
                (host, port) = (self.host, self.port)
                if TEST_LOSSRATE:
                    if (count % 10) != 9:
                        self.lock.acquire()
                        sendDataLen = self.sock.sendto(str_cmd, (host, port))
                        #self.last_snd_time = now_time #更新太频繁，
                        self.lock.release()
                elif str_cmd != None:
                    self.lock.acquire()
                    sendDataLen = self.sock.sendto(str_cmd, (host, port))
                    #self.last_snd_time = now_time
                    self.lock.release()
                count += 1
            else:
                # str_cmd = self.createdata(now_time) #test
                # self.data_master.PushQueue(str_cmd, self.host, self.port, now_time) #test
                datalist = self.data_master.PopQueue()
                #datalist = self.data_master.PopQueueAll()
                if len(datalist) > 0:
                    #(str_cmd, host, port, recvTime) = datalist[0]
                    sumlen = 0
                    for (str_cmd, host, port, recvTime) in datalist:
                        sendDataLen = 0
                        if TEST_LOSSRATE:
                            if (count % 10) not in [1,3,9]:
                                self.lock.acquire()
                                sendDataLen = self.sock.sendto(str_cmd, (host, port))
                                #self.last_snd_time = now_time
                                self.lock.release()
                            else:
                                #print("fullspeed: skip pkt count=", count)
                                pass
                        elif str_cmd != None:
                            self.lock.acquire()
                            sendDataLen = self.sock.sendto(str_cmd, (host, port))
                            #self.last_snd_time = now_time #更新太频繁，服务器只在信令层刷新相关状态，数据层将被忽略
                            self.lock.release()
                        sumlen += sendDataLen
                        count += 1
                    sendDataLen = sumlen
                else:
                    time.sleep(0.0001)
                    return (count, sendDataLen, 0)
            # 668Mbps
        # except IOError, error:
        except IOError as error:  # python3
            print("run error: ", error)
            self.stop()
            return count
        else:
            # if (self.send_packet_num % 100000) == 0:
            #    print("EchoClientThread: run: (sendDataLen, self.idx)=", (sendDataLen, self.idx))
            if self.start_time == 0:
                self.start_time = now_time
            # time.sleep(0.00001)
        if self.attribute > 0:
            difftime = self.getinfo(now_time, sendDataLen)
            # time.sleep(0.005)
            # time.sleep(0.0001)
            # time.sleep(0.00001)
            if False:
                sleepn = 1000  # 500 #1000
                for i in range(sleepn):
                    j = 0
        return (count, sendDataLen, difftime)
    def run(self):
        self.dead.clear()
        self.count = 0
        while self.__running.isSet():
            (self.count, sendDataLen, difftime) = self.fullspeed(self.count)
            if self.attribute > 0:
                if self.bitrate > 0 and sendDataLen > 0:
                    self.pacesend(sendDataLen, difftime)
            else:
                time.sleep(0.1)
        self.dead.set()
        print("SendThread: run over")
    def stop(self):
        if self.heartbeat != None:
            self.heartbeat.cancel()
        if self.data_master != None:
            self.data_master.readable.set()
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals

        print("SendThread: stop over")

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞
class RecvThread(threading.Thread):
    def __init__(self, client):
        threading.Thread.__init__(self)
        self.client = client
        self.actor = client.actor
        self.sessionId = client.sessionId
        self.chanId = client.chanId
        self.idx = (self.sessionId << 16) + self.chanId
        self.sock = client.sock
        self.host = client.host
        self.port = client.port
        self.revbufsize = client.revbufsize
        self.lock = client.lock
        array_type = c_char_p * 4
        self.outparam = array_type()
        handle_size = 16
        self.ll_handle = create_string_buffer(handle_size)
        # self.ll_handle = (c_char * handle_size)()
        array_type = c_char_p * 1
        self.ctime = array_type()
        ###
        self.dead = threading.Event()
        self.dead.set()
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True

        self.last_snd_time = 0
        self.recv_packet_num = 0
        self.start_time = 0
        self.times = 0
        self.offset = 0
        self.interval = INTERVAL

        self.data_master = DataManager(self.revbufsize)
        cmd = {"cmd": "heartbeat", "actor": self.actor, "sessionId": self.sessionId}
        self.hbCmd = json2str(cmd)
        self.heartbeat = None

    def __del__(self):
        print("RecvThread del")
    def hbcycle(self, now_time):
        # print("HeartBeat start")
        self.lock.acquire()
        last_time = self.last_snd_time  # 避免意外包
        self.lock.release()
        #now = time.time()
        diff = int(now_time - last_time)
        interval = HEARTBEAT_TIME
        # wait_time = interval - diff #199, 200, 300
        # wait_time = 1 if wait_time < 1 else wait_time
        # 周期内无网络传输；
        # 周期内有网络传输，且在100秒前;
        # 100秒内已发生过网络传输，则取消当下发送；
        # 心跳包最大间隔为为300s
        # print("HeartBeat start: last_time= ", last_time)
        # print("HeartBeat start: now= ", now)
        # print("HeartBeat start: diff= ", diff)
        if diff > (interval >> 1):
            self.sendheartbeat()
            print("hbcycle: self.chanId= ", self.chanId)

        #self.heartbeat = threading.Timer(interval, self.hbcycle, [self])
        #self.heartbeat.setDaemon(True)
        #self.heartbeat.start()
        # print("HeartBeat ok")
        return
    def startheartbeat(self):
        self.heartbeat = threading.Timer(HEARTBEAT_TIME, self.hbcycle, [self])
        self.heartbeat.setDaemon(True)
        self.heartbeat.start()  # 开启定时器
    def sendheartbeat(self):
        now_time = time.time()
        try:
            self.lock.acquire()
            sendDataLen = self.sock.sendto(self.hbCmd, (self.host, self.port))
            self.last_snd_time = now_time
            self.lock.release()
        # except IOError, error:
        except IOError as error:  # python3
            print("run error: ", error)
            self.stop()
        else:
            pass
    def setPriority(self, priority):
        if not self.isAlive():
            print('Unable to set priority of stopped thread')
        if w32 != None:
            handle = w32.OpenThread(THREAD_SET_INFORMATION, False, self.tid)
            result = w32.SetThreadPriority(handle, priority)
            w32.CloseHandle(handle)
            if not result:
                print('Failed to set priority of thread', w32.GetLastError())
    def getinfo(self, now_time, revDataLen):
        self.times += 1
        self.offset += revDataLen
        self.recv_packet_num += (revDataLen > 0)
        difftime = int((now_time - self.start_time) * 1000000)
        if difftime > self.interval and self.start_time > 0:
            if self.client.actor == DECACTOR:
                self.hbcycle(now_time)
                pass
            freq = int((self.times * 1000000) / difftime)
            print("RecvThread: run: access freq= ", freq)
            bitrate = ((self.offset) << 3)  #
            bitrate = int(1000000 * bitrate / difftime)
            # / (1024 * 1024)
            bitrate >>= 10 #20
            # bitrate /= (1024 * 1024)
            if bitrate > 0:
                sessionId = self.sessionId #(self.idx >> 16)
                chanId = self.chanId #(self.idx & 0xFFFF)
                if bitrate > 10240:
                    bitrate = (bitrate + 512) >> 10
                    print("RecvThread: run: recvfrom: bitrate(Mbps)= ", (sessionId, chanId, bitrate))
                else:
                    print("RecvThread: run: recvfrom: bitrate(Kbps)= ", (sessionId, chanId, bitrate))
            self.start_time = now_time
            #self.recv_packet_num = 0
            self.times = 0
            self.offset = 0
    def run(self):
        if w32 != None:
            self.tid = w32.GetCurrentThreadId()
            print("self.tid= ", self.tid)
        self.dead.clear()
        #与客户端线程数不对等，导致访问频率减缓
        if w32 != None:
            self.tid = w32.GetCurrentThreadId()
            print("self.tid= ", self.tid)
        name = self.getName()
        print("name= ", name)
        while self.__running.isSet():
            #self.__flag.wait()   # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            recvData = None
            data = ""
            now_time = 0#time.time()
            revDataLen = 0
            try:
                rdata = self.sock.recvfrom(DATA_SIZE)
                if rdata[0] != '' and rdata[1] != None:
                    recvData, (remoteHost, remotePort) = rdata
                else:
                    print("RecvThread: run: rdata= ", rdata)
                    break
            #except IOError, error:  # python2
            except IOError as error:  # python3
                if hasattr(error, '__iter__') and "timed out" in error:
                    #print("EchoServerThread: run: timeout: recvfrom error= ", error)
                    pass
                elif error.errno == errno.EWOULDBLOCK:
                    #print("EchoServerThread: run: EWOULDBLOCK: recvfrom error= ", error)
                    pass
                elif error.errno == errno.WSAETIMEDOUT:
                    pass
                elif error.errno == errno.ENAMETOOLONG:
                    #print("EchoServerThread: run: ENAMETOOLONG: recvfrom error= ", error)
                    pass
                else:
                    print("RecvThread: run: recvfrom error= ", error)
            else:
                now_time = time.time()
                if self.start_time == 0:
                    self.start_time = now_time
                #print("EchoServerThread: run: recvData= ", recvData)
                self.data_master.PushQueue(recvData, remoteHost, remotePort, now_time)# c_time_ll)
                revDataLen = len(recvData)

            self.getinfo(now_time, revDataLen)
        self.dead.set()
        print("RecvThread: run over")
    def stop(self):
        if self.heartbeat != None:
            self.heartbeat.cancel()
        if self.data_master != None:
            self.data_master.readable.set()
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals

        print("RecvThread: stop over")
    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞

class NetInfo(object):
    def __init__(self, idx):
        self.idx = idx
        #self.lasseq = -1
        self.Reset()
        self.netList = []
        self.start_time = 0
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
    def Reset(self):
        self.lasseq = -1
        self.maxdseq = 0
        self.minseq = 1 << 31
        self.maxseq = -1
        self.mintime = 1 << 63
        self.maxtime = -1
        self.delay = -1
        self.rtt = 0
        self.avgdelay = 0
        self.m = 4
        self.cnt = 0
        self.start_time = 0
    def CountInfo(self, revdata, idx):
        #print("CountInfo: (idx, self.idx, self.cnt)= ", (idx, self.idx, self.cnt))
        (seqnum, time0, time2, data) = revdata
        if seqnum == None:
            return 0
        if self.idx < 0:
            self.idx = idx
        if idx != self.idx:
            return 0
        (seqnum, time0, time2, data) = revdata
        if self.lasseq >= 0:
            dseqnum = abs(seqnum - self.lasseq)
            if dseqnum > self.maxdseq:
                self.maxdseq = dseqnum
        if seqnum < self.minseq:
            self.minseq = seqnum
        if seqnum > self.maxseq:
            self.maxseq = seqnum
        self.lasseq = seqnum
        ###
        delay0 = (time2 - time0)
        if self.cnt < self.m:
            self.rtt += delay0
        if time0 < self.mintime:
            self.mintime = time0
        if time0 > self.maxtime:
            self.maxtime = time0

        if delay0 > self.delay:
            self.delay = delay0

        self.avgdelay += delay0
        self.cnt += 1
        if self.cnt > 100 and ((self.cnt % 10) == 0):
            now_time = time.time()
            if self.start_time == 0:
                self.start_time = now_time
            difftime = int((now_time - self.start_time) * 1000)
            if difftime >= 2000 and True:
                ret = self.CountLossRate2()
                self.start_time = now_time
        return 1
    def CountLossRate2(self):
        ret = []
        if self.cnt == 0:
            return ret
        #return ret
        rtt = int((self.rtt * 1000) / self.m)
        #print("CountLossRate2: self.cnt= ", self.cnt)
        avgdelay = int((self.avgdelay * 1000) / self.cnt) - rtt
        seqcnt = self.maxseq - self.minseq + 1
        lossrate = int((1.0 - float(self.cnt) / float(seqcnt)) * 1000)  # choas?
        delay = int(self.delay * 1000) - rtt
        ret.append((lossrate, delay, rtt, avgdelay, self.maxdseq ,self.cnt, self.idx))
        if lossrate > 0:
            print("CountLossRate2: ret= ", ret)
        self.Reset()
        return ret
    def CountLossRate(self):
        #now_time0 = time.time()
        RevDataList = []
        ret = []
        for j in range(len(RevDataList)):
            thisRevDataList = RevDataList[j]
            n = len(thisRevDataList)
            if n > 0:
                minseq = 1 << 31
                maxseq = -1
                mintime = 1 << 63
                maxtime = -1
                delay = -1
                m = 4
                rtt = 0
                sumdelay = 0
                for i in range(n):
                    (seqnum, time, time2, sockId) = thisRevDataList[i]
                    if seqnum < minseq:
                        minseq = seqnum
                    if seqnum > maxseq:
                        maxseq = seqnum
                    ###
                    delay0 = (time2 - time)
                    if i < m:
                        rtt += delay0
                    else:
                        if time < mintime:
                            mintime = time
                        if time > maxtime:
                            maxtime = time

                        if delay0 > delay:
                            delay = delay0
                    sumdelay += delay0
                rtt = int((rtt * 1000) / 4)
                avgdelay = int((sumdelay * 1000) / n) - rtt
                seqcnt = maxseq - minseq + 1
                lossrate = int((1.0 - float(n) / float(seqcnt)) * 1000) #choas?
                delay = int(delay * 1000) - rtt
                ret.append((lossrate, delay, rtt, avgdelay, j))
            RevDataList[j] = []
        #now_time1 = time.time()
        #difftime = int((now_time1 - now_time0) * 1000)
        #print("CountLossRate: difftime=", difftime)
        return ret
class DataProcess(object):
    def __init__(self, client, idx):
        self.client = client
        self.idx = idx
        self.idx2 = random.randint(0, (1 << 15))
        self.offset = 0
        self.imgsize = ((WIDTH * HEIGHT * 3) >> 1)
        self.data = bytearray(self.imgsize << 1)
        self.lastseqnum = -1
        self.netInfo = NetInfo(idx)
        self.cmdheader = CmdHead()
        self.pktnum = 0
        self.last_loss_pktnum = 0
    def ProcessRaw(self, data):
        (revcData, remoteHost, remotePort, recv_time) = data
        sockId = remoteHost + "_" + str(remotePort)
        revdata = None
        if False:
            jsondata = str2json(revcData)
            if jsondata != None:
                # cmd = {"idx":self.idx, "seqnum":self.send_packet_num, "time":now_time, "data": self.testdata}
                # cmd = {"actor": self.actor, "sessionId": self.sessionId, "chanId": self.idx, "seqnum": self.send_packet_num, "time": now_time, "data": self.testdata}
                cmddata = CmdData()
                cmddata.GetCmd(jsondata)
                # sessionId = jsondata.get("sessionId")
                # chanId = jsondata.get("chanId")
                # actor = jsondata.get("actor")
                # seqnum = jsondata.get("seqnum")
                # time = jsondata.get("time")
                # data = jsondata.get("data")
                time2 = recv_time
                revdata = (cmddata.seqnum, cmddata.now_time, time2, cmddata.data)
                if self.netInfo != None:
                    self.netInfo.CountInfo(revdata, self.idx)
        else:
            #item = revcData.decode().split(",")
            #seqnum = int(item[0])
            #time0 = float(item[1])
            #r2 = item[2]
            (r1, r2) = self.cmdheader.unpackdata(revcData)
            #print("DataProcess:Process: r1= ", r1)
            #print("DataProcess:Process: len(r2)= ", len(r2))
            (idinfo, seqnum, time0) = r1
            if RESULTSHOW:
                if self.lastseqnum >= 0:
                    dseqnum = seqnum - self.lastseqnum
                #self.offset += ((seqnum + 1) * len(r2)) % self.imgsize
                self.data[self.offset:] = r2
                ###
                #self.offset += ((seqnum + 1) * len(r2)) % self.imgsize
                self.offset += len(r2)
                #self.offset = ((seqnum + 1) * len(r2)) % self.imgsize
                loss_pktnum = seqnum - self.pktnum
                this_loss_pktnum = loss_pktnum - self.last_loss_pktnum
                loss_pktsize = this_loss_pktnum * len(r2)
                #loss_pktsize = 0 #test
                ###
                self.lastseqnum = seqnum
                if self.offset >= self.imgsize:
                    self.offset += loss_pktsize
                    #print("DataProcess:Process: self.offset= ", (self.idx, self.offset))
                    global global_render
                    img = self.data[0:self.imgsize]
                    if global_render != None:
                        (w, h) = (WIDTH, HEIGHT)
                        global_render.PushFrame(self.idx2, img, w, h)
                    else:
                        ret = self.cmdheader.show_yuv(img, WIDTH, HEIGHT)
                    taildata = self.data[self.imgsize:self.offset]
                    tail = self.offset - self.imgsize
                    self.data[0:tail] = taildata
                    self.offset = tail
                    self.last_loss_pktnum = loss_pktnum
            self.pktnum += 1
            revdata = (seqnum, time0, recv_time, r2)
            if self.netInfo != None:
                self.netInfo.CountInfo(revdata, self.idx)
        return (revdata, data)
    def ProcessCmd(self, data):
        ret = None
        #print("ProcessCmd: data= ", data)
        return ret
#以socket为单元，多线程处理
class SocketThread(threading.Thread):
    def __init__(self, parent, sessionId, chanId, actor, selfmode, host, port):
        threading.Thread.__init__(self)
        self.parent = parent
        self.server = parent.server
        self.sock = parent.server.sock
        self.sessionId = sessionId
        self.chanId = chanId
        self.actor = actor
        self.selfmode = selfmode
        self.idx = (sessionId << 16) + chanId
        self.host = host
        self.port = port
        self.revbufsize = parent.server.revbufsize
        self.lock = threading.Lock()
        self.start_time = 0
        self.DataList = []
        self.netInfo = NetInfo(self.idx)
        self.sendthread = None
        attribute = 0 #"ack_cmd"
        if (actor == DECACTOR):
            attribute = 1#"relay_data"
        self.sendthread = SendThread(self, False, attribute)
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
        self.readable = threading.Event()
        self.readable.clear()
        if self.sendthread != None:
            self.sendthread.setDaemon(True)
            self.sendthread.start()
    def PopQueue(self):
        ret = []
        #self.__flag.wait()
        #self.__flag.clear()
        self.lock.acquire()
        if len(self.DataList) > 0:
            ret.append(self.DataList[0])
            del(self.DataList[0])
        self.lock.release()
        #self.__flag.set()
        return ret
    def PopQueueAll(self):
        #self.__flag.wait()
        #self.__flag.clear()
        self.lock.acquire()
        ret = self.DataList
        self.DataList = []
        self.lock.release()
        #self.__flag.set()
        return ret

    def PushQueue(self, revcData):
        #self.__flag.wait()
        #self.__flag.clear()
        self.lock.acquire()
        self.DataList.append(revcData)
        #print("DataManager:PushQueue: len(self.DataList)= ", len(self.DataList))
        n = len(self.DataList)

        self.lock.release()
        self.readable.set()
    def run(self):
        while self.__running.isSet():
            #self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            self.readable.clear()
            self.readable.wait()
            now_time = time.time()
            if self.start_time <= 0:
                self.start_time = now_time
            #data = self.PopQueue()
            data = self.PopQueueAll()
            if self.netInfo != None:
                for (thisdata, revcData) in data:
                    self.netInfo.CountInfo(thisdata, self.idx)
                    if self.sendthread != None:
                        if self.sendthread.data_master != None:
                            (remoteHost, remotePort) = (self.sendthread.host, self.sendthread.port)
                            self.sendthread.data_master.PushQueue(revcData, remoteHost, remotePort, now_time)

            if data == None or data == []:
                #过频繁的访问会导致锁被占用
                time.sleep(0.001)
            else:
                #time.sleep(0.00001) #影响收包效率!!!
                pass
    def stop(self):
        self.readable.set()
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals
        if self.sendthread != None:
            self.sendthread.stop()
        if self.sendthread != None:
            self.sendthread.dead.wait()
        print("SocketThread: stop")

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞
#one broadcast one user
class BroadCastThread(threading.Thread):
    def __init__(self, parent):
        threading.Thread.__init__(self)
        self.parent = parent
        self.server = parent.server
        self.lock = threading.Lock()
        self.clientList = []
        #self.inforList = []
        self.inforQueue = {}
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
    def __del__(self):
        print("BroadCastThread del")

    def PushClient(self, sockId):
        self.lock.acquire()
        if sockId not in self.clientList:
            self.clientList.append(sockId)
        self.lock.release()
    def DeleteClient(self, sockId):
        self.lock.acquire()
        n = len(self.clientList)
        j = 0
        for i in range(n):
            i -= j
            if i >= len(self.clientList):
                break
            sockId0 = self.clientList[i]
            if sockId == sockId0:
                del self.clientList[i]
                j += 1
        self.lock.release()
    def DeleteSession(self, sessionId):
        print("BroadCastThread: DeleteSession")
        self.parent.DeleteSession(sessionId)
        self.lock.acquire()
        infor = self.inforQueue.get(sessionId)
        if infor != None:
            self.inforQueue.pop(sessionId)
        self.lock.release()
    def PushQueue(self, sockId, sessionId, modeId, liveTime, startTime, status):
        self.lock.acquire()
        infor = self.inforQueue.get(sessionId)
        if infor == None:
            self.inforQueue.update({sessionId:(sockId, modeId, liveTime, startTime, status)})
        else:
            self.inforQueue[sessionId] = (sockId, modeId, liveTime, startTime, status)
        #self.inforList.append((sessionId, modeId, status))
        self.lock.release()
    #def PopQueue(self):
    #    ret = None
    #    self.lock.acquire()
    #    if len(self.inforList) > 0:
    #        ret = self.inforList[0]
    #        del (self.inforList[0])
    #    self.lock.release()
    #    return ret
    def run(self):
        while self.__running.isSet():
            self.__flag.wait()   # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            now_time = time.time()
            self.lock.acquire()
            if len(self.clientList) > 0:
                #sessionInfo = self.PopQueue()
                keyList = list(self.inforQueue.keys())
                n = len(keyList)
                for i in range(n):
                    sessionId = keyList[i]
                    if sessionId in self.inforQueue.keys():
                        value = self.inforQueue[sessionId]
                        (sockId0, modeId, liveTime, startTime, status) = value
                        if status:
                            difftime = now_time - startTime
                            if difftime > liveTime:
                                status = 0
                                print("BroadCastThread: run: liveTime < difftime=", difftime)
                        # 通知所有的socket
                        for sockId in self.clientList:
                            self.lock.release()
                            remoteHost = sockId.split("_")[0]
                            remotePort = int(sockId.split("_")[1])
                            jsonobj = {"cmd":"broadcast" ,"sessionId": sessionId, "modeId": modeId, "note": "test", "status": status}
                            json_str = json2str(jsonobj)
                            sendDataLen = self.server.sock.sendto(json_str, (remoteHost, remotePort))
                            self.lock.acquire()
                        if status == 0:
                            # 删除该信息
                            self.lock.release()
                            self.DeleteSession(sessionId)
                            self.lock.acquire()
                        elif status == 88:
                            self.lock.release()
                            self.DeleteSession(sessionId)
                            self.DeleteClient(sockId)
                            self.lock.acquire()
                self.lock.release()
                time.sleep(1) #每隔1秒播报一次会议状态
                self.lock.acquire()
            else:
                self.lock.release()
                time.sleep(0.01)
                self.lock.acquire()
            self.lock.release()
        print("BroadCastThread: run over")
    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals

        print("BroadCastThread: stop")

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞
#失联的sock不转发数据
class SockManager(threading.Thread):
    def __init__(self, parent):
        threading.Thread.__init__(self)
        self.parent = parent
        self.server = parent.server
        self.lock = self.parent.lock
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
    def CheckSocket(self):
        now_time = time.time()
        self.lock.acquire()
        sessionDict = self.parent.parent.sessionDict
        for key,value in sessionDict.items():
            sessionId = key
            sessionInfo = value
            if sessionInfo != None:
                actors = sessionInfo["info"]["actors"] #[1, 2, 3]
                for actor in actors:
                    sockdict = sessionInfo.get(actor)
                    for key, value in sockdict.items():
                        if value != None:
                            sockId = key
                            chanId = value[0]
                            recv_time = value[1]
                            difftime = now_time - recv_time
                            if difftime < 60:  # 1分钟内存活/alive in one minutes
                                pass
                            else:
                                #delete socket
                                sessionInfo["info"]["chanIdList"][chanId] = -1
                                pass
        self.lock.release()
    def run(self):
        while self.__running.isSet():
            # self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            self.CheckSocket()
    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals
        print("CmdThread: stop")

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞
class CmdThread(threading.Thread):
    def __init__(self, parent):
        threading.Thread.__init__(self)
        self.parent = parent
        self.server = parent.server
        self.lock = self.parent.lock2
        self.sock = parent.server.sock
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
        self.data_master = DataManager(self.server.revbufsize)
        self.broadCast = BroadCastThread(self)
        self.broadCast.setDaemon(True)
        self.broadCast.start()
        #self.sockmaster = SockManager(self)
        #self.sockmaster.setDaemon(True)
        #self.sockmaster.start()

    def DeleteSession(self, sessionId):
        #self.parent.DeleteSession(sessionId)
        print("CmdThread: DeleteSession")
        now_time = time.time()
        self.lock.acquire()
        if True:
            sessionInfo = self.parent.sessionDict.get(sessionId)
            if sessionInfo != None:
                actors = sessionInfo["info"]["actors"]#[1, 2, 3]
                for actor in actors:
                    sockdict = sessionInfo.get(actor)
                    if sockdict != None:
                        #for key, value in sockdict.items():
                        keyList = list(sockdict.keys())
                        n = len(keyList)
                        for i in range(n):
                            key = keyList[i]
                            value = sockdict.get(key)
                            if value != None:
                                sockId = key
                                chanId = value[0]
                                recv_time = value[1]
                                difftime = now_time - recv_time
                                if difftime < 60:  # 1分钟内存活/alive in one minutes
                                    # sayby
                                    pass
                                if self.parent.socketDict.get(sockId) != None:
                                    self.parent.socketDict.pop(sockId)
                self.parent.sessionDict.pop(sessionId)
        self.lock.release()
    def AssignChanId(self, sessionId, actor):
        ret = -1
        now_time = time.time()
        self.lock.acquire()
        sessionInfo = self.parent.sessionDict.get(sessionId)
        if sessionInfo != None:
            chanIdList = sessionInfo["info"]["chanIdList"]
            print("AssignChanId: chanIdList= ", chanIdList)
            for i in range(len(chanIdList)):
                chanId = chanIdList[i]
                if chanId == -1:
                    ret = i
                    sessionInfo["info"]["chanIdList"][i] = -2 -i # 在确认中
                    break
            if ret < 0:
                actors = sessionInfo["info"]["actors"]#[1, 2, BCACTOR]
                actors.append(BCACTOR)
                isok = False
                for actor in actors:
                    if isok:
                        break
                    sockdict = sessionInfo.get(actor)
                    if sockdict != None:
                        #for key, value in sockdict.items():
                        keyList = list(sockdict.keys())
                        n = len(keyList)
                        for i in range(n):
                            if isok:
                                break
                            key = keyList[i]
                            value = sockdict.get(key)
                            if value != None:
                                sockId = key
                                chanId = value[0]
                                recv_time = value[1]
                                difftime = now_time - recv_time
                                if difftime > (HEARTBEAT_TIME << 1):  # 1分钟内存活/alive in one minutes
                                    if chanId >= 0:
                                        ret = chanId
                                        sessionInfo["info"]["chanIdList"][chanId] = -2 - chanId  # 在确认中
                                        sockdict.pop(sockId)
                                        print("AssignChanId: (actor, chanId)= ", (actor, chanId))
                                        isok = True
                                        break
                                    else:
                                        #ret = -(2 + chanId)  # 在确认中
                                        sockdict.pop(sockId)
                                        print("AssignChanId: 2: (actor, chanId)= ", (actor, chanId))
                                        pass
        self.lock.release()
        return ret
    def CmdProcess(self, data):
        iscmd = True
        (revcData, remoteHost, remotePort, recv_time) = data
        sockId = remoteHost + "_" + str(remotePort)
        n = len(revcData)
        if True:
            jsondata = str2json(revcData)
            if jsondata != None:
                # cmd = {"idx":self.idx, "seqnum":self.send_packet_num, "time":now_time, "data": self.testdata}
                # cmd = {"actor": self.actor, "sessionId": self.sessionId, "chanId": self.idx, "seqnum": self.send_packet_num, "time": now_time, "data": self.testdata}
                cmddata = CmdData()
                cmddata.GetCmd(jsondata)
                #cmdtype = jsondata.get("cmd")
                #sessionId = jsondata.get("sessionId")
                #chanId = jsondata.get("chanId")
                #actor = jsondata.get("actor")
                # idx = jsondata.get("idx")
                #seqnum = jsondata.get("seqnum")
                #time = jsondata.get("time")
                #data = jsondata.get("data")
                sessionId = cmddata.sessionId
                chanId = cmddata.chanId
                modeId = cmddata.modeId
                status = cmddata.status
                cmdtype = cmddata.cmdtype
                actor = cmddata.actor
                st0 = cmddata.st0
                if cmdtype in ["broadcast"]:
                    print("CmdProcess: revcData=", revcData)
                    if sessionId == None:
                        if st0 != None:
                            now_time = time.time()
                            jsonobj = {"cmd": "broadcast", "actor": actor, "st0":st0, "rt0":now_time, "st1":now_time}
                            revcData = json2str(jsonobj)
                        else:
                            self.broadCast.PushClient(sockId)
                    elif status == 0:
                        self.broadCast.PushQueue(sockId, sessionId, modeId, -1, -1, 0)
                    elif chanId == -1 and sessionId >= 0:
                        self.lock.acquire()
                        sessionInfo = self.parent.sessionDict.get(sessionId)
                        if sessionInfo != None:
                            if sessionInfo.get(actor) == None:
                                self.lock.release()
                                chanId = self.AssignChanId(sessionId, actor)
                                self.lock.acquire()
                                sessionInfo.update({actor: {sockId: (chanId, recv_time)}})
                            else:
                                sockitem = sessionInfo[actor].get(sockId)
                                if sockitem != None:
                                    chanId = sockitem[0]
                                    if chanId < 0:
                                        self.lock.release()
                                        chanId = self.AssignChanId(sessionId, actor)
                                        self.lock.acquire()
                                        sessionInfo[actor].update({sockId: (chanId, recv_time)})
                                else:
                                    self.lock.release()
                                    chanId = self.AssignChanId(sessionId, actor)
                                    self.lock.acquire()
                                    sessionInfo[actor].update({sockId: (chanId, recv_time)})
                        self.lock.release()
                        #if chanId >= 0:
                        jsonobj = {"cmd":"broadcast", "sessionId": sessionId, "chanId": chanId, "modeId": modeId}
                        revcData = json2str(jsonobj)
                    sendDataLen = self.sock.sendto(revcData, (remoteHost, remotePort))  # ack
                    return iscmd
                elif cmdtype in ["heartbeat"]:
                    #print("CmdProcess: heartbeat")
                    self.lock.acquire()
                    sessionInfo = self.parent.sessionDict.get(sessionId)
                    if sessionInfo != None:
                        sockdict = sessionInfo.get(actor)
                        if sockdict != None:
                            if sockdict.get(sockId) != None:
                                #刷新时间
                                chanId = self.parent.sessionDict[sessionId][actor][sockId][0]
                                self.parent.sessionDict[sessionId][actor][sockId] = (chanId, recv_time)
                    self.lock.release()
                    sendDataLen = self.sock.sendto(revcData, (remoteHost, remotePort))  # ack
                    return iscmd
                selfmode = cmddata.selfmode
                liveTime = cmddata.liveTime

                time2 = recv_time
                revdata = (cmddata.seqnum, cmddata.now_time, time2, cmddata.data)
                (sockThread, sessionId2) = (None, None)

                self.lock.acquire()
                #sessionDict = copy.deepcopy(self.sessionDict)
                #socketDict = copy.deepcopy(self.socketDict)
                #self.parent.lock2.release()

                sockItem = self.parent.socketDict.get(sockId)
                if sockItem == None:
                    #if (actor in [3]):
                    if MULT_THREAD:
                        sockThread = SocketThread(self, sessionId, chanId, actor, selfmode, remoteHost, remotePort)
                        # sockThread.setPriority(2)
                        sockThread.setDaemon(True)
                        sockThread.start()
                    self.parent.socketDict.update({sockId: (sockThread, sessionId, chanId, actor, selfmode, remoteHost, remotePort)})
                else:
                    (sockThread, sessionId2, chanId2, actor2, selfmode2, remoteHost2, remotePort2) = sockItem
                if True:
                    #会影响转发率
                    #sockId = remoteHost + "_" + str(remotePort)
                    sessionInfo = self.parent.sessionDict.get(sessionId)
                    if sessionInfo == None:
                        chanIdList = None
                        chanIdMap = [1, 2, 2, 2, 2, 2, 2, 3, 4, 4, 5, 5, 5, 5, 8, 8, 8, 8, 9, 16, 32, 64]
                        if modeId >= 0 and modeId < len(chanIdMap):
                            chanIdList = [-1 for x in range(chanIdMap[modeId])]
                            chanIdList[chanId] = chanId
                            # head_dict.update({"chanIdList":chanIdList})
                            self.broadCast.PushQueue(sockId, sessionId, modeId, liveTime, recv_time, 1)
                        print("CmdProcess: sessionId= ", sessionId)
                        print("CmdProcess: modeId= ", modeId)
                        print("CmdProcess: chanId= ", chanId)
                        #print("CmdProcess: chanIdList= ", chanIdList)
                        #self.parent.sessionDict.update({sessionId: {actor: [(sockId, chanId, recv_time)]}})
                        self.parent.sessionDict.update({sessionId: {actor: {sockId:(chanId, recv_time)}}})
                        #(modeId, selfmode) = (0, 0)
                        self.parent.sessionDict[sessionId].update({"info": {"modeId": modeId, "selfmode": selfmode}})
                        self.parent.sessionDict[sessionId]["info"].update({"liveTime": liveTime})
                        self.parent.sessionDict[sessionId]["info"].update({"startTime": recv_time})
                        #chanIdList = [chanId]
                        self.parent.sessionDict[sessionId]["info"].update({"chanIdList": chanIdList})
                        self.parent.sessionDict[sessionId]["info"].update({"actors": [actor]})
                        dataDict = {}
                        # dataDict.update({sockId: {"longdata": [(revcData, recv_time)]}})
                        # dataDict[sockId].update({"data": [(revcData, recv_time)]})
                        dataDict.update({sockId: {"longdata": []}})
                        dataDict[sockId].update({"shortdata": []})
                        self.parent.sessionDict[sessionId].update({"data": dataDict})
                    else:
                        sessionInfo["data"].update({sockId: {"longdata": []}})
                        sessionInfo["data"][sockId].update({"shortdata": []})
                        if actor not in sessionInfo["info"]["actors"]:
                            sessionInfo["info"]["actors"].append(actor)
                        if sessionInfo.get(actor) == None:
                            #sessionInfo.update({actor: [(sockId, chanId, recv_time)]})
                            sessionInfo.update({actor: {sockId:(chanId, recv_time)}})
                        else:
                            #sessionInfo[actor].append((sockId, chanId, recv_time))
                            sessionInfo[actor].update({sockId:(chanId, recv_time)})
                        if actor < DECACTOR:
                            print("CmdProcess: sessionId= ", sessionId)
                            print("CmdProcess: modeId= ", modeId)
                            print("CmdProcess: chanId= ", chanId)
                            #print("CmdProcess: chanIdList= ", sessionInfo["info"]["chanIdList"])
                            sessionInfo["info"]["chanIdList"][chanId] = chanId
                        else:
                            pass
                #self.parent.lock2.acquire()
                #self.sessionDict = sessionDict
                #self.socketDict = socketDict
                self.lock.release()

                if sockThread != None and True:
                    sockThread.PushQueue((revdata, revcData))
                else:
                    sendDataLen = self.sock.sendto(revcData, (remoteHost, remotePort))#ack
        else:
            print("CmdProcess: data=", data)
        return iscmd

    def run(self):
        while self.__running.isSet():
            # self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            datalist = self.data_master.PopQueueAll()
            if datalist == None or datalist == []:  # or len(datalist) < 15:
                # 过频繁的访问会导致锁被占用
                time.sleep(0.001)
            else:
                for data in datalist:
                    self.CmdProcess(data)
    def stop(self):
        if self.broadCast != None:
            self.broadCast.stop()
        #if self.sockmaster != None:
        #    self.sockmaster.stop()
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals
        print("CmdThread: stop")

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞
class RelayThread(threading.Thread):
    def __init__(self, parent):
        threading.Thread.__init__(self)
        self.parent = parent
        self.server = parent.server
        self.sock = parent.server.sock
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals
        print("CmdThread: stop")

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞
#数据socket分流处理
class SocketProcessThread(threading.Thread):
    def __init__(self, server):
        threading.Thread.__init__(self)
        self.server = server
        self.sock = server.sock
        self.lock = threading.Lock()
        self.lock2 = threading.Lock()
        self.socketDict = {}
        self.sessionDict = {}
        self.start_time = 0
        self.netDataList = []
        self.netInfoDict = {}
        ###
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True

        self.readable = threading.Event()
        self.readable.clear()
        ###
        self.start_time = 0
        self.times = 0
        self.send_packet_num = 0
        self.offset = 0
        self.interval = INTERVAL
        self.cmdthread = CmdThread(self)
        self.cmdthread.setDaemon(True)
        self.cmdthread.start()
        self.cmdheader = CmdHead()

    def PopQueue(self):
        ret = []
        # self.__flag.wait()
        # self.__flag.clear()
        self.lock.acquire()
        if len(self.netDataList) > 0:
            ret.append(self.netDataList[0])
            del (self.netDataList[0])
        self.lock.release()
        # self.__flag.set()
        return ret
    def PopQueueAll(self):
        #self.__flag.wait()
        #self.__flag.clear()
        self.lock.acquire()
        ret = self.netDataList
        self.netDataList = []
        self.lock.release()
        #self.__flag.set()
        return ret
    def PushQueue(self, datalist):
        self.lock.acquire()
        self.netDataList.append(datalist)
        self.lock.release()
        self.readable.set()
    def DeleteSession(self, sessionId):
        print("SocketProcessThread: DeleteSession")
        pass

    def getinfo(self, sessionId, now_time, sendDataLen):
        self.times += 1
        self.offset += sendDataLen
        #self.send_packet_num += (sendDataLen > 0)
        self.send_packet_num += 1
        difftime = int((now_time - self.start_time) * 1000000)
        if difftime > self.interval and self.start_time > 0:
            freq = int((self.times * 1000000) / difftime)
            print("SendThread: run: access freq= ", freq)
            bitrate = ((self.offset) << 3)  #
            bitrate = int(1000000 * bitrate / difftime)
            # / (1024 * 1024)
            bitrate >>= 10
            # bitrate /= (1024 * 1024)
            if bitrate > 0:
                if not MULT_SERVER:
                    sessionId = -1
                if bitrate > 10240:
                    bitrate = (bitrate + 512) >> 10
                    print("SendThread: run: sendto: bitrate(Mbps)= ", (sessionId, bitrate))
                else:
                    print("SendThread: run: sendto: bitrate(Kbps)= ", (sessionId, bitrate))
            self.start_time = now_time
            #self.send_packet_num = 0
            self.times = 0
            self.offset = 0
        return difftime

    def RelayProcess(self, data):
        (revcData, remoteHost, remotePort, recv_time) = data
        sockId = remoteHost + "_" + str(remotePort)
        self.lock2.acquire()
        # sessionDict = copy.deepcopy(self.sessionDict)
        # socketDict = copy.deepcopy(self.socketDict)
        # self.lock2.release()
        sockItem = self.socketDict.get(sockId)
        sent = False
        sumlen = 0
        sessionId = -1
        if sockItem != None:
            (sockThread, sessionId, chanId, actor, selfmode, remoteHost, remotePort) = sockItem
            sessionInfo = self.sessionDict.get(sessionId)
            if sessionInfo != None:
                #item = revcData.decode().split(",")
                #seqnum = int(item[0])
                #time0 = float(item[1])
                #thisdata = item[2]
                (r1, r2) = self.cmdheader.unpackdata(revcData)
                #print("SocketProcessThread:RelayProcess: r1= ", r1)
                #print("SocketProcessThread:RelayProcess: len(r2)= ", len(r2))
                (idinfo, seqnum, time0) = r1
                netInfo = self.netInfoDict.get(idinfo)
                if netInfo == None:
                    netInfo = NetInfo(idinfo)
                    self.netInfoDict.update({idinfo:netInfo})
                revdata = (seqnum, time0, recv_time, r2)
                netInfo.CountInfo(revdata, idinfo)
                #print("SocketProcessThread:RelayProcess: (seqnum, time0)= ", (seqnum, time0))

                actors = [DECACTOR] #[3]
                #actors = [1, 2, 3] #selfmode
                for thisactor in actors:
                    sockdict = sessionInfo.get(thisactor)
                    if sockdict != None:
                        #for key, value in sockdict.items():
                        keyList = list(sockdict.keys())
                        n = len(keyList)
                        for i in range(n):
                            key = keyList[i]
                            value = sockdict.get(key)
                            if value != None:
                                sockId2 = key
                                chanId2 = value[0]
                                recv_time2 = value[1]
                                sockItem2 = self.socketDict.get(sockId2)
                                if sockItem2 != None:
                                    (sockThread2, sessionId2, chanId2, actor2, selfmode2, remoteHost2, remotePort2) = sockItem2
                                    if sessionId2 == sessionId and (chanId2 == chanId or (selfmode2 and chanId2 != chanId)):
                                        self.lock2.release()
                                        if sockThread2 != None:
                                            sockThread2.PushQueue((revdata, revcData))
                                        else:
                                            sendDataLen = self.sock.sendto(revcData,(remoteHost2, remotePort2))  # relay
                                            sumlen += sendDataLen
                                        self.lock2.acquire()
                                        sent = True

        else:
            #print("error: RelayProcess: (remoteHost, remotePort)= ", (remoteHost, remotePort))
            pass

        self.lock2.release()

        if not MULT_THREAD:
            now_time = time.time()
            if self.start_time == 0 and sent:
                self.start_time = now_time
            difftime = self.getinfo(sessionId, now_time, sumlen)
    def ProcessData(self):
        #datalist = self.PopQueue()
        datalist = self.PopQueueAll() #slower than push as the flollow case, it will consume too much time
        if datalist != [] and True:
            for thisDataList in datalist:
                for data in thisDataList:
                    (revcData, remoteHost, remotePort, recv_time) = data
                    sockId = remoteHost + "_" + str(remotePort)
                    n = len(revcData)
                    #吞吐量取决于CmdProcess/RawProcess的实时性(串行sendto)
                    iscmd = n <= CMD_SIZE and ('{' in str(revcData[0:3])) and ('}' in str(revcData[n - 3:]))
                    if iscmd:
                        iscmd = '"cmd":' in str(revcData)
                    if iscmd:
                        #iscmd = self.CmdProcess(data)
                        self.cmdthread.data_master.PushQueue1(data)
                    else:
                        self.RelayProcess(data)
                        #print("SocketProcessThread: len(revcData)= ", len(revcData))
                        #if len(revcData) < 250:
                        #    print("SocketProcessThread: cmdstr= ", revcData)
                        #print("SocketProcessThread: recv_time= ", recv_time)
        return datalist
    def run(self):
        while self.__running.isSet():
            #self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回

            self.readable.clear()
            self.readable.wait()

            data = self.ProcessData()
            if data == None or data == []:
                #过频繁的访问会导致锁被占用
                time.sleep(0.001)
                #self.readable.clear()
                #self.readable.wait()
            else:
                #time.sleep(0.00001) #影响收包效率!!!
                pass
    def stop(self):
        if self.cmdthread != None:
            self.cmdthread.stop()
        self.readable.set()
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals
        for key, value in self.socketDict.items():
            if value != None:
                value.stop()
        print("SocketProcessThread: stop")

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞
#数据集中处理
class DataProcessThread(threading.Thread):
    def __init__(self, server):
        threading.Thread.__init__(self)
        self.server = server
        self.data_master = self.server.data_master
        self.start_time = 0
        self.times = 0
        self.interval = INTERVAL
        ###
        self.sockProcess = None
        self.sockProcess = SocketProcessThread(server)
        #self.sockProcess.setPriority(2)
        self.sockProcess.setDaemon(True)
        self.sockProcess.start()

        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True

    def run(self):
        while self.__running.isSet():
            self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            #self.data_master.readable.clear()
            #self.data_master.readable.wait()
            now_time = time.time()
            #核心数据临界，对等读取速度,首要的是不要妨碍push数据
            #在不阻塞push数据的前提下，pop数据
            #理想状态是在push数据的间隙，pop数据
            #datalist = self.data_master.PopQueue() #单次阻塞push的时间更短
            datalist = self.data_master.PopQueueAll() #减少阻塞push的频率
            #datalist = self.data_master.PopQueueGroup(30)
            if self.sockProcess != None:
                self.sockProcess.PushQueue(datalist)
            # datalist = self.PopQueueAll() #slower than push as the flollow case, it will consume too much time
            if datalist == None or datalist == []:# or len(datalist) < 15:
                #过频繁的访问会导致锁被占用
                time.sleep(0.001)
                #time.sleep(0.0009)
                #self.data_master.readable.clear()
                #self.data_master.readable.wait() #访问响应更快，太快则会妨碍push数据
            else:
                #time.sleep(0.001)
                #time.sleep(0.00001) #影响收包效率!!!
                if self.start_time == 0:
                    self.start_time = now_time
                pass
            self.times += 1

            difftime = int((now_time - self.start_time) * 1000000)
            if difftime > self.interval and self.start_time > 0:
                freq = int((self.times * 1000000) / difftime)
                #print("DataProcessThread: run: access freq= ", freq)
                self.start_time = now_time
                self.times = 0

    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals
        if self.sockProcess != None:
            self.sockProcess.stop()
        print("DataProcessThread: stop")

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞
class PortParams(object):
    def __init__(self):
        self.actor = 0
        self.sessionId = 0
        self.chanId = 0
        self.modeId = 0
        self.avtype = 0
        self.selfmode = 0
        self.width = WIDTH
        self.height = HEIGHT
        self.testmode = True
        self.nettime = 0
        self.port = global_port
        self.host = global_host
        self.idx = 0
        self.windId = 0
        self.isEncoder = 0

class EchoClientThread(threading.Thread):
    def __init__(self, params):
        threading.Thread.__init__(self)
        print("EchoClientThread: __init__")
        self.lock = threading.Lock()
        self.params = params
        self.actor = params.actor
        self.sessionId = params.sessionId
        self.chanId = params.chanId
        self.idx = params.chanId
        self.modeId = params.modeId
        self.avtype = params.avtype
        self.selfmode = params.selfmode
        self.testmode = params.testmode
        self.nettime = params.nettime
        self.width = params.width
        self.height = params.height
        self.status = False
        params.idx = params.chanId
        #self.actor = actor
        self.host = params.host
        self.port = params.port
        global DATA_SIZE
        global AUDIO_DATA_SIZE
        if self.avtype == 1:
            #DATA_SIZE = AUDIO_DATA_SIZE
            self.port += 1
        print("EchoClientThread: (host, port)= ", (self.host, self.port))
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        #self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, SO_SNDBUF)
        #self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, SO_RCVBUF)
        self.revbufsize = self.sock.getsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF)
        #revbufsize >>= 10
        print("revbufsize(kB)= ", self.revbufsize >> 10)
        if (platform.system() == 'Windows'):
            #val = struct.pack("Q", 5050)
            val = struct.pack("Q", 500) #500ms
            self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVTIMEO, val)
            print("EchoClientThread: windows")
        else:
            #self.sock.settimeout(100)
            #val = struct.pack("QQ", 5, 50000)  # 5.05s
            val = struct.pack("QQ", 0, 500000) #500ms
            self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVTIMEO, val)
        if False:
            self.sock.setblocking(False)
            self.sock.settimeout(10)
        print("EchoClientThread: __init__ 0")
        sucess = self.Register()
        if sucess == False:
            return
        self.init()
        print("EchoClientThread: __init__ ok")

    def __del__(self):
        print("EchoClientThread del")

    def init(self):
        print("EchoClientThread init")
        self.status = True
        self.data_master = None
        self.dataprocess = None
        self.netInfo = None

        self.sendthread = None
        if self.actor != DECACTOR:
            #if testmode == False: push data to SendThread
            self.sendthread = SendThread(self, self.testmode, 2)  # "send_data"
        self.recvthread = None
        self.recvthread = RecvThread(self)
        self.flag = threading.Event()  # 用于暂停线程的标识
        self.flag.set()  # 设置为True
        self.running = threading.Event()  # 用于停止线程的标识
        self.running.set()  # 将running设置为True

        if self.actor != DECACTOR:#encoder
            if self.sendthread != None:
                self.sendthread.setDaemon(True)
                self.sendthread.start()
                #self.sendthread.startheartbeat()
        if self.recvthread != None:
            self.recvthread.setDaemon(True)
            self.recvthread.start()
            if self.actor == DECACTOR:#decoder
                #self.recvthread.startheartbeat()
                pass
            self.data_master = self.recvthread.data_master
            idx = (self.sessionId << 16) + self.chanId
            self.dataprocess = DataProcess(self, idx)
            self.netInfo = self.dataprocess.netInfo

        self.interval = INTERVAL
        print("EchoClientThread init ok!")
        return

    def CmdCore(self, cmd):
        str_cmd = json2str(cmd)
        acked = False
        count = 0
        while (acked == False):
            self.lock.acquire()
            sendDataLen = self.sock.sendto(str_cmd, (self.host, self.port))
            self.lock.release()
            try:
                rdata = self.sock.recvfrom(CMD_SIZE)
            # except IOError, error:  # python2
            except IOError as error:  # python3
                print("EncoderClient: CmdCore: recvfrom error= ", error)
                print("EncoderClient: CmdCore: recvfrom count= ", count)
                print("EncoderClient: CmdCore: sendto str_cmd= ", str_cmd)
                if error.errno == errno.EWOULDBLOCK:
                    pass
                elif error.errno == errno.WSAETIMEDOUT:
                    pass
                else:
                    print("EncoderClient: CmdCore: recvfrom error= ", error)
                    break
            else:
                if rdata[0] != '' and rdata[1] != None:
                    acked = True
                    thisDict = str2json(rdata[0])
                    chanId = thisDict.get("chanId")
                    print("EncoderClient:CmdCore: (chanId, self.chanId)=", (chanId, self.chanId))
                    sessionId = thisDict.get("sessionId")
                    #apply chanId
                    if chanId != None:
                        count += 1
                    flag = chanId != None and chanId != self.chanId
                    #flag |= sessionId != None and sessionId != self.sessionId
                    if flag:
                        self.chanId = chanId
                        cmd["chanId"] = self.chanId
                        str_cmd = json2str(cmd)
                        self.lock.acquire()
                        sendDataLen = self.sock.sendto(str_cmd, (self.host, self.port))
                        self.lock.release()
                        print("EncoderClient:CmdCore: sendDataLen=", sendDataLen)

            if count > 10:
                print("EncoderClient:CmdCore: reg fail !", (self.sessionId, self.chanId))
                break
        return acked

    def Register(self):
        cmd = {"cmd": "reg", "actor": self.actor, "sessionId": self.sessionId, "chanId": self.chanId,
               "modeId": self.modeId,
               "selfmode": self.selfmode, "liveTime": LIVETIME}
        ret = self.CmdCore(cmd)
        return ret
    def UnRegister(self):
        cmd = {"cmd": "ureg", "actor": self.actor, "sessionId": self.sessionId, "chanId": self.chanId}
        ret = self.CmdCore(cmd)
        return ret
    def CloseSession(self):
        cmd = {"cmd": "close_session", "actor": self.actor, "sessionId": self.sessionId, "chanId": self.chanId}
        ret = self.CmdCore(cmd)
        return ret
    def EncodeFrame(self):
        time.sleep(1)
    def run(self):
        print("EchoClientThread: run")
        while self.running.isSet():
            self.flag.wait()  #会延缓发送频率
            #print("EchoClientThread run")
            now_time = time.time()
            #decoder data process and encoder rtt/lossrate process
            if self.data_master != None:
                #print("EchoClientThread run: self.actor= ", self.actor)
                datalist = self.data_master.PopQueueAll()
                if datalist == None or datalist == []:  # or len(datalist) < 15:
                    # 过频繁的访问会导致锁被占用
                    time.sleep(0.001)
                    #print("EchoClientThread run: 2: self.actor= ", self.actor)
                else:
                    for data in datalist:
                        (revcData, remoteHost, remotePort, recv_time) = data
                        if self.dataprocess != None:
                            n = len(revcData)
                            iscmd = n <= CMD_SIZE and ('{' in str(revcData[0:3])) and ('}' in str(revcData[n - 3:]))
                            if iscmd:
                                iscmd = '"cmd":' in str(revcData)
                            if iscmd:
                                #print("EchoClientThread run: 0: self.actor= ", self.actor)
                                self.dataprocess.ProcessCmd(data)
                            else:
                                #print("EchoClientThread run: 1: self.actor= ", self.actor)
                                self.dataprocess.ProcessRaw(data)
            else:
                #print("EchoClientThread run: self.actor= ", self.actor)
                self.EncodeFrame()
        print("EchoClientThread run over")
        #self.sock.close()

    def stop(self):
        self.flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.running.clear()  # 设置为Fals
        print("EchoClientThread: start stop")
        if self.sendthread != None:
            self.sendthread.stop()
        if self.recvthread != None:
            self.recvthread.stop()
        if self.sendthread != None:
            self.sendthread.dead.wait()
        if self.recvthread != None:
            self.recvthread.dead.wait()
        try:
            self.sock.shutdown(socket.SHUT_RDWR)
            self.sock.close()
            pass
        # except IOError, error:  # python2
        except IOError as error:  # python3
            print("EchoClientThread: stop error: ", error)
        else:
            print("stop ok")

        print("EchoClientThread: stop over")

    def pause(self):
        self.flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.flag.set()  # 设置为True, 让线程停止阻塞


class EchoServerThread(threading.Thread):
    def __init__(self, params):
        threading.Thread.__init__(self)
        self.lock = threading.Lock()
        ###
        self.params = params
        self.idx = params.idx
        self.avtype = params.avtype
        self.actor = 0
        self.sessionId = -1
        if MULT_SERVER:
            self.sessionId = params.idx
        self.chanId = -1
        self.host = params.host
        self.port = params.port
        global DATA_SIZE
        global AUDIO_DATA_SIZE
        if self.avtype == 1:
            #DATA_SIZE = AUDIO_DATA_SIZE
            self.port += 1
        self.tid = None
        self.setName("EchoServerThread")
        ###
        self.load = loadserver.gload #LoadLib()
        if self.load == None:
            self.status = False
            return
        array_type = c_char_p * 4
        self.outparam = array_type()


        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        #self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, SO_SNDBUF)
        #self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, SO_RCVBUF)
        self.revbufsize = self.sock.getsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF)
        #self.revbufsize >>= 10
        print("revbufsize(kB)= ", self.revbufsize >> 10)
        if (platform.system() == 'Windows'):
            #val = struct.pack("Q", 5050)
            val = struct.pack("Q", 500) #500ms
            self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVTIMEO, val)
        else:
            #self.sock.settimeout(100)
            #val = struct.pack("QQ", 5, 50000)  # 5.05s
            val = struct.pack("QQ", 0, 500000) #500ms
            self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVTIMEO, val)
        print("EchoServerThread: self.host= ", self.host)
        print("EchoServerThread: self.port= ", self.port)
        try:
            self.sock.bind((self.host, self.port))  # 绑定同一个域名下的所有机器
        # except IOError, error:  # python2
        except IOError as error:  # python3
            print("EchoServerThread: fail to bind: error= ", error)
        if False:
            self.sock.setblocking(False)
            self.sock.settimeout(10)

        self.sendthread = None
        #self.sendthread = SendThread(self)
        self.recvthread = None
        self.recvthread = RecvThread(self)
        ###
        self.flag = threading.Event()  # 用于暂停线程的标识
        self.flag.set()  # 设置为True
        self.running = threading.Event()  # 用于停止线程的标识
        self.running.set()  # 将running设置为True

        self.data_master = None
        if self.sendthread != None:
            self.sendthread.setDaemon(True)
            self.sendthread.start()
        if self.recvthread != None:
            self.data_master = self.recvthread.data_master
            self.recvthread.setDaemon(True)
            self.recvthread.start()

        #self.data_master = DataManager(self.revbufsize)
        #self.process = None

        self.process = DataProcessThread(self)
        #self.process.setPriority(2)
        self.process.setDaemon(True)
        self.process.start()
    def __del__(self):
        print("EchoServerThread del")

    def run(self):
        #与客户端线程数不对等，导致访问频率减缓
        name = self.getName()
        print("name= ", name)
        while self.running.isSet():
            self.flag.wait()   # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            #print("EchoServerThread run")
            time.sleep(1)
        print("EchoServerThread: run over")
    def stop(self):
        if self.data_master != None:
            self.data_master.readable.set()
        self.flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.running.clear()  # 设置为Fals
        print("EchoServerThread: start stop")
        if self.sendthread != None:
            self.sendthread.stop()
        if self.recvthread != None:
            self.recvthread.stop()
        if self.sendthread != None:
            self.sendthread.dead.wait()
        if self.recvthread != None:
            self.recvthread.dead.wait()
        if self.process != None:
            self.process.stop()
        try:
            self.sock.shutdown(socket.SHUT_RDWR)
            self.sock.close()
            pass
        #except IOError, error:  # python2
        except IOError as error:  # python3
            print("EchoServerThread: stop self.actor=: ", self.actor)
            print("EchoServerThread: stop error: ", error)
        else:
            print("stop ok")

        print("EchoServerThread: stop over")
    def pause(self):
        self.flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.flag.set()  # 设置为True, 让线程停止阻塞
def RunServer(idx, speakernum, host, port):
    if True:
        # idx = raw_input('please input to exit(eg: -1 ): ')
        # count = 0
        params = PortParams()
        (params.idx, params.actor) = (idx, 0)
        (params.modeId, params.avtype, params.selfmode) = (DEFAULT_MODE, 0, 0)
        (params.host, params.port) = (host, port)
        thread = EchoServerThread(params)
        thread.setDaemon(True)
        thread.start()
        #thread.setPriority(THREAD_PRIORITY_ABOVE_NORMAL)
        while idx >= 0 and idx < 4:
            try:
                idx = int(raw_input('please input to exit(eg: 0 ): '))
            except:
                idx = int(input('please input to exit(eg: 0 ): '))
            print("idx= ", idx)
            #thread.status = False if idx == 0 else True
        print("main: start stop...")
        thread.stop()
        #thread.join()
        #time.sleep(5)
def RunClient(base_idx, proc_idx, speakernum, host, port):
    global global_render
    if True:
        # idx = raw_input('please input to exit(eg: -1 ): ')
        # count = 0
        if global_render != None:
            global_render.setDaemon(True)
            global_render.start()
        idx = 0
        #clientnum = 1#16#2#1#16 #2#1#2
        threadnum = speakernum * speakernum
        threadlist = [None for x in range(threadnum)]
        for i in range(base_idx, speakernum, 1):
            for j in range(speakernum):
                #actor_idx = proc_idx * threadnum + i
                k = i * speakernum + j
                if (i | j) == 0:
                    actor = 1
                elif(j == 0):
                    actor = 2
                else:
                    actor = DECACTOR
                params = PortParams()
                (params.sessionId, params.chanId, params.actor) = (proc_idx, i, actor)
                (params.modeId, params.avtype, params.selfmode) = (DEFAULT_MODE, 0, 0)
                (params.host, params.port) = (host, port)
                threadlist[k] = EchoClientThread(params)
                if threadlist[k].status == False:
                    print("RunClient: fail: threadlist: k=", k)
                    del threadlist[k]
                    continue
                threadlist[k].setDaemon(True)
                threadlist[k].start()
                # threadlist[i].setPriority(1)
        while idx >= 0 and idx < 4:
            try:
                idx = int(raw_input('please input to exit(eg: 0 ): '))
            except:
                idx = int(input('please input to exit(eg: 0 ): '))
            print("idx= ", idx)
            #thread.status = False if idx == 0 else True
        print("main: start stop...")
        for i in range(threadnum):
            for j in range(speakernum):
                k = i * speakernum + j
                if threadlist[k] != None:
                    threadlist[k].stop()

        if global_render != None:
            global_render.stop()
        #thread.join()
        #time.sleep(5)

def RunSession(actor, base_idx, proc_idx, speakernum, host, port):
    print("RunSession开始执行,进程号为%d" % (os.getpid()))
    if actor == 0:
        RunServer(proc_idx, speakernum, host, port)
    else:
        RunClient(base_idx, proc_idx, speakernum, host, port)
def worker(msg, a, b, c):
    print("%s开始执行,进程号为%d"%(msg, os.getpid()))
    print("(msg, a, b, c)=",(msg, a, b, c))
    sum = msg + a + b
    print("sum= ", sum)
    time.sleep(1)
    print("%s执行完毕"%(msg))
if __name__ == "__main__":
    print("start udptest")
    if global_actor == 0 and False:
        RunSession(global_actor, global_baseidx, 0, global_speakernum, global_host, global_port)
    else:
        print('父进程%d.' % os.getpid())
        # p = Process(target=RunSession, args=('test',))
        # print('子进程将要执行')
        # p.start()
        po = Pool(global_procnum)  # 定义一个进程池，最大进程数3
        for i in range(global_procnum):
            # Pool.apply_async(要调用的目标,(传递给目标的参数元祖,))
            # 每次循环将会用空闲出来的子进程去调用目标
            #po.apply_async(worker, (i, 1, 2, "strname"))
            port = global_port + i
            if MULT_SERVER:
                port = global_port + (i << 1) #(video, audio)
            if global_actor in [1]:
                po.apply_async(RunSession, (global_actor, global_baseidx, i, global_speakernum, global_host, port))
            else:
                po.apply_async(RunSession, (global_actor, global_baseidx, (i >> 1), global_speakernum, global_host, port))
        idx = 0
        while idx >= 0 and idx < 4:
            try:
                idx = int(raw_input('please input to exit(eg: 0 ): '))
            except:
                idx = int(input('please input to exit(eg: 0 ): '))
            print("idx= ", idx)
            #thread.status = False if idx == 0 else True
        print("main: start stop...")
        print("----start----")
        po.close()  # 关闭进程池，关闭后po不再接收新的请求
        po.join()  # 等待po中所有子进程执行完成，必须放在close语句之后

    print("end udptest")

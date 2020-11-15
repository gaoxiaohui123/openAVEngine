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
import signal
import time
import json
import numpy as np
import cv2
from ctypes import *
import math

import loadlib
from callvideoencode import CallVideoEncode
from callvideodecode import CallVideoDecode
from mysdl import MySDL
from capture import VideoCapture

CMD_SIZE = 256
DATA_SIZE = 1500
SO_SNDBUF = 1024 * 1024
SO_RCVBUF = 1024 * 1024
LOSS_RATE = 0.2#0.4 #0.2  # 0.8 #0.6 #0.2
DISTILL_LIST = [0, 1]  # [0] #[0, 1]
#DISTILL_LIST = [0]
EXCHANGE = 0# 1#0#1
SHOW_WIDTH = loadlib.WIDTH
SHOW_HEIGHT = loadlib.HEIGHT
SHOW_TYPE = 0# 1
SHOW_POLL_TIME = 10  # 5
SVC_REFS = 2#16#2
QOS_LEVEL = 1#0#1#3#2#1#0

global_port = 10088 #8097  # 8888
global_host = 'localhost'
# global_host = '172.16.0.17'
# global_host = '111.230.226.17'
if len(sys.argv) > 2:
    global_port = int(sys.argv[2])
if len(sys.argv) > 1:
    global_host = sys.argv[1]


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
def data2str(data):
    try:
        if sys.version_info >= (3, 0):
            outjson = json.loads(data.decode())
        else:
            outjson = json.loads(data, encoding='utf-8')
    except:
        try:
            outjson = json.loads(data)
        except:
            print("data2str: not str")
            # print("not cmd: str_cmd=", str_cmd)
        else:
            pass
    else:
        pass
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
            print("ReadCmd: not cmd")
            # print("not cmd: str_cmd=", str_cmd)
        else:
            pass
    else:
        pass
    return outjson


def get_13_timestamp():
    # 函数功能：获取当前时间的时间戳（13位）
    # 13位时间戳的获取方式跟10位时间戳获取方式一样
    # 两者之间的区别在于10位时间戳是秒级，13位时间戳是毫秒级
    timestamp = time.time()
    return int(round(timestamp) * 1000)


class FrameBuffer(object):
    def __init__(self, id):
        self.id = id
        self.lock = threading.Lock()
        self.framelist = []
        self.init_timestamp = 0  # 防止漂移
        self.init_start_time = 0  # 防止漂移
        self.base_timestamp = 0
        self.base_start_time = 0
        self.time_offset = 0
        self.last_timestamp = 0
        self.search_count = 0
        self.audio_timestamp = 0
        self.audio_start_time = 0
        self.audio_frequence = 0
    def __del__(self):
        print("FrameBuffer del")
        for thisobj in self.framelist:
            del thisobj
    def SetAudioBase(self, timestamp, play_time, frequence):
        self.audio_timestamp = timestamp
        self.audio_start_time = play_time
        self.audio_frequence = frequence


class ShowThread(threading.Thread):
    def __init__(self, winhnd, screen_width, screen_height, width, height):
        threading.Thread.__init__(self)
        self.lock = threading.Lock()
        self.show_lock = threading.Lock()
        self.load = loadlib.gload
        self.winhnd = winhnd
        self.sdl = MySDL(self.winhnd, screen_width, screen_height)
        (self.sdl.width, self.sdl.height) = (width, height)
        self.sdl.init()
        self.sdl.start()
        self.DataList = []
        self.FrameList = []
        self.RectList = []
        self.way = 0
        self.show_type = SHOW_TYPE
        self.start_time = 0
        self.show_poll_time = SHOW_POLL_TIME
        self.poll_idx = 1
        self.poll_times = 0
        self.idMap = []
        self.father = None
        self.height = height
        self.width = width
        self.max_delay_time = 0
        self.max_delay_packet = -1
        array_type = c_char_p * 4
        self.outparam = array_type()

        handle_size = 16
        self.ll_handle = create_string_buffer(handle_size)
        #self.ll_handle = (c_char * handle_size)()
        array_type = c_char_p * 1
        self.ctime = array_type()
        # cv2.namedWindow("dec-frame", cv2.WINDOW_NORMAL)
        # self.show_tab = [(1,1),()]
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
    def __del__(self):
        print("ShowThread del")
        for thisobj in self.DataList:
            del thisobj
        del self.DataList
        for thisobj in self.FrameList:
            del thisobj
        del self.FrameList
        for thisobj in self.RectList:
            del thisobj
        del self.RectList
        for thisobj in self.idMap:
            del thisobj
        del self.idMap
        if self.sdl != None:
            del self.sdl
        if self.outparam != None:
            del self.outparam
    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞

    def SetRects(self, rectList):
        self.RectList = rectList
        print("SetRects: self.RectList=", self.RectList)
    def InsertId(self, id):
        self.idMap.append(id)

    def PopQueue(self):
        ret = None
        self.lock.acquire()
        if len(self.DataList) > 0:
            flag = len(self.DataList) > 10
            if flag:
                print("ShowThread: 0: len(self.DataList)= ", len(self.DataList))
            ret = self.DataList[0]
            del (self.DataList[0])
            if flag:
                print("ShowThread: 1: len(self.DataList)= ", len(self.DataList))
        self.lock.release()
        return ret

    def PushQueue(self, id, data, recvTime):
        self.lock.acquire()
        self.DataList.append((id, data, recvTime))
        self.lock.release()

    def data2img(self, data0):
        (h, w) = (self.height, self.width)
        # print("data2img:  (h, w)= ", (h, w))
        frame0 = np.frombuffer(data0, dtype=np.uint8)
        yuv_I420_0 = frame0.reshape((h * 3) / 2, w)
        img0 = cv2.cvtColor(yuv_I420_0, cv2.COLOR_YUV2BGR_I420)
        return img0

    def mult_windows(self, father, son, idx, num, cut):
        if cut:
            (h, w) = (son.shape[0], son.shape[1])
            (x0, y0, x1, y1) = (64, 64, w - 64, h - 64)
            cropImg = son[y0:y1, x0:x1]
            son = cropImg
        if np.all(father) == None:
            # father = cv2.resize(son, (1280, 720), interpolation=cv2.INTER_CUBIC)
            father = np.ones((self.height, self.width, 3), dtype="uint8")
        # print(round(math.sqrt(2)))
        # print(int(math.sqrt(2) + 0.9999999999))
        n = int(math.sqrt(num) + 0.9999999999)
        (h, w) = (father.shape[0], father.shape[1])
        m = n
        # if num < len(showlist):
        #    (m, n) = showlist[num]
        #    print("(m, n)= ", (m, n))
        hn = int(h / n)
        wn = int(w / m)
        print("(wn, hn)= ", (wn, hn))
        son2 = cv2.resize(son, (wn, hn), interpolation=cv2.INTER_CUBIC)
        i = int(idx / n)
        j = int(idx % m)
        # print("(i, j)= ", (i, j))
        y0 = int(i * hn)
        y1 = int((i + 1) * hn)
        x0 = int(j * wn)
        x1 = int((j + 1) * wn)
        father[y0:y1, x0:x1] = son2
        return father

    def exchange_id(self):
        n = len(self.idMap)
        id0 = self.idMap[0]
        id1 = self.idMap[self.poll_idx]
        self.idMap[0] = id1
        self.idMap[self.poll_idx] = id0
        self.poll_idx += 1
        if self.poll_idx > (n - 1):
            self.poll_idx = 1

    def get_rect(self, id):
        (h, w) = (self.height, self.width)
        #print("ShowThread: get_rect: (h, w)=", (h, w))
        ret = None #(0, 0, w, h)
        idx = self.idMap.index(id)
        if len(self.RectList) > 0 and True:
            if idx < len(self.RectList):
                ret = self.RectList[idx]
                return ret
        #print("self.idMap= ", self.idMap)
        #print("(idx, id)= ", (idx, id))

        if self.show_type in [1]:
            if self.start_time == 0:
                self.start_time = time.time()

            difftime = (int)((time.time() - self.start_time))
            poll_n = difftime / self.show_poll_time
            poll_idx = difftime % self.show_poll_time
            #print("get_rect: (poll_n, poll_idx)= ", (poll_n, self.poll_times, poll_idx))
            # poll_idx = poll_n % num
            if poll_idx in [0] and poll_n == self.poll_times:
                self.exchange_id()
                self.poll_times += 1
                print("poll show")
            idx = self.idMap.index(id)
            (w2, h2) = ((w >> 2), (h >> 2))
            if idx in [0]:
                return (0, 0, w, h)
            else:
                if idx in [1]:
                    ret = (0, 0, w2, h2)
                elif idx in [2]:
                    ret = (0, h2, w2, h2)
                elif idx in [3]:
                    ret = (0, (h2 << 1), w2, h2)
                elif idx in [4]:
                    ret = (0, (h2 << 1) + h2, w2, h2)
                elif idx in [5]:
                    ret = (w2, h - h2, w2, h2)
                elif idx in [6]:
                    ret = (w2 << 1, h - h2, w2, h2)
                elif idx in [7]:
                    ret = ((w2 << 1) + w2, h - h2, w2, h2)
                elif idx in [8]:
                    ret = ((w2 << 2), h - h2, w2, h2)
                return ret
        else:
            (w2, h2) = ((w >> 2), (h >> 2))
            if idx in [0]:
                return (0, 0, w, h)
            else:
                #return ret #test
                if idx in [1]:
                    ret = (w - w2, 0, w2, h2)
                elif idx in [2]:
                    ret = (0, h2, w2, h2)
                elif idx in [3]:
                    ret = (0, (h2 << 1), w2, h2)
                elif idx in [4]:
                    ret = (0, (h2 << 1) + h2, w2, h2)
                elif idx in [5]:
                    ret = (w2, h - h2, w2, h2)
                elif idx in [6]:
                    ret = (w2 << 1, h - h2, w2, h2)
                elif idx in [7]:
                    ret = ((w2 << 1) + w2, h - h2, w2, h2)
                elif idx in [8]:
                    ret = ((w2 << 2), h - h2, w2, h2)
                return ret
        num = len(self.idMap)
        n = int(math.sqrt(num) + 0.9999999999)
        m = n

        hn = int(h / n)
        wn = int(w / m)
        # print("(wn, hn)= ", (wn, hn))

        i = int(idx / n)
        j = int(idx % m)
        # print("(i, j)= ", (i, j))
        y0 = int(i * hn)
        y1 = int((i + 1) * hn)
        x0 = int(j * wn)
        x1 = int((j + 1) * wn)
        return (x0, y0, x1 - x0, y1 - y0)

    def show_yuv(self, frame, id):
        cv2.namedWindow("dec-frame" + str(id), cv2.WINDOW_NORMAL)
        cv2.imshow("dec-frame" + str(id), frame)
        cv2.waitKey(1)

    #def play_right_now(self, data, id):
    #    self.show_lock.acquire()
    #    img = self.data2img(data)
    #    # print("ShowThread img.shape= ", img.shape)
    #    self.father = self.mult_windows(self.father, img, id, self.way, 0)
    #    if np.all(self.father) != None:
    #        self.show_yuv(self.father, 0)
    #    self.show_lock.release()

    #def play_right_now2(self, data, id):
    #    self.show_lock.acquire()
    #    img = self.data2img(data)
    #    # print("ShowThread img.shape= ", img.shape)
    #    self.father = self.mult_windows(self.father, img, id, self.way, 0)
    #    # if np.all(self.father) != None:
    #    #    self.show_yuv(self.father, 0)
    #    self.show_lock.release()

    def play_right_now3(self, data, id, show_flag):
        self.show_lock.acquire()
        if (self.sdl.sdl_status() == 0):
            rect = self.get_rect(id)
            if rect != None:
                self.sdl.sdl_refresh(data, rect, show_flag)
        else:
            self.sdl.sdl_stop()
            self.stop()
        self.show_lock.release()

    def sdl_stop(self):
        self.sdl.sdl_stop()
        self.sdl.stop()
    def run(self):
        show_flag = 0
        if len(self.idMap) == 1:
            show_flag = 1
        while self.__running.isSet():
            self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            item = self.PopQueue()
            if item != None:
                now_time = time.time()
                (id, data, recvTime) = item
                ###
                # print("ShowThread start show: id= ", id)
                # print("ShowThread start show: len(data)= ", len(data))
                if len(data) > 0:
                    thisFrmBuf = None
                    for frameBuff in self.FrameList:
                        if frameBuff.id == id:
                            thisFrmBuf = frameBuff
                            break
                    if thisFrmBuf == None:
                        thisFrmBuf = FrameBuffer(id)
                        self.FrameList.append(thisFrmBuf)
                    if thisFrmBuf.last_timestamp and (recvTime < thisFrmBuf.last_timestamp):
                        if sys.version_info >= (3, 0):
                            thisFrmBuf.time_offset += int(1 << 32) - 1
                        else:
                            thisFrmBuf.time_offset += long(1 << 32) - 1
                    if thisFrmBuf.base_timestamp == 0:
                        thisFrmBuf.base_timestamp = recvTime
                        thisFrmBuf.init_timestamp = recvTime
                    else:
                        pass
                    if thisFrmBuf.base_start_time == 0:
                        thisFrmBuf.base_start_time = now_time
                        thisFrmBuf.init_start_time = now_time
                        self.play_right_now3(data, id, show_flag)
                    else:
                        difftime = (recvTime + thisFrmBuf.time_offset - thisFrmBuf.base_timestamp) / 90  # (ms)
                        pre_play_time = thisFrmBuf.base_start_time * 1000 + difftime
                        delay = (now_time - thisFrmBuf.base_start_time) * 1000 - difftime  # (ms)
                        if id in [1]:
                            # print("ShowThread start show: id= ", id)
                            # print("ShowThread start show: int(delay)= ", int(delay))
                            pass
                        # 理想状态delay == 0
                        # 延迟到达delay > 0
                        # 早到delay < 0
                        if delay >= 0:
                            if len(thisFrmBuf.framelist) == 0:
                                if ((delay) > 100) and False:
                                    thisFrmBuf.base_timestamp = recvTime
                                    thisFrmBuf.base_start_time = now_time
                                # show right now ???
                                self.play_right_now3(data, id, show_flag)
                                pass
                            else:
                                thisFrmBuf.framelist.append((data, recvTime, now_time))
                        else:
                            if len(thisFrmBuf.framelist) > 0:
                                first_frame = thisFrmBuf.framelist[0]
                                recvTime0 = first_frame[1]
                                difftime = (recvTime0 + thisFrmBuf.time_offset - thisFrmBuf.base_timestamp) / 90  # (ms)
                                pre_play_time0 = thisFrmBuf.base_start_time * 1000 + difftime  # (ms)
                                delay0 = now_time * 1000 - pre_play_time0
                                if delay0 >= 0:
                                    self.play_right_now3(first_frame[0], id, show_flag)
                                    del thisFrmBuf.framelist[0]
                                thisFrmBuf.framelist.append((data, recvTime, now_time))
                            else:
                                if ((-delay) > 100) and True:
                                    thisFrmBuf.base_timestamp = recvTime
                                    thisFrmBuf.base_start_time = now_time
                                    thisFrmBuf.search_count += 1
                                    if id in [1]:
                                        # print("ShowThread start show: thisFrmBuf.search_count= ", thisFrmBuf.search_count)
                                        pass
                                    self.play_right_now3(data, id, show_flag)
                                else:
                                    thisFrmBuf.framelist.append((data, recvTime, now_time))
                        n = len(thisFrmBuf.framelist)
                        if n > 10:
                            (data0, recvTime0, now_time0) = thisFrmBuf.framelist[0]
                            (data1, recvTime1, now_time1) = thisFrmBuf.framelist[n - 1]
                            difftime0 = recvTime1 - recvTime0
                            difftime1 = now_time1 - now_time0
                            if difftime0 > 100 or difftime1 > 100:
                                print("ShowThread: run: (id, n, difftime0, difftime1)= ",
                                      (id, n, difftime0, difftime1))
                    thisFrmBuf.last_timestamp = recvTime
                # print("ShowThread show")
            else:
                flag = False
                # 循环检测，不另外使用定时器
                for thisFrmBuf in self.FrameList:
                    n = len(thisFrmBuf.framelist)
                    if n > 0:
                        first_frame = thisFrmBuf.framelist[0]
                        last_frame = thisFrmBuf.framelist[n - 1]
                        recvTime0 = first_frame[1]
                        recvTime1 = last_frame[1]
                        saveTime0 = first_frame[2]
                        saveTime1 = last_frame[2]
                        difftime = (recvTime0 + thisFrmBuf.time_offset - thisFrmBuf.base_timestamp) / 90  # (ms)
                        pre_play_time0 = thisFrmBuf.base_start_time * 1000 + difftime  # (ms)
                        difftime = (recvTime1 + thisFrmBuf.time_offset - thisFrmBuf.base_timestamp) / 90  # (ms)
                        pre_play_time1 = thisFrmBuf.base_start_time * 1000 + difftime  # (ms)
                        now_time_ms = time.time() * 1000
                        # pre_play_time1 = pre_play_time1 if pre_play_time1 > now_time_ms else now_time_ms
                        strideTime = (pre_play_time1 - pre_play_time0)  # 分配播放跨度
                        interval = strideTime / n
                        delay0 = (pre_play_time0 - interval) - now_time_ms
                        if id in [0, 1, 2, 3]:
                            test_item = []
                            test_item.append((int(strideTime), int(interval), n))
                            # print("ShowThread start show: test_item= ", test_item)
                        if delay0 <= 0:  # 过点了
                            self.play_right_now3(first_frame[0], thisFrmBuf.id, show_flag)
                            del thisFrmBuf.framelist[0]
                            flag = True
                        else:
                            if id in [1]:
                                # print("ShowThread start show: delay0= ", delay0)
                                pass

                if flag == False:
                    time.sleep(0.01)
                # print("ShowThread sleep")
        print("ShowThread: run : start to sdl_stop")
        self.sdl_stop()
        print("ShowThread: run over")

    def run_0(self):
        while self.__running.isSet():
            self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            item = self.PopQueue()
            if item != None:
                (id, data, recvTime) = item
                if len(data) > 0:
                    # now = time.time()
                    #self.ctime[0] = "0123456789012345"
                    itime = self.load.lib.api_get_time2(self.ll_handle, self.ctime)
                    now = char2long(self.ctime[0])
                    if now == 0:
                        print("itime= ", itime)

                    difftime = int(now - recvTime)
                    if difftime > 500 and False:
                        self.lock.acquire()
                        listlen = len(self.DataList)
                        self.lock.release()
                        print("ShowThread: (id, listlen, difftime)= ", (id, listlen, difftime))
                # 注意：不合理的渲染，会导致“倒帧”；
                show_flag = 0
                if len(self.idMap) == 1:
                    show_flag = 1
                self.play_right_now3(data, id, show_flag)
                # time.sleep(0.001)
                pass

        else:
            time.sleep(0.01)
        print("ShowThread: run : start to sdl_stop")
        self.sdl_stop()
        print("ShowThread: run over")


class UdpClient(object):
    def tcpclient(self):
        clientSock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        clientSock.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 1024 * 1024)
        clientSock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 1024 * 1024)
        sendDataLen = clientSock.sendto("this is send data from client", ('localhost', 9527))
        recvData = clientSock.recvfrom(DATA_SIZE)
        print("sendDataLen: ", sendDataLen)
        print("recvData: ", recvData)
        clientSock.close()

class RecvCmdThread(threading.Thread):
    def __init__(self, client):
        threading.Thread.__init__(self)
        self.client = client
        ###
        array_type = c_char_p * 4
        self.outparam = array_type()

        handle_size = 16
        self.ll_handle = create_string_buffer(handle_size)
        # self.ll_handle = (c_char * handle_size)()
        array_type = c_char_p * 1
        self.ctime = array_type()
        ###
        self.min_rtt = 1000 * 1000
        self.max_rtt = 0
        self.time_offset = 0
        self.rtt_status = 0
        ###
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
    def __del__(self):
        print("RecvCmdThread del")
        if self.outparam != None:
            del self.outparam
        if self.ll_handle != None:
            del self.ll_handle
    def ReadCmd(self, str_cmd):
        cmd = {}
        try:
            if sys.version_info >= (3, 0):
                cmd = json.loads(str_cmd.decode())
            else:
                cmd = json.loads(str_cmd, encoding='utf-8')
        except:
            try:
                cmd = json.loads(str_cmd)
            except:
                print("ReadCmd: not cmd")
                #print("not cmd: str_cmd=", str_cmd)
            else:
                pass
        else:
            pass
        return cmd

    def run(self):
        while self.__running.isSet():
            self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            try:
                ##print("RecvCmdThread: start recvfrom")
                #recvData, (remoteHost, remotePort) = self.client.sock.recvfrom(CMD_SIZE)
                rdata = self.client.sock.recvfrom(CMD_SIZE)
                if rdata[0] != '' and rdata[1] != None:
                    recvData, (remoteHost, remotePort) = rdata
                else:
                    print("rdata= ", rdata)
                    break
            # except:
            #except IOError, error:  # python2
            except IOError as error:  # python3
                print("RecvCmdThread: run: recvfrom error", error)
                break
            else:
                #print("RecvCmdThread: len(recvData)= ", len(recvData))
                self.client.recv_packet_num += 1
                #self.ctime[0] = "0123456789012345"
                itime = self.client.encode0.load.lib.api_get_time2(self.ll_handle, self.ctime)
                c_time_ll = char2long(self.ctime[0])
                if c_time_ll == 0:
                    print("itime= ", itime)
                    continue
                #self.outparam[0] = ""
                #self.client.encode0.load.lib.api_get_time_stamp(self.client.encode0.obj_id, self.outparam)
                #if self.outparam[0] == "":
                #    print("RecvCmdThread: get time fail : id= ", self.client.encode0.obj_id)
                #    continue
                #try:
                #    c_time_ll = long(self.outparam[0])
                #except IOError, error:  # python2
                #    # except IOError as error:  # python3
                #    print("RecvCmdThread: run: error", error)
                #    print("RecvCmdThread: run: self.outparam[0]= ", self.outparam[0])
                #    contine
                cmd = self.ReadCmd(recvData)
                if cmd != {}:
                    rtt_item = cmd.get("rtt")
                    if rtt_item != None:
                        rttlist = rtt_item.get("rttlist")
                        # seqnum 臨界點處理?
                        rttlist = sorted(rttlist, key=lambda s: s[0], reverse=False)
                        # print("RecvCmdThread: rttlist= ", rttlist)
                        st1 = rtt_item.get("st1")
                        rt1 = c_time_ll
                        rtt_list = []
                        for (seqnum, st0, rt0) in rttlist:
                            if self.time_offset == 0:
                                # self.time_offset = (((rt0 - rtt0 - st0) + (st1 + rtt1 - rt1)) >> 1)
                                self.time_offset = int(((rt0 - st0) + (st1 - rt1)) >> 1)
                            rtt = ((int(rt1 - st0) - int(st1 - rt0)) >> 1)

                            if rtt > self.max_rtt:
                                self.max_rtt = rtt
                            if rtt < self.min_rtt:
                                self.min_rtt = rtt
                                self.time_offset = int(((rt0 - st0) + (st1 - rt1)) >> 1)
                            # print("RecvCmdThread: self.time_offset= ", self.time_offset)
                            if self.time_offset == 0:
                                self.time_offset = 1

                            rtt_list.append(rtt)

                            self.client.ack.set()
                        count = 0
                        rtt0 = rtt_list[0]
                        for i in range(1, len(rtt_list), 1):
                            rtt1 = rtt_list[i]
                            if rtt1 > rtt0:
                                count += 1
                            rtt0 = rtt1
                        if count >= (len(rtt_list) - 1):
                            self.rtt_status = 1
                            #print("RecvCmdThread: self.rtt_status= ", self.rtt_status)
                            self.client.ack.set()
                    nack_item = cmd.get("nack")
                    if nack_item != None:
                        pass

        print("RecvCmdThread: exit")

    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals
        #try:
        #    self.client.sock.shutdown(socket.SHUT_RDWR)
        #    self.client.sock.close()
        #except:
        #    print("RecvCmdThread: stop error")
        #else:
        #    print("RecvCmdThread: stop ok")
        print("RecvCmdThread: stop")

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞


class DecodeFrameThread(threading.Thread):
    def __init__(self, client):
        threading.Thread.__init__(self)
        self.lock = threading.Lock()
        self.client = client
        self.framelist = []
        self.idr_status = 0
        self.width = SHOW_WIDTH
        self.height = SHOW_HEIGHT
        #self.ref_idc_list = []
        self.infobuf = create_string_buffer(1024)
        ###
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
    def __del__(self):
        print("DecodeFrameThread del")
        for thisobj in self.framelist:
            del thisobj
        del self.framelist
        if self.infobuf != None:
            del self.infobuf
    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals
        print("DecodeFrameThread: stop")

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞

    def PopQueue(self):
        ret = None
        self.lock.acquire()
        if len(self.framelist) > 0:
            ret = self.framelist[0]
            del (self.framelist[0])
        self.lock.release()
        return ret

    def PushQueue(self, item):
        self.lock.acquire()
        self.framelist.append(item)
        self.lock.release()

    def show_yuv(self, data0, id):
        (h, w) = (self.height, self.width)
        if id in [2]:
            frame0 = np.frombuffer(data0, dtype=np.uint8)
            yuv_I420_0 = frame0.reshape((h * 3) / 2, w)
            img0 = cv2.cvtColor(yuv_I420_0, cv2.COLOR_YUV2BGR_I420)
            # frame1 = np.frombuffer(data1, dtype=np.uint8)
            # yuv_I420_1 = frame1.reshape((h * 3) / 2, w)
            # img1 = cv2.cvtColor(yuv_I420_1, cv2.COLOR_YUV2BGR_I420)
            # img0 = process.DoProcess(img0)
            frame = img0
            # if flag:
            #    frame = np.hstack([img0, img1])  # vstack #hstack
            #    frame = cv2.resize(frame, (frame.shape[1] << 1, frame.shape[0] << 1), interpolation=cv2.INTER_CUBIC)
            cv2.imshow("dec-frame" + str(id), frame)
            cv2.waitKey(1)

    def DecodeFrame(self, item):
        (data4, pktSize, complete, frame_timestamp) = item
        # print("DecodeFrame: complete= ", complete)
        frame_info = complete.split(b",")
        ref_idc = int(frame_info[0]) - 1
        #if ref_idc == 0:
        #    self.ref_idc_list = []
        #    self.ref_idc_list.append(0)
        if QOS_LEVEL:
            if QOS_LEVEL >= 3:
                if frame_info[1] != "complete":
                    #and ref_idc in [3, 4]):
                    if ref_idc in [0, 1]:
                        #wait next idr
                        self.idr_status = ref_idc + 1
                        #return
                    elif (ref_idc in [2]) and (SVC_REFS >= 2) and (self.idr_status == 0):
                        #wait next [0, 1]
                        self.idr_status = ref_idc + 1
                        #return
                else:
                    #self.ref_idc_list.append(ref_idc)
                    if ref_idc in [0]:
                        self.idr_status = 0
                    elif ref_idc in [1]:
                        if self.idr_status > 2: # last loss ref_idc >= 2
                            self.idr_status = 0
                    elif ref_idc in [2] and (SVC_REFS >= 2):
                        if self.idr_status > 3: # last loss ref_idc >= 3
                            self.idr_status = 0
            elif QOS_LEVEL >= 2:
                if frame_info[1] != "complete":
                    #and ref_idc in [3, 4]):
                    if ref_idc in [0, 1]:
                        #wait next idr
                        self.idr_status = ref_idc + 1
                        #return
                    elif (ref_idc in [2]) and (SVC_REFS >= 2) and (self.idr_status == 0):
                        #wait next [0, 1]
                        self.idr_status = ref_idc + 1
                        #return
                else:
                    #self.ref_idc_list.append(ref_idc)
                    if ref_idc in [0]:
                        self.idr_status = 0
                    elif ref_idc in [1]:
                        if self.idr_status > 1: # last loss ref_idc >= 1
                            self.idr_status = 0
                    elif ref_idc in [2] and (SVC_REFS >= 2):
                        if self.idr_status > 2: # last loss ref_idc >= 2
                            self.idr_status = 0
                    #if self.idr_status:
                    #    return
                #print("DecodeFrame: (self.idr_status, ref_idc, frame_info[1])= ", (self.idr_status, ref_idc, frame_info[1]))

            elif QOS_LEVEL >= 1:
                if (frame_info[1] != "complete" and ref_idc in [3, 4]):
                    self.idr_status = ref_idc + 1
                    #return
                else:
                    self.idr_status = 0

        runflag = True
        data = data4
        ret = len(data)
        rtpSize = pktSize
        enable_fec = 0
        if True:
            self.client.decode0.outparam[0] = c_char_p(self.infobuf.raw)
            ret2 = self.client.decode0.load.lib.api_get_extern_info(data4, self.client.decode0.outparam)
            if ret2 > 0:
                if sys.version_info >= (3, 0):
                    outjson = str2json(self.client.decode0.outparam[0].decode())
                else:
                    outjson = str2json(self.client.decode0.outparam[0])
                if outjson != None:
                    enable_fec = outjson["enable_fec"]
            # print("DecodeFrame: enable_fec= ", enable_fec)
        if enable_fec:
            # data4 = self.client.decode0.outbuf.raw[:ret]
            # pktSize = self.client.decode0.outparam[0].split(",")
            self.client.decode0.outparam[0] = b""
            self.client.decode0.outparam[1] = b""
            sizelist = []
            for packet_size in pktSize:
                sizelist.append(int(packet_size))

            if self.client.decode0.param.get("insize") != None:
                del (self.client.decode0.param["insize"])
            self.client.decode0.param.update({"inSize": sizelist})
            #param_str = json.dumps(self.client.decode0.param, encoding='utf-8', ensure_ascii=False, sort_keys=True)
            param_str = json2str(self.client.decode0.param)
            # print("DecodeFrame: out frame api_fec_decode")
            ret = self.client.decode0.load.lib.api_fec_decode(self.client.decode0.handle, data4, param_str,
                                                              self.client.decode0.outbuf,
                                                              self.client.decode0.outparam)
            del (self.client.decode0.param["inSize"])
            if ret > 0 and True:
                # print("DecodeFrame: fec: decode raw size= ", ret)
                pktSize = self.client.decode0.outparam[0].split(b",")
                # print("DecodeFrame: fec: pktSize= ", pktSize)
                # print("DecodeFrame: fec: len(pktSize)= ", len(pktSize))
                data5 = self.client.decode0.outbuf.raw[:ret]

                # del(self.client.decode0.param["inSize"])
                # (param, outbuf, outparam) = self.setparam()

                data = data5
                rtpSize = pktSize
            else:
                print("DecodeFrame: fec: decode raw size= ", ret)
                print("DecodeFrame: fec: len(sizelist)= ", len(sizelist))
        # if "complete" != complete:
        if False:
            outjson = self.str2json(self.client.decode0.outparam[1])
            if outjson != None:
                refs = outjson["refs"]
                ref_idx = outjson["ref_idx"]
                ref_idc = outjson["ref_idc"]
                timestamp = outjson["timestamp"]
                seqnum = outjson["seqnum"]
                if self.client.log_fp != None and False:
                    self.client.log_fp.write("ref_idx= " + str(ref_idx) + "\n")
                    self.client.log_fp.write("timestamp= " + str(timestamp) + "\n")
                    self.client.log_fp.write("seqnum= " + str(seqnum) + "\n")
                    self.client.log_fp.flush()
                print("DecodeFrame: ref_idc= ", ref_idc)
                print("DecodeFrame: ref_idx= ", ref_idx)
                print("DecodeFrame: refs= ", refs)
                if ref_idx in [1, 3, 5, 7] and False:
                    runflag = False
                    pass
                # if ref_idc > 2:
                #    runflag = False
                if "complete" != complete and False:
                    if (ref_idx == 0) or (ref_idx == refs):
                        pass
                    else:
                        runflag = False
        # print("DecodeFrame: runflag= ", runflag)
        if runflag and ret > 0:
            # data = self.client.decode0.outbuf.raw[:ret]
            # rtpSize = self.client.decode0.outparam[0].split(",")
            # print("rtpSize= ", rtpSize)
            ###
            self.client.decode0.outparam[0] = b""
            self.client.decode0.outparam[1] = b""
            sizelist = []
            for packet_size in rtpSize:
                sizelist.append(int(packet_size))
            self.client.decode0.param.update({"rtpSize": sizelist})
            self.client.decode0.param.update({"mtu_size": self.client.decode0.mtu_size})
            # self.client.decode0.param.update({"mtu_size": 150})
            #param_str = json.dumps(self.client.decode0.param, encoding='utf-8', ensure_ascii=False, sort_keys=True)
            param_str = json2str(self.client.decode0.param)
            # print("param_str= ", param_str)
            # 獲取幀後，進行解包
            ret = self.client.decode0.load.lib.api_rtp_packet2raw(self.client.decode0.handle, data, param_str,
                                                                  self.client.decode0.outbuf,
                                                                  self.client.decode0.outparam)
            data4 = self.client.decode0.outbuf.raw[:ret]
            self.client.decode0.param.update({"insize": ret})
            #param_str = json.dumps(self.client.decode0.param, encoding='utf-8', ensure_ascii=False, sort_keys=True)
            param_str = json2str(self.client.decode0.param)
            ###
            # 解包後進行解碼
            start_time = time.time()
            # print("DecodeFrame: out frame api_video_decode_one_frame")
            # ret = self.client.decode0.load.lib.api_video_decode_one_frame(self.client.decode0.obj_id, data4, param_str, self.client.decode0.outbuf, self.client.decode0.outparam)
            ret = self.client.decode0.load.lib.api_video_decode_one_frame(self.client.decode0.handle, data4,
                                                                          param_str, self.client.decode0.outbuf,
                                                                          self.client.decode0.outparam)
            end_time = time.time()
            difftime = int((end_time - start_time) * 1000)
            if difftime > 100:
                print(
                    "DecodeFrame: api_video_decode_one_frame: (id, difftime)= ", (self.client.decode0.obj_id, difftime))
            if ret > 0:
                # print("DecodeFrame: out frame size= ", ret)
                if self.client.log_fp != None and False:
                    self.client.log_fp.write("frame_type= " + self.client.decode0.outparam[0] + "\n")
                    self.client.log_fp.flush()
                # 解碼成功，緩存或顯示
                # self.client.decode0.frame_size
                data5 = self.client.decode0.outbuf.raw[:ret]
                # print("DecodeFrame: start show id= ", self.client.decode0.obj_id)
                if self.idr_status:
                    return
                if self.client.show != None:
                    # self.client.show.PushQueue(self.client.decode0.obj_id, data5, time.time())
                    self.client.show.PushQueue(self.client.decode0.obj_id, data5, frame_timestamp)
                else:
                    self.show_yuv(data5, self.client.decode0.obj_id)

    def run(self):
        while self.__running.isSet():
            self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            item = self.PopQueue()
            if item != None:
                self.DecodeFrame(item)
            else:
                time.sleep(0.01)
        self.client.decode0.load.lib.api_video_decode_close(self.client.decode0.handle)
        print("video DecodeFrame: run over")

class RecvTaskManagerThread(threading.Thread):
    def __init__(self, client):
        threading.Thread.__init__(self)
        self.client = client
        self.frame_decode = DecodeFrameThread(client)
        (self.param, self.outbuf, self.outparam) = self.client.decode0.setparam2()
        (self.frame_decode.width, self.frame_decode.height) = (self.client.decode0.width, self.client.decode0.height)
        self.frame_decode.start()

        self.max_delay_time = 0
        self.max_delay_packet = -1
        self.recv_packet_num = 0
        self.log_fp = None
        # try:
        #    self.log_fp = open("../../mytest/log.txt", "w")
        # except:
        #    print("open file fail !")

        ###
        # array_type = c_char_p * 4
        # self.outparam = array_type()

        handle_size = 16
        self.ll_handle = create_string_buffer(handle_size)
        # self.ll_handle = (c_char * handle_size)()
        array_type = c_char_p * 1
        self.ctime = array_type()
        #self.ctime[0] = c_char_p("0123456789012345")
        ###
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
    def __del__(self):
        print("RecvTaskManagerThread del")
        if self.param != None:
            del self.param
        if self.outbuf != None:
            del self.outbuf
        if self.outparam != None:
            del self.outparam
        if self.ll_handle != None:
            del self.ll_handle
    def ResortPacket(self, revcData, remoteHost, remotePort, recv_time):
        sockId = remoteHost + "_" + str(remotePort)
        # print("ResortPacket: start: id= ", self.client.decode0.obj_id)
        # del (self.param["inSize"])
        self.param.update({"insize": len(revcData)})
        if len(revcData) == 0:
            print("error: ResortPacket: insize = 0")
        # self.param.update({"min_distance": 2})
        # self.param.update({"delay_time": 100})
        # self.param.update({"delay_time": 100})
        # self.param.update({"delay_time": 200})
        # self.param.update({"buf_size": 1000})
        #
        self.param.update({"min_distance": self.client.decode0.min_distance})
        self.param.update({"delay_time": self.client.decode0.delay_time})
        self.param.update({"buf_size": self.client.decode0.buf_size})
        self.param.update({"qos_level": QOS_LEVEL})
        self.param.update({"adapt_cpu": 1})

        self.param.update({"loglevel": 1})  # 0/1/2
        # 重排序，並取幀
        # print("ResortPacket: self.client.decode0.obj_id= ", self.client.decode0.obj_id)
        #param_str = json.dumps(self.param, encoding='utf-8', ensure_ascii=False, sort_keys=True)
        param_str = json2str(self.param)
        self.outparam[0] = b""
        self.outparam[1] = b""
        self.outparam[2] = b"no frame"
        self.outparam[3] = b""

        # print("param_str= ", param_str)
        ret = self.client.decode0.load.lib.api_resort_packet(self.client.decode0.handle, revcData, param_str,
                                                             self.outbuf, self.outparam)

        # print("self.outparam[1]= ", self.outparam[1])
        # print("ret= ", ret)
        if False:
            outjson = str2json(self.outparam[1])
            if outjson != None:
                time_status = outjson["time_status"]
                if time_status == 0:
                    packet_time_stamp = long(outjson["packet_time_stamp"])
                    # recv_time_ll = long(round(recv_time) * 1000)
                    # print("recv_time_ll= ", recv_time_ll)
                    diff_time = int(recv_time - packet_time_stamp)
                    print("ResortPacket diff_time= ", diff_time)
                    if diff_time < 0:
                        print("error: ResortPacket diff_time= ", diff_time)
                    # if diff_time > self.max_delay_time and self.recv_packet_num > 100:
                    if diff_time > self.max_delay_time:
                        self.max_delay_time = diff_time
                        self.max_delay_packet = self.recv_packet_num
                        # if self.log_fp != None:
                        #    self.log_fp.write("max_delay_time= " + str(self.max_delay_time) + "\n")
                        #    self.log_fp.flush()
        if ret > 0 and True:
            # print("ResortPacket: ret= ", ret)
            runflag = True
            frame_timestamp = char2long(self.outparam[3])
            # print("ResortPacket: frame_timestamp= ", frame_timestamp)
            complete = self.outparam[2]
            # print("ResortPacket: complete= ", complete)
            # if(complete != "complete"):
            #    return
            data = self.outbuf.raw[:ret]
            rtpSize = self.outparam[0].split(b",")

            data4 = self.outbuf.raw[:ret]
            pktSize = self.outparam[0].split(b",")
            if self.frame_decode != None:
                self.frame_decode.PushQueue((data4, pktSize, complete, frame_timestamp))
                #self.ctime[0] = "0123456789012345"
                itime = self.client.decode0.load.lib.api_get_time2(self.ll_handle, self.ctime)
                now = char2long(self.ctime[0])
                if now == 0:
                    print("itime= ", itime)
                difftime = int(now - frame_timestamp)
                if difftime > 500 and False:
                    id = self.client.decode0.obj_id
                    print("RecvTaskManagerThread: (id, difftime)= ", (id, difftime))

        else:
            # print("ResortPacket: ret= ", ret)
            pass

    def run(self):
        while self.__running.isSet():
            self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            data = self.client.data_master.PopQueue()
            if data != None:
                self.recv_packet_num += 1
                (revcData, remoteHost, remotePort, recv_time) = data
                # recv_time = time.time()
                # print("TaskManagerThread: recv_time= ", recv_time)
                loss_rate = loadlib.gload.lib.api_count_loss_rate(revcData, len(revcData))
                if loss_rate >= 0:
                    print("Video TaskManagerThread: loss_rate= ", loss_rate)
                self.ResortPacket(revcData, remoteHost, remotePort, recv_time)
            else:
                time.sleep(0.002)  # 20ms
        #先退出resort，以免内存被先释放
        if self.frame_decode != None:
            self.frame_decode.stop()
        print("RecvTaskManagerThread: exit")

    def stop(self):
        if self.log_fp != None:
            self.log_fp.write("max_delay_time= " + str(self.max_delay_time) + "\n")
            self.log_fp.write("max_delay_packet= " + str(self.max_delay_packet) + "\n")
            self.log_fp.flush()

        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals
        print("RecvTaskManagerThread: stop")

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞


class DataManager(object):
    def __init__(self, id):
        self.lock = threading.Lock()
        self.id = id
        self.DataList = []
    def __del__(self):
        print("DataManager del")
        for thisobj in self.DataList:
            del thisobj
        del self.DataList
    def PopQueue(self):
        ret = None
        self.lock.acquire()
        if len(self.DataList) > 0:
            ret = self.DataList[0]
            del (self.DataList[0])
        self.lock.release()
        return ret

    def PushQueue(self, recvData, remoteHost, remotePort, recvTime):
        self.lock.acquire()
        # self.DataList.append((recvData, remoteHost, remotePort, time.time()))
        self.DataList.append((recvData, remoteHost, remotePort, recvTime))
        n = len(self.DataList)
        if n > 20:
            (data0, a, b, recv_time0) = self.DataList[0]
            (data1, a, b, recv_time1) = self.DataList[n - 1]
            difftime = int(recv_time1 - recv_time0)
            ##if difftime > 100:
            ##    print("DataManager: PushQueue: (self.id, n, difftime)= ", (self.id, n, difftime))
        self.lock.release()


class EchoClientThread(threading.Thread):
    def __init__(self, actor, host, port):
        threading.Thread.__init__(self)
        self.actor = actor
        self.host = host
        self.port = port
        self.status = True
        self.max_delay_time = 0
        self.max_delay_packet = -1
        self.recv_packet_num = 0
        print("EchoClientThread: host= ", host)
        print("EchoClientThread: port= ", port)

        # self.__flag = threading.Event()  # 用于暂停线程的标识
        # self.__flag.set()  # 设置为True
        # self.__running = threading.Event()  # 用于停止线程的标识
        # self.__running.set()  # 将running设置为True

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, SO_SNDBUF)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, SO_RCVBUF)
    def __del__(self):
        print("EchoClientThread del")

    def stop(self):
        self.status = False
        self.sock.shutdown(socket.SHUT_RDWR)
        self.sock.close()

    def run(self):
        while self.status:
            time.sleep(0.01)
        self.sock.close()


class PacedSend(threading.Thread):
    def __init__(self, client):
        threading.Thread.__init__(self)
        self.lock = threading.Lock()
        self.client = client
        print("PacedSend: init: ", (self.client.host, self.client.port))
        print("PacedSend: init: self.sock.encode0.bit_rate= ", self.client.encode0.bit_rate)
        print("PacedSend: init: self.sock.encode0.mtu_size= ", self.client.encode0.mtu_size)

        self.DataList = []
        self.packet_interval = 0
        self.sum = 0
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
    def __del__(self):
        print("PacedSend del")
        for thisobj in self.DataList:
            del thisobj
        del self.DataList
    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals


    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞

    def PopQueue(self):
        ret = None
        self.lock.acquire()
        if len(self.DataList) > 0:
            ret = self.DataList[0]
            del (self.DataList[0])
        self.lock.release()
        return ret

    def PushQueue(self, data, rtpSize):
        self.lock.acquire()
        now_time = time.time() * 1000
        self.DataList.append((data, rtpSize, now_time))
        self.lock.release()

    def test_reverse(self, data, rtpSize):
        # print("test_reverse: rtpSize= ", rtpSize)
        sum = 0
        datalist = []
        for csize in rtpSize:
            size = int(csize)
            data2 = data[sum:(sum + size)]
            datalist.append(data2)
            sum += size
        datalist.reverse()
        buf = bytearray(sum)
        offset = 0
        for thisdata in datalist:
            size = len(thisdata)
            buf[offset:(offset + size)] = thisdata
            offset += size
        rtpSize.reverse()
        return (buf, rtpSize)

    def run(self):
        pktnumps = (self.client.encode0.bit_rate >> 3) / self.client.encode0.mtu_size
        self.packet_interval = 1000.0 / pktnumps  # ms
        print("PacedSend: run: self.packet_interval= ", self.packet_interval)
        while self.__running.isSet():
            self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            item = self.PopQueue()
            if item != None:
                sum = 0
                (data, rtpSize, now_time) = item

                # (data, rtpSize) = self.test_reverse(data, rtpSize)

                for csize in rtpSize:
                    start_time = time.time()
                    size = int(csize)
                    data2 = data[sum:(sum + size)]
                    if sum == 0:
                        (ssrc, lossRate) = (0, 0)
                        # is_pass = loadlib.gload.lib.api_check_packet(data2, size, ssrc, lossRate)
                        # if is_pass:
                        #    break
                    # self.encode0.load.lib.api_renew_time_stamp(data2)
                    try:
                        sendDataLen = self.client.sock.sendto(data2, (self.client.host, self.client.port))
                    #except IOError, error:
                    except IOError as error:  # python3
                        print("PacedSend: run error: ", error)
                        break
                    else:
                        # print("sendDataLen= ", sendDataLen)
                        # time.sleep(0.001)
                        sum += sendDataLen
                        if sendDataLen != size:
                            print("warning: PacedSend: run: (size, sendDataLen)= ", (size, sendDataLen))
                        time.sleep(0.0001)
                        # time.sleep(0.0001)
                        end_time = time.time()
                        difftime = (end_time - start_time) * 1000
                        # print("PacedSend: run: difftime= ", difftime)
                self.sum += sum
                # print("PacedSend: run: self.sock.encode0.bit_rate= ", (self.sock.encode0.bit_rate))
                pass
            else:
                # print("PacedSend: run: self.sock.encode0.bit_rate= ", (self.sock.encode0.bit_rate))
                time.sleep(0.004)  # 4ms
        print("PacedSend: run over")

class EncoderClient(EchoClientThread):
    def __init__(self, id, sessionId, actor, host, port):
        EchoClientThread.__init__(self, actor, host, port)
        self.id = id
        self.sessionId = sessionId
        self.log_fp = None
        self.show = None
        self.capture = None
        try:
            filename = "../../mytest/enc_log_" + str(id) + ".txt"
            self.log_fp = open(filename, "w")
        except:
            print("EncoderClient:open file fail: filename=", filename)
        ###下劃線表示私有變量： '__'
        self.ack = threading.Event()
        self.ack.set()
        self.ack.clear()
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
        ###
        array_type = c_char_p * 4
        self.outparam = array_type()

        handle_size = 16
        self.ll_handle = create_string_buffer(handle_size)
        # self.ll_handle = (c_char * handle_size)()
        array_type = c_char_p * 1
        self.ctime = array_type()
        ###
        self.cmd_master = RecvCmdThread(self)
        self.cmd_master.start()
        self.data_master = DataManager(id)
        # self.recv_task = RecvTaskManagerThread(self)
        # self.recv_task.start()
        ###
        self.denoise = 0
        self.input_name = "v4l2"
        self.device_name = "/dev/video0"
        self.input_format = "mjpeg"
        self.framerate = 25
        self.select_device = 1#-1
        ###
        (self.width, self.height) = (SHOW_WIDTH, SHOW_HEIGHT)
        self.frame_size = (self.width * self.height * 3) / 2
        self.imglist = []
        self.img_num = 0

        self.encode0 = CallVideoEncode(id, self.width, self.height)
        self.encode0.refs = 1  # 16
        self.encode0.enable_fec = 0  # 1#0
        self.fec_level = 0
        self.start_time = 0

        self.paced_send = PacedSend(self)
    def __del__(self):
        print("EncoderClient del")
        if self.ll_handle != None:
            del self.ll_handle
        if self.cmd_master != None:
            del self.cmd_master
        if self.data_master != None:
            del self.data_master
        if self.paced_send != None:
            del self.paced_send
        if self.encode0 != None:
            del self.encode0
        if self.show != None:
            del self.show
        if self.capture != None:
            del self.capture
        if self.outparam != None:
            del self.outparam
        del self.sock
    def opendevice(self, devicetype):
        self.devicetype = devicetype
        if devicetype > 0:
            if devicetype == 1:
                self.input_name = "v4l2"
                self.device_name = "/dev/video0"
                self.input_format = "mjpeg"
                self.select_device = -1
            elif devicetype == 2:
                self.input_name = "v4l2"
                self.device_name = "/dev/video0"
                self.input_format = "raw"
                self.select_device = -1
            elif devicetype == 3:
                self.input_name = "x11grab"
                self.device_name = ":0.0"
                self.input_format = "raw"
                self.select_device = -1
            elif devicetype == 4:
                self.select_device = -1
            self.capture = VideoCapture()
            self.capture.input_name = self.input_name
            self.capture.device_name = self.device_name
            self.capture.input_format = self.input_format
            self.capture.select_device = self.select_device
            print("opendevice: self.input_name= ", self.input_name)
            print("opendevice: self.device_name= ", self.device_name)
            print("opendevice: self.input_format= ", self.input_format)
            print("opendevice: self.select_device= ", self.select_device)
            #self.capture.framerate = self.encode0.frame_rate
            self.capture.framerate = self.framerate #以25fps进行编码，以15fps采集视频
            (self.capture.width, self.capture.height) = (self.width, self.height)
            (self.capture.cap_width, self.capture.cap_height) = (self.width, self.height)
            self.capture.denoise = self.denoise
            self.capture.init()
            self.capture.start()

        if self.capture == None:  # or self.capture.status == False:
            filename1 = loadlib.yuvfilename
            print("filename1= ", filename1)
            self.fp = open(filename1, 'rb')
            #
            i = 0
            if self.fp:
                while True:
                    data = self.fp.read(self.frame_size)
                    if len(data) == self.frame_size:
                        if (i & 1) in DISTILL_LIST:
                            self.imglist.append(data)
                            self.img_num += 1
                        i += 1
                    else:
                        break

    def opencodec(self):
        self.frame_size = (self.width * self.height * 3) / 2
        (self.encode0.param, self.encode0.outbuf, self.encode0.outparam) = self.encode0.setparam()

        # self.encode0.param.update({"fec": 1})

        #param_str = json.dumps(self.encode0.param, encoding='utf-8', ensure_ascii=False, indent=4, sort_keys=True)
        param_str = json2str(self.encode0.param)
        # self.encode0.load.lib.api_video_encode_open(self.encode0.obj_id, param_str)
        ret = self.encode0.load.lib.api_video_encode_open(self.encode0.handle, param_str)
        print("EncoderClient: opencodec: open ret= ", ret)
        ###
        cmd = {"actor": self.actor, "sessionId": self.sessionId}
        #str_cmd = json.dumps(cmd, encoding='utf-8', ensure_ascii=False, sort_keys=True)
        str_cmd = json2str(cmd)
        sendDataLen = self.sock.sendto(str_cmd, (self.host, self.port))
        ##print("sendDataLen= ", sendDataLen)
        if self.paced_send != None:
            self.paced_send.start()

    def read_ack(self):
        if self.ack.isSet():
            time_offset = self.cmd_master.time_offset
            if time_offset > 0:
                self.encode0.param.update({"time_offset": time_offset})
                #print("EncoderClient:read_ack: time_offset= ", time_offset)
            rtt_status = self.cmd_master.rtt_status
            if rtt_status > 0:
                self.encode0.param.update({"rtt_status": rtt_status})
                self.cmd_master.rtt_status = 0
                #print("EncoderClient:read_ack: rtt_status= ", rtt_status)
            self.ack.clear()

    def send_data(self, data, rtpSize):
        sum = 0
        if self.paced_send != None:
            pass
            self.paced_send.PushQueue(data, rtpSize)
            return sum
        for csize in rtpSize:
            size = int(csize)
            data2 = data[sum:(sum + size)]
            self.encode0.load.lib.api_renew_time_stamp(data2)
            sendDataLen = self.sock.sendto(data2, (self.host, self.port))
            # print("sendDataLen= ", sendDataLen)
            # time.sleep(0.001)
            sum += size
        return sum

    def show_yuv(self, data0, id):
        (h, w) = (self.height, self.width)
        if True:  # id in [20]:
            frame0 = np.frombuffer(data0, dtype=np.uint8)
            yuv_I420_0 = frame0.reshape((h * 3) / 2, w)
            frame = cv2.cvtColor(yuv_I420_0, cv2.COLOR_YUV2BGR_I420)
            print("start to show id= ", id)
            cv2.imshow("enc-frame" + str(id), frame)
            cv2.waitKey(1)
            print("end to show id= ", id)

    def run(self):
        i = 0
        interval = 1000.0 / float(self.encode0.frame_rate)
        print("EncoderClient: interval= ", interval)
        self.test_max_time = 500
        self.start_time = 0
        self.frame_idx = 0
        self.sum_bits = 0
        self.last_timestamp = 0
        test_fec_k = 0
        test_fec_n = 0
        test_sum_fec_k = 0
        test_sum_fec_n = 0
        while self.__running.isSet():
            self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            test_time_read = 0
            test_time_enc = 0
            test_time_rtp = 0
            test_time_fec = 0
            test_time_all = 0

            now_time = time.time()
            # 读文件冲突：多个线程同时读取同一文件
            if self.capture != None:  # and self.capture.status == True:
                data = self.capture.ReadFrame()
                ret = len(data)
                if ret > 0:
                    pass
                else:
                    time.sleep(0.01)
                    continue
            elif self.img_num:
                if self.imglist == []:
                    data = self.fp.read(self.frame_size)
                else:
                    img_idx = self.frame_idx % self.img_num
                    img_floor = self.frame_idx / self.img_num
                    if (img_floor & 1) == 1:
                        img_idx = self.img_num - 1 - img_idx
                    data = self.imglist[img_idx]

            test_now_time = time.time()
            test_time_read = int((test_now_time - now_time) * 1000)
            # if test_time_read > 0:
            #    print("EncoderClient: 0: (test_time_read, id)= ", (test_time_read, self.id))
            # print("read data")
            #if len(data) == self.frame_size:
            if len(data) > 0:
                # self.show_yuv(data, self.id)
                if self.show != None:
                    #print("EncoderClient: show 0")
                    # self.show.play_right_now2(data, self.id)
                    # self.show.play_right_now3(data, self.id, 1)
                    #self.ctime[0] = "0123456789012345"
                    itime = self.encode0.load.lib.api_get_time2(self.ll_handle, self.ctime)
                    #this_frame_time = long(self.ctime[0])
                    this_frame_time = char2long(self.ctime[0])
                    if this_frame_time == 0:
                        print("itime= ", itime)
                    self.show.PushQueue(self.id, data, this_frame_time)
                    #print("EncoderClient: show 1")
                    pass
                pict_type = 0
                if i % self.encode0.gop_size:
                    I = (i % self.encode0.gop_size)
                    if (self.encode0.max_b_frames == 0) and (self.encode0.refs != 1):
                        j = (I - 1) % (self.encode0.refs)
                        # I P0 P1 P2 P0 P1 P2 (ref=3)
                        ref_idx = j + 1
                        if I >= (int(self.encode0.gop_size / self.encode0.max_refs) * self.encode0.max_refs):
                            ref_idx = 1
                        elif (ref_idx & 1) == 0 and ref_idx != self.encode0.refs and self.encode0.refs > 2:
                            ref_idx = 1
                        elif ref_idx == (self.encode0.refs - 1) and (self.encode0.refs > 4):
                            ref_idx = 1
                        ref_idx0 = ref_idx
                        # ref_idx = 1
                    else:
                        j = (I - 1) % (self.encode0.max_b_frames + 1)
                        # I P0 P1 P2 P0 P1 P2 (ref=3)
                        ref_idx = j + 1
                        #print("EncoderClient: ref_idx= ", ref_idx)
                        if ref_idx == (self.encode0.max_b_frames + 1):
                            pict_type = 2  # P Frame
                        else:
                            pict_type = 3  # B Frame
                        if I == (self.encode0.gop_size - 1):
                            pict_type = 2  # P Frame
                else:
                    ref_idx = 0
                    ref_idx0 = ref_idx
                    if i > 0:
                        self.encode0.refresh_idr = 0
                        self.encode0.param.update({"refresh_idr": self.encode0.refresh_idr})
                # if self.log_fp != None:
                #    self.log_fp.write("ref_idx= " + str(ref_idx) + "\n")
                #    self.log_fp.flush()
                #print("test_encode: (i, ref_idx)= ", (i, ref_idx))
                self.encode0.param.update({"frame_idx": i})
                self.encode0.param.update({"seqnum": self.encode0.seqnum})
                self.encode0.param.update({"res_num": 0})
                self.encode0.param.update({"res_idx": 0})
                if ((self.encode0.refs & 1) == 0) and True:
                    self.encode0.param.update({"ref_idx": ref_idx})
                else:
                    #self.encode0.param.update({"ref_idx": ref_idx})
                    self.encode0.param.update({"pict_type": pict_type})
                self.read_ack()
                #param_str = json.dumps(self.encode0.param, encoding='utf-8', ensure_ascii=False, indent=4,sort_keys=True)
                param_str = json2str(self.encode0.param)
                enc_start = time.time()
                ret = self.encode0.load.lib.api_video_encode_one_frame(self.encode0.handle, data, param_str,
                                                                       self.encode0.outbuf, self.encode0.outparam)

                test_now_time = time.time()
                test_time_enc = int((test_now_time - now_time) * 1000)
                if test_time_enc > self.test_max_time:
                    print("EncoderClient: 1: (test_time_enc, id)= ", (test_time_enc, self.encode0.outparam[0], self.id))
                    # print("EncoderClient: ret= ", ret)

                test_time_enc = int((test_now_time - enc_start) * 1000)
                # if test_time_enc > 40:
                #    print("EncoderClient: (test_time_enc, id)= ", (test_time_enc, self.encode0.outparam[0], self.id))

                if ret > 0:
                    # print("encode raw size= ", ret)
                    data2 = self.encode0.outbuf.raw[:ret]
                    # test rtp
                    if True:
                        self.encode0.outparam[0] = b""
                        self.encode0.outparam[1] = b""
                        self.encode0.param.update({"insize": ret})
                        self.encode0.param.update({"seqnum": self.encode0.seqnum})
                        ###
                        timestamp = 0
                        if self.start_time == 0:
                            self.start_time = now_time
                        else:
                            difftime = now_time - self.start_time
                            frame_num = int(difftime * 1000) / int(interval)
                            if sys.version_info >= (3, 0):
                                long_timestamp = int(difftime * 1000 * 90000 / 1000)
                                MAX_UINT = ((int(1) << 32) - 1)
                            else:
                                long_timestamp = long(difftime * 1000 * 90000 / 1000)
                                MAX_UINT = ((long(1) << 32) - 1)
                            timestamp = int(long_timestamp)
                            if long_timestamp > MAX_UINT:
                                timestamp = int(long_timestamp - MAX_UINT)
                                print("EncoderClient: overflow: (timestamp, id)= ", (timestamp, self.id))

                            if (timestamp < self.last_timestamp):
                                print("(timestamp, self.last_timestamp)= ", (timestamp, self.last_timestamp))
                            self.last_timestamp = timestamp
                            ###
                            test_item = []
                            test_item.append(int(difftime * 1000))
                            test_item.append(timestamp)
                            test_item.append(self.frame_idx)
                            test_item.append(self.id)
                            if self.id in [4, 5, 6] and False:
                                if abs(frame_num - self.frame_idx) > 2:
                                    print("EncoderClient: test_item= ", test_item)

                        self.encode0.param.update({"timestamp": timestamp})
                        ###
                        #param_str = json.dumps(self.encode0.param, encoding='utf-8', ensure_ascii=False, indent=4,sort_keys=True)
                        param_str = json2str(self.encode0.param)
                        ret2 = self.encode0.load.lib.api_raw2rtp_packet(self.encode0.handle, data2, param_str,
                                                                        self.encode0.outbuf, self.encode0.outparam)

                        test_now_time = time.time()
                        test_time_rtp = int((test_now_time - now_time) * 1000)
                        if test_time_rtp > self.test_max_time:
                            print("EncoderClient: 2: (test_time_rtp, id)= ", (test_time_rtp, self.id))
                            # print("EncoderClient: ret2= ", ret2)
                        # print("outparam[0]= ", outparam[0])
                        self.encode0.seqnum = int(self.encode0.outparam[1])
                        # print("self.encode0.seqnum= ", self.encode0.seqnum)
                        rtpSize = self.encode0.outparam[0].split(b",")
                        ##print("EncoderClient: rtpSize= ", rtpSize)
                        ##print("EncoderClient: len(rtpSize)= ", len(rtpSize))
                        data3 = self.encode0.outbuf.raw[:ret2]
                        enable_fec = 0
                        self.encode0.param.update({"enable_fec": enable_fec})
                        if self.encode0.enable_fec:
                            # ret2 = self.encode0.load.lib.api_get_extern_info(self.encode0.obj_id, data3, self.encode0.outparam)
                            # if ret2 > 0:
                            #    outjson = self.str2json(self.encode0.outparam[0])
                            #    if outjson != None:
                            #        refs = outjson["refs"]
                            #        ref_idx = outjson["ref_idx"]
                            if self.encode0.refs != 1:
                                if self.fec_level == 0:
                                    if (ref_idx == 0) or (ref_idx == self.encode0.refs):
                                        enable_fec = 1
                                elif self.fec_level == 1:
                                    if (ref_idx != 1) or (self.encode0.refs == 2):
                                        enable_fec = 1
                                else:
                                    enable_fec = 1
                            else:
                                if self.fec_level == 0:
                                    if pict_type in [1, 2]:
                                        enable_fec = 1
                                elif self.fec_level == 1:
                                    if pict_type in [1, 2, 3]:
                                        enable_fec = 1
                                else:
                                    enable_fec = 1
                        #print("(enable_fec, ref_idx)= ", (enable_fec, ref_idx))
                        #print("EncoderClient: enable_fec= ", enable_fec)
                        #print("EncoderClient: pict_type= ", pict_type)
                        if enable_fec:
                            self.encode0.param.update({"enable_fec": self.encode0.enable_fec})
                            self.encode0.param.update({"loss_rate": self.encode0.lost_rate})
                            self.encode0.param.update({"code_rate": self.encode0.code_rate})
                            sizelist = []
                            for packet_size in rtpSize:
                                sizelist.append(int(packet_size))
                            fec_k = len(sizelist)
                            #print("EncoderClient: api_fec_encode: sizelist=", sizelist)
                            del (self.encode0.param["insize"])
                            self.encode0.param.update({"inSize": sizelist})
                            fec_n = int(fec_k / self.encode0.code_rate + 0.8)
                            if fec_k < 5 and self.fec_level > 2:
                                if self.fec_level == 3:
                                    self.encode0.param.update({"loss_rate": 0.5})
                                    self.encode0.param.update({"code_rate": 0.5})
                                    fec_n = int(fec_k / 0.5 + 0.8)
                                elif self.fec_level == 4:
                                    self.encode0.param.update({"loss_rate": 0.8})
                                    self.encode0.param.update({"code_rate": 0.2})
                                    fec_n = int(fec_k / 0.2 + 0.8)
                            test_fec_k += fec_k
                            test_fec_n += fec_n
                            #param_str = json.dumps(self.encode0.param, encoding='utf-8', ensure_ascii=False,sort_keys=True)
                            param_str = json2str(self.encode0.param)
                            #print("EncoderClient: api_fec_encode: param_str=", param_str)
                            ret = self.encode0.load.lib.api_fec_encode(self.encode0.handle, data3, param_str,
                                                                       self.encode0.outbuf, self.encode0.outparam)

                            test_now_time = time.time()
                            test_time_fec = int((test_now_time - now_time) * 1000)
                            #print("EncoderClient: api_fec_encode: ret=", ret)
                            if ret > 0:
                                # print("encode raw size= ", ret)
                                self.encode0.seqnum = int(self.encode0.outparam[1])
                                pktSize = self.encode0.outparam[0].split(b",")
                                # print("pktSize= ", pktSize)
                                # print("len(pktSize)= ", len(pktSize))
                                data4 = self.encode0.outbuf.raw[:ret]
                                ###test
                                if False:
                                    sizelist = []
                                    for packet_size in pktSize:
                                        sizelist.append(int(packet_size))
                                    self.encode0.param.update({"inSize": sizelist})
                                    param_str = json.dumps(self.encode0.param, encoding='utf-8', ensure_ascii=False,
                                                           sort_keys=True)
                                    ret = self.encode0.load.lib.api_fec_decode(self.encode0.obj_id + 1, data4,
                                                                               param_str, self.encode0.outbuf,
                                                                               self.encode0.outparam)
                                    if ret > 0 and True:
                                        print("EncoderClient: decode raw size= ", ret)
                                        pktSize = self.encode0.outparam[0].split(",")
                                        print("EncoderClient: pktSize= ", pktSize)
                                        print("EncoderClient: len(pktSize)= ", len(pktSize))
                                        data5 = self.encode0.outbuf.raw[:ret]

                                        # del(param["inSize"])
                                        # (param, outbuf, outparam) = self.setparam()

                                        data3 = data5
                                        rtpSize = pktSize
                                else:
                                    data3 = data4
                                    rtpSize = pktSize

                            del (self.encode0.param["inSize"])
                        else:
                            sizelist = []
                            for packet_size in rtpSize:
                                sizelist.append(int(packet_size))
                            fec_k = len(sizelist)
                            fec_n = fec_k
                            test_fec_k += fec_k
                            test_fec_n += fec_n

                        self.send_data(data3, rtpSize)

                test_now_time = time.time()
                test_time_all = int((test_now_time - now_time) * 1000)
                if test_time_all > self.test_max_time:
                    print("EncoderClient: 3: (test_time_all,self.id)= ", (test_time_all, self.id))

                if (i % self.encode0.frame_rate) == (self.encode0.frame_rate - 1) and self.encode0.enable_fec:
                    if test_fec_k:
                        recovery_rate = int(100 * float(test_fec_n - test_fec_k) / test_fec_k)
                        # print("EncoderClient: (self.id, loss_rate)= ", (self.id, recovery_rate))
                        # print("EncoderClient: (test_fec_k, test_fec_n)= ", (test_fec_k, test_fec_n))
                        test_sum_fec_k += test_fec_k
                        test_sum_fec_n += test_fec_n
                        recovery_rate = int(100 * float(test_sum_fec_n - test_sum_fec_k) / test_sum_fec_k)
                        if (self.frame_idx % 1000) == 0:
                            print("EncoderClient: (self.id, loss_rate)= ", (self.id, recovery_rate))
                    test_fec_k = 0
                    test_fec_n = 0

                i += 1
                self.frame_idx += 1
                self.sum_bits += ret << 3
                # if i > 4:
                #    self.pause()
                #    break

                # if self.start_time == 0:
                #    self.start_time = now_time
                now_time = time.time()

                cap_time = self.start_time * 1000 + self.frame_idx * interval
                wait_time = cap_time - now_time * 1000
                if self.id in [4, 5, 6] and False:
                    difftime = (now_time - self.start_time) * 1000
                    frame_num = int(difftime) / int(interval)
                    test_item = []
                    test_item.append(frame_num)
                    test_item.append(self.frame_idx)
                    test_item.append(int(wait_time))
                    test_item.append(self.id)
                    if abs(frame_num - self.frame_idx) > 2:
                        print("EncoderClient: test_item1= ", test_item)
                        test_item2 = []
                        test_item2.append(test_time_read)
                        test_item2.append(test_time_enc)
                        test_item2.append(test_time_rtp)
                        test_item2.append(test_time_fec)
                        test_item2.append(self.id)
                        print("EncoderClient: test_item2= ", test_item2)

                #    print("EncoderClient: wait_time= ", wait_time)
                #    #print("EncoderClient: difftime= ", difftime)
                # difftime = now_time - self.start_time
                # wait_time = (self.frame_idx * interval) - (difftime * 1000)
                wait_time = float(wait_time) / 1000
                wait_time = wait_time if wait_time > 0.001 else 0.001
                # if self.id in [4]:
                #    print("EncoderClient: wait_time= ", wait_time)
                ###
                difftime = (int)((time.time() - self.start_time))
                if (self.frame_idx % 250) == 1 and difftime > 0:
                    framerate = self.frame_idx / difftime
                    bitsrate = (self.sum_bits / difftime) / 1024
                    print("EncoderClient: (self.id, framerate, bitsrate)= ", (self.id, framerate, bitsrate))
                    print("EncoderClient: (self.id, self.frame_idx)= ", (self.id, self.frame_idx))

                # print('{} test_encode time: {:.3f}ms'.format(time.ctime(), difftime * 1000))
                # wait_time = interval - difftime
                # wait_time = float(wait_time) / 1000
                # wait_time = wait_time if wait_time > 0.001 else 0.001
                # print("wait_time= ", wait_time)
                if self.devicetype == 0:
                    time.sleep(wait_time)
            else:
                print("EncoderClient:run: (len(data), self.frame_size)= ", (len(data), self.frame_size))
                self.__running.clear()
                # try:
                #    rerun = int(raw_input('please input to exit(eg: 0 ): '))
                # except:
                #    rerun = int(input('please input to exit(eg: 0 ): '))
                rerun = 1  # 0 #1#0 #1
                print("EncoderClient:run: rerun= ", rerun)
                if self.capture == None:
                    if rerun > 0:
                        self.fp.seek(0, os.SEEK_SET)
                    self.__running.set()
            ###
        if self.paced_send != None:
            self.paced_send.stop()
        self.cmd_master.stop()
        #if self.log_fp != None:
        #    self.fp.close()
        ##self.sock.close()
        ###需要保证paced_send已经停止
        self.encode0.load.lib.api_video_encode_close(self.encode0.handle)
        print("EncoderClient: run over")

    def stop(self):
        if self.capture != None:
            #self.capture.Close()
            self.capture.stop()
        if self.paced_send != None:
            self.paced_send.stop()
        self.cmd_master.stop()
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals

        try:
            self.sock.shutdown(socket.SHUT_RDWR)
            self.sock.close()
        #except IOError, err:
        except IOError as error:  # python3
            print("EncoderClient: stop error: ", error)
        else:
            print("EncoderClient: stop ok")
        if self.log_fp != None:
            self.log_fp.close()
        print("EncoderClient stop over")

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞


class DecoderClient(EchoClientThread):
    def __init__(self, id, sessionId, actor, host, port):
        EchoClientThread.__init__(self, actor, host, port)
        self.id = id
        self.sessionId = sessionId
        self.show = None
        ###
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
        ###
        self.width = SHOW_WIDTH
        self.height = SHOW_HEIGHT
        self.log_fp = None
        try:
            filename = "../../mytest/dec_log_" + str(id) + ".txt"
            self.log_fp = open(filename, "w")
        except:
            print("DecoderClient:open file fail !")
        ###
        array_type = c_char_p * 4
        self.outparam = array_type()

        handle_size = 16
        self.ll_handle = create_string_buffer(handle_size)
        # self.ll_handle = (c_char * handle_size)()
        array_type = c_char_p * 1
        self.ctime = array_type()
        ###
        self.decode0 = CallVideoDecode(self.id, self.width, self.height)
        ###
        self.data_master = DataManager(id)
        #self.recv_task = RecvTaskManagerThread(self)
        #self.recv_task.start()

        ###
        cmd = {"actor": self.actor, "sessionId": self.sessionId}
        #str_cmd = json.dumps(cmd, encoding='utf-8', ensure_ascii=False, sort_keys=True)
        str_cmd = json2str(cmd)
        sendDataLen = self.sock.sendto(str_cmd, (self.host, self.port))
        # print("sendDataLen= ", sendDataLen)
    def __del__(self):
        print("DecoderClient del")
        if self.ll_handle != None:
            del self.ll_handle
        if self.data_master != None:
            del self.data_master
        if self.recv_task != None:
            del self.recv_task
        if self.decode0 != None:
            del self.decode0
        if self.show != None:
            del self.show
        if self.outparam != None:
            del self.outparam
        del self.sock
    def opencodec(self):
        (self.decode0.param, self.decode0.outbuf, self.decode0.outparam) = self.decode0.setparam2()
        start_time = time.time()
        #param_str = json.dumps(self.decode0.param, encoding='utf-8', ensure_ascii=False, sort_keys=True)
        param_str = json2str(self.decode0.param)
        ret = self.decode0.load.lib.api_video_decode_open(self.decode0.handle, param_str)
        print("DecoderClient: opencodec: open ret= ", ret)
        print("DecoderClient: opencodec: open self.handle= ", self.decode0.handle)

        difftime = time.time() - start_time
        print('{} DecoderClient: opencodec time: {:.3f}ms'.format(time.ctime(), difftime * 1000))

        self.recv_task = RecvTaskManagerThread(self)
        self.recv_task.start()

    def run(self):
        self.start_time = 0
        while self.__running.isSet():
            self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            try:
                # print("DecoderClient: run: start recvform ")
                now_time = time.time()
                if self.start_time == 0:
                    self.start_time = now_time
                else:
                    difftime = int(now_time - self.start_time)
                    # print('DecoderClient: run: difftime: {:.2f}ms'.format(difftime * 1000))
                    # print('DecoderClient: run: difftime: {:.2f}s'.format(difftime * 1000 / 1000))
                    # print("DecoderClient: run: difftime= ", difftime)
                    if (difftime > 20):  # heartbeat
                        self.start_time = now_time
                        cmd = {"actor": self.actor, "sessionId": self.sessionId}
                        #str_cmd = json.dumps(cmd, encoding='utf-8', ensure_ascii=False, sort_keys=True)
                        str_cmd = json2str(cmd)
                        sendDataLen = self.sock.sendto(str_cmd, (self.host, self.port))

                #recvData, (remoteHost, remotePort) = self.sock.recvfrom(DATA_SIZE)
                rdata = self.sock.recvfrom(DATA_SIZE)
                if rdata[0] != '' and rdata[1] != None:
                    recvData, (remoteHost, remotePort) = rdata
                else:
                    print("rdata= ", rdata)
                    break
            except:
                print("DecoderClient: run: recvfrom error")
                break
            else:
                # print("DecoderClient: run: len(recvData)= ", len(recvData))
                self.recv_packet_num += 1
                #self.ctime[0] = "0123456789012345"
                itime = self.decode0.load.lib.api_get_time2(self.ll_handle, self.ctime)
                c_time_ll = char2long(self.ctime[0])
                if c_time_ll == 0:
                    print("itime= ", itime)
                diff_time = self.decode0.load.lib.api_get_pkt_delay(recvData, len(recvData))
                if diff_time > 500:
                    print("DecoderClient: (diff_time, self.id)= ", (diff_time, self.id))
                    #print("DecoderClient: (self.recv_packet_num, self.id)= ", (self.recv_packet_num, self.id))

                if diff_time > self.max_delay_time:
                    self.max_delay_time = diff_time
                    self.max_delay_packet = self.recv_packet_num

                self.data_master.PushQueue(recvData, remoteHost, remotePort, c_time_ll)
                # print("DecoderClient: run: 2: len(recvData)= ", len(recvData))
            # time.sleep(0.01)
        self.recv_task.stop()
        # self.sock.shutdown(socket.SHUT_RDWR)
        self.sock.close()
        print("DecoderClient: run exit")

    def stop(self):
        if self.log_fp != None:
            self.log_fp.write("max_delay_time= " + str(self.max_delay_time) + "\n")
            self.log_fp.write("max_delay_packet= " + str(self.max_delay_packet) + "\n")
            self.log_fp.flush()
        #self.recv_task.stop()
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals
        try:
            self.sock.shutdown(socket.SHUT_RDWR)
            self.sock.close()
        except:
            print("DecoderClient: stop error")
        else:
            print("DecoderClient: stop ok")
        # print("DecoderClient stop")


def GetParams(refs):
    ret = (3 * 1024 * 1024, 1400, 0, 10)
    (h, w) = (SHOW_HEIGHT, SHOW_WIDTH)
    if (w * h) <= 352 * 288:
        fec_level = 2
        if refs <= 2:
            fec_level = 0
        ret = (512 * 1024, 400, fec_level, 9)
    elif (w * h) <= 640 * 480:
        fec_level = 3
        if refs <= 2:
            fec_level = 0
        ret = (800 * 1024, 600, fec_level, 10)
    elif (w * h) <= 704 * 576:
        fec_level = 3
        if refs <= 2:
            fec_level = 0
        ret = (1000 * 1024, 600, fec_level, 10)
    elif (w * h) <= 1280 * 720:
        fec_level = 1
        if refs <= 2:
            fec_level = 0
        ret = (int(1.5 * 1024 * 1024), 800, fec_level, 11)
    elif (w * h) <= 1920 * 1080:
        fec_level = 1
        if refs <= 2:
            fec_level = 0
        ret = (2.5 * 1024 * 1024, 1100, fec_level, 12)
    else:
        fec_level = 0
        ret = (6 * 1024 * 1024, 1400, fec_level, 13)
    return ret


def RunClient(flag):
    if flag:
        # idx = raw_input('please input to exit(eg: -1 ): ')
        # count = 0
        thread_show = None
        thread0 = None
        thread1 = None
        thread2 = None
        thread3 = None
        thread4 = None
        thread5 = None
        thread6 = None
        thread7 = None
        thread8 = None
        thread9 = None

        (bitrate, mtu_size, fec_level, buffer_shift) = GetParams(SVC_REFS)
        print("RunClient: bitrate= ", bitrate)
        thread_show = ShowThread(0, SHOW_WIDTH, SHOW_HEIGHT, SHOW_WIDTH, SHOW_HEIGHT)
        idx = 0
        (id0, sessionId0, actor0) = (0, 100, 2)
        (id1, sessionId1, actor1) = (1, 200, 2)
        (id2, sessionId2, actor2) = (2, 300, 2)
        (id3, sessionId3, actor3) = (3, 400, 2)
        (id4, sessionId4, actor4) = (4, 100, 1)
        (id5, sessionId5, actor5) = (5, 200, 1)
        (id6, sessionId6, actor6) = (6, 300, 1)
        (id7, sessionId7, actor7) = (7, 400, 1)
        (id8, sessionId8, actor8) = (8, 500, 2)
        (id9, sessionId9, actor9) = (9, 600, 2)

        #thread8 = DecoderClient(id8, sessionId8, actor8, global_host, global_port)
        #thread9 = DecoderClient(id9, sessionId9, actor9, global_host, global_port)

        #thread0 = DecoderClient(id0, sessionId0, actor0, global_host, global_port)
        #thread1 = DecoderClient(id1, sessionId1, actor1, global_host, global_port)
        thread2 = DecoderClient(id2, sessionId2, actor2, global_host, global_port)
        #thread3 = DecoderClient(id3, sessionId3, actor3, global_host, global_port)


        #thread4 = EncoderClient(id4, sessionId4, actor4, global_host, global_port)
        #thread5 = EncoderClient(id5, sessionId5, actor5, global_host, global_port)
        thread6 = EncoderClient(id6, sessionId6, actor6, global_host, global_port)
        #thread7 = EncoderClient(id7, sessionId7, actor7, global_host, global_port)
        ###
        if True:
            if thread0 != None:
                (thread0.decode0.width, thread0.decode0.height) = (SHOW_WIDTH, SHOW_HEIGHT)
                thread0.decode0.min_distance = 2
                thread0.decode0.delay_time = 100
                thread0.decode0.buf_size = (1 << buffer_shift)  # 1024#必须是2的指数
                thread0.decode0.mtu_size = mtu_size
                thread0.opencodec()
            if thread1 != None:
                (thread1.decode0.width, thread1.decode0.height) = (SHOW_WIDTH, SHOW_HEIGHT)
                thread1.decode0.min_distance = 2
                thread1.decode0.delay_time = 100
                thread1.decode0.buf_size = (1 << buffer_shift)  # 1024#必须是2的指数
                thread1.decode0.mtu_size = mtu_size
                thread1.opencodec()
            if thread2 != None:
                (thread2.decode0.width, thread2.decode0.height) = (SHOW_WIDTH, SHOW_HEIGHT)
                thread2.decode0.min_distance = 2
                thread2.decode0.delay_time = 100
                thread2.decode0.buf_size = (1 << buffer_shift)  # 1024#必须是2的指数
                thread2.decode0.mtu_size = mtu_size
                thread2.opencodec()
            if thread3 != None:
                (thread3.decode0.width, thread3.decode0.height) = (SHOW_WIDTH, SHOW_HEIGHT)
                thread3.decode0.min_distance = 2
                thread3.decode0.delay_time = 100
                thread3.decode0.buf_size = (1 << buffer_shift)  # 1024#必须是2的指数
                thread3.decode0.mtu_size = mtu_size
                thread3.opencodec()
            if thread8 != None:
                (thread8.decode0.width, thread8.decode0.height) = (SHOW_WIDTH, SHOW_HEIGHT)
                thread8.decode0.min_distance = 2
                thread8.decode0.delay_time = 100
                thread8.decode0.buf_size = (1 << buffer_shift)  # 1024#必须是2的指数
                thread8.decode0.mtu_size = mtu_size
                thread8.opencodec()
            if thread9 != None:
                (thread9.decode0.width, thread9.decode0.height) = (SHOW_WIDTH, SHOW_HEIGHT)
                thread9.decode0.min_distance = 2
                thread9.decode0.delay_time = 100
                thread9.decode0.buf_size = (1 << buffer_shift)  # 1024#必须是2的指数
                thread9.decode0.mtu_size = mtu_size
                thread9.opencodec()
        if True:
            if thread4 != None:
                (thread4.encode0.width, thread4.encode0.height) = (SHOW_WIDTH, SHOW_HEIGHT)
                thread4.encode0.refs = 1
                thread4.fec_level = 1#0 #fec_level
                thread4.encode0.enable_fec = 1
                thread4.encode0.lost_rate = LOSS_RATE  # 0.2 #0.3
                thread4.encode0.code_rate = (1 - thread4.encode0.lost_rate)
                thread4.encode0.bit_rate = bitrate
                thread4.encode0.mtu_size = mtu_size
                thread4.encode0.max_b_frames = 0#4  # 1#0
                thread4.encode0.thread_count = 1
                thread4.encode0.setparam()
                thread4.opendevice(0)
                thread4.opencodec()
            if thread5 != None:
                (thread5.encode0.width, thread5.encode0.height) = (SHOW_WIDTH, SHOW_HEIGHT)
                thread5.encode0.refs = SVC_REFS#2#16#2#1#16#2#16#2#4#16#4#2#16
                thread5.fec_level = 0#1#0
                thread5.encode0.enable_fec = 1#0
                thread5.encode0.lost_rate = LOSS_RATE  # 0.2 #0.3
                thread5.encode0.code_rate = (1 - thread5.encode0.lost_rate)
                thread5.encode0.bit_rate = bitrate
                thread5.encode0.mtu_size = mtu_size
                thread5.encode0.setparam()
                thread5.opendevice(1)
                thread5.opencodec()
            if thread6 != None:
                (thread6.encode0.width, thread6.encode0.height) = (SHOW_WIDTH, SHOW_HEIGHT)
                thread6.encode0.refs = SVC_REFS#16
                thread6.fec_level = fec_level  # 1#0#2#0#1#2#3#0#4 #3 #2#0
                thread6.encode0.enable_fec = 1
                thread6.encode0.lost_rate = LOSS_RATE  # 0.2 #0.3
                thread6.encode0.code_rate = (1 - thread6.encode0.lost_rate)
                bitrate = int(thread6.encode0.code_rate * bitrate)
                thread6.encode0.bit_rate = bitrate
                thread6.encode0.mtu_size = mtu_size
                thread6.encode0.setparam()
                thread6.opendevice(1)
                thread6.opencodec()
            if thread7 != None:
                (thread7.encode0.width, thread7.encode0.height) = (SHOW_WIDTH, SHOW_HEIGHT)
                thread7.encode0.refs = 16
                thread7.fec_level = fec_level
                thread7.encode0.enable_fec = 1
                thread7.encode0.lost_rate = LOSS_RATE  # 0.3
                thread7.encode0.code_rate = (1 - thread7.encode0.lost_rate)
                thread7.encode0.bit_rate = bitrate
                thread7.encode0.mtu_size = mtu_size
                thread7.encode0.setparam()
                thread7.opendevice(2)
                thread7.opencodec()
        ###

        if thread_show != None:
            if thread0 != None:
                thread0.show = thread_show
                thread_show.InsertId(thread0.id)
            if thread1 != None:
                thread1.show = thread_show
                thread_show.InsertId(thread1.id)
            if thread2 != None:
                thread2.show = thread_show
                thread_show.InsertId(thread2.id)
            if thread3 != None:
                thread3.show = thread_show
                thread_show.InsertId(thread3.id)
            if thread4 != None:
                thread4.show = thread_show
                thread_show.InsertId(thread4.id)
            if thread5 != None:
                thread5.show = thread_show
                thread_show.InsertId(thread5.id)
            if thread6 != None:
                thread6.show = thread_show
                thread_show.InsertId(thread6.id)
            if thread7 != None:
                thread7.show = thread_show
                thread_show.InsertId(thread7.id)
            if thread8 != None:
                thread8.show = thread_show
                thread_show.InsertId(thread8.id)
            if thread9 != None:
                thread9.show = thread_show
                thread_show.InsertId(thread9.id)
            if EXCHANGE:
                thread_show.exchange_id()

            thread_show.start()
        ###dec
        if thread0 != None:
            thread0.start()
        if thread1 != None:
            thread1.start()
        if thread2 != None:
            thread2.start()
        if thread3 != None:
            thread3.start()
        if thread8 != None:
            thread8.start()
        if thread9 != None:
            thread9.start()
        # time.sleep(0.8)
        ###enc
        if thread4 != None:
            thread4.start()
        if thread5 != None:
            thread5.start()
        if thread6 != None:
            thread6.start()
        if thread7 != None:
            thread7.start()

        while idx >= 0 and idx < 4:
            try:
                idx = int(raw_input('please input to exit(eg: 0 ): '))
            except:
                idx = int(input('please input to exit(eg: 0 ): '))
            print("idx= ", idx)
            # thread.status = False if idx == 0 else True
        print("main: start stop...")

        if thread4 != None:
            thread4.stop()
        if thread5 != None:
            thread5.stop()
        if thread6 != None:
            thread6.stop()
        if thread7 != None:
            thread7.stop()

        if thread0 != None:
            thread0.stop()
        if thread1 != None:
            thread1.stop()
        if thread2 != None:
            thread2.stop()
        if thread3 != None:
            thread3.stop()
        if thread8 != None:
            thread8.stop()
        if thread9 != None:
            thread9.stop()
        ###test
        #time.sleep(2)
        if thread_show != None:
            thread_show.stop()
            pass


if __name__ == "__main__":
    print("start client")
    # udpClient = UdpClient()
    # udpClient.tcpclient()
    RunClient(True)
    print("end client")

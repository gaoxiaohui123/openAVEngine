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
from ctypes import *
import math
import errno
import platform
import struct
import loadlib
from audiocapture import AudioCapture
from callaudiocodec import CallAudioCodec
from audioplayer import AudioPlayer
import udpbase as base

CMD_SIZE = 256
DATA_SIZE = 800#300#1500
SO_SNDBUF = 1024 * 1024
SO_RCVBUF = 1024 * 1024

EXCHANGE = 0# 1#0#1
QOS_LEVEL = 0

gTag = "audio:"

global_port = 8098  # 8888
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
        del self.framelist
    def SetAudioBase(self, timestamp, play_time, frequence):
        self.audio_timestamp = timestamp
        self.audio_start_time = play_time
        self.audio_frequence = frequence


class PlayThread(threading.Thread):
    def __init__(self, way):
        threading.Thread.__init__(self)
        self.lock = threading.Lock()
        self.play_lock = threading.Lock()
        self.load = loadlib.gload
        self.mix_num = way
        self.sdl = AudioPlayer(0)
        self.sdl.mix_num = way

        self.sdl.init()
        #self.sdl.start()
        self.DataList = []
        self.FrameList = []
        self.DataQueue = {}
        self.baseId = -1
        #self.way = way
        self.start_time = 0
        self.frame_idx = 0
        self.poll_idx = 1
        self.idMap = []
        self.father = None
        self.max_delay_time = 0
        self.max_delay_packet = -1
        array_type = c_char_p * 4
        self.outparam = array_type()

        self.frame_size = self.sdl.frame_size #2048 * 2 * 2
        self.outbuf = create_string_buffer(self.frame_size << 1)

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
        print("PlayThread del")
        for thisobj in self.DataList:
            del thisobj
        del self.DataList
        for thisobj in self.FrameList:
            del thisobj
        del self.FrameList
        for thisobj in self.idMap:
            del thisobj
        del self.idMap
        if self.outbuf != None:
            del self.outbuf
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

    def InsertId(self, id):
        self.idMap.append(id)
        self.mix_num = len(self.idMap)

    def PopQueue(self):
        ret = None
        self.lock.acquire()
        if len(self.DataList) > 0:
            flag = len(self.DataList) > 10
            if flag:
                print("PlayThread: 0: len(self.DataList)= ", len(self.DataList))
            ret = self.DataList[0]
            del (self.DataList[0])
            if flag:
                print("PlayThread: 1: len(self.DataList)= ", len(self.DataList))
        self.lock.release()
        return ret

    def PopQueue2(self, idx):
        ret = None
        self.lock.acquire()
        i = -1
        for i in range(len(self.DataList)):
            (id, data, recvTime) = self.DataList[i]
            #if id == idx: #(idx & 0xFFFF):
            if (id & 0xFFFF) == (idx & 0xFFFF):
                #print("PopQueue2: (id, idx)=", (id, idx))
                #ret = self.DataList[idx]
                ret = self.DataList[i]
                break
        if ret != None:
            del (self.DataList[i])
        if len(self.DataList) > 20:
            print("PlayThread: PopQueue2: len(self.DataList)= ", len(self.DataList))

        self.lock.release()
        return ret

    def PushQueue(self, id, data, recvTime):
        self.lock.acquire()
        self.DataList.append((id, data, recvTime))
        self.lock.release()

    def PushQueue3(self, id, data, recvTime):
        self.lock.acquire()
        dataList = self.DataQueue.get(id)
        if dataList == None:
            self.DataQueue.update({id:[(data, recvTime)]})
        else:
            self.DataQueue[id].append((data, recvTime))
        self.lock.release()

    def PopQueue3(self):
        ret = []
        self.lock.acquire()
        if self.baseId >= 0 and False:
            # 多路同步播放
            dataList = self.DataQueue.get(self.baseId)
            if dataList != None and len(dataList) > 0:
                pass
            else:
                self.lock.release()
                return ret

        keyList = list(self.DataQueue.keys())
        n = len(keyList)
        if True:
            # 多路同步播放
            for i in range(n):
                key = keyList[i]
                value = self.DataQueue[key]
                if len(value) > 0:
                    pass
                else:
                    self.lock.release()
                    return ret
        for i in range(n):
            key = keyList[i]
            value = self.DataQueue[key]
            if len(value) > 0:
                if self.baseId < 0:
                    self.baseId = key
                (data, recvTime) = value[0]
                ret.append(value[0])
                del self.DataQueue[key][0]
        self.lock.release()
        return ret
    def exchange_id(self):
        n = len(self.idMap)
        id0 = self.idMap[0]
        id1 = self.idMap[self.poll_idx]
        self.idMap[0] = id1
        self.idMap[self.poll_idx] = id0
        self.poll_idx += 1
        if self.poll_idx > (n - 1):
            self.poll_idx = 1

    def play_right_now(self, data, id):
        self.play_lock.acquire()
        self.outbuf[0:self.frame_size] = data[0:self.frame_size]
        ret = self.sdl.play_one_frame(self.outbuf, self.frame_size)
        if len(data) > self.frame_size:
            self.outbuf[0:self.frame_size] = data[self.frame_size:(self.frame_size << 1)]
            ret = self.sdl.play_one_frame(self.outbuf, self.frame_size)
        self.play_lock.release()
    def play_mix_now(self, data_size, mix_num):
        self.sdl.play_one_frame_mix(data_size, mix_num)

    #def sdl_stop(self):
    #    self.sdl.sdl_stop()
    def run_1(self):
        self.interval = float(1000.0 / (self.sdl.out_sample_rate / self.sdl.out_nb_samples))
        print("PlayThread: run: self.interval= ", self.interval)
        self.adjust = 20
        #多路同步播放
        while self.__running.isSet():
            self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            itime = self.load.lib.api_get_time2(self.ll_handle, self.ctime)
            now_time = char2long(self.ctime[0])
            (data_size, mix_num) = (0, len(self.idMap))
            if mix_num > 0:
                self.sdl.reset_mix_buf(mix_num)
                #print("PlayThread: run: mix_num= ", mix_num)

            i = 0
            for idx in self.idMap:
                item = self.PopQueue2(idx)
                ###test
                #if self.idMap.index(idx) in [0]:
                if idx in [0]:
                    pass
                else:
                    #print("PlayThread: run: idx=", idx)
                    #print("PlayThread: run: self.idMap.index(idx)=", self.idMap.index(idx))
                    continue
                ###需要做同步
                if item != None:
                    (id, data, recvTime) = item
                    if (len(data) > 0):
                        data_size = len(data)
                        #print("(id, i)= ", (id, i))
                        self.sdl.mix_buf[i] = data
                        i += 1
            if i > 0:
                self.play_mix_now(data_size, i)
                self.frame_idx += 1
                #print("PlayThread: run: data_size= ", data_size)
                #print("PlayThread: run: self.frame_idx= ", self.frame_idx)
                if self.start_time == 0:
                    self.start_time = now_time

            if self.start_time == 0:
                time.sleep(0.01)
            else:
                #cap_time = float(self.start_time + self.frame_idx * self.interval)
                dx = (1000.0 * self.frame_idx * self.sdl.out_nb_samples) / self.sdl.out_sample_rate
                cap_time = float(self.start_time + float(dx))
                wait_time = ((cap_time - now_time) / 1000.0)
                wait_time = wait_time if wait_time > 0.001 else 0.001
                #print("PlayThread: run: wait_time= ", wait_time)

                #wait_time = float((self.interval / 2) / 1000.0) #test
                time.sleep(wait_time)
        print("ShowThread: run: player_stop")
        self.sdl.player_stop()
        print("ShowThread: run over")
    def run(self):
        self.interval = float(1000.0 / (self.sdl.out_sample_rate / self.sdl.out_nb_samples))
        print("PlayThread: run: self.interval= ", self.interval)
        self.adjust = 20
        #多路同步播放
        while self.__running.isSet():
            self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            itime = self.load.lib.api_get_time2(self.ll_handle, self.ctime)
            now_time = char2long(self.ctime[0])
            (data_size, mix_num) = (0, len(self.idMap))
            #print("PlayThread: run:")
            dataList = self.PopQueue3()
            mix_num = len(dataList)
            if mix_num > 0:
                self.sdl.reset_mix_buf(mix_num)
                for i in range(mix_num):
                    (data, recvTime) = dataList[i]
                    if (len(data) > 0):
                        data_size = len(data)
                        self.sdl.mix_buf[i] = data
                    else:
                        print("error:PlayThread: len(data)= ", len(data))
            if mix_num > 0:
                #self.sdl.reset_mix_buf(mix_num)
                #print("PlayThread: run: (data_size,mix_num)= ", (data_size,mix_num))
                self.play_mix_now(data_size, mix_num)
                #print("PlayThread: run: mix_num= ", mix_num)
                self.frame_idx += 1
                #print("PlayThread: run: data_size= ", data_size)
                #print("PlayThread: run: self.frame_idx= ", self.frame_idx)
                if self.start_time == 0:
                    self.start_time = now_time

            if self.start_time == 0:
                time.sleep(0.01)
            else:
                #cap_time = float(self.start_time + self.frame_idx * self.interval)
                dx = (1000.0 * self.frame_idx * self.sdl.out_nb_samples) / self.sdl.out_sample_rate
                cap_time = float(self.start_time + float(dx))
                wait_time = ((cap_time - now_time) / 1000.0)
                wait_time = wait_time if wait_time > 0.001 else 0.001
                #print("PlayThread: run: wait_time= ", wait_time)

                #wait_time = float((self.interval / 2) / 1000.0) #test
                time.sleep(wait_time)
        print("ShowThread: run: player_stop")
        self.sdl.player_stop()
        print("ShowThread: run over")
    def run_0(self):
        #多路同步播放
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
                        print("PlayThread: (id, listlen, difftime)= ", (id, listlen, difftime))
                self.play_right_now(data, id)
                # time.sleep(0.001)
                pass

        else:
            time.sleep(0.01)
        #self.sdl_stop()


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
                if error.errno == errno.EWOULDBLOCK:
                    pass
                elif error.errno == errno.WSAETIMEDOUT:
                    pass
                else:
                    print(gTag + "RecvCmdThread: run: recvfrom error", error)
                    break
            else:
                ##print("RecvCmdThread: len(recvData)= ", len(recvData))
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
        #self.ref_idc_list = []

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
        #print("DecodeFrameThread:PushQueue: len(self.framelist)= ", len(self.framelist))
        self.lock.release()

    def DecodeFrame(self, item):
        (data4, pktSize, complete, frame_timestamp) = item
        # print("DecodeFrame: complete= ", complete)
        frame_info = complete.split(",")
        #ref_idc = int(frame_info[0]) - 1

        runflag = True
        data = data4
        ret = len(data)

        if runflag and ret > 0:
            self.client.decode0.outparam[0] = b""
            self.client.decode0.outparam[1] = b""
            param_str = json2str(self.client.decode0.param)
            # print("param_str= ", param_str)
            # 獲取幀後，進行解包
            (osize, outbuf, outparam) = self.client.decode0.rtpunpacket(data, ret)
            # print("rtpunpacket: osize=", osize)
            if osize > 0:
                # 解包後進行解碼
                start_time = time.time()
                (osize2, outbuf2, outparam2) = self.client.decode0.codecframe(outbuf, osize)
                #print("DecodeFrame: osize2= ", osize2)
                end_time = time.time()
                difftime = int((end_time - start_time) * 1000)
                if difftime > 100:
                    id = self.client.decode0.obj_id
                    print("DecodeFrame: api_audio_decode_one_frame: (id, difftime)= ", (id, difftime))
                if osize2 > 0:
                    #
                    #if osize2 > 8192:
                    #    print("DecodeFrame: api_audio_decode_one_frame: osize2= ", osize2)
                    # 解碼成功，緩存或顯示
                    data5 = outbuf2.raw[:osize2]
                    #print("DecodeFrame: api_audio_decode_one_frame: osize2= ", osize2)
                    if self.client.show != None:
                        self.client.show.PushQueue3(self.client.decode0.obj_id, data5, frame_timestamp)
                        #print("DecodeFrame: api_audio_decode_one_frame: self.client.decode0.obj_id= ", self.client.decode0.obj_id)
                    else:
                        pass

    def run(self):
        while self.__running.isSet():
            self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            #print("DecodeFrameThread: run")
            item = self.PopQueue()
            if item != None:
                self.DecodeFrame(item)
            else:
                time.sleep(0.01)
        self.client.decode0.codecclose()
        print("audio DecodeFrame: run over")

class RecvTaskManagerThread(threading.Thread):
    def __init__(self, client):
        threading.Thread.__init__(self)
        self.client = client
        self.id = self.client.id
        self.decode = self.client.decode0
        (self.param, self.outbuf, self.outparam) = self.client.decode0.getparam()
        self.frame_decode = DecodeFrameThread(client)
        self.frame_decode.start()
        self.max_delay_time = 0
        self.max_delay_packet = -1
        self.recv_packet_num = 0
        self.loss_rate = 0
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
    def GetClientChanIds(self):
        ret = self.client.encChanIdList
        return ret
    def PushNetInfo(self, netInfoList):
        self.client.push_net_info(netInfoList)
    def ResortPacket(self, revcData, remoteHost, remotePort, recv_time, delaytime):
        sockId = remoteHost + "_" + str(remotePort)
        if len(revcData) == 0:
            print("error: ResortPacket: insize = 0")
        self.client.decode0.param2.update({"selfChanId": (self.client.id + 1)})
        self.client.decode0.param2.update({"ChanId": self.GetClientChanIds()})
        #self.loss_rate = 90#80#test
        self.client.decode0.param2.update({"loss_rate": self.loss_rate})
        # 重排序，並取幀
        (ret, outbuf, outparam, frame_timestamp) = self.client.decode0.resort(revcData, len(revcData))
        #print("ResortPacket: ret", ret)
        # print("self.outparam[1]= ", self.outparam[1])
        # print("ret= ", ret)
        if ret > 0 and True:
            # print("ResortPacket: ret= ", ret)
            if len(self.outparam[1]) > 0 and base.NETINFO:
                # print("ResortPacket: self.outparam[1]= ", self.outparam[1])
                netDict = str2json(self.outparam[1])
                if netDict != None:
                    # print("ResortPacket: netDict= ", netDict)
                    netInfoList = netDict.get("netInfo")
                    if netInfoList != None:
                        # print("ResortPacket: netInfoList= ", netInfoList)
                        optflg = True  # False
                        if optflg:
                            netInfoList2 = []
                            #self.hasNetList = []
                            for thisInfo in netInfoList:
                                chanId = thisInfo.get("chanId")
                                info_status = thisInfo.get("info_status")
                                item = (chanId, info_status)
                                if item not in self.hasNetList:
                                    self.hasNetList.append(item)
                                    netInfoList2.append(thisInfo)
                                else:
                                    # print("ResortPacket: netInfoList2= ", netInfoList2)
                                    # netInfoList2.append(thisInfo)  # test
                                    pass
                            # print("ResortPacket: netInfoList2= ", netInfoList2)
                            #print("ResortPacket: (self.id, self.hasNetList)= ", (self.id, self.hasNetList))
                            if len(netInfoList2) > 0:
                                self.PushNetInfo(netInfoList2)
                        else:
                            self.PushNetInfo(netInfoList)
                else:
                    ##print("error: audio: ResortPacket: netDict= ", netDict)
                    pass
            #if delaytime > 500:
            #    print(gTag + "ResortPacket: delaytime= ", delaytime)
            complete = ""
            data4 = outbuf.raw[:ret]
            pktSize = []
            pktSize.append(str(ret))
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
            #id = self.client.decode0.obj_id
            self.hasNetList = []
            #datalist = self.client.data_master2.PopQueue()
            (datalist, delaytime) = self.client.data_master2.PopQueueAll()
            for data in datalist:
            #if data != None:
                self.recv_packet_num += 1
                (revcData, remoteHost, remotePort, recv_time) = data
                # recv_time = time.time()
                # print("TaskManagerThread: recv_time= ", recv_time)
                #loss_rate = loadlib.gload.lib.api_count_loss_rate(revcData, len(revcData), 27)
                loss_rate = loadlib.gload.lib.api_count_loss_rate2(self.decode.handle, revcData, len(revcData), 27)
                if loss_rate > 0:
                    self.loss_rate = loss_rate
                    self.client.loss_rate = loss_rate
                    if loss_rate > 1:
                        print(gTag + "TaskManagerThread: loss_rate= ", (loss_rate - 1))
                self.ResortPacket(revcData, remoteHost, remotePort, recv_time, delaytime)
            if len(datalist) == 0:
                time.sleep(0.002)  # 20ms
                #print("TaskManagerThread: id= ", id)
        if self.frame_decode != None:
            self.frame_decode.stop()
        print("audio RecvTaskManagerThread: over")

    def stop(self):
        if self.log_fp != None:
            self.log_fp.write("max_delay_time= " + str(self.max_delay_time) + "\n")
            self.log_fp.write("max_delay_packet= " + str(self.max_delay_packet) + "\n")
            self.log_fp.flush()
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals
        print("audio RecvTaskManagerThread: stop")

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
        ret = []
        difftime = 0
        self.lock.acquire()
        if len(self.DataList) > 0:
            ret.append(self.DataList[0])
            del (self.DataList[0])
        self.lock.release()
        return (ret, difftime)

    def PopQueueAll(self):
        difftime = 0
        self.lock.acquire()
        ret = self.DataList
        self.DataList = []
        self.lock.release()
        n = len(ret)
        if n > 20:
            (data0, a, b, recv_time0) = ret[0]
            (data1, a, b, recv_time1) = ret[n - 1]
            difftime = int(recv_time1 - recv_time0)
            if difftime > 500:
                print(gTag + "DataManager: PopQueueAll: (self.id, n, difftime)= ", (self.id, n, difftime))
        return (ret, difftime)

    def PushQueue(self, recvData, remoteHost, remotePort, recvTime):
        self.lock.acquire()
        # self.DataList.append((recvData, remoteHost, remotePort, time.time()))
        self.DataList.append((recvData, remoteHost, remotePort, recvTime))
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
        if (platform.system() == 'Windows'):
            #val = struct.pack("Q", 5050)
            val = struct.pack("Q", 500) #500ms
            self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVTIMEO, val)
        else:
            #self.sock.settimeout(100)
            #val = struct.pack("QQ", 5, 50000)  # 5.05s
            val = struct.pack("QQ", 0, 500000) #500ms
            self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVTIMEO, val)
        if False:
            self.sock.setblocking(False)
            self.sock.settimeout(10)
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

class NetInfoMaster(object):
    def __init__(self, id):
        self.id = id
        self.sum_loss_rate = 0
        self.cnt_loss_rate = 0
        self.cnt_max = 0
        self.loss_rate = -1
        #
        self.sum_rtt = 0
        self.cnt_rtt = 0
        self.rtt = -1
        self.max_rtt = 0
        self.min_rtt = 100000

        if id == 0:
            self.cnt_max = 1
        elif id == 1:
            self.cnt_max = 10
        elif id == 2:
            self.cnt_max = 100
    def SumRtt(self, rtt):
        if rtt < 0:
            return
        self.sum_rtt += rtt
        self.cnt_rtt += 1
        #if self.rtt == -1:
        #    self.rtt = rtt
        if self.cnt_rtt >= self.cnt_max:
            self.rtt = int(self.sum_rtt / self.cnt_max)
            self.sum_rtt = 0
            self.cnt_rtt = 0
        else:
            self.rtt = int(self.sum_rtt / self.cnt_rtt)

        if rtt > self.max_rtt:
            self.max_rtt = rtt
        if rtt < self.min_rtt:
            self.min_rtt = rtt
    def SumLossRate(self, loss_rate):
        self.sum_loss_rate += loss_rate
        self.cnt_loss_rate += 1
        #if self.loss_rate == -1:
        #    self.loss_rate = loss_rate
        if self.cnt_loss_rate >= self.cnt_max:
            self.loss_rate = int(self.sum_loss_rate / self.cnt_max)
            self.sum_loss_rate = 0
            self.cnt_loss_rate = 0
        else:
            self.loss_rate = int(self.sum_loss_rate / self.cnt_loss_rate)
    def GetLossRate(self, loss_rate):
        ret = loss_rate
        if loss_rate < self.loss_rate:
            ret = self.loss_rate
        return ret

class PacedSend(threading.Thread):
    def __init__(self, client):
        threading.Thread.__init__(self)
        self.lock = threading.Lock()
        self.client = client
        print("PacedSend: init: ", (self.client.host, self.client.port))
        print("PacedSend: init: self.client.encode0.bit_rate= ", self.client.encode0.bit_rate)
        print("PacedSend: init: self.client.encode0.mtu_size= ", self.client.encode0.mtu_size)

        self.DataList = []
        self.packet_interval = 0
        self.sum = 0
        self.redundancy = 0
        self.seqnum = 0
        self.redundancyList = []
        self.net_info_list = [NetInfoMaster(0), NetInfoMaster(1), NetInfoMaster(2)]
        self.delayPktNum = 4 #10
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
    def __del__(self):
        print("PacedSend del")
        for thisobj in self.DataList:
            del thisobj
        del self.DataList

        del self.redundancyList

    def stop(self):
        print("audio PacedSend: stop 0")
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        print("audio PacedSend: stop 1")
        self.__running.clear()  # 设置为Fals
        print("audio PacedSend: stop 2")

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞
    def PoPRedundancy(self):
        ret = []
        n = len(self.redundancyList)
        k = self.redundancy
        for i in range(k):
            if i >= n:
                break
            thisData = self.redundancyList[i]
            ret.append(thisData)
        for i in range(k):
            if i >= n:
                break
            del self.redundancyList[0]
        return ret
    def PushRedundancy(self, data):
        n = len(self.redundancyList)
        k = self.redundancy + self.delayPktNum #self.redundancy
        self.redundancyList.append(data)
        if n > k:
            del self.redundancyList[0]
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
        #print("PacedSend: PushQueue: len(self.DataList)= ", len(self.DataList))
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
        pktnumps = (self.client.encode0.bit_rate >> 3) / 100 #self.client.encode0.mtu_size
        print("PacedSend: run: pktnumps= ", pktnumps)
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
                    #print("PacedSend: run: size= ", size)
                    #print("PacedSend: run: self.client.host= ", self.client.host)
                    #print("PacedSend: run: self.client.port= ", self.client.port)
                    try:
                        #self.client.encode0.reset_seqnum(data2)
                        #self.PushRedundancy(data2)
                        self.seqnum = self.client.encode0.load.lib.api_reset_seqnum(data2, len(data2), self.seqnum)
                        sendDataLen = self.client.sock.sendto(data2, (self.client.host, self.client.port))
                    #except IOError, err:
                    except IOError as error:  # python3
                        print("PacedSend: run error: ", error)
                        break
                    else:
                        sum += sendDataLen
                        ###redundancy
                        netInfoList = self.client.pop_self_net_info()
                        #print("PacedSend: len(netInfoList)= ", len(netInfoList))
                        maxLossRate = 0

                        if len(netInfoList) > 0 and True:
                            for thisNetInfo in netInfoList:
                                #st0 = thisNetInfo["st0"]
                                #rt0 = thisNetInfo["rt0"]
                                #st1 = thisNetInfo["st1"]
                                #rt1 = thisNetInfo["rt1"]
                                #rtt = ((int(rt1 - st0) - int(st1 - rt0)) >> 1)
                                #print("PacedSend: rtt= ", rtt)
                                #decodeId = thisNetInfo.get('decodeId')
                                #if decodeId != None:
                                #    #print("PacedSend: decodeId= ", decodeId)
                                #    pass
                                #self.net_info_list[0].SumRtt(rtt)
                                #self.net_info_list[1].SumRtt(rtt)
                                #self.net_info_list[2].SumRtt(rtt)

                                loss_rate = thisNetInfo["loss_rate"]
                                if loss_rate > maxLossRate:
                                    maxLossRate = loss_rate
                                    #print("PacedSend: maxLossRate= ", maxLossRate)

                        if maxLossRate > 0:
                            maxLossRate -= 1
                            self.net_info_list[0].SumLossRate(maxLossRate)
                            self.net_info_list[1].SumLossRate(maxLossRate)
                            self.net_info_list[2].SumLossRate(maxLossRate)
                            #print("PacedSend: self.net_info_list[1].rtt= ", self.net_info_list[1].rtt)
                        #存在无携带netinfo的包
                        #print("PacedSend: maxLossRate= ", maxLossRate)
                        if maxLossRate > 0 and True:
                            alfa = 0.8
                            ##maxLossRate = self.net_info_list[1].GetLossRate(maxLossRate)
                            ##maxLossRate = self.net_info_list[2].GetLossRate(maxLossRate)
                            #print("PacedSend: maxLossRate= ", maxLossRate)
                            loss_rate = float(maxLossRate) / 100.0
                            #print("PacedSend: loss_rate= ", loss_rate)
                            code_rate = (1.0 - loss_rate)
                            code_rate = code_rate * alfa
                            #print("PacedSend: code_rate= ", code_rate)
                            #redundancy = int((1.0 / code_rate - 1.0) + 0.99)
                            if loss_rate == 0:
                                self.redundancy = 0
                            elif loss_rate <= 0.05:
                                self.redundancy = 1
                            elif loss_rate <= 0.10:
                                self.redundancy = 2
                            elif loss_rate <= 0.20:
                                self.redundancy = 3
                            elif loss_rate <= 0.30:
                                self.redundancy = 4
                            elif loss_rate <= 0.40:
                                self.redundancy = 5 #6
                            elif loss_rate <= 0.50:
                                self.redundancy = 6
                            elif loss_rate <= 0.60:
                                self.redundancy = 8
                            elif loss_rate <= 0.70:
                                self.redundancy = 11
                            elif loss_rate <= 0.80:
                                self.redundancy = 20
                            #print("PacedSend: run loss_rate: ", loss_rate)
                            #print("PacedSend: redundancy= ", redundancy)
                            #self.redundancy = 4  # 3#2#1#10
                            #for i in range(self.redundancy):
                            #    self.client.encode0.reset_seqnum(data2)
                            #    sendDataLen = self.client.sock.sendto(data2, (self.client.host, self.client.port))
                            #    sum += sendDataLen
                        else:
                            #print("PacedSend: maxLossRate= ", maxLossRate)
                            #self.redundancy = 15 #0.8
                            #self.redundancy = 20#11#8#7#6#5#7#9
                            pass
                        #print("PacedSend: run self.redundancy: ", self.redundancy)
                        #self.redundancy = 8
                        if self.redundancy > 0 and True:
                            #self.redundancy = 4#3#2#1#10
                            fec_seqnum = 0
                            #dataList = self.PoPRedundancy()
                            for i in range(self.redundancy):
                                thisData = data2
                                #fec_seqnum = self.client.encode0.reset_seqnum(thisData)
                                self.seqnum = self.client.encode0.load.lib.api_reset_seqnum(thisData, len(thisData),self.seqnum)
                                sendDataLen = self.client.sock.sendto(thisData, (self.client.host, self.client.port))
                                sum += sendDataLen
                                time.sleep(0.001)
                            #print("fec_seqnum= ", fec_seqnum)
                        # print("sendDataLen= ", sendDataLen)
                        # time.sleep(0.001)
                        if sendDataLen != size:
                            print("warning: PacedSend: run: (size, sendDataLen)= ", (size, sendDataLen))
                        time.sleep(0.001)
                        # time.sleep(0.0001)
                        end_time = time.time()
                        difftime = (end_time - start_time) * 1000
                        # print("PacedSend: run: difftime= ", difftime)
                self.sum += sum
                # print("PacedSend: run: self.client.encode0.bit_rate= ", (self.client.encode0.bit_rate))
                pass
            else:
                # print("PacedSend: run: self.client.encode0.bit_rate= ", (self.client.encode0.bit_rate))
                time.sleep(0.004)  # 4ms
        print("audio PacedSend: run over")

class EncoderClient(base.EchoClientThread):
    def __init__(self, params): # id, sessionId, actor, selfmode, modeId, host, port):
        base.EchoClientThread.__init__(self, params)
        self.id = params.idx
        self.iNetTime = int(self.nettime * 1000)
        self.stream = None
        self.send_pkt_num = 0
        print("EncoderClient: __init__")

    def init2(self):
        self.log_fp = None
        self.show = None
        self.capture = None
        try:
            filename = "../../mytest/enc_log_audio" + str(self.id) + ".txt"
            self.log_fp = open(filename, "w")
        except:
            print("EncoderClient:open file fail !")

        self.selfNetInfoList = []
        self.selfNetInfoList2 = []
        self.otherNetInfoList = []
        self.speakList = []
        ###
        ###
        array_type = c_char_p * 4
        self.outparam = array_type()

        handle_size = 16
        self.ll_handle = create_string_buffer(handle_size)
        # self.ll_handle = (c_char * handle_size)()
        array_type = c_char_p * 1
        self.ctime = array_type()
        ###
        #self.cmd_master = None #RecvCmdThread(self)
        #self.cmd_master.start()
        #self.data_master = DataManager(id)
        # self.recv_task = RecvTaskManagerThread(self)
        # self.recv_task.start()

        ###
        self.imglist = []
        self.img_num = 0

        self.encode0 = CallAudioCodec(self.id, 0)
        self.start_time = 0

        self.paced_send = None #PacedSend(self)
        ###
        self.in_channels = 2
        self.in_sample_rate = 48000
        self.in_sample_fmt = "AV_SAMPLE_FMT_S16"
        self.out_nb_samples = 1024
        self.frame_size = 1024 * 2 * 2
        self.block_size = 8192
        self.data_buf = create_string_buffer(self.block_size << 1)
        self.offset = 0
        self.outbuf = create_string_buffer(self.block_size)
        print("EncoderClient: init2 ok")

    def __del__(self):
        print("EncoderClient del")
        if self.ll_handle != None:
            del self.ll_handle
        #if self.cmd_master != None:
        #    del self.cmd_master
        #if self.data_master != None:
        #    del self.data_master
        if self.paced_send != None:
            del self.paced_send
        if self.encode0 != None:
            del self.encode0
        if self.show != None:
            del self.show
        if self.capture != None:
            del self.capture
        if self.outbuf != None:
            del self.outbuf
        if self.outparam != None:
            del self.outparam
        del self.sock
    def get_codec_handle(self):
        return self.encode0.handle
    def AddSpeakerId(self, id):
        self.speakList.append(id)
    def opendevice(self, devicetype):
        self.devicetype = devicetype
        if devicetype > 0:
            if devicetype == 1:
                self.input_name = "alsa"
                self.device_name = "default"
            elif devicetype == 2:
                pass
            else:
                self.input_name = "alsa"
                self.device_name = "default"
                #self.out_channel_layout
                #self.out_buffer_size

            self.capture = AudioCapture(self.id)
            self.capture.input_name = self.input_name
            self.capture.device_name = self.device_name
            self.capture.in_channels = self.in_channels
            self.capture.in_sample_rate = self.in_sample_rate
            self.capture.in_sample_fmt = self.in_sample_fmt
            self.capture.out_channels = self.in_channels
            self.capture.out_sample_fmt = self.in_sample_fmt
            self.capture.out_sample_rate = self.in_sample_rate
            self.capture.datatype = 3
            self.capture.init()
            self.capture.start()

    def opencodec(self):
        self.encode0.init()
        # self.encode0.param.update({"fec": 1})
        param_str = json2str(self.encode0.param)
        # self.encode0.load.lib.api_video_encode_open(self.encode0.obj_id, param_str)
        #ret = self.encode0.load.lib.api_video_encode_open(self.encode0.handle, param_str)
        #print("opencodec: init: open ret= ", ret)

        if self.paced_send != None:
            self.paced_send.start()
    def push_net_info(self, netInfo, recvId, chanId, info_status):
        self.lock.acquire()
        #decodeId = netInfo.get("decodeId")
        #if decodeId != None:
        #    print("push_net_info: otherNetInfoList: decodeId=", decodeId)
        #chanId = netInfo.get("chanId")
        #info_status = netInfo.get("info_status")
        if chanId != None and info_status != None:
            if chanId == (self.id + 1) and info_status == 1:
                ##print("push_net_info: selfNetInfoList: (chanId, self.id)=", (chanId, self.id))
                self.selfNetInfoList.append((netInfo, recvId))
                self.selfNetInfoList2.append((netInfo, recvId))
            else:
                #print("push_net_info: otherNetInfoList: (chanId, self.id)=", (chanId, self.id))
                self.otherNetInfoList.append((netInfo, recvId))
        self.lock.release()
    def pop_net_info(self):
        ret = []
        idlist = []
        #分开保存，以保证机会均等
        self.lock.acquire()
        for (netInfo, recvId) in self.otherNetInfoList:
            chanId = netInfo.get("chanId")
            decodeId = netInfo.get("decodeId")
            item = (recvId, chanId, decodeId)
            if item not in idlist:
                idlist.append(item)
                ret.append(netInfo)
        self.otherNetInfoList = []

        #n = len(self.otherNetInfoList)
        #if n > 0:
        #    ret = self.otherNetInfoList
        #    self.otherNetInfoList = []
        self.lock.release()
        #print("pop_net_info: len(ret)= ", len(ret))
        return ret
    def pop_self_net_info(self):
        ret = []
        idlist = []
        self.lock.acquire()
        for (netInfo, recvId) in self.selfNetInfoList:
            chanId = netInfo.get("chanId")
            decodeId = netInfo.get("decodeId")
            item = (recvId, chanId, decodeId)
            if item not in idlist:
                idlist.append(item)
                ret.append(netInfo)
        self.selfNetInfoList = []
        #n = len(self.selfNetInfoList)
        #if n > 0:
        #    ret = self.selfNetInfoList
        #    self.selfNetInfoList = []
        #print("pop_net_info: len(self.selfNetInfoList)= ", len(self.selfNetInfoList))
        self.lock.release()
        return ret
    def pop_self_net_info2(self):
        ret = []
        idlist = []
        self.lock.acquire()
        n = len(self.selfNetInfoList2)
        if n > 0:
            m = len(self.speakList)
            #if n > 10 * m and m > 0:
            #    print("pop_self_net_info2: n=", n)
            #    for i in range(n):
            #        del (self.selfNetInfoList2[0])
            #        if len(self.selfNetInfoList2) < 2 * len(self.speakList):
            #            break
            for (netInfo, recvId) in self.selfNetInfoList2:
                chanId = netInfo.get("chanId")
                decodeId = netInfo.get("decodeId")
                item = (recvId, chanId, decodeId)
                if item not in idlist:
                    ret.append(netInfo)
            self.selfNetInfoList2 = []
            #n = len(self.selfNetInfoList2)
            #if n > 0:
            #    for i in range(n):
            #        ret.append(self.selfNetInfoList2[0])
            #        del (self.selfNetInfoList2[0])
        self.lock.release()
        return ret

    def send_data(self, data, rtpSize):
        sum = 0
        if self.paced_send != None:
            pass
            self.paced_send.PushQueue(data, rtpSize)
            return sum
        now_time = time.time()
        for csize in rtpSize:
            size = int(csize)
            data2 = data[sum:(sum + size)]
            self.encode0.load.lib.api_renew_time_stamp(data2)
            if self.sendthread != None:
                now_time2 = now_time - self.nettime
                pktdata = self.sendthread.packdata(data2, self.send_pkt_num, now_time2)
                self.sendthread.data_master.PushQueue(pktdata, self.host, self.port, now_time)
                self.send_pkt_num += 1
            else:
                sendDataLen = self.sock.sendto(data2, (self.host, self.port))
            # print("sendDataLen= ", sendDataLen)
            # time.sleep(0.001)
            sum += size
        return sum

    def str2json(self, json_str):
        try:
            outjson = json.loads(json_str, encoding='utf-8')
        except:
            print("error: python version")
            try:
                outjson = json.loads(json_str)
            except:
                print("self.client.decode0.outparam[1] not json")
            else:
                json_str2 = json2str(outjson)
                print("1: json_str2= ", json_str2)
        else:
            json_str2 = json2str(outjson)
            print("0: json_str2= ", json_str2)
        return outjson
    def read_frame(self):
        data_size2 = 0
        (data, data_size) = self.capture.ReadFrame()
        if data_size > 0:
            self.data_buf[self.offset:(self.offset + data_size)] = data[0:data_size]
            self.offset += data_size
            if self.offset >= self.block_size:
                self.outbuf[0:self.block_size] = self.data_buf[0:self.block_size]
                data_size2 = self.block_size
                tail = self.offset - self.block_size
                self.data_buf[0:tail] = self.data_buf[self.offset - tail:self.offset]
                self.offset = tail

        return (self.outbuf, data_size2)
    def run(self):
        zero_cnt = 0
        while self.running.isSet():
            self.flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            (data, data_size) = self.read_frame()
            if data_size > 0:
                zero_cnt = 0
                (osize, outbuf, outparam) = self.encode0.codecframe(self.outbuf, self.block_size)
                # print("audioplayer: osize= ", osize)
                if osize > 0 and True:
                    # print("rtppacket: osize=", osize)
                    # self.encode0.param.update({"timestamp": timestamp})
                    if True:
                        self.encode0.param.update({"selfChanId": (self.id + 1)})
                        self.encode0.param.update({"speakers": self.speakList})
                        self.encode0.param.update({"time_offset": self.iNetTime})
                        # print("EncoderClient: self.speakList= ", self.speakList)
                        netInfoList = self.pop_net_info()
                        # print("EncoderClient: 2：len(netInfoList)= ", len(netInfoList))
                        if len(netInfoList) > 0:
                            self.encode0.param.update({"netInfo": netInfoList})
                        netInfoList2 = self.pop_self_net_info2()
                        if len(netInfoList2) > 0:
                            thisNetInfo = netInfoList2[0]
                            st0 = thisNetInfo["st0"]
                            rt0 = thisNetInfo["rt0"]
                            st1 = thisNetInfo["st1"]
                            rt1 = thisNetInfo["rt1"]
                            rtt = ((int(rt1 - st0) - int(st1 - rt0)) >> 1)
                            decodeId = thisNetInfo.get('decodeId')
                            if decodeId != None:
                                rttDict = {"decodeId": decodeId, "rtt": rtt}
                                # print("EncoderClient: decodeId= ", decodeId)
                                self.encode0.param.update({"rttInfo": [rttDict]})
                    (osize, outbuf, outparam) = self.encode0.rtppacket(outbuf, osize)
                    rtpSize = []
                    rtpSize.append(str(osize))
                    self.send_data(outbuf, rtpSize)
            else:
                zero_cnt += 1
                time.sleep(0.001 * zero_cnt)
                if zero_cnt > 10:
                    zero_cnt = 10
                #防止抢占CPU
            ###
        print("audio EncoderClient: run exit")
        if self.paced_send != None:
            self.paced_send.stop()

        #if self.cmd_master != None:
        #    self.cmd_master.stop()
        self.encode0.codecclose()
        #self.fp.close()
        #self.sock.close()
        print("audio EncoderClient: run over")

    def stop2(self):
        print("audio EncoderClient: stop paced_send 0")
        self.stop()
        if self.capture != None:
            self.capture.stop()
        if self.paced_send != None:
            self.paced_send.stop()
            #self.paced_send = None
        #if self.cmd_master != None:
        #    self.cmd_master.stop()
        if self.log_fp != None:
            self.log_fp.close()
        print("audio EncoderClient stop")

class DecoderClient(base.EchoClientThread):
    def __init__(self, params): #id, sessionId, actor, host, port):
        base.EchoClientThread.__init__(self, params)
        self.id = params.idx
        self.iNetTime = int(self.nettime * 1000)
        self.stream = None
        self.max_delay_time = 0
        self.max_delay_packet = -1
        self.recv_packet_num = 0
        print("DecoderClient: __init__")
    def init2(self):
        self.show = None
        self.encList = []  # 建立编码与解码的链接通道
        self.encChanIdList = []
        self.speakList = []
        self.hasNetList = []
        self.hasSelfNetList = []
        ###
        self.log_fp = None
        try:
            filename = "../../mytest/dec_log_audio" + str(self.id) + ".txt"
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
        self.decode0 = CallAudioCodec(self.id, 1)
        ###
        self.data_master2 = DataManager(self.id)
        self.recv_task = RecvTaskManagerThread(self)
        self.recv_task.start()
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

    def get_codec_handle(self):
        return self.decode0.handle
    def AddSpeakerId(self, id):
        self.speakList.append(id)
    def add_client(self, thisClient):
        self.encList.append(thisClient) #建立编解码链接通道
        self.encChanIdList.append(thisClient.id)

    def push_net_info(self, netInfoList):
        for thisInfo in netInfoList:
            info_status = thisInfo.get("info_status")
            chanId0 = thisInfo.get("chanId")
            if info_status == 1:
                for chanId in self.encChanIdList:
                    ###opt
                    if len(self.hasSelfNetList) >= len(self.encChanIdList):
                        self.hasSelfNetList = []
                    if chanId not in self.hasSelfNetList:
                        self.hasSelfNetList.append(chanId)
                    else:
                        continue
                    ###
                    if chanId0 == (chanId + 1):
                        idx = self.encChanIdList.index(chanId)
                        thisClient = self.encList[idx]
                        if thisClient != None:
                            thisClient.push_net_info(thisInfo, self.id, chanId0, info_status)
                            pass
            else:
                for chanId in self.encChanIdList:
                    ###opt
                    if len(self.hasNetList) >= len(self.encChanIdList):
                        self.hasNetList = []
                    if chanId not in self.hasNetList:
                        self.hasNetList.append(chanId)
                    else:
                        continue
                    ###
                    if (chanId0 != (chanId + 1)) or (self.selfmode > 0):
                        idx = self.encChanIdList.index(chanId)
                        thisClient = self.encList[idx]
                        if thisClient != None:
                            thisClient.push_net_info(thisInfo, self.id, chanId0, info_status)
                            pass
                    else:
                        print(gTag + "DecoderClient: push_net_info: (chanId0, (chanId + 1))= ", (chanId0, (chanId + 1)))
    def opencodec(self):
        self.decode0.init()
        pass

    def run(self):
        print("DecoderClient: run")
        while self.running.isSet():
            self.flag.wait()  # 会延缓发送频率
            # print("EchoClientThread run")
            now_time = time.time()
            # print("DecoderClient: run: 0")
            # decoder data process and encoder rtt/lossrate process
            if self.data_master != None:
                # print("EchoClientThread run: self.actor= ", self.actor)
                datalist = self.data_master.PopQueueAll()
                if datalist == None or datalist == []:  # or len(datalist) < 15:
                    # 过频繁的访问会导致锁被占用
                    time.sleep(0.001)
                    # print("EchoClientThread run: 2: self.actor= ", self.actor)
                else:
                    optFlag = True
                    for data in datalist:
                        (revcData, remoteHost, remotePort, recv_time) = data
                        if self.dataprocess != None:
                            n = len(revcData)
                            iscmd = n <= CMD_SIZE and ('{' in str(revcData[0:3])) and ('}' in str(revcData[n - 3:]))
                            if iscmd:
                                iscmd = '"cmd":' in str(revcData)
                            if iscmd:
                                print("DecoderClient run: 0: n= ", n)
                                self.dataprocess.ProcessCmd(data)
                            else:
                                # print("EchoClientThread run: 1: self.actor= ", self.actor)
                                (recvData1, data1) = self.dataprocess.ProcessRaw(data)
                                (seqnum, time0, recv_time, recvData) = recvData1
                                if optFlag:
                                    if self.data_master2 != None:
                                        self.data_master2.PushQueue(recvData, remoteHost, remotePort, now_time)
                                    self.recv_packet_num += 1
                                else:
                                    # print(gTag + "DecoderClient: run: len(recvData)= ", len(recvData))
                                    isrtp = self.decode0.load.lib.api_isrtp(recvData, len(recvData), 0, 126)
                                    if isrtp > 0:
                                        pass
                                    else:
                                        print(gTag + "DecoderClient: run: isrtp= ", isrtp)
                                        # print(gTag + "DecoderClient: run: recvData= ", recvData)
                                        continue
                                    self.recv_packet_num += 1
                                    # self.ctime[0] = "0123456789012345"
                                    itime = self.decode0.load.lib.api_get_time2(self.ll_handle, self.ctime)
                                    c_time_ll = char2long(self.ctime[0])
                                    if c_time_ll == 0:
                                        print("itime= ", itime)
                                    # diff_time = self.decode0.load.lib.api_get_pkt_delay(recvData, len(recvData))
                                    diff_time = self.decode0.load.lib.api_get_pkt_delay2(recvData, len(recvData), 0,
                                                                                         self.iNetTime, 126)
                                    if diff_time > 500:
                                        print(gTag + "DecoderClient: self.iNetTime= ", self.iNetTime)
                                        print(gTag + "DecoderClient: (diff_time, self.id)= ", (diff_time, self.id))
                                        # print("DecoderClient: (self.recv_packet_num, self.id)= ", (self.recv_packet_num, self.id))

                                    if diff_time > self.max_delay_time:
                                        self.max_delay_time = diff_time
                                        self.max_delay_packet = self.recv_packet_num
                                    if self.data_master2 != None:
                                        # print(gTag + "DecoderClient: run: len(recvData)= ", len(recvData))
                                        self.data_master2.PushQueue(recvData, remoteHost, remotePort, c_time_ll)
            else:
                # print("EchoClientThread run: self.actor= ", self.actor)
                pass
            end_time = time.time()
            difftime = int((end_time - now_time) * 1000)
        if self.recv_task != None:
            self.recv_task.stop()
        print("DecoderClient run over")

    def stop2(self):
        self.stop()
        if self.log_fp != None:
            self.log_fp.write("max_delay_time= " + str(self.max_delay_time) + "\n")
            self.log_fp.write("max_delay_packet= " + str(self.max_delay_packet) + "\n")
            self.log_fp.flush()

def RunSessionStart(base_idx, proc_idx, speakernum, host, port):
    #global base.global_render
    if True:
        # idx = raw_input('please input to exit(eg: -1 ): ')
        # count = 0
        if base.global_render != None:
            base.global_render.setDaemon(True)
            base.global_render.start()
        thread_show = PlayThread(2)
        thread_show.setDaemon(True)
        thread_show.start()
        idx = 0
        #clientnum = 1#16#2#1#16 #2#1#2
        threadnum = speakernum * speakernum
        threadlist = [None for x in range(threadnum)]
        isok = False
        upChanList = []
        for i in range(base_idx, speakernum, 1):
            if isok:
                break
            for j in range(speakernum):
                if isok:
                    break
                #actor_idx = proc_idx * threadnum + i
                k = i * speakernum + j
                if (i | j) == 0:
                    actor = 1
                elif(j == 0):
                    actor = 2
                else:
                    actor = 3

                params = base.PortParams()

                (params.modeId, params.avtype, params.selfmode) = (base.DEFAULT_MODE, 1, 0)
                (params.testmode) = (False)
                (params.host, params.port) = (host, port)

                if actor in [1, 2]:
                    (params.sessionId, params.chanId, params.actor) = (proc_idx, i, actor)
                    threadlist[k] = EncoderClient(params)
                    if threadlist[k].status == False:
                        print("RunSession: fail: threadlist: k=", k)
                        del threadlist[k]
                        continue
                    threadlist[k].init2()
                    #print("RunSession: bitrate= ", bitrate)
                    #threadlist[k].AddSpeakerId(params.chanId)
                    threadlist[k].input_name = "alsa"
                    if (platform.system() == 'Windows'):
                        threadlist[k].input_name = "dshow"
                        print('Windows系统')
                    elif (platform.system() == 'Linux'):
                        print('Linux系统')
                    else:
                        print('其他')
                    threadlist[k].device_name = "default"
                    threadlist[k].in_channels = 2
                    threadlist[k].in_sample_rate = 48000
                    threadlist[k].encode0.out_nb_samples = 2048

                    threadlist[k].encode0.bit_rate = 24000
                    threadlist[k].encode0.mtu_size = 300
                    threadlist[k].opendevice(2)
                    threadlist[k].opencodec()

                    threadlist[k].show = thread_show
                    thread_show.InsertId(params.chanId)

                    threadlist[k].setDaemon(True)
                    threadlist[k].start()
                    upChanList.append(threadlist[k])
                else:
                    chanId = j
                    if j == i:
                        chanId = 0
                    (params.sessionId, params.chanId, params.actor) = (proc_idx, chanId, actor)
                    threadlist[k] = DecoderClient(params)
                    if threadlist[k].status == False:
                        print("RunSession: fail: threadlist: k=", k)
                        del threadlist[k]
                        continue
                    threadlist[k].init2()
                    #threadlist[k].AddSpeakerId(i)

                    threadlist[k].decode0.min_distance = 2
                    threadlist[k].decode0.delay_time = 100
                    threadlist[k].decode0.buf_size = (1 << 11)  # 1024#必须是2的指数
                    threadlist[k].decode0.mtu_size = 300
                    for thisThread in upChanList:
                        threadlist[k].add_client(thisThread)
                    #threadlist[k].opencodec()
                    threadlist[k].show = thread_show
                    thread_show.InsertId(params.chanId)
                    threadlist[k].setDaemon(True)
                    threadlist[k].start()
                ###
                #isok = True
            #if base_idx == 0:
            isok = True
    return (thread_show, threadlist)
def RunSessionStop(thread_show, threadlist):
    for thisthread in threadlist:
        if thisthread != None:
            thisthread.stop2()
    if thread_show != None:
        thread_show.stop()

def RunSession(base_idx, proc_idx, speakernum, host, port):
    #global base.global_render
    if True:
        # idx = raw_input('please input to exit(eg: -1 ): ')
        # count = 0
        if base.global_render != None:
            base.global_render.setDaemon(True)
            base.global_render.start()
        thread_show = PlayThread(2)
        thread_show.setDaemon(True)
        thread_show.start()
        idx = 0
        #clientnum = 1#16#2#1#16 #2#1#2
        threadnum = speakernum * speakernum
        threadlist = [None for x in range(threadnum)]
        isok = False
        upChanList = []
        for i in range(base_idx, speakernum, 1):
            if isok:
                break
            for j in range(speakernum):
                if isok:
                    break
                #actor_idx = proc_idx * threadnum + i
                k = i * speakernum + j
                if (i | j) == 0:
                    actor = 1
                elif(j == 0):
                    actor = 2
                else:
                    actor = 3

                params = base.PortParams()

                (params.modeId, params.avtype, params.selfmode) = (base.DEFAULT_MODE, 1, 0)
                (params.testmode) = (False)
                (params.width, params.height) = (1280, 720)
                (params.host, params.port) = (host, port)

                if actor in [1, 2]:
                    (params.sessionId, params.chanId, params.actor) = (proc_idx, i, actor)
                    threadlist[k] = EncoderClient(params)
                    if threadlist[k].status == False:
                        print("RunSession: fail: threadlist: k=", k)
                        del threadlist[k]
                        continue
                    threadlist[k].init2()
                    #print("RunSession: bitrate= ", bitrate)
                    #threadlist[k].AddSpeakerId(params.chanId)
                    threadlist[k].input_name = "alsa"
                    if (platform.system() == 'Windows'):
                        threadlist[k].input_name = "dshow"
                        print('Windows系统')
                    elif (platform.system() == 'Linux'):
                        print('Linux系统')
                    else:
                        print('其他')
                    threadlist[k].device_name = "default"
                    threadlist[k].in_channels = 2
                    threadlist[k].in_sample_rate = 48000
                    threadlist[k].encode0.out_nb_samples = 2048

                    threadlist[k].encode0.bit_rate = 24000
                    threadlist[k].encode0.mtu_size = 300
                    threadlist[k].opendevice(2)
                    threadlist[k].opencodec()

                    threadlist[k].show = thread_show
                    thread_show.InsertId(params.chanId)

                    threadlist[k].setDaemon(True)
                    threadlist[k].start()
                    upChanList.append(threadlist[k])
                else:
                    chanId = j
                    if j == i:
                        chanId = 0
                    (params.sessionId, params.chanId, params.actor) = (proc_idx, chanId, actor)
                    threadlist[k] = DecoderClient(params)
                    if threadlist[k].status == False:
                        print("RunSession: fail: threadlist: k=", k)
                        del threadlist[k]
                        continue
                    threadlist[k].init2()
                    #threadlist[k].AddSpeakerId(i)

                    threadlist[k].decode0.min_distance = 2
                    threadlist[k].decode0.delay_time = 100
                    threadlist[k].decode0.buf_size = (1 << 11)  # 1024#必须是2的指数
                    threadlist[k].decode0.mtu_size = 300
                    for thisThread in upChanList:
                        threadlist[k].add_client(thisThread)
                    #threadlist[k].opencodec()
                    threadlist[k].show = thread_show
                    thread_show.InsertId(params.chanId)
                    threadlist[k].setDaemon(True)
                    threadlist[k].start()
                ###
                #isok = True
            #if base_idx == 0:
            isok = True
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
                    threadlist[k].stop2()

        if base.global_render != None:
            base.global_render.stop()
        if thread_show != None:
            thread_show.stop()
        #thread.join()
        #time.sleep(5)

if __name__ == "__main__":
    print("start client")
    if True:
        print('父进程%d.' % os.getpid())
        # p = Process(target=RunSession, args=('test',))
        # print('子进程将要执行')
        # p.start()
        po = base.Pool(base.global_procnum)  # 定义一个进程池，最大进程数3
        for i in range(base.global_procnum):
            # Pool.apply_async(要调用的目标,(传递给目标的参数元祖,))
            # 每次循环将会用空闲出来的子进程去调用目标
            # po.apply_async(worker, (i, 1, 2, "strname"))
            port = base.global_port
            if base.MULT_SERVER:
                port = base.global_port + (i << 1)  # (video, audio)
            po.apply_async(RunSession, (base.global_baseidx, i, base.global_speakernum, base.global_host, port))
        idx = 0
        while idx >= 0 and idx < 4:
            try:
                idx = int(raw_input('please input to exit(eg: 0 ): '))
            except:
                idx = int(input('please input to exit(eg: 0 ): '))
            print("idx= ", idx)
            # thread.status = False if idx == 0 else True
        print("main: start stop...")
        print("----start----")
        po.close()  # 关闭进程池，关闭后po不再接收新的请求
        po.join()  # 等待po中所有子进程执行完成，必须放在close语句之后
    print("end client")

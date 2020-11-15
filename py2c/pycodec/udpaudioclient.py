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
from audiocapture import AudioCapture
from callaudiocodec import CallAudioCodec
from audioplayer import AudioPlayer

CMD_SIZE = 256
DATA_SIZE = 300#1500
SO_SNDBUF = 1024 * 1024
SO_RCVBUF = 1024 * 1024

EXCHANGE = 0# 1#0#1
QOS_LEVEL = 0


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
        if len(self.DataList) > 10:
            print("PlayThread: PopQueue2: len(self.DataList)= ", len(self.DataList))

        self.lock.release()
        return ret

    def PushQueue(self, id, data, recvTime):
        self.lock.acquire()
        self.DataList.append((id, data, recvTime))
        self.lock.release()


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
            if mix_num > 0:
                self.sdl.reset_mix_buf(mix_num)
                #print("PlayThread: run: mix_num= ", mix_num)

            i = 0
            for idx in self.idMap:
                item = self.PopQueue2(idx)
                ###test
                if self.idMap.index(idx) in [0]:
                    pass
                else:
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
                print("RecvCmdThread: run: recvfrom error", error)
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
                    # 解碼成功，緩存或顯示
                    data5 = outbuf2.raw[:osize2]
                    if self.client.show != None:
                        self.client.show.PushQueue(self.client.decode0.obj_id, data5, frame_timestamp)
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
        self.frame_decode = DecodeFrameThread(client)
        self.frame_decode.start()
        (self.param, self.outbuf, self.outparam) = self.client.decode0.getparam()
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
        if len(revcData) == 0:
            print("error: ResortPacket: insize = 0")
        # 重排序，並取幀
        (ret, outbuf, outparam, frame_timestamp) = self.client.decode0.resort(revcData, len(revcData))
        #print("ResortPacket: ret", ret)
        # print("self.outparam[1]= ", self.outparam[1])
        # print("ret= ", ret)
        if ret > 0 and True:
            # print("ResortPacket: ret= ", ret)
            runflag = True
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
            data = self.client.data_master.PopQueue()

            id = self.client.decode0.obj_id
            if data != None:
                self.recv_packet_num += 1
                (revcData, remoteHost, remotePort, recv_time) = data
                # recv_time = time.time()
                # print("TaskManagerThread: recv_time= ", recv_time)
                loss_rate = loadlib.gload.lib.api_count_loss_rate(revcData, len(revcData))
                if loss_rate >= 0:
                    print("Audio TaskManagerThread: loss_rate= ", loss_rate)
                self.ResortPacket(revcData, remoteHost, remotePort, recv_time)
            else:
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
        #print("DataManager: PushQueue: (self.id, n)= ", (self.id, n))
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
        print("PacedSend: init: self.client.encode0.bit_rate= ", self.client.encode0.bit_rate)
        print("PacedSend: init: self.client.encode0.mtu_size= ", self.client.encode0.mtu_size)

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
        print("audio PacedSend: stop 0")
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        print("audio PacedSend: stop 1")
        self.__running.clear()  # 设置为Fals
        print("audio PacedSend: stop 2")

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
                        sendDataLen = self.client.sock.sendto(data2, (self.client.host, self.client.port))
                    #except IOError, err:
                    except IOError as error:  # python3
                        print("PacedSend: run error: ", error)
                        break
                    else:
                        ###redundancy
                        if False:
                            sendDataLen = self.client.sock.sendto(data2, (self.client.host, self.client.port))
                            sendDataLen = self.client.sock.sendto(data2, (self.client.host, self.client.port))
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
                # print("PacedSend: run: self.client.encode0.bit_rate= ", (self.client.encode0.bit_rate))
                pass
            else:
                # print("PacedSend: run: self.client.encode0.bit_rate= ", (self.client.encode0.bit_rate))
                time.sleep(0.004)  # 4ms
        print("audio PacedSend: run over")

class EncoderClient(EchoClientThread):
    def __init__(self, id, sessionId, actor, host, port):
        EchoClientThread.__init__(self, actor, host, port)
        self.id = id
        self.sessionId = sessionId
        self.log_fp = None
        self.show = None
        self.capture = None
        try:
            filename = "../../mytest/enc_log_audio" + str(id) + ".txt"
            self.log_fp = open(filename, "w")
        except:
            print("EncoderClient:open file fail !")
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
        self.imglist = []
        self.img_num = 0

        self.encode0 = CallAudioCodec(id, 0)
        self.start_time = 0

        self.paced_send = PacedSend(self)
        ###
        self.in_channels = 2
        self.in_sample_rate = 48000
        self.in_sample_fmt = "AV_SAMPLE_FMT_S16"
        self.out_nb_samples = 1024
        self.frame_size = 1024 * 2 * 2
        self.outbuf = create_string_buffer(self.frame_size << 1)
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
        if self.outbuf != None:
            del self.outbuf
        if self.outparam != None:
            del self.outparam
        del self.sock
    def opendevice(self, devicetype):
        self.devicetype = devicetype
        if devicetype > 0:
            if devicetype == 1:
                self.input_name = "alsa"
                self.device_name = "default"
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
        ###
        cmd = {"actor": self.actor, "sessionId": self.sessionId}
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

    def run(self):
        sample_cnt = 0
        while self.__running.isSet():
            self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            (data, data_size) = self.capture.ReadFrame()
            if data_size > 0:
                self.outbuf[sample_cnt:(sample_cnt + 4096)] = data[0:4096]
                sample_cnt += data_size
                if sample_cnt >= 8192 and True:
                    (osize, outbuf, outparam) = self.encode0.codecframe(self.outbuf, 8192)
                    # print("audioplayer: osize= ", osize)
                    if osize > 0 and True:
                        #print("rtppacket: osize=", osize)
                        (osize, outbuf, outparam) = self.encode0.rtppacket(outbuf, osize)
                        rtpSize = []
                        rtpSize.append(str(osize))
                        self.send_data(outbuf, rtpSize)
                    sample_cnt = 0
            ###
        print("audio EncoderClient: run exit")
        if self.paced_send != None:
            self.paced_send.stop()

        if self.cmd_master != None:
            self.cmd_master.stop()
        self.encode0.codecclose()
        #self.fp.close()
        #self.sock.close()
        print("audio EncoderClient: run over")

    def stop(self):
        print("audio EncoderClient: stop paced_send 0")
        if self.capture != None:
            self.capture.stop()
        if self.paced_send != None:
            self.paced_send.stop()
            #self.paced_send = None
        print("audio EncoderClient: stop paced_send 1")
        self.cmd_master.stop()
        #self.cmd_master = None
        print("audio EncoderClient: stop cmd_master")
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals

        try:
            self.sock.shutdown(socket.SHUT_RDWR)
            self.sock.close()
        except:
            print("EncoderClient: stop error")
        else:
            print("EncoderClient: stop ok")
        if self.log_fp != None:
            self.log_fp.close()
        print("audio EncoderClient stop")

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
        self.log_fp = None
        try:
            filename = "../../mytest/dec_log_audio" + str(id) + ".txt"
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
        self.decode0 = CallAudioCodec(id, 1)
        ###
        self.data_master = DataManager(id)
        self.recv_task = RecvTaskManagerThread(self)
        self.recv_task.start()

        ###
        cmd = {"actor": self.actor, "sessionId": self.sessionId}
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
        self.decode0.init()
        pass
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
                print("audio DecoderClient: run: recvfrom error")
                break
            else:
                #print("DecoderClient: run: len(recvData)= ", len(recvData))
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
        print("audio DecoderClient: run over")

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
            print("audio DecoderClient: stop error")
        else:
            print("audio DecoderClient: stop ok")
        print("audo DecoderClient stop over")

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

        (bitrate, mtu_size, buffer_shift) = (24000, 300, 10)
        print("RunClient: bitrate= ", bitrate)
        # thread_show = PlayThread(6)
        thread_show = PlayThread(2)
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

        thread0 = DecoderClient(id0, sessionId0, actor0, global_host, global_port)
        #thread1 = DecoderClient(id1, sessionId1, actor1, global_host, global_port)
        #thread2 = DecoderClient(id2, sessionId2, actor2, global_host, global_port)
        #thread3 = DecoderClient(id3, sessionId3, actor3, global_host, global_port)


        thread4 = EncoderClient(id4, sessionId4, actor4, global_host, global_port)
        #thread5 = EncoderClient(id5, sessionId5, actor5, global_host, global_port)
        #thread6 = EncoderClient(id6, sessionId6, actor6, global_host, global_port)
        #thread7 = EncoderClient(id7, sessionId7, actor7, global_host, global_port)
        ###
        if True:
            if thread0 != None:
                thread0.decode0.min_distance = 2
                thread0.decode0.delay_time = 100
                thread0.decode0.buf_size = (1 << buffer_shift)  # 1024#必须是2的指数
                thread0.decode0.mtu_size = mtu_size
            if thread1 != None:
                thread1.decode0.min_distance = 2
                thread1.decode0.delay_time = 100
                thread1.decode0.buf_size = (1 << buffer_shift)  # 1024#必须是2的指数
                thread1.decode0.mtu_size = mtu_size
            if thread2 != None:
                thread2.decode0.min_distance = 2
                thread2.decode0.delay_time = 100
                thread2.decode0.buf_size = (1 << buffer_shift)  # 1024#必须是2的指数
                thread2.decode0.mtu_size = mtu_size
            if thread3 != None:
                thread3.decode0.min_distance = 2
                thread3.decode0.delay_time = 100
                thread3.decode0.buf_size = (1 << buffer_shift)  # 1024#必须是2的指数
                thread3.decode0.mtu_size = mtu_size
            if thread8 != None:
                thread8.decode0.min_distance = 2
                thread8.decode0.delay_time = 100
                thread8.decode0.buf_size = (1 << buffer_shift)  # 1024#必须是2的指数
                thread8.decode0.mtu_size = mtu_size
            if thread9 != None:
                thread9.decode0.min_distance = 2
                thread9.decode0.delay_time = 100
                thread9.decode0.buf_size = (1 << buffer_shift)  # 1024#必须是2的指数
                thread9.decode0.mtu_size = mtu_size
        if True:
            if thread4 != None:
                thread4.encode0.bit_rate = bitrate
                thread4.encode0.mtu_size = mtu_size
                thread4.opendevice(1)
                thread4.opencodec()
            if thread5 != None:
                thread5.encode0.bit_rate = bitrate
                thread5.encode0.mtu_size = mtu_size
                thread5.opendevice(1)
                thread5.opencodec()
            if thread6 != None:
                thread6.encode0.bit_rate = bitrate
                thread6.encode0.mtu_size = mtu_size
                thread6.opendevice(1)
                thread6.opencodec()
            if thread7 != None:
                thread7.encode0.bit_rate = bitrate
                thread7.encode0.mtu_size = mtu_size
                thread7.encode0.setparam()
                thread7.opendevice(1)
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

        if thread_show != None:
            thread_show.stop()


if __name__ == "__main__":
    print("start client")
    # udpClient = UdpClient()
    # udpClient.tcpclient()
    RunClient(True)
    print("end client")

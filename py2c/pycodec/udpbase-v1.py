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
import queue
import time
import signal
import json
#from ctypes import *
import ctypes
from ctypes import *
from ctypes import c_longlong as ll
import random
import numpy as np
import errno
import struct
import platform
import copy
import loadserver

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
#note:
#localhost:
# DATA_SIZE=800, 历史峰值为1446Mbps #430Mbps, 丢包率为1.8%, delay为29
# DATA_SIZE=1500, 历史峰值为2500Mbps #700Mbps, 丢包率为0.6%, delay为24
# DATA_SIZE=64k, 历史峰值为2000Mbps, 丢包率为0, delay为2
# 标准的以太网IP报文大小是：1500 bytes，不包含以太网头和FCS的18 bytes（6+6+2+4），
# 如果包含以太网头和FCS，则为1518 bytes
# network card (1 client):
# mtusize = 1400, 历史峰值为900Mbps, rate = 90%
#python udptest.py 0 0 10.184.101.78
#python udptest.py 1 0 10.184.101.78

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
#DATA_SIZE = 64 * 1024

SO_SNDBUF = 64 * 1024
SO_RCVBUF = 64 * 1024
LongMaxBufLen = 30 * 1024 #每帧最大1024个包，最多保存30帧
ShortMaxBufLen = 100 #4 * 1024 #每帧最大1024个包，最大延时4帧间隔

gTag = "video:"
gFreq = 90

global_port = 8097 #8888
global_host = 'localhost'
global_actor = 0
global_idx = 0
#global_host = '172.16.0.17'
#global_host = '111.230.226.17'
if len(sys.argv) > 4:
    global_port = int(sys.argv[4])
if len(sys.argv) > 3:
    global_host = sys.argv[3]
if len(sys.argv) > 2:
    global_idx = int(sys.argv[2])
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
            print("ReadCmd: not cmd")
            # print("not cmd: str_cmd=", str_cmd)
        else:
            pass
    else:
        pass
    return outjson

class RecvThread(threading.Thread):
    def __init__(self, client):
        threading.Thread.__init__(self)
        self.client = client
        self.start_time = time.time()
        self.data_master = DataManager(client.revbufsize)
        self.recv_packet_num = 0
        self.start_time = 0
        self.times = 0
        self.offset = 0
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
    def getinfo(self, now_time, revDataLen):
        self.times += 1
        self.offset += revDataLen
        self.recv_packet_num += (revDataLen > 0)
        difftime = int((now_time - self.start_time) * 1000)
        if difftime > 2000 and self.start_time > 0:
            freq = int((self.times * 1000) / difftime)
            print("RecvThread: run: access freq= ", freq)
            bitrate = ((self.offset) << 3)  #
            bitrate = int(1000 * bitrate / difftime)
            # / (1024 * 1024)
            bitrate >>= 20
            # bitrate /= (1024 * 1024)
            if bitrate > 0:
                print(gTag + "RecvThread: run: recvfrom: bitrate(Mbps)= ", bitrate)
            self.start_time = now_time
            #self.recv_packet_num = 0
            self.times = 0
            self.offset = 0
    def run(self):
        while self.__running.isSet():
            self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            recvData = None
            data = ""
            revDataLen = 0
            now_time = 0#time.time()
            try:
                rdata = self.client.sock.recvfrom(DATA_SIZE)
                if rdata[0] != '' and rdata[1] != None:
                    recvData, (remoteHost, remotePort) = rdata
                else:
                    print(gTag + "RecvThread: run: rdata= ", rdata)
                    break
            # except IOError, error:  # python2
            except IOError as error:  # python3
                if hasattr(error, '__iter__') and "timed out" in error:
                    # print("EchoServerThread: run: timeout: recvfrom error= ", error)
                    pass
                elif error.errno == errno.EWOULDBLOCK:
                    # print("EchoServerThread: run: EWOULDBLOCK: recvfrom error= ", error)
                    pass
                elif error.errno == errno.WSAETIMEDOUT:
                    pass
                elif error.errno == errno.ENAMETOOLONG:
                    # print("EchoServerThread: run: ENAMETOOLONG: recvfrom error= ", error)
                    pass
                else:
                    print(gTag + "RecvThread: run: recvfrom error= ", error)
            else:
                now_time = time.time()
                if self.start_time == 0:
                    self.start_time = now_time
                revDataLen = len(recvData)
                # print(gTag + "EchoServerThread: run: recvfrom: revDataLen= ", revDataLen)
                # print(gTag + "EchoServerThread: run: recvData= ", recvData)
                self.data_master.PushQueue(recvData, remoteHost, remotePort, now_time)  # c_time_ll)

            self.getinfo(now_time, revDataLen)
        #self.sock.close()
        print(gTag + "RecvThread: run over")
    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals

        print(gTag + "RecvThread: stop")

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞

class NetInfo(object):
    def __init__(self, idx):
        threading.Thread.__init__(self)
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
    def CountInfo(self, revdata, idx):
        #print("CountInfo: (idx, self.idx, self.cnt)= ", (idx, self.idx, self.cnt))
        if self.idx < 0:
            self.idx = idx
        if idx != self.idx:
            return 0
        (seqnum, time, time2, data) = revdata
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
        delay0 = (time2 - time)
        if self.cnt < self.m:
            self.rtt += delay0
        if time < self.mintime:
            self.mintime = time
        if time > self.maxtime:
            self.maxtime = time

        if delay0 > self.delay:
            self.delay = delay0

        self.avgdelay += delay0
        self.cnt += 1
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
        #print(gTag + "CountLossRate2: ret= ", ret)
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
    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals

        print(gTag + "NetInfoThread: stop")

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞
#以socket为单元，多线程处理
class SocketThread(threading.Thread):
    def __init__(self, idx):
        threading.Thread.__init__(self)
        self.idx = idx
        self.lock = threading.Lock()
        self.start_time = 0
        self.DataList = []
        self.netInfo = NetInfo(idx)

        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
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
        #self.__flag.set()
    def run(self):
        while self.__running.isSet():
            self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            now_time = time.time()
            if self.start_time <= 0:
                self.start_time = now_time
            #data = self.PopQueue()
            data = self.PopQueueAll()
            if self.netInfo != None:
                for thisdata in data:
                    self.netInfo.CountInfo(thisdata, self.idx)
                difftime = int((now_time - self.start_time) * 1000)
                if difftime >= 2000 and True:
                    now_time0 = time.time()
                    # ret = self.CountLossRate()
                    ret = self.netInfo.CountLossRate2()
                    if len(ret) == 0:
                        if data == None or data == []:
                            # 过频繁的访问会导致锁被占用
                            time.sleep(0.001)
                        continue
                    now_time1 = time.time()
                    difftime = int((now_time1 - now_time0) * 1000)
                    #print(gTag + "SocketThread: run: CountLossRate: difftime= ", difftime)
                    print(gTag + "SocketThread: run: CountLossRate: ret= ", ret)
                    self.start_time = time.time()

            #print(gTag + "NetInfoThread: run: CountLossRate: difftime= ", difftime)
            if data == None or data == []:
                #过频繁的访问会导致锁被占用
                time.sleep(0.001)
            else:
                #time.sleep(0.00001) #影响收包效率!!!
                pass
    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals
        print(gTag + "DataProcessThread: stop")

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞
#数据socket分散处理
class SocketProcessThread(threading.Thread):
    def __init__(self, server):
        threading.Thread.__init__(self)
        self.server = server
        self.lock = threading.Lock()
        self.socketDict = {}
        self.start_time = 0
        self.netDataList = []
        ###
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True

        self.readable = threading.Event()
        self.readable.clear()
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
    def ProcessData(self):
        #datalist = self.PopQueue()
        datalist = self.PopQueueAll() #slower than push as the flollow case, it will consume too much time
        if datalist != [] and True:
            for thisDataList in datalist:
                for data in thisDataList:
                    (revcData, remoteHost, remotePort, recv_time) = data
                    sockId = remoteHost + "_" + str(remotePort)
                    jsondata = str2json(revcData)
                    if jsondata != None:
                        # cmd = {"idx":self.idx, "seqnum":self.send_packet_num, "time":now_time, "data": self.testdata}
                        idx = jsondata.get("idx")
                        seqnum = jsondata.get("seqnum")
                        time = jsondata.get("time")
                        data = jsondata.get("data")
                        time2 = recv_time
                        revdata = (seqnum, time, time2, data)
                        sockThread = self.socketDict.get(sockId)
                        if sockThread == None:
                            sockThread = SocketThread(idx)
                            #sockThread.setPriority(2)
                            sockThread.setDaemon(True)
                            sockThread.start()
                            self.socketDict.update({sockId: sockThread})
                        sockThread.PushQueue(revdata)
                    else:
                        print("SocketProcessThread: revcData= ", revcData)
                        print("SocketProcessThread: recv_time= ", recv_time)
        return datalist
    def run(self):
        while self.__running.isSet():
            self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回

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
        self.readable.set()
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals
        for key, value in self.socketDict.items():
            if value != None:
                value.stop()
        print(gTag + "DataProcessThread: stop")

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

            difftime = int((now_time - self.start_time) * 1000)
            if difftime > 2000 and self.start_time > 0:
                freq = int((self.times * 1000) / difftime)
                print("DataProcessThread: run: access freq= ", freq)
                self.start_time = now_time
                self.times = 0

    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals
        if self.sockProcess != None:
            self.sockProcess.stop()
        print(gTag + "DataProcessThread: stop")

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞
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
        print(gTag + "DataManager del")
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
        if n > 300:
            (revcData0, remoteHost0, remotePort0, recvTime0) = self.DataList[0]
            (revcData1, remoteHost1, remotePort1, recvTime1) = self.DataList[n - 1]
            delay = int((recvTime1 - recvTime0) * 1000)
            ##print("PushQueue: (n, delay)= ", (n, delay))
            #self.DataList = [] #test
        self.lock.release()
        #self.readable.set() #会减缓push节奏

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
class EchoClientThread(threading.Thread):
    def __init__(self, actor, host, port):
        threading.Thread.__init__(self)
        global global_idx

        self.idx = global_idx
        global_idx += 1
        self.actor = actor
        self.host = host
        self.port = port

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        #self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, SO_SNDBUF)
        #self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, SO_RCVBUF)
        revbufsize = self.sock.getsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF)
        #revbufsize >>= 10
        print("revbufsize(kB)= ", revbufsize >> 10)
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

        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
        ###
        #self.data_master = DataManager()
        self.data_recv = None

        #self.data_recv = RecvThread(self)
        #self.data_recv.start()

        self.data = create_string_buffer(DATA_SIZE)
        self.testdata = ""
        self.str_cmd = None
        self.start_time = 0
        self.times = 0
        self.send_packet_num = 0
        self.offset = 0
        self.init()
    def __del__(self):
        print("EchoClientThread del")
    def init(self):
        for i in range(DATA_SIZE - 100):
            self.data[i] = i % 10
            self.testdata += str(i%10)
    def renewdata(self):
        seqnum = str(self.send_packet_num)
        self.data[0:] = seqnum
    def getinfo(self, now_time, sendDataLen):
        self.times += 1
        self.offset += sendDataLen
        self.send_packet_num += (sendDataLen > 0)
        difftime = int((now_time - self.start_time) * 1000)
        if difftime > 2000 and self.start_time > 0:
            freq = int((self.times * 1000) / difftime)
            print("EchoClientThread: run: access freq= ", freq)
            bitrate = ((self.offset) << 3)  #
            bitrate = int(1000 * bitrate / difftime)
            # / (1024 * 1024)
            bitrate >>= 20
            # bitrate /= (1024 * 1024)
            if bitrate > 0:
                print(gTag + "EchoClientThread: run: sendto: bitrate(Mbps)= ", bitrate)
            self.start_time = now_time
            #self.send_packet_num = 0
            self.times = 0
            self.offset = 0
    def run(self):
        while self.__running.isSet():
            #self.__flag.wait()  #会延缓发送频率
            sendDataLen = 0
            now_time = time.time()#会影响发包效率
            try:
                #self.renewdata()
                #now_time = time.time()#会影响发包效率
                if self.str_cmd == None:
                    cmd = {"idx":self.idx, "seqnum":self.send_packet_num, "time":now_time, "data": self.testdata}
                    str_cmd = json2str(cmd)
                    ##self.str_cmd = str_cmd
                else:
                    data = ""
                    #data = str(self.idx) + ","
                    data = str(self.idx) + "," + str(self.send_packet_num) + ","
                    #data = str(self.idx) + "," + str(self.send_packet_num) + "," + str(now_time) + ","
                    #inow_time = int(now_time * 1000 * 1000 * 1000)
                    #data = str(self.idx) + "," + str(self.send_packet_num) + "," + str(inow_time) + ","
                    data += self.testdata
                    str_cmd = data.encode('utf-8')
                    #str_cmd = self.str_cmd

                if (self.send_packet_num % 10) != 0 and False:#test loss rate
                    sendDataLen = self.sock.sendto(str_cmd, (self.host, self.port))
                else:
                    sendDataLen = self.sock.sendto(str_cmd, (self.host, self.port))
            # except IOError, error:
            except IOError as error:  # python3
                print("run error: ", error)
                break
            else:
                #if (self.send_packet_num % 100000) == 0:
                #    print("EchoClientThread: run: (sendDataLen, self.idx)=", (sendDataLen, self.idx))
                if self.start_time == 0:
                    self.start_time = now_time
                #time.sleep(0.00001)
            self.getinfo(now_time, sendDataLen)
            #time.sleep(0.0001)
            #time.sleep(0.00001)
            if False:
                sleepn = 1000 #500 #1000
                for i in range(sleepn):
                    j = 0
        self.sock.close()

    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals
        print(gTag + "EchoClientThread: start stop")
        if self.data_recv != None:
            self.data_recv.stop()
        try:
            self.sock.shutdown(socket.SHUT_RDWR)
            self.sock.close()
            pass
        # except IOError, error:  # python2
        except IOError as error:  # python3
            print(gTag + "stop error: ", error)
        else:
            print(gTag + "stop ok")

        print(gTag + "EchoClientThread: stop over")

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞


class EchoServerThread(threading.Thread):
    def __init__(self, actor, host, port):
        threading.Thread.__init__(self)
        ###
        self.tid = None
        self.setName("EchoServerThread")
        ###
        self.load = loadserver.gload #LoadLib()
        if self.load == None:
            self.status = False
            return
        array_type = c_char_p * 4
        self.outparam = array_type()
        self.actor = actor
        self.host = host
        self.port = port

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
        self.sock.bind((self.host, self.port))  # 绑定同一个域名下的所有机器
        if False:
            self.sock.setblocking(False)
            self.sock.settimeout(10)
        print(gTag + "EchoServerThread: self.host= ", self.host)
        print(gTag + "EchoServerThread: self.port= ", self.port)
        handle_size = 16
        self.ll_handle = create_string_buffer(handle_size)
        # self.ll_handle = (c_char * handle_size)()
        array_type = c_char_p * 1
        self.ctime = array_type()
        ###
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True

        self.data_master = DataManager(self.revbufsize)
        self.process = None

        self.process = DataProcessThread(self)
        #self.process.setPriority(2)
        self.process.setDaemon(True)
        self.process.start()

        self.recv_packet_num = 0
        self.start_time = 0
        self.times = 0
        self.offset = 0
    def __del__(self):
        print(gTag + "EchoServerThread del")

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
        difftime = int((now_time - self.start_time) * 1000)
        if difftime > 2000 and self.start_time > 0:
            freq = int((self.times * 1000) / difftime)
            print("EchoServerThread: run: access freq= ", freq)
            bitrate = ((self.offset) << 3)  #
            bitrate = int(1000 * bitrate / difftime)
            # / (1024 * 1024)
            bitrate >>= 20
            # bitrate /= (1024 * 1024)
            if bitrate > 0:
                print(gTag + "EchoServerThread: run: recvfrom: bitrate(Mbps)= ", bitrate)
            self.start_time = now_time
            #self.recv_packet_num = 0
            self.times = 0
            self.offset = 0
    def run(self):
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
                    print(gTag + "EchoServerThread: run: rdata= ", rdata)
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
                    print(gTag + "EchoServerThread: run: recvfrom error= ", error)
            else:
                now_time = time.time()
                if self.start_time == 0:
                    self.start_time = now_time
                #print(gTag + "EchoServerThread: run: recvData= ", recvData)
                self.data_master.PushQueue(recvData, remoteHost, remotePort, now_time)# c_time_ll)
                revDataLen = len(recvData)

            self.getinfo(now_time, revDataLen)
        self.sock.close()
        print(gTag + "EchoServerThread: run over")
    def stop(self):
        if self.data_master != None:
            self.data_master.readable.set()
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals
        print(gTag + "EchoServerThread: start stop")

        if self.process != None:
            self.process.stop()
        try:
            self.sock.shutdown(socket.SHUT_RDWR)
            self.sock.close()
            pass
        #except IOError, error:  # python2
        except IOError as error:  # python3
            print(gTag + "stop error: ", error)
        else:
            print(gTag + "stop ok")

        print(gTag + "EchoServerThread: stop over")
    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞
def RunServer(flag):
    if flag:
        # idx = raw_input('please input to exit(eg: -1 ): ')
        # count = 0
        idx = 0
        thread = EchoServerThread(0, global_host, global_port)
        thread.setDaemon(True)
        thread.start()
        thread.setPriority(THREAD_PRIORITY_ABOVE_NORMAL)
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
def RunClient(flag):
    if flag:
        # idx = raw_input('please input to exit(eg: -1 ): ')
        # count = 0
        idx = 0
        clientnum = 1#16#2#1#16 #2#1#2
        threadlist = [None for x in range(clientnum)]
        for i in range(clientnum):
            threadlist[i] = EchoClientThread(1, global_host, global_port)
            threadlist[i].setDaemon(True)
            threadlist[i].start()
            #threadlist[i].setPriority(1)
        while idx >= 0 and idx < 4:
            try:
                idx = int(raw_input('please input to exit(eg: 0 ): '))
            except:
                idx = int(input('please input to exit(eg: 0 ): '))
            print("idx= ", idx)
            #thread.status = False if idx == 0 else True
        print("main: start stop...")
        for i in range(clientnum):
            threadlist[i].stop()
        #thread.join()
        #time.sleep(5)
def RunSession(actor):
    if actor == 0:
        RunServer(True)
    else:
        RunClient(True)
if __name__ == "__main__":
    print("start udptest")
    RunSession(global_actor)
    print("end udptest")

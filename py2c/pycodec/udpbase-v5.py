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
from ctypes import c_longlong as ll
import random
import numpy as np
import errno
import struct
import platform
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
CMD_SIZE = DATA_SIZE #test

SO_SNDBUF = 64 * 1024
SO_RCVBUF = 64 * 1024
LongMaxBufLen = 30 * 1024 #每帧最大1024个包，最多保存30帧
ShortMaxBufLen = 100 #4 * 1024 #每帧最大1024个包，最大延时4帧间隔
TEST_LOSSRATE = False #True #False
PACE_BITRATE = 0
PACE_BITRATE = 380*1024*1024 #固定码率发送
PACE_BITRATE = 512*1024
FIX_CMD_SIZE = 160
SELFMODE = 0
MULT_THREAD = False
MULT_SERVER = False
INTERVAL = 10000000 #2000000

#支持4 * 16 (* 16)并发



#["ack_cmd", "relay_data", "send_data"]
gTag = "video:"
gFreq = 90

global_port = 8097 #8888
global_host = 'localhost'
global_actor = 0
global_speakernum = 1
global_procnum = 1
#global_host = '172.16.0.17'
#global_host = '111.230.226.17'
if len(sys.argv) > 5:
    global_port = int(sys.argv[5])
if len(sys.argv) > 4:
    global_host = sys.argv[4]
if len(sys.argv) > 3:
    global_speakernum = int(sys.argv[3])
if len(sys.argv) > 2:
    global_procnum = int(sys.argv[2])
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
class CmdData(object):
    def __init__(self):
        self.cmdtype = ""
        self.actor = -1
        self.sessionId = -1
        self.chanId = -1
        self.modeId = -1
        self.selfmode = SELFMODE
        self.seqnum = -1
        self.now_time = 0
        self.status = 1
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
        cmd.update({"seqnum": self.seqnum})
        cmd.update({"time": self.now_time})
        cmd.update({"status": self.status})
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
        self.seqnum = jsondata.get("seqnum")
        self.now_time = jsondata.get("time")
        self.status = jsondata.get("status")
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
        self.sock = client.sock
        self.host = client.host
        self.port = client.port
        self.revbufsize = client.revbufsize
        self.lock = threading.Lock()
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
        self.init()
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
        self.cmddata.cmdtype = "Reg"
        self.cmddata.actor = self.actor
        self.cmddata.sessionId = self.sessionId
        self.cmddata.chanId = self.chanId
        self.cmddata.modeId = 0
        self.cmddata.selfmode = SELFMODE
        self.cmddata.seqnum = self.send_packet_num
        self.cmddata.now_time = 0
        self.cmddata.data = self.testdata
        self.cmddata.SetCmd()
    def createdata(self, now_time):
        if self.str_cmd == None and False:
            #"actor": self.actor, "sessionId": self.sessionId, "chanId": self.id
            #cmd = {"actor": self.actor, "sessionId": self.sessionId, "chanId": self.chanId, "seqnum": self.send_packet_num, "time": now_time, "data": self.testdata}
            self.cmddata.cmdDict.update({"seqnum": self.send_packet_num})
            self.cmddata.cmdDict.update({"time": now_time})
            cmd = self.cmddata.cmdDict
            str_cmd = json2str(cmd)
            ##self.str_cmd = str_cmd
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
                if bitrate > 1024:
                    bitrate = (bitrate + 512) >> 10
                    print(gTag + "SendThread: run: sendto: bitrate(Mbps)= ", (sessionId, chanId, bitrate))
                else:
                    print(gTag + "SendThread: run: sendto: bitrate(Kbps)= ", (sessionId, chanId, bitrate))
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
                        sendDataLen = self.sock.sendto(str_cmd, (host, port))
                        self.last_snd_time = now_time
                elif str_cmd != None:
                    sendDataLen = self.sock.sendto(str_cmd, (host, port))
                    self.last_snd_time = now_time
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
                        if TEST_LOSSRATE:
                            if (count % 10) != 9:
                                sendDataLen = self.sock.sendto(str_cmd, (host, port))
                                self.last_snd_time = now_time
                        elif str_cmd != None:
                            sendDataLen = self.sock.sendto(str_cmd, (host, port))
                            self.last_snd_time = now_time
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
        count = 0
        while self.__running.isSet():
            (count, sendDataLen, difftime) = self.fullspeed(count)
            if self.attribute > 0:
                if self.bitrate > 0 and sendDataLen > 0:
                    self.pacesend(sendDataLen, difftime)
            else:
                time.sleep(0.1)
        self.dead.set()
        print(gTag + "SendThread: run over")
    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals

        print(gTag + "SendThread: stop over")

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞
class RecvThread(threading.Thread):
    def __init__(self, client):
        threading.Thread.__init__(self)
        self.client = client
        self.sessionId = client.sessionId
        self.chanId = client.chanId
        self.idx = (self.sessionId << 16) + self.chanId
        self.sock = client.sock
        self.revbufsize = client.revbufsize
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

        self.send_packet_num = 0
        self.start_time = 0
        self.times = 0
        self.offset = 0
        self.interval = INTERVAL

        self.data_master = DataManager(self.revbufsize)

    def __del__(self):
        print(gTag + "RecvThread del")
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
        self.send_packet_num += (revDataLen > 0)
        difftime = int((now_time - self.start_time) * 1000000)
        if difftime > self.interval and self.start_time > 0:
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
                if bitrate > 1024:
                    bitrate = (bitrate + 512) >> 10
                    print(gTag + "RecvThread: run: recvfrom: bitrate(Mbps)= ", (sessionId, chanId, bitrate))
                else:
                    print(gTag + "RecvThread: run: recvfrom: bitrate(Kbps)= ", (sessionId, chanId, bitrate))
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
                    print(gTag + "RecvThread: run: rdata= ", rdata)
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
                    print(gTag + "RecvThread: run: recvfrom error= ", error)
            else:
                now_time = time.time()
                if self.start_time == 0:
                    self.start_time = now_time
                #print(gTag + "EchoServerThread: run: recvData= ", recvData)
                self.data_master.PushQueue(recvData, remoteHost, remotePort, now_time)# c_time_ll)
                revDataLen = len(recvData)

            self.getinfo(now_time, revDataLen)
        self.dead.set()
        print(gTag + "RecvThread: run over")
    def stop(self):
        if self.data_master != None:
            self.data_master.readable.set()
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals

        print(gTag + "RecvThread: stop over")
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
    def CountInfo(self, revdata, idx):
        #print("CountInfo: (idx, self.idx, self.cnt)= ", (idx, self.idx, self.cnt))
        (seqnum, time, time2, data) = revdata
        if seqnum == None:
            return 0
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
        if lossrate > 0:
            print(gTag + "CountLossRate2: ret= ", ret)
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
    def __init__(self, idx):
        self.idx = idx
        self.netInfo = NetInfo(idx)
    def Process(self, data):
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
            item = revcData.decode().split(",")
            seqnum = int(item[0])
            time = float(item[1])
            thisdata = item[2]
            revdata = (seqnum, time, recv_time, thisdata)
            if self.netInfo != None:
                self.netInfo.CountInfo(revdata, self.idx)
        return (revdata, data)
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
        if (actor in [3]):
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
                    #print(gTag + "SocketThread: run: CountLossRate: ret= ", ret)
                    self.start_time = time.time()

            #print(gTag + "NetInfoThread: run: CountLossRate: difftime= ", difftime)
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
        print(gTag + "SocketThread: stop")

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞
class CmdThread(threading.Thread):
    def __init__(self, parent):
        threading.Thread.__init__(self)
        self.parent = parent
        self.server = parent.server
        self.sock = parent.server.sock
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
        self.data_master = DataManager(self.server.revbufsize)
    def CmdProcess(self, data):
        iscmd = True
        (revcData, remoteHost, remotePort, recv_time) = data
        sockId = remoteHost + "_" + str(remotePort)
        n = len(revcData)
        iscmd = n <= CMD_SIZE and ('{' in str(revcData[0:3])) and ('}' in str(revcData[n - 3:]))
        if iscmd:
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
                selfmode = cmddata.selfmode
                actor = cmddata.actor
                time2 = recv_time
                revdata = (cmddata.seqnum, cmddata.now_time, time2, cmddata.data)
                (sockThread, sessionId2) = (None, None)

                self.parent.lock2.acquire()
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
                    sockId = remoteHost + "_" + str(remotePort)
                    sessionInfo = self.parent.sessionDict.get(sessionId)
                    if sessionInfo == None:
                        self.parent.sessionDict.update({sessionId: {actor: [(sockId, chanId, recv_time)]}})
                        #(modeId, selfmode) = (0, 0)
                        self.parent.sessionDict[sessionId].update({"info": {"modeId": modeId, "selfmode": selfmode}})
                        chanIdList = [chanId]
                        self.parent.sessionDict[sessionId]["info"].update({"chanIdList": chanIdList})
                        dataDict = {}
                        # dataDict.update({sockId: {"longdata": [(revcData, recv_time)]}})
                        # dataDict[sockId].update({"data": [(revcData, recv_time)]})
                        dataDict.update({sockId: {"longdata": []}})
                        dataDict[sockId].update({"shortdata": []})
                        self.parent.sessionDict[sessionId].update({"data": dataDict})
                    else:
                        sessionInfo["data"].update({sockId: {"longdata": []}})
                        sessionInfo["data"][sockId].update({"shortdata": []})
                        if sessionInfo.get(actor) == None:
                            sessionInfo.update({actor: [(sockId, chanId, recv_time)]})
                        else:
                            sessionInfo[actor].append((sockId, chanId, recv_time))
                        sessionInfo["info"]["chanIdList"].append(chanId)

                #self.parent.lock2.acquire()
                #self.sessionDict = sessionDict
                #self.socketDict = socketDict
                self.parent.lock2.release()
                if sockThread != None and True:
                    sockThread.PushQueue((revdata, revcData))
                else:
                    sendDataLen = self.sock.sendto(revcData, (remoteHost, remotePort))#ack
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
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals
        print(gTag + "CmdThread: stop")

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
        print(gTag + "CmdThread: stop")

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
                if bitrate > 1024:
                    bitrate = (bitrate + 512) >> 10
                    print(gTag + "SendThread: run: sendto: bitrate(Mbps)= ", (sessionId, bitrate))
                else:
                    print(gTag + "SendThread: run: sendto: bitrate(Kbps)= ", (sessionId, bitrate))
            self.start_time = now_time
            #self.send_packet_num = 0
            self.times = 0
            self.offset = 0
        return difftime
    def CmdProcess(self, data):
        iscmd = True
        (revcData, remoteHost, remotePort, recv_time) = data
        sockId = remoteHost + "_" + str(remotePort)
        n = len(revcData)
        iscmd = n <= CMD_SIZE and ('{' in str(revcData[0:3])) and ('}' in str(revcData[n - 3:]))
        if iscmd:
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
                selfmode = cmddata.selfmode
                actor = cmddata.actor
                time2 = recv_time
                revdata = (cmddata.seqnum, cmddata.now_time, time2, cmddata.data)
                (sockThread, sessionId2) = (None, None)
                sockItem = self.socketDict.get(sockId)
                if sockItem == None:
                    #if (actor in [3]):
                    if MULT_THREAD:
                        sockThread = SocketThread(self, sessionId, chanId, actor, selfmode, remoteHost, remotePort)
                        # sockThread.setPriority(2)
                        sockThread.setDaemon(True)
                        sockThread.start()
                    self.socketDict.update({sockId: (sockThread, sessionId, chanId, actor, selfmode, remoteHost, remotePort)})
                else:
                    (sockThread, sessionId2, chanId2, actor2, selfmode2, remoteHost2, remotePort2) = sockItem
                if True:
                    #会影响转发率
                    sockId = remoteHost + "_" + str(remotePort)
                    sessionInfo = self.sessionDict.get(sessionId)
                    if sessionInfo == None:
                        self.sessionDict.update({sessionId: {actor: [(sockId, chanId, recv_time)]}})
                        #(modeId, selfmode) = (0, 0)
                        self.sessionDict[sessionId].update({"info": {"modeId": modeId, "selfmode": selfmode}})
                        chanIdList = [chanId]
                        self.sessionDict[sessionId]["info"].update({"chanIdList": chanIdList})
                        dataDict = {}
                        # dataDict.update({sockId: {"longdata": [(revcData, recv_time)]}})
                        # dataDict[sockId].update({"data": [(revcData, recv_time)]})
                        dataDict.update({sockId: {"longdata": []}})
                        dataDict[sockId].update({"shortdata": []})
                        self.sessionDict[sessionId].update({"data": dataDict})
                    else:
                        sessionInfo["data"].update({sockId: {"longdata": []}})
                        sessionInfo["data"][sockId].update({"shortdata": []})
                        if sessionInfo.get(actor) == None:
                            sessionInfo.update({actor: [(sockId, chanId, recv_time)]})
                        else:
                            sessionInfo[actor].append((sockId, chanId, recv_time))
                        sessionInfo["info"]["chanIdList"].append(chanId)
                if sockThread != None and True:
                    sockThread.PushQueue((revdata, revcData))
                else:
                    sendDataLen = self.sock.sendto(revcData, (remoteHost, remotePort))#ack
        return iscmd
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
                item = revcData.decode().split(",")
                seqnum = int(item[0])
                time0 = float(item[1])
                thisdata = item[2]
                revdata = (seqnum, time0, recv_time, thisdata)
                actors = [3]
                #actors = [1, 2, 3] #selfmode
                for thisactor in actors:
                    decodeActors = sessionInfo.get(thisactor)
                    if decodeActors != None:
                        for (sockId2, chanId2, recv_time2) in decodeActors:
                            sockItem2 = self.socketDict.get(sockId2)
                            if sockItem2 != None:
                                (sockThread2, sessionId2, chanId2, actor2, selfmode2, remoteHost2, remotePort2) = sockItem2
                                if sessionId2 == sessionId and chanId2 == chanId:
                                    self.lock2.release()
                                    if sockThread2 != None:
                                        sockThread2.PushQueue((revdata, revcData))
                                    else:
                                        sendDataLen = self.sock.sendto(revcData, (remoteHost2, remotePort2))  #relay
                                        sumlen += sendDataLen
                                    self.lock2.acquire()
                                    sent = True

        else:
            print("error: RawProcess: (remoteHost, remotePort)= ", (remoteHost, remotePort))

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
                        #iscmd = self.CmdProcess(data)
                        self.cmdthread.data_master.PushQueue1(data)
                    else:
                        self.RelayProcess(data)
                        #print("SocketProcessThread: revcData= ", revcData)
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
        print(gTag + "SocketProcessThread: stop")

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

class EchoClientThread(threading.Thread):
    def __init__(self, sessionId, chanId, actor, host, port):
        threading.Thread.__init__(self)
        self.actor = actor
        self.sessionId = sessionId
        self.chanId = chanId
        self.modeId = 0
        self.selfmode = 1
        self.status = False
        idx = (sessionId << 16) + chanId
        #self.actor = actor
        self.host = host
        self.port = port

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
        print("EchoClientThread: init 0")
        sucess = self.Register()
        if sucess == False:
            return
        self.status = True
        self.data_master = None
        self.dataprocess = None
        self.netInfo = None

        self.sendthread = None
        if actor in [1, 2]:
            self.sendthread = SendThread(self, True, 2) #"send_data"
        self.recvthread = None
        self.recvthread = RecvThread(self)
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True

        if self.sendthread != None:
            self.sendthread.setDaemon(True)
            self.sendthread.start()
        if self.recvthread != None:
            self.recvthread.setDaemon(True)
            self.recvthread.start()
            self.data_master = self.recvthread.data_master
            self.dataprocess = DataProcess(idx)
            self.netInfo = self.dataprocess.netInfo

        self.start_time = 0
        self.interval = INTERVAL
        print("EchoClientThread: init ok")

    def __del__(self):
        print("EchoClientThread del")
    def CmdCore(self, cmd):
        str_cmd = json2str(cmd)
        sendDataLen = self.sock.sendto(str_cmd, (self.host, self.port))
        acked = False
        count = 0
        while (acked == False):
            try:
                rdata = self.sock.recvfrom(CMD_SIZE)
            # except IOError, error:  # python2
            except IOError as error:  # python3

                if error.errno == errno.EWOULDBLOCK:
                    pass
                elif error.errno == errno.WSAETIMEDOUT:
                    pass
                else:
                    print(gTag + "EncoderClient: CmdCore: recvfrom error= ", error)
                    break
            else:
                if rdata[0] != '' and rdata[1] != None:
                    acked = True
                    thisDict = str2json(rdata[0])
                    chanId = thisDict.get("chanId")
                    print(gTag + "EncoderClient:CmdCore: (chanId, self.chanId)=", (chanId, self.chanId))
                    sessionId = thisDict.get("sessionId")
                    #apply chanId
                    flag = chanId != None and chanId != self.chanId
                    #flag |= sessionId != None and sessionId != self.sessionId
                    if flag:
                        self.chanId = chanId
                        cmd["chanId"] = self.chanId
                        str_cmd = json2str(cmd)
                        sendDataLen = self.sock.sendto(str_cmd, (self.host, self.port))
                        print(gTag + "EncoderClient:CmdCore: sendDataLen=", sendDataLen)
            count += 1
            if count > 10:
                print(gTag + "EncoderClient:CmdCore: reg fail !", (self.sessionId, self.chanId))
                break
        return acked

    def Register(self):
        cmd = {"cmd": "reg", "actor": self.actor, "sessionId": self.sessionId, "chanId": self.chanId,
               "modeId": self.modeId,
               "selfmode": self.selfmode}
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

    def run(self):
        while self.__running.isSet():
            self.__flag.wait()  #会延缓发送频率
            #print("EchoClientThread run")
            now_time = time.time()
            if self.data_master != None:
                datalist = self.data_master.PopQueueAll()
                if datalist == None or datalist == []:  # or len(datalist) < 15:
                    # 过频繁的访问会导致锁被占用
                    time.sleep(0.001)
                else:
                    for data in datalist:
                        (revcData, remoteHost, remotePort, recv_time) = data
                        if self.dataprocess != None:
                            self.dataprocess.Process(data)
                    if self.start_time == 0:
                        self.start_time = now_time
                difftime = int((now_time - self.start_time) * 1000000)
                if difftime >= self.interval and True:
                    ret = self.netInfo.CountLossRate2()
                    self.start_time = time.time()
            else:
                time.sleep(1)
        print("EchoClientThread run over")
        #self.sock.close()

    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals
        print(gTag + "EchoClientThread: start stop")
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
            print(gTag + "stop error: ", error)
        else:
            print(gTag + "stop ok")

        print(gTag + "EchoClientThread: stop over")

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞


class EchoServerThread(threading.Thread):
    def __init__(self, idx, host, port):
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
        self.actor = 0
        self.sessionId = -1
        if MULT_SERVER:
            self.sessionId = idx
        self.chanId = -1
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
        self.sendthread = None
        #self.sendthread = SendThread(self)
        self.recvthread = None
        self.recvthread = RecvThread(self)
        ###
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True

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
        print(gTag + "EchoServerThread del")

    def run(self):
        #与客户端线程数不对等，导致访问频率减缓
        name = self.getName()
        print("name= ", name)
        while self.__running.isSet():
            self.__flag.wait()   # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            #print("EchoServerThread run")
            time.sleep(1)
        print(gTag + "EchoServerThread: run over")
    def stop(self):
        if self.data_master != None:
            self.data_master.readable.set()
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals
        print(gTag + "EchoServerThread: start stop")
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
            print(gTag + "stop error: ", error)
        else:
            print(gTag + "stop ok")

        print(gTag + "EchoServerThread: stop over")
    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞
def RunServer(idx, speakernum, host, port):
    if True:
        # idx = raw_input('please input to exit(eg: -1 ): ')
        # count = 0
        thread = EchoServerThread(idx, host, port)
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
def RunClient(proc_idx, speakernum, host, port):
    if True:
        # idx = raw_input('please input to exit(eg: -1 ): ')
        # count = 0
        idx = 0
        #clientnum = 1#16#2#1#16 #2#1#2
        threadnum = speakernum * speakernum
        threadlist = [None for x in range(threadnum)]
        for i in range(speakernum):
            for j in range(speakernum):
                #actor_idx = proc_idx * threadnum + i
                k = i * speakernum + j
                if (i | j) == 0:
                    actor = 1
                elif(j == 0):
                    actor = 2
                else:
                    actor = 3
                threadlist[k] = EchoClientThread(proc_idx, i, actor, host, port)
                if threadlist[k].status == False:
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
        #thread.join()
        #time.sleep(5)

def RunSession(actor, proc_idx, speakernum, host, port):
    print("RunSession开始执行,进程号为%d" % (os.getpid()))
    if actor == 0:
        RunServer(proc_idx, speakernum, host, port)
    else:
        RunClient(proc_idx, speakernum, host, port)
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
        RunSession(global_actor, 0, global_speakernum, global_host, global_port)
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
            port = global_port
            if MULT_SERVER:
                port = global_port + i
            po.apply_async(RunSession, (global_actor, i, global_speakernum, global_host, port))
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

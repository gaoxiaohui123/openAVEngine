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
import platform
import loadserver
import struct
import errno
import udpbase as base

CMD_SIZE = 256
DATA_SIZE = 1500
SO_SNDBUF = 1024 * 1024
SO_RCVBUF = 1024 * 1024

SERVER_TYPE = "video"

global_port = 8097 #8888
global_host = 'localhost'
#global_host = '172.16.0.17'
#global_host = '111.230.226.17'
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
class BroadCastThread(threading.Thread):
    def __init__(self, parent, avtype, host, port):
        threading.Thread.__init__(self)
        self.parent = parent
        self.avtype = avtype
        self.actor = base.BCACTOR
        self.host = host
        self.port = port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, SO_SNDBUF)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, SO_RCVBUF)
        self.sessionId = -1
        self.chanId = -1
        self.nettime = 0
        self.rtt = 0
        self.status = False

        if (platform.system() == 'Windows'):
            #val = struct.pack("Q", 5050)
            val = struct.pack("Q", 500) #500ms
            self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVTIMEO, val)
        else:
            #self.sock.settimeout(100)
            #val = struct.pack("QQ", 5, 50000)  # 5.05s
            val = struct.pack("QQ", 0, 500000) #500ms
            self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVTIMEO, val)
        #self.sock.settimeout(1)

        self.lock = threading.Lock()
        self.inforList = []
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True

        cmd = {"actor": self.actor}
        cmd = {"cmd":"broadcast", "actor": self.actor}
        str_cmd = json2str(cmd)
        is_acked = False
        while not is_acked:
            sendDataLen = self.sock.sendto(str_cmd, (self.host, self.port))
            if sendDataLen > 0:
                try:
                    rdata = self.sock.recvfrom(CMD_SIZE)
                # except IOError, error:  # python2
                except IOError as error:  # python3
                    print("BroadCastThread: error= ", error) #Resource temporarily unavailable
                    if error.errno == errno.EWOULDBLOCK:
                        print("BroadCastThread: 1: error.errno= ", error.errno)
                        break
                    elif error.errno == errno.WSAETIMEDOUT:
                        pass
                        print("BroadCastThread: 2: error.errno= ", error.errno)
                        break
                    #elif error.errno == errno.ETIMEDOUT:
                    #    print("BroadCastThread: 3: error.errno= ", error.errno)
                    else:
                        break
                else:
                    is_acked = True
                    self.status = True
            else:
                print("BroadCastThread: sendDataLen=", sendDataLen)
                break
        if self.status:
            (self.nettime, self.rtt) = self.NetTime()
            print("BroadCastThread: is ok, self.nettime=(s) ", self.nettime)
        print("BroadCastThread: is ok, (avtype, is_acked)=", (avtype, is_acked))
    def __del__(self):
        print("BroadCastThread del")
    def NetTime(self):
        n = 3
        j = 0
        sumdelay = 0
        sumrtt = 0
        for i in range(n):
            is_acked = False
            while not is_acked:
                now_time = time.time()
                cmd = {"cmd": "broadcast", "actor": self.actor, "st0":now_time}
                str_cmd = json2str(cmd)
                try:
                    sendDataLen = self.sock.sendto(str_cmd, (self.host, self.port))
                    rdata = self.sock.recvfrom(CMD_SIZE)
                # except IOError, error:  # python2
                except IOError as error:  # python3
                    print("BroadCastThread: NetTime: error= ", error) #Resource temporarily unavailable
                    if error.errno == errno.EWOULDBLOCK:
                        print("BroadCastThread: NetTime: 1: error.errno= ", error.errno)
                        break
                    elif error.errno == errno.WSAETIMEDOUT:
                        pass
                        print("BroadCastThread: NetTime: 2: error.errno= ", error.errno)
                        break
                    #elif error.errno == errno.ETIMEDOUT:
                    #    print("BroadCastThread: 3: error.errno= ", error.errno)
                    else:
                        self.status = False
                        break
                else:
                    if rdata[0] != '' and rdata[1] != None:
                        #print("BroadCastThread:NetTime:  rdata[0]= ", rdata[0])
                        thisDict = str2json(rdata[0])
                        if thisDict != None:
                            st0 = thisDict.get("st0")
                            if st0 != None:
                                rt0 = thisDict.get("rt0")
                                st1 = thisDict.get("st1")
                                rt1 = time.time()
                                rtt = ((rt1 - st0) - (st1 - rt0)) / 2  # ((rt0 - st0) + (rt1 - st1)) / 2 #
                                sumrtt += rtt
                                # delay0 = (st0 + rtt) - rt0
                                # delay1 = (rt1 - rtt) - st1
                                # delay = (delay0 + delay1) / 2
                                delay = ((rt1 + st0) - (st1 + rt0)) / 2
                                sumdelay += delay
                                #print("sumdelay= ", sumdelay)
                                is_acked = True
                                j += 1
        #delay = int((sumdelay * 1000) / n)
        (delay, rtt) = (0, 0)
        if j > 0:
            delay = sumdelay / j
            rtt = sumrtt / j
            iNettime = int(delay * 1000)
            irtt = int(rtt * 1000)
            print("BroadCastThread: NetTime: (iNettime, irtt)= ", (iNettime, irtt))
        #servertime = localtime - delay
        return (delay, rtt)
    def SetSessionId(self, sessionId):
        self.sessionId = sessionId
    def GetChanId(self, sessionId, modeId, actor):
        self.chanId = -1
        max_times = 10
        count = 0
        self.pause()
        #cmd = {"actor": self.actor, "sessionId": sessionId, "chanId": -1, "modeId": modeId}
        cmd = {"cmd":"broadcast", "actor": actor, "sessionId": sessionId, "chanId": -1, "modeId": modeId}
        str_cmd = json2str(cmd)
        is_acked = False
        while not is_acked and (count < max_times) and (self.chanId < 0):
            #count += 1
            sendDataLen = self.sock.sendto(str_cmd, (self.host, self.port))
            if sendDataLen > 0:
                try:
                    rdata = self.sock.recvfrom(CMD_SIZE)
                # except IOError, error:  # python2
                except IOError as error:  # python3
                    #print("BroadCastThread: error= ", error)  # Resource temporarily unavailable
                    if error.errno == errno.EWOULDBLOCK:
                        print("BroadCastThread: GetChanId: error.errno= ", error.errno)
                        #break
                    elif error.errno == errno.WSAETIMEDOUT:
                        pass
                        print("BroadCastThread: GetChanId: error.errno= ", error.errno)
                        #break
                    # elif error.errno == errno.ETIMEDOUT:
                    #    print("BroadCastThread: 3: error.errno= ", error.errno)
                    else:
                        break
                else:
                    #is_acked = True
                    if rdata[0] != '' and rdata[1] != None:
                        #print("BroadCastThread:GetChanId:  rdata[0]= ", rdata[0])
                        thisDict = str2json(rdata[0])
                        if thisDict != None:
                            sessionId2 = thisDict.get("sessionId")
                            modeId2 = thisDict.get("modeId")
                            status2 = thisDict.get("status")
                            chanId2 = thisDict.get("chanId")
                            if chanId2 != None:
                                self.chanId = chanId2
                                is_acked = True
                                count += 1
                            else:
                                #print("BroadCastThread:GetChanId:  rdata[0]= ", rdata[0])
                                pass
            else:
                print("BroadCastThread:GetChanId: sendDataLen=", sendDataLen)
                break
            print("BroadCastThread:GetChanId:  count= ", count)
        print("BroadCastThread:GetChanId: is ok, (avtype, is_acked, self.chanId)=", (self.avtype, is_acked, self.chanId))
        self.resume()
        return self.chanId
    def SayBy(self, value):
        if self.__running.isSet():
            cmd = {"actor":self.actor ,"sessionId": self.sessionId, "status":0}
            cmd = {"cmd":"broadcast", "actor": self.actor, "sessionId": self.sessionId, "status": value}
            str_cmd = json2str(cmd)
            sendDataLen = self.sock.sendto(str_cmd, (self.host, self.port))
    def PushQueue(self, sessionId, modeId, note, status):
        #print("BroadCastThread: PushQueue: ", (sessionId, modeId, note, status))
        self.lock.acquire()
        if status:
            if (sessionId, modeId, note) not in self.inforList:
                self.inforList.append((sessionId, modeId, note))
                if self.parent != None:
                    self.parent.RenewSessionInfo(self.avtype, sessionId, modeId, note, 1)
        else:
            n = len(self.inforList)
            for i in range(n):
                if i >= len(self.inforList):
                    break
                (sessionId2, modeId2, note2) = self.inforList[i]
                if sessionId2 == sessionId:
                    if self.parent != None:
                        self.parent.RenewSessionInfo(self.avtype, sessionId, modeId, note, 0)
                    del self.inforList[i]
                    i -= 1
        self.lock.release()
    def GetSessionInfo(self):
        ret = []
        self.lock.acquire()
        ret = self.inforList
        self.lock.release()
        return ret
    def PopQueue(self):
        ret = None
        self.lock.acquire()
        if len(self.inforList) > 0:
            ret = self.inforList[0]
            del (self.inforList[0])
        self.lock.release()
        return ret
    def run(self):
        while self.__running.isSet():
            self.__flag.wait()   # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            try:
                rdata = self.sock.recvfrom(CMD_SIZE)
            # except IOError, error:  # python2
            except IOError as error:  # python3
                #print("BroadCastThread: run: error= ", error)  # Resource temporarily unavailable
                if error.errno == errno.EWOULDBLOCK:
                    #print("BroadCastThread: run: error.errno= ", error.errno)
                    pass
                elif error.errno == errno.WSAETIMEDOUT:
                    pass
                # elif error.errno == errno.ETIMEDOUT:
                #    print("BroadCastThread: 2: error.errno= ", error.errno)
                else:
                    print("BroadCastThread: run: error= ", error)
                    break
            else:
                if rdata[0] != '' and rdata[1] != None:
                    #print("BroadCastThread: rdata= ", rdata)
                    thisDict = str2json(rdata[0])
                    if thisDict != None:
                        sessionId = thisDict.get("sessionId")
                        modeId = thisDict.get("modeId")
                        note = thisDict.get("note")
                        status = thisDict.get("status")
                        chanId = thisDict.get("chanId")
                        if chanId != None:
                            self.chanId = chanId
                        else:
                            self.PushQueue(sessionId, modeId, note, status)
                else:
                    self.stop()
    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals
        try:
            self.sock.shutdown(socket.SHUT_RDWR)
            self.sock.close()
        #except IOError, err:
        except IOError as error:  # python3
            print("BroadCastThread: stop error: ", error)
        else:
            print("BroadCastThread: stop ok")

        print("BroadCastThread: stop")

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞


if __name__ == "__main__":
    print("start broadcast")
    bc = BroadCastThread(None, 1, global_host, global_port)
    if bc.status:
        bc.setDaemon(True)
        bc.start()
        time.sleep(2)
        inforList = bc.GetSessionInfo()
        for thisinfor in inforList:
            if thisinfor != None:
                (sessionId, modeId, note) = thisinfor
                print("(sessionId, modeId, note)=", (sessionId, modeId, note))
                chanId = bc.GetChanId(sessionId, modeId)
                print("chanId=", chanId)
    print("end broadcast")
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
import struct
import platform
import copy
import loadserver


CMD_SIZE = 256
DATA_SIZE = 1500
SO_SNDBUF = 1024 * 1024
SO_RCVBUF = 1024 * 1024
LongMaxBufLen = 30 * 1024 #每帧最大1024个包，最多保存30帧
ShortMaxBufLen = 100 #4 * 1024 #每帧最大1024个包，最大延时4帧间隔

gTag = "video:"
gFreq = 90

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

#异常退出下的僵尸信息尚未处理，noted_20210108
class BroadCastThread(threading.Thread):
    def __init__(self, server):
        threading.Thread.__init__(self)
        self.server = server
        self.lock = threading.Lock()
        self.clientList = []
        #self.inforList = []
        self.inforQueue = {}
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
    def __del__(self):
        print(gTag + "BroadCastThread del")

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
        self.lock.acquire()
        infor = self.inforQueue.get(sessionId)
        if infor != None:
            self.inforQueue.pop(sessionId)
        self.lock.release()
    def PushQueue(self, sockId, sessionId, modeId, status):
        self.lock.acquire()
        infor = self.inforQueue.get(sessionId)
        if infor == None:
            self.inforQueue.update({sessionId:(sockId, modeId, status)})
        else:
            self.inforQueue[sessionId] = (sockId, modeId, status)
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
            self.lock.acquire()
            if len(self.clientList) > 0:
                #sessionInfo = self.PopQueue()
                keyList = list(self.inforQueue.keys())
                n = len(keyList)
                for i in range(n):
                    sessionId = keyList[i]
                    if sessionId in self.inforQueue.keys():
                        value = self.inforQueue[sessionId]
                        (sockId0, modeId, status) = value
                        # 通知所有的socket
                        for sockId in self.clientList:
                            self.lock.release()
                            remoteHost = sockId.split("_")[0]
                            remotePort = int(sockId.split("_")[1])
                            jsonobj = {"sessionId": sessionId, "modeId": modeId, "note": "test", "status": status}
                            json_str = json2str(jsonobj)
                            sendDataLen = self.server.sock.sendto(json_str, (remoteHost, remotePort))
                            self.lock.acquire()
                        if status == 0:
                            # 删除该信息
                            self.lock.release()
                            self.DeleteSession(sessionId)
                            self.lock.acquire()
                self.lock.release()
                time.sleep(1)
                self.lock.acquire()
            else:
                self.lock.release()
                time.sleep(0.01)
                self.lock.acquire()
            self.lock.release()
        print(gTag + "BroadCastThread: run over")
    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals

        print(gTag + "BroadCastThread: stop")

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞
class ClientManager(object):
    def __init__(self, server):
        self.server = server
        self.lock = threading.Lock()
        self.clientQueue = {}
        self.dataQueue = {}
        self.broadCast = BroadCastThread(server)
        self.broadCast.start()
        #优化：采用时间戳替代固定长度,但需要重排序
        #积压包过多：导致瞬间带宽峰值；导致解码端缓存溢出
        self.LongMaxBufLen = LongMaxBufLen
        self.ShortMaxBufLen = ShortMaxBufLen
        self.longTime = 3000 #3s
        self.shortTime = 400 #100ms:服务器透传最大延时容限

    def __del__(self):
        print(gTag + "ClientManager del")
        self.broadCast.stop()
    def stop(self):
        self.broadCast.stop()

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
                print(gTag + "ReadCmd: not cmd")
                # print("not cmd: str_cmd=", str_cmd)
            else:
                pass
        else:
            pass
        return cmd
    def DeleteClient(self, sessionId):
        print(gTag + "ClientManager: DeleteClient: sessionId= ", sessionId)
        #self.broadCast.DeleteSession(sessionId)
        self.lock.acquire()
        keyList = list(self.clientQueue.keys())
        print(gTag + "ClientManager: DeleteClient: keyList= ", keyList)
        n = len(keyList)
        for i in range(n):
            this_sessionId = keyList[i]
            if this_sessionId == sessionId:
                del self.clientQueue[this_sessionId]
                self.broadCast.DeleteClient(this_sessionId)
        self.lock.release()
    #def PopQueue(self, sockId):
    #    self.lock.acquire()
    #    if self.clientQueue.get(sockId) != None:
    #        pop_obj = self.clientQueue.pop(sockId)
    #        print(gTag + "ClientManager: PopQueue: pop_obj= ", pop_obj)
    #    self.lock.release()

    def PushQueue(self, data, remoteHost, remotePort, recv_time):
        ret = -1
        (cmdtype, actor, sessionId, chanId, modeId, selfmode, status) = (None, -1, -1, -1, -1, -1, -1)
        #print("video:ClientManager: PushQueue: len(data)= ", len(data))
        n = len(data)
        #print(gTag + "ClientManager: PushQueue: (data)= ", (data))
        cmd = None
        if n <= CMD_SIZE and ('{' in str(data[0:3])) and ('}' in str(data[n - 3:])):
            cmd = self.ReadCmd(data)
            if cmd not in [{}, None]:
                #print("PushQueue: data[0:3]=", data[0:3])
                #print("PushQueue: data=", data)
                cmdtype = cmd.get("cmd")
                if cmdtype != None:
                    if cmdtype == "heartbeat":
                        return ret
                if cmd.get("actor") != None:
                    actor = cmd.get("actor")
                if cmd.get("sessionId") != None:
                    sessionId = cmd.get("sessionId")
                if cmd.get("chanId") != None:
                    chanId = cmd.get("chanId")
                if cmd.get("selfmode") != None:
                    selfmode = cmd.get("selfmode")
                if cmd.get("modeId") != None:
                    modeId = cmd.get("modeId")
                if cmd.get("status") != None:
                    status = cmd.get("status")
            else:
                pass
        sockId = remoteHost + "_" + str(remotePort)

        if actor >= 0: #self.clientQueue.get(sockId) == None and
            print(gTag + "ClientManager: PushQueue: actor= ", actor)
            #head_dict = {"sockId":sockId, "chanId":chanId, "modeId":modeId, "selfmode":selfmode}
            chanIdList = None
            if actor == 1:
                jsonobj = {"chanId": chanId, "modeId": modeId}
                json_str = json2str(jsonobj)
                sendDataLen = self.server.sock.sendto(json_str, (remoteHost, remotePort))  # ack
                chanIdMap = [1, 2, 2, 2, 2, 2, 2, 3, 4, 4, 5, 5, 5, 5, 8, 8, 8, 8, 9, 16, 32, 64]
                if modeId >= 0 and modeId < len(chanIdMap):
                    chanIdList = [-1 for x in range(chanIdMap[modeId])]
                    chanIdList[chanId] = chanId
                    #head_dict.update({"chanIdList":chanIdList})
                    self.broadCast.PushQueue(sockId, sessionId, modeId, 1)
                self.lock.acquire()
                if self.clientQueue.get(sessionId) == None:
                    self.clientQueue.update({sessionId: {actor: {sockId: [chanId,recv_time]}}})
                    self.clientQueue[sessionId].update({"chanIdList": chanIdList})
                    self.clientQueue[sessionId].update({"modeId": modeId})
                    self.clientQueue[sessionId].update({"selfmode": selfmode})
                else:
                    if self.clientQueue[sessionId].get(actor) == None:
                        self.clientQueue[sessionId].update({actor: {sockId: [chanId,recv_time]}})
                        self.clientQueue[sessionId].update({"chanIdList": chanIdList})
                        self.clientQueue[sessionId].update({"modeId": modeId})
                        self.clientQueue[sessionId].update({"selfmode": selfmode})
                    else:
                        chanIdList = self.clientQueue[sessionId].get("chanIdList")
                        print(gTag + "PushQueue: chanIdList=", chanIdList)
                        chanIdList[chanId] = chanId
                        self.clientQueue[sessionId][actor].update({sockId: [chanId,recv_time]})
                        self.clientQueue[sessionId].update({"chanIdList": chanIdList})
                        self.clientQueue[sessionId].update({"modeId": modeId})
                        self.clientQueue[sessionId].update({"selfmode": selfmode})
                self.lock.release()
                print(gTag + "ClientManager: PushQueue: self.clientQueue[sessionId]= ", self.clientQueue[sessionId])
                #print(gTag + "PushQueue: head_dict=", head_dict)
            elif actor == 100:
                if sessionId < 0:
                    #注册broadcast,并获取sessionId
                    jsonobj = {"actor": actor}
                    json_str = json2str(jsonobj)
                    sendDataLen = self.server.sock.sendto(json_str, (remoteHost, remotePort)) #ack
                    print(gTag + "PushQueue: (remoteHost, remotePort)=", (remoteHost, remotePort))
                    self.broadCast.PushClient(sockId)
                else:
                    if chanId < 0 and status < 0:
                        #获取chanId
                        self.lock.acquire()
                        chanIdList = self.clientQueue[sessionId].get("chanIdList")
                        modeId = self.clientQueue[sessionId].get("modeId")
                        self.lock.release()
                        if chanIdList != None:
                            hit_chanId = False
                            for i in range(len(chanIdList)):
                                thisChanId = chanIdList[i]
                                if thisChanId == -1:
                                    jsonobj = {"sessionId": sessionId, "chanId": i, "modeId": modeId}
                                    json_str = json2str(jsonobj)
                                    sendDataLen = self.server.sock.sendto(json_str, (remoteHost, remotePort))  # ack
                                    chanIdList[i] = -2  # 在确认中
                                    self.lock.acquire()
                                    if self.clientQueue.get(sessionId) != None:
                                        self.clientQueue[sessionId]["chanIdList"] = chanIdList
                                    self.lock.release()
                                    hit_chanId = True
                                    print(gTag + "ClientManager: PushQueue: i= ", i)
                                    break
                                else:
                                    print(gTag + "warning: ClientManager: PushQueue: thisChanId= ", thisChanId)
                            if not hit_chanId:
                                print(gTag + "warning: ClientManager: PushQueue: hit_chanId= ", hit_chanId)

                        pass
                    elif status == 0:
                        print(gTag + "PushQueue: cmd=", cmd)
                        self.broadCast.PushQueue(sockId, sessionId, modeId, 0)
                        self.DeleteClient(sessionId)
                        pass
                    else:
                        pass
            elif actor == 2:
                jsonobj = {"chanId": chanId, "modeId": modeId}
                json_str = json2str(jsonobj)
                sendDataLen = self.server.sock.sendto(json_str, (remoteHost, remotePort))  # ack
                self.lock.acquire()
                chanIdList = self.clientQueue[sessionId].get("chanIdList")
                modeId = self.clientQueue[sessionId].get("modeId")
                self.lock.release()
                if chanIdList != None:
                    if True:
                        if chanId in chanIdList:
                            pass
                        else:
                            chanIdList[chanId] = chanId
                            self.lock.acquire()
                            if self.clientQueue.get(sessionId) != None:
                                if self.clientQueue[sessionId].get(actor) == None:
                                    self.clientQueue[sessionId].update({actor: {sockId: [chanId,recv_time]}})
                                else:
                                    self.clientQueue[sessionId][actor].update({sockId: [chanId,recv_time]})
                                self.clientQueue[sessionId]["chanIdList"] = chanIdList
                            print(gTag + "ClientManager: PushQueue: chanIdList= ", chanIdList)
                            self.lock.release()
                print(gTag + "ClientManager: PushQueue: 2: self.clientQueue[sessionId]= ", self.clientQueue[sessionId])
            else:
                jsonobj = {"chanId": chanId, "modeId": modeId}
                json_str = json2str(jsonobj)
                sendDataLen = self.server.sock.sendto(json_str, (remoteHost, remotePort))  # ack
                self.lock.acquire()
                if self.clientQueue.get(sessionId) != None:
                    if self.clientQueue[sessionId].get(actor) == None:
                        self.clientQueue[sessionId].update({actor: {sockId: [chanId,recv_time]}})
                    else:
                        self.clientQueue[sessionId][actor].update({sockId: [chanId,recv_time]})
                self.lock.release()
            #
            ret = 0
        elif actor < 0:
            #数据转发
            #print(gTag + "ClientManager: PushQueue: data")
            #print(gTag + "ClientManager: PushQueue: actor=", actor)
            self.lock.acquire()
            head_dict = self.dataQueue.get(sockId)
            if head_dict == None:
                self.dataQueue.update({sockId:{"longdata" : [(data, recv_time)]}})
                self.dataQueue[sockId].update({"data" : [(data, recv_time)]})
            else:
                #print(gTag + "ClientManager: PushQueue: actor=", actor)
                #缓存用于重传
                if head_dict.get("longdata") == None:
                    self.dataQueue[sockId].update({"longdata" : [(data, recv_time)]})
                else:
                    datalist = head_dict.get("longdata")
                    if len(datalist) > 0:
                        (data0, recv_time0) = datalist[0]
                        difftime = int(recv_time - recv_time0)
                        # if len(datalist) > self.LongMaxBufLen:
                        if difftime > self.longTime:
                            del datalist[0]
                    datalist.append((data, recv_time))
                    self.dataQueue[sockId]["longdata"] = datalist
                #用于透传
                if head_dict.get("data") == None:
                    self.dataQueue[sockId].update({"data" : [(data, recv_time)]})
                else:
                    datalist = head_dict.get("data")
                    if len(datalist) > 0:
                        (data0, recv_time0) = datalist[0]
                        difftime = int(recv_time - recv_time0)
                        # if len(datalist) > self.ShortMaxBufLen:
                        if difftime > self.shortTime:
                            del datalist[0]
                            print(gTag + "ClientManager: PushQueue: (len(datalist), difftime)= ", (len(datalist), difftime))
                    datalist.append((data, recv_time))
                    self.dataQueue[sockId]["data"] = datalist
                ret = 0
            self.lock.release()
        self.server.send_task.flag.set()
        return ret
    def Transport(self):
        ret = False
        #print(gTag + "ClientManager: Transport: self.clientQueu= ", self.clientQueue)
        self.lock.acquire()
        keyList = list(self.clientQueue.keys())
        n = len(keyList)
        for i in range(n):
            sessionId = keyList[i]
            head_dict = copy.deepcopy(self.clientQueue.get(sessionId))
            self.lock.release()
            if head_dict != None:
                selfmode = head_dict["selfmode"]
                #print(gTag + "ClientManager: Transport: head_dict= ", head_dict)
                # self.clientQueue.update({sessionId: {actor: {sockId: chanId}}})
                actor = 3
                targetPairs = head_dict.get(actor)
                #print(gTag + "ClientManager: Transport: targetPairs= ", targetPairs)
                if targetPairs != None:
                    actors = [1, 2]
                    for actor in actors:
                        #print(gTag + "ClientManager: Transport: head_dict= ", head_dict)
                        pairs = head_dict.get(actor)
                        #print(gTag + "ClientManager: Transport: pairs= ", pairs)
                        if pairs != None:
                            for sockId, [chanId, base_time] in pairs.items():
                                self.lock.acquire()
                                dataDict = self.dataQueue.get(sockId)
                                self.lock.release()
                                if dataDict != None:
                                    datalist = dataDict.get("data")
                                    if datalist != None:
                                        m = len(datalist)
                                        used = False
                                        #print(gTag + "ClientManager: Transport: len(datalist)= ", m)
                                        for (data, recv_time) in datalist:
                                            for sockId2, [chanId2, base_time2] in targetPairs.items():
                                                #print(gTag + "ClientManager: Transport: (chanId, chanId2)= ",(chanId, chanId2))
                                                host_port = sockId2.split('_')
                                                (remoteHost, remotePort) = (host_port[0], int(host_port[1]))
                                                flag = False
                                                if selfmode:
                                                    flag = True
                                                else:
                                                    if chanId == chanId2:
                                                        flag = True
                                                if flag:
                                                    ret = True
                                                    used = True
                                                    if recv_time >= base_time2:
                                                        try:
                                                            sendDataLen = self.server.sock.sendto(data,(remoteHost,remotePort))
                                                        # except IOError, error:  # python2
                                                        except IOError as error:  # python3
                                                            print("(error, sendDataLen,remoteHost, remotePort): ",
                                                                  (error, sendDataLen, remoteHost, remotePort))
                                                        else:
                                                            if len(targetPairs) <= 1:
                                                                time.sleep(0.0001)
                                                                # time.sleep(0.001)
                                                            pass
                                                    else:
                                                        #数据收到时间早于用户注册时间
                                                        print(gTag + "ClientManager:Transport: recv_time= ", recv_time)
                                                        print(gTag + "ClientManager:Transport: base_time2= ", base_time2)
                                        if used:
                                            #print(gTag + "ClientManager: m: Transport: len(datalist)= ", m)
                                            self.lock.acquire()
                                            for j in range(m):
                                                del self.dataQueue[sockId]["data"][0]
                                            self.lock.release()
            self.lock.acquire()
        self.lock.release()
        return ret

class DataManager(object):
    def __init__(self):
        self.lock = threading.Lock()
        self.DataList = []
    def __del__(self):
        print(gTag + "DataManager del")
    def PopQueue(self):
        ret = None
        self.lock.acquire()
        if len(self.DataList) > 0:
            ret = self.DataList[0]
            del(self.DataList[0])
        self.lock.release()
        return ret

    def PushQueue(self, revcData, remoteHost, remotePort, recvTime):
        self.lock.acquire()
        self.DataList.append((revcData, remoteHost, remotePort, recvTime))
        #print("DataManager:PushQueue: len(self.DataList)= ", len(self.DataList))
        self.lock.release()

class UdpServer(object):
    def tcpServer(self):
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 1024 * 1024)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 1024 * 1024)
        sock.bind(('', 9527))  # 绑定同一个域名下的所有机器
        while True:
            revcData, (remoteHost, remotePort) = sock.recvfrom(DATA_SIZE)
            print("[%s:%s] connect" % (remoteHost, remotePort))# 接收客户端的ip, port
            sendDataLen = sock.sendto("this is send  data from server", (remoteHost, remotePort))
            print("revcData: ", revcData)
            print("sendDataLen: ", sendDataLen)
        sock.close()

class RecvTaskManagerThread(threading.Thread):
    def __init__(self, server):
        threading.Thread.__init__(self)
        self.server = server
        self.handle_size = 8
        self.handle = create_string_buffer(self.handle_size)

        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
    def __del__(self):
        print(gTag + "RecvTaskManagerThread del")
    def run(self):
        count = 0
        while self.__running.isSet():
            self.__flag.wait()   # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            data = self.server.data_master.PopQueue()
            if data != None:
                #print("video:RecvTaskManagerThread: run 0")
                (revcData, remoteHost, remotePort, recv_time) = data
                #loss_rate = self.server.load.lib.api_count_loss_rate(revcData, len(revcData), gFreq)
                loss_rate = self.server.load.lib.api_count_loss_rate3(self.handle, revcData, len(revcData), gFreq)
                if loss_rate > 1:
                    print(gTag + "RecvTaskManagerThread: loss_rate= ", (loss_rate - 1))
                    pass
                #print("video:RecvTaskManagerThread: run 1")
                #print("TaskManagerThread: recv_time= ", recv_time)
                #start_time = time.time()
                self.server.client_master.PushQueue(revcData, remoteHost, remotePort, recv_time)
                #print("video:RecvTaskManagerThread: run 2")
                #difftime = time.time() - start_time
                #print('{} client_master.PushQueue time: {:.3f}ms'.format(time.ctime(), difftime * 1000))
            else:
                time.sleep(0.001) #20ms
            #print("video:RecvTaskManagerThread: run alive: ", count)
            count += 1
        print(gTag + "RecvTaskManagerThread: run over")
    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals
        #try:
        #    self.sock.close()
        #except:
        #    print("stop error")
        #else:
        #    print
        print(gTag + "RecvTaskManagerThread: stop")

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞

class SendTaskManagerThread(threading.Thread):
    def __init__(self, server):
        threading.Thread.__init__(self)
        self.server = server

        self.max_delay_time = 0
        self.max_delay_packet = -1
        self.ack_times = 0

        array_type = c_char_p * 4
        self.outparam = array_type()
        self.infobuf = create_string_buffer(1024)
        handle_size = 16
        self.ll_handle = create_string_buffer(handle_size)
        # self.ll_handle = (c_char * handle_size)()
        array_type = c_char_p * 1
        self.ctime = array_type()
        self.AckQueue = {}
        self.flag = threading.Event()  # 用于暂停线程的标识
        self.flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
    def __del__(self):
        print(gTag + "SendTaskManagerThread del")

    def send_ack(self, task_data, jsonobj):
        (remoteHost, remotePort, target_sessionId, chanId, selfmode, data) = task_data
        json_str = json2str(jsonobj)
        sendDataLen = self.server.sock.sendto(json_str, (remoteHost, remotePort))
        if (self.ack_times % 10000) == 0:
            print(gTag + "send_ack: (sendDataLen, self.ack_times): ", (sendDataLen, self.ack_times))
        ret = len(json_str)
        self.ack_times += 1
        return ret
    def PushQueue(self, task_data):
        (remoteHost, remotePort, target_sessionId, chanId, selfmode, data) = task_data
        sockId = remoteHost + "_" + str(remotePort)
        (this_data, recv_time) = data
        #print("SendTaskManagerThread: PushQueue: start")
        self.outparam[0] = c_char_p(self.infobuf.raw)
        ret = self.server.load.lib.api_get_extern_info(this_data, len(this_data), self.outparam)
        #print("SendTaskManagerThread: PushQueue: ret= ", ret)
        if ret > 0:
            #print("video:SendTaskManagerThread: PushQueue: self.outparam[0]= ", self.outparam[0])
            outjson = str2json(self.outparam[0])
            if outjson != None:
                #time_status = outjson["time_status"]
                #print("SendTaskManagerThread: PushQueue: time_status= ", time_status)
                #if time_status == 0:
                if True:
                    #self.ctime[0] = "0123456789012345"
                    itime = self.server.load.lib.api_get_time2(self.ll_handle, self.ctime)
                    c_time_ll = char2long(self.ctime[0])
                    #diff_time = self.server.load.lib.api_get_pkt_delay(this_data, len(this_data))
                    if c_time_ll == 0:
                        print("itime= ", itime)
                        return ret
                    seqnum = outjson["seqnum"]
                    if sys.version_info >= (3, 0):
                        packet_time_stamp = int(outjson["packet_time_stamp"])
                    else:
                        packet_time_stamp = long(outjson["packet_time_stamp"])
                    #self.outparam[1] = "api_get_time_stamp2"
                    #self.server.load.lib.api_get_time_stamp2(12, this_data, self.outparam)
                    #c_time_ll = long(self.outparam[0])
                    rttlist = self.AckQueue.get(sockId)
                    #print("SendTaskManagerThread: PushQueue: rttlist= ", rttlist)
                    if rttlist == None:
                        rttlist = [(seqnum, packet_time_stamp, recv_time)]
                        self.AckQueue.update({sockId: rttlist})
                    else:
                        if len(rttlist) > 2:
                            jsonobj = {"rtt" : {"rttlist" : rttlist, "st1" : c_time_ll}}
                            ret = self.send_ack(task_data, jsonobj)
                            rttlist = [(seqnum, packet_time_stamp, recv_time)]
                            self.AckQueue[sockId] = rttlist
                        else:
                            rttlist.append((seqnum, packet_time_stamp, recv_time))
                            self.AckQueue[sockId] = rttlist
            else:
                print(gTag + "PushQueue: not json: ", gTag)
        return ret

    def send_ack_0(self, task_data):
        (remoteHost, remotePort, target_sessionId, data) = task_data
        (this_data, recv_time) = data

        ret = self.server.load.lib.api_get_extern_info(this_data, len(this_data), self.outparam)
        if ret > 0:
            print("send_ack: ret= ", ret)
            outjson = str2json(self.outparam[0])
            if outjson != None:
                #time_status = outjson["time_status"]
                #if time_status == 0:
                if True:
                    seqnum = outjson["seqnum"]
                    if sys.version_info >= (3, 0):
                        packet_time_stamp = int(outjson["packet_time_stamp"])
                    else:
                        packet_time_stamp = long(outjson["packet_time_stamp"])
                    self.outparam[1] = "api_get_time_stamp2"
                    self.server.load.lib.api_get_time_stamp2(12, this_data, self.outparam)
                    if sys.version_info >= (3, 0):
                        c_time_ll = int(self.outparam[0])
                    else:
                        c_time_ll = long(self.outparam[0])
                    rtt = {}
                    rtt.update({"seqnum": seqnum})
                    rtt.update({"st0": packet_time_stamp})
                    rtt.update({"rt0": recv_time})
                    rtt.update({"st1": c_time_ll})
                    jsonobj = {"rtt": rtt}
                    json_str = json2str(jsonobj)
                    sendDataLen = self.server.sock.sendto(json_str, (remoteHost, remotePort))
                    print("send_ack: sendDataLen: ", sendDataLen)
                    ret = len(json_str)

        return ret

    def run_0(self):
        while self.__running.isSet():
            self.flag.wait()   # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            (actor, sessionId) = (1, -1)
            start_time = time.time()

            task_data_list = self.server.client_master.GetClient([1, 2], sessionId)
            has_send = False
            #print(gTag + "SendTaskManagerThread: (actor, sessionId)= ", (actor, sessionId))
            #if len(task_data_list) > 0:
            #    print("video: SendTaskManagerThread: len(task_data_list)= ", len(task_data_list))
            for task_data in task_data_list:
            #if task_data != None:
                (remoteHost, remotePort, target_sessionId, chanId, selfmode, data) = task_data
                #print(gTag + "SendTaskManagerThread: target_sessionId= ", target_sessionId)
                if target_sessionId >= 0:
                    self.PushQueue(task_data)
                    task_data_list2 = self.server.client_master.GetClient([3], target_sessionId)
                    for task_data2 in task_data_list2:
                        (remoteHost2, remotePort2, target_sessionId2, chanId2, selfmode2, data2) = task_data2
                        if selfmode > 0:
                            match_client = target_sessionId2 == target_sessionId
                        else:
                            match_client = (chanId == chanId2) and (target_sessionId2 == target_sessionId)
                        #print(gTag + "SendTaskManagerThread: target_sessionId= ", target_sessionId)
                        #print(gTag + "SendTaskManagerThread: target_sessionId2= ", target_sessionId2)
                        #print(gTag + "SendTaskManagerThread: (chanId, chanId2)= ", (chanId, chanId2))
                        if match_client:
                            (this_data, recv_time) = data
                            #print(gTag + "SendTaskManagerThread: len(this_data)= ", len(this_data))
                            if True:
                                #self.ctime[0] = "0123456789012345"
                                #self.server.load.lib.api_get_time(self.ctime)
                                #c_time_ll = char2long(self.ctime[0])
                                diff_time = self.server.load.lib.api_get_pkt_delay(this_data, len(this_data))
                                #self.outparam[1] = "api_get_time_stamp2"
                                #self.server.load.lib.api_get_time_stamp2(14, this_data, self.outparam)
                                #c_time_ll = long(self.outparam[0])
                                #if self.outparam[1] != "api_get_time_stamp2":
                                #    c_time_ll2 = long(self.outparam[1])
                                #    diff_time = int(c_time_ll - c_time_ll2)

                                # if diff_time > self.max_delay_time and self.recv_packet_num > 100:
                                if diff_time > self.max_delay_time:
                                    self.max_delay_time = diff_time
                                    print("video: send2send diff_time= ", diff_time)
                            try:
                                sendDataLen = self.server.sock.sendto(this_data, (remoteHost2, remotePort2))
                                #print(gTag + "(sendDataLen,remoteHost2, remotePort2): ", (sendDataLen, remoteHost2, remotePort2))
                            #except IOError, error:  # python2
                            except IOError as error:  # python3
                                print("(error, sendDataLen,remoteHost2, remotePort2): ", (error, sendDataLen, remoteHost2, remotePort2))
                            else:
                                #time.sleep(0.0001)
                                has_send = True
                                pass
            if has_send == False:
                time.sleep(0.001) #1ms
                #self.flag.clear()
            difftime = int((time.time() - start_time) * 1000)
            if difftime > 100:
                print('SendTaskManagerThread time: difftime= ', difftime)
        print(gTag + "SendTaskManagerThread: run over")

    def run(self):
        while self.__running.isSet():
            self.flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            (actor, sessionId) = (1, -1)
            start_time = time.time()
            has_send = self.server.client_master.Transport()
            if has_send == False:
                time.sleep(0.001)  # 1ms
                # self.flag.clear()
            difftime = int((time.time() - start_time) * 1000)
            if difftime > 100:
                print('SendTaskManagerThread time: difftime= ', difftime)
        print(gTag + "SendTaskManagerThread: run over")
    def stop(self):
        self.flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals
        if self.server.log_fp != None:
            self.server.log_fp.write("send2send: max_delay_time= " + str(self.max_delay_time) + "\n")
            self.server.log_fp.flush()
        print(gTag + "SendTaskManagerThread: stop")

    def pause(self):
        self.flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.flag.set()  # 设置为True, 让线程停止阻塞


class EchoServerThread(threading.Thread):
    def __init__(self, actor, host, port):
        threading.Thread.__init__(self)
        ###
        self.load = loadserver.gload #LoadLib()
        if self.load == None:
            self.status = False
            return
        array_type = c_char_p * 4
        self.outparam = array_type()

        self.infobuf = create_string_buffer(1024)

        self.max_delay_time = 0
        self.max_delay_packet = -1
        self.log_fp = None
        self.filelist0 = []
        self.filelist1 = []
        if False:
            try:
                self.log_fp = open("../../mytest/server_log.txt", "w")
            except:
                try:
                    self.log_fp = open("./server_log.txt", "w")
                except:
                    self.log_fp = None

        array_type = c_char_p * 4
        self.outparam = array_type()

        handle_size = 16
        self.ll_handle = create_string_buffer(handle_size)
        # self.ll_handle = (c_char * handle_size)()
        array_type = c_char_p * 1
        self.ctime = array_type()
        ###
        self.actor = actor
        self.host = host
        self.port = port
        #self.status = True
        self.loss_rate = 0.0# 0.05 #0 #0.1 #10%
        self.net_start_time = 0 #time.time()
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
        self.sock.bind((self.host, self.port))  # 绑定同一个域名下的所有机器
        if False:
            self.sock.setblocking(False)
            self.sock.settimeout(10)
        print(gTag + "EchoServerThread: self.host= ", self.host)
        print(gTag + "EchoServerThread: self.port= ", self.port)
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
        self.recv_packet_num = 0
        self.client_master = ClientManager(self)
        self.data_master = DataManager()
        self.recv_task = RecvTaskManagerThread(self)
        self.recv_task.start()
        self.send_task = SendTaskManagerThread(self)
        self.send_task.start()
    def __del__(self):
        print(gTag + "EchoServerThread del")

    def rand_lost_packet(self, rate):
        ret = True
        start_time = time.time()
        #np.random.seed(0)
        #p = np.array([rate, 0.1, 0.7, 0.2])
        rate2 = 0.8 - rate
        #p = [0.1, 0.0, 0.7, 0.2]
        p = [rate, 0.1, rate2, 0.1]
        a = [0, 1, 2, 3]
        #index = np.random.choice([0, 1, 2, 3], p=p.ravel())
        #index = np.random.choice(np.array([0, 1, 2, 3]), 1, p)
        index = np.random.choice(a=a, size=1, replace=True, p=p)
        if 0 in index:
            ret = False
            #print("####################################################rand_lost_packet: ret= ", ret)
        #print("rand_lost_packet: index= ", index)
        difftime = time.time() - start_time
        #print('{} rand_lost_packet: time: {:.3f}ms'.format(time.ctime(), difftime * 1000))
        return ret
    def save_info(self, data, remoteHost, remotePort):
        self.outparam[0] = c_char_p(self.infobuf.raw)
        ret = self.load.lib.api_get_extern_info(data, len(data), self.outparam)
        if ret > 0:
            outjson = str2json(self.outparam[0])
            if outjson != None:
                seqnum = outjson["seqnum"]
                sockId = remoteHost + "_" + str(remotePort)
                #print("EchoServerThread: save_info: (sockId, seqnum)= ", (sockId, seqnum))
                if sockId not in self.filelist0:
                    self.filelist0.append(sockId)
                    try:
                        fp = open("./log/server_recv_" + sockId + ".txt", "w")
                    except:
                        pass
                    if fp != None:
                        self.filelist1.append(fp)

                index = self.filelist0.index(sockId)
                if len(self.filelist1) > 0:
                    fp = self.filelist1[index]
                    if fp != None:
                        fp.write(str(seqnum) + " \n")
                        fp.flush()
                        #fp.close()
            else:
                print(gTag + "save_info: not json", gTag)
        return ret
    def run(self):
        time_factor = 10
        time_step = 10  # s
        while self.__running.isSet():
            self.__flag.wait()   # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            recvData = None
            try:
                #recvData, (remoteHost, remotePort) = self.sock.recvfrom(DATA_SIZE)
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
                self.recv_packet_num += 1
                #ret = self.save_info(recvData, remoteHost, remotePort)
                #print(gTag + "EchoServerThread: run: recvfrom: len(revcData)= ", len(recvData))
                if (self.recv_packet_num % 10000) == 0:
                    print(gTag + "EchoServerThread: run: recvfrom: self.recv_packet_num", self.recv_packet_num)

                #self.ctime[0] = "0123456789012345"
                itime = self.load.lib.api_get_time2(self.ll_handle, self.ctime)
                c_time_ll = char2long(self.ctime[0])
                if c_time_ll == 0:
                    print("itime= ", itime)
                diff_time = self.load.lib.api_get_pkt_delay(recvData, len(recvData))

                if diff_time > self.max_delay_time:
                    self.max_delay_time = diff_time
                    self.max_delay_packet = self.recv_packet_num
                    print(gTag + "EchoServerThread: send2receive diff_time= ", diff_time)
                    #if self.log_fp != None:
                    #    self.log_fp.write("max_delay_time= " + str(self.max_delay_time) + "\n")
                    #    self.log_fp.flush()
                if abs(diff_time) > 1:
                    pass
                #random.randint(1, 10)  # 产生 1 到 10 的一个整数型随机数
                #random.shuffle(a)
                flag = True
                now_time = time.time()
                if self.net_start_time == 0:
                    self.net_start_time = now_time
                else:
                    net_diff_time = int((now_time - self.net_start_time))
                    if (net_diff_time / time_factor) == 1 and False:
                        print(gTag + "EchoServerThread: self.loss_rate= ", self.loss_rate)
                        time_factor += time_step
                        self.loss_rate += 0.05
                        if self.loss_rate > 0.8:
                            self.loss_rate = 0.8
                if self.loss_rate > 0:
                    flag = self.rand_lost_packet(self.loss_rate)
                if flag: # or (ret == 0):
                    self.data_master.PushQueue(recvData, remoteHost, remotePort, c_time_ll)
            #print("[%s:%s] connect" % (remoteHost, remotePort))  # 接收客户端的ip, port
            #print("len(revcData)= ", len(revcData))

        self.sock.close()
        print(gTag + "EchoServerThread: run over")
    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals
        print(gTag + "EchoServerThread: start stop")
        try:
            self.sock.shutdown(socket.SHUT_RDWR)
            self.sock.close()
            pass
        #except IOError, error:  # python2
        except IOError as error:  # python3
            print(gTag + "stop error: ", error)
        else:
            print(gTag + "stop ok")
        #del self.client_master
        self.client_master.stop()
        self.recv_task.stop()
        self.send_task.stop()


        if self.log_fp != None:
            self.log_fp.write("EchoServerThread: max_delay_time= " + str(self.max_delay_time) + "\n")
            self.log_fp.write("EchoServerThread: max_delay_packet= " + str(self.max_delay_packet) + "\n")
            self.log_fp.flush()
            self.log_fp.close()
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
        thread.start()
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

if __name__ == "__main__":
    print("start server")
    #udpServer = UdpServer()
    #udpServer.tcpServer()
    RunServer(True)
    print("end server")

#!/usr/bin/env python
# -*- coding:utf8 -*-

import sys

# for python2
try:
    print (sys.getdefaultencoding())
    reload(sys)
    sys.setdefaultencoding('utf-8')
    print (sys.getdefaultencoding())
except:
    pass

import socket
import threading
import time
import signal
import json
from ctypes import *
import random
import numpy as np
import errno

info_outparam_list = []
time_outparam_list = []
CMD_SIZE = 256
DATA_SIZE = 300 #1500
SO_SNDBUF = 1024 * 1024
SO_RCVBUF = 1024 * 1024

SERVER_TYPE = "audio"

global_port = 8098 #8888
global_host = 'localhost'
#global_host = '172.16.0.17'
#global_host = '111.230.226.17'

if len(sys.argv) > 2:
    global_port = int(sys.argv[2])
if len(sys.argv) > 1:
    global_host = sys.argv[1]


def char2long(x):
    ret = 0
    try:  # Add these 3 lines
        ret = long(x)
    except: # ValueError:
        print("Something went wrong {!r}".format(x))
    return ret
def str2json(json_str):
    outjson = None
    try:
        outjson = json.loads(json_str, encoding='utf-8')
    except:
        print("error: python version")
        try:
            outjson = json.loads(json_str)
        except:
            print("not json")
        else:
            json_str2 = json.dumps(outjson, encoding='utf-8', ensure_ascii=False,
                                      sort_keys=True)
            #print("1: json_str2= ", json_str2)
    else:
        json_str2 = json.dumps(outjson, encoding='utf-8', ensure_ascii=False,
                                  sort_keys=True)
        #print("0: json_str2= ", json_str2)
    return outjson

class ClientManager(object):
    def __init__(self, server):
        self.server = server
        self.lock = threading.Lock()
        self.clientQueue = {}

    def ReadCmd(self, str_cmd):
        cmd = {}
        try:
            cmd = json.loads(str_cmd, encoding='utf-8')
        except:
            try:
                cmd = json.loads(str_cmd)
            except:
                #print("not cmd")
                pass
            else:
                pass
        else:
            pass
        return cmd

    def PopQueue(self, sockId):
        self.lock.acquire()
        if self.clientQueue.get(sockId) == None:
            self.clientQueue.pop(sockId)
        self.lock.release()

    def PushQueue(self, data, remoteHost, remotePort, recv_time):
        ret = -1
        actor = -1
        sessionId = -1
        if len(data) <= CMD_SIZE:
            cmd = self.ReadCmd(data)
            if cmd != {}:
                if cmd.get("actor") != None:
                    actor = cmd.get("actor")
                else:
                    pass
                if cmd.get("sessionId") != None:
                    sessionId = cmd.get("sessionId")
                else:
                    pass
            else:
                pass
        sockId = remoteHost + "_" + str(remotePort)

        if False:
            test_str = json.dumps(self.clientQueue, encoding='utf-8', ensure_ascii=False, indent=4, sort_keys=True)
            print(test_str)
        self.lock.acquire()
        if self.clientQueue.get(sockId) == None and actor >= 0:
            print("ClientManager: PushQueue: cmd: actor= ", actor)
            head_dict = {"actor": actor, "sessionId" : sessionId}
            self.clientQueue.update({sockId: head_dict})
            ret = 0
        elif actor < 0:
            #print("ClientManager: PushQueue: len(data)= ", len(data))
            if self.clientQueue.get(sockId) != None:
                head_dict = self.clientQueue.get(sockId)
                if head_dict.get("data") == None:
                    head_dict.update({"data" : [(data, recv_time)]})
                else:
                    datalist = head_dict.get("data")
                    datalist.append((data, recv_time))
                    ###
                    if len(datalist) > 40:
                        (data0, recv_time0) = datalist[0]
                        (data1, recv_time1) = datalist[len(datalist) - 1]
                        difftime = int(recv_time1 - recv_time0)
                        if difftime > 40:
                            type = SERVER_TYPE
                            print("ClientManager: PushQueue: (type, sockId, len(datalist), difftime)= ", (type, sockId, len(datalist), difftime))
                    self.clientQueue[sockId]["data"] = datalist
                ret = 0
        self.lock.release()
        self.server.send_task.flag.set()
        return ret

    def GetClient(self, actor, sessionId):
        (remoteHost, remotePort, target_sessionId, data) = ('', 0, -1, None)
        ret = (remoteHost, remotePort, target_sessionId, data)
        ret = []
        self.lock.acquire()
        for key, value in self.clientQueue.items():
            this_actor = value.get("actor")
            this_sessionId = value.get("sessionId")
            if this_actor != None and this_sessionId != None:
                if this_actor == 1 and this_actor == actor: #share
                    if sessionId < 0 or sessionId == this_sessionId:
                        dataList = value.get("data")
                        if dataList != None:
                            # if len(dataList) > 0:
                            #    data = dataList[0]
                            #    del(dataList[0])
                            #    self.clientQueue[key]["data"] = dataList
                            for data in dataList:
                                host_port = key.split('_')
                                (remoteHost, remotePort) = (host_port[0], int(host_port[1]))
                                target_sessionId = this_sessionId
                                ret0 = (remoteHost, remotePort, target_sessionId, data)
                                ret.append(ret0)
                            self.clientQueue[key]["data"] = []
                elif this_actor == 2 and this_actor == actor: #watch
                    if sessionId == this_sessionId:
                        host_port = key.split('_')
                        (remoteHost, remotePort) = (host_port[0], int(host_port[1]))
                        target_sessionId = this_sessionId
                        ret0 = (remoteHost, remotePort, target_sessionId, data)
                        ret.append(ret0)
        self.lock.release()
        return ret
class DataManager(object):
    def __init__(self):
        self.lock = threading.Lock()
        self.DataList = []

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
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
    def run(self):
        while self.__running.isSet():
            self.__flag.wait()   # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            data = self.server.data_master.PopQueue()
            if data != None:
                (revcData, remoteHost, remotePort, recv_time) = data
                loss_rate = self.server.load.lib.api_count_loss_rate(revcData, len(revcData))
                if loss_rate >= 0:
                    print("Audio RecvTaskManagerThread: loss_rate= ", loss_rate)
                #print("TaskManagerThread: recv_time= ", recv_time)
                #start_time = time.time()
                self.server.client_master.PushQueue(revcData, remoteHost, remotePort, recv_time)
                #difftime = time.time() - start_time
                #print('{} client_master.PushQueue time: {:.3f}ms'.format(time.ctime(), difftime * 1000))
            else:
                time.sleep(0.001) #20ms

    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals
        try:
            self.sock.close()
        except:
            print("stop error")
        else:
            print("stop ok")

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


    def send_ack(self, task_data, jsonobj):
        (remoteHost, remotePort, target_sessionId, data) = task_data
        json_str = json.dumps(jsonobj, encoding='utf-8', ensure_ascii=False, sort_keys=False)
        sendDataLen = self.server.sock.sendto(json_str, (remoteHost, remotePort))
        if (self.ack_times % 10000) == 0:
            print("send_ack: (sendDataLen, self.ack_times): ", (sendDataLen, self.ack_times))
        ret = len(json_str)
        self.ack_times += 1
        return ret
    def PushQueue(self, task_data):
        (remoteHost, remotePort, target_sessionId, data) = task_data
        sockId = remoteHost + "_" + str(remotePort)
        (this_data, recv_time) = data
        #print("SendTaskManagerThread: PushQueue: start")
        id = 16
        if id not in info_outparam_list:
            info_outparam_list.append(id)
        ret = self.server.load.lib.api_get_audio_extern_info(id, this_data, self.outparam)
        #print("SendTaskManagerThread: PushQueue: ret= ", ret)
        if ret > 0:
            #print("SendTaskManagerThread: PushQueue: ret= ", ret)
            outjson = str2json(self.outparam[0])
            if outjson != None:
                time_status = outjson["time_status"]
                #print("SendTaskManagerThread: PushQueue: time_status= ", time_status)
                if time_status == 0:
                    #self.ctime[0] = "0123456789012345"
                    itime = self.server.load.lib.api_get_time2(self.ll_handle, self.ctime)
                    c_time_ll = char2long(self.ctime[0])
                    #diff_time = self.server.load.lib.api_get_pkt_delay(this_data, len(this_data))
                    if c_time_ll == 0:
                        print("itime= ", itime)
                        return ret
                    seqnum = outjson["seqnum"]
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
                print("save_info: not json", SERVER_TYPE)
        return ret

    def send_ack_0(self, task_data):
        (remoteHost, remotePort, target_sessionId, data) = task_data
        (this_data, recv_time) = data

        ret = self.server.load.lib.api_audio_get_extern_info(12, this_data, self.outparam)
        if ret > 0:
            print("send_ack: ret= ", ret)
            outjson = str2json(self.outparam[0])
            if outjson != None:
                time_status = outjson["time_status"]
                if time_status == 0:
                    seqnum = outjson["seqnum"]
                    packet_time_stamp = long(outjson["packet_time_stamp"])
                    self.outparam[1] = "api_get_time_stamp2"
                    self.server.load.lib.api_get_time_stamp2(12, this_data, self.outparam)
                    c_time_ll = long(self.outparam[0])
                    rtt = {}
                    rtt.update({"seqnum": seqnum})
                    rtt.update({"st0": packet_time_stamp})
                    rtt.update({"rt0": recv_time})
                    rtt.update({"st1": c_time_ll})
                    jsonobj = {"rtt": rtt}
                    json_str = json.dumps(jsonobj, encoding='utf-8', ensure_ascii=False, sort_keys=True)
                    sendDataLen = self.server.sock.sendto(json_str, (remoteHost, remotePort))
                    print("send_ack: sendDataLen: ", sendDataLen)
                    ret = len(json_str)

        return ret

    def run(self):
        while self.__running.isSet():
            self.flag.wait()   # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            (actor, sessionId) = (1, -1)
            start_time = time.time()
            task_data_list = self.server.client_master.GetClient(actor, sessionId)
            has_send = False
            for task_data in task_data_list:
            #if task_data != None:
                (remoteHost, remotePort, target_sessionId, data) = task_data
                if target_sessionId >= 0:
                    self.PushQueue(task_data)
                    task_data_list2 = self.server.client_master.GetClient(2, target_sessionId)
                    for task_data2 in task_data_list2:
                        (remoteHost2, remotePort2, target_sessionId2, data2) = task_data2
                        if target_sessionId2 == target_sessionId:
                            (this_data, recv_time) = data

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
                                    print("send2send diff_time= ", diff_time)
                            try:
                                sendDataLen = self.server.sock.sendto(this_data, (remoteHost2, remotePort2))
                                #
                            except IOError, error:  # python2
                            # except IOError as error:  # python3
                                print("(error, sendDataLen,remoteHost2, remotePort2): ", (error, sendDataLen, remoteHost2, remotePort2))
                            else:
                                #print("(sendDataLen,remoteHost2, remotePort2): ", (sendDataLen, remoteHost2, remotePort2))
                                time.sleep(0.0001)
                                has_send = True
                                pass
            if has_send == False:
                time.sleep(0.001) #1ms
                #self.flag.clear()
            difftime = int((time.time() - start_time) * 1000)
            if difftime > 100:
                print('SendTaskManagerThread time: difftime= ', difftime)

    def stop(self):
        self.flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals
        if self.server.log_fp != None:
            self.server.log_fp.write("send2send: max_delay_time= " + str(self.max_delay_time) + "\n")
            self.server.log_fp.flush()



    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞

class EchoServerThread(threading.Thread):
    def __init__(self, actor, host, port):
        threading.Thread.__init__(self)
        ###
        import loadlib
        self.load = loadlib.gload

        array_type = c_char_p * 4
        self.outparam = array_type()

        self.max_delay_time = 0
        self.max_delay_packet = -1
        self.log_fp = None
        self.filelist0 = []
        self.filelist1 = []
        if True:
            try:
                self.log_fp = open("../../mytest/server_log_audio.txt", "w")
            except:
                try:
                    self.log_fp = open("./server_log_audio.txt", "w")
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
        self.lost_packet_rate = 0 #0.1 #10%
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, SO_SNDBUF)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, SO_RCVBUF)
        self.sock.bind((self.host, self.port))  # 绑定同一个域名下的所有机器
        print("EchoServerThread: self.host= ", self.host)
        print("EchoServerThread: self.port= ", self.port)
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
            print("####################################################rand_lost_packet: ret= ", ret)
        #print("rand_lost_packet: index= ", index)
        difftime = time.time() - start_time
        #print('{} rand_lost_packet: time: {:.3f}ms'.format(time.ctime(), difftime * 1000))
        return ret
    def save_info(self, data, remoteHost, remotePort):
        id = 15
        if id not in info_outparam_list:
            info_outparam_list.append(id)
        ret = self.load.lib.api_get_audio_extern_info(id, data, self.outparam)
        if ret > 0:
            outjson = str2json(self.outparam[0])
            if outjson != None:
                seqnum = outjson["seqnum"]
                sockId = remoteHost + "_" + str(remotePort)
                #print("EchoServerThread: save_info: (sockId, seqnum)= ", (sockId, seqnum))
                if sockId not in self.filelist0:
                    self.filelist0.append(sockId)
                    try:
                        fp = open("./log/server_recv_audio_" + sockId + ".txt", "w")
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
                print("save_info: not json", SERVER_TYPE)
        return ret
    def run(self):
        while self.__running.isSet():
            self.__flag.wait()   # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            try:
                recvData, (remoteHost, remotePort) = self.sock.recvfrom(DATA_SIZE)
            except IOError, error:  # python2
            # except IOError as error:  # python3
                print("run: recvfrom error= ", error)
            else:
                self.recv_packet_num += 1
                #ret = self.save_info(recvData, remoteHost, remotePort)
                #print("run: recvfrom: len(revcData)= ", len(recvData))
                if (self.recv_packet_num % 10000) == 0:
                    print("run: recvfrom: self.recv_packet_num", self.recv_packet_num)

                #print("run: recvfrom: len(recvData)", len(recvData))

                #self.ctime[0] = "0123456789012345"
                itime = self.load.lib.api_get_time2(self.ll_handle, self.ctime)
                c_time_ll = char2long(self.ctime[0])
                if c_time_ll == 0:
                    print("itime= ", itime)
                diff_time = self.load.lib.api_get_pkt_delay(recvData, len(recvData))

                if diff_time > self.max_delay_time:
                    self.max_delay_time = diff_time
                    self.max_delay_packet = self.recv_packet_num
                    print("send2receive diff_time= ", diff_time)
                    #if self.log_fp != None:
                    #    self.log_fp.write("max_delay_time= " + str(self.max_delay_time) + "\n")
                    #    self.log_fp.flush()
                if abs(diff_time) > 1:
                    pass
                #random.randint(1, 10)  # 产生 1 到 10 的一个整数型随机数
                #random.shuffle(a)
                flag = True
                flag = self.rand_lost_packet(self.lost_packet_rate)
                if flag:# or (ret == 0):
                    self.data_master.PushQueue(recvData, remoteHost, remotePort, c_time_ll)
            #print("[%s:%s] connect" % (remoteHost, remotePort))  # 接收客户端的ip, port
            #print("len(revcData)= ", len(revcData))

        self.sock.close()

    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals
        try:
            self.sock.shutdown(socket.SHUT_RDWR)
            self.sock.close()
        except:
            print("stop error")
        else:
            print("stop ok")
        self.recv_task.stop()
        self.send_task.stop()

        if self.log_fp != None:
            self.log_fp.write("EchoServerThread: max_delay_time= " + str(self.max_delay_time) + "\n")
            self.log_fp.write("EchoServerThread: max_delay_packet= " + str(self.max_delay_packet) + "\n")
            self.log_fp.flush()
            self.log_fp.close()

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

if __name__ == "__main__":
    print("start server")
    #udpServer = UdpServer()
    #udpServer.tcpServer()
    RunServer(True)
    print("end server")
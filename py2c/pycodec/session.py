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

import os
import socket
import threading
import signal
import time
import json
from ctypes import *

import loadlib
from udpclient import DecoderClient as vidodec
import udpclient
from udpclient import EncoderClient as videoenc
from udpclient import ShowThread as display
from udpaudioclient import DecoderClient as audiodec
from udpaudioclient import EncoderClient as audioenc
from udpaudioclient import PlayThread as player

LOSS_RATE = 0.2#0.4 #0.2  # 0.8 #0.6 #0.2
EXCHANGE = 0# 1#0#1
SHOW_WIDTH = loadlib.WIDTH
SHOW_HEIGHT = loadlib.HEIGHT
SHOW_TYPE = 0# 1
SHOW_POLL_TIME = 10  # 5
SVC_REFS = 2#16#2
QOS_LEVEL = 1#0#1#3#2#1#0


global_port0 = 8097
global_port1 = 8098
global_host = 'localhost'
# global_host = '172.16.0.17'
# global_host = '111.230.226.17'
if len(sys.argv) > 3:
    global_port1 = int(sys.argv[3])
if len(sys.argv) > 2:
    global_port0 = int(sys.argv[2])
if len(sys.argv) > 1:
    global_host = sys.argv[1]



class AVSession(threading.Thread):
    def __init__(self, windId, params):
        threading.Thread.__init__(self)
        self.windId = windId
        self.params = params
        self.width = 1280
        self.height = 720
        self.conferenceInfo = params.get("conference")
        self.multdevice = params.get("multdevice")
        self.control = params.get("control")
        self.general = params.get("general")
        #if self.multdevice != None:
        #    print("creat_preview: self.multdevice= ", self.multdevice)
        self.video_thread_show = None #display(windId, SHOW_WIDTH, SHOW_HEIGHT)
        self.video_thread_list = []

        self.audio_thread_show = None #player(2)
        self.audio_thread_list = []
        ###
        self.init()
        ###
        self.lock = threading.Lock()
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
        self.pause()
        self.status = 0
        self.preview = None
        self.winid = 0
    def init(self):
        if self.multdevice != None:
            print("creat_preview: self.multdevice= ", self.multdevice)
            self.deviceList = []
            for key, value in self.multdevice.items():
                self.deviceList.append(value)
        if self.control != None:
            mode = self.control["mode"]
            rect = self.control["rect"]
            if mode != None and rect != None:
                # self.deviceNum = len(rect)
                self.upList = []
                for thisRect in rect:
                    chanId = thisRect["chanId"]
                    deviceId = thisRect["deviceId"]
                    devicePos = thisRect["pos"]
                    self.upList.append((chanId, deviceId, devicePos))
        if self.general != None:
            mode = self.general["mode"]
            if mode != None:
                self.modeId = mode["modeId"]
                self.rects = mode["modePos"]
                #print("AVSession: init: self.rects= ", self.rects)
            supprams = self.general["supprams"]
            if supprams != None:
                spatiallayer = int(supprams["spatiallayer"]["name"])
                print("AVSession: init: spatiallayer= ", spatiallayer)
        if self.conferenceInfo != None:
            self.width = self.conferenceInfo["width"]
            self.height = self.conferenceInfo["height"]

        udpclient.SHOW_WIDTH = self.width
        udpclient.SHOW_HEIGHT = self.height
        self.video_thread_show = display(self.windId, self.width, self.height)
        self.audio_thread_show = player(2)
        self.video_thread_show.start()
        self.audio_thread_show.start()
    def create_clients(self):
        #导播合成
        #无导播合成多屏
        #导播合成多屏
        #导播多流
        if len(self.upList) > 0 and len(self.deviceList) > 0: #导播多流
            for thisUp in self.upList:
                chanId = thisUp[0]
                deviceId = thisUp[1]
                devicePos = thisUp[2]
                if deviceId >= 0:
                    deviceDict = self.deviceList[deviceId]
                    ###test
                    self.create_down_clients(chanId + 1, devicePos)
                    ###
                    self.create_up_clients(chanId, devicePos, deviceDict)

                else:
                    self.create_down_clients(chanId, devicePos)

    def create_up_clients(self, chanId, devicePos, deviceDict):
        (id, sessionId, actor) = (chanId, 100, 1)
        conf = self.conferenceInfo
        sessionId = conf["sessionId"]
        sup = self.general["supprams"]
        spatiallayer = int(sup["spatiallayer"]["name"])
        refs = int(sup["temporallayer"]["name"])
        serverAddr = sup["serverAddr"]
        host = serverAddr.split(":")[0]
        port = int(serverAddr.split(":")[1])
        ###video
        (bitrate, mtu_size, fec_level, buffer_shift) = self.GetParams(refs)
        video_thread = videoenc(id, sessionId, actor, host, port)
        video_thread.encode0.refs = refs  # 16
        video_thread.fec_level = fec_level  # 1#0#2#0#1#2#3#0#4 #3 #2#0
        video_thread.encode0.enable_fec = 1
        video_thread.encode0.lost_rate = LOSS_RATE  # 0.2 #0.3
        video_thread.encode0.code_rate = (1 - video_thread.encode0.lost_rate)
        bitrate = int(video_thread.encode0.code_rate * bitrate)
        video_thread.encode0.bit_rate = bitrate
        video_thread.encode0.mtu_size = mtu_size
        video_thread.encode0.setparam()
        video_thread.opendevice(1)
        video_thread.opencodec()
        video_thread.show = self.video_thread_show
        self.video_thread_show.InsertId(video_thread.id)
        video_thread.start()
        self.video_thread_list.append(video_thread)
        ###audio
        (bitrate, mtu_size, buffer_shift) = (24000, 300, 10)
        bitrate = int(deviceDict["audios"]["bitrate"]["name"])
        port = port + 1
        audio_thread = audioenc(id, sessionId, actor, host, port)
        audio_thread.encode0.bit_rate = bitrate
        audio_thread.encode0.mtu_size = mtu_size
        audio_thread.opendevice(1)
        audio_thread.opencodec()
        audio_thread.show = self.audio_thread_show
        self.audio_thread_show.InsertId(audio_thread.id)
        audio_thread.start()
        self.audio_thread_list.append(audio_thread)

    def create_down_clients(self, chanId, devicePos):
        (id, sessionId, actor) = (chanId, 100, 2)
        conf = self.conferenceInfo
        sessionId = conf["sessionId"]
        sup = self.general["supprams"]
        spatiallayer = int(sup["spatiallayer"]["name"])
        refs = int(sup["temporallayer"]["name"])
        serverAddr = sup["serverAddr"]
        host = serverAddr.split(":")[0]
        port = int(serverAddr.split(":")[1])
        ###video
        (bitrate, mtu_size, fec_level, buffer_shift) = self.GetParams(refs)
        video_thread = vidodec(id, sessionId, actor, host, port)
        video_thread.decode0.min_distance = 2
        video_thread.decode0.delay_time = 100
        video_thread.decode0.buf_size = (1 << buffer_shift)  # 1024#必须是2的指数
        video_thread.decode0.mtu_size = mtu_size
        video_thread.show = self.video_thread_show
        self.video_thread_show.InsertId(video_thread.id)
        video_thread.start()
        self.video_thread_list.append(video_thread)
        ###audio
        (bitrate, mtu_size, buffer_shift) = (24000, 300, 10)
        port = port + 1
        audio_thread = audiodec(id, sessionId, actor, host, port)
        audio_thread.decode0.min_distance = 2
        audio_thread.decode0.delay_time = 100
        audio_thread.decode0.buf_size = (1 << buffer_shift)  # 1024#必须是2的指数
        audio_thread.decode0.mtu_size = mtu_size
        audio_thread.show = self.audio_thread_show
        self.audio_thread_show.InsertId(audio_thread.id)
        audio_thread.start()
        self.audio_thread_list.append(audio_thread)
    def clients_stop(self):
        for video_thread in self.video_thread_list:
            video_thread.stop()
        self.video_thread_show.stop()
        for audio_thread in self.audio_thread_list:
            audio_thread.stop()
        self.audio_thread_show.stop()
    def get_status(self):
        ret = 0
        self.lock.acquire()
        ret = self.status
        self.lock.release()
        return ret
    def set_status(self, value):
        self.lock.acquire()
        self.status = value
        self.lock.release()
    def set_winid(self, win_hnd):
        if win_hnd > 0:
            self.winid = win_hnd
    def creat_session(self, win_hnd):
        if win_hnd > 0:
            self.winid = win_hnd
        self.create_clients()
    def close_session(self):
        self.clients_stop()
        self.set_status(0)

    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞

    def run(self):
        while self.__running.isSet():
            self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            status = self.get_status()
            if status == 1:
                self.creat_session(0)
                self.pause()
            elif status == 2:
                self.close_session()
                self.set_status(0)
        if self.get_status():
            self.close_preview()
        self.stop()
        print("AVSession: run over")

    def GetParams(self, refs):
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

if __name__ == "__main__":
    print("start avclient")

    print("end avclient")

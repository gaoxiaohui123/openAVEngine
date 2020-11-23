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

MAX_CHANID = 0x10000 #8#1#0x10000
LOSS_RATE = 0.05 #0.2#0.4 #0.2  # 0.8 #0.6 #0.2
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
        self.screen_width = 1280
        self.screen_height = 720
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
        #self.deviceList = []
        self.speakList = [] #记录共享者的ID号
        self.upChanList = []
        self.upAudioChanList = []
        self.clientList = []
        self.init_params()
        #self.init_video()#注意：不能在主线程中创建SDL，否则退出后重启崩溃
        self.init_audio()
        ###
        self.lock = threading.Lock()
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
        self.pause()
        self.status = 0
        self.winid = 0
    def __del__(self):
        print("AVSession del")
        if self.video_thread_show != None:
            del self.video_thread_show
        if self.audio_thread_show != None:
            del self.audio_thread_show

        n = len(self.video_thread_list)
        if n > 0:
            for i in range(n):
                del self.video_thread_list[0]
            del self.video_thread_list

        n = len(self.audio_thread_list)
        if n > 0:
            for i in range(n):
                del self.audio_thread_list[0]
            del self.audio_thread_list

        n = len(self.clientList)
        if n > 0:
            for i in range(n):
                del self.clientList[0]
            del self.clientList

        n = len(self.rectList)
        if n > 0:
            for i in range(n):
                del self.rectList[0]
            del self.rectList

        n = len(self.upChanList)
        if n > 0:
            for i in range(n):
                del self.upChanList[0]
            del self.upChanList

        if self.conferenceInfo != None:
            del self.conferenceInfo
        if self.multdevice != None:
            del self.multdevice
        if self.control != None:
            del self.control
        if self.general != None:
            del self.general

    def init_params(self):
        self.rectList = []
        if self.general != None and self.general.get("mode") != None:
            if self.general.get("mode") != None:
                modeId = self.general["mode"]["modeId"]
                modePos = self.general["mode"]["modePos"]
                for thisRect in modePos:
                    chanId = thisRect["chanId"]
                    deviceId = thisRect["deviceId"]
                    devicePos = thisRect["pos"]
                    self.speakList.append(chanId)
                    self.clientList.append((chanId, deviceId, devicePos))
                    self.rectList.append(devicePos)
                    if deviceId >= 0:
                        deviceDict = self.multdevice[str(deviceId)]
                        if deviceDict != None:
                            videos = deviceDict["videos"]
                            if videos != None:
                                scapture = videos["capture"]["name"]
                                sresolution = videos["resolution"]["name"]
                                if sresolution not in ["其他"]:
                                    swidth = sresolution.split("x")[0]
                                    sheight = sresolution.split("x")[1]
                                    (self.width, self.height) = (int(swidth), int(sheight))

        elif self.control != None:
            mode = self.control["mode"]
            rect = self.control["rect"]
            if mode != None and rect != None:
                # self.deviceNum = len(rect)
                #self.clientList = []
                for thisRect in rect:
                    chanId = thisRect["chanId"]
                    deviceId = thisRect["deviceId"]
                    devicePos = thisRect["pos"]
                    self.clientList.append((chanId, deviceId, devicePos))
                    self.rectList.append(devicePos)
                    if deviceId >= 0:
                        deviceDict = self.multdevice[str(deviceId)]
                        if deviceDict != None:
                            videos = deviceDict["videos"]
                            if videos != None:
                                scapture = videos["capture"]["name"]
                                sresolution = videos["resolution"]["name"]
                                if sresolution not in ["其他"]:
                                    swidth = sresolution.split("x")[0]
                                    sheight = sresolution.split("x")[1]
                                    (self.width, self.height) = (int(swidth), int(sheight))
        if self.general != None:
            mode = self.general["mode"]
            if mode != None:
                if mode.get("modeId") != None:
                    self.modeId = mode["modeId"]
                if mode.get("modePos") != None:
                    self.rects = mode["modePos"]
                # print("AVSession: init: self.rects= ", self.rects)
            supprams = self.general["supprams"]
            if supprams != None:
                spatiallayer = int(supprams["spatiallayer"]["name"])
                print("AVSession: init_video: spatiallayer= ", spatiallayer)
        if self.conferenceInfo != None:
            self.screen_width = self.conferenceInfo["width"]
            self.screen_height = self.conferenceInfo["height"]
    def init_video(self):
        udpclient.SHOW_WIDTH = self.screen_width
        udpclient.SHOW_HEIGHT = self.screen_height
        print("AVSession: init_video: (self.width, self.height)= ", (self.width, self.height))
        self.video_thread_show = display(self.windId, self.screen_width, self.screen_height, self.width, self.height)
        if len(self.rectList) > 0:
            self.video_thread_show.SetRects(self.rectList)
        self.video_thread_show.start()
        return
    def init_audio(self):
        self.audio_thread_show = player(2)
        self.audio_thread_show.start()
        return
    def reset_pos(self, rectList):
        if self.video_thread_show != None:
            self.rectList = rectList
            self.video_thread_show.SetRects(self.rectList)
    def AddSpeakerId(self, thisObj):
        for thisChanId in self.speakList:
            thisObj.AddSpeakerId(thisChanId)
    def create_clients(self):
        self.init_video()
        #self.init_audio()#音频必须在主线程中启动，否则会崩溃
        #导播合成
        #无导播合成多屏
        #导播合成多屏
        #导播多流
        if len(self.clientList) > 0 and (self.multdevice != None): #len(self.deviceList) > 0: #导播多流
            #先建立编码通道
            for thisClient in self.clientList:
                chanId = thisClient[0]
                deviceId = int(thisClient[1])
                devicePos = thisClient[2]
                if deviceId >= 0:
                    deviceDict = self.multdevice[str(deviceId)]  # self.deviceList[deviceId]
                    print("create_clients: deviceDict= ", deviceDict)
                    video_thread = self.create_up_video(chanId, devicePos, deviceDict)
                    self.upChanList.append(video_thread)
                    audio_thread = self.create_up_audio(chanId, devicePos, deviceDict)
                    self.upAudioChanList.append(audio_thread)

            # 再建立解码通道
            for thisClient in self.clientList:
                chanId = thisClient[0]
                deviceId = int(thisClient[1])
                devicePos = thisClient[2]
                if deviceId >= 0:
                    pass
                else:
                    self.create_down_video(chanId, devicePos)
                    self.create_down_audio(chanId, devicePos)

    def create_up_video(self, chanId, devicePos, deviceDict):
        print("create_up_video: chanId= ", chanId)
        (id, sessionId, actor) = (chanId, 100, 1)
        conf = self.conferenceInfo
        sessionId = conf["sessionId"]
        sup = self.general["supprams"]
        spatiallayer = int(sup["spatiallayer"]["name"])
        refs = int(sup["temporallayer"]["name"])
        print("create_up_video: refs=", refs)
        serverAddr = sup["serverAddr"]
        host = serverAddr.split(":")[0]
        port = int(serverAddr.split(":")[1])
        ###video
        (bitrate, mtu_size, fec_level, buffer_shift) = self.GetParams(refs, self.width, self.height)
        video_thread = videoenc(id, sessionId, actor, host, port)
        self.AddSpeakerId(video_thread)
        ###
        devicetype = 4
        osd_enable = sup["osd"]
        fecenable = 1
        if deviceDict != None:
            thisDevice = deviceDict["videos"]
            if thisDevice != None:
                sresolution = thisDevice["resolution"]["name"]
                if sresolution not in ["其他"]:
                    swidth = sresolution.split("x")[0]
                    sheight = sresolution.split("x")[1]
                    video_thread.width = int(swidth)
                    video_thread.height = int(sheight)
                (bitrate, mtu_size, fec_level, buffer_shift) = self.GetParams(refs, video_thread.width, video_thread.height)
                sbitrate = thisDevice["bitrate"]["name"]
                if sbitrate not in ["default", "其他"]:
                    sbitrate = sbitrate.replace("kbps","")
                    ibitrate = int(sbitrate)
                    bitrate = ibitrate * 1024
                    print("create_up_video: ibitrate= ", ibitrate)
                scapture = thisDevice["capture"]["name"]
                print("create_up_video: scapture= ", scapture)
                if scapture in ["YUV"]:
                    yuvfilename = thisDevice["yuvfilename"]
                    if yuvfilename != None:
                        video_thread.yuvfilename = yuvfilename
                        devicetype = -1
                elif "video" in scapture:
                    video_thread.input_name = "v4l2"
                    video_thread.device_name = "/dev/" + str(scapture.split(":")[0])
                    video_thread.input_format = str(scapture.split(":")[1])
                elif "fb" in scapture:
                    video_thread.input_name = "x11grab"
                    video_thread.device_name = ":0.0"
                    video_thread.input_format = "raw"
                else:
                    pass
                print("create_up_video: video_thread.input_name= ", video_thread.input_name)
                print("create_up_video: video_thread.device_name= ", video_thread.device_name)
                print("create_up_video: video_thread.input_format= ", video_thread.input_format)
                scodec = thisDevice["codec"]["name"]
                fecenable = thisDevice["fec"]
                denoise = thisDevice["denoise"]
                if denoise:
                    video_thread.denoise = 2
                sframerate = thisDevice["framerate"]["name"]
                if "fps" in sframerate:
                    sframerate = sframerate.replace("fps", "")
                    video_thread.framerate = int(sframerate)
                    #video_thread.encode0.frame_rate: fixed 25

        print("create_up_video: bitrate= ", bitrate)
        print("create_up_video: fecenable= ", fecenable)
        ###
        (video_thread.encode0.width, video_thread.encode0.height) = (video_thread.width, video_thread.height)
        video_thread.encode0.refs = refs  # 16
        video_thread.fec_level = fec_level  # 1#0#2#0#1#2#3#0#4 #3 #2#0
        video_thread.encode0.enable_fec = fecenable
        video_thread.encode0.loss_rate = LOSS_RATE  # 0.2 #0.3
        video_thread.encode0.code_rate = (1 - video_thread.encode0.loss_rate)
        bitrate = int(video_thread.encode0.code_rate * bitrate)
        video_thread.encode0.bit_rate = bitrate
        video_thread.encode0.mtu_size = mtu_size
        video_thread.encode0.setparam()
        video_thread.opendevice(devicetype)
        video_thread.opencodec()
        video_thread.show = self.video_thread_show
        self.video_thread_show.InsertId(video_thread.id)
        video_thread.start()
        self.video_thread_list.append(video_thread)
        return video_thread
    def create_up_audio(self, chanId, devicePos, deviceDict):
        (id, sessionId, actor) = (chanId, 100, 1)
        conf = self.conferenceInfo
        sessionId = conf["sessionId"]
        sup = self.general["supprams"]
        spatiallayer = int(sup["spatiallayer"]["name"])
        refs = int(sup["temporallayer"]["name"])
        serverAddr = sup["serverAddr"]
        host = serverAddr.split(":")[0]
        port = int(serverAddr.split(":")[1])
        ###audio
        (bitrate, mtu_size, buffer_shift) = (24000, 300, 10)
        bitrate = int(deviceDict["audios"]["bitrate"]["name"])
        port = port + 1
        audio_thread = audioenc(id, sessionId, actor, host, port)
        self.AddSpeakerId(audio_thread)
        ###
        if deviceDict != None:
            thisDevice = deviceDict["audios"]
            if thisDevice != None:
                sbitrate = thisDevice["bitrate"]["name"]
                if sbitrate not in ["default", "其他"]:
                    #sbitrate = sbitrate.replace("kbps", "")
                    bitrate = int(sbitrate)
                    print("create_up_audio: bitrate= ", bitrate)
                srecorder = thisDevice["recorder"]["name"]
                if srecorder not in ["default"]:
                    pass
                ccodec = thisDevice["codec"]["name"]
                cchannels = thisDevice["channels"]["name"]
                channels = int(cchannels)
                caudiofmt = thisDevice["audiofmt"]["name"]
                cframesize = thisDevice["framesize"]["name"]
                framesize = int(cframesize)
                csamplerate = thisDevice["samplerate"]["name"]
                samplerate = int(csamplerate)

                audio_thread.in_channels = channels
                audio_thread.in_sample_rate = samplerate
                #audio_thread.in_sample_fmt = "AV_SAMPLE_FMT_S16"
                #audio_thread.out_nb_samples
                audio_thread.encode0.out_nb_samples = framesize
        audio_thread.encode0.bit_rate = bitrate
        audio_thread.encode0.mtu_size = mtu_size
        audio_thread.opendevice(2)
        audio_thread.opencodec()
        audio_thread.show = self.audio_thread_show
        self.audio_thread_show.InsertId(audio_thread.id)
        audio_thread.start()
        self.audio_thread_list.append(audio_thread)
        return audio_thread
    def create_down_video(self, chanId, devicePos):
        (id, sessionId, actor) = (chanId, 100, 2)
        conf = self.conferenceInfo
        sessionId = conf["sessionId"]
        sup = self.general["supprams"]
        spatiallayer = int(sup["spatiallayer"]["name"])
        refs = int(sup["temporallayer"]["name"])
        osd_enable = sup["osd"]
        serverAddr = sup["serverAddr"]
        host = serverAddr.split(":")[0]
        port = int(serverAddr.split(":")[1])

        (bitrate, mtu_size, fec_level, buffer_shift) = self.GetParams(refs, self.width, self.height)
        video_thread = vidodec(id, sessionId, actor, host, port)
        self.AddSpeakerId(video_thread)
        ###video
        (video_thread.decode0.width, video_thread.decode0.height) = (self.width, self.height)
        video_thread.decode0.min_distance = 2
        video_thread.decode0.delay_time = 100
        video_thread.decode0.buf_size = (1 << buffer_shift)  # 1024#必须是2的指数
        video_thread.decode0.mtu_size = mtu_size
        ###
        for thisThread in self.upChanList:
            video_thread.add_client(thisThread)
        ###
        video_thread.opencodec()
        video_thread.show = self.video_thread_show
        self.video_thread_show.InsertId(video_thread.id)
        video_thread.start()
        self.video_thread_list.append(video_thread)

    def create_down_audio(self, chanId, devicePos):
        (id, sessionId, actor) = (chanId, 100, 2)
        conf = self.conferenceInfo
        sessionId = conf["sessionId"]
        sup = self.general["supprams"]
        spatiallayer = int(sup["spatiallayer"]["name"])
        refs = int(sup["temporallayer"]["name"])
        serverAddr = sup["serverAddr"]
        host = serverAddr.split(":")[0]
        port = int(serverAddr.split(":")[1])
        ###audio
        (bitrate, mtu_size, buffer_shift) = (24000, 300, 11)
        port = port + 1
        audio_thread = audiodec(id, sessionId, actor, host, port)
        self.AddSpeakerId(audio_thread)
        audio_thread.decode0.min_distance = 2
        audio_thread.decode0.delay_time = 100
        audio_thread.decode0.buf_size = (1 << buffer_shift)  # 1024#必须是2的指数
        audio_thread.decode0.mtu_size = mtu_size
        ###
        for thisThread in self.upAudioChanList:
            audio_thread.add_client(thisThread)
        ###
        #audio_thread.opencodec()
        audio_thread.show = self.audio_thread_show
        self.audio_thread_show.InsertId(audio_thread.id)
        audio_thread.start()
        self.audio_thread_list.append(audio_thread)
    def clients_stop(self):

        for audio_thread in self.audio_thread_list:
            #print("clients_stop: audio: 0")
            audio_thread.stop()
            #print("clients_stop: audio: 1")

        for video_thread in self.video_thread_list:
            #print("clients_stop: video: 0")
            video_thread.stop()
            #print("clients_stop: video: 1")


        print("clients_stop: video_thread_show")
        if self.video_thread_show != None:
            self.video_thread_show.stop()
        print("clients_stop: audio_thread_show")
        if self.audio_thread_show != None:
            self.audio_thread_show.stop()
        print("clients_stop: over")
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
        #time.sleep(2)
        print("close_session")
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
            self.close_session()
        self.stop()
        print("AVSession: run over")

    def GetParams(self, refs, width, height):
        ret = (3 * 1024 * 1024, 1400, 0, 10)
        (h, w) = (width, height)
        if (w * h) <= 352 * 288:
            fec_level = 2
            if refs <= 2:
                fec_level = 0
            ret = (512 * 1024, 400, fec_level, 11)
        elif (w * h) <= 640 * 480:
            fec_level = 3
            if refs <= 2:
                fec_level = 0
            ret = (800 * 1024, 600, fec_level, 12)
        elif (w * h) <= 704 * 576:
            fec_level = 3
            if refs <= 2:
                fec_level = 0
            ret = (1200 * 1024, 600, fec_level, 13)
        elif (w * h) <= 1280 * 720:
            fec_level = 1
            if refs <= 2:
                fec_level = 0
            ret = (int(2.0 * 1024 * 1024), 800, fec_level, 14)
        elif (w * h) <= 1920 * 1080:
            fec_level = 1
            if refs <= 2:
                fec_level = 0
            ret = (3.0 * 1024 * 1024, 1100, fec_level, 15)
        else:
            fec_level = 0
            ret = (6 * 1024 * 1024, 1400, fec_level, 16)
        return ret

if __name__ == "__main__":
    print("start avclient")

    print("end avclient")

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
import platform

import loadlib
from udpvclient import DecoderClient as vidodec
import udpvclient
from udpvclient import EncoderClient as videoenc
from udpvclient import ShowThread as display
from udpaclient import DecoderClient as audiodec
from udpaclient import EncoderClient as audioenc
from udpaclient import PlayThread as player
from avstream import AVStream as avstream
#import udpbase as base

DECACTOR  = 128
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

def json2str(jsonobj):
    if sys.version_info >= (3, 0):
        json_str = json.dumps(jsonobj, ensure_ascii=False, sort_keys=False).encode('utf-8')
    else:
        json_str = json.dumps(jsonobj, encoding='utf-8', ensure_ascii=False, sort_keys=False)

    return json_str

class AVSession(threading.Thread):
    def __init__(self, handle, windId, params, nettime, width, height, taskId, isCreater):
        threading.Thread.__init__(self)
        self.status = 0
        self.is_creater = isCreater
        #self.handle = handle
        self.handle_size = 8
        self.handle = create_string_buffer(self.handle_size)
        #self.handle2 = create_string_buffer(self.handle_size)
        self.handle2 = self.handle
        self.windId = windId
        self.params = params
        self.nettime = nettime
        self.modeId = -1
        self.taskId = 0#taskId
        self.screen_width = 1280
        self.screen_height = 720
        (self.width, self.height) = (width, height)
        #(self.width, self.height) = (1920/2, 1080/2)
        self.conferenceInfo = params.get("conference")
        self.multdevice = params.get("multdevice")
        self.control = params.get("control")
        self.general = params.get("general")
        # if self.multdevice != None:
        #    print("creat_preview: self.multdevice= ", self.multdevice)
        self.avstream_thread_list = {}
        self.istream_name = ""
        self.ostream_name = ""

        ###
        # self.deviceList = []
        self.upStreamList = []
        self.downStreamList = []
        self.speakList = []  # 记录共享者的ID号
        self.upChanList = []
        self.upAudioChanList = []
        self.downChanList = []
        self.downAudioChanList = []
        self.vDevList = []
        self.aDevList = []
        self.clientList = []
        self.init_params()
        print("self.screen_width= ",  self.screen_width)
        print("self.width= ", self.width)
        (self.vrtaskId, self.aptaskId, self.mcuTaskId) = (-1, -1, -1)
        loadlib.gload.lib.api_set_time_offset(nettime)
        ###
        param = {}
        #param.update({"taskType": 0})
        param.update({"taskType": 1})
        param.update({"taskId": self.taskId})
        param.update({"aliveTaskNum": 128})
        param.update({"taskNum": 256})
        #param.update({"port": port})
        #param.update({"server_ip": server_ip})
        param_str = json2str(param)
        loadlib.gload.lib.api_pool_start(self.handle, param_str)
        #param.update({"taskType": 1})
        #param_str = json2str(param)
        #loadlib.gload.lib.api_pool_start(self.handle2, param_str)
        ###
        loadlib.gload.lib.api_av_sdl_init()
        self.init_video()
        self.init_audio()
        ###
        self.lock = threading.Lock()
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
        self.pause()
    def __del__(self):
        print("AVSession del")
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
                self.modeId = self.general["mode"]["modeId"]
                modePos = self.general["mode"]["modePos"]
                for thisRect in modePos:
                    chanId = thisRect["chanId"]
                    deviceId = thisRect["deviceId"]
                    devicePos = thisRect["pos"]
                    actor = thisRect.get("actor")
                    if actor == None:
                        actor = chanId
                    self.speakList.append(chanId)
                    (width, height) = (self.width, self.height)

                    self.rectList.append(devicePos)
                    print("init_params: deviceId= ", deviceId)
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
                                    #(self.width, self.height) = (int(swidth), int(sheight))
                                    (width, height) = (int(swidth), int(sheight))
                                    (self.width, self.height) = (width, height)
                    self.clientList.append((chanId, actor, deviceId, devicePos, width, height))

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
                    actor = thisRect.get("actor")
                    if actor == None:
                        actor = chanId
                    (width, height) = (self.width, self.height)
                    #self.clientList.append((chanId, actor, deviceId, devicePos))
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
                                    #(self.width, self.height) = (int(swidth), int(sheight))
                                    (width, height) = (int(swidth), int(sheight))
                                    (self.width, self.height) = (width, height)
                    self.clientList.append((chanId, actor, deviceId, devicePos, width, height))
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
                stream = supprams.get("stream")
                if stream != None:
                    streamValue = stream.get("value")
                    self.ostream_name = stream.get("name")

        if self.conferenceInfo != None:
            self.screen_width = self.conferenceInfo["width"]
            self.screen_height = self.conferenceInfo["height"]
    def init_video(self):
        sup = self.general["supprams"]
        serverAddr = sup["serverAddr"]
        host = serverAddr.split(":")[0]
        port = int(serverAddr.split(":")[1])
        osd_enable = sup["osd"]
        params = {}
        params.update({"taskId": self.taskId})
        params.update({"server_ip": host})
        params.update({"port": port})
        params.update({"screen_width": self.screen_width})
        params.update({"screen_height": self.screen_height})

        #params.update({"width": self.width}) #pixel_width for render sdlTexture
        #params.update({"height": self.height}) #pixel_height for render sdlTexture

        params.update({"width": self.screen_width})
        params.update({"height": self.screen_height})

        params.update({"win_id": self.windId})
        params.update({"osd_enable": osd_enable})

        #param_str = json2str(params)
        #self.vctaskId = loadlib.gload.lib.api_start_capture(self.handle, param_str)
        #print("init_video: self.vctaskId=", self.vctaskId)
        #self.taskId += 1
        #params.update({"taskId": self.taskId})
        param_str = json2str(params)
        self.vrtaskId = loadlib.gload.lib.api_start_render(self.handle, param_str)
        print("init_video: self.vrtaskId=", self.vrtaskId)
        self.taskId += 1
        return
    def init_audio(self):
        sup = self.general["supprams"]
        serverAddr = sup["serverAddr"]
        host = serverAddr.split(":")[0]
        port = int(serverAddr.split(":")[1])
        params = {}
        params.update({"taskId": self.taskId})
        params.update({"server_ip": host})
        params.update({"port": port})
        #param_str = json2str(params)
        #self.actaskId = loadlib.gload.lib.api_start_acapture(self.handle, param_str)
        #print("init_audio: self.actaskId=", self.actaskId)
        #self.taskId += 1
        #params.update({"taskId": self.taskId})
        param_str = json2str(params)
        self.aptaskId = loadlib.gload.lib.api_start_player(self.handle, param_str)
        print("init_audio: self.aptaskId=", self.aptaskId)
        self.taskId += 1
        return
    def poll_pos(self, rectList):
        #if self.video_thread_show != None:
        #    self.rectList = rectList
        #    self.video_thread_show.SetRects(self.rectList)
        if self.vrtaskId >= 0:
            loadlib.gload.lib.api_poll_rect(self.handle, self.vrtaskId)
        return
    def reset_pos(self, modeId, width, height):
        #if self.video_thread_show != None:
        #    self.rectList = rectList
        #    self.video_thread_show.SetRects(self.rectList)
        if self.vrtaskId >= 0:
            loadlib.gload.lib.api_reset_rect(self.handle, self.vrtaskId, modeId, width, height)
        return
    def create_clients(self):
        print("create_clients: start")
        #导播合成
        #无导播合成多屏
        #导播合成多屏
        #导播多流
        if len(self.clientList) > 0 and (self.multdevice != None): #len(self.deviceList) > 0: #导播多流
            # 创建流通道
            sup = self.general["supprams"]
            substream = ""
            if sup.get("substream") != None:
                substream = sup["substream"]["name"]
            for thisClient in self.clientList:
                handle = None
                chanId = thisClient[0]
                actor = thisClient[1]
                deviceId = int(thisClient[2])
                width = thisClient[4]
                height = thisClient[5]
                if deviceId >= 0:
                    deviceDict = self.multdevice[str(deviceId)]
                    thisDevice = deviceDict["videos"]
                    inStreamName = ""
                    rstreamTaskId = -1
                    if thisDevice != None:
                        scapture = thisDevice["capture"]["name"]
                        if scapture in ["FILE"]:
                            inStreamName = thisDevice["yuvfilename"]
                    print("create_clients: inStreamName=", inStreamName)
                    # thisDevice = deviceDict["audios"]
                    # if thisDevice != None:
                    #    srecorder = thisDevice["recorder"]["name"]
                    #    if srecorder in ["stream"]:
                    #        inStreamName = thisDevice["streamname"]
                    if inStreamName != "":
                        params = {}
                        params.update({"infile": inStreamName})
                        params.update({"taskId": self.taskId})
                        params.update({"width": width})
                        params.update({"height": height})
                        params.update({"scodec": "HCSVC"})
                        params.update({"bitrate": 3 * 1024 * 1024})
                        params.update({"mtu_size": 1100})
                        params.update({"denoise": 0})
                        params.update({"osd_enable": 0})
                        params.update({"framerate": 25})
                        params.update({"refs": 1})
                        param_str = json2str(params)
                        rstreamTaskId = loadlib.gload.lib.api_start_stream(self.handle2, param_str)
                        self.taskId += 1
                        ###
                    self.upStreamList.append(rstreamTaskId)
                else:
                    ##substream = "/home/gxh/works/substream_" + str(chanId) + ".flv" #test
                    if substream != "":
                        handle = create_string_buffer(self.handle_size)
                        params = {}
                        params.update({"outfile": substream})
                        param_str = json2str(params)
                        ret = loadlib.gload.lib.api_avstream_init(handle, param_str)
                        if ret < 0:
                            print("create_clients: api_avstream_init fail ! ")
                    self.downStreamList.append(handle)
            time.sleep(1)
            #先建立编码通道
            for thisClient in self.clientList:
                chanId = thisClient[0]
                actor = thisClient[1]
                deviceId = int(thisClient[2])
                devicePos = thisClient[3]
                width = thisClient[4]
                height = thisClient[5]
                if deviceId >= 0:
                    deviceDict = self.multdevice[str(deviceId)]  # self.deviceList[deviceId]
                    print("create_clients: deviceDict= ", deviceDict)
                    (audioTaskId, aDevTaskId) = self.create_up_audio(chanId, actor, devicePos, deviceDict)
                    (videoTaskId, vDevTaskId) = self.create_up_video(chanId, actor, devicePos, deviceDict, width, height)
                    #print("create_clients: audioTaskId= ", audioTaskId)
                    self.upChanList.append((videoTaskId, vDevTaskId))
                    self.upAudioChanList.append((audioTaskId, aDevTaskId))
                    if self.ostream_name != "" and False:
                        streamObj = avstream.AVStream(chanId, width, height)
                        streamObj.outfile = self.ostream_name
                        #streamObj.video_codec_handle = video_thread.get_codec_handle()
                        #streamObj.audio_codec_handle = audio_thread.get_codec_handle()
                        #streamObj.openstream()
                        self.avstream_thread_list.append(streamObj)

            # 再建立解码通道
            for thisClient in self.clientList:
                chanId = thisClient[0]
                deviceId = int(thisClient[2])
                devicePos = thisClient[3]
                width = thisClient[4]
                height = thisClient[5]
                if deviceId >= 0:
                    pass
                else:
                    audioTaskId = self.create_down_audio(chanId, devicePos)
                    videoTaskId = self.create_down_video(chanId, devicePos, width, height)
                    self.downChanList.append(videoTaskId)
                    self.downAudioChanList.append(audioTaskId)
                    print("create_clients: chanId= ", chanId)
        ###
        time.sleep(2)

        ###创建合成通道
        mcustream = ""
        sup = self.general["supprams"]
        if sup.get("stream") != None:
            mcustream = sup["stream"]["name"]
        #mcustream = "/home/gxh/works/mcustream_0" + ".flv"  # test
        print("create_clients:mcustream= ", mcustream)
        if mcustream != "":
            params = {}
            params.update({"outfile": mcustream})
            params.update({"taskId": self.taskId})
            #params.update({"width": self.width})#mcu_width
            #params.update({"height": self.height})#mcu_height
            params.update({"width": self.screen_width})  # mcu_width
            params.update({"height": self.screen_height})  # mcu_height
            params.update({"scodec": "HCSVC"})
            params.update({"bitrate": 3*1024*1024})
            params.update({"mtu_size": 1100})
            params.update({"denoise": 0})
            params.update({"osd_enable": 0})
            params.update({"framerate": 25})
            params.update({"refs": 1})
            param_str = json2str(params)
            self.mcuTaskId = loadlib.gload.lib.api_start_mcu(self.handle2, param_str)
            self.taskId += 1

        #关联设备
        #for (taskId, devTaskId) in self.upAudioChanList:
        for i in range(len(self.upAudioChanList)):
            (taskId, devTaskId) = self.upAudioChanList[i]
            #stream = self.upStreamList[i]
            rstreamTaskId = self.upStreamList[i]
            params = {}
            params.update({"codecTaskId": taskId})
            params.update({"captureTaskId": devTaskId})
            params.update({"renderTaskId": self.aptaskId})
            params.update({"isEncoder": 1})
            param_str = json2str(params)
            loadlib.gload.lib.api_set4codec_audio(self.handle, self.handle2, param_str)
            #if stream != None:
            #    loadlib.gload.lib.api_setstream2audio(self.handle, stream, param_str)
            if rstreamTaskId >= 0:
                params = {}
                params.update({"codecTaskId": taskId})
                params.update({"rstreamTaskId": rstreamTaskId})
                param_str = json2str(params)
                loadlib.gload.lib.api_setreadstream2audio(self.handle, self.handle2, param_str)

        ###
        # self.vctaskId, self.vrtaskId, self.actaskId, self.aptaskId
        # for (taskId, devTaskId) in self.upChanList:
        for i in range(len(self.upChanList)):
            (taskId, devTaskId) = self.upChanList[i]
            print("create_clients: taskId=", taskId)
            #stream = self.upStreamList[i]
            rstreamTaskId = self.upStreamList[i]
            params = {}
            params.update({"codecTaskId": taskId})
            params.update({"captureTaskId": devTaskId})
            params.update({"renderTaskId": self.vrtaskId})
            params.update({"isEncoder": 1})
            param_str = json2str(params)
            loadlib.gload.lib.api_set4codec_video(self.handle, self.handle2, param_str)
            #if stream != None:
            #    loadlib.gload.lib.api_setstream2video(self.handle, stream, param_str)
            if rstreamTaskId >= 0:
                params = {}
                params.update({"codecTaskId": taskId})
                params.update({"rstreamTaskId": rstreamTaskId})
                param_str = json2str(params)
                loadlib.gload.lib.api_setreadstream2video(self.handle, self.handle2, param_str)


        #for taskId in self.downAudioChanList:
        for i in range(len(self.downAudioChanList)):
            taskId = self.downAudioChanList[i]
            stream = self.downStreamList[i]
            params = {}
            params.update({"codecTaskId": taskId})
            params.update({"captureTaskId": -1})
            params.update({"renderTaskId": self.aptaskId})
            params.update({"isEncoder": 0})
            param_str = json2str(params)
            loadlib.gload.lib.api_set4codec_audio(self.handle, self.handle2, param_str)
            if stream != None:
                loadlib.gload.lib.api_setstream2audio(self.handle, stream, param_str)
        ###
        # for taskId in self.downChanList:
        for i in range(len(self.downChanList)):
            taskId = self.downChanList[i]
            stream = self.downStreamList[i]
            params = {}
            params.update({"codecTaskId": taskId})
            params.update({"captureTaskId": -1})
            params.update({"renderTaskId": self.vrtaskId})
            params.update({"isEncoder": 0})
            param_str = json2str(params)
            loadlib.gload.lib.api_set4codec_video(self.handle, self.handle2, param_str)
            if stream != None:
                loadlib.gload.lib.api_setstream2video(self.handle, stream, param_str)
        ###
        if mcustream != "":
            params = {}
            params.update({"mcuTaskId": self.mcuTaskId})
            params.update({"renderTaskId": self.vrtaskId})
            params.update({"playerTaskId": self.aptaskId})
            param_str = json2str(params)
            loadlib.gload.lib.api_set4mcu(self.handle, self.handle2, param_str)

    def create_up_video(self, chanId, actor, devicePos, deviceDict, width, height):
        print("create_up_video: chanId= ", chanId)
        (id, sessionId, selfmode) = (chanId, 100, 0)
        #if not self.is_creater:
        #    actor = 2
        conf = self.conferenceInfo
        sessionId = conf["sessionId"]
        sup = self.general["supprams"]
        spatiallayer = int(sup["spatiallayer"]["name"])
        refs = int(sup["temporallayer"]["name"])
        bwthreshold = 0.25
        if sup.get("bwadjust") != None:
            bwthreshold = float(sup["bwadjust"]["name"])
        print("create_up_video: bwthreshold=", bwthreshold)
        lossrate = 0.0
        if sup.get("lossrate") != None:
            lossrate = float(sup["lossrate"]["name"])
        print("create_up_video: lossrate=", lossrate)

        print("create_up_video: refs=", refs)
        serverAddr = sup["serverAddr"]
        host = serverAddr.split(":")[0]
        port = int(serverAddr.split(":")[1])
        selfmode = sup["selfmode"]
        enable_netack = sup["netack"]
        ###video
        (bitrate, mtu_size, fec_level, buffer_shift) = self.GetParams(refs, width, height)
        (input_name, device_name, input_format, yuvfilename) = ("", "", "", "")
        (scodec, fecenable, denoise, framerate) = ("", 0, 0, 25)
        devicetype = 4
        osd_enable = sup["osd"]
        if deviceDict != None:
            thisDevice = deviceDict["videos"]
            if thisDevice != None:
                sresolution = thisDevice["resolution"]["name"]
                if sresolution not in ["其他"]:
                    swidth = sresolution.split("x")[0]
                    sheight = sresolution.split("x")[1]
                    width = int(swidth)
                    height = int(sheight)
                    (bitrate, mtu_size, fec_level, buffer_shift) = self.GetParams(refs, width, height)
                sbitrate = thisDevice["bitrate"]["name"]
                if sbitrate not in ["default", "其他"]:
                    sbitrate = sbitrate.replace("kbps", "")
                    ibitrate = int(sbitrate)
                    bitrate = ibitrate * 1024
                scapture = thisDevice["capture"]["name"]
                print("create_up_video: scapture= ", scapture)
                if scapture in ["FILE"]:
                    yuvfilename = thisDevice["yuvfilename"]
                    if yuvfilename != None:
                        devicetype = -1
                elif (platform.system() == 'Windows'):
                    input_name = "dshow"
                    device_name = scapture
                    input_format = "mjpeg"
                elif "video" in scapture:
                    input_name = "v4l2"
                    device_name = "/dev/" + str(scapture.split(":")[0])
                    input_format = str(scapture.split(":")[1])
                elif "fb" in scapture:
                    input_name = "x11grab"
                    device_name = ":0.0"
                    input_format = "raw"
                else:
                    print("create_up_video: scapture= ", scapture)
                    input_name = "dshow"
                    device_name = scapture
                    input_format = "mjpeg"
                scodec = thisDevice["codec"]["name"]
                fecenable = thisDevice["fec"]
                denoise = thisDevice["denoise"]
                if denoise:
                    denoise = 2
                sframerate = thisDevice["framerate"]["name"]
                if "fps" in sframerate:
                    sframerate = sframerate.replace("fps", "")
                    framerate = int(sframerate)

        params = {}
        params.update({"actor": actor})
        params.update({"sessionId": sessionId})
        params.update({"chanId": chanId})
        params.update({"modeId": self.modeId})
        params.update({"avtype": 0})
        params.update({"selfmode": selfmode})
        params.update({"width": width})
        params.update({"height": height})
        params.update({"screen_width": self.screen_width})
        params.update({"screen_height": self.screen_height})
        params.update({"enable_netack": enable_netack})
        params.update({"testmode": 0})
        params.update({"nettime": self.nettime})
        params.update({"port": port})
        params.update({"server_ip": host})
        params.update({"idx": 0})
        params.update({"win_id": self.windId})
        params.update({"isEncoder": 1})
        params.update({"taskId": self.taskId})
        #
        params.update({"yuvfilename": yuvfilename})
        params.update({"input_name": input_name})
        params.update({"device_name": device_name})
        params.update({"input_format": input_format})
        params.update({"bitrate": bitrate})
        params.update({"mtu_size": mtu_size})
        params.update({"fecenable": fecenable})
        params.update({"fec_level": fec_level})
        params.update({"buffer_shift": buffer_shift})
        params.update({"scodec": scodec})
        params.update({"denoise": denoise})
        params.update({"osd_enable": osd_enable})
        params.update({"framerate": framerate})
        params.update({"refs": refs})
        params.update({"bwthreshold": bwthreshold})
        params.update({"lossrate": lossrate})


        print("create_up_video: yuvfilename= ", yuvfilename)
        vctaskId = -1
        if yuvfilename == "":
            param_str = json2str(params)
            vctaskId = loadlib.gload.lib.api_start_capture(self.handle, param_str)
            self.vDevList.append(vctaskId)
            self.taskId += 1

        params.update({"taskId": self.taskId})
        param_str = json2str(params)
        codecTaskId = loadlib.gload.lib.api_start_video(self.handle2, param_str)
        self.taskId += 1
        return (codecTaskId, vctaskId)

    def create_up_audio(self, chanId, actor, devicePos, deviceDict):
        (id, sessionId, selfmode) = (chanId, 100, 0)
        #if not self.is_creater:
        #    actor = 2
        conf = self.conferenceInfo
        sessionId = conf["sessionId"]
        sup = self.general["supprams"]
        spatiallayer = int(sup["spatiallayer"]["name"])
        refs = int(sup["temporallayer"]["name"])
        serverAddr = sup["serverAddr"]
        host = serverAddr.split(":")[0]
        port = int(serverAddr.split(":")[1])
        selfmode = sup["selfmode"]
        enable_netack = sup["netack"]
        lossrate = 0.0
        if sup.get("lossrate") != None:
            lossrate = float(sup["lossrate"]["name"])
        print("create_up_audio: lossrate=", lossrate)
        ###audio
        (bitrate, mtu_size, buffer_shift) = (24000, 300, 10)
        bitrate = int(deviceDict["audios"]["bitrate"]["name"])
        (input_name, device_name, yuvfilename) = ("", "", "")
        (scodec, fecenable, denoise) = ("", 0, 0)
        (in_channels, in_sample_rate, out_nb_samples) = (0, 0, 0)

        if deviceDict != None:
            thisDevice2 = deviceDict["videos"]
            if thisDevice2 != None:
                scapture = thisDevice2["capture"]["name"]
                if scapture in ["FILE"]:
                    yuvfilename = thisDevice2["yuvfilename"]
                    if yuvfilename != None:
                        devicetype = -1
            thisDevice = deviceDict["audios"]
            if thisDevice != None:
                sbitrate = thisDevice["bitrate"]["name"]
                if sbitrate not in ["default", "其他"]:
                    #sbitrate = sbitrate.replace("kbps", "")
                    bitrate = int(sbitrate)
                srecorder = thisDevice["recorder"]["name"]

                input_name = "alsa"
                if (platform.system() == 'Windows'):
                    input_name = "dshow"
                    print('Windows系统')
                elif (platform.system() == 'Linux'):
                    print('Linux系统')
                else:
                    print('其他')
                device_name = srecorder
                if srecorder not in ["default"]:
                    pass
                scodec = thisDevice["codec"]["name"]
                cchannels = thisDevice["channels"]["name"]
                channels = int(cchannels)
                caudiofmt = thisDevice["audiofmt"]["name"]
                cframesize = thisDevice["framesize"]["name"]
                framesize = int(cframesize)
                csamplerate = thisDevice["samplerate"]["name"]
                samplerate = int(csamplerate)
                in_channels = channels
                in_sample_rate = samplerate
                #audio_thread.in_sample_fmt = "AV_SAMPLE_FMT_S16"
                #audio_thread.out_nb_samples
                out_nb_samples = framesize
        #port = port + 1
        params = {}
        params.update({"actor": actor})
        params.update({"sessionId": sessionId})
        params.update({"chanId": chanId})
        params.update({"modeId": self.modeId})
        params.update({"avtype": 1})
        params.update({"selfmode": selfmode})
        #params.update({"width": self.width})
        #params.update({"height": self.height})
        params.update({"enable_netack": enable_netack})
        params.update({"testmode": 0})
        params.update({"nettime": self.nettime})
        params.update({"port": port + 1})
        params.update({"server_ip": host})
        params.update({"idx": 0})
        params.update({"win_id": self.windId})
        params.update({"isEncoder": 1})
        params.update({"taskId": self.taskId})
        #
        params.update({"yuvfilename": yuvfilename})
        params.update({"input_name": input_name})
        params.update({"device_name": device_name})
        params.update({"bitrate": bitrate})
        params.update({"mtu_size": mtu_size})
        params.update({"fecenable": fecenable})
        params.update({"buffer_shift": buffer_shift})
        params.update({"scodec": scodec})
        params.update({"denoise": denoise})
        params.update({"in_channels": in_channels})
        params.update({"in_sample_rate": in_sample_rate})
        params.update({"out_nb_samples": out_nb_samples})
        params.update({"lossrate": lossrate})

        actaskId = -1
        if yuvfilename == "":
            param_str = json2str(params)
            actaskId = loadlib.gload.lib.api_start_acapture(self.handle, param_str)
            self.vDevList.append(actaskId)
            self.taskId += 1

        params.update({"taskId": self.taskId})
        param_str = json2str(params)
        codecTaskId = loadlib.gload.lib.api_start_audio(self.handle2, param_str)
        self.taskId += 1
        return (codecTaskId, actaskId)


    def create_down_video(self, chanId, devicePos, width, height):
        print("create_down_video: chanId= ", chanId)
        (id, sessionId, actor) = (chanId, 100, DECACTOR)
        conf = self.conferenceInfo
        sessionId = conf["sessionId"]
        sup = self.general["supprams"]
        spatiallayer = int(sup["spatiallayer"]["name"])
        refs = int(sup["temporallayer"]["name"])
        osd_enable = sup["osd"]
        selfmode = sup["selfmode"]
        serverAddr = sup["serverAddr"]
        host = serverAddr.split(":")[0]
        port = int(serverAddr.split(":")[1])
        osd_enable = sup["osd"]
        enable_netack = sup["netack"]

        (bitrate, mtu_size, fec_level, buffer_shift) = self.GetParams(refs, width, height)
        params = {}
        params.update({"actor": actor})
        params.update({"sessionId": sessionId})
        params.update({"chanId": chanId})
        params.update({"modeId": self.modeId})
        params.update({"avtype": 0})
        params.update({"selfmode": selfmode})
        params.update({"enable_netack": enable_netack})
        params.update({"screen_width": self.screen_width})
        params.update({"screen_height": self.screen_height})
        params.update({"width": width})
        params.update({"height": height})
        params.update({"testmode": 0})
        params.update({"nettime": self.nettime})
        params.update({"port": port})
        params.update({"server_ip": host})
        params.update({"idx": 0})
        params.update({"win_id": self.windId})
        params.update({"isEncoder": 0})
        params.update({"taskId": self.taskId})
        params.update({"osd_enable": osd_enable})
        param_str = json2str(params)
        codecTaskId = loadlib.gload.lib.api_start_video(self.handle2, param_str)
        self.taskId += 1
        return codecTaskId

    def create_down_audio(self, chanId, devicePos):
        (id, sessionId, actor) = (chanId, 100, DECACTOR)
        conf = self.conferenceInfo
        sessionId = conf["sessionId"]
        sup = self.general["supprams"]
        selfmode = sup["selfmode"]
        enable_netack = sup["netack"]
        spatiallayer = int(sup["spatiallayer"]["name"])
        refs = int(sup["temporallayer"]["name"])
        serverAddr = sup["serverAddr"]
        host = serverAddr.split(":")[0]
        port = int(serverAddr.split(":")[1])
        ###audio
        (bitrate, mtu_size, buffer_shift) = (24000, 300, 11)
        #port = port + 1
        params = {}
        params.update({"actor": actor})
        params.update({"sessionId": sessionId})
        params.update({"chanId": chanId})
        params.update({"modeId": self.modeId})
        params.update({"avtype": 1})
        params.update({"selfmode": selfmode})
        params.update({"enable_netack": enable_netack})
        #params.update({"width": self.width})
        #params.update({"height": self.height})
        params.update({"testmode": 0})
        params.update({"nettime": self.nettime})
        params.update({"port": port + 1})
        params.update({"server_ip": host})
        params.update({"idx": 0})
        params.update({"win_id": self.windId})
        params.update({"isEncoder": 0})
        params.update({"taskId": self.taskId})
        param_str = json2str(params)
        codecTaskId = loadlib.gload.lib.api_start_audio(self.handle2, param_str)
        self.taskId += 1
        return codecTaskId

    def clients_stop(self):
        print("stop codec")
        for (taskId, devTaskId) in self.upChanList:
            loadlib.gload.lib.api_stop_task(self.handle2, taskId)
        for (taskId, devTaskId) in self.upAudioChanList:
            loadlib.gload.lib.api_stop_task(self.handle2, taskId)
        for taskId in self.downChanList:
            loadlib.gload.lib.api_stop_task(self.handle2, taskId)
        for taskId in self.downAudioChanList:
            loadlib.gload.lib.api_stop_task(self.handle2, taskId)

        for taskId in self.upStreamList:
            if taskId >= 0:
                loadlib.gload.lib.api_stop_task(self.handle2, taskId)

        #loadlib.gload.lib.api_pool_stop(self.handle2)
        #loadlib.gload.lib.api_stop_task(self.handle2, -1)

        print("stop capture and render/player")

        for taskId in self.vDevList:
            loadlib.gload.lib.api_stop_task(self.handle, taskId)
        for taskId in self.aDevList:
            loadlib.gload.lib.api_stop_task(self.handle, taskId)

        #音视频播放／渲染退出存在冲突
        print("clients_stop: self.aptaskId=", self.aptaskId)
        if self.aptaskId >= 0:
            loadlib.gload.lib.api_stop_task(self.handle, self.aptaskId)

        #time.sleep(1)
        print("clients_stop: self.vrtaskId=", self.vrtaskId)
        if self.vrtaskId >= 0:
            loadlib.gload.lib.api_stop_task(self.handle, self.vrtaskId)

        if self.mcuTaskId >= 0:
            loadlib.gload.lib.api_stop_task(self.handle, self.mcuTaskId)
        #loadlib.gload.lib.api_pool_stop(self.handle)

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
            self.windId = win_hnd

    def close_session(self):
        self.clients_stop()
        print("close_session")
        #self.set_status(0)
        #while self.get_status() != 0:
        #    time.sleep(0.1)

    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞

    def run(self):
        self.create_clients()
        self.set_status(1)
        loadlib.gload.lib.api_pool_stop(self.handle)
        loadlib.gload.lib.api_stop_task(self.handle, -1)
        ###
        #for handle in self.upStreamList:
        #    if handle != None:
        #        loadlib.gload.lib.api_avstream_close(handle)
        for handle in self.downStreamList:
            if handle != None:
                loadlib.gload.lib.api_avstream_close(handle)


        ###
        self.set_status(0)

    def GetParams(self, refs, width, height):
        #(bitrate, mtu_size, fec_level, buffer_shift)
        #buffer_shift不能过于接近seqnum的最大值,不能超过14位
        ret = (3 * 1024 * 1024, 1400, 0, 10)
        (h, w) = (width, height)
        if (w * h) <= 176 * 144:
            fec_level = 2
            if refs <= 2:
                fec_level = 2
            ret = (256 * 1024, 300, fec_level, 10)
        elif (w * h) <= 352 * 288:
            fec_level = 2
            if refs <= 2:
                fec_level = 2
            ret = (512 * 1024, 400, fec_level, 10)
        elif (w * h) <= 640 * 480:
            fec_level = 2
            if refs <= 2:
                fec_level = 2
            ret = (800 * 1024, 600, fec_level, 11)
        elif (w * h) <= 704 * 576:
            fec_level = 2
            if refs <= 2:
                fec_level = 2
            ret = (1200 * 1024, 600, fec_level, 12)
        elif (w * h) <= 960 * 540:
            fec_level = 2
            if refs <= 2:
                fec_level = 2
            ret = (1500 * 1024, 720, fec_level, 12)
        elif (w * h) <= 1280 * 720:
            fec_level = 2
            if refs <= 2:
                fec_level = 2
            ret = (int(2.0 * 1024 * 1024), 800, fec_level, 13)
        elif (w * h) <= 1920 * 1080:
            fec_level = 2
            if refs <= 2:
                fec_level = 2
            ret = (3.0 * 1024 * 1024, 1100, fec_level, 14)
        else:
            fec_level = 2
            ret = (6 * 1024 * 1024, 1400, fec_level, 14)
        return ret

if __name__ == "__main__":
    print("start avclient")

    print("end avclient")

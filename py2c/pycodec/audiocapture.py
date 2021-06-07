# -*- coding: utf-8 -*


import sys
import os
from ctypes import *
import json
import loadlib
import threading
import time

def json2str(jsonobj):
    if sys.version_info >= (3, 0):
        json_str = json.dumps(jsonobj, ensure_ascii=False, sort_keys=False).encode('utf-8')
    else:
        json_str = json.dumps(jsonobj, encoding='utf-8', ensure_ascii=False, sort_keys=False)

    return json_str
class AudioCapture(threading.Thread):
    def __init__(self, id):
        threading.Thread.__init__(self)
        self.id = id
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
        self.load = loadlib.gload
        self.status = True
        ###
        self.input_name = "alsa"
        self.device_name = "default" #"plughw:0,0" #"hw"
        #self.device_name = "pulse"
        #self.device_name = "plughw:0,0"
        #self.device_name = "hw:0,0"
        #self.device_name = "hw:1,0" #card 1: ..., device 0
        #self.device_name = "hw: 0" #card 0
        #self.device_name = "hw: 1" # card 1
        #self.device_name = "/dev/snd/pcmC0D0c"

        #self.input_name = "sndio"
        #self.device_name = "/dev/audio0"
        ###
        self.handle_size = 8
        self.handle = create_string_buffer(self.handle_size)

        self.in_channels = 2
        self.in_sample_rate = 48000
        #self.in_sample_rate = 44100 #重采样细节处理尚未优化
        self.in_sample_fmt = "AV_SAMPLE_FMT_S16"

        self.filename = "/home/gxh/works/audio/dukou_ref3.wav"  # #20063.wav
        # self.filename = ""
        self.filename = "./dukou_ref3.wav"
        #self.filename = ""

        self.pcmfile = "/home/gxh/works/cap_resample_" + str(id) + ".pcm"
        self.pcmfile = "./acap_resample_" + str(id) + ".pcm"
        self.pcmfile = ""
        self.capfile = "/home/gxh/works/cap_" + str(id) + ".pcm"
        self.capfile = "./acap" + str(id) + ".pcm"
        self.capfile = ""
        self.out_channels = 2
        self.out_nb_samples = 1024
        self.out_sample_fmt = "AV_SAMPLE_FMT_S16"  # 2Bytes/sample
        self.out_sample_rate = 48000
        self.out_channel_layout = "AV_CH_LAYOUT_STEREO"
        self.frame_size = self.out_channels * 2 * self.out_nb_samples
        self.out_buffer_size = self.frame_size
        self.max_buf_num = 8
        self.datatype = 2
        self.process = 1#0#1
        self.select_device = -1
        self.param = {}
        self.outbuf = None

    def __del__(self):
        print("AudioCapture del")
        if self.param != None:
            del self.param
        if self.outbuf != None:
            del self.outbuf
        if self.handle != None:
            del self.handle
    def get_device_info(self):
        infolist = []
        cmd = b"select_audio_recorder"
        outbuf2 = create_string_buffer(10240)
        ret = self.load.lib.api_get_dev_info(cmd, outbuf2)
        if ret <= 0:
            print("AudioCapture: get_device_info: ret=", ret)
            return infolist
        data = outbuf2.raw
        data2 = data[0:ret]
        result = None
        try:
            if sys.version_info >= (3, 0):
                #data2 = data2.replace(b"\'", b"\"")
                print("data2= ", data2)
                #在json字符串中不能出现单引号
                #result = json.loads(data2, encoding='utf-8')
                result = json.loads(data2.decode())
            else:
                result = json.loads(data2)
        except:
            print("audio: get_device_info: failed !")
            return infolist
        if result != None:
            windev = result.get("windev")
            if windev != None:
                audiodev = windev.get("audiodev")
                if audiodev != None:
                    return audiodev
            for key, value in result.items():
                if ("card" in key) and ("device" in value):
                    key = key.replace("card", "")
                    value = value.replace("device", "")
                    key = key.replace(" ", "")
                    value = value.replace(" ", "")
                    infolist.append("hw:" + key + "," + value)
        print("AudioCapture: get_device_info: infolist=", infolist)
        return infolist
    def get_device(self, id):
        infolist = self.get_device_info()
        if len(infolist) > 0:
            if id >= 0 and len(infolist) >= id:
                self.device_name = str(infolist[id])
                print("get_device: self.device_name= ", self.device_name)
    def init(self):
        #说明："pulse"或"default"（安装了pulse时）会通过pulse进行音频采集，否则启用的是asla，音质较差，不建议选择设备；
        #实际测试发现，pulse有自动切换麦克风的功能（在拔插麦克风情况下）;
        if (self.select_device >= 0):
            self.get_device(self.select_device)
        self.param = {}
        self.param.update({"input_name": self.input_name})
        self.param.update({"device_name": self.device_name})
        self.param.update({"in_channels": self.in_channels})
        self.param.update({"in_sample_rate": self.in_sample_rate})
        self.param.update({"in_sample_fmt": self.in_sample_fmt})
        self.param.update({"out_channels": self.out_channels})
        self.param.update({"out_nb_samples": self.out_nb_samples})
        self.param.update({"out_sample_fmt": self.out_sample_fmt})
        self.param.update({"out_sample_rate": self.out_sample_rate})
        self.param.update({"out_channel_layout": self.out_channel_layout})
        self.param.update({"out_buffer_size": self.out_buffer_size})
        self.param.update({"frame_size": self.frame_size})
        self.param.update({"max_buf_num": self.max_buf_num})
        self.param.update({"filename": self.filename})
        self.param.update({"pcmfile": self.pcmfile})
        self.param.update({"capfile": self.capfile})
        self.param.update({"datatype": self.datatype})
        self.param.update({"process": self.process})
        self.param.update({"print": 0})
        param_str = json2str(self.param)
        ret = self.load.lib.api_audio_capture_init(self.handle, param_str)
        # print("init: ret= ", ret)
        if ret >= 0:
            self.status = True
        else:
            self.status = False
            print("AudioCapture:init: fail !!!")
        self.outbuf = create_string_buffer(self.frame_size << 1)  # redundancy
        print("AudioCapture:init: over ")
    def run(self):
        print("AudioCapture: run 0")
        while self.__running.isSet():
            self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            ret = 0
            if self.status:
                # ret = self.load.lib.api_audio_capture_read_frame(self.handle, 0)
                ret = self.load.lib.api_audio_capture_read_frame(self.handle)
                if ret > 0:
                    #print("VideoCapture: run: ret= ", ret)
                    pass
                else:
                    time.sleep(0.01)
            else:
                # print("VideoCapture: run: ret= ", ret)
                # self.stop()
                # break
                time.sleep(0.01)
        self.Close()
        self.stop()

    def ReadFrame(self):
        if not self.status:
            return ("", -1)
        data = ""
        ret = self.load.lib.api_audio_capture_read_frame2(self.handle, self.outbuf)
        if ret > 0:
            data = self.outbuf
            #print("ReadFrame: run: ret= ", ret)
        return (data, ret)

    def Close(self):
        self.load.lib.api_audio_capture_close(self.handle)
        self.status = False
        print("AudioCapture: Close over")
        return

    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals


    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞


    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞


if __name__ == '__main__':
    print('Start AudioCapture.')
    call = AudioCapture()
    call.init()
    call.start()
    # time.sleep(2)
    idx = 0
    while idx >= 0 and idx < 4:
        try:
            idx = int(raw_input('please input to exit(eg: 0 ): '))
        except:
            idx = int(input('please input to exit(eg: 0 ): '))
        print("idx= ", idx)
    call.stop()
    print('End AudioCapture.')

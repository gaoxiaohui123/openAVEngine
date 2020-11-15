# -*- coding: utf-8 -*


import sys
import os
import gc
from ctypes import *
import json
import loadlib
import threading
import time
from postprocess import PostProcess

def json2str(jsonobj):
    if sys.version_info >= (3, 0):
        json_str = json.dumps(jsonobj, ensure_ascii=False, sort_keys=False).encode('utf-8')
    else:
        json_str = json.dumps(jsonobj, encoding='utf-8', ensure_ascii=False, sort_keys=False)

    return json_str
class VideoCapture(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
        self.load = loadlib.gload
        self.status = True
        ###
        # self.input_name = "x11grab"
        # self.device_name = ":0.0"
        self.input_name = "v4l2"
        # self.input_name = "video4linux2"
        self.device_name = "/dev/video0"
        self.load.lib.api_list_devices(self.input_name, self.device_name)
        ###
        self.handle_size = 8
        self.handle = create_string_buffer(self.handle_size)
        (self.width, self.height) = (loadlib.WIDTH, loadlib.HEIGHT)
        (self.cap_width, self.cap_height) = (loadlib.WIDTH, loadlib.HEIGHT)  # (1280, 720)
        self.framerate = 25
        self.input_format = "mjpeg"
        #self.input_format = "raw"
        self.max_buf_num = 4  # 8#3
        self.ratio = 3  # 1
        self.denoise = 2#0#2  # 1#0#1#0#1
        self.select_device = 1#1#0#-1  # 2#1#0#1#-1
        self.osd = 1
        self.orgX = 0
        self.orgY = 0
        self.scale = 0
        self.color = 1
        self.process = None
        # self.process = PostProcess(1)
        # self.init()
        self.param = {}
        self.outbuf = None
    def __del__(self):
        print("VideoCapture del")
        if self.param != None:
            del self.param
        if self.outbuf != None:
            del self.outbuf
        if self.handle != None:
            del self.handle
        del self.load
        gc.collect()
    def get_device_info(self):
        infolist = []
        cmd = b"select_video_capture"
        outbuf2 = create_string_buffer(10240)
        ret = self.load.lib.api_get_dev_info(cmd, outbuf2)
        #print("get_device_info: ret=", ret)
        if ret > 0:
            data = outbuf2.raw
            data2 = data[0:ret]
            result = None
            if sys.version_info >= (3, 0):
                print("data2= ", data2)
                # 在json字符串中不能出现单引号
                result = json.loads(data2.decode())
            else:
                result = json.loads(data2)
            if result != None:
                print("get_device_info: result=", result)
                # for key, value in result.items():
                for key in result.keys():
                    # print("key=", key)
                    value = result[key]
                    valuelist = []
                    if isinstance(value, dict):
                        keys = value.keys()
                        # print("keys=", keys)
                        for key2 in keys:
                            value2 = value[key2]
                            rlist2 = []
                            rlist = value2.split(' ')
                            for thisdata in rlist:
                                if "x" in thisdata:
                                    items = thisdata.split('x')
                                    value2 = (int(items[0]), int(items[1]))
                                    rlist2.append(value2)

                            key2 = key2.replace(" ", "")
                            valuelist.append((key2, rlist2))
                    else:
                        value = value.replace(" ", "")
                        items = value.split('x')
                        value2 = (int(items[0]), int(items[1]))
                        valuelist.append(value2)

                    key = key.replace(" ", "")
                    infolist.append((key, valuelist))
                if self.select_device < 0:
                    for item in infolist:
                        if "video" in item[0]:
                            if len(item[1]) > 1:
                                for item2 in item[1]:
                                    if "raw" in item2[0]:
                                        j = item[1].index(item2)
                                        del (item[1][j]) #优先选择mjpeg
        print("infolist=", infolist)
        return infolist

    def get_device(self, id):
        infolist = self.get_device_info()
        compress = self.input_format
        if infolist != None:
            if len(infolist) > 0:
                if id >= 0 and len(infolist) >= id:
                    item = infolist[id]
                    print("get_device: len(infolist)=", len(infolist))
                    print("get_device: item[0]=", str(item[0]))
                    if "video" in str(item[0]):
                        self.device_name = "/dev/" + str(item[0])
                        for item2 in item[1]:
                            print("(compress, item2[0])=", (compress, str(item2[0])))
                            if compress in str(item2[0]):
                                if "raw" in str(item2[0]):
                                    self.input_format = "raw"
                                    print("get_device: self.device_name=", self.device_name)
                                    print("get_device: self.input_format=", self.input_format)
                                    return
                                else:
                                    self.input_format = str(item2[0])
                                    print("get_device: self.device_name=", self.device_name)
                                    print("get_device: self.input_format=", self.input_format)
                                    return
                    elif "fb" in str(item[0]):
                        self.input_name = "x11grab"
                        self.device_name = ":0.0"
                        # if "x11grab" in self.device_name:
                        if True:
                            width = item[1][0][0]
                            height = item[1][0][1]
                            if width > 0 and height > 0:
                                self.cap_width = width
                                self.cap_height = height
                            print("get_device: self.device_name=", self.device_name)
                            return

                for item in infolist:
                    if "video" in str(item[0]):
                        if "video" in self.device_name:
                            self.device_name = "/dev/" + str(item[0])
                            for item2 in item[1]:
                                if "raw" in item2[0]:
                                    self.input_format = "raw"
                                else:
                                    self.input_format = str(item2[0])
                            break
                    elif "fb" in str(item[0]):
                        if "x11grab" in self.device_name:
                            width = item[1][0][0]
                            height = item[1][0][1]
                            if width > 0 and height > 0:
                                self.cap_width = width
                                self.cap_height = height
                            break
        return

    def init(self):
        print("VideoCapture: init: self.select_device= ", self.select_device)
        print("VideoCapture: init: self.input_name= ", self.input_name)
        print("VideoCapture: init: self.device_name= ", self.device_name)
        print("VideoCapture: init: self.input_format= ", self.input_format)
        # self.load.lib.api_list_devices(self.input_name, self.device_name)
        # if self.select_device >= 0:
        if self.select_device >= 0:
            self.get_device(self.select_device)
        self.param = {}
        # self.param.update({"input_name": "video4linux2"}) #x11grab #v4l2
        self.param.update({"input_name": self.input_name})
        self.param.update({"device_name": self.device_name})
        # self.param.update({"input_name": "x11grab"})
        # self.param.update({"device_name": ":0.0"}) #
        if self.input_name not in ["v4l2", "video4linux2"]:
            self.input_format = "raw"
            self.denoise = 0
        self.param.update({"input_format": self.input_format})
        self.param.update({"pixformat": "AV_PIX_FMT_YUV420P"})
        self.param.update({"scale_type": "SWS_BILINEAR"})  # SWS_BICUBIC
        self.param.update({"cap_width": self.cap_width})
        self.param.update({"cap_height": self.cap_height})
        self.param.update({"framerate": self.framerate})
        self.param.update({"max_buf_num": self.max_buf_num})
        self.param.update({"width": self.width})
        self.param.update({"height": self.height})
        self.param.update({"ratio": self.ratio})
        self.param.update({"denoise": self.denoise})
        self.param.update({"osd": self.osd})
        self.param.update({"orgX": self.orgX})
        self.param.update({"orgY": self.orgY})
        self.param.update({"scale": self.scale})
        self.param.update({"color": self.color})
        self.param.update({"print": 0})
        param_str = json2str(self.param)
        ret = self.load.lib.api_capture_init(self.handle, param_str)
        if ret >= 0:
            self.status = True
        else:
            self.status = False

        self.frame_size = int((self.width * self.height * 3) / 2)
        self.outbuf = create_string_buffer(self.frame_size)

    def run(self):
        print("VideoCapture: run 0")
        while self.__running.isSet():
            self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            ret = self.load.lib.api_capture_read_frame(self.handle, 0)
            if ret > 0:
                pass
            else:
                # print("VideoCapture: run: ret= ", ret)
                # self.stop()
                # break
                time.sleep(0.01)
        self.Close()

    def ReadFrame(self):
        data = ""
        ret = self.load.lib.api_capture_read_frame2(self.handle, self.outbuf)
        #print("ReadFrame: ret=", ret)
        if ret > 0:
            data = self.outbuf
            if self.process != None:
                (data2, w2, h2) = self.process.denoise(data, self.cap_width, self.cap_height)
                print("ReadFrame: len(data)=", len(data))
                size = (w2 * h2 * 3) >> 1
                self.outbuf[0:size] = data2[0:size]
        return data

    def Close(self):
        self.load.lib.api_capture_close(self.handle)
        self.status = False
        return

    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞


if __name__ == '__main__':
    print('Start pycall.')

    print('End pycall.')

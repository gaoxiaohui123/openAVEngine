# -*- coding: utf-8 -*


import sys
import os
import gc
from ctypes import *
import json
import loadlib
import threading
import time
from capture import VideoCapture

DISTILL_LIST = [0, 1]

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
    #outjson = None
    try:
        outjson = json.loads(json_str, encoding='utf-8')
    except:
        print("error: python version")
        try:
            outjson = json.loads(json_str)
        except:
            print("not json")
        else:
            json_str2 = json2str(outjson)
            # print("1: json_str2= ", json_str2)
    else:
        json_str2 = json2str(outjson)
        # print("0: json_str2= ", json_str2)
    return outjson
class MySDL(threading.Thread):
    def __init__(self, id, width, height):
        threading.Thread.__init__(self)
        self.lock = threading.Lock()
        self.load = loadlib.gload
        self.id = id
        self.status = True
        self.width = width
        self.height = height
        self.screen_width = width
        self.screen_height = height
        self.osd = 1
        self.orgX = 0
        self.orgY = 0
        self.scale = 0
        self.color = 4
        self.showlist = []
        self.showmap = {}
        self.layerlist = []
        self.shownum = 0
        self.stacklist = []
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True

        self.handle_size = 8
        self.handle = create_string_buffer(self.handle_size)
        self.param = {}
        self.outbuf = None
    def __del__(self):
        print("MySDL del")
        if self.param != None:
            del self.param
        if self.outbuf != None:
            del self.outbuf
        if self.handle != None:
            del self.handle
        del self.load
        del self.lock
        gc.collect()
    def reset(self):
        self.showlist = []
        self.showmap = {}
        self.layerlist = []
        self.shownum = 0
        self.stacklist = []
    def init(self):
        self.param = {}
        self.param.update({"name": "decode"})
        self.param.update({"screen_w": self.screen_width})
        self.param.update({"screen_h": self.screen_height})
        #self.param.update({"pixel_w": loadlib.WIDTH})
        #self.param.update({"pixel_h": loadlib.HEIGHT})
        self.param.update({"pixel_w": self.width})
        self.param.update({"pixel_h": self.height})
        # self.param.update({"pixel_w": 320})
        # self.param.update({"pixel_h": 240})
        self.param.update({"bpp": 12})
        self.param.update({"wait_time": 0})
        self.param.update({"pixformat": "SDL_PIXELFORMAT_IYUV"})
        self.param.update({"win_id": self.id})
        self.param.update({"osd": self.osd})
        self.param.update({"orgX": self.orgX})
        self.param.update({"orgY": self.orgY})
        self.param.update({"scale": self.scale})
        self.param.update({"color": self.color})
        self.param.update({"print": 0})

        param_str = json2str(self.param)
        ret = self.load.lib.api_sdl_init(self.handle, param_str)
    def sdl_status(self):
        return self.load.lib.api_sdl_status(self.handle)

    def is_mux(self, a, b):
        (x1, y1, w2, h2) = a
        (x3, y3, w4, h4) = b
        (x1, y1, x2, y2) = (x1, y1, x1 + w2, y1 + h2)
        (x3, y3, x4, y4) = (x3, y3, x3 + w4, y3 + h4)

        # sum_area = 0
        minx = x1 if x1 > x3 else x3  # max(x1,x3)
        miny = y1 if y1 > y3 else y3  # max(y1,y3)
        maxx = x2 if x2 < x4 else x4  # min(x2,x4)
        maxy = y2 if y2 < y4 else y4  # min(y2,y4)

        # 最小的中的最大的还比最大的中的最小的要小就相离，否则相交
        if ((maxx < minx) or (maxy < miny)):
            # 要判断是否相离，如果相离直接输出两个矩形面积的和
            return 0
        t0 = abs((x1 - x2) * (y1 - y2))
        t1 = abs((x3 - x4) * (y3 - y4))
        t = t0 + t1
        s = abs(t)  # 矩形相交的最大面积，也是矩形相离的面积
        s0 = w2 * h2
        s1 = w4 * h4
        rat0 = float(s0) / s
        rat1 = float(s1) / s
        ret = 1 + (rat0 > rat1)
        return ret

    def renew_layer(self, idx):
        rect0 = self.showlist[idx]
        for key, value in self.showmap.items():
            if key != idx:
                # print("renew_layer: idx= ", key)
                rect1 = self.showlist[key]
                # print("renew_layer: rect1= ", rect1)
                mux = self.is_mux(rect0, rect1)
                # print("renew_layer: mux= ", mux)
                if mux == 2:
                    # 存在容易被遮挡的层
                    self.layerlist.append((idx, key)) # （遮挡， 被遮挡）
                    # 被遮挡的层是否也存在被其遮挡的层
                    self.renew_layer(key)
        # return self.layerlist

    def mult_layer_refresh(self):
        refreshlist = []
        i = 0
        for (idx0, idx1) in self.layerlist:
            if idx0 not in refreshlist: #先刷新遮挡
                rect = self.showlist[idx0]
                data = self.showmap[idx0]["data"]
                param = {}
                param.update({"rect_x": rect[0]})
                param.update({"rect_y": rect[1]})
                param.update({"rect_w": rect[2]})
                param.update({"rect_h": rect[3]})
                show_flag = 0
                param.update({"show_flag": show_flag})
                param_str = json2str(param)
                #self.load.lib.api_split_screen(self.handle, data, param_str, loadlib.WIDTH)
                self.load.lib.api_split_screen(self.handle, data, param_str, self.width)
                refreshlist.append(idx0)
                # print("mult_layer_refresh: 0: rect= ", rect)
            if idx1 not in refreshlist: #后刷新被遮挡
                show_flag = 0
                if i == (len(self.layerlist) - 1):
                    show_flag = 1
                rect = self.showlist[idx1]
                data = self.showmap[idx1]["data"]
                param = {}
                param.update({"rect_x": rect[0]})
                param.update({"rect_y": rect[1]})
                param.update({"rect_w": rect[2]})
                param.update({"rect_h": rect[3]})
                param.update({"show_flag": show_flag})
                param_str = json2str(param)
                #self.load.lib.api_split_screen(self.handle, data, param_str, loadlib.WIDTH)
                self.load.lib.api_split_screen(self.handle, data, param_str, self.width)
                refreshlist.append(idx1)
                # print("mult_layer_refresh: 1: rect= ", rect)
            i += 1

    def sdl_refresh(self, data, rect, show_flag):
        if self.handle == 0:
            return False
        # self.load.lib.api_sdl_clear(self.handle)
        ##self.param = {}
        self.param.update({"rect_x": rect[0]})
        self.param.update({"rect_y": rect[1]})
        self.param.update({"rect_w": rect[2]})
        self.param.update({"rect_h": rect[3]})
        #show_flag = 0 #注意：不合理的渲染，会导致“倒帧”；
        self.param.update({"show_flag": show_flag})
        param_str = json2str(self.param)
        self.lock.acquire()
        idx = -1
        if rect not in self.showlist:
            self.showlist.append(rect)
            idx = self.showlist.index(rect)
        else:
            idx = self.showlist.index(rect)
            # print("sdl_refresh: rect in sdl_refresh: idx= ", idx)
            pass
        # self.showmap[idx] = data
        data2 = self.showmap.get(idx)
        if data2 == None:
            # print("sdl_refresh: data2 is None")
            data2 = {}
            data2.update({"data": data})
            data2.update({"layer": 0})
            data2.update({"time": time.time()})
            self.showmap[idx] = data2
        else:
            self.showmap[idx]["data"] = data
            self.showmap[idx]["time"] = time.time()
        self.layerlist = []
        self.renew_layer(idx)
        if len(self.layerlist) > 0:
            # print("sdl_refresh: self.layerlist= ", self.layerlist)
            self.mult_layer_refresh()
        else:
            # print("sdl_refresh: rect= ", rect)
            #self.load.lib.api_split_screen(self.handle, data, param_str, loadlib.WIDTH)
            n = len(self.showlist)
            if n > 1:
                if (self.shownum % n) == (n - 1) and False:
                    self.param.update({"show_flag": 1})
                    param_str = json2str(self.param)
            if idx in self.stacklist or len(self.stacklist) == n:
                # 注意：同一区域，同时推入多帧，可能会导致显示乱序，视觉上出现“倒帧”
                self.param.update({"show_flag": 1})
                param_str = json2str(self.param)
                self.stacklist = []
            else:
                self.stacklist.append(idx)
            self.load.lib.api_split_screen(self.handle, data, param_str, self.width)
            #print("sdl_refresh: param_str= ", param_str)
            #print("sdl_refresh: show_flag= ", show_flag)
            #print("sdl_refresh: len(self.showlist)= ", len(self.showlist))
            self.shownum += 1
        self.lock.release()
        return True
    def clean_screen(self,data, rect, show_flag):
        #self.param = {}
        self.param.update({"rect_x": rect[0]})
        self.param.update({"rect_y": rect[1]})
        self.param.update({"rect_w": rect[2]})
        self.param.update({"rect_h": rect[3]})
        # show_flag = 0 #注意：不合理的渲染，会导致“倒帧”；
        self.param.update({"show_flag": show_flag})
        self.param.update({"osd": 0})
        param_str = json2str(self.param)
        self.lock.acquire()
        self.load.lib.api_split_screen(self.handle, data, param_str, self.width)
        self.param.update({"osd": self.osd})
        self.lock.release()
    def sdl_refresh2(self, data, rect, show_flag):
        if self.handle == 0:
            return False
        # self.load.lib.api_sdl_clear(self.handle)
        self.param = {}
        self.param.update({"rect_x": rect[0]})
        self.param.update({"rect_y": rect[1]})
        self.param.update({"rect_w": rect[2]})
        self.param.update({"rect_h": rect[3]})
        self.param.update({"show_flag": show_flag})
        param_str = json2str(self.param)
        self.lock.acquire()
        print("sdl_refresh_0")
        #self.load.lib.api_split_screen(self.handle, data, param_str, loadlib.WIDTH)
        self.load.lib.api_split_screen(self.handle, data, param_str, self.width)
        self.lock.release()
        return True

    def sdl_test(self, show_buffer, width, height, delay):
        self.load.lib.api_test_sdl(show_buffer, width, height, delay)

    def sdl_push_event(self, id):
        self.load.lib.api_sdl_push_event(id)

    def sdl_stop(self):
        self.load.lib.api_sdl_stop(self.handle)

    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞

    def sdl_show_run(self):
        self.load.lib.api_sdl_show_run(self.handle)
        self.load.lib.api_sdl_close(self.handle)
        print("sdl_show_run: over")

    def run(self):
        print("MySDL: run 0")
        self.status = True
        while self.__running.isSet():
            self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            self.load.lib.api_sdl_show_run(self.handle)
            self.stop()
            print("MySDL: run stop")
        self.status = False
        self.load.lib.api_sdl_close(self.handle)
        print("MySDL: run: over")

class ReadFrame(threading.Thread):
    def __init__(self, id):
        threading.Thread.__init__(self)
        self.load = loadlib.gload
        self.id = id
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
        self.yuvfilename = loadlib.yuvfilename
        self.input_name = "v4l2"
        self.device_name = "/dev/video0"
        self.input_format = "mjpeg"
        self.denoise = 2
        self.devicetype = 3#2#1
        self.select_device = -1
        self.yuvfilename = ""
        self.width = 0
        self.height = 0
        self.frame_rate = 25
        self.frame_size = int((self.width * self.height * 3) / 2)
        self.imglist = []
        self.img_num = 0
        self.sdl = None
        self.cap = None
    def __del__(self):
        print("ReadFrame del")
        if self.sdl != None:
            del self.sdl
        if self.cap != None:
            del self.cap
        if len(self.imglist) > 0:
            for i in range(len(self.imglist)):
                del self.imglist[0]
            del self.imglist
        del self.load
        gc.collect()
    def init(self, screen_width, screen_height, cap_width, cap_height):
        self.sdl = MySDL(self.id, screen_width, screen_height)
        (self.sdl.width, self.sdl.height) = (cap_width, cap_height)
        (self.width, self.height) = (cap_width, cap_height)
        self.frame_size = int((self.width * self.height * 3) / 2)
        self.sdl.init()

        # self.cap.input_name = "x11grab"
        # self.cap.device_name = ":0.0"
        if self.devicetype > 0:
            self.cap = VideoCapture()
            if self.devicetype == 1:
                self.cap.input_name = "v4l2"
                self.cap.device_name = "/dev/video0"
                self.cap.input_format = "mjpeg"
                self.cap.select_device = self.select_device
            elif self.devicetype == 2:
                self.cap.input_name = "v4l2"
                self.cap.device_name = "/dev/video0"
                self.cap.input_format = "raw"
                self.cap.select_device = self.select_device
            elif self.devicetype == 3:
                self.cap.input_name = "x11grab"
                self.cap.device_name = ":0.0"
                self.cap.input_format = "raw"
                self.cap.select_device = self.select_device
            else:
                self.cap.input_name = self.input_name
                self.cap.device_name = self.device_name
                self.cap.input_format = self.input_format
                self.cap.denoise = self.denoise
                self.cap.select_device = -1
            self.cap.framerate = 0  # 15#25#0

            (self.cap.cap_width, self.cap.cap_height) = (cap_width, cap_height)
            (self.cap.width, self.cap.height) = (cap_width, cap_height)
            self.cap.init()
            self.cap.setDaemon(True)
            self.cap.start()
        if self.cap == None:  # or self.capture.status == False:
            filename1 = loadlib.yuvfilename
            if self.yuvfilename != "":
                filename1 = self.yuvfilename
            print("filename1= ", filename1)
            self.fp = open(filename1, 'rb')
            #
            i = 0
            if self.fp:
                while True:
                    data = self.fp.read(self.frame_size)
                    if len(data) == self.frame_size:
                        if (i & 1) in DISTILL_LIST:
                            self.imglist.append(data)
                            self.img_num += 1
                        i += 1
                    else:
                        break
        # time.sleep(1)
        self.sdl.setDaemon(True)
        self.sdl.start()
    def init2(self):
        self.sdl = MySDL(self.id, 1920, 1080)
        self.sdl.init()
        self.cap = VideoCapture()
        # self.cap.input_name = "x11grab"
        # self.cap.device_name = ":0.0"
        devicetype = 1  # 2#1
        if devicetype == 1:
            self.cap.input_name = "v4l2"
            self.cap.device_name = "/dev/video0"
        else:
            self.cap.input_name = "x11grab"
            self.cap.device_name = ":0.0"

        self.cap.framerate = 0  # 15#25#0

        self.cap.init()
        self.cap.start()
    def sdl_show_run(self):
        self.sdl.sdl_show_run()

    def sdl_push_event(self, id):
        self.sdl.sdl_push_event(id)

    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞

    def run(self):
        print("ReadFrame: run 0")
        interval = 1000.0 / float(self.frame_rate)
        start_time = 0
        renew_interval = 2000
        frame_idx = 0
        misscnt = 0
        while self.__running.isSet():
            self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            now_time = time.time()
            if start_time == 0:
                start_time = now_time
            ret = 0
            if self.cap != None:
                data = self.cap.ReadFrame()
                ret = len(data)
            elif self.img_num:
                img_idx = frame_idx % self.img_num
                img_floor = int(frame_idx / self.img_num)
                if (img_floor & 1) == 1:
                    img_idx = self.img_num - 1 - img_idx
                data = self.imglist[img_idx]
                ret = len(data)
            #print("ReadFrame: cach miss: ret= ", ret)
            if ret > 0:
                misscnt = 0
                # print("ReadFrame: ret= ", ret)
                if frame_idx % 2 in [0, 1]:
                    rect = (0, 0, self.width, self.height)
                else:
                    rect = (0, 0, self.width >> 1, self.height >> 1)
                ret = self.sdl.sdl_refresh(data, rect, 1)
                if not ret:
                    print("ReadFrame: cach miss :(ret, misscnt)= ", (ret, misscnt))
                    misscnt += 1
                    # self.load.lib.api_capture_close(self.handle)
                    self.cap.stop()
                    self.stop()
                    break

                # time.sleep(0.04)
                # time.sleep(0.1)
                # continue
            else:
                # print("ReadFrame: cach miss :misscnt= ", misscnt)
                if self.cap != None:
                    if self.cap.status == False or self.sdl.status == False:
                        print("ReadFrame: stop")
                        self.stop()
                        break
                misscnt += 1
                # print("ReadFrame: run stop")
                # self.load.lib.api_capture_close(self.handle)
                # self.cap.stop()
                # self.stop()
                # break
                time.sleep(0.01)
            now_time = time.time()

            difftime = (int)((time.time() - start_time) * 1000)

            cap_time = start_time * 1000 + frame_idx * interval
            wait_time = cap_time - now_time * 1000
            wait_time = float(wait_time) / 1000
            wait_time = wait_time if wait_time > 0.001 else 0.001
            if self.devicetype <= 0:
                time.sleep(wait_time)
            if difftime >= renew_interval and False:
                start_time = now_time
                frame_idx = 0
            frame_idx += 1
            continue

        print("ReadFrame: run exit")
        self.stop()
        #self.load.lib.api_capture_close(self.handle)
        if self.cap != None:
            self.cap.stop()
        print("ReadFrame: run: self.cap.stop ok")
        self.sdl.sdl_stop()
        print("ReadFrame: run: self.sdl.sdl_stop ok")
        self.sdl.stop()
        print("ReadFrame: run: self.sdl.stop ok")
        #self.stop()
        #del self.cap
        #del self.sdl
        print("ReadFrame: run: over")

if __name__ == '__main__':
    print('Start pycall.')
    call = ReadFrame(0)
    #(width, height) = (1280, 720)
    (width, height) = (1920, 1080)
    call.init(width, height, width, height)
    call.start()
    #time.sleep(2)
    idx = 0
    while idx >= 0 and idx < 4:
        try:
            idx = int(raw_input('please input to exit(eg: 0 ): '))
        except:
            idx = int(input('please input to exit(eg: 0 ): '))
        print("idx= ", idx)
    call.stop()
    for i in range(10):
        print("test: wait exit: i=", i)
        time.sleep(1)
    print('End pycall.')

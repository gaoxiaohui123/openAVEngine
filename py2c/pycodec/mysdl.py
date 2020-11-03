# -*- coding: utf-8 -*


import sys
import os
from ctypes import *
import json
import loadlib
import threading
import time
from capture import VideoCapture

class MySDL(threading.Thread):
    def __init__(self, id, width, height):
        threading.Thread.__init__(self)
        self.lock = threading.Lock()
        self.load = loadlib.gload
        self.id = id
        self.status = True
        self.width = width
        self.height = height
        self.osd = 1
        self.orgX = 0
        self.orgY = 0
        self.scale = 0
        self.color = 4
        self.showlist = []
        self.showmap = {}
        self.layerlist = []
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True

        self.handle_size = 8
        self.handle = create_string_buffer(self.handle_size)

    def init(self):
        self.param = {}
        self.param.update({"name": "decode"})
        self.param.update({"screen_w": self.width})
        self.param.update({"screen_h": self.height})
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

        param_str = json.dumps(self.param, encoding='utf-8', ensure_ascii=False, sort_keys=True)
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
                param_str = json.dumps(param, encoding='utf-8', ensure_ascii=False, sort_keys=True)
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
                param_str = json.dumps(param, encoding='utf-8', ensure_ascii=False, sort_keys=True)
                #self.load.lib.api_split_screen(self.handle, data, param_str, loadlib.WIDTH)
                self.load.lib.api_split_screen(self.handle, data, param_str, self.width)
                refreshlist.append(idx1)
                # print("mult_layer_refresh: 1: rect= ", rect)
            i += 1

    def sdl_refresh(self, data, rect, show_flag):
        if self.handle == 0:
            return False
        # self.load.lib.api_sdl_clear(self.handle)
        self.param = {}
        self.param.update({"rect_x": rect[0]})
        self.param.update({"rect_y": rect[1]})
        self.param.update({"rect_w": rect[2]})
        self.param.update({"rect_h": rect[3]})
        #show_flag = 0 #注意：不合理的渲染，会导致“倒帧”；
        self.param.update({"show_flag": show_flag})
        param_str = json.dumps(self.param, encoding='utf-8', ensure_ascii=False, sort_keys=True)
        self.lock.acquire()
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
            self.load.lib.api_split_screen(self.handle, data, param_str, self.width)
        self.lock.release()
        return True

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
        param_str = json.dumps(self.param, encoding='utf-8', ensure_ascii=False, sort_keys=True)
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
        priint("sdl_show_run: over")

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

    def init(self, width, height):
        self.sdl = MySDL(self.id, width, height)
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

        (self.cap.width, self.cap.height) = (width, height)
        (self.cap.cap_width, self.cap.cap_height) = (self.cap.width, self.cap.height)
        self.cap.init()
        self.cap.start()
        # time.sleep(1)
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
        frame_idx = 0
        misscnt = 0
        while self.__running.isSet():
            self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            if True:
                #ret = self.load.lib.api_capture_read_frame(self.handle, self.outbuf)
                #ret = self.load.lib.api_capture_read_frame(self.handle, 0)
                data = self.cap.ReadFrame()
                ret = len(data)
                if ret > 0:
                    misscnt = 0
                    #print("ReadFrame: ret= ", ret)
                    if frame_idx % 2 in [0 ,1]:
                        rect = (0, 0, self.cap.width, self.cap.height)
                    else:
                        rect = (0, 0, self.cap.width >> 1, self.cap.height >> 1)
                    ret = self.sdl.sdl_refresh(data, rect, 1)
                    if not ret:
                        print("ReadFrame: cach miss :(ret, misscnt)= ", (ret, misscnt))
                        misscnt += 1
                        #self.load.lib.api_capture_close(self.handle)
                        self.cap.stop()
                        self.stop()
                        break
                    frame_idx += 1
                    #time.sleep(0.04)
                    #time.sleep(0.1)
                    continue
                else:
                    #print("ReadFrame: cach miss :misscnt= ", misscnt)
                    if self.cap.status == False or self.sdl.status == False:
                        print("ReadFrame: stop")
                        self.stop()
                        break
                    misscnt += 1
                    #print("ReadFrame: run stop")
                    #self.load.lib.api_capture_close(self.handle)
                    #self.cap.stop()
                    #self.stop()
                    #break
                    time.sleep(0.01)
                    continue
            ###
            with open(self.yuvfilename, 'rb') as fp:
                while (self.sdl.sdl_status() == 0):
                    data = fp.read(self.frame_size)
                    print("ReadFrame: run len(data)= ", len(data))
                    if len(data) > 0:
                        rect = (0, 0, loadlib.WIDTH >> 1, loadlib.HEIGHT >> 1)
                        #self.sdl.sdl_refresh_0(data, rect, 1)
                        # rect = (loadlib.WIDTH >> 1, loadlib.HEIGHT >> 1, loadlib.WIDTH, loadlib.HEIGHT)
                        self.sdl.sdl_refresh(data, rect, 1)
                        #self.sdl.sdl_test(data, loadlib.WIDTH, loadlib.HEIGHT, 1000)
                        #time.sleep(1)
                    else:
                        fp.seek(0, os.SEEK_SET)
                        # self.sdl.sdl_stop()
                        break
                    time.sleep(0.04)
                    #time.sleep(0.001)
                    print("ReadFrame: frame_idx= ", frame_idx)
                    if frame_idx > 10:
                        #self.sdl.sdl_stop()
                        break
                    frame_idx += 1
            print("ReadFrame: run exit")
            self.stop()
        #self.load.lib.api_capture_close(self.handle)
        self.cap.stop()
        print("ReadFrame: run: self.cap.stop ok")
        self.sdl.sdl_stop()
        print("ReadFrame: run: self.sdl.sdl_stop ok")
        self.sdl.stop()
        print("ReadFrame: run: self.sdl.stop ok")
        self.stop()
        print("ReadFrame: run: over")

if __name__ == '__main__':
    print('Start pycall.')
    call = ReadFrame(0)
    (width, height) = (1280, 720)
    #(width, height) = (1920, 1080)
    call.init(width, height)
    call.start()
    #time.sleep(2)
    # idx = 0
    # while idx >= 0 and idx < 4:
    #    try:
    #        idx = int(raw_input('please input to exit(eg: 0 ): '))
    #    except:
    #        idx = int(input('please input to exit(eg: 0 ): '))
    #    print("idx= ", idx)
    # call.stop()
    print('End pycall.')

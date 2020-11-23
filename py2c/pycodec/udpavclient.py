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

import time
import loadlib
from udpclient import DecoderClient as vidodec
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


global_port0 = 10088 #8097
global_port1 = 10089 #8098
global_host = 'localhost'
# global_host = '172.16.0.17'
# global_host = '111.230.226.17'
if len(sys.argv) > 3:
    global_port1 = int(sys.argv[3])
if len(sys.argv) > 2:
    global_port0 = int(sys.argv[2])
if len(sys.argv) > 1:
    global_host = sys.argv[1]



class AVClient(object):
    def __init__(self):
        self.video_thread_show = None
        self.video_thread0 = None
        self.video_thread1 = None
        self.video_thread2 = None
        self.video_thread3 = None
        self.video_thread4 = None
        self.video_thread5 = None
        self.video_thread6 = None
        self.video_thread7 = None
        self.video_thread8 = None
        self.video_thread9 = None

        self.audio_thread_show = None
        self.audio_thread0 = None
        self.audio_thread1 = None
        self.audio_thread2 = None
        self.audio_thread3 = None
        self.audio_thread4 = None
        self.audio_thread5 = None
        self.audio_thread6 = None
        self.audio_thread7 = None
        self.audio_thread8 = None
        self.audio_thread9 = None
    def __del__(self):
        print("AVClient del")
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
    def StartClientVideo(self):

        (bitrate, mtu_size, fec_level, buffer_shift) = self.GetParams(SVC_REFS)
        print("RunClient: bitrate= ", bitrate)
        self.video_thread_show = display(0, SHOW_WIDTH, SHOW_HEIGHT, SHOW_WIDTH, SHOW_HEIGHT)
        idx = 0
        (id0, sessionId0, actor0) = (0, 100, 2)
        (id1, sessionId1, actor1) = (1, 200, 2)
        (id2, sessionId2, actor2) = (2, 300, 2)
        (id3, sessionId3, actor3) = (3, 400, 2)
        (id4, sessionId4, actor4) = (4, 100, 1)
        (id5, sessionId5, actor5) = (5, 200, 1)
        (id6, sessionId6, actor6) = (6, 300, 1)
        (id7, sessionId7, actor7) = (7, 400, 1)
        (id8, sessionId8, actor8) = (8, 500, 2)
        (id9, sessionId9, actor9) = (9, 600, 2)

        #self.video_thread8 = vidodec(id8, sessionId8, actor8, global_host, global_port0)
        #self.video_thread9 = vidodec(id9, sessionId9, actor9, global_host, global_port0)

        self.video_thread0 = vidodec(id0, sessionId0, actor0, global_host, global_port0)
        #self.video_thread1 = vidodec(id1, sessionId1, actor1, global_host, global_port0)
        #self.video_thread2 = vidodec(id2, sessionId2, actor2, global_host, global_port0)
        #self.video_thread3 = DecoderClient(id3, sessionId3, actor3, global_host, global_port0)


        self.video_thread4 = videoenc(id4, sessionId4, actor4, global_host, global_port0)
        #self.video_thread5 = videoenc(id5, sessionId5, actor5, global_host, global_port0)
        #self.video_thread6 = videoenc(id6, sessionId6, actor6, global_host, global_port0)
        #self.video_thread7 = videoenc(id7, sessionId7, actor7, global_host, global_port0)
        ###
        if True:
            if self.video_thread0 != None:
                (self.video_thread0.decode0.width, self.video_thread0.decode0.height) = (SHOW_WIDTH, SHOW_HEIGHT)
                self.video_thread0.decode0.min_distance = 2
                self.video_thread0.decode0.delay_time = 100
                self.video_thread0.decode0.buf_size = (1 << buffer_shift)  # 1024#必须是2的指数
                self.video_thread0.decode0.mtu_size = mtu_size
                self.video_thread0.opencodec()
            if self.video_thread1 != None:
                (self.video_thread1.decode0.width, self.video_thread1.decode0.height) = (SHOW_WIDTH, SHOW_HEIGHT)
                self.video_thread1.decode0.min_distance = 2
                self.video_thread1.decode0.delay_time = 100
                self.video_thread1.decode0.buf_size = (1 << buffer_shift)  # 1024#必须是2的指数
                self.video_thread1.decode0.mtu_size = mtu_size
                self.video_thread1.opencodec()
            if self.video_thread2 != None:
                (self.video_thread2.decode0.width, self.video_thread2.decode0.height) = (SHOW_WIDTH, SHOW_HEIGHT)
                self.video_thread2.decode0.min_distance = 2
                self.video_thread2.decode0.delay_time = 100
                self.video_thread2.decode0.buf_size = (1 << buffer_shift)  # 1024#必须是2的指数
                self.video_thread2.decode0.mtu_size = mtu_size
                self.video_thread2.opencodec()
            if self.video_thread3 != None:
                (self.video_thread3.decode0.width, self.video_thread3.decode0.height) = (SHOW_WIDTH, SHOW_HEIGHT)
                self.video_thread3.decode0.min_distance = 2
                self.video_thread3.decode0.delay_time = 100
                self.video_thread3.decode0.buf_size = (1 << buffer_shift)  # 1024#必须是2的指数
                self.video_thread3.decode0.mtu_size = mtu_size
                self.video_thread3.opencodec()
            if self.video_thread8 != None:
                (self.video_thread8.decode0.width, self.video_thread8.decode0.height) = (SHOW_WIDTH, SHOW_HEIGHT)
                self.video_thread8.decode0.min_distance = 2
                self.video_thread8.decode0.delay_time = 100
                self.video_thread8.decode0.buf_size = (1 << buffer_shift)  # 1024#必须是2的指数
                self.video_thread8.decode0.mtu_size = mtu_size
                self.video_thread8.opencodec()
            if self.video_thread9 != None:
                (self.video_thread9.decode0.width, self.video_thread9.decode0.height) = (SHOW_WIDTH, SHOW_HEIGHT)
                self.video_thread9.decode0.min_distance = 2
                self.video_thread9.decode0.delay_time = 100
                self.video_thread9.decode0.buf_size = (1 << buffer_shift)  # 1024#必须是2的指数
                self.video_thread9.decode0.mtu_size = mtu_size
                self.video_thread9.opencodec()
        if True:
            if self.video_thread4 != None:
                (self.video_thread4.encode0.width, self.video_thread4.encode0.height) = (SHOW_WIDTH, SHOW_HEIGHT)
                self.video_thread4.encode0.refs = SVC_REFS  # 16
                self.video_thread4.fec_level = fec_level  # 1#0#2#0#1#2#3#0#4 #3 #2#0
                self.video_thread4.encode0.enable_fec = 1
                self.video_thread4.encode0.loss_rate = LOSS_RATE  # 0.2 #0.3
                self.video_thread4.encode0.code_rate = (1 - self.video_thread4.encode0.loss_rate)
                bitrate = int(self.video_thread4.encode0.code_rate * bitrate)
                self.video_thread4.encode0.bit_rate = bitrate
                self.video_thread4.encode0.mtu_size = mtu_size
                self.video_thread4.encode0.setparam()
                self.video_thread4.opendevice(1)
                self.video_thread4.opencodec()
            if self.video_thread5 != None:
                (self.video_thread5.encode0.width, self.video_thread5.encode0.height) = (SHOW_WIDTH, SHOW_HEIGHT)
                self.video_thread5.encode0.refs = SVC_REFS  # 16
                self.video_thread5.fec_level = fec_level  # 1#0#2#0#1#2#3#0#4 #3 #2#0
                self.video_thread5.encode0.enable_fec = 1
                self.video_thread5.encode0.loss_rate = LOSS_RATE  # 0.2 #0.3
                self.video_thread5.encode0.code_rate = (1 - self.video_thread5.encode0.loss_rate)
                bitrate = int(self.video_thread5.encode0.code_rate * bitrate)
                self.video_thread5.encode0.bit_rate = bitrate
                self.video_thread5.encode0.mtu_size = mtu_size
                self.video_thread5.encode0.setparam()
                self.video_thread5.opendevice(1)
                self.video_thread5.opencodec()
            if self.video_thread6 != None:
                (self.video_thread6.encode0.width, self.video_thread6.encode0.height) = (SHOW_WIDTH, SHOW_HEIGHT)
                self.video_thread6.encode0.refs = SVC_REFS  # 16
                self.video_thread6.fec_level = fec_level  # 1#0#2#0#1#2#3#0#4 #3 #2#0
                self.video_thread6.encode0.enable_fec = 1
                self.video_thread6.encode0.loss_rate = LOSS_RATE  # 0.2 #0.3
                self.video_thread6.encode0.code_rate = (1 - self.video_thread6.encode0.loss_rate)
                bitrate = int(self.video_thread6.encode0.code_rate * bitrate)
                self.video_thread6.encode0.bit_rate = bitrate
                self.video_thread6.encode0.mtu_size = mtu_size
                self.video_thread6.encode0.setparam()
                self.video_thread6.opendevice(1)
                self.video_thread6.opencodec()
            if self.video_thread7 != None:
                (self.video_thread7.encode0.width, self.video_thread7.encode0.height) = (SHOW_WIDTH, SHOW_HEIGHT)
                self.video_thread7.encode0.refs = SVC_REFS  # 16
                self.video_thread7.fec_level = fec_level  # 1#0#2#0#1#2#3#0#4 #3 #2#0
                self.video_thread7.encode0.enable_fec = 1
                self.video_thread7.encode0.loss_rate = LOSS_RATE  # 0.2 #0.3
                self.video_thread7.encode0.code_rate = (1 - self.video_thread7.encode0.loss_rate)
                bitrate = int(self.video_thread7.encode0.code_rate * bitrate)
                self.video_thread7.encode0.bit_rate = bitrate
                self.video_thread7.encode0.mtu_size = mtu_size
                self.video_thread7.encode0.setparam()
                self.video_thread7.opendevice(1)
                self.video_thread7.opencodec()
        ###

        if self.video_thread_show != None:
            if self.video_thread0 != None:
                self.video_thread0.show = self.video_thread_show
                self.video_thread_show.InsertId(self.video_thread0.id)
            if self.video_thread1 != None:
                self.video_thread1.show = self.video_thread_show
                self.video_thread_show.InsertId(self.video_thread1.id)
            if self.video_thread2 != None:
                self.video_thread2.show = self.video_thread_show
                self.video_thread_show.InsertId(self.video_thread2.id)
            if self.video_thread3 != None:
                self.video_thread3.show = self.video_thread_show
                self.video_thread_show.InsertId(self.video_thread3.id)
            if self.video_thread4 != None:
                self.video_thread4.show = self.video_thread_show
                self.video_thread_show.InsertId(self.video_thread4.id)
            if self.video_thread5 != None:
                self.video_thread5.show = self.video_thread_show
                self.video_thread_show.InsertId(self.video_thread5.id)
            if self.video_thread6 != None:
                self.video_thread6.show = self.video_thread_show
                self.video_thread_show.InsertId(self.video_thread6.id)
            if self.video_thread7 != None:
                self.video_thread7.show = self.video_thread_show
                self.video_thread_show.InsertId(self.video_thread7.id)
            if self.video_thread8 != None:
                self.video_thread8.show = self.video_thread_show
                self.video_thread_show.InsertId(self.video_thread8.id)
            if self.video_thread9 != None:
                self.video_thread9.show = self.video_thread_show
                self.video_thread_show.InsertId(self.video_thread9.id)
            #if EXCHANGE:
            #    self.video_thread_show.exchange_id()
            self.video_thread_show.start()
        ###dec
        if self.video_thread0 != None:
            self.video_thread0.start()
        if self.video_thread1 != None:
            self.video_thread1.start()
        if self.video_thread2 != None:
            self.video_thread2.start()
        if self.video_thread3 != None:
            self.video_thread3.start()
        if self.video_thread8 != None:
            self.video_thread8.start()
        if self.video_thread9 != None:
            self.video_thread9.start()
        # time.sleep(0.8)
        ###enc
        if self.video_thread4 != None:
            self.video_thread4.start()
        if self.video_thread5 != None:
            self.video_thread5.start()
        if self.video_thread6 != None:
            self.video_thread6.start()
        if self.video_thread7 != None:
            self.video_thread7.start()

        return

    def StopClientVideo(self):
        print("StopClientVideo")

        if self.video_thread4 != None:
            self.video_thread4.stop()
        if self.video_thread5 != None:
            self.video_thread5.stop()
        if self.video_thread6 != None:
            self.video_thread6.stop()
        if self.video_thread7 != None:
            self.video_thread7.stop()

        if self.video_thread0 != None:
            self.video_thread0.stop()
        if self.video_thread1 != None:
            self.video_thread1.stop()
        if self.video_thread2 != None:
            self.video_thread2.stop()
        if self.video_thread3 != None:
            self.video_thread3.stop()
        if self.video_thread8 != None:
            self.video_thread8.stop()
        if self.video_thread9 != None:
            self.video_thread9.stop()

        if self.video_thread_show != None:
            self.video_thread_show.stop()

        print("StopClientVideo ok")

    def StartClientAudio(self):

        (bitrate, mtu_size, buffer_shift) = (24000, 300, 10)
        print("RunClient: bitrate= ", bitrate)
        # thread_show = ShowThread(6)
        self.audio_thread_show = player(2)
        idx = 0
        (id0, sessionId0, actor0) = (0, 100, 2)
        (id1, sessionId1, actor1) = (1, 200, 2)
        (id2, sessionId2, actor2) = (2, 300, 2)
        (id3, sessionId3, actor3) = (3, 400, 2)
        (id4, sessionId4, actor4) = (4, 100, 1)
        (id5, sessionId5, actor5) = (5, 200, 1)
        (id6, sessionId6, actor6) = (6, 300, 1)
        (id7, sessionId7, actor7) = (7, 400, 1)
        (id8, sessionId8, actor8) = (8, 500, 2)
        (id9, sessionId9, actor9) = (9, 600, 2)

        #self.audio_thread8 = audiodec(id8, sessionId8, actor8, global_host, global_port1)
        #self.audio_thread9 = audiodec(id9, sessionId9, actor9, global_host, global_port1)

        self.audio_thread0 = audiodec(id0, sessionId0, actor0, global_host, global_port1)
        #self.audio_thread1 = audiodec(id1, sessionId1, actor1, global_host, global_port1)
        #self.audio_thread2 = audiodec(id2, sessionId2, actor2, global_host, global_port1)
        #self.audio_thread3 = audiodec(id3, sessionId3, actor3, global_host, global_port1)


        self.audio_thread4 = audioenc(id4, sessionId4, actor4, global_host, global_port1)
        #self.audio_thread5 = audioenc(id5, sessionId5, actor5, global_host, global_port1)
        #self.audio_thread6 = audioenc(id6, sessionId6, actor6, global_host, global_port1)
        #self.audio_thread7 = audioenc(id7, sessionId7, actor7, global_host, global_port1)
        ###
        if True:
            if self.audio_thread0 != None:
                self.audio_thread0.decode0.min_distance = 2
                self.audio_thread0.decode0.delay_time = 100
                self.audio_thread0.decode0.buf_size = (1 << buffer_shift)  # 1024#必须是2的指数
                self.audio_thread0.decode0.mtu_size = mtu_size
            if self.audio_thread1 != None:
                self.audio_thread1.decode0.min_distance = 2
                self.audio_thread1.decode0.delay_time = 100
                self.audio_thread1.decode0.buf_size = (1 << buffer_shift)  # 1024#必须是2的指数
                self.audio_thread1.decode0.mtu_size = mtu_size
            if self.audio_thread2 != None:
                self.audio_thread2.decode0.min_distance = 2
                self.audio_thread2.decode0.delay_time = 100
                self.audio_thread2.decode0.buf_size = (1 << buffer_shift)  # 1024#必须是2的指数
                self.audio_thread2.decode0.mtu_size = mtu_size
            if self.audio_thread3 != None:
                self.audio_thread3.decode0.min_distance = 2
                self.audio_thread3.decode0.delay_time = 100
                self.audio_thread3.decode0.buf_size = (1 << buffer_shift)  # 1024#必须是2的指数
                self.audio_thread3.decode0.mtu_size = mtu_size
            if self.audio_thread8 != None:
                self.audio_thread8.decode0.min_distance = 2
                self.audio_thread8.decode0.delay_time = 100
                self.audio_thread8.decode0.buf_size = (1 << buffer_shift)  # 1024#必须是2的指数
                self.audio_thread8.decode0.mtu_size = mtu_size
            if self.audio_thread9 != None:
                self.audio_thread9.decode0.min_distance = 2
                self.audio_thread9.decode0.delay_time = 100
                self.audio_thread9.decode0.buf_size = (1 << buffer_shift)  # 1024#必须是2的指数
                self.audio_thread9.decode0.mtu_size = mtu_size
        if True:
            if self.audio_thread4 != None:
                self.audio_thread4.encode0.bit_rate = bitrate
                self.audio_thread4.encode0.mtu_size = mtu_size
                self.audio_thread4.opendevice(1)
                self.audio_thread4.opencodec()
            if self.audio_thread5 != None:
                self.audio_thread5.encode0.bit_rate = bitrate
                self.audio_thread5.encode0.mtu_size = mtu_size
                self.audio_thread5.opendevice(1)
                self.audio_thread5.opencodec()
            if self.audio_thread6 != None:
                self.audio_thread6.encode0.bit_rate = bitrate
                self.audio_thread6.encode0.mtu_size = mtu_size
                self.audio_thread6.opendevice(1)
                self.audio_thread6.opencodec()
            if self.audio_thread7 != None:
                self.audio_thread7.encode0.bit_rate = bitrate
                self.audio_thread7.encode0.mtu_size = mtu_size
                self.audio_thread7.encode0.setparam()
                self.audio_thread7.opendevice(1)
                self.audio_thread7.opencodec()
        ###

        if self.audio_thread_show != None:
            if self.audio_thread0 != None:
                self.audio_thread0.show = self.audio_thread_show
                self.audio_thread_show.InsertId(self.audio_thread0.id)
            if self.audio_thread1 != None:
                self.audio_thread1.show = self.audio_thread_show
                self.audio_thread_show.InsertId(self.audio_thread1.id)
            if self.audio_thread2 != None:
                self.audio_thread2.show = self.audio_thread_show
                self.audio_thread_show.InsertId(self.audio_thread2.id)
            if self.audio_thread3 != None:
                self.audio_thread3.show = self.audio_thread_show
                self.audio_thread_show.InsertId(self.audio_thread3.id)
            if self.audio_thread4 != None:
                self.audio_thread4.show = self.audio_thread_show
                self.audio_thread_show.InsertId(self.audio_thread4.id)
            if self.audio_thread5 != None:
                self.audio_thread5.show = self.audio_thread_show
                self.audio_thread_show.InsertId(self.audio_thread5.id)
            if self.audio_thread6 != None:
                self.audio_thread6.show = self.audio_thread_show
                self.audio_thread_show.InsertId(self.audio_thread6.id)
            if self.audio_thread7 != None:
                self.audio_thread7.show = self.audio_thread_show
                self.audio_thread_show.InsertId(self.audio_thread7.id)
            if self.audio_thread8 != None:
                self.audio_thread8.show = self.audio_thread_show
                self.audio_thread_show.InsertId(self.audio_thread8.id)
            if self.audio_thread9 != None:
                self.audio_thread9.show = self.audio_thread_show
                self.audio_thread_show.InsertId(self.audio_thread9.id)
            #if EXCHANGE:
            #    thread_show.exchange_id()
            self.audio_thread_show.start()
        ###dec
        if self.audio_thread0 != None:
            self.audio_thread0.start()
        if self.audio_thread1 != None:
            self.audio_thread1.start()
        if self.audio_thread2 != None:
            self.audio_thread2.start()
        if self.audio_thread3 != None:
            self.audio_thread3.start()
        if self.audio_thread8 != None:
            self.audio_thread8.start()
        if self.audio_thread9 != None:
            self.audio_thread9.start()
        # time.sleep(0.8)
        ###enc
        if self.audio_thread4 != None:
            self.audio_thread4.start()
        if self.audio_thread5 != None:
            self.audio_thread5.start()
        if self.audio_thread6 != None:
            self.audio_thread6.start()
        if self.audio_thread7 != None:
            self.audio_thread7.start()

        return

    def StopClientAudio(self):
        print("StopClientAudio")

        if self.audio_thread4 != None:
            self.audio_thread4.stop()
        if self.audio_thread5 != None:
            self.audio_thread5.stop()
        if self.audio_thread6 != None:
            self.audio_thread6.stop()
        if self.audio_thread7 != None:
            self.audio_thread7.stop()

        if self.audio_thread0 != None:
            self.audio_thread0.stop()
        if self.audio_thread1 != None:
            self.audio_thread1.stop()
        if self.audio_thread2 != None:
            self.audio_thread2.stop()
        if self.audio_thread3 != None:
            self.audio_thread3.stop()
        if self.audio_thread8 != None:
            self.audio_thread8.stop()
        if self.audio_thread9 != None:
            self.audio_thread9.stop()

        if self.audio_thread_show != None:
            self.audio_thread_show.stop()
        print("StopClientAudio ok")
    def Release(self):
        if self.video_thread4 != None:
            del self.video_thread4
        if self.video_thread5 != None:
            del self.video_thread5
        if self.video_thread6 != None:
            del self.video_thread6
        if self.video_thread7 != None:
            del self.video_thread7

        if self.video_thread0 != None:
            del self.video_thread0
        if self.video_thread1 != None:
            del self.video_thread1
        if self.video_thread2 != None:
            del self.video_thread2
        if self.video_thread3 != None:
            del self.video_thread3
        if self.video_thread8 != None:
            del self.video_thread8
        if self.video_thread9 != None:
            del self.video_thread9

        if self.video_thread_show != None:
            del self.video_thread_show

        if self.audio_thread4 != None:
            del self.audio_thread4
        if self.audio_thread5 != None:
            del self.audio_thread5
        if self.audio_thread6 != None:
            del self.audio_thread6
        if self.audio_thread7 != None:
            del self.audio_thread7

        if self.audio_thread0 != None:
            del self.audio_thread0
        if self.audio_thread1 != None:
            del self.audio_thread1
        if self.audio_thread2 != None:
            del self.audio_thread2
        if self.audio_thread3 != None:
            del self.audio_thread3
        if self.audio_thread8 != None:
            del self.audio_thread8
        if self.audio_thread9 != None:
            del self.audio_thread9

        if self.audio_thread_show != None:
            del self.audio_thread_show
def RunClient(flag):
    if flag:
        client = None
        idx = 0
        while idx >= 0 and idx < 4:
            if client == None:
                client = AVClient()
            client.StartClientVideo()
            client.StartClientAudio()
            if sys.version_info >= (3, 0):
                idx = int(input('please input to exit(eg: 0 ): '))
            else:
                idx = int(raw_input('please input to exit(eg: 0 ): '))
            client.StopClientVideo()
            client.StopClientAudio()
            for i in range(10):
                print("RunClient: wait exit: i=", i)
                time.sleep(1)
            #client.Release()
            #del client
            #client = None
            for i in range(1):
                print("RunClient: wait exit: i=", i)
                time.sleep(1)
            print("idx= ", idx)
        print("main: start stop...")

        print("RunClient: over")

        return
if __name__ == "__main__":
    print("start avclient")
    RunClient(True)
    print("end avclient")

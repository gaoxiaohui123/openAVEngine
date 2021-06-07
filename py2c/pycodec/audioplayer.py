# -*- coding: utf-8 -*


import sys
import os
from ctypes import *
import json
import loadlib
import threading
import time
from audiocapture import AudioCapture
from callaudiocodec import CallAudioCodec

def json2str(jsonobj):
    if sys.version_info >= (3, 0):
        json_str = json.dumps(jsonobj, ensure_ascii=False, sort_keys=False).encode('utf-8')
    else:
        json_str = json.dumps(jsonobj, encoding='utf-8', ensure_ascii=False, sort_keys=False)

    return json_str
class DataOffer(object):
    def __init__(self, id):
        self.id = id
        self.out_channels = 2
        self.out_nb_samples = 1024
        self.out_sample_fmt = "AV_SAMPLE_FMT_S16"  # 2Bytes/sample
        self.out_sample_rate = 48000
        self.out_channel_layout = "AV_CH_LAYOUT_STEREO"
        self.factor = self.out_channels * 2
        self.frame_size = self.factor * self.out_nb_samples
        self.out_buffer_size = self.frame_size
        self.outbuf = create_string_buffer(self.frame_size << 1)  # redundancy
        self.sample_cnt = 0
        self.offset = 0
        self.block_size = 8192
        self.data_buf = create_string_buffer(self.block_size << 1)
    def get_frame_size(self):
        ret = 0
        count = 0
        while True:
            (data, data_size) = self.cap.ReadFrame()
            if data_size > 0:
                ret = data_size
                break
            time.sleep(0.01)
            count += 1
            if count > 100:
                break
        return ret
    def test_init(self):
        ###
        id = (self.id << 2)
        id0 = id + 1
        id1 = id + 0
        #(id0, id1) = ((id + 1), (id + 0))
        self.encode = CallAudioCodec(id0, 0)
        self.encode.init()
        self.decode = CallAudioCodec(id1, 1)
        ###
        self.cap = AudioCapture(self.id)
        self.cap.datatype = 3#8
        if self.id in [0]:
            self.cap.datatype = 0
        self.cap.init()
        self.cap.start()
        ret = self.get_frame_size()
        if ret > 0:
            self.frame_size = ret
            self.out_nb_samples = int(self.frame_size / self.factor)
            self.out_buffer_size = self.frame_size
            print("audioplayer: self.out_nb_samples= ", self.out_nb_samples)
        return ret
    def stop(self):
        if self.cap != None:
            self.cap.stop()
    def read_frame(self):
        ret = None
        data_size2 = 0
        (data, data_size) = self.cap.ReadFrame()
        if data_size > 0:
            self.data_buf[self.offset:(self.offset + data_size)] = data[0:data_size]
            self.offset += data_size
            if self.offset >= self.block_size:
                self.outbuf[0:self.block_size] = self.data_buf[0:self.block_size]
                data_size2 = self.block_size
                tail = self.offset - self.block_size
                self.data_buf[0:tail] = self.data_buf[self.offset - tail:self.offset]
                self.offset = tail
                ret = (data_size2, self.outbuf)

        return ret
    def out_frame(self):
        ret = None
        (data, data_size) = self.cap.ReadFrame()
        #print("DataOffer:out_frame: data_size= ", data_size)
        if data_size > 0:
            self.outbuf[self.sample_cnt:(self.sample_cnt + self.frame_size)] = data[0:self.frame_size]
            self.sample_cnt += data_size
            if self.sample_cnt >= (self.frame_size << 1) and True:
                (osize, outbuf, outparam) = self.encode.codecframe(self.outbuf, (self.frame_size << 1))
                # print("audioplayer: osize= ", osize)
                if osize > 0 and True:
                    # print("rtppacket: osize=", osize)
                    (osize, outbuf, outparam) = self.encode.rtppacket(outbuf, osize)
                    # print("resort: osize=", osize)
                    (osize, outbuf, outparam, frame_timestamp) = self.decode.resort(outbuf, osize)
                     # print("rtpunpacket: osize=", osize)
                    if osize > 0:
                        (osize, outbuf, outparam) = self.decode.rtpunpacket(outbuf, osize)
                        # print("rtpunpacket: 2: osize=", osize)
                if osize > 0:
                    # print("start decode: osize= ", osize)
                    (osize2, outbuf2, outparam2) = self.decode.codecframe(outbuf, osize)
                    ret = (osize2, outbuf2)
                self.sample_cnt = 0

            elif self.sample_cnt >= (self.frame_size << 1) and False:
                #print("DataOffer:out_frame: self.sample_cnt= ", self.sample_cnt)
                ret = ((self.frame_size << 1), self.outbuf)
                self.sample_cnt = 0
        return ret
class AudioPlayer(threading.Thread):
    def __init__(self, id):
        threading.Thread.__init__(self)
        self.id = id
        self.lock = threading.Lock()
        self.load = loadlib.gload
        self.status = False
        self.showlist = []
        self.showmap = {}
        self.layerlist = []
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True

        self.handle_size = 8
        self.handle = create_string_buffer(self.handle_size)
        self.param = {}

        self.format = "AUDIO_S16SYS"
        self.silence = 0
        self.out_channels = 2
        self.out_nb_samples = 2048 #1024
        self.out_sample_fmt = "AV_SAMPLE_FMT_S16"  # 2Bytes/sample
        self.out_sample_rate = 48000
        self.out_channel_layout = "AV_CH_LAYOUT_STEREO"
        self.factor = self.out_channels * 2
        self.frame_size = self.factor * self.out_nb_samples
        self.out_buffer_size = self.frame_size
        self.mix_num = 2#4
        self.sdl_status = 1
        self.max_mix_num = 16

        self.pcmfile = "/home/gxh/works/play_" + str(id) + ".pcm"
        self.pcmfile = "./play_" + str(id) + ".pcm"
        self.pcmfile = ""

        #self.init()
        self.data_offer = None
        self.data_offer2 = None
        ###
        self.param = {}
        self.outbuf = None
    def __del__(self):
        print("AudioPlayer del")
        if self.param != None:
            del self.param
        if self.outbuf != None:
            del self.outbuf
        if self.handle != None:
            del self.handle
    def init(self):
        self.param = {}
        self.param.update({"mix_num": self.mix_num})
        self.param.update({"format": self.format})
        self.param.update({"silence": self.silence})
        self.param.update({"out_channels": self.out_channels})
        self.param.update({"out_nb_samples": self.out_nb_samples})
        self.param.update({"out_sample_fmt": self.out_sample_fmt})
        self.param.update({"out_sample_rate": self.out_sample_rate})
        self.param.update({"out_channel_layout": self.out_channel_layout})
        self.param.update({"out_buffer_size": self.out_buffer_size})
        self.param.update({"frame_size": self.frame_size})
        self.param.update({"pcmfile": self.pcmfile})
        self.param.update({"sdl_status": self.sdl_status})
        self.param.update({"print": 0})
        param_str = json2str(self.param)
        print("param_str= ", param_str)
        ret = self.load.lib.api_player_init(self.handle, param_str)
        print("init: ret= ", ret)
        if ret >= 0:
            self.status = True
        else:
            self.status = False
        self.outbuf = create_string_buffer(self.frame_size << 1) #redundancy
        array_type = c_char_p * self.max_mix_num
        self.mix_buf = array_type()

    def player_stop(self):
        self.load.lib.api_player_close(self.handle)

    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals
        if self.data_offer == None and self.data_offer2 == None:
            self.player_stop()

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞
    def reset_mix_buf(self, mix_num):
        if mix_num > self.mix_num:
            self.mix_num = mix_num
            if mix_num > self.max_mix_num:
                array_type = c_char_p * self.mix_num
                self.mix_buf = array_type()
    def play_one_frame(self, data, data_size):
        self.param.update({"mix_num": 1})
        param_str = json2str(self.param)
        #ret = self.load.lib.audio_play_frame(self.handle, self.outbuf, self.frame_size)
        ret = self.load.lib.audio_play_frame(self.handle, param_str, data, data_size)
        return ret
    def play_one_frame_mix(self, data_size, mix_num):
        self.param.update({"mix_num": mix_num})
        param_str = json2str(self.param)
        #ret = self.load.lib.audio_play_frame(self.handle, self.outbuf, self.frame_size)
        ret = self.load.lib.audio_play_frame_mix(self.handle, param_str, self.mix_buf, data_size)
        return ret

    def run(self):
        print("audioplayer: run 0")
        ###
        data3 = create_string_buffer(self.frame_size)
        while self.__running.isSet():
            self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            #print("audioplayer: ReadFrame")
            if (self.data_offer != None) or (self.data_offer2 != None):
                flag = 0
                mix_flag = False
                if self.data_offer != None and self.data_offer2 != None:
                    mix_flag = True
                #print("audioplayer: run: mix_flag= ", mix_flag)
                if mix_flag:
                    if True:
                        (ret, ret2) = (None, None)
                        while (ret == None) or (ret2 == None):
                            if ret == None:
                                #ret = self.data_offer.out_frame()
                                ret = self.data_offer.read_frame()
                                if ret != None:
                                    flag |= 1
                            if ret2 == None:
                                #ret2 = self.data_offer2.out_frame()
                                ret2 = self.data_offer2.read_frame()
                                if ret2 != None:
                                    flag |= 2
                            if flag != 3:
                                time.sleep(0.001)
                        ###
                        (osize, data) = ret
                        self.mix_buf[0] = data.raw
                        (osize, data2) = ret2
                        self.mix_buf[1] = data2.raw
                        #self.mix_buf[1] = data.raw

                        ret = self.play_one_frame_mix(osize, 2)
                        flag = 0
                else:
                    if self.data_offer != None:
                        #ret = self.data_offer.out_frame()
                        ret = self.data_offer.read_frame()
                        if ret != None:
                            (osize, data) = ret
                            #print("AudioPlayer: run: osize= ", osize)
                            #data3[0:4096] = data[0:4096]
                            #ret = self.play_one_frame(data3, 4096)
                            #data3[0:4096] = data[4096:8192]
                            #ret = self.play_one_frame(data3, 4096)
                            ret = self.play_one_frame(data, osize)
                            flag = 1
                    if self.data_offer2 != None:
                        #ret = self.data_offer2.out_frame()
                        ret = self.data_offer2.read_frame()
                        if ret != None:
                            (osize, data) = ret
                            #print("AudioPlayer: run: osize= ", osize)
                            #data3[0:4096] = data[0:4096]
                            #ret = self.play_one_frame(data3, 4096)
                            #data3[0:4096] = data[4096:8192]
                            #ret = self.play_one_frame(data3, 4096)
                            ret = self.play_one_frame(data, osize)
                            flag = 1
                if flag == 0:
                    time.sleep(0.01)
            else:
                #print("audioplayer: ReadFrame: null")
                time.sleep(0.001)
        print("audioplayer: run stop")
        #self.stop()

        if self.data_offer != None:
            self.data_offer.stop()
        if self.data_offer2 != None:
            self.data_offer2.stop()
        self.player_stop()

        self.status = False
        print("audioplayer: run over")

if __name__ == '__main__':
    print('Start AudioPlayer.')
    (call0, call1) = (None, None)
    loadlib.gload.lib.api_ffmpeg_register()
    call0 = AudioPlayer(0)
    #call1 = AudioPlayer(1)
    if call0 != None:
        call0.data_offer = DataOffer(1)
        call0.frame_size = call0.data_offer.test_init()
        #call0.frame_size = 8192  # test
        call0.data_offer2 = DataOffer(0)
        call0.frame_size = call0.data_offer2.test_init()
        #call0.frame_size = 8192  # test
        call0.init()
        call0.start()
    if call1 != None:
        call1.data_offer = DataOffer(1)
        call1.frame_size = call1.data_offer.test_init()
        #call1.frame_size = 8192  # test
        call1.init()
        call1.start()
    #time.sleep(2)
    if False:
        from mysdl import *

        call = ReadFrame(0)
        (width, height) = (1280, 720)
        # (width, height) = (1920, 1080)
        call.init(width, height)
        call.start()

    idx = 0
    while idx >= 0 and idx < 4:
       try:
           idx = int(raw_input('please input to exit(eg: 0 ): '))
       except:
            idx = int(input('please input to exit(eg: 0 ): '))
       print("idx= ", idx)
    if call0 != None:
        call0.stop()
    if call1 != None:
        call1.stop()
    print('End AudioPlayer.')

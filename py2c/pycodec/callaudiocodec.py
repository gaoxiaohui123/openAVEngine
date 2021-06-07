# -*- coding: utf-8 -*


import sys
import os
import shutil
import time
import threading

#import matplotlib.pyplot as plt # plt 用于显示图片
#from matplotlib import pyplot as plt
#import matplotlib.image as mpimg # mpimg 用于读取图片
import numpy as np
#from PIL import Image
import math

import ctypes
from ctypes import *
import json
import loadlib

#(WIDTH, HEIGHT) = (352, 288)
#src_dir = r'/data/home/gxh/works/datashare/for_ENC/352X288/foreman_cif.yuv'

sys.path.append(".")

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
class CallAudioCodec(object):
    def __init__(self, id, type):
        self.load = loadlib.gload
        self.obj_id = id
        self.codec_type = type
        self.lock = threading.Lock()
        ###
        self.handle_size = 8
        self.handle = create_string_buffer(self.handle_size)

        self.codec_id = "AV_CODEC_ID_AAC"
        self.bit_rate = 24000
        self.sample_fmt = "AV_SAMPLE_FMT_S16"

        ###
        filename = "aac_" + "decode_" + str(id) + ".pcm"
        if type > 0:
            filename = "aac_" + "decode_" + str(id) + ".pcm"
        self.filename = "/home/gxh/works/" + filename
        self.filename = "./" + filename
        self.filename = ""

        self.out_sample_rate = 48000
        self.out_channels = 2
        self.out_nb_samples = 2048#1024
        self.out_channel_layout = "AV_CH_LAYOUT_STEREO"
        self.param = {}
        self.param2 = {}
        self.outbuf = None
        if type > 0:
            self.init()
        ###
        self.mtu_size = 800#300
        self.start_time = 0
        self.last_timestamp = 0
        self.min_distance = 1
        self.delay_time = 100
        self.buf_size = 1 << 10
        self.qos_level = 0
        self.seqnum = 0
        self.fec_seqnum = 0
        self.pkt_buf = create_string_buffer(self.mtu_size)
        self.resort_buf = create_string_buffer(self.mtu_size)

    def __del__(self):
        print("CallAudioCodec del")
        if self.param != None:
            del self.param
        if self.outbuf != None:
            del self.outbuf
        if self.pkt_buf != None:
            del self.pkt_buf
        if self.resort_buf != None:
            del self.resort_buf
        if self.handle != None:
            del self.handle
    def init(self):
        self.param = {}
        self.param.update({"codec_type": self.codec_type})
        self.param.update({"codec_id": self.codec_id})
        self.param.update({"bit_rate": self.bit_rate})
        self.param.update({"sample_fmt": self.sample_fmt})
        self.param.update({"out_nb_samples": self.out_nb_samples})
        self.param.update({"out_sample_rate": self.out_sample_rate})
        self.param.update({"out_channels": self.out_channels})
        self.param.update({"out_channel_layout": self.out_channel_layout})
        self.param.update({"filename": self.filename})
        self.param.update({"print": 0})
        param_str = json2str(self.param)
        ret = self.load.lib.api_audio_codec_init(self.handle, param_str)
        if ret > 0:
            print("CallAudioCodec: init: ok: ", self.codec_type)
        else:
            print("CallAudioCodec: init: error: ", self.codec_type)

        self.frame_size = self.out_channels * 2 * self.out_nb_samples
        self.outbuf = create_string_buffer(self.frame_size << 1)
        array_type = c_char_p * 4
        self.outparam = array_type()
    def getparam(self):
        return (self.param, self.outbuf, self.outparam)
    def codecframe(self, data, insize):
        ret = 0
        if len(data) > 0:
            self.param.update({"insize": insize})
            param_str = json2str(self.param)
            ###
            self.outparam[0] = b""
            ret = self.load.lib.api_audio_codec_one_frame(self.handle, data, param_str, self.outbuf, self.outparam)
            #print("CallAudioCodec: ret= ", ret)
            #if self.codec_type > 0:
            #    print("codecframe: ret=", ret)
        return (ret, self.outbuf, self.outparam)

    def codecclose(self):
        self.load.lib.api_audio_codec_close(self.handle)
        return

    def createtimestamp(self):
        interval = 1000 / (48000 / 2048)
        timestamp = 0
        now_time = time.time()
        if self.start_time == 0:
            self.start_time = now_time
        else:
            difftime = now_time - self.start_time
            frame_num = int(difftime * 1000) / int(interval)
            if sys.version_info >= (3, 0):
                long_timestamp = int(difftime * 1000 * 27000 / 1000)
                MAX_UINT = ((int(1) << 32) - 1)
            else:
                long_timestamp = long(difftime * 1000 * 27000 / 1000)
                MAX_UINT = ((long(1) << 32) - 1)
            timestamp = int(long_timestamp)
            if long_timestamp > MAX_UINT:
                timestamp = int(long_timestamp - MAX_UINT)
                print("EncoderClient: overflow: (timestamp, id)= ", (timestamp, self.obj_id))

            if (timestamp < self.last_timestamp):
                print("(timestamp, self.last_timestamp)= ", (timestamp, self.last_timestamp))
            self.last_timestamp = timestamp
        return timestamp
    #def reset_seqnum(self, data):
    #    #ret = -1
    #    #self.lock.acquire()
    #    self.fec_seqnum = self.load.lib.api_audio_reset_seqnum(self.handle, data, len(data), self.fec_seqnum)
    #    ret = self.fec_seqnum
    #    #self.lock.release()
    #    return ret
    def rtppacket(self, data, insize):
        ret = 0
        if insize > 0:
            self.param.update({"insize": insize})
            timestamp = self.createtimestamp()
            #print("rtppacket: timestamp= ", timestamp)
            self.param.update({"timestamp": timestamp})
            self.param.update({"seqnum": self.seqnum})
            param_str = json2str(self.param)
            ###
            self.outparam[0] = b""
            self.pkt_buf[0:insize] = data[0:insize]
            #self.lock.acquire()
            ret = self.load.lib.api_audio_raw2rtp_packet(self.handle, self.pkt_buf, param_str, self.outbuf, self.outparam)
            if ret > 0:
                self.seqnum = int(self.outparam[1])
            #self.lock.release()
        return (ret, self.outbuf, self.outparam)
    def rtpunpacket(self, data, insize):
        ret = 0
        if insize > 0:
            #print("rtpunpacket:insize= ", insize)
            self.param.update({"insize": insize})
            param_str = json2str(self.param)
            ###
            self.outparam[0] = b""
            #print("rtpunpacket:param_str= ", param_str)
            self.pkt_buf[0:insize] = data[0:insize]
            ret = self.load.lib.api_audio_rtp_packet2raw(self.handle, self.pkt_buf, param_str, self.outbuf, self.outparam)
        return (ret, self.outbuf, self.outparam)
    def resort(self, data, insize):
        ret = 0
        frame_timestamp = 0
        self.param2.update({"min_distance": self.min_distance})
        self.param2.update({"delay_time": self.delay_time})
        self.param2.update({"buf_size": self.buf_size})
        self.param2.update({"qos_level": self.qos_level})
        self.param2.update({"loglevel": 1})

        if insize > 0:
            self.param2.update({"insize": insize})
            param_str = json2str(self.param2)
            #print("resort:param_str= ", param_str)
            ###
            self.outparam[0] = b""
            self.resort_buf[0:insize] = data[0:insize]
            ret = self.load.lib.api_audio_resort_packet(self.handle, self.resort_buf, param_str, self.outbuf, self.outparam)
            if ret > 0:
                frame_timestamp = char2long(self.outparam[3])
        return (ret, self.outbuf, self.outparam, frame_timestamp)

if __name__ == '__main__':
    print('Start pycall.')
    call = CallAudioCodec(0, 0)
    print('End pycall.')
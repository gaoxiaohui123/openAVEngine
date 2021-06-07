# -*- coding: utf-8 -*


import sys
import os
import shutil
import time

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
class AVStream(object):
    def __init__(self, id, width, height):
        self.load = loadlib.gload
        self.width = width
        self.height = height
        self.id = id
        self.handle_size = 8
        self.handle = create_string_buffer(self.handle_size)
        self.min_distance = 2
        self.delay_time = 100
        self.buf_size = 1024
        #
        self.is_write = 1
        self.is_file = -1
        #self.is_audio; // else is_video
        self.infile = ""
        self.outfile = ""
        self.video_codec_handle = None
        self.audio_codec_handle = None
        self.filetype = ".mp4"
        self.param = {}
        #(self.param, self.outbuf, self.outparam) = self.setparam()
        self.outbuf = None
    def __del__(self):
        print("AVStream del")
        if self.param != None:
            del self.param
        if self.outbuf != None:
            del self.outbuf
        if self.handle != None:
            del self.handle
    def setparam(self):
        #self.width = loadlib.WIDTH
        #self.height = loadlib.HEIGHT
        print("AVStream: setparam: (self.width, self.height)= ", (self.width, self.height))
        if self.is_write:
            if self.outfile != "":
                self.filetype = self.outfile.split(".")[1]
                if "." in self.filetype:
                    self.is_file = 1
                else:
                    self.filetype = self.outfile.split("//")[0]  #rtmp://stram_name_id
                    if self.filetype != "":
                        self.is_file = 0
        else:
            if self.infile != "":
                self.filetype = self.infile.split(".")[1]
                if "." in self.filetype:
                    self.is_file = 1
                else:
                    self.filetype = self.outfile.split("//")[0]  #rtmp://stram_name_id
                    if self.filetype != "":
                        self.is_file = 0
        self.param.update({"infile": self.infile})
        self.param.update({"outfile": self.outfile})
        self.param.update({"is_write": self.is_write})
        self.param.update({"is_file": self.is_file})
        frame_size = int((self.width * self.height * 3) / 2)
        outbuf = create_string_buffer(frame_size)
        array_type = c_char_p * 4
        outparam = array_type()
        return
    def openstream(self):
        self.setparam()
        param_str = json2str(self.param)
        print("openstream: param_str=", param_str)
        ret = self.load.lib.api_avstream_init(self.handle, param_str)
        if self.video_codec_handle != None:
            self.load.lib.api_avstream_set_video_handle(self.handle, self.video_codec_handle)
        if self.video_codec_handle != None:
            self.load.lib.api_avstream_set_audio_handle(self.handle, self.audio_codec_handle)

if __name__ == '__main__':
    print('Start AVStream.')
    call = AVStream(0, 1280, 720)

    print('End AVStream.')
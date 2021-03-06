# -*- coding: utf-8 -*


import sys
import os
import shutil
import time
#import cv2

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
def str2json(json_str):
    outjson = None
    try:
        if sys.version_info >= (3, 0):
            outjson = json.loads(json_str.decode())
        else:
            outjson = json.loads(json_str, encoding='utf-8')
    except:
        try:
            outjson = json.loads(json_str)
        except:
            print("ReadCmd: not cmd")
            # print("not cmd: str_cmd=", str_cmd)
        else:
            pass
    else:
        pass
    return outjson
class CallVideoEncode(object):
    def __init__(self, id, width, height):
        self.load = loadlib.gload
        self.id = id
        self.handle_size = 8
        self.handle = create_string_buffer(self.handle_size)
        self.seqnum = 0 #can be changed by fec
        self.enable_fec = 1 #0#1
        self.refresh_idr = 1
        self.loss_rate = 0.3
        self.code_rate = (1 - self.loss_rate)
        (self.width, self.height) = (width, height)
        self.max_refs = 16
        self.mtu_size = 1400 #150 #500 #200 #1100#1400
        self.frame_rate = 25 #15 #25
        self.gop_size = 50 #self.frame_rate << 1 #50
        self.qp = 26
        self.qmin = 20
        self.qmax = 40 #45#40
        self.refs = 4
        self.bit_rate = self.get_bit_rate() #2000*1024 #524288
        self.max_b_frames = 0
        self.thread_type = 1#2 #1 #2
        self.thread_count = 1#4#1
        self.adapt_cpu = 0#1
        #
        self.osd = 1
        self.orgX = 0
        self.orgY = 0
        self.scale = 0
        self.color = 1
        self.param = {}
        (self.param, self.outbuf, self.outparam) = self.setparam()
        self.sizelist = []
        start_time = time.time()
        difftime = time.time() - start_time
        print('{} CallVideoEncode: init time: {:.3f}ms'.format(time.ctime(), difftime * 1000))
        self.outbuf = None
        self.up_bitrate = 0
        self.last_timestamp = 0
    def __del__(self):
        print("CallVideoEncode del")
        if self.param != None:
            del self.param
        if self.outbuf != None:
            del self.outbuf
        if self.sizelist != None:
            del self.sizelist
        if self.handle != None:
            del self.handle

    def get_bit_rate(self):
        ret = 2000*1024
        (h, w) = (self.height, self.width)
        if (w * h) <= 352 * 288:
            ret = 512 * 1024
        elif (w * h) <= 640 * 480:
            ret = 800 * 1024
        elif (w * h) <= 704 * 576:
            ret = 1000 * 1024
        elif (w * h) <= 1280 * 720:
            ret = int(1.5 * 1024 * 1024)
        elif (w * h) <= 1920 * 1080:
            ret = 2.5 * 1024 * 1024

        return ret
    def get_cpx(self):
        ret = "superfast"
        #ret = "ultrafast"
        #return ret
        (h, w) = (self.height, self.width)
        if (w * h) <= 352 * 288:
            ret = "medium"
            ret = "fast"
            ret = "veryfast"
        elif (w * h) <= 704 * 576:
            ret = "fast"
            ret = "veryfast"
        elif (w * h) <= 1280 * 720:
            ret = "veryfast"
        elif (w * h) <= 1920 * 1080:
            #ret = "veryfast"
            ret = "superfast"
            #ret = "ultrafast"
        else:
            ret = "superfast"
        return ret
    def setparam(self):
        #param = {}
        width = self.width
        height = self.height
        self.param.update({"seqnum": self.seqnum})
        self.param.update({"enable_fec": self.enable_fec})
        self.param.update({"refresh_idr": self.refresh_idr})
        self.param.update({"width": self.width})
        self.param.update({"height": self.height})
        k = 1.0 #0.8
        self.param.update({"bit_rate": int(self.bit_rate * k)}) #400000
        self.param.update({"gop_size": self.gop_size})
        self.cpx = self.get_cpx()
        self.param.update({"preset": self.cpx})  # "veryfast", "fast" #"slow" fast/faster/verfast/superfast/ultrafas
        # param.update({"preset": "slow"})
        self.param.update({"tune": "zerolatency"})
        if self.mtu_size > 0:
            self.param.update({"slice-max-size": str(self.mtu_size)})
            self.param.update({"mtu_size": self.mtu_size})
        self.param.update({"tff": 0})
        self.param.update({"bff": 0})
        self.param.update({"qmin": self.qmin})  # 20
        self.param.update({"qmax": self.qmax})  # 40
        self.param.update({"qp": self.qp})  # 20
        self.param.update({"max_b_frames": self.max_b_frames})
        self.param.update({"coder_type": 2})  # 1:cavlc/2:cabac
        self.param.update({"refs": self.refs})
        self.param.update({"frame_rate": self.frame_rate})
        self.param.update({"thread_type": self.thread_type}) #FF_THREAD_FRAME: 1 #FF_THREAD_SLICE: 2
        self.param.update({"thread_count": self.thread_count})
        ###
        self.param.update({"osd": self.osd})
        self.param.update({"orgX": self.orgX})
        self.param.update({"orgY": self.orgY})
        self.param.update({"scale": self.scale})
        self.param.update({"color": self.color})
        self.param.update({"adapt_cpu": self.adapt_cpu})
        self.param.update({"print": 0})
        ##param.update({"sdp": 1})
        frame_size = int((width * height * 3) / 2)
        self.outbuf = create_string_buffer(frame_size)
        array_type = c_char_p * 4
        self.outparam = array_type()
        return (self.param, self.outbuf, self.outparam)
    def resetqp(self, qp):
        self.param.update({"qmin": qp})  # 20
        self.param.update({"qmax": qp})  # 40
    def resetresolution(self, w, h):
        self.param.update({"width": w})
        self.param.update({"height": h})
    def encode_one_frame(self, i, data):
        pict_type = 0
        if i % self.gop_size:
            I = (i % self.gop_size)
            if (self.max_b_frames == 0) and (self.refs != 1):
                j = (I - 1) % (self.refs)
                # I P0 P1 P2 P0 P1 P2 (ref=3)
                ref_idx = j + 1
                if I >= (int(self.gop_size / self.max_refs) * self.max_refs):
                    ref_idx = 1
                elif (ref_idx & 1) == 0 and ref_idx != self.refs and self.refs > 2:
                    ref_idx = 1
                elif ref_idx == (self.refs - 1) and (self.refs > 4):
                    ref_idx = 1
                ref_idx0 = ref_idx
                # ref_idx = 1
            else:
                j = (I - 1) % (self.max_b_frames + 1)
                # I P0 P1 P2 P0 P1 P2 (ref=3)
                ref_idx = j + 1
                # print("EncoderClient: ref_idx= ", ref_idx)
                if ref_idx == (self.max_b_frames + 1):
                    pict_type = 2  # P Frame
                else:
                    pict_type = 3  # B Frame
                if I == (self.gop_size - 1):
                    pict_type = 2  # P Frame
        else:
            ref_idx = 0
            ref_idx0 = ref_idx
            if i > 0:
                self.refresh_idr = 0
                self.param.update({"refresh_idr": self.refresh_idr})
        # if self.log_fp != None:
        #    self.log_fp.write("ref_idx= " + str(ref_idx) + "\n")
        #    self.log_fp.flush()
        # print("test_encode: (i, ref_idx)= ", (i, ref_idx))
        self.param.update({"frame_idx": i})
        self.param.update({"seqnum": self.seqnum})
        self.param.update({"res_num": 0})
        self.param.update({"res_idx": 0})
        self.param.update({"up_bitrate": int(self.up_bitrate)})
        if ((self.refs & 1) == 0) and True:
            self.param.update({"ref_idx": ref_idx})
        else:
            # self.param.update({"ref_idx": ref_idx})
            self.param.update({"pict_type": pict_type})
        #self.read_ack()

        # param_str = json.dumps(self.param, encoding='utf-8', ensure_ascii=False, indent=4,sort_keys=True)
        param_str = json2str(self.param)
        enc_start = time.time()
        ret = self.load.lib.api_video_encode_one_frame(self.handle, data, param_str,
                                                               self.outbuf, self.outparam)

        #test_now_time = time.time()
        #test_time_enc = int((test_now_time - now_time) * 1000)
        #if test_time_enc > self.test_max_time:
        #    print("EncoderClient: 1: (test_time_enc, id)= ", (test_time_enc, self.outparam[0], self.id))
        #    # print("EncoderClient: ret= ", ret)
        print("outparam[1]= ", self.outparam[1])
        #test_time_enc = int((test_now_time - enc_start) * 1000)
        return (ret, ref_idx, pict_type)
    def encode_packet(self, ret, now_time, start_time, interval, rttDictList, netInfoList):
        data2 = self.outbuf.raw[:ret]
        # test rtp
        if True:
            self.outparam[0] = b""
            self.outparam[1] = b""
            self.param.update({"insize": ret})
            self.param.update({"seqnum": self.seqnum})
            ###
            timestamp = 0
            if start_time == 0:
                start_time = now_time
            else:
                difftime = now_time - start_time
                frame_num = int(difftime * 1000) / int(interval)
                if sys.version_info >= (3, 0):
                    long_timestamp = int(difftime * 1000 * 90000 / 1000)
                    MAX_UINT = ((int(1) << 32) - 1)
                else:
                    long_timestamp = long(difftime * 1000 * 90000 / 1000)
                    MAX_UINT = ((long(1) << 32) - 1)
                timestamp = int(long_timestamp)
                if long_timestamp > MAX_UINT:
                    timestamp = int(long_timestamp - MAX_UINT)
                    print("EncoderClient: overflow: (timestamp, id)= ", (timestamp, self.id))

                if (timestamp < self.last_timestamp):
                    print("(timestamp, self.last_timestamp)= ", (timestamp, self.last_timestamp))
                self.last_timestamp = timestamp
            self.param.update({"timestamp": timestamp})
            self.param.update({"selfChanId": (self.id + 1)})
            if len(rttDictList) > 0:
                self.param.update({"rttInfo": rttDictList})
            # print("EncoderClient: 2：len(netInfoList)= ", len(netInfoList))
            if len(netInfoList) > 0:
                self.param.update({"netInfo": netInfoList})
            ###
            # param_str = json.dumps(self.param, encoding='utf-8', ensure_ascii=False, indent=4,sort_keys=True)
            param_str = json2str(self.param)
            ret2 = self.load.lib.api_raw2rtp_packet(self.handle, data2, param_str,
                                                            self.outbuf, self.outparam)

            #test_now_time = time.time()
            if len(netInfoList) > 0:
                del (self.param["netInfo"])
            #test_time_rtp = int((test_now_time - now_time) * 1000)
            #if test_time_rtp > self.test_max_time:
            #    print("EncoderClient: 2: (test_time_rtp, id)= ", (test_time_rtp, self.id))
            #    # print("EncoderClient: ret2= ", ret2)
            # print("outparam[0]= ", outparam[0])
            self.seqnum = int(self.outparam[1])
            # print("self.seqnum= ", self.seqnum)
            rtpSize = self.outparam[0].split(b",")
            ##print("EncoderClient: rtpSize= ", rtpSize)
            if len(rtpSize) > 512:
                print("EncoderClient: len(rtpSize)= ", len(rtpSize))
            data3 = self.outbuf.raw[:ret2]
            return (data3, rtpSize)
    def encode_fec(self, data3, ref_idx, pict_type, rtpSize, fec_level):
        ret = 0
        enable_fec = 0
        self.param.update({"enable_fec": enable_fec})
        if self.enable_fec:
            # ret2 = self.load.lib.api_get_extern_info(self.id, data3, self.outparam)
            # if ret2 > 0:
            #    outjson = self.str2json(self.outparam[0])
            #    if outjson != None:
            #        refs = outjson["refs"]
            #        ref_idx = outjson["ref_idx"]
            if self.refs != 1:
                if fec_level == 0:
                    if (ref_idx == 0) or (ref_idx == self.refs):
                        enable_fec = 1
                elif fec_level == 1:
                    if (ref_idx != 1) or (self.refs == 2):
                        enable_fec = 1
                else:
                    enable_fec = 1
            else:
                if fec_level == 0:
                    if pict_type in [1, 2]:
                        enable_fec = 1
                elif fec_level == 1:
                    if pict_type in [1, 2, 3]:
                        enable_fec = 1
                else:
                    enable_fec = 1
        # print("(enable_fec, ref_idx)= ", (enable_fec, ref_idx))
        # print("EncoderClient: enable_fec= ", enable_fec)
        # print("EncoderClient: pict_type= ", pict_type)
        if enable_fec:
            self.param.update({"enable_fec": self.enable_fec})
            self.param.update({"loss_rate": self.loss_rate})
            self.param.update({"code_rate": self.code_rate})
            sizelist = []
            for packet_size in rtpSize:
                sizelist.append(int(packet_size))
            fec_k = len(sizelist)
            # print("EncoderClient: api_fec_encode: sizelist=", sizelist)
            del (self.param["insize"])
            self.param.update({"inSize": sizelist})
            fec_n = int(fec_k / self.code_rate + 0.8)
            if fec_k < 5 and fec_level > 2:
                if fec_level == 3:
                    self.param.update({"loss_rate": 0.5})
                    self.param.update({"code_rate": 0.5})
                    fec_n = int(fec_k / 0.5 + 0.8)
                elif fec_level == 4:
                    self.param.update({"loss_rate": 0.8})
                    self.param.update({"code_rate": 0.2})
                    fec_n = int(fec_k / 0.2 + 0.8)
            ##test_fec_k += fec_k
            ##test_fec_n += fec_n
            # param_str = json.dumps(self.param, encoding='utf-8', ensure_ascii=False,sort_keys=True)
            param_str = json2str(self.param)
            # print("EncoderClient: api_fec_encode: param_str=", param_str)
            ret = self.load.lib.api_fec_encode(self.handle, data3, param_str,
                                               self.outbuf, self.outparam)

            # test_now_time = time.time()
            # test_time_fec = int((test_now_time - now_time) * 1000)
            # print("EncoderClient: api_fec_encode: ret=", ret)
            if ret > 0:
                # print("encode raw size= ", ret)
                self.seqnum = int(self.outparam[1])
                pktSize = self.outparam[0].split(b",")
                # print("pktSize= ", pktSize)
                # print("len(pktSize)= ", len(pktSize))
                data4 = self.outbuf.raw[:ret]
                ###test
                if False:
                    sizelist = []
                    for packet_size in pktSize:
                        sizelist.append(int(packet_size))
                    self.param.update({"inSize": sizelist})
                    param_str = json.dumps(self.param, encoding='utf-8', ensure_ascii=False,
                                           sort_keys=True)
                    ret = self.load.lib.api_fec_decode(self.id + 1, data4,
                                                       param_str, self.outbuf,
                                                       self.outparam)
                    if ret > 0 and True:
                        print("EncoderClient: decode raw size= ", ret)
                        pktSize = self.outparam[0].split(",")
                        print("EncoderClient: pktSize= ", pktSize)
                        print("EncoderClient: len(pktSize)= ", len(pktSize))
                        data5 = self.outbuf.raw[:ret]

                        # del(param["inSize"])
                        # (param, outbuf, outparam) = self.setparam()

                        data3 = data5
                        rtpSize = pktSize
                else:
                    data3 = data4
                    rtpSize = pktSize

            del (self.param["inSize"])
        return (ret, enable_fec, data3, rtpSize)

    def encodeframe(self, data, i):
        ret = 0
        if len(data) > 0:
            self.param.update({"frame_idx": i})
            param_str = json2str(self.param)
            ###
            self.outparam[0] = b"test1"
            ###
            ret = self.load.lib.api_video_encode_one_frame(self.id, data, param_str, self.outbuf, self.outparam)

            #print("encodeframe: self.id= ", self.id)
            #print("encodeframe: outparam[0]= ", self.outparam[0])
            #print("encodeframe: ret= ", ret)
        return (ret, self.outbuf, self.outparam)
    def test_encode(self, yuvfilename, h264filename):
        sizelist = []
        streamlist = []
        rtplist = []
        #yuvfilename = '/data/home/gxh/works/datashare/for_ENC/352X288/foreman_cif.yuv'
        #h264filename = "test_py.h264"
        try:
            fout = open(h264filename, "wb")
        except:
            print("open file fail !")
            return
        print("start run test_encode")
        #id = 0
        (width, height) = (loadlib.WIDTH, loadlib.HEIGHT)
        (param, outbuf, outparam) = self.setparam()

        start_time = time.time()
        param_str = json2str(param)
        self.load.lib.api_video_encode_open(self.handle, param_str)
        difftime = time.time() - start_time
        print('{} test_encode:api_video_encode_open: time: {:.3f}ms'.format(time.ctime(), difftime * 1000))

        frame_size = int((width * height * 3) / 2)
        ###
        gop_size = self.gop_size
        refs = self.refs
        ###
        #outbuf = bytearray(frame_size)
        #content = content.ljust(frame_size) #填充空格
        #buffers.value = content #将数据写入
        with open(yuvfilename, 'rb') as fp:
            i = 0
            total_bytes = 0
            key_bytes = 0
            FB_bytes = 0
            while True:
                data = fp.read(frame_size)
                if len(data) > 0:
                    if i % gop_size:
                        I = (i % gop_size)
                        j = (I - 1) % (refs)
                        # I P0 P1 P2 P0 P1 P2 (ref=3)
                        ref_idx = j + 1
                        if I >= (int(gop_size / self.max_refs) * self.max_refs):
                            ref_idx = 1
                        elif (ref_idx & 1) == 0 and ref_idx != refs and refs > 2:
                            ref_idx = 1
                        elif ref_idx == (refs - 1) and (refs > 4):
                            ref_idx = 1
                        ref_idx0 = ref_idx
                        #ref_idx = 1
                    else:
                        ref_idx = 0
                        ref_idx0 = ref_idx
                        if i > 0:
                            self.refresh_idr = 0
                            param.update({"refresh_idr": self.refresh_idr})

                    print("test_encode: (i, ref_idx)= ", (i, ref_idx))
                    param.update({"frame_idx": i})
                    param.update({"seqnum": self.seqnum})
                    if ((refs & 1) == 0) and True:
                        param.update({"ref_idx": ref_idx})
                        if False:
                            if (ref_idx0 == 0) or (ref_idx0 == refs):
                                #param.update({"qmin": self.qp})  # 20
                                #param.update({"qmax": self.qp})  # 40
                                param.update({"qp": self.qp})
                            else:
                                #param.update({"qmin": (self.qp + 3)})  # 20
                                #param.update({"qmax": (self.qp + 3)})  # 40
                                param.update({"qp": (self.qp + 6)})

                    elif False:
                        if (ref_idx0 == 0):
                            param.update({"qp": self.qp})
                        elif ((i % gop_size) == 16):
                            param.update({"qp": (self.qp + 4)})
                        else:
                            param.update({"qp": (self.qp + 8)})
                    else:
                        param.update({"pic_type": ref_idx})

                    param_str = json2str(param)
                    ###
                    outparam[0] = b"test1"
                    ###
                    start_time = time.time()
                    ret = self.load.lib.api_video_encode_one_frame(self.handle, data, param_str, outbuf, outparam)
                    difftime = time.time() - start_time
                    ##print('{} test_encode time: {:.3f}ms'.format(time.ctime(), difftime * 1000))
                    print("outparam[0]= ", outparam[0])
                    #print("outparam[1]= ", outparam[1])
                    #print("len(s)= ", len(s))
                    print("ret= ", ret)
                    if ret > 0:
                        #sizelist.append(ret)
                        total_bytes += ret
                        if outparam[0] in ["PIC_TYPE_KEYFRAME"]:
                            key_bytes += ret
                            print("test_encode: I frame: (i, ref_idx)= ", (i, ref_idx))
                        elif (ref_idx0 == refs):
                            FB_bytes += ret
                        #print("outbuf.raw=", outbuf.raw)
                        data2 = outbuf.raw[:ret]
                        #test rtp
                        #outparam[0] = ""
                        #outparam[1] = ""
                        param.update({"insize": ret})
                        param_str = json2str(param)
                        #print("param_str= ", param_str)
                        ret2 = self.load.lib.api_raw2rtp_packet(self.handle, data2, param_str, outbuf, outparam)
                        #print("outparam[0]= ", outparam[0])
                        self.seqnum = int(outparam[1])
                        #rtpSize = outparam[0].split(b",")
                        rtpDict = str2json(self.outparam[0])
                        rtpSize = rtpDict.get("rtpSize")
                        #print("rtpSize= ", rtpSize)
                        #print("len(rtpSize)= ", len(rtpSize))
                        data3 = outbuf.raw[:ret2]
                        #print("self.enable_fec= ", self.enable_fec)
                        #print("self.loss_rate= ", self.loss_rate)
                        if self.enable_fec:
                            param.update({"enable_fec": self.enable_fec})
                            param.update({"loss_rate": float(self.loss_rate)})
                            param.update({"code_rate": float(self.code_rate)})
                            sizelist = []
                            for packet_size in rtpSize:
                                sizelist.append(int(packet_size))
                            #print("sizelist= ", sizelist)
                            del (param["insize"])
                            param.update({"inSize": sizelist})
                            param_str = json2str(param)
                            #print("param_str= ", param_str)
                            start_time = time.time()
                            ret = self.load.lib.api_fec_encode(self.handle, data3, param_str, outbuf, outparam)
                            difftime = time.time() - start_time
                            #print('{} test_encode: api_fec_encode time: {:.3f}ms'.format(time.ctime(), difftime * 1000))
                            #print("encode raw size= ", ret)
                            if ret > 0:
                                #print("encode raw size= ", ret)
                                self.seqnum = int(outparam[1])
                                #pktSize = outparam[0].split(b",")
                                rtpDict = str2json(self.outparam[0])
                                pktSize = rtpDict.get("rtpSize")

                                #loss packet
                                fake_loss_packet = True#False
                                fecOffset = 0
                                data4 = outbuf.raw[fecOffset:ret]
                                if fake_loss_packet:
                                    pass
                                    losslist = [0, 3, 5] #[]#[0]
                                    pktSize2 = []
                                    totlesize = 0
                                    losssize = 0
                                    l = 0
                                    for size in pktSize:
                                        if l not in losslist:
                                            totlesize += size
                                            pktSize2.append(size)
                                        else:
                                            losssize += size
                                        l += 1
                                    l = 0
                                    data_4 = b""
                                    for size in pktSize:
                                        if l not in losslist:
                                            data_4 += outbuf.raw[fecOffset:(fecOffset + size)]
                                        fecOffset += size
                                        l += 1
                                    data4 = data_4
                                    pktSize = pktSize2
                                    print("(ret, len(data4), losssize)= ", (ret, len(data4), losssize))
                                #if fake_loss_packet: #
                                #    fecOffset = pktSize[0]
                                #    del pktSize[0]
                                #print("pktSize= ", pktSize)
                                #print("len(pktSize)= ", len(pktSize))
                                #data4 = outbuf.raw[fecOffset:ret]
                                ###
                                if True:
                                    sizelist = []
                                    for packet_size in pktSize:
                                        sizelist.append(int(packet_size))
                                    param.update({"inSize": sizelist})
                                    param_str = json2str(param)
                                    #print("param_str= ", param_str)
                                    start_time = time.time()
                                    ret = self.load.lib.api_fec_decode(self.handle, data4, param_str, outbuf, outparam)
                                    difftime = time.time() - start_time
                                    print('{} test_encode: api_fec_decode time: {:.3f}ms'.format(time.ctime(),
                                                                                                 difftime * 1000))
                                    if ret > 0 and True:
                                        #print("decode raw size= ", ret)
                                        #pktSize = outparam[0].split(b",")
                                        rtpDict = str2json(self.outparam[0])
                                        pktSize = rtpDict.get("rtpSize")
                                        #print("pktSize= ", pktSize)
                                        #print("len(pktSize)= ", len(pktSize))
                                        data5 = outbuf.raw[:ret]

                                        #del(param["inSize"])
                                        #(param, outbuf, outparam) = self.setparam()

                                        data3 = data5
                                        rtpSize = pktSize
                            del(param["inSize"])

                        ###

                        #print("data2= ", data2)
                        #print("len(data2)= ", len(data2))
                        ###
                        #if (ref_idx0 == 0) or (ref_idx0 == refs):
                        if ref_idx0 not in [1,3,5,7]:
                        #if (ref_idx == 0):
                        #if i not in [1,4]:
                        #if True:
                            print("test_encode: write: (i, ref_idx)= ", (i, ref_idx))
                            rtplist.append((data3, rtpSize))
                            sizelist.append(ret)
                            streamlist.append(data2)
                            fout.write(data2)
                            if (ref_idx0 == refs) and False:
                                for l in range(refs - 1):
                                    sizelist.append(ret)
                                    fout.write(data2)
                            fout.flush()
                    i += 1
                else:
                    print("len(data)= ", len(data))
                    break
                #if i > 0:
                #    break
            rate = float(key_bytes) / float(total_bytes)
            rate2 = float(FB_bytes) / float(total_bytes)
            print("(rate, rate2)= ", (100 * rate, 100 * rate2))
        fp.close()
        fout.close()
        print("End run dll")
        print('***finish***')
        return (sizelist, streamlist, rtplist)

    def test_call(self):
        yuvfilename0 = loadlib.yuvfilename
        h264filename = "../mytest/test_py.h264"
        (sizelist,streamlist) = self.test_encode(yuvfilename0, h264filename)
    def test_call2(self):
        yuvfilename0 = loadlib.yuvfilename
        h264filename = "../mytest/test_py.h264"
        (sizelist,streamlist) = self.test_encode(yuvfilename0, h264filename)
        yuvfilename1 = '../mytest/out.yuv'
        h264filename = "../mytest/test_py.h264"
        from callvideodecode import CallVideoDecode
        self.decode = CallVideoDecode(1)
        self.decode.test_decode(yuvfilename1, h264filename, sizelist)
######################################
def show_yuv(yuvfilename0, yuvfilename1, flag, refs):
    import cv2
    from postprocess import PostProcess
    process = PostProcess(4)
    (w, h) = (loadlib.WIDTH, loadlib.HEIGHT)
    frame_size = int((w * h * 3) / 2)
    fp0 = open(yuvfilename0, 'rb')
    fp1 = open(yuvfilename1, 'rb')
    ###
    cv2.namedWindow("frame", cv2.WINDOW_NORMAL)
    cv2.moveWindow("frame", 10, 10)
    i = 0
    status = True
    while status:
        data0 = fp0.read(frame_size)
        data1 = fp1.read(frame_size)
        if len(data0) > 0 and len(data1) > 0:
            frame0 = np.frombuffer(data0, dtype=np.uint8)
            yuv_I420_0 = frame0.reshape(int((h * 3) / 2), w)
            img0 = cv2.cvtColor(yuv_I420_0, cv2.COLOR_YUV2BGR_I420)
            frame1 = np.frombuffer(data1, dtype=np.uint8)
            yuv_I420_1 = frame1.reshape(int((h * 3) / 2), w)
            img1 = cv2.cvtColor(yuv_I420_1, cv2.COLOR_YUV2BGR_I420)
            #img0 = process.DoProcess(img0)
            frame = img0
            if flag:
                frame = np.hstack([img0, img1])  # vstack #hstack
                frame = cv2.resize(frame, (frame.shape[1] << 1, frame.shape[0] << 1), interpolation=cv2.INTER_CUBIC)
            cv2.imshow("frame", frame)
            #cv2.waitKey(40)
        else:
            status = False
        ##keydown = cv2.waitKey(40 * refs) & 0xFF
        keydown = cv2.waitKey(0 * refs) & 0xFF
        if keydown == ord('q'):
            break
        elif keydown == ord('b'):
            fp0.seek(0, os.SEEK_SET)
            fp1.seek(0, os.SEEK_SET)
    cv2.destroyAllWindows()
    return

def show_yuv2(yuvfilename0, yuvfilename1, refs, sizelist0):
    import cv2
    (w, h) = (loadlib.WIDTH, loadlib.HEIGHT)
    frame_size = (w * h * 3) / 2
    fp0 = open(yuvfilename0, 'rb')
    fp1 = open(yuvfilename1, 'rb')
    totle_bytes = 0
    lost_bytes = 0
    i = 0
    skip_num = 1
    skiplists = [
        [8], #16 / 2 = 8
        [5, 10], # 16 / 3 = 5
        [4, 8, 12], # 16 / 4 = 4
        [3, 6, 9, 12], # 16 / 5 = 3
        [3, 6, 9, 12, 15], # 16 / 6 = 2
        [2, 4, 7, 9, 12, 14], # 16 / 7 = 2
        [2, 4, 6, 8, 10, 12, 14], # 16 / 8 = 2
        [1, 3, 5, 7, 9, 11, 13, 15], # 8 + 1
        [1, 2, 3, 5, 7, 9, 11, 13, 15], # 9 + 1
        [1, 2, 4, 5, 7, 9, 11, 12, 14, 15],  # 10 + 1
        [1, 2, 4, 5, 7, 8, 10, 11, 13, 14, 15],  # 11 + 1
        [1, 2, 3, 5, 6, 7, 9, 10, 11, 13, 14, 15],  # 12 + 1
        [1, 2, 3, 4, 6, 7, 8, 9, 11, 12, 13, 14, 15],  # 13 + 1
        [1, 2, 3, 4, 5, 6, 7, 9, 10, 11, 12, 13, 14, 15],  # 14 + 1
        [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15],  # 15 + 1
        [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16],  # 15 + 1
    ]
    skiplists = [
        [7,8],
        [5,6, 11,12],
        [3,4, 7,8, 11,12],
        [1,2, 5,6, 9,10, 13,14,15],
        [1,2,3,4, 7,8,9,10, 13,14,15],
        [1,2,3,4,5,6, 9,10,11,12,13,14,15],
        [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16],
    ]
    skiplist = skiplists[skip_num - 1]
    #step = refs / (skip_num + 1) # + (1 if refs % (skip_num + 1) else 0)
    #for j in range(step + 1, (refs + 1), step): #9,17
    #    skiplist.append(j)
    print("skiplist= ", skiplist)
    gop_size = 50
    #refs = 8#4
    frame_idx = 0
    last_imag = None
    status = True
    pause = False
    while status:
        I = (i % gop_size)
        if i % gop_size:
            I = (i % gop_size)
            j = (I - 1) % (refs)
            # I P0 P1 P2 P0 P1 P2 (ref=3)
            ref_idx = j + 1
        else:
            flag = 0
            ref_idx = 0
        data0 = fp0.read(frame_size)
        data1 = fp1.read(frame_size)
        if len(data0) > 0 and len(data1) > 0:
            frame0 = np.frombuffer(data0, dtype=np.uint8)
            yuv_I420_0 = frame0.reshape((h * 3) / 2, w)
            img0 = cv2.cvtColor(yuv_I420_0, cv2.COLOR_YUV2BGR_I420)
            totle_bytes += sizelist0[i]
            if (ref_idx == 0): # or (ref_idx == refs):
                pass
            else:
                if ref_idx in skiplist:
                    img0 = last_imag
                    lost_bytes += sizelist0[i]
            frame1 = np.frombuffer(data1, dtype=np.uint8)
            yuv_I420_1 = frame1.reshape((h * 3) / 2, w)
            img1 = cv2.cvtColor(yuv_I420_1, cv2.COLOR_YUV2BGR_I420)
            frame = np.hstack([img0, img1])  # vstack #hstack
            frame = img0 #
            #frame = cv2.resize(frame, (frame.shape[1] << 1, frame.shape[0] << 1), interpolation=cv2.INTER_CUBIC)
            cv2.imshow("frame", frame)
            last_imag = img0
        else:
            rate = float(lost_bytes) / float(totle_bytes)
            print("rate= ", rate * 100)
        i += 1

        if not (len(data0) > 0 and len(data1) > 0) and not pause:
            skip_num += 1
            if skip_num >= (refs / 2):
                skip_num = 1
                keydown = cv2.waitKey(0) & 0xFF
            skiplist = skiplists[skip_num - 1]
            print("(skip_num, skiplist)= ", (skip_num, skiplist))
            fp0.seek(0, os.SEEK_SET)
            fp1.seek(0, os.SEEK_SET)
            i = 0
        keydown = cv2.waitKey(40) & 0xFF
        if keydown == ord('q'):
            break
        elif keydown == ord('a'):
            pause = False
            skip_num += 1
            if skip_num >= (refs / 2):
                skip_num = 1
            #skiplist = []
            #step = refs / (skip_num + 1) # + (1 if refs % (skip_num + 1) else 0)
            #for j in range(step + 1, (refs + 1), step): #
            #    skiplist.append(j)
            skiplist = skiplists[skip_num - 1]
            print("(skip_num, skiplist)= ", (skip_num, skiplist))
            fp0.seek(0, os.SEEK_SET)
            fp1.seek(0, os.SEEK_SET)
            totle_bytes = 0
            lost_bytes = 0
            i = 0
        elif keydown == ord('2'):
            pause = True
            skip_num = 2
            skiplist = skiplists[skip_num - 1]
            print("(skip_num, skiplist)= ", (skip_num, skiplist))
            fp0.seek(0, os.SEEK_SET)
            fp1.seek(0, os.SEEK_SET)
            totle_bytes = 0
            lost_bytes = 0
            i = 0
        elif keydown == ord('5'):
            pause = True
            skip_num = 5
            skiplist = skiplists[skip_num - 1]
            print("(skip_num, skiplist)= ", (skip_num, skiplist))
            fp0.seek(0, os.SEEK_SET)
            fp1.seek(0, os.SEEK_SET)
            totle_bytes = 0
            lost_bytes = 0
            i = 0
        elif keydown == ord('6'):
            pause = True
            skip_num = 6
            skiplist = skiplists[skip_num - 1]
            print("(skip_num, skiplist)= ", (skip_num, skiplist))
            fp0.seek(0, os.SEEK_SET)
            fp1.seek(0, os.SEEK_SET)
            totle_bytes = 0
            lost_bytes = 0
            i = 0
        elif keydown == ord('7'):
            pause = True
            skip_num = 7
            skiplist = skiplists[skip_num - 1]
            print("(skip_num, skiplist)= ", (skip_num, skiplist))
            fp0.seek(0, os.SEEK_SET)
            fp1.seek(0, os.SEEK_SET)
            totle_bytes = 0
            lost_bytes = 0
            i = 0
        elif keydown == ord('b'):
            fp0.seek(0, os.SEEK_SET)
            fp1.seek(0, os.SEEK_SET)
            totle_bytes = 0
            lost_bytes = 0
            i = 0

    return

def test_call():
        yuvfilename0 = loadlib.yuvfilename
        h264filename0 = "../mytest/test_py.h264"
        encode0 = CallVideoEncode(0)
        (sizelist0,streamlist0) = encode0.test_encode(yuvfilename0, h264filename0)
        ###
        h264filename1 = "../mytest/test_py_1.h264"
        encode1 = CallVideoEncode(2)
        qp = encode1.qp
        encode1.qp = qp + (6+3)
        (sizelist1,streamlist1) = encode1.test_encode(yuvfilename0, h264filename1)
        ###
        yuvfilename1 = '../mytest/out.yuv'
        from callvideodecode import CallVideoDecode
        decode0 = CallVideoDecode(1)
        decode0.test_decode(yuvfilename1, h264filename0, sizelist0)
        ###
        yuvfilename2 = '../mytest/out_1.yuv'
        decode1 = CallVideoDecode(3)
        decode1.test_decode(yuvfilename2, h264filename1, sizelist1)

def test_call2(refs):

    streamlist1 = []
    yuvfilename0 = loadlib.yuvfilename
    h264filename0 = "../../mytest/test_py.h264"
    (width, height) = (loadlib.WIDTH, loadlib.HEIGHT)
    encode0 = CallVideoEncode(0, width, height)
    encode0.refs = refs #2#4#8#16#1#2#4#8#16#1#16
    #encode0.bit_rate = 2000*1024
    #encode0.bit_rate = 10 * 1024 * 1024
    (sizelist0, streamlist0, rtplist0) = encode0.test_encode(yuvfilename0, h264filename0)
    #return
    ###
    yuvfilename1 = '../../mytest/out.yuv'
    from callvideodecode import CallVideoDecode
    decode0 = CallVideoDecode(1, width, height)
    decode0.test_decode2(yuvfilename1, streamlist0, streamlist1, rtplist0)
    ###
    if False:
        h264filename1 = "../../mytest/test_py_1.h264"
        encode1 = CallVideoEncode(2)
        encode1.refs = 16#1#16
        qp = encode1.qp
        encode1.qp = qp + (6 + 3)
        (sizelist1, streamlist1) = encode1.test_encode(yuvfilename0, h264filename1)
        ###
        yuvfilename2 = '../../mytest/out_1.yuv'
        decode1 = CallVideoDecode(3)
        decode1.test_decode2(yuvfilename2, streamlist1, streamlist0)
    print("ready to show")
    ##show_yuv2(yuvfilename1, yuvfilename1, encode0.refs, sizelist0)
    #show_yuv(yuvfilename1, yuvfilename1, False, refs)
    show_yuv(yuvfilename1, yuvfilename1, False, 1)
    print("start enc close")
    encode0.load.lib.api_video_encode_close(0)
    print("start dec close")
    decode0.load.lib.api_video_decode_close(1)
def cut_stream(yuvfilename0, yuvfilename2, streamlist0, streamlist1, streamlist2):
    if True:
        try:
            fout0 = open(yuvfilename0, "wb")
            fout2 = open(yuvfilename2, "wb")
        except:
            print("open file fail !")
            return
        print("start run test_decode")
        ###
        from callvideodecode import CallVideoDecode
        decode0 = CallVideoDecode(1)
        decode1 = CallVideoDecode(3)
        decode2 = CallVideoDecode(5)
        ###
        (param0, outbuf0, outparam0) = decode0.setparam2()
        (param1, outbuf1, outparam1) = decode1.setparam2()
        (param2, outbuf2, outparam2) = decode2.setparam2()
        param0.update({"insize": 0})
        param1.update({"insize": 0})
        param2.update({"insize": 0})
        start_time = time.time()
        param_str0 = json2str(param0)
        param_str1 = json2str(param1)
        param_str2 = json2str(param2)
        decode0.load.lib.decode_open2(decode0.id, param_str0)
        decode1.load.lib.decode_open2(decode1.id, param_str1)
        decode2.load.lib.decode_open2(decode2.id, param_str2)
        difftime = time.time() - start_time
        print('{} test_decode:decode_open2: time: {:.3f}ms'.format(time.ctime(), difftime * 1000))

        gop_size = 50
        refs = 4
        frame_idx = 0
        flag = 0
        flag0 = 0
        for i in range(len(streamlist0)):
            I = (i % gop_size)
            if i % gop_size:
                I = (i % gop_size)
                j = (I - 1) % (refs)
                # I P0 P1 P2 P0 P1 P2 (ref=3)
                ref_idx = j + 1
            else:
                flag = 0
                ref_idx = 0
                ref_idx0 = ref_idx
            data0 = streamlist0[i]
            frame_size0 = len(data0)
            data1 = streamlist1[i]
            frame_size1 = len(data1)
            data2 = streamlist2[i]
            frame_size2 = len(data2)
            flag0 = 0
            #if (i % gop_size) in [1,2,3,4]:
            if ref_idx in [1] and (i % gop_size) in [20,21,22,23,24] and (i / gop_size) in [0,4,5]:
                flag = 1
                flag0 = flag
            elif ref_idx in [1] and (i % gop_size) in [0,1,2,3,4] and (i / gop_size) in [1,5]:
                flag = 2
                flag0 = flag
            elif ref_idx in [4] and (i % gop_size) in [0,1,2,3,4] and (i / gop_size) in [2]:
                flag = 3
                flag0 = flag
            elif ref_idx in [0] and (i % gop_size) in [0,1,2,3,4] and (i / gop_size) in [3]:
                flag = 4
                flag0 = flag
            print("cut_stream: (i, flag)= ", (i, flag))
            if True:
                if len(data0) > 0:
                    if flag in [1, 2]:
                        frame_size0 = frame_size1
                        data0 = data1
                    #test
                    if flag in [3, 4]:
                        frame_size0 = frame_size1
                        data0 = data1

                    param0.update({"insize": frame_size0})
                    param_str0 = json2str(param0)

                    param1.update({"insize": frame_size1})
                    param_str1 = json2str(param1)

                    param2.update({"insize": frame_size2})
                    param_str2 = json2str(param2)

                    start_time = time.time()
                    ret0 = decode0.load.lib.api_video_decode_one_frame(decode0.id, data0, param_str0, outbuf0, outparam0)
                    ret1 = decode1.load.lib.api_video_decode_one_frame(decode1.id, data1, param_str1, outbuf1,
                                                                       outparam1)
                    if flag0:
                        print("#################################lost ", outparam1[0])
                    else:
                        ret2 = decode2.load.lib.api_video_decode_one_frame(decode2.id, data2, param_str2, outbuf2,
                                                                       outparam2)
                    difftime = time.time() - start_time
                    #print('{} test_decode time: {:.3f}ms'.format(time.ctime(), difftime * 1000))
                    #print("outparam[0]= ", outparam0[0])
                    #print("ret= ", ret0)
                    #print("frame_idx= ", frame_idx)
                    data3 = outbuf0.raw[:ret0]
                    data4 = outbuf1.raw[:ret1]
                    data5 = outbuf2.raw[:ret2]
                    ###复制補償數據
                    if flag in [3, 4]:
                        fout0.write(data4)
                    else:
                        fout0.write(data3)
                    fout0.flush()
                    ###
                    if flag0:
                        pass
                    else:
                        fout2.write(data5)
                        fout2.flush()
                    ###

                    frame_idx += 1
                    if flag in [3,4]:
                        pass
                        if flag in [3]:
                            flag = 0 #test
                    else:
                        flag = 0
    return




def test_cut_stream():
    yuvfilename0 = loadlib.yuvfilename
    h264filename0 = "../mytest/test_py.h264"
    encode0 = CallVideoEncode(0)
    (sizelist0, streamlist0) = encode0.test_encode(yuvfilename0, h264filename0)

    ###
    h264filename1 = "../mytest/test_py_1.h264"
    encode1 = CallVideoEncode(2)
    qp = encode1.qp
    encode1.qp = qp + (6 + 3)
    (sizelist1, streamlist1) = encode1.test_encode(yuvfilename0, h264filename1)
    ###
    h264filename2 = "../mytest/test_py_2.h264"
    encode2 = CallVideoEncode(4)
    encode2.refs = 1
    (sizelist2, streamlist2) = encode2.test_encode(yuvfilename0, h264filename2)
    ###
    yuvfilename1 = '../mytest/out_cut.yuv'
    yuvfilename2 = '../mytest/out.yuv'
    cut_stream(yuvfilename1, yuvfilename2, streamlist0, streamlist1, streamlist2)
    show_yuv(yuvfilename1, yuvfilename2)

if __name__ == '__main__':
    print('Start pycall.')
    #call = CallVideoEncode(0)
    #call.test_call2()
    #for refs in [2,4,8,16]:
    #    test_call2(refs)
    test_call2(16)
    #test_cut_stream()
    print('End pycall.')
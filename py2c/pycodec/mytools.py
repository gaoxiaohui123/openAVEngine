# -*- coding: utf-8 -*


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
import shutil
import time
#import cv2

#import matplotlib.pyplot as plt # plt 用于显示图片
#from matplotlib import pyplot as plt
#import matplotlib.image as mpimg # mpimg 用于读取图片
import numpy as np
#from PIL import Image
import math
import threading

import ctypes
from ctypes import *
import json
#from pcapng import FileScanner
#import scapy
#from scapy.all import *
#from scapy.utils import PcapReader
#import dpkt
import socket
import json
import binascii
#from bitarray import bitarray


def encode(s): #str2bin
    return ' '.join([bin(ord(c)).replace('0b', '') for c in s])


def decode(s): #bin2str
    return ''.join([chr(i) for i in [int(b, 2) for b in s.split(' ')]])

def str2bin(s):
    data = []
    for i in range(len(s)):
        data.append(ord(s[i]))
    return data

'''
typedef struct
{
    /**//* byte 0 */
    unsigned char csrc_len:4;        /**//* expect 0 */
    unsigned char extension:1;        /**//* expect 1, see RTP_OP below */
    unsigned char padding:1;        /**//* expect 0 */
    unsigned char version:2;        /**//* expect 2 */
    /**//* byte 1 */
    unsigned char payload:7;        /**//* RTP_PAYLOAD_RTSP */
    unsigned char marker:1;        /**//* expect 1 */
    /**//* bytes 2, 3 */
    unsigned short seq_no;
    /**//* bytes 4-7 */
    unsigned  int timestamp;
    /**//* bytes 8-11 */
    unsigned int ssrc;            /**//* stream number is used here. */
} RTP_FIXED_HEADER;
'''
class RtpPy():
    def __init__(self):
        self.payloadtype = 0
    def GetPayLoadType(self, data):
        #print(repr(data))
        #print(ord(data[1]))
        #print(data[1])
        self.payloadtype = int(data[1]) & 0x7F
        return self.payloadtype
    def GetSeqNum(self, data):
        #seqnum = int(data[2]) | (int(data[3]) << 8)
        seqnum = int(data[3]) | (int(data[2]) << 8)
        #print((seqnum))
        #print(len(seqnum))
        self.seqnum = int(seqnum) & 0xFFFF
        return self.seqnum
    def GetSsrc(self, data):
        #Ssrc = long(data[8]) | (long(data[9]) << 8) | (long(data[10]) << 16) | (long(data[11]) << 24)
        Ssrc = long(data[11]) | (long(data[10]) << 8) | (long(data[9]) << 16) | (long(data[8]) << 24)
        return Ssrc

class MyTools():
    def __init__(self):
        self.filename = r"/home/gxh/works/recv_pkt_wrtc_gxh_735296241.txt"
        self.fout = None #open(yuvfilename, "wb")
    def save2file(self, ssrc, seqnum):
        if self.fout == None:
            outfile = '/home/gxh/works/wireshark_' + str(ssrc) + '.txt'
            self.fout = open(outfile, "wb")
        self.fout.write(str(seqnum) + " \n")
        self.fout.flush()
    def readlinefile(self, filename):
        if filename == "":
            filename = self.filename
        ret_list = []
        this_list = []
        # with open(filename, encoding='utf-8') as f: #从TXT文件中读出数据
        with open(filename, 'r') as f:
            for line1 in f:
                this_list.append(line1)  # 通过for循环一行一行加载
        #print("readlinefile: this_list= ", this_list)
        datalist = []  # 定义一个数组
        for item in this_list:  # 通过一个for循环将每一行按照空格分成不同的字符段
            # l = item.split("\t")  #这句使用空格将item分割成字符段
            l = item.split("\n")
            l[0] = l[0].replace(' ', '')
            # l[0].replace('\t', '')
            # l[0].replace('\n', '')
            datalist.append(l[0])  # 将l放入数组
        #print("readlinefile: datalist= ", datalist)
        for item in datalist:
            seqnum = int(item)
            ret_list.append(seqnum)
            #if seqnum not in ret_list:
            #    ret_list.append(seqnum)
            #else:
            #    print("repeat: seqnum= ", seqnum)
        #print("readlinefile: ret_list= ", ret_list)
        return ret_list
    def rtpanalize(self, rtplist):
        lossnum = 0
        interval = 0
        count = 0
        #last_seqnum = -1
        #first_seqnum = -1
        #min_seqnum = 1 << 31
        #max_seqnum = -1
        seqnumsum = 0
        #offset = 0
        self.last_seqnum = -1
        self.offset = 0
        self.min_seqnum = 1 << 31
        self.max_seqnum = -1

        for seqnum in rtplist:
            flag = 0
            this_loss_num = 0
            if self.last_seqnum >= 0:
                diffnum = seqnum - self.last_seqnum
                if (self.last_seqnum > (1 << 15)) and (seqnum < (1 << 15)):
                    diffnum = seqnum + ((1 << 16) - 1) - self.last_seqnum
                if diffnum > 0:
                    this_loss_num = (diffnum - 1)
                    flag = this_loss_num
                else:
                    this_loss_num = (-diffnum)
                    flag = -1
            interval += 1
            lossnum += this_loss_num
            if flag:
                if flag > 0:
                    print(
                    "rtpanalize: (count, self.last_seqnum, seqnum, this_loss_num, interval)= ", (count, self.last_seqnum, seqnum, this_loss_num,interval))
                else:
                    print("rtpanalize: 2:       (count, self.last_seqnum, seqnum, this_loss_num, interval)= ",
                          (count, self.last_seqnum, seqnum, this_loss_num, interval))
                #print("rtpanalize: (self.last_seqnum, seqnum, interval)= ", (self.last_seqnum, seqnum, interval))
                count += 1
                interval = 0

            correct_seqnum = self.correct_seqnum(seqnum)

        seqnumsum += self.max_seqnum - self.min_seqnum + 1
        diffnum = len(rtplist)#last_seqnum - first_seqnum
        print("rtpanalize: rtplist[len(rtplist) - 1]= ", rtplist[len(rtplist) - 1])
        print("rtpanalize: (lossnum, len(rtplist))= ", (lossnum, len(rtplist)))
        loss_rate = 100 * float(lossnum) / float(len(rtplist))
        print('rtpanalize: loss_rate= {:.3f} %'.format(loss_rate))
        loss_rate3 = 100 * (1.0 - float(diffnum) / float(seqnumsum))
        print('rtpanalize: loss_rate3= {:.3f} %'.format(loss_rate3))

        return lossnum
    #def read_pcap_0(self, filename):
    #    with open(filename) as fp:
    #        scanner = FileScanner(fp)
    #        for block in scanner:
    #            pass  # do something with the block...
    #            print("block=", block)
    def correct_seqnum(self, seqnum):
        self.stride_ushort = (1 << 16)
        self.half_ushort = (1 << 15)
        self.quart_ushort = (1 << 14)
        #self.last_seqnum = -1
        #self.offset = 0
        #self.min_seqnum = 1 << 31
        #self.max_seqnum = -1

        if self.last_seqnum < 0:
            self.min_seqnum = seqnum
            self.max_seqnum = seqnum
            correct_seqnum = seqnum
        else:
            if seqnum < self.half_ushort and self.last_seqnum > self.half_ushort:
                # 切变
                print("self.max_seqnum= ", self.max_seqnum)
                if (self.max_seqnum % self.stride_ushort) > self.half_ushort:
                    # 近期首次切变
                    self.offset += self.stride_ushort
                    print("self.offset= ", self.offset)
            correct_seqnum = seqnum + self.offset
            if (seqnum > self.half_ushort) and ((self.max_seqnum % self.stride_ushort) < self.quart_ushort):
                # 近期刚发生切变
                correct_seqnum -= self.stride_ushort
                print("self.offset= ", self.offset)
                print("self.max_seqnum= ", self.max_seqnum)
                print("seqnum= ", seqnum)

            if correct_seqnum < self.min_seqnum:
                self.min_seqnum = correct_seqnum
            if correct_seqnum > self.max_seqnum:
                self.max_seqnum = correct_seqnum
        self.last_seqnum = seqnum
        return correct_seqnum
    def read_pcap(self, filename):
        print("start read ")
        rtp = RtpPy()
        packets = rdpcap(filename)  # mac路径
        print("read file ok")
        packetlist = []
        dportlis = []
        count = 0
        for data in packets:
            print(data.payload.name)  # 打印出'IP','IPV6','ARP'或者其他
            #print("data=",data)
            dport = 0
            if 'UDP' in data:
                if count == 0:
                    s = repr(data) #二进制显示为字符串
                    print(s)
                flag = True
                if flag:
                    print(data['Ether'].type)
                    print(data['IP'].len)
                    print("sport= ", data['UDP'].sport)
                    print("dport= ", data['UDP'].dport)
                    print(data['UDP'].len)
                dport = data['UDP'].dport
                #if dport not in dportlis:
                #    dportlis.append(dport)
                payload = data['Raw'].load
                if 'rtt' in payload:
                    flag = False
                    if flag:
                        try:
                            jsondata = json.loads(payload, encoding='utf-8')
                        except:
                            try:
                                jsondata = json.loads(payload)
                            except:
                                print("json.laods fail")
                        rtt = jsondata['rtt']
                        print("rtt= ", rtt)
                else:
                    if True: #dport == 51040:
                        raw = str2bin(payload)
                        payloadtype = rtp.GetPayLoadType(raw)
                        print("payloadtype= ", payloadtype)
                        if payloadtype in [96, 98, 105, 100]: #, 100
                            if dport not in dportlis:
                                dportlis.append(dport)
                            if dport in [10070] and True: #[37669, 52135, 36822] #[51043, 51042]
                                packetlist.append(raw)

                    pass
                count += 1
            #if count > 3:
            #    break
        print("dportlis= ", dportlis)
        if len(packetlist) == 0:
            return

        loss_num = 0
        first_seqnum = -1
        Ssrc = 0
        seqnumlist = []
        seqnumsum = 0
        self.last_seqnum = -1
        self.min_seqnum = 1 << 31
        self.max_seqnum = -1
        self.stride_ushort = (1 << 16)
        self.half_ushort = (1 << 15)
        self.quart_ushort = (1 << 14)
        self.offset = 0
        for data in packetlist:
            print("len(data)= ", len(data))
            payloadtype = rtp.GetPayLoadType(data)
            #print("payloadtype= ", payloadtype)
            seqnum = rtp.GetSeqNum(data)
            #print("seqnum= ", seqnum)
            Ssrc = rtp.GetSsrc(data)
            #print("Ssrc= ", Ssrc)
            self.save2file(Ssrc, seqnum)
            if first_seqnum < 0:
                first_seqnum = seqnum
            diffnum = seqnum - self.last_seqnum
            if(self.last_seqnum >= 0) and (diffnum != 1):
                if diffnum < 0:
                    #print("diffnum= ", diffnum)
                    pass
                else:
                    if diffnum > 1:
                        #print("diffnum= ", diffnum)
                        if diffnum > 10:
                            #print("self.last_seqnum= ", self.last_seqnum)
                            #print("seqnum= ", seqnum)
                            pass
                        loss_num += diffnum - 1

            correct_seqnum = self.correct_seqnum(seqnum)
            #print("correct_seqnum= ", correct_seqnum)
            if correct_seqnum not in seqnumlist: #超过一个循环则失效
                seqnumsum += 1
            else:
                #print("repeat: correct_seqnum= ", correct_seqnum)
                pass
            seqnumlist.append(correct_seqnum)
        loss_rate = 100 * float(loss_num) / float(len(packetlist))
        diffnum = self.max_seqnum - self.min_seqnum + 1 #last_seqnum - first_seqnum
        loss_rate2 = 100 * float(diffnum - len(packetlist)) / float(diffnum)
        loss_rate3 = 100 * (1.0 - float(seqnumsum) / float(diffnum))
        print("(seqnumsum, diffnum)= ", (seqnumsum, diffnum))
        print("(max_seqnum, min_seqnum)= ", (self.max_seqnum, self.min_seqnum))
        print('read_pcap: loss_rate= {:.3f} %'.format(loss_rate))
        print('read_pcap: loss_rate2= {:.3f} %'.format(loss_rate2))
        print('read_pcap: loss_rate3= {:.3f} %'.format(loss_rate3))
        print("len(packetlist)= ", len(packetlist))
        print("diffnum= ", diffnum)
        print("Ssrc= ", Ssrc)
        #print("seqnumlist=", seqnumlist)
        return

if __name__ == '__main__':
    print('Start MyTools.')
    case = MyTools()
    filename = r"/home/gxh/works/recv_pkt_wrtc_udp_gxh_883247981.txt"
    rtplist = case.readlinefile(filename)
    case.rtpanalize(rtplist)
    filename = "/home/gxh/works/datashare/wireshar_udp_8.pcapng"
    #case.read_pcap(filename)
    print('End MyTools.')
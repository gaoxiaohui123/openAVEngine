#!/usr/bin/env python
# -*- coding:utf8 -*-

import sys

# for python2
if sys.version_info >= (3, 0):
    pass
else:
    try:
        print (sys.getdefaultencoding())
        reload(sys)
        sys.setdefaultencoding('utf-8')
        print (sys.getdefaultencoding())
    except:
        pass

import socket
import threading
import time
import signal
import json
from ctypes import *
import random
import numpy as np
import errno
from udpserver import EchoServerThread as videoserver
from udpaudioserver import EchoServerThread as audioserver


global_port0 = 10088 #8097
global_port1 = 10089 #8098
global_host = 'localhost'
#global_host = '172.16.0.17'
#global_host = '111.230.226.17'
print("len(sys.argv)= ", len(sys.argv))
if len(sys.argv) > 3:
    global_port1 = int(sys.argv[3])
if len(sys.argv) > 2:
    global_port0 = int(sys.argv[2])
if len(sys.argv) > 1:
    global_host = sys.argv[1]


def RunServer(flag):
    if flag:
        # idx = raw_input('please input to exit(eg: -1 ): ')
        # count = 0
        idx = 0
        (thread0, thread1) = (None, None)

        thread0 = videoserver(0, global_host, global_port0)
        thread1 = audioserver(0, global_host, global_port1)

        if thread0 != None:
            thread0.start()
        if thread1 != None:
            thread1.start()
        while idx >= 0 and idx < 4:
            if sys.version_info >= (3, 0):
                idx = int(input('please input to exit(eg: 0 ): '))
            else:
                idx = int(raw_input('please input to exit(eg: 0 ): '))
            print("idx= ", idx)
            #thread.status = False if idx == 0 else True
        print("main: start stop...")
        if thread0 != None:
            thread0.stop()
        if thread1 != None:
            thread1.stop()
        #thread.join()

if __name__ == "__main__":
    print("start server")
    #udpServer = UdpServer()
    #udpServer.tcpServer()
    RunServer(True)
    print("end server")
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
import loadserver


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

def RunServer():
    print("RunServer: global_host=", global_host)
    handle_size = 8
    handle = []
    handle.append(create_string_buffer(handle_size))
    handle.append(create_string_buffer(handle_size))
    vtaskId = 0
    loadserver.gload.lib.pool_start_server(handle[0], vtaskId, global_port0, global_host.encode('utf-8'))
    ataskId = 1
    loadserver.gload.lib.pool_start_server(handle[1], ataskId, global_port1, global_host.encode('utf-8'))
    idx = 0
    while idx >= 0 and idx < 4:
        if sys.version_info >= (3, 0):
            idx = int(input('please input to exit(eg: 0 ): '))
        else:
            idx = int(raw_input('please input to exit(eg: 0 ): '))
        print("idx= ", idx)
        #thread.status = False if idx == 0 else True
    print("main: start stop...")
    loadserver.gload.lib.api_stop_server_task(handle[0], vtaskId)
    loadserver.gload.lib.api_stop_server_task(handle[1], ataskId)
    loadserver.gload.lib.pool_stop_server(handle[0])
    loadserver.gload.lib.pool_stop_server(handle[1])
if __name__ == "__main__":
    print("start server")
    RunServer()
    print("end server")
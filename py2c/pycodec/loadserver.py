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

import os
import socket
import threading
import time
import signal
import json
#from ctypes import *
import ctypes
from ctypes import *
from ctypes import c_longlong as ll
import random
import numpy as np
import errno
import platform

gload = None

class LoadLib(object):
    def __init__(self):
        ll = ctypes.cdll.LoadLibrary
        thispath = os.path.abspath('./udpserver.py')
        print("thispath= ", thispath)
        print("LoadLib: platform.system()= ", platform.system())
        current_working_dir = os.getcwd()
        if (platform.system() == 'Windows'):
            print('Windows系统')
            dllfile = os.path.join(current_working_dir, "librtpserver.dll")
            try:
                self.lib = ll(dllfile)
            # except IOError, error: #python2
            except IOError as error:  # python3
                print("LoadLib: error=", error)
            else:
                pass
        elif (platform.system() == 'Linux'):
            print('Linux系统')
            try:
                self.lib = ll("./librtpserver.so")
            # except IOError, error: #python2
            except IOError as error:  # python3
                print("LoadLib: error=", error)
                self.lib = ll("../librtpserver.so")
        else:
            print('其他')

gload = LoadLib()

if __name__ == "__main__":
    print("start server")
    #udpServer = UdpServer()
    #udpServer.tcpServer()
    RunServer(True)
    print("end server")
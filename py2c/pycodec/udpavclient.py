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

import os
import time

import udpbase as base
import udpvclient
import udpaclient

#python udpavclient.py 1 0 1 2 10.184.101.78 1108
#python udpavclient.py 1 1 1 2 10.184.101.78 1108

def RunSession(base_idx, proc_idx, speakernum, host, port):
    (thread_show0, threadlist0) = udpvclient.RunSessionStart(base_idx, proc_idx, speakernum, host, port)
    (thread_show1, threadlist1) = udpaclient.RunSessionStart(base_idx, proc_idx, speakernum, host, port)
    idx = 0
    while idx >= 0 and idx < 4:
        try:
            idx = int(raw_input('please input to exit(eg: 0 ): '))
        except:
            idx = int(input('please input to exit(eg: 0 ): '))
        print("idx= ", idx)
    udpvclient.RunSessionStop(thread_show0, threadlist0)
    udpaclient.RunSessionStop(thread_show1, threadlist1)

if __name__ == "__main__":
    print("start client")
    if True:
        print('父进程%d.' % os.getpid())
        # p = Process(target=RunSession, args=('test',))
        # print('子进程将要执行')
        # p.start()
        po = base.Pool(base.global_procnum)  # 定义一个进程池，最大进程数3
        for i in range(base.global_procnum):
            # Pool.apply_async(要调用的目标,(传递给目标的参数元祖,))
            # 每次循环将会用空闲出来的子进程去调用目标
            # po.apply_async(worker, (i, 1, 2, "strname"))
            port = base.global_port
            #if base.MULT_SERVER:
            port = base.global_port + (i << 1)  # (video, audio)
            po.apply_async(RunSession, (base.global_baseidx, (i >> 1), base.global_speakernum, base.global_host, port))
        idx = 0
        while idx >= 0 and idx < 4:
            try:
                idx = int(raw_input('please input to exit(eg: 0 ): '))
            except:
                idx = int(input('please input to exit(eg: 0 ): '))
            print("idx= ", idx)
            # thread.status = False if idx == 0 else True
        print("main: start stop...")
        print("----start----")
        po.close()  # 关闭进程池，关闭后po不再接收新的请求
        po.join()  # 等待po中所有子进程执行完成，必须放在close语句之后
    print("end client")

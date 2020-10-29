# -*- coding: utf-8 -*


import sys
import os
import shutil
import time
#import cv2

#import matplotlib.pyplot as plt # plt 用于显示图片
#from matplotlib import pyplot as plt
#import matplotlib.image as mpimg # mpimg 用于读取图片
#import numpy as np
#from PIL import Image

import ctypes


class PyCallclass(object):
    def __init__(self):
        pass
    def loadlib(self):
        so = ctypes.cdll.LoadLibrary
        lib = so("./libpycallclass.so")
        print('display()')
        lib.display()

        ret = lib.display_int(100)
        print('ret= ', ret)


if __name__ == '__main__':
    print('Start pycall.')
    call = PyCallclass()
    call.loadlib()
    print('End pycall.')
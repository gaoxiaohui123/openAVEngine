# -*- coding: utf-8 -*


import sys
import os
import io
from ctypes import *
import json
import loadlib
import threading
import time
from mysdl import *
import loadlib

from sdl2 import *
import Tkinter as tk
from Tkinter import *
import random, ctypes
# import Tkinter.messagebox
import pickle
# from tux import Image_sys   #调用文件tux
#from PIL import ImageTk, Image
import ttk
import tkSimpleDialog as dl
import tkMessageBox as mb


#refer to:
# https://blog.csdn.net/ahilll/article/details/81531587
# https://vlight.me/2017/12/04/Layout-Management/

class JsonFile(object):
    def __init__(self, filename):
        self.filename = filename
        self.dict = {}
    def writefile(self, data0, output_json_file):
        data = self.dict
        if output_json_file != "":
            self.filename = output_json_file
        if data0 != None:
            data = data0
        flag = 0  # -1
        if flag == 0:
            json_str = json.dumps(data, encoding='utf-8', ensure_ascii=False, indent=4, sort_keys=True)
            thisfile = open(self.filename, "w")
            thisfile.write(json_str)
            thisfile.flush()
        elif flag == 1:
            json.dump(data, io.open(self.filename, 'w', encoding='utf-8'), ensure_ascii=False, encoding='utf-8',
                      indent=4, sort_keys=True)
    def readfile(self, filename):
        print("JsonFile: readfile: start: self.filename= ", self.filename)
        if filename != "":
            self.filename = filename
        #print("JsonFile: readfile: 1: self.filename= ", self.filename)
        if not os.path.exists(self.filename):
            print("JsonFile: readfile: no exists: self.filename= ", self.filename)
            return None
        data = None
        with open(self.filename, 'r') as f:
            filedata = f.read()
            filedata = filedata.decode(encoding='utf-8', errors='strict')
            try:
                data = json.loads(filedata, encoding='utf-8')
            except:
                try:
                    data = json.loads(filedata)
                except:
                    print("read_json: fail : filename= ", filename)
                else:
                    self.dict = data
                    print("JsonFile: readfile: ok")
            else:
                self.dict = data
                print("JsonFile: readfile: ok")
        return data
#noused
class PyLogin(object):
    def __init__(self):
        pass

    def log_frame(self):
        window = tk.Tk()
        window.title('欢迎进入会畅SVC调试系统')
        window.geometry('690x500')
        # 画布放置图片
        canvas = tk.Canvas(window, height=400, width=800)
        imagefile = tk.PhotoImage(file='hc.png')
        image = canvas.create_image(0, 0, anchor='nw', image=imagefile)
        canvas.pack(side='top')
        # `在这里插入代码片`
        # 标签 用户名密码
        tk.Label(window, text='用户名:').place(x=200, y=320)
        tk.Label(window, text='密码:').place(x=200, y=360)
        # 用户名输入框
        var_usr_name = tk.StringVar()
        entry_usr_name = tk.Entry(window, textvariable=var_usr_name)
        entry_usr_name.place(x=260, y=320)
        # 密码输入框
        var_usr_pwd = tk.StringVar()
        entry_usr_pwd = tk.Entry(window, textvariable=var_usr_pwd, show='*')
        entry_usr_pwd.place(x=260, y=360)
        # 登录 注册按钮
        bt_login = tk.Button(window, text='登录', command=self.usr_log_in)
        bt_login.place(x=260, y=400)
        bt_logup = tk.Button(window, text='注册', command=self.usr_sign_up)

        bt_logup.place(x=330, y=400)
        bt_logquit = tk.Button(window, text='退出', command=self.usr_sign_quit)
        bt_logquit.place(x=400, y=400)

        window.mainloop()

    def usr_log_in(self):
        # 输入框获取用户名密码
        usr_name = var_usr_name.get()
        usr_pwd = var_usr_pwd.get()
        # 从本地字典获取用户信息，如果没有则新建本地数据库
        try:
            with open('usr_info.pickle', 'rb') as usr_file:
                usrs_info = pickle.load(usr_file)
        except FileNotFoundError:
            with open('usr_info.pickle', 'wb') as usr_file:
                usrs_info = {'admin': 'admin'}
                pickle.dump(usrs_info, usr_file)
        # 判断用户名和密码是否匹配
        if usr_name in usrs_info:
            if usr_pwd == usrs_info[usr_name]:
                tk.messagebox.showinfo(title='welcome',
                                       message='欢迎您：' + usr_name)
                window.destroy()
                Image_sys()  # 打开主界面

            else:
                tk.messagebox.showerror(message='密码错误')
        # 用户名密码不能为空
        elif usr_name == '' or usr_pwd == '':
            tk.messagebox.showerror(message='用户名或密码为空')
        # 不在数据库中弹出是否注册的框
        else:
            is_signup = tk.messagebox.askyesno('欢迎', '您还没有注册，是否现在注册')
            if is_signup:
                usr_sign_up()  # 打开注册界面

    def usr_sign_up(self):
        # 确认注册时的相应函数
        def signtowcg():
            # 获取输入框内的内容
            nn = new_name.get()
            np = new_pwd.get()
            npf = new_pwd_confirm.get()

            # 本地加载已有用户信息,如果没有则已有用户信息为空
            try:
                with open('usr_info.pickle', 'rb') as usr_file:
                    exist_usr_info = pickle.load(usr_file)
            except FileNotFoundError:
                exist_usr_info = {}

                # 检查用户名存在、密码为空、密码前后不一致
            if nn in exist_usr_info:
                tk.messagebox.showerror('错误', '用户名已存在')
            elif np == '' or nn == '':
                tk.messagebox.showerror('错误', '用户名或密码为空')
            elif np != npf:
                tk.messagebox.showerror('错误', '密码前后不一致')
            # 注册信息没有问题则将用户名密码写入数据库
            else:
                exist_usr_info[nn] = np
                with open('usr_info.pickle', 'wb') as usr_file:
                    pickle.dump(exist_usr_info, usr_file)
                tk.messagebox.showinfo('欢迎', '注册成功')
                # 注册成功关闭注册框
                window_sign_up.destroy()

        # 新建注册界面

    def new_usr_sign_up(self):
        window_sign_up = tk.Toplevel(window)
        window_sign_up.geometry('350x200')
        window_sign_up.title('注册')
        # 用户名变量及标签、输入框
        new_name = tk.StringVar()
        tk.Label(window_sign_up, text='用户名：').place(x=10, y=10)
        tk.Entry(window_sign_up, textvariable=new_name).place(x=150, y=10)
        # 密码变量及标签、输入框
        new_pwd = tk.StringVar()
        tk.Label(window_sign_up, text='请输入密码：').place(x=10, y=50)
        tk.Entry(window_sign_up, textvariable=new_pwd, show='*').place(x=150, y=50)
        # 重复密码变量及标签、输入框
        new_pwd_confirm = tk.StringVar()
        tk.Label(window_sign_up, text='请再次输入密码：').place(x=10, y=90)
        tk.Entry(window_sign_up, textvariable=new_pwd_confirm, show='*').place(x=150, y=90)
        # 确认注册按钮及位置
        bt_confirm_sign_up = tk.Button(window_sign_up, text='确认注册',
                                       command=signtowcg)
        bt_confirm_sign_up.place(x=150, y=130)

    def usr_sign_quit(self):
        print("usr_sign_quit ")


class Pyui(object):
    def __init__(self):
        self.renderer = None
        self.window = None
        self.event = None
        self.load = loadlib.gload
        self.test = None


    def draw(self):
        x1 = ctypes.c_int(random.randrange(0, 600))
        y1 = ctypes.c_int(random.randrange(0, 500))
        x2 = ctypes.c_int(random.randrange(0, 600))
        y2 = ctypes.c_int(random.randrange(0, 500))
        r = ctypes.c_ubyte(random.randrange(0, 255))
        g = ctypes.c_ubyte(random.randrange(0, 255))
        b = ctypes.c_ubyte(random.randrange(0, 255))
        SDL_SetRenderDrawColor(self.renderer, r, g, b, ctypes.c_ubyte(255))
        SDL_RenderDrawLine(self.renderer, x1, y1, x2, y2)

    def sdl_update(self):
        SDL_RenderPresent(self.renderer);
        if SDL_PollEvent(ctypes.byref(self.event)) != 0:
            if self.event.type == SDL_QUIT:
                SDL_DestroyRenderer(self.renderer)
                SDL_DestroyWindow(self.window)
                SDL_Quit()

    # tkinter stuff #
    def FrameTest(self):
        root = tk.Tk()
        # 第2步，给窗口的可视化起名字
        root.title('HCSVC Demo')
        embed = tk.Frame(root, width=500, height=500)  # creates embed frame for pygame window
        embed.grid(columnspan=(600), rowspan=500)  # Adds grid
        embed.pack(side=LEFT)  # packs window to the left
        buttonwin = tk.Frame(root, width=75, height=500)
        buttonwin.pack(side=LEFT)
        button1 = Button(buttonwin, text='Draw', command=self.draw)
        button1.pack(side=LEFT)
        root.update()
        #################################
        winid = embed.winfo_id()
        print("FrameTest: winid=", winid)
        # SDL window stuff #
        SDL_Init(SDL_INIT_VIDEO)
        self.window = SDL_CreateWindowFrom(embed.winfo_id())

        self.renderer = SDL_CreateRenderer(self.window, -1, 0)
        SDL_SetRenderDrawColor(self.renderer, ctypes.c_ubyte(255), ctypes.c_ubyte(255),
                               ctypes.c_ubyte(255), ctypes.c_ubyte(255))
        SDL_RenderClear(self.renderer)
        self.event = SDL_Event()
        self.draw()

        while True:
            self.sdl_update()
            root.update()
            time.sleep(0.02)

    def draw2(self):
        print("draw2")
        self.test.sdl_push_event(3)

    def check_event(self):
        REFRESH_EVENT = (SDL_USEREVENT + 1)
        BREAK_EVENT = (SDL_USEREVENT + 2)
        if SDL_PollEvent(ctypes.byref(self.event)) != 0:
            if self.event.type == SDL_QUIT:
                print("check_event: quit")
            elif self.event.type == REFRESH_EVENT:
                print("check_event: refresh")
            elif self.event.type == SDL_WINDOWEVENT:
                print("check_event: window")
            elif self.event.type == SDL_KEYDOWN:
                print("check_event: keydown")
            elif self.event.type == BREAK_EVENT:
                print("check_event: break")

    def FrameCallSDL(self):
        root = tk.Tk()
        # 第2步，给窗口的可视化起名字
        root.title('HCSVC Demo')
        # 第3步，设定窗口的大小(长 * 宽)
        # root.geometry('500x300')  # 这里的乘是小x
        # 第4步，在图形界面上创建一个标签用以显示内容并放置
        # tk.Label(root, text='on the window', bg='red', font=('Arial', 16)).pack()  # 和前面部件分开创建和放置不同，其实可以创建和放置一步完成
        # 设置窗口是否可变长、宽，True：可变，False：不可变
        root.resizable(width=True, height=True)

        (self.width, self.height) = loadlib.WIDTH, loadlib.HEIGHT
        embed = tk.Frame(root, width=self.width, height=self.height)  # creates embed frame for pygame window
        embed.grid(columnspan=(self.width + 100), rowspan=self.height)  # Adds grid
        embed.pack(side=LEFT)  # packs window to the left
        buttonwin = tk.Frame(root, width=75, height=self.height)
        buttonwin.pack(side=LEFT)
        button1 = Button(buttonwin, text='Draw', command=self.draw2)
        button1.pack(side=LEFT)
        root.update()
        #################################
        # SDL window stuff #
        winid = embed.winfo_id()
        self.test = ReadFrame(winid)
        self.test.init()
        self.test.start()
        # self.event = SDL_Event()
        # self.test.sdl_show_run()
        # while True:
        #    self.check_event()
        #    root.update()
        #    time.sleep(0.02)
        root.mainloop()
        return


# refor to: https://docstore.mik.ua/orelly/perl3/tk/ch02_03.htm
# -anchor => 'n' | 'ne' | 'e' | 'se' | 's' | 'sw' | 'w' | 'nw' | 'center'
## Sets the position in the widget that will be placed at the specified coordinates. docstore.mik.ua/orelly/perl3/tk/ch02_03.htm

class ExternalSDL(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.lock = threading.Lock()
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
        self.pause()

        self.status = 0
        self.test_meeting = None
        self.winid = 0
    def get_status(self):
        ret = 0
        self.lock.acquire()
        ret = self.status
        self.lock.release()
        return ret
    def set_status(self, value):
        self.lock.acquire()
        self.status = value
        self.lock.release()
    def set_winid(self, win_hnd):
        if win_hnd > 0:
            self.winid = win_hnd
    def creat_meeting(self, win_hnd):
        if win_hnd > 0:
            self.winid = win_hnd
        if self.test_meeting == None:
            self.test_meeting = ReadFrame(self.winid)
            self.test_meeting.init()
            self.test_meeting.start()
    def close_meeting(self):
        if self.test_meeting != None:
            self.test_meeting.stop()
            #time.sleep(2)
            ##self.test_meeting.sdl_push_event(4)
            ##time.sleep(2)
            print("run start to delete self.test_meeting")
            ##self.test_meeting.stop()
            ##del self.test_meeting
            ##self.test_meeting = None
            self.set_status(0)
    def stop(self):
        self.__flag.set()  # 将线程从暂停状态恢复, 如何已经暂停的话
        self.__running.clear()  # 设置为Fals

    def pause(self):
        self.__flag.clear()  # 设置为False, 让线程阻塞

    def resume(self):
        self.__flag.set()  # 设置为True, 让线程停止阻塞

    def run(self):
        while self.__running.isSet():
            self.__flag.wait()  # 为True时立即返回, 为False时阻塞直到内部的标识位为True后返回
            self.lock.acquire()
            if self.status == 1:
                self.lock.release()
                self.creat_meeting(0)
                self.pause()
                self.lock.acquire()
            elif self.status == 2:
                self.lock.release()
                self.close_meeting()
                self.status = 0
                #self.pause()
                self.lock.acquire()
            self.lock.release()
        if self.get_status():
            self.close_meeting()
        self.stop()
        print("ExternalSDL: run over")

class ClientFrame(object):
    def __init__(self, parent):
        self.parent = parent
        self.test_meeting = None
        self.meeting_frame = None
        self.canvas = None

    def enter_meeting(self):
        self.exit_meeting()
        self.openFrame()
    def exit_meeting(self):
        if self.meeting_frame != None:
            self.meeting_frame.withdraw()
        if True:
            count = 0
            status = 0
            if self.test_meeting != None:
                status = self.test_meeting.get_status()
                if status != 2:
                    self.test_meeting.set_status(2)
                self.test_meeting.resume()
                self.test_meeting.stop()
                status = self.test_meeting.get_status()
            while status:
                status = self.test_meeting.get_status()
                time.sleep(0.1)
                count += 1
                if (count % 50) == 0:
                    print("exit_meeting: (count, status)=", (count, status))
            #self.test_meeting.resume()

            self.test_meeting = None
        print("ClientFrame: exit_meeting end")

        return
    def refrash_frame(self):
        if self.meeting_frame != None:
            num = 0
            # self.itext = self.canvas.create_text((self.width - 300), 0, text=str(num), font=("Comic Sans", 15),fill='yellow',anchor='nw')
            status = 1
            while status:
                #print("refrash_frame: enter")
                num += 1
                ###
                if True:
                    cvalue0 = "[欢迎进入会畅SVC调试系统]" + "\t" + "\t"
                    cvalue1 = "frmrate:" + "\t" + str(num) + "fps" + "|\t"
                    cvalue2 = "recv-brate:" + "\t" + str(num) + "kbps" + "|\t"
                    cvalue3 = "send-brate:" + "\t" + str(num) + "kbps" + "|\t"
                    cvalue4 = "lossrate:" + "\t" + str(num) + "%" + "|\t"
                    cvalue5 = "cpu:" + "\t" + str(num) + "%" + "|\t"
                    cvalue6 = "record-volum:" + "\t" + str(num) + "%" + "|\t"
                    cvalue7 = "play-volum:" + "\t" + str(num) + "%" + "|\t"
                    cvalue = cvalue0 + cvalue1 + cvalue2 + cvalue3 + cvalue4 + cvalue5 + cvalue6 + cvalue7
                    #print("refrash_frame: 0")
                    self.meeting_frame.title(cvalue)
                    #print("refrash_frame: 1")
                #print("refrash_frame: 2")
                # self.canvas.delete("all")
                self.image1 = tk.PhotoImage(file='icon/home.png').subsample(3, 4)  # zoom(2, 2)
                ##handler = lambda: self.onClosemeeting_frame(self.meeting_frame)
                self.btn_home = tk.Button(self.meeting_frame, text="home", image=self.image1,
                                          command=self.parent.show)  # , width=50, height=30)
                self.btn_home.place(x=0, y=0)

                ###
                #print("refrash_frame: start update")
                self.meeting_frame.update()
                #print("refrash_frame: start update ok")
                # print('num=%d' % num)
                # self.meeting_frame.after(500)
                # self.meeting_frame.after(4)
                time.sleep(0.1)
                if (num % 10) == 0:
                    print("openFrame: (self.status, num)= ", (status, num))
                if self.test_meeting != None:
                    status = self.test_meeting.get_status()
                else:
                    status = 0
            ###
            #子线程不能删除主线程创建的实例
            print("refrash_frame: meeting_frame.destroy")
            if self.canvas != None:
                self.canvas.delete("all")
                self.canvas.destroy()
                #del self.canvas
            if self.meeting_frame != None:
                self.meeting_frame.destroy()
                print("refrash_frame: meeting_frame.destroy: ok")
                #del self.meeting_frame
                self.meeting_frame = None
            print("refrash_frame over")
    def on_meeting_closing(self):
        print("meeting frame close")
        self.exit_meeting()
        #time.sleep(2)
        if self.meeting_frame != None:
            self.canvas.delete("all")
            self.canvas.destroy()
            #del self.canvas
            self.canvas = None
            self.meeting_frame.destroy()
            print("refrash_frame: meeting_frame.destroy: ok")
            #del self.meeting_frame
            self.meeting_frame = None
        else:
            self.parent.show()

    def openFrame(self):
        self.parent.hide()
        if True:
            #self.status = True
            self.meeting_frame = tk.Toplevel()

            self.meeting_frame.title("欢迎进入会畅SVC调试系统")
            str_resolution = str(self.parent.width) + "x" + str(self.parent.height)
            self.meeting_frame.geometry(str_resolution) #"1280x720"
            #self.meeting_frame.attributes("-toolwindow", 1)
            #self.meeting_frame.iconbitmap("My icon.ico")
            #self.meeting_frame.attributes("-alpha", 0.5)

            self.imagefile1 = tk.PhotoImage(file='icon/hc_2.png')#.zoom(2)
            self.canvas = tk.Canvas(self.meeting_frame, height=self.parent.height, width=self.parent.width)
            self.image_2 = self.canvas.create_image(0, 0, anchor='nw', image=self.imagefile1)
            self.canvas.pack(side='top')
            #label_img = tk.Label(self.meeting_frame, image=self.imagefile1)
            #label_img.pack()
            self.winid = self.canvas.winfo_id()

            self.image1 = tk.PhotoImage(file='icon/home.png').subsample(3, 4)  # zoom(2, 2)
            #handler = lambda: self.onClosemeeting_frame(self.meeting_frame)
            self.btn_home = tk.Button(self.meeting_frame, text="home", image=self.image1, command=self.parent.show)  # , width=50, height=30)
            self.btn_home.place(x=0, y=0)
            # btn_home.pack()
            #self.btn_home.attributes('-topmost', True)
            #self.meeting_frame.attributes('-topmost', True)

            #self.image0 = tk.PhotoImage(file='icon/video_start.png').subsample(3, 4)  # zoom(2, 2)
            #self.bt_video_start = tk.Button(self.meeting_frame, text='启动', image=self.image0, command=self.draw)  # , width=50, height=30)
            #self.bt_video_start.place(x=60, y=0)
            #self.bt_video_start.pack()

            self.meeting_frame.update()

            if self.test_meeting == None:
                self.test_meeting = ExternalSDL()
                self.test_meeting.start()
            self.test_meeting.set_winid(self.winid)
            self.test_meeting.set_status(1)
            self.test_meeting.resume()
            #time.sleep(20)
            #self.draw()
            #self.resume()
            # button2 = tk.Button(self.meeting_frame, text='bmp', bitmap = 'error')
            # button2.pack()
            self.meeting_frame.protocol("WM_DELETE_WINDOW", self.on_meeting_closing) #root.iconify #root.destroy #customized_function
            #self.meeting_frame.protocol("WM_DELETE_WINDOW", self.meeting_frame.iconify)
            self.refrash_frame()
            #self.meeting_frame.mainloop()

        print("openFrame: over")
class ConfigGeneralFrame(object):
    def __init__(self, parent, parentFrame, width0, height0, generalDict):
        self.parent = parent
        self.parentFrame = parentFrame
        self.dict = generalDict
        self.modeDict = {}
        self.mcuDict = {}
        self.suppramsDict = {}
        if generalDict != None:
            if generalDict.get("mode") != None:
                self.modeDict = generalDict.get("mode")
            if generalDict.get("mcu") != None:
                self.mcuDict = generalDict.get("mcu")
            if generalDict.get("supprams") != None:
                self.suppramsDict = generalDict.get("supprams")
        self.splitMode = 0
        (self.width, self.height) = (width0, height0)
        (orgx, orgy, width1, height1) = (0, 0, (width0 >> 1), ((height0 >> 1) - 16))
        self.splitFrame0 = ttk.LabelFrame(parentFrame, text='显示分屏', width=width1, height=height1)
        self.splitFrame0.place(x=orgx, y=orgy)
        (orgx, orgy) = (width1, 0)
        self.splitFrame1 = ttk.LabelFrame(parentFrame, text='通道关联', width=width1, height=height1)
        self.splitFrame1.place(x=orgx, y=orgy)
        (orgx, orgy) = (0, height1)
        self.mcuFrame = ttk.LabelFrame(parentFrame, text='合成', width=width1, height=height1)
        self.mcuFrame.place(x=orgx, y=orgy)
        (orgx, orgy) = (width1, height1)
        self.suparamsFrame = ttk.LabelFrame(parentFrame, text='高级调参', width=width1, height=height1)
        self.suparamsFrame.place(x=orgx, y=orgy)
    def CreateModeTab(self, id):
        ret = []
        factor = 4
        (w, h) = (self.width, self.height)
        w1 = self.width / factor
        h1 = self.height / factor
        if id == 0:
            ret.append((0, 0, w, h)) #(x, y, w, h)
        elif id == 1: #s
            ret.append((0, 0, w, (h1 << 1)))
            ret.append((0, (h1 << 1), w, (h1 << 1)))
        elif id == 2: #s
            ret.append((0, 0, (w1 << 1), h))
            ret.append(((w1 << 1), 0, (w1 << 1), h))
        elif id == 3:
            ret.append((0, 0, w, h))
            ret.append((0, 0, w1, h1))
        elif id == 4:
            ret.append((0, 0, w, h))
            ret.append(((w - w1), 0, w1, h1))
        elif id == 5:
            ret.append((0, 0, w, h))
            ret.append((0, (h - h1), w1, h1))
        elif id == 6:
            ret.append((0, 0, w, h))
            ret.append(((w - w1), (h - h1), w1, h1))
        elif id == 7: #s
            ret.append((w1, 0, (w - w1), h))
            ret.append((0, 0, w1, (h1 << 1)))
            ret.append((0, (h1 << 1), w1, (h1 << 1)))
        elif id == 8:
            ret.append((0, 0, (w1 << 1), (h1 << 1)))
            ret.append(((w1 << 1), 0, (w1 << 1), (h1 << 1)))
            ret.append((0, (h1 << 1), (w1 << 1), (h1 << 1)))
            ret.append(((w1 << 1), (h1 << 1), (w1 << 1), (h1 << 1)))
        elif id == 9: #s
            ret.append((w1, 0, (w - w1), h))
            ret.append((0, 0, w1, (h / 3)))
            ret.append((0, (h / 3), w1, (h / 3)))
            ret.append((0, (2 * h / 3), w1, (h / 3)))
        elif id == 10:
            ret.append((0, 0, w, h))
            ret.append((0, 0, w1, h1))
            ret.append((0, h1, w1, h1))
            ret.append((0, (2 * h1), w1, h1))
            ret.append((0, (3 * h1), w1, h1))
        elif id == 11:
            ret.append((0, 0, w, h))
            ret.append(((w - w1), 0, w1, h1))
            ret.append(((w - w1), h1, w1, h1))
            ret.append(((w - w1), (2 * h1), w1, h1))
            ret.append(((w - w1), (3 * h1), w1, h1))
        elif id == 12:
            ret.append((0, 0, w, h))
            ret.append((0, (h - h1), w1, h1))
            ret.append((w1, (h - h1), w1, h1))
            ret.append((2 * w1, (h - h1), w1, h1))
            ret.append((3 * w1, (h - h1), w1, h1))
        elif id == 13:
            ret.append((0, 0, w, h))
            ret.append((0, 0, w1, h1))
            ret.append((w1, 0, w1, h1))
            ret.append((2 * w1, 0, w1, h1))
            ret.append((3 * w1, 0, w1, h1))
        elif id == 14:
            ret.append((0, 0, w, h))
            ret.append((0, 0, w1, h1))
            ret.append((0, h1, w1, h1))
            ret.append((0, (2 * h1), w1, h1))
            ret.append((0, (h - h1), w1, h1))
            ret.append((w1, (h - h1), w1, h1))
            ret.append((2 * w1, (h - h1), w1, h1))
            ret.append((3 * w1, (h - h1), w1, h1))
        elif id == 15:
            ret.append((0, 0, w, h))
            ret.append(((w - w1), 0, w1, h1))
            ret.append(((w - w1), h1, w1, h1))
            ret.append(((w - w1), (2 * h1), w1, h1))
            ret.append((0, (h - h1), w1, h1))
            ret.append((w1, (h - h1), w1, h1))
            ret.append((2 * w1, (h - h1), w1, h1))
            ret.append((3 * w1, (h - h1), w1, h1))
        elif id == 16:
            ret.append((0, 0, w, h))
            ret.append((0, 0, w1, h1))
            ret.append((w1, 0, w1, h1))
            ret.append((2 * w1, 0, w1, h1))
            ret.append((3 * w1, 0, w1, h1))
            ret.append((0, h1, w1, h1))
            ret.append((0, (2 * h1), w1, h1))
            ret.append((0, (3 * h1), w1, h1))
        elif id == 17:
            ret.append((0, 0, w, h))
            ret.append((0, 0, w1, h1))
            ret.append((w1, 0, w1, h1))
            ret.append((2 * w1, 0, w1, h1))
            ret.append((3 * w1, 0, w1, h1))
            ret.append(((w - w1), h1, w1, h1))
            ret.append(((w - w1), (2 * h1), w1, h1))
            ret.append(((w - w1), (3 * h1), w1, h1))
        elif id == 18:
            ret.append((        0,        0, w / 3, h / 3))
            ret.append((    w / 3,        0, w / 3, h / 3))
            ret.append((2 * w / 3,        0, w / 3, h / 3))
            ret.append((        0,    h / 3, w / 3, h / 3))
            ret.append((    w / 3,    h / 3, w / 3, h / 3))
            ret.append((2 * w / 3,    h / 3, w / 3, h / 3))
            ret.append((        0,2 * h / 3, w / 3, h / 3))
            ret.append((    w / 3,2 * h / 3, w / 3, h / 3))
            ret.append((2 * w / 3,2 * h / 3, w / 3, h / 3))
        elif id == 19:
            ret.append((        0, 0, w / 4, h / 4))
            ret.append((    w / 4, 0, w / 4, h / 4))
            ret.append((2 * w / 4, 0, w / 4, h / 4))
            ret.append((3 * w / 4, 0, w / 4, h / 4))
            ret.append((        0, h / 4, w / 4, h / 4))
            ret.append((    w / 4, h / 4, w / 4, h / 4))
            ret.append((2 * w / 4, h / 4, w / 4, h / 4))
            ret.append((3 * w / 4, h / 4, w / 4, h / 4))
            ret.append((        0, 2 * h / 4, w / 4, h / 4))
            ret.append((    w / 4, 2 * h / 4, w / 4, h / 4))
            ret.append((2 * w / 4, 2 * h / 4, w / 4, h / 4))
            ret.append((3 * w / 4, 2 * h / 4, w / 4, h / 4))
            ret.append((        0, 3 * h / 4, w / 4, h / 4))
            ret.append((    w / 4, 3 * h / 4, w / 4, h / 4))
            ret.append((2 * w / 4, 3 * h / 4, w / 4, h / 4))
            ret.append((3 * w / 4, 3 * h / 4, w / 4, h / 4))
        return ret

    def CreateSplitFrame(self):
        modeDict = self.modeDict
        self.modePosList = []
        self.modeNum = 20
        n = 5
        (width0, height0) = (self.width, self.height - 32)
        w = (width0 >> 1) - 16
        h = (height0 >> 1) - 16
        stepx = w / n
        stepy = h / n #(n + 1)
        offset = 8
        w2 = stepx - (offset << 1)
        h2 = stepy - (offset << 1)
        #w3 = 16
        #h3 = 3#h2 >> 3
        self.imglist = []
        self.splitModeVar = tk.IntVar()

        idValue = 0
        if modeDict != None:
            modeId = modeDict.get("modeId")
            if modeId != None:
                idValue = modeId
        #self.splitimage = tk.PhotoImage(file='icon/stock_table_split.png').zoom(6,3)
        self.splitSubFrameList = None
        def modePage(v):
            print("modePage: v= ", v)
            if self.splitSubFrameList != None:
                for splitSubFrame in self.splitSubFrameList:
                    splitSubFrame.destroy()
            self.splitMode = v
            self.splitRects = self.CreateModeTab(v - 1)
            posList = []
            print("modePage: self.splitRects= ", self.splitRects)
            self.chanBntList = []  # [k]
            #self.chanBntVarList = [len(self.splitRects)]  # [k]
            #self.splitSubFrameList = []
            self.chanBntVarList = [0 for i in range(len(self.splitRects))]
            self.splitSubFrameList = [None for i in range(len(self.splitRects))]
            self.chanIdList = [0 for i in range(len(self.splitRects))]
            self.chanIdVarList = [0 for i in range(len(self.splitRects))] #tk.IntVar()

            def chanPage(v2):
                print("chanPage: (v, v2)= ", (v, v2))
                #self.chainId = v2
                #self.chanIdList[id] = v2
                #self.modeDict["modePos"][id]["devideId"] = v2
                i = v2 - 1
                splitSubFrame = self.splitSubFrameList[i]
                wx = splitSubFrame.winfo_x()
                wy = splitSubFrame.winfo_y()
                ww = splitSubFrame.winfo_width()
                wh = splitSubFrame.winfo_height()
                winfo0 = self.parent.config_frame.winfo_geometry()
                winfo1 = self.parentFrame.winfo_geometry()
                winfo2 = self.splitFrame1.winfo_geometry()
                winfo3 = splitSubFrame.winfo_geometry()
                print("chanPage: winfo0", winfo0)
                print("chanPage: winfo1", winfo1)
                print("chanPage: winfo2", winfo2)
                print("chanPage: winfo3", winfo3)
                wx = self.parent.config_frame.winfo_x()
                wy = self.parent.config_frame.winfo_y()
                wx += self.parentFrame.winfo_x()
                wy += self.parentFrame.winfo_y()
                wx += self.splitFrame1.winfo_x()
                wy += self.splitFrame1.winfo_y()
                wx += splitSubFrame.winfo_x()
                wy += splitSubFrame.winfo_y()

                print("chanPage: (wx, wy)", (wx, wy))
                print("chanPage: (ww, wh)", (ww, wh))
                popFrame = tk.Toplevel()
                #popFrame = tk.Frame(splitSubFrame, width=640, height=480)
                popFrame.title("设备及通道选择")
                (popwidth, popheight) = (320, 240)
                str_resolution = str(popwidth) + "x" + str(popheight) + "+" + str(wx) + "+" + str(wy)
                popFrame.geometry(str_resolution)
                popFrame.resizable(width=False, height=False)  # 禁止改变窗口大小
                #popFrame.pack()
                #popFrame.wm_attributes('-topmost', 1)
                #popFrame.place(x=wx, y=wy)
                popFrame.update()
                #popFrame.deiconify()

                (orgx, orgy, fsize, lsize, step, combSize, btnWidth) = (0, 0, 8, 8, 32, 3, 10)
                if i == 0:
                    (orgx, orgy) = (100, 100)
                deviceId = None
                value = 0
                #if suppramsDict != None:
                #    spatiallayer = suppramsDict.get("spatiallayer")
                #    if spatiallayer != None:
                #        value = spatiallayer.get("value")
                #        name = spatiallayer.get("name")
                ttk.Label(popFrame, text="设备", width=lsize).place(x=orgx, y=orgy)
                self.deviceIdVar = tk.StringVar()
                self.deviceIdChosen = ttk.Combobox(popFrame, width=combSize,
                                                       textvariable=self.deviceIdVar,
                                                       state='readonly')
                self.deviceIdChosen['values'] = (0, 1, 2, 3)  # 设置下拉列表的值
                self.deviceIdChosen.current(value)  # 设置下拉列表默认显示的值，0为 numberChosen['values'] 的下标值
                self.deviceIdChosen.place(x=(orgx + 40), y=orgy)
                orgy += step

                chanId = None
                value = 0
                # if suppramsDict != None:
                #    spatiallayer = suppramsDict.get("spatiallayer")
                #    if spatiallayer != None:
                #        value = spatiallayer.get("value")
                #        name = spatiallayer.get("name")
                ttk.Label(popFrame, text="通道", width=lsize).place(x=orgx, y=orgy)
                self.chanIdVar = tk.StringVar()
                self.chanIdChosen = ttk.Combobox(popFrame, width=combSize,
                                                   textvariable=self.chanIdVar,
                                                   state='readonly')
                self.chanIdChosen['values'] = (1, 2, 3, 4)  # 设置下拉列表的值
                self.chanIdChosen.current(value)  # 设置下拉列表默认显示的值，0为 numberChosen['values'] 的下标值
                self.chanIdChosen.place(x=(orgx + 40), y=orgy)

            self.chanBntVar = tk.IntVar()
            #for rect in self.splitRects:
            for i in range(len(self.splitRects)):
                self.chanIdVarList[i] = tk.IntVar()
                self.chanIdVarList[i].set(i)
                print("modePage: self.chanIdVarList[i].get()= ", self.chanIdVarList[i].get())
                rect = self.splitRects[i]
                captureId = 0 if i == 0 else -1
                posList.append({"chanId":i, "devideId": captureId, "pos":rect})
                #x0 = rect[0] / 2
                x0 = int((rect[0]) / 2.2)
                y0 = int((rect[1]) / 2.4)
                #dw = rect[2] / 2
                #dh = (rect[3] - 32) / 2
                dw = int((rect[2]) / 2.2)
                dh = int((rect[3]) / 2.4)
                #if y0 > 0:
                #    y0 -= 32
                #x0 = rect[0] / 2
                #y0 = (rect[1]) / 2
                #dw = rect[2] / 2
                #dh = (rect[3]) / 2
                #dw -= 32
                #dh -=
                #def SetChan(v):
                #    print("SetChan: v=", v)
                #splitSubFrame = ttk.LabelFrame(self.splitFrame1, text=str(i), width=dw, height=dh)
                #splitSubFrame = tk.Button(self.splitFrame1, text=str(i), command=lambda: SetChan(i), width=dw/10,
                #                          height=dh/30)

                splitSubFrame = tk.Radiobutton(self.splitFrame1, text=str(i), value=(i + 1), variable=self.chanBntVar,
                               command=lambda: chanPage(self.chanBntVar.get()), indicatoron=0,width=dw/10,height=dh/30)
                splitSubFrame.place(x=x0, y=y0)

                k = len(self.splitRects)
                n2 = 4
                k = n2 * n2
                stepx2 = dw / n2
                stepy2 = dh / (n2 + 2)
                offset2 = 2#8
                w3 = stepx2 - (offset2 << 1)
                h3 = stepy2 - (offset2 << 1)
                #chanBntVar = tk.IntVar()
                #self.chanBntVarList[i] = chanBntVar #tk.IntVar()
                ###chan select
                ###device select
                self.splitSubFrameList[i] = splitSubFrame
                if False:
                    for j in range(n2):
                        for l in range(n2):
                            id = j * n2 + l
                            orgx = l * stepx2 + offset2
                            orgy = j * stepy2 + offset2
                            chanBnt = tk.Radiobutton(splitSubFrame, text=str(id), value=(id + 1), variable=chanBntVar,
                                                     command=lambda: chanPage(chanBntVar.get(), i), indicatoron=0,
                                                     width=w3,
                                                     height=h3)
                            chanBnt.place(x=orgx, y=orgy)
                    #self.chanBntVarList.append(chanBntVar)
                    #self.chanBntList.append(chanBnt)
                    #self.splitSubFrameList.append(splitSubFrame)
                    self.splitSubFrameList[i] = splitSubFrame
            self.splitModeVar.set(v)
            self.modeDict.update({"modeId":v, "modePos":posList})
            return
        disableList = [1, 2, 7, 9]
        for i in range(n):
            for j in range(n):
                orgx = j * stepx + offset
                orgy = i * stepy + offset
                id = i * n + j
                if id < self.modeNum:
                    name = str(id)
                    imgname = 'mm' + str(id)
                    imgpath = 'icon/' + imgname + '.png'
                    splitimage = tk.PhotoImage(file=imgpath)
                    self.imglist.append(splitimage)
                    #self.subFrame = ttk.LabelFrame(self.splitFrame0, text=name, width=w2, height=h2)
                    #self.subFrame.place(x=orgx, y=orgy)
                    splitBtn = tk.Radiobutton(self.splitFrame0, text=name, value=(id + 1), variable=self.splitModeVar, image=splitimage,
                                              command=lambda: modePage(self.splitModeVar.get()), indicatoron=0, width=w2, height=h2)
                    splitBtn.place(x=orgx, y=orgy)
                    #self.subFrame = ttk.LabelFrame(splitBtn, text='', width=w2, height=h2)
                    #self.subFrame.place(x=0, y=0)
                    if id in disableList:
                        splitBtn.configure(state='disabled')
        self.splitModeVar.set(idValue)
        return modeDict
    def CreateSplitChanFrame(self):
        pass
    def CreateMcuFrame(self):
        mcuDict = self.mcuDict
        return mcuDict
    def CreateSuppramsFrame(self):
        suppramsDict = self.suppramsDict
        w = (self.width >> 1) - 16
        h = (self.height >> 1) - 16
        w2 = w >> 1
        h2 = (h >> 1)
        (x0, y0, dw, dh) = (0, 0, w2, h2)
        (orgx, orgy, fsize, lsize, step, combSize, btnWidth) = (0, 0, 8, 11, 32, 4, 10)
        otherSubFrame = ttk.LabelFrame(self.suparamsFrame, text="", width=w, height=(h - (h / 2)))
        otherSubFrame.place(x=x0, y=y0)

        ttk.Label(otherSubFrame, text="服务器地址", width=lsize).place(x=orgx, y=orgy)
        #
        serverAddVar = "47.92.7.66:10088"
        self.suppramsDict = suppramsDict
        if suppramsDict != None:
            serverAddr = suppramsDict.get("serverAddr")
            if serverAddr != None:
                serverAddVar = serverAddr

        self.var_sever_add = tk.StringVar()
        entry_sever_add = tk.Entry(otherSubFrame, textvariable=self.var_sever_add)
        entry_sever_add.place(x=(orgx + 100), y=orgy)
        self.var_sever_add.set(serverAddVar)

        orgx1 = dw
        self.compatible = tk.IntVar()
        compatibleValue = 1
        if suppramsDict != None:
            compatible = suppramsDict.get("compatible")
            if compatible != None:
                compatibleValue = compatible

        self.selectCompatible = tk.Checkbutton(otherSubFrame, text='兼容模式', variable=self.compatible, onvalue=1,
                                               offvalue=0)  # 传值原理类似于radiobutton部件
        self.selectCompatible.place(x=orgx1, y=orgy)
        self.compatible.set(compatibleValue)
        # offsety += step

        orgx1 += 100
        self.osd = tk.IntVar()
        osdValue = 1
        if suppramsDict != None:
            osd = suppramsDict.get("osd")
            if osd != None:
                osdValue = osd

        self.selectOSD = tk.Checkbutton(otherSubFrame, text='内置OSD', variable=self.osd, onvalue=1,
                                        offvalue=0)  # 传值原理类似于radiobutton部件
        self.selectOSD.place(x=orgx1, y=orgy)
        self.osd.set(osdValue)
        orgy += step
        #
        orgx2 = fsize * lsize
        spatiallayer = None
        value = 0
        if suppramsDict != None:
            spatiallayer = suppramsDict.get("spatiallayer")
            if spatiallayer != None:
                value = spatiallayer.get("value")
                name = spatiallayer.get("name")
        ttk.Label(otherSubFrame, text="空域层", width=lsize).place(x=orgx, y=orgy)
        self.spatialLayerVar = tk.StringVar()
        self.spatialLayerChosen = ttk.Combobox(otherSubFrame, width=combSize,
                                                  textvariable=self.spatialLayerVar,
                                                  state='readonly')
        self.spatialLayerChosen['values'] = (1, 2, 3, 4, 5)  # 设置下拉列表的值
        self.spatialLayerChosen.current(value)  # 设置下拉列表默认显示的值，0为 numberChosen['values'] 的下标值
        self.spatialLayerChosen.place(x=orgx2, y=orgy)
        #orgy += step

        orgx1 = fsize * lsize * 2
        orgx2 += fsize * lsize * 2
        temporallayer = None
        value = 1
        if suppramsDict != None:
            temporallayer = suppramsDict.get("temporallayer")
            if temporallayer != None:
                value = temporallayer.get("value")
                name = temporallayer.get("name")
        ttk.Label(otherSubFrame, text="时域层", width=lsize).place(x=orgx1, y=orgy)
        self.temporalLayerVar = tk.StringVar()
        self.temporalLayerChosen = ttk.Combobox(otherSubFrame, width=combSize,
                                               textvariable=self.temporalLayerVar,
                                               state='readonly')
        self.temporalLayerChosen['values'] = (1, 2, 4, 8, 16)  # 设置下拉列表的值
        value0 = self.temporalLayerChosen.current(value)  # 设置下拉列表默认显示的值，0为 numberChosen['values'] 的下标值
        print("value0=", value0)
        self.temporalLayerChosen.place(x=orgx2, y=orgy)
        #orgy += step

        orgx1 = dw + 20
        values = [0, 1, 2]
        if suppramsDict != None:
            uploadlayers = suppramsDict.get("uploadlayers")
            if uploadlayers != None:
                values = uploadlayers
        ttk.Label(otherSubFrame, text="上传层", width=lsize).place(x=orgx1, y=orgy)
        self.scrolly = Scrollbar(otherSubFrame)
        # scrolly.pack(side=RIGHT, fill=Y)
        self.scrolly.place(x=(orgx1 + 70), y=orgy)
        self.uplayers = tk.Listbox(otherSubFrame, selectmode=tk.MULTIPLE, width=2, height=2, yscrollcommand=self.scrolly.set)
        self.uplayers.place(x=(orgx1 + 50), y=orgy)
        for item in [1, 2, 3, 4, 5]:
            self.uplayers.insert(tk.END, item)
        # lb.selection_set(0, 2) #0~2
        for thisvalue in values:
            self.uplayers.selection_set(thisvalue)
        # lb.selection_clear(0, 3)
        print("has select:", self.uplayers.curselection())
        print(self.uplayers.selection_includes(4))
        self.scrolly.config(command=self.uplayers.yview)

        def saveConfigPage():
            # self.parent.save_config()
            self.SaveConfig()

        self.saveConfigVar = tk.IntVar()
        saveConfigBtn = tk.Radiobutton(otherSubFrame, text='参数保存', value=1, variable=self.saveConfigVar,
                                       # image=image4,
                                       command=saveConfigPage, indicatoron=0, width=btnWidth)
        self.saveConfigVar.set(0)
        saveConfigBtn.place(x=(orgx1 + 100), y=orgy)

        orgy += step

        #(orgx, orgy, lsize, step) = (w2, 0, 11, 32)
        netSubFrame = ttk.LabelFrame(self.suparamsFrame, text="网络适配调试", width=w, height=(dh + 16)) #((2 * h / 3) - 16))
        netSubFrame.place(x=x0, y=(h - (2 * h / 3)))
        (orgx, orgy, lsize, step) = (0, 0, 11, 32)
        ttk.Label(netSubFrame, text="网络丢包", width=lsize).place(x=orgx, y=orgy)
        netLossVar = "sudo tc qdisc add dev eno1 root netem loss 5%"
        self.var_net_loss = tk.StringVar()
        entry_net_loss = tk.Entry(netSubFrame, textvariable=self.var_net_loss, width=50)
        entry_net_loss.place(x=(orgx + 100), y=orgy)
        self.var_net_loss.set(netLossVar)

        def NetTest(v):
            self.saveConfigVar.set(0)
            thisvalue = v #self.netloss.get()
            print("NetLossTest: thisvalue= ", thisvalue)
            if v in [5]:
                self.netloss.set(0)
                self.netdelay.set(0)
                self.netreorder.set(0)
                self.netcorrupt.set(0)
            elif v in [1, 2, 3, 4]:
                self.netreset.set(0)
        self.netloss = tk.IntVar()
        netlossValue = 0
        self.selectNetLoss = tk.Checkbutton(netSubFrame, text='', variable=self.netloss, onvalue=1,
                                               offvalue=0, command=lambda:NetTest(self.netloss.get()))  # 传值原理类似于radiobutton部件
        self.selectNetLoss.place(x=(w - 50), y=orgy)
        self.netloss.set(netlossValue)

        orgy += step
        ttk.Label(netSubFrame, text="网络延迟", width=lsize).place(x=orgx, y=orgy)
        netDelayVar = "sudo tc qdisc add dev eth0 root netem delay 300ms 100ms 30%"
        self.var_net_delay = tk.StringVar()
        entry_net_delay = tk.Entry(netSubFrame, textvariable=self.var_net_delay, width=50)
        entry_net_delay.place(x=(orgx + 100), y=orgy)
        self.var_net_delay.set(netDelayVar)

        self.netdelay = tk.IntVar()
        netdelayValue = 0
        self.selectNetDelay = tk.Checkbutton(netSubFrame, text='', variable=self.netdelay, onvalue=2,
                                               offvalue=0,
                                               command=lambda: NetTest(self.netdelay.get()))  # 传值原理类似于radiobutton部件
        self.selectNetDelay.place(x=(w - 50), y=orgy)
        self.netdelay.set(netdelayValue)
        orgy += step
        ttk.Label(netSubFrame, text="包乱序", width=lsize).place(x=orgx, y=orgy)
        netReorderVar = "sudo tc qdisc change dev eno1 root netem delay 10ms reorder 25% 50%"
        self.var_net_reorder = tk.StringVar()
        entry_net_reorder = tk.Entry(netSubFrame, textvariable=self.var_net_reorder, width=50)
        entry_net_reorder.place(x=(orgx + 100), y=orgy)
        self.var_net_reorder.set(netReorderVar)
        self.netreorder = tk.IntVar()
        netreorderValue = 0
        self.selectNetReorder = tk.Checkbutton(netSubFrame, text='', variable=self.netreorder, onvalue=3,
                                               offvalue=0,
                                               command=lambda: NetTest(self.netreorder.get()))  # 传值原理类似于radiobutton部件
        self.selectNetReorder.place(x=(w - 50), y=orgy)
        self.netreorder.set(netreorderValue)
        orgy += step
        ttk.Label(netSubFrame, text="包损坏", width=lsize).place(x=orgx, y=orgy)
        netCorruptVar = "sudo tc qdisc add dev eno1 root netem corrupt 0.2%"
        self.var_net_corrupt = tk.StringVar()
        entry_net_corrupt = tk.Entry(netSubFrame, textvariable=self.var_net_corrupt, width=50)
        entry_net_corrupt.place(x=(orgx + 100), y=orgy)
        self.var_net_corrupt.set(netCorruptVar)
        self.netcorrupt = tk.IntVar()
        netcorruptValue = 0
        self.selectNetCorrupt = tk.Checkbutton(netSubFrame, text='', variable=self.netcorrupt, onvalue=4,
                                               offvalue=0,
                                               command=lambda: NetTest(self.netcorrupt.get()))  # 传值原理类似于radiobutton部件
        self.selectNetCorrupt.place(x=(w - 50), y=orgy)
        self.netcorrupt.set(netcorruptValue)
        orgy += step

        ttk.Label(netSubFrame, text="复位", width=lsize).place(x=orgx, y=orgy)
        netResetVar = "sudo tc qdisc del dev eno1 root"
        self.var_net_reset = tk.StringVar()
        entry_net_reset = tk.Entry(netSubFrame, textvariable=self.var_net_reset, width=50)
        entry_net_reset.place(x=(orgx + 100), y=orgy)
        self.var_net_reset.set(netResetVar)
        self.netreset = tk.IntVar()
        netresetValue = 0
        self.selectNetReset = tk.Checkbutton(netSubFrame, text='', variable=self.netreset, onvalue=5,
                                               offvalue=0,
                                               command=lambda: NetTest(self.netreset.get()))  # 传值原理类似于radiobutton部件
        self.selectNetReset.place(x=(w - 50), y=orgy)
        self.netreset.set(netresetValue)
        orgy += step
        return suppramsDict
    def SaveConfig(self):
        thisvalue = self.saveConfigVar.get()
        print("CreateSuppramsFrame: saveConfigPage: thisvalue= ", thisvalue)
        self.suppramsDict.update({"serverAddr": self.var_sever_add.get()})
        self.suppramsDict.update({"compatible": self.compatible.get()})
        self.suppramsDict.update({"osd": self.osd.get()})
        name = self.spatialLayerChosen.get()
        value = self.spatialLayerChosen.current()
        self.suppramsDict.update({"spatiallayer": {"name": name, "value": value}})
        name = self.temporalLayerChosen.get()
        value = self.temporalLayerChosen.current()
        print("value=", value)
        self.suppramsDict.update({"temporallayer": {"name": name, "value": value}})
        print("has select:", self.uplayers.curselection())
        self.suppramsDict.update({"uploadlayers": self.uplayers.curselection()})
        # self.suppramsDict.update({"uploadlayers": (0,1,2)})
        if self.parent.json.dict.get("general") == None:
            self.parent.json.dict.update({"general": {}})
        self.parent.json.dict["general"].update({"supprams": self.suppramsDict})
        if self.mcuDict != None:
            self.parent.json.dict["general"].update({"mcu": self.mcuDict})
        if self.modeDict != None:
            self.parent.json.dict["general"].update({"mode": self.modeDict})
        self.parent.json.writefile(None, "")
class ConfigControlFrame(object):
    def __init__(self, parent, parentFrame):
        self.parent = parent
        self.parentFrame = parentFrame
        self.audioFrame = None
        self.videoFarame = None
        self.width = 280
    def CreateControlFrame(self, offset):
        # Create a control container to hold labels
        contrFrame = ttk.LabelFrame(self.parentFrame, text='控制面板', width=160, height=720)
        contrFrame.place(x=offset, y=0)
        ###
        def addDevice():
            self.deviceNum += 1
            print("addDevice: self.deviceNum= ", self.deviceNum)

        (orgx, offsety, step, btnWidth) = (8, 8, 40, 10)
        image4 = tk.PhotoImage(file='icon/config.png').subsample(3, 4)
        self.addDeviceVar = tk.IntVar()
        addDeviceBtn = tk.Radiobutton(contrFrame, text='增加通道', value=1, variable=self.addDeviceVar,  # image=image4,
                                    command=addDevice, indicatoron=0, width=btnWidth)
        self.addDeviceVar.set(0)
        addDeviceBtn.place(x=orgx, y=offsety)
        offsety += step

        def frontPage():
            print("fronPage: self.deviceNum= ", self.deviceNum)

        self.frontVar = tk.IntVar()
        frontBtn = tk.Radiobutton(contrFrame, text='前一页', value=1, variable=self.frontVar,  # image=image4,
                                  command=frontPage, indicatoron=0, width=btnWidth)
        self.frontVar.set(0)
        frontBtn.place(x=orgx, y=offsety)
        offsety += step

        def backPage():
            print("backPage: self.deviceNum= ", self.deviceNum)

        self.backVar = tk.IntVar()
        backBtn = tk.Radiobutton(contrFrame, text='后一页', value=1, variable=self.backVar,  # image=image4,
                                 command=backPage, indicatoron=0, width=btnWidth)
        self.backVar.set(0)
        backBtn.place(x=orgx, y=offsety)
        offsety += step

        def mcuViewPage():
            print("mcuViewPage: self.deviceNum= ", self.deviceNum)

        self.mcuViewVar = tk.IntVar()
        mcuViewBtn = tk.Radiobutton(contrFrame, text='合成预览', value=1, variable=self.mcuViewVar,  # image=image4,
                                    command=mcuViewPage, indicatoron=0, width=btnWidth)
        self.mcuViewVar.set(0)
        mcuViewBtn.place(x=orgx, y=offsety)
        offsety += step

        def saveConfigPage():
            print("ConfigControlFrame: saveConfigPage: ")
            self.parent.save_config()

        self.saveConfigVar = tk.IntVar()
        saveConfigBtn = tk.Radiobutton(contrFrame, text='参数保存', value=1, variable=self.saveConfigVar,  # image=image4,
                                    command=saveConfigPage, indicatoron=0, width=btnWidth)
        self.saveConfigVar.set(0)
        saveConfigBtn.place(x=orgx, y=offsety)
        offsety += step

        orgx = 0
        # Create a control container to hold labels
        mcuFrame = ttk.LabelFrame(contrFrame, text='编码合成分屏', width=(160 - (orgx << 1)), height=(720 - offsety))
        mcuFrame.place(x=orgx, y=offsety)

        (orgx, offsety, step, btnWidth) = (8, 8, 40, 10)

        def splitPage(v):
            print("splitPage: v= ", v)

        splitNum = 8
        # self.splitVarList = []
        self.splitVar = tk.IntVar()
        for splitId in range(splitNum):
            splitName = '分屏' + str(splitId)
            splitBtn = tk.Radiobutton(mcuFrame, text=splitName, value=(splitId + 1), variable=self.splitVar,
                                      # image=image4,
                                      command=lambda: splitPage(self.splitVar.get()), indicatoron=0, width=btnWidth)
            # self.splitVar.set(splitId)
            # print("self.splitVar.get()=", self.splitVar.get())
            splitBtn.place(x=orgx, y=offsety)
            offsety += step
            # self.splitVarList.append(splitVar)
class ConfigDeviceFrame(object):
    def __init__(self, parent, parentFrame, deviceId, deviceDict):
        self.parent = parent
        self.parentFrame = parentFrame
        self.deviceId = deviceId
        self.audioFrame = None
        self.videoFarame = None
        self.width = 280
        self.dict = deviceDict
    def SaveConfig(self):
        audios = None
        videos = None
        if self.dict != None:
            audios = self.dict.get("audios")
            videos = self.dict.get("videos")
        else:
            audios = {}
            videos = {}
            self.dict.update({"audios": audios})
            self.dict.update({"videos": videos})

        value = self.audioDeviceChosen.current()
        name = self.audioDeviceChosen.get()
        #print("SaveConfig: value=", value)
        audios.update({"recorder":{"value": value, "name": name}})

        value = self.audioCodecChosen.current()
        name = self.audioCodecChosen.get()
        audios.update({"codec": {"value": value, "name": name}})

        value = self.audioSampleRateChosen.current()
        name = self.audioSampleRateChosen.get()
        audios.update({"samplerate": {"value": value, "name": name}})

        value = self.audioChannelsChosen.current()
        name = self.audioChannelsChosen.get()
        audios.update({"channels": {"value": value, "name": name}})

        value = self.audioFmtChosen.current()
        name = self.audioFmtChosen.get()
        audios.update({"audiofmt": {"value": value, "name": name}})

        value = self.audioFrameSizeChosen.current()
        name = self.audioFrameSizeChosen.get()
        audios.update({"framesize": {"value": value, "name": name}})

        value = self.denoise.get()
        audios.update({"denoise": value})
        value = self.agc.get()
        audios.update({"agc": value})
        value = self.aec.get()
        audios.update({"aec": value})
        value = self.detect.get()
        audios.update({"detect": value})
        value = self.vad.get()
        audios.update({"vad": value})

        ###
        value = self.videoDeviceChosen.current()
        name = self.videoDeviceChosen.get()
        videos.update({"capture": {"value": value, "name": name}})

        value = self.videoCodecChosen.current()
        name = self.videoCodecChosen.get()
        videos.update({"codec": {"value": value, "name": name}})

        value = self.videoResolutionChosen.current()
        name = self.videoResolutionChosen.get()
        videos.update({"resolution": {"value": value, "name": name}})

        value = self.videoFrameRateChosen.current()
        name = self.videoFrameRateChosen.get()
        videos.update({"framerate": {"value": value, "name": name}})

        value = self.videoBitRateChosen.current()
        name = self.videoBitRateChosen.get()
        videos.update({"bitrate": {"value": value, "name": name}})

        value = self.vdenoise.get()
        videos.update({"denoise": value})
        value = self.vFEC.get()
        videos.update({"fec": value})

        self.dict.update({"audios": audios})
        self.dict.update({"videos": videos})
        return self.dict

    def CreateDevice(self):
        ret = {}
        audios = None
        videos = None
        if self.dict != None:
            ret = self.dict
            audios = self.dict.get("audios")
            videos = self.dict.get("videos")
        else:
            audios = {}
            videos = {}
            ret.update({"audios": audios})
            ret.update({"videos": videos})
        # ttk.Label(frame3, text="Chooes a number").grid(column=1, row=0)  # 添加一个标签，并将其列设置为1，行设置为0
        # ttk.Label(frame3, text="Enter a name:").grid(column=0, row=0)  # 设置其在界面中出现的位置  column代表列   row 代表行

        # ft0 = tkFont.Font(family='Fixdsys', size=10, weight=tkFont.BOLD)
        # ft1 = tkFont.Font(size=20, slant=tkFont.ITALIC)
        # ft2 = tkFont.Font(size=30, weight=tkFont.BOLD, underline=1, overstrike=1)
        (orgx, orgy, offsetx, offsety, step) = (8, 8, 8, 8, 32)
        (fsize0, fsize, lsize, combSize) = (10, 8, 11, 14)
        ttk.Style().configure(".", font=("仿宋", fsize0))
        Font1 = ("Arial", fsize)
        # Create an audio container to hold labels
        frame30 = ttk.LabelFrame(self.parentFrame, text='音频设置', width=self.width, height=400)
        self.audioFrame = frame30
        frame30.place(x=0, y=0)
        # frame30 = frame3
        # buttons_frame.grid(column=1, row=6)

        # Place labels into the container element
        # ttk.Label(buttons_frame, text='Label1').grid(column=0, row=0, sticky=tk.W)
        # ttk.Label(buttons_frame, text='Lable2').grid(column=0, row=1, sticky=tk.W)
        # ttk.Label(buttons_frame, text='Label3').grid(column=0, row=2, sticky=tk.W)

        # for child in buttons_frame.winfo_children():
        #    child.grid_configure(padx=8, pady=40)
        # buttons_frame.winfo_children()[0].grid_configure(padx=8, pady=8)
        # buttons_frame.winfo_children()[1].grid_configure(padx=8, pady=40)
        # buttons_frame.winfo_children()[2].grid_configure(padx=8, pady=80)
        recorder = None
        value = 1
        name = "test"
        if audios != None:
            recorder = audios.get("recorder")
        if recorder != None:
            value = recorder.get("value")
            name = recorder.get("name")
        ttk.Label(frame30, text="麦克风", font=Font1, width=lsize).place(x=orgx, y=offsety)
        offsetx += fsize * lsize
        # 创建一个下拉列表
        self.audioDevice = tk.StringVar()
        self.audioDeviceChosen = ttk.Combobox(frame30, font=Font1, width=combSize, textvariable=self.audioDevice,
                                         state='readonly')
        self.audioDeviceChosen['values'] = ("hw:0,0", "macro", 4, 42, 100)  # 设置下拉列表的值
        # numberChosen.grid(column=0, row=0)  # 设置其在界面中出现的位置  column代表列   row 代表行
        self.audioDeviceChosen.current(value)  # 设置下拉列表默认显示的值，0为 numberChosen['values'] 的下标值
        # state = 'readonly'  # 将下拉列表设置成为只读模式
        self.audioDeviceChosen.place(x=offsetx, y=offsety)
        offsety += step

        #print("CreateDevice: audioDeviceChosen.get()=", self.audioDeviceChosen.get())
        #print("CreateDevice: audioDeviceChosen.current()=", self.audioDeviceChosen.current())

        if recorder != None:
            #print("CreateDevice: 0: recorder=", recorder)
            pass
        else:
            #print("CreateDevice: 1: recorder=", recorder)
            name = self.audioDeviceChosen['values'][value]
            audios.update({"recorder":{"value": value, "name": name}})

        codec = None
        value = 0
        if audios != None:
            codec = audios.get("codec")
        if codec != None:
            value = codec.get("value")
            name = codec.get("name")
        ttk.Label(frame30, text="编码器", font=Font1, width=lsize).place(x=orgx, y=offsety)
        self.audioCodec = tk.StringVar()
        self.audioCodecChosen = ttk.Combobox(frame30, font=Font1, width=combSize, textvariable=self.audioCodec,
                                        state='readonly')
        self.audioCodecChosen['values'] = ("AAC", "OPUS", 4, 42, 100)  # 设置下拉列表的值
        # numberChosen.grid(column=0, row=0)  # 设置其在界面中出现的位置  column代表列   row 代表行
        self.audioCodecChosen.current(value)  # 设置下拉列表默认显示的值，0为 numberChosen['values'] 的下标值
        self.audioCodecChosen.place(x=offsetx, y=offsety)
        offsety += step

        samplerate = None
        value = 4
        if audios != None:
            samplerate = audios.get("samplerate")
        if samplerate != None:
            value = samplerate.get("value")
            name = samplerate.get("name")
        ttk.Label(frame30, text="采样率", font=Font1, width=lsize).place(x=orgx, y=offsety)
        self.audioSampleRate = tk.StringVar()
        self.audioSampleRateChosen = ttk.Combobox(frame30, font=Font1, width=combSize, textvariable=self.audioSampleRate,
                                             state='readonly')
        self.audioSampleRateChosen['values'] = (800, 16000, 32000, 441000, 48000, 96000)  # 设置下拉列表的值
        self.audioSampleRateChosen.current(value)  # 设置下拉列表默认显示的值，0为 numberChosen['values'] 的下标值
        self.audioSampleRateChosen.place(x=offsetx, y=offsety)
        offsety += step

        channels = None
        value = 1
        if audios != None:
            channels = audios.get("channels")
        if channels != None:
            value = channels.get("value")
            name = channels.get("name")
        ttk.Label(frame30, text="通道数", font=Font1, width=lsize).place(x=orgx, y=offsety)
        self.audioChannels = tk.StringVar()
        self.audioChannelsChosen = ttk.Combobox(frame30, font=Font1, width=combSize, textvariable=self.audioChannels,
                                           state='readonly')
        self.audioChannelsChosen['values'] = (1, 2)  # 设置下拉列表的值
        self.audioChannelsChosen.current(value)  # 设置下拉列表默认显示的值，0为 numberChosen['values'] 的下标值
        self.audioChannelsChosen.place(x=offsetx, y=offsety)
        offsety += step

        audiofmt = None
        value = 0
        if audios != None:
            audiofmt = audios.get("audiofmt")
        if audiofmt != None:
            value = audiofmt.get("value")
            name = audiofmt.get("name")
        ttk.Label(frame30, text="数据格式", font=Font1, width=lsize).place(x=orgx, y=offsety)
        self.audioFmt = tk.StringVar()
        self.audioFmtChosen = ttk.Combobox(frame30, font=Font1, width=combSize, textvariable=self.audioFmt,
                                      state='readonly')
        self.audioFmtChosen['values'] = ("default", "S16L")  # 设置下拉列表的值
        self.audioFmtChosen.current(value)  # 设置下拉列表默认显示的值，0为 numberChosen['values'] 的下标值
        self.audioFmtChosen.place(x=offsetx, y=offsety)
        offsety += step

        framesize = None
        value = 1
        if audios != None:
            framesize = audios.get("framesize")
        if framesize != None:
            value = framesize.get("value")
            name = framesize.get("name")
            print("CreateDevice: framesize: value=", value)
            print("CreateDevice: framesize: name=", name)
        ttk.Label(frame30, text="帧大小", font=Font1, width=lsize).place(x=orgx, y=offsety)
        self.audioFrameSize = tk.StringVar()
        self.audioFrameSizeChosen = ttk.Combobox(frame30, font=Font1, width=combSize, textvariable=self.audioFrameSize,
                                            state='readonly')
        self.audioFrameSizeChosen['values'] = ("default", 1024, 2048)  # 设置下拉列表的值
        self.audioFrameSizeChosen.current(value)  # 设置下拉列表默认显示的值，0为 numberChosen['values'] 的下标值
        self.audioFrameSizeChosen.place(x=offsetx, y=offsety)
        offsety += step

        # 第6步，定义触发函数功能
        def print_selection():
            #if (self.denoise.get() == 1) & (self.agc.get() == 0):  # 如果选中第一个选项，未选中第二个选项
            #    # l.config(text='I love only Python ')
            #    print("select 0")
            #elif (self.denoise.get() == 0) & (self.agc.get() == 1):  # 如果选中第二个选项，未选中第一个选项
            #    # l.config(text='I love only C++')
            #    print("select 1")
            #elif (self.denoise.get() == 0) & (self.agc.get() == 0):  # 如果两个选项都未选中
            #    # l.config(text='I do not love either')
            #    print("select 2")
            #else:
            #    # l.config(text='I love both')  # 如果两个选项都选中
            #    print("select all")
            pass

        # 第5步，定义两个Checkbutton选项并放置
        self.denoise = tk.IntVar()  # 定义var1和var2整型变量用来存放选择行为返回值
        self.agc = tk.IntVar()
        self.aec = tk.IntVar()
        self.detect = tk.IntVar()
        self.vad = tk.IntVar()
        # chkSize = 20 #fsize * lsize
        #Checkbutton 的值不仅仅是 1 或 0，可以是其他类型的数值，可以通过 onvalue 和 offvalue 属性设置 Checkbutton 的状态值。
        orgx1 = 150

        denoiseValue = 1
        if audios != None:
            denoiseValue = audios.get("denoise")
            if denoiseValue == None:
                denoiseValue = 1

        self.selectDenoise = tk.Checkbutton(frame30, text='去噪声', font=Font1, variable=self.denoise, onvalue=1,
                                            offvalue=0,
                                            command=print_selection)  # 传值原理类似于radiobutton部件
        self.selectDenoise.place(x=orgx, y=offsety)
        # offsety += step

        aecValue = 0
        if audios != None:
            if audios.get("aec") != None:
                aecValue = audios.get("aec")
        self.selectAec = tk.Checkbutton(frame30, text='回声消除', font=Font1, variable=self.aec, onvalue=1, offvalue=0,
                                  command=print_selection)
        self.selectAec.place(x=orgx1, y=offsety)
        offsety += step

        vadValue = 0
        if audios != None:
            if audios.get("vad") != None:
                vadValue = audios.get("vad")
        self.selectVad = tk.Checkbutton(frame30, text='静音检测', font=Font1, variable=self.vad, onvalue=1, offvalue=0,
                                  command=print_selection)
        self.selectVad.place(x=orgx, y=offsety)
        # offsety += step

        agcValue = 0
        if audios != None:
            if audios.get("agc") != None:
                agcValue = audios.get("agc")
        self.selectAgc = tk.Checkbutton(frame30, text='自动增益控制', font=Font1, variable=self.agc, onvalue=1, offvalue=0,
                                  command=print_selection)
        self.selectAgc.place(x=orgx1, y=offsety)
        offsety += step

        detectValue = 0
        if audios != None:
            if audios.get("detect") != None:
                detectValue = audios.get("detect")
        self.selectDetect = tk.Checkbutton(frame30, text='语音/音频检测', font=Font1, variable=self.detect, onvalue=1, offvalue=0,
                                     command=print_selection)
        self.selectDetect.place(x=orgx, y=offsety)
        offsety += step

        self.denoise.set(denoiseValue)
        self.agc.set(agcValue)
        self.aec.set(aecValue)
        self.detect.set(detectValue)
        self.vad.set(vadValue)

        #self.denoise.get()
        # 第4步，在图形界面上创建一个标签label用以显示并放置
        # l = tk.Label(frame30, bg='green', fg='white', width=20, text='empty')
        # l.place(x=orgx, y=offsety)

        # 第6步，定义一个触发函数功能
        def print_selection(v):
            # l.config(text='音量 ' + v)
            print(v)

        def preplay():
            print("audio pre play")

        orgx2 = 210
        # 第5步，创建一个尺度滑条，长度200字符，从0开始10结束，以2为刻度，精度为0.01，触发调用print_selection函数
        s = tk.Scale(frame30, font=Font1, label='音量', from_=0, to=255, orient=tk.HORIZONTAL, length=200, showvalue=0,
                     tickinterval=50, resolution=1.0, command=print_selection)
        s.place(x=orgx, y=offsety)
        s.set(100)

        self.btnAudioPlay = tk.Button(frame30, text="播放", command=preplay)
        self.btnAudioPlay.place(x=orgx2, y=offsety)

        # Create a video container to hold labels
        # frame31 = ttk.LabelFrame(frame3, text='视频设置', width=600, height=700)
        # frame31.place(x=300, y=0)

        frame31 = ttk.LabelFrame(self.parentFrame, text='视频设置', width=self.width, height=300)
        self.videoFarame = frame31
        frame31.place(x=0, y=400)

        (orgx, orgy, offsetx, offsety, step) = (8, 8, 8, 8, 32)
        (fSize, lSize, combSize) = (10, 11, 14)
        ttk.Label(frame31, text="摄像头", font=Font1, width=lSize).place(x=orgx, y=offsety)
        offsetx += fsize * lsize

        capture = None
        value = 1
        name = "test"
        if videos != None:
            capture = videos.get("capture")
        if capture != None:
            if capture.get("value") != None:
                value = capture.get("value")
            if capture.get("name") != None:
                name = capture.get("name")
        # 创建一个下拉列表
        self.videoDevice = tk.StringVar()
        self.videoDeviceChosen = ttk.Combobox(frame31, font=Font1, width=combSize, textvariable=self.videoDevice, state='readonly')
        self.videoDeviceChosen['values'] = ("video0", "fb0", "RTMP", "RTSP", "其他")  # 设置下拉列表的值
        self.videoDeviceChosen.current(value)  # 设置下拉列表默认显示的值，0为 numberChosen['values'] 的下标值
        self.videoDeviceChosen.place(x=offsetx, y=offsety)
        offsety += step

        if capture != None:
            print("CreateDevice: 0: capture=", capture)
        else:
            print("CreateDevice: 1: capture=", capture)
            name = self.videoDeviceChosen['values'][value]
            videos.update({"capture":{"value": value, "name": name}})

        codec = None
        value = 0
        if videos != None:
            codec = videos.get("codec")
        if codec != None:
            if codec.get("value") != None:
                value = codec.get("value")
            if codec.get("name") != None:
                name = codec.get("name")
        ttk.Label(frame31, text="编码器", font=Font1, width=lSize).place(x=orgx, y=offsety)
        self.videoCodec = tk.StringVar()
        self.videoCodecChosen = ttk.Combobox(frame31, font=Font1, width=combSize, textvariable=self.videoCodec,
                                        state='readonly')
        self.videoCodecChosen['values'] = ("HCSVC", "H264", "openH264", "openH265", "VP8", "VP9")  # 设置下拉列表的值
        self.videoCodecChosen.current(value)  # 设置下拉列表默认显示的值，0为 numberChosen['values'] 的下标值
        self.videoCodecChosen.place(x=offsetx, y=offsety)
        offsety += step

        resolution = None
        value = 1
        if videos != None:
            resolution = videos.get("resolution")
        if resolution != None:
            if resolution.get("value") != None:
                value = resolution.get("value")
            if resolution.get("name") != None:
                name = resolution.get("name")
        ttk.Label(frame31, text="分辨率", font=Font1, width=lSize).place(x=orgx, y=offsety)
        self.videoResolution = tk.StringVar()
        self.videoResolutionChosen = ttk.Combobox(frame31, font=Font1, width=combSize, textvariable=self.videoResolution,
                                             state='readonly')
        self.videoResolutionChosen['values'] = (
            "1920x1080", "1280x720", "960x540", "704x576", "640x480", "352x288", "320x240", "其他")
        self.videoResolutionChosen.current(value)  # 设置下拉列表默认显示的值，0为 numberChosen['values'] 的下标值
        self.videoResolutionChosen.place(x=offsetx, y=offsety)
        offsety += step

        framerate = None
        value = 1
        if videos != None:
            framerate = videos.get("framerate")
        if framerate != None:
            if framerate.get("value") != None:
                value = framerate.get("value")
            if framerate.get("name") != None:
                name = framerate.get("name")
        ttk.Label(frame31, text="帧率", font=Font1, width=lSize).place(x=orgx, y=offsety)
        self.videoFrameRate = tk.StringVar()
        self.videoFrameRateChosen = ttk.Combobox(frame31, font=Font1, width=combSize, textvariable=self.videoFrameRate,
                                            state='readonly')
        self.videoFrameRateChosen['values'] = ("5fps", "15fps", "25fps", "30fps", "60fps", "其他")
        self.videoFrameRateChosen.current(value)  # 设置下拉列表默认显示的值，0为 numberChosen['values'] 的下标值
        self.videoFrameRateChosen.place(x=offsetx, y=offsety)
        offsety += step

        bitrate = None
        value = 1
        if videos != None:
            bitrate = videos.get("bitrate")
        if bitrate != None:
            if bitrate.get("value") != None:
                value = bitrate.get("value")
            if bitrate.get("name") != None:
                name = bitrate.get("name")
        ttk.Label(frame31, text="码率", font=Font1, width=lSize).place(x=orgx, y=offsety)
        self.videoBitRate = tk.StringVar()
        self.videoBitRateChosen = ttk.Combobox(frame31, font=Font1, width=combSize, textvariable=self.videoBitRate,
                                          state='readonly')
        self.videoBitRateChosen['values'] = (
            "256kbps", "512kbps", "800kbps", "1000kbps", "1500kbps", "2000kbps", "2500kbps", "3000kbps", "6000kbps",
            "其他")
        self.videoBitRateChosen.current(value)  # 设置下拉列表默认显示的值，0为 numberChosen['values'] 的下标值
        self.videoBitRateChosen.place(x=offsetx, y=offsety)
        offsety += step

        self.vdenoise = tk.IntVar()
        self.vFEC = tk.IntVar()

        denoiseValue = 1
        if videos != None:
            if videos.get("denoise") != None:
                denoiseValue = videos.get("denoise")
        self.selectVideoDenoise = tk.Checkbutton(frame31, text='去噪声', font=Font1, variable=self.vdenoise, onvalue=1,
                                                 offvalue=0,
                                                 command=print_selection)  # 传值原理类似于radiobutton部件
        self.selectVideoDenoise.place(x=orgx, y=offsety)
        # offsety += step

        fecValue = 0
        if videos != None:
            if videos.get("fec") != None:
                fecValue = videos.get("fec")
        self.selectVideoFEC = tk.Checkbutton(frame31, text='FEC', font=Font1, variable=self.vFEC, onvalue=1,
                                             offvalue=0,
                                             command=print_selection)  # 传值原理类似于radiobutton部件
        self.selectVideoFEC.place(x=orgx1, y=offsety)
        # offsety += step

        self.vdenoise.set(denoiseValue)
        self.vFEC.set(fecValue)

        if fecValue > 0:
            self.vFEC.set(fecValue)

        def preview():
            print("video pre view")

        self.btnVideoView = tk.Button(frame31, text="预览", command=preview)
        self.btnVideoView.place(x=orgx2, y=offsety)
        offsety += step
        ret["audios"] = audios
        ret["videos"] = videos
        self.dict = ret
        return ret

#refer to:
# https://vlight.me/2017/12/04/Layout-Management/
class ConfigFrame(object):
    def __init__(self, parent):
        self.parent = parent
        self.config_frame = None
        self.iv0_0 = 0
        self.iv0 = tk.IntVar()
        self.json = JsonFile("config.json")
        data = self.json.readfile("")
        #if data != None and data != {}:
        #    for key, value in data.items():
        #        print("(key, value)= ", (key, value))

    def on_config_closing(self):
        print("config frame close")
        #self.save_config()
        if self.config_frame != None:
            self.config_canvas.delete("all")
            self.config_canvas.destroy()
            #del self.config_canvas
            self.config_canvas = None
            self.config_frame.destroy()
            #del self.config_frame
            self.config_frame = None

            self.iv0_0 = 0
            self.iv0.set(self.iv0_0)

    def callCheckbutton(self):
        self.v0.set("hello Checkbutton")
    def channel0(self):
        print("ConfigFrame: channel0")
    def channel1(self):
        print("ConfigFrame: channel1")
    def channel2(self):
        print("ConfigFrame: channel2")
    def channel3(self):
        print("ConfigFrame: channel3")
    def channel4(self):
        print("ConfigFrame: channel4")
    def channel5(self):
        print("ConfigFrame: channel5")
    def exit(self):
        print("ConfigFrame: exit")
    def save_config(self):
        multdevice = self.json.dict.get("multdevice")
        if multdevice != None:
            #print("ConfigFrame:save_config: len(multdevice)= ", len(multdevice))
            pass
        else:
            multdevice = {}
        for configDevice in self.configDeviceFrameList:
            deviceDict = configDevice.SaveConfig()
            if deviceDict != None:
                deviceId = configDevice.deviceId
                # multdevice[str(deviceId)] = deviceDict
                multdevice.update({str(deviceId): deviceDict})
        if multdevice != None:
            #print("ConfigFrame:save_config: len(multdevice)= ", len(multdevice))
            for key, value in multdevice.items():
                #print("ConfigFrame:save_config: key= ", key)
                if value != None:
                    for key2, value2 in value.items():
                        #print("ConfigFrame:save_config: key2= ", key2)
                        if value2 != None and False:
                            for key3, value3 in value2.items():
                                print("ConfigFrame:save_config: key3= ", key3)
                                print("ConfigFrame:save_config: value3= ", value3)
            self.json.dict.update({"multdevice" : multdevice})
            self.json.writefile(None, "")
    def open_config_frame(self):
        multdevice = self.json.dict.get("multdevice")
        if multdevice != None:
            print("ConfigFrame:open_config_frame: len(multdevice)= ", len(multdevice))
        else:
            multdevice = {}
        generalDict = self.json.dict.get("general")
        if generalDict != None:
            print("ConfigFrame:open_config_frame: len(generalDict)= ", len(generalDict))
        else:
            generalDict = {}
        #self.hide()
        if True:
            # self.status = True
            self.config_frame = tk.Toplevel()

            cvalue0 = "[欢迎进入会畅SVC调试系统]" + "\t" + "\t"
            cvalue1 = "参数设置"
            self.config_frame.title(cvalue0 + cvalue1)
            (width0, height0) = (1280, 720)
            str_resolution = str(width0) + "x" + str(height0)
            self.config_frame.geometry(str_resolution)  # "1280x720"
            self.config_imagefile = tk.PhotoImage(file='icon/hc_2.png')  # .zoom(2)

            if False:
                self.menubar = tk.Menu(self.config_frame)

                self.pageMenu = tk.Menu(self.menubar, tearoff=0)
                self.menubar.add_cascade(label="参数设置", menu=self.pageMenu)
                self.pageMenu.add_command(label="通道0", command=self.channel0)
                self.pageMenu.add_command(label="通道1", command=self.channel1)
                self.pageMenu.add_command(label="通道2", command=self.channel2)
                self.pageMenu.add_command(label="通道3", command=self.channel3)
                self.pageMenu.add_command(label="合成",  command=self.channel4)
                self.pageMenu.add_command(label="媒体保存", command=self.channel5)
                self.pageMenu.add_separator()
                self.pageMenu.add_command(label="退出", command=self.exit)
                ###
                self.menu_chan1 = tk.Menu(self.menubar, tearoff=0)
                self.menubar.add_cascade(label="设置通道1", menu=self.menu_chan1)

                # 关联窗口
                self.config_frame.config(menu=self.menubar)
            if True:
                ttk.Style().configure(".", font=("仿宋", 12))
                tab = ttk.Notebook(self.config_frame)

                frame0 = tk.Frame(tab, bg="blue")
                tab0 = tab.add(frame0, text="设备管理")

                frame1 = tk.Frame(tab, bg="yellow")
                tab1 = tab.add(frame1, text="通用")

                #frame2 = tk.Frame(tab, bg="green")
                #tab2 = tab.add(frame2, text="显示屏")

                #frame3 = tk.Frame(tab, bg="red")
                #tab3 = tab.add(frame3, text="高级调参")

                #frame4 = tk.Frame(tab, bg="white")
                #tab4 = tab.add(frame4, text="合成通道")

                tab.pack(expand=True, fill=tk.BOTH)

                # 设置选中tab3
                tab.select(frame0)
            if False:
                m1 = PanedWindow(frame2)  # 默认是左右分布的
                m1.pack(fill=BOTH, expand=1)

                #left = Label(m1, text='标签1', bg='blue', width=20)
                left = Label(m1, text='标签1', bg='blue')
                m1.add(left)

                m2 = PanedWindow(frame2, orient=VERTICAL)
                m1.add(m2)

                #top = Label(m2, text='标签2', bg='green', height=20)
                top = Label(m2, text='标签2', bg='green')
                m2.add(top)

                bottom = Label(m2, text='标签3', bg='red')
                m2.add(bottom)

            self.config_canvas = tk.Canvas(frame1, height=self.parent.height, width=self.parent.width)
            self.config_image2 = self.config_canvas.create_image(0, 0, anchor='nw', image=self.config_imagefile)
            self.config_canvas.pack(side='top')

            #self.config_canvas.place(x=0, y=0)
            # label_img = tk.Label(self.meeting_frame, image=self.imagefile1)
            # label_img.pack()
            #self.winid = self.config_canvas.winfo_id()

            if False:
                m1 = PanedWindow(frame2)  # 默认是左右分布的
                #m1.pack(fill=BOTH, expand=1)
                m1.place(x=0, y=0)

                #left = Label(m1, text='标签1', bg='blue', width=20)
                left = Label(m1, text='标签1')
                m1.add(left)

                m2 = PanedWindow(frame2, orient=VERTICAL)
                m1.add(m2)

                #top = Label(m2, text='标签2', bg='green', height=20)
                top = Label(m2, text='标签2')
                m2.add(top)

                bottom = Label(m2, text='标签3')
                m2.add(bottom)

            if False:
                sh = ttk.Separator(frame2, orient='horizontal', style='red.TSeparator')
                sh.pack(fill=tk.X)
                sv = ttk.Separator(frame2, orient=VERTICAL)
                sv.pack(fill=tk.Y)
            if False:
                # Create a container to hold labels
                buttons_frame = ttk.LabelFrame(frame1, text=' Label in a Frame')
                buttons_frame.grid(column=0, row=7)
                # buttons_frame.grid(column=1, row=6)

                # Place labels into the container element
                ttk.Label(buttons_frame, text='Label1').grid(column=0, row=0, sticky=tk.W)
                ttk.Label(buttons_frame, text='Lable2').grid(column=1, row=0, sticky=tk.W)
                ttk.Label(buttons_frame, text='Label3').grid(column=2, row=0, sticky=tk.W)

                #name_entered.focus()  # Place cursor into name Entry

            if False:
                # Create a container to hold labels
                buttons_frame = ttk.LabelFrame(frame0, text=' Label in a Frame')
                buttons_frame.grid(column=0, row=7)
                # buttons_frame.grid(column=1, row=6)

                # Place labels into the container element
                ttk.Label(buttons_frame, text='Label1').grid(column=0, row=0, sticky=tk.W)
                ttk.Label(buttons_frame, text='Lable2').grid(column=0, row=1, sticky=tk.W)
                ttk.Label(buttons_frame, text='Label3').grid(column=0, row=2, sticky=tk.W)

                #for child in buttons_frame.winfo_children():
                #    child.grid_configure(padx=8, pady=40)
                buttons_frame.winfo_children()[0].grid_configure(padx=8, pady=8)
                buttons_frame.winfo_children()[1].grid_configure(padx=8, pady=40)
                buttons_frame.winfo_children()[2].grid_configure(padx=8, pady=80)

                #nameEntered.focus()  # Place cursor into name Entry
            if False:
                # 第4步，在图形界面上创建一个标签label用以显示并放置
                var1 = tk.StringVar()  # 创建变量，用var1用来接收鼠标点击具体选项的内容
                l = tk.Label(frame1, bg='green', fg='yellow', font=('Arial', 12), width=10, textvariable=var1)
                l.pack()

                # 第6步，创建一个方法用于按钮的点击事件
                def print_selection():
                    value = lb.get(lb.curselection())  # 获取当前选中的文本
                    var1.set(value)  # 为label设置值

                # 第5步，创建一个按钮并放置，点击按钮调用print_selection函数
                b1 = tk.Button(frame1, text='print selection', width=15, height=2, command=print_selection)
                b1.pack()

                # 第7步，创建Listbox并为其添加内容
                var2 = tk.StringVar()
                var2.set((1, 2, 3, 4))  # 为变量var2设置值
                # 创建Listbox
                lb = tk.Listbox(frame1, listvariable=var2)  # 将var2的值赋给Listbox
                # 创建一个list并将值循环添加到Listbox控件中
                list_items = [11, 22, 33, 44]
                for item in list_items:
                    lb.insert('end', item)  # 从最后一个位置开始加入值
                lb.insert(1, 'first')  # 在第一个位置加入'first'字符
                lb.insert(2, 'second')  # 在第二个位置加入'second'字符
                lb.delete(2)  # 删除第二个位置的字符
                lb.pack()
            if False:
                def go(*args):  # 处理事件，*args表示可变参数
                    print(comboxlist.get())  # 打印选中的值

                comvalue = tkinter.StringVar()  # 窗体自带的文本，新建一个值
                comboxlist = ttk.Combobox(frame3, textvariable=comvalue)  # 初始化
                comboxlist["values"] = ("1", "2", "3", "4")
                comboxlist.current(0)  # 选择第一个
                comboxlist.bind("<<ComboboxSelected>>", go)  # 绑定事件,(下拉列表框被选中时，绑定go()函数)
                comboxlist.pack()
            ###deviceframe
            self.deviceNum = 4
            offset = 0
            self.framelist = []
            self.configDeviceFrameList = []
            for deviceId in range(self.deviceNum):
                # Create an audio container to hold labels
                deviceFrame = ttk.LabelFrame(frame0, text='通道'+str(deviceId), width=280, height=720)
                deviceFrame.place(x=offset, y=0)
                deviceDict = None
                if multdevice != None:
                    deviceDict = multdevice.get(str(deviceId))
                self.device0 = ConfigDeviceFrame(self, deviceFrame, deviceId, deviceDict)
                deviceDict = self.device0.CreateDevice()
                if deviceDict != None:
                    #multdevice[str(deviceId)] = deviceDict
                    multdevice.update({str(deviceId) : deviceDict})
                offset += 280
                self.framelist.append(deviceFrame)
                self.configDeviceFrameList.append(self.device0)
            if multdevice != None and False:
                print("ConfigFrame:open_config_frame: len(multdevice)= ", len(multdevice))
                for key,value in multdevice.items():
                    print("ConfigFrame:open_config_frame: key= ", key)
                    if value != None:
                        for key2, value2 in value.items():
                            print("ConfigFrame:open_config_frame: key2= ", key2)
                            if value2 != None:
                                for key3, value3 in value2.items():
                                    print("ConfigFrame:open_config_frame: key3= ", key3)
                                    print("ConfigFrame:open_config_frame: value3= ", value3)
            #self.save_config() ##test
            self.ctrFrame = ConfigControlFrame(self, frame0)
            self.ctrFrame.CreateControlFrame(offset)
            ###genernalframe
            self.genFrame = ConfigGeneralFrame(self, frame1, width0, height0, generalDict)
            modeDict = self.genFrame.CreateSplitFrame()
            mcuDict = self.genFrame.CreateMcuFrame()
            suppramsDict = self.genFrame.CreateSuppramsFrame()
            if modeDict != None:
                generalDict.update({"mode": modeDict})
            if mcuDict != None:
                generalDict.update({"mcu": mcuDict})
            if suppramsDict != None:
                generalDict.update({"supprams": suppramsDict})
            #generalDict = self.genFrame.dict
            if generalDict != None:
                self.json.dict.update({"general": generalDict})

            if False:
                # 文本框
                name = tk.StringVar()  # StringVar是Tk库内部定义的字符串变量类型，在这里用于管理部件上面的字符；不过一般用在按钮button上。改变StringVar，按钮上的文字也随之改变。
                nameEntered = ttk.Entry(frame30, width=14,
                                        textvariable=name)  # 创建一个文本框，定义长度为12个字符长度，并且将文本框中的内容绑定到上一句定义的name变量上，方便clickMe调用
                # nameEntered.grid(column=1, row=0)  # 设置其在界面中出现的位置  column代表列   row 代表行
                nameEntered.focus()  # 当程序运行时,光标默认会出现在该文本框中
                nameEntered.pack()

                # button被点击之后会被执行
                def clickMe():  # 当acction被点击时,该函数则生效
                    # action.configure(text='Hello ' + name.get())  # 设置button显示的内容
                    # action.configure(text='Hello ' + name.get() + ' ' + numberChosen.get())
                    action.configure(text='录制设备：' + ' ' + self.audioDevice.get())
                    # action.configure(state='disabled')  # 将按钮设置为灰色状态，不可使用状态
                # 按钮
                action = ttk.Button(frame30, text="音频设备选择", width=14,
                                    command=clickMe)  # 创建一个按钮, text：显示按钮上面显示的文字, command：当这个按钮被点击之后会调用command函数
                # action.grid(column=1, row=1)  # 设置其在界面中出现的位置  column代表列   row 代表行
                action.pack()

                # 第4步，在图形界面上创建一个标签label用以显示并放置
                l = tk.Label(frame30, bg='yellow', width=20, text='empty')
                l.pack()

            if False:
                # 第4步，在图形界面上创建一个标签label用以显示并放置
                l = tk.Label(frame30, bg='green', fg='white', width=20, text='empty')
                l.pack()

                # 第6步，定义一个触发函数功能
                def print_selection(v):
                    l.config(text='you have selected ' + v)

                # 第5步，创建一个尺度滑条，长度200字符，从0开始10结束，以2为刻度，精度为0.01，触发调用print_selection函数
                s = tk.Scale(frame30, label='volum', from_=0, to=255, orient=tk.HORIZONTAL, length=200, showvalue=0,
                             tickinterval=50, resolution=1.0, command=print_selection)
                s.pack()
                s.set(100)

            #self.config_image = tk.PhotoImage(file='icon/home.png').subsample(3, 4)  # zoom(2, 2)
            #handler = lambda: self.onClosemeeting_frame(self.meeting_frame)
            #self.config_btn_home = tk.Button(self.config_frame, text="home", image=self.config_image,
            #                          command=self.show)  # , width=50, height=30)
            #self.config_btn_home.place(x=0, y=0)

            self.config_frame.update()
            self.config_frame.protocol("WM_DELETE_WINDOW", self.on_config_closing)
    def setting(self):
        #print("setting: (meeting_id, usr_name)= ", (meeting_id, usr_name))
        #当前按钮事件无法正确取出值，其他按钮事件可以正确取出该变量值得
        value = self.iv0.get()
        value = self.iv0_0
        #print("setting: 0: self.iv0= ", value)
        #self.iv0.set(1)
        #return
        if self.iv0_0 == 0:
            self.iv0_0 = 1
            self.iv0.set(self.iv0_0)
            value = self.iv0.get()
            #print("setting: value= ", value)
            self.open_config_frame()
        else:
            self.iv0_0 = 0
            self.iv0.set(self.iv0_0)
            value = self.iv0.get()

            if self.config_frame != None:
                self.config_canvas.delete("all")
                self.config_canvas.destroy()
                #del self.config_canvas
                self.config_canvas = None
                self.config_frame.destroy()
                #del self.config_frame
                self.config_frame = None
            #print("setting: value= ", value)
        return
class Conference(object):
    def __init__(self):
        self.root = None
        #self.test_meeting = ExternalSDL()
        #self.test_meeting.start()
        #self.test_meeting = None
        self.client_frame = None #ClientFrame(self.root)
        self.config_frame = None
        self.top = None
        (self.width, self.height) = (loadlib.WIDTH, loadlib.HEIGHT)

    def on_root_close(self):
        #self.hide()
        if self.config_frame != None:
            #self.config_frame.destroy()
            self.config_frame. on_config_closing()
        #if self.meeting_frame != None and False:
        #    self.meeting_frame.destroy()
        #else:
        #    self.exit_meeting()
        #time.sleep(5)
        if self.client_frame != None:
            self.client_frame.on_meeting_closing()
        if self.client_frame.meeting_frame == None:
            self.root.destroy()

    def hide(self):
        self.root.withdraw()

    def show(self):
        self.root.update()
        self.root.deiconify()

    def onClosemeeting_frame(self, meeting_frame):
        # meeting_frame.destroy()
        self.show()

    def draw(self):
        print("draw")

    def root_frame(self):
        self.root = tk.Tk()
        self.root.title('欢迎进入会畅SVC调试系统')
        self.root.geometry('690x500')
        ##self.frame = tk.Frame(self.root)
        # self.frame = tk.Frame(self.root, width=640, height=480)
        ##self.frame.pack()
        # btn = Tk.Button(self.frame, text="Open Frame", command=self.openFrame)
        # btn.pack()
        # 画布放置图片
        canvas = tk.Canvas(self.root, height=400, width=800)
        #self.canvas = canvas
        #self.winid = canvas.winfo_id()
        #print("log_frame: self.winid= ", self.winid)
        imagefile = tk.PhotoImage(file='hc.png')
        image = canvas.create_image(0, 0, anchor='nw', image=imagefile)
        canvas.pack(side='top')

        #self.mainframe1 = Frame(self.root, height=250, width=250, bg='white')
        #canvas_frame = canvas.create_window((4, 4), window=self.mainframe1, anchor="nw")

        # `在这里插入代码片`
        # 标签 用户名密码
        tk.Label(self.root, text='会议号:').place(x=200, y=320)
        tk.Label(self.root, text='帐号:').place(x=200, y=360)
        # 会议号输入框
        self.var_meeting_id = tk.StringVar()
        entry_meeting_id = tk.Entry(self.root, textvariable=self.var_meeting_id)
        entry_meeting_id.place(x=260, y=320)
        # 帐号输入框
        self.var_usr_name = tk.StringVar()
        entry_usr_name = tk.Entry(self.root, textvariable=self.var_usr_name)
        entry_usr_name.place(x=260, y=360)
        # 登录 注册按钮
        # image = Image.open('icon/video_chat.png')
        # image = image.resize((40, 60), Image.ANTIALIAS)
        # image0 = tk.PhotoImage(image)
        image0 = tk.PhotoImage(file='icon/video_chat.png').subsample(3, 4)  # zoom(2, 2)
        self.bt_login = tk.Button(self.root, text='创建', image=image0, command=self.create_meeting, width=50, height=30)
        self.bt_login.place(x=200, y=400)
        # self.bt_login.pack()

        image1 = tk.PhotoImage(file='icon/call.png').subsample(3, 4)
        self.bt_logup = tk.Button(self.root, text='加入', image=image1, command=self.enter_meeting, width=50, height=30)
        self.bt_logup.place(x=270, y=400)
        # self.bt_logup.pack()

        image2 = tk.PhotoImage(file='icon/webcam.png').subsample(3, 4)
        self.bt_logquit = tk.Button(self.root, text='观看', image=image2, command=self.watch_meeting, width=50, height=30)
        self.bt_logquit.place(x=340, y=400)
        # self.bt_logquit.pack()

        image3 = tk.PhotoImage(file='icon/exit.png').subsample(3, 4)
        self.bt_exit = tk.Button(self.root, text='退出', image=image3, command=self.exit_meeting, width=50, height=30)
        self.bt_exit.place(x=410, y=400)
        # self.bt_config.pack()

        #image4 = tk.PhotoImage(file='icon/config.png').subsample(3, 4)
        #self.bt_config = tk.Button(self.root, text='设置', image=image4, command=self.setting, width=50, height=30)
        #self.bt_config.place(x=480, y=400)

        self.client_frame = ClientFrame(self)  # .root)
        self.config_frame = ConfigFrame(self)  # .root)

        #self.iv0_0 = 0
        #self.iv0 = tk.IntVar()
        image4 = tk.PhotoImage(file='icon/config.png').subsample(3, 4)
        self.bt_config = tk.Radiobutton(self.root, text='设置', value=1, variable=self.config_frame.iv0, image=image4, command=self.setting, indicatoron=0, width=50, height=30)
        #self.bt_config = tk.Radiobutton(self.root, text='设置', value=1, image=image4, command=self.setting, indicatoron=0, width=50, height=30)
        #self.iv0.set(self.iv0_0)
        self.config_frame.iv0.set(0)
        self.bt_config.place(x=480, y=400)

        self.root.protocol("WM_DELETE_WINDOW", self.on_root_close)  # 只要其中一个窗口关闭,就同时关闭两个窗口
        self.root.mainloop()
        #self.exit_meeting()
        print("root exit")

    def create_meeting(self):
        meeting_id = self.var_meeting_id.get()
        usr_name = self.var_usr_name.get()
        print("create_meeting: (meeting_id, usr_name)= ", (meeting_id, usr_name))
        # self.winid = self.image.winfo_id()
        #self.winid = self.mainframe1.winfo_id()
        #if self.test_meeting == None:
        #    self.test_meeting = ReadFrame(self.winid)
        #    self.test_meeting.init()
        #    self.test_meeting.start()
        #if self.meeting_frame != None:
        #    self.meeting_frame.update()
        #    self.meeting_frame.deiconify()

    def enter_meeting(self):
        meeting_id = self.var_meeting_id.get()
        usr_name = self.var_usr_name.get()
        print("enter_meeting: (meeting_id, usr_name)= ", (meeting_id, usr_name))
        if self.client_frame != None:
            self.client_frame.enter_meeting()
        #self.exit_meeting()
        #self.openFrame()

    def watch_meeting(self):
        meeting_id = self.var_meeting_id.get()
        usr_name = self.var_usr_name.get()
        print("watch_meeting: (meeting_id, usr_name)= ", (meeting_id, usr_name))

        return

    def exit_meeting(self):
        meeting_id = self.var_meeting_id.get()
        usr_name = self.var_usr_name.get()
        print("exit_meeting: (meeting_id, usr_name)= ", (meeting_id, usr_name))
        if self.client_frame != None:
            self.client_frame.exit_meeting()
        print("Conference: exit_meeting end")

        return

    def setting(self):
        meeting_id = self.var_meeting_id.get()
        usr_name = self.var_usr_name.get()
        #print("setting: (meeting_id, usr_name)= ", (meeting_id, usr_name))
        if self.config_frame != None:
            self.config_frame.setting()

def mytest():
    root = Tk()
    w = Label(root, text="Label Title")
    w.pack()

    mb.showinfo("welcome", "Welcome Message")
    guess = dl.askinteger("Number", "Enter a number")

    output = 'This is output message'
    mb.showinfo("Output:", output)
    return

if __name__ == '__main__':
    print('Start pycall.')
    #mytest()

    call = Conference()
    #call.start()
    call.root_frame()

    #call = Pyui()
    # call.FrameTest()
    #call.FrameCallSDL()
    print('End pycall.')

# -*- coding: utf-8 -*


import sys
import os
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
class Conference(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.lock = threading.Lock()
        self.__flag = threading.Event()  # 用于暂停线程的标识
        self.__flag.set()  # 设置为True
        self.__running = threading.Event()  # 用于停止线程的标识
        self.__running.set()  # 将running设置为True
        self.pause()

        self.root = None
        self.test_meeting = None
        self.otherFrame = None
        self.top = None
        (self.width, self.height) = (loadlib.WIDTH, loadlib.HEIGHT)

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
            if self.test_meeting == None:
                self.test_meeting = ReadFrame(self.winid)
                self.test_meeting.init()
                self.test_meeting.start()
                self.pause()
        self.stop()
    def hide(self):
        self.root.withdraw()

    def show(self):
        self.root.update()
        self.root.deiconify()

    def onCloseOtherFrame(self, otherFrame):
        # otherFrame.destroy()
        self.show()

    def draw(self):
        #self.winid = self.otherFrame.winfo_id()
        if self.test_meeting == None:
            self.test_meeting = ReadFrame(self.winid)
            self.test_meeting.init()
            self.test_meeting.start()

    def openFrame(self):
        self.hide()

        if self.otherFrame != None:
            if self.test_meeting != None:
                self.test_meeting.sdl_push_event(4)
                self.test_meeting = None
                time.sleep(1)
            self.otherFrame.destroy()
            self.otherFrame = None
        if True:
            otherFrame = tk.Toplevel()
            self.otherFrame = otherFrame

            otherFrame.title("欢迎进入会畅SVC调试系统")
            str_resolution = str(self.width) + "x" + str(self.height)
            otherFrame.geometry(str_resolution) #"1280x720"
            #otherFrame.attributes("-toolwindow", 1)
            #otherFrame.iconbitmap("My icon.ico")
            #otherFrame.attributes("-alpha", 0.5)

            if False:
                menubar = tk.Menu(otherFrame)
                pageMenu = tk.Menu(menubar)
                pageMenu.add_command(label="PageOne")
                menubar.add_cascade(label="PageOne", menu=pageMenu)
                #关联窗口
                otherFrame.config(menu=menubar)

            self.imagefile1 = tk.PhotoImage(file='icon/hc_2.png')#.zoom(2)
            self.canvas2 = tk.Canvas(otherFrame, height=self.height, width=self.width)
            self.image_2 = self.canvas2.create_image(0, 0, anchor='nw', image=self.imagefile1)
            self.canvas2.pack(side='top')
            #label_img = tk.Label(otherFrame, image=self.imagefile1)
            #label_img.pack()
            self.winid = self.canvas2.winfo_id()

            self.image1 = tk.PhotoImage(file='icon/home.png').subsample(3, 4)  # zoom(2, 2)
            handler = lambda: self.onCloseOtherFrame(otherFrame)
            self.btn_home = tk.Button(otherFrame, text="home", image=self.image1, command=handler)  # , width=50, height=30)
            self.btn_home.place(x=0, y=0)
            # btn_home.pack()
            #self.btn_home.attributes('-topmost', True)
            #otherFrame.attributes('-topmost', True)

            #self.image0 = tk.PhotoImage(file='icon/video_start.png').subsample(3, 4)  # zoom(2, 2)
            #self.bt_video_start = tk.Button(otherFrame, text='启动', image=self.image0, command=self.draw)  # , width=50, height=30)
            #self.bt_video_start.place(x=60, y=0)
            #self.bt_video_start.pack()

            otherFrame.update()
            #self.draw()
            self.resume()
            # button2 = tk.Button(otherFrame, text='bmp', bitmap = 'error')
            # button2.pack()
            if True:
                num = 0
                #self.itext = self.canvas2.create_text((self.width - 300), 0, text=str(num), font=("Comic Sans", 15),fill='yellow',anchor='nw')
                while num < 70000000:
                    num += 1

                    ###
                    if False:
                        cvalue = "framerate:" + str(num) + "fps_"
                        self.btn_fps = tk.Button(otherFrame, text=cvalue) #, state="DISABLED")#, width=20, height=10)
                        self.btn_fps.place(x=(self.width - 200), y=0)
                        #self.btn_text.config(bg='systemTransparent')
                        cvalue = "bitsrate_: " + str(num) + "kbps"
                        self.btn_bps = tk.Button(otherFrame, text=cvalue)  # , state="DISABLED")#, width=20, height=10)
                        self.btn_bps.place(x=(self.width - 200), y=30)

                        cvalue = "lossrate_: " + str(num) + "%___"
                        self.btn_loss = tk.Button(otherFrame, text=cvalue)  # , state="DISABLED")#, width=20, height=10)
                        self.btn_loss.place(x=(self.width - 200), y=60)

                        cvalue = "cpu______: " + str(num) + "%___"
                        self.btn_cpu = tk.Button(otherFrame, text=cvalue)  # , state="DISABLED")#, width=20, height=10)
                        self.btn_cpu.place(x=(self.width - 200), y=90)
                    elif False:
                        cvalue = "framerate: " + str(num) + "fps"
                        self.canvas2.itemconfig(self.itext, text=cvalue)
                        self.canvas2.insert(self.itext, 12, '')
                        #self.canvas2.insert(self.itext, 12, cvalue)
                    else:
                        cvalue0 = "[欢迎进入会畅SVC调试系统]" + "\t" + "\t"
                        cvalue1 = "frmrate:" + "\t" + str(num) + "fps" + "|\t"
                        cvalue2 = "recv-brate:" + "\t" + str(num) + "kbps" + "|\t"
                        cvalue3 = "send-brate:" + "\t" + str(num) + "kbps" + "|\t"
                        cvalue4 = "lossrate:" + "\t" + str(num) + "%" + "|\t"
                        cvalue5 = "cpu:" + "\t" + str(num) + "%" + "|\t"
                        cvalue6 = "record-volum:" + "\t" + str(num) + "%" + "|\t"
                        cvalue7 = "play-volum:" + "\t" + str(num) + "%" + "|\t"
                        cvalue = cvalue0 + cvalue1 + cvalue2 + cvalue3 + cvalue4  + cvalue5 + cvalue6 + cvalue7
                        otherFrame.title(cvalue)


                    #self.canvas2.delete("all")
                    self.image1 = tk.PhotoImage(file='icon/home.png').subsample(3, 4)  # zoom(2, 2)
                    handler = lambda: self.onCloseOtherFrame(otherFrame)
                    self.btn_home = tk.Button(otherFrame, text="home", image=self.image1,
                                              command=handler)  # , width=50, height=30)
                    self.btn_home.place(x=0, y=0)

                    ###
                    otherFrame.update()
                    #print('num=%d' % num)
                    otherFrame.after(500)
                    #otherFrame.after(4)
                    #time.sleep(1)
            #otherFrame.mainloop()

    def top_frame(self):
        #离开函数的同时也会销毁局部变量tk_image，所以图片就无法显示。
        self.top = tk.Toplevel()
        self.top.title("test")
        self.top.geometry("1280x720")
        self.imagefile1 = tk.PhotoImage(file='hc.png')
        # label_img = tk.Label(self.top, image=imagefile0)
        # label_img.pack()

        canvas2 = tk.Canvas(self.top, height=720, width=1280)
        image_2 = canvas2.create_image(0, 0, anchor='nw', image=self.imagefile1)
        canvas2.pack(side='top')

        self.image0 = tk.PhotoImage(file='icon/video_start.png').subsample(3, 4)  # zoom(2, 2)
        bt_video_start = tk.Button(self.top, text='启动', image=self.image0, command=self.draw)  # , width=50, height=30)
        bt_video_start.place(x=260, y=660)
        # bt_video_start.pack()

        self.image1 = tk.PhotoImage(file='icon/home.png').subsample(3, 4)  # zoom(2, 2)
        handler = lambda: self.onCloseOtherFrame(self.top)
        btn_home = tk.Button(self.top, text="home", command=handler)  # , width=50, height=30)
        btn_home.place(x=330, y=660)
        #btn_home.pack()

        ##self.top.withdraw()
        return

    def root_frame(self):
        self.root = tk.Tk()
        self.root.title('欢迎进入会畅SVC调试系统')
        self.root.geometry('690x500')
        ##self.frame = tk.Frame(self.root)
        # self.frame = tk.Frame(self.root, width=640, height=480)
        ##self.frame.pack()
        # btn = Tk.Button(self.frame, text="Open Frame", command=self.openFrame)
        # btn.pack()
        if False:
            self.top_frame()
        # 画布放置图片
        canvas = tk.Canvas(self.root, height=400, width=800)
        self.canvas = canvas
        self.winid = canvas.winfo_id()
        print("log_frame: self.winid= ", self.winid)
        imagefile = tk.PhotoImage(file='hc.png')
        self.image = canvas.create_image(0, 0, anchor='nw', image=imagefile)
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
        self.bt_exit = tk.Button(self.root, text='退出', image=image3, command=self.setting, width=50, height=30)
        self.bt_exit.place(x=410, y=400)
        # self.bt_config.pack()

        image4 = tk.PhotoImage(file='icon/config.png').subsample(3, 4)
        self.bt_config = tk.Button(self.root, text='设置', image=image4, command=self.setting, width=50, height=30)
        self.bt_config.place(x=480, y=400)

        self.root.mainloop()

    def restart(self):
        try:
            if ('normal' == self.self.root.state()):
                # if True:
                self.canvas.delete("all")
                self.canvas.destroy()
                del self.canvas
                self.self.root.destroy()
                print("restart: window.destroy")
        finally:
            self.root = tk.Tk()
            self.root.title('欢迎进入会畅SVC调试系统')
            self.root.geometry('1280x720')
            canvas = tk.Canvas(self.root, height=720, width=1280)
            self.canvas = canvas
            self.winid = canvas.winfo_id()
            print("restart: self.winid= ", self.winid)
            imagefile = tk.PhotoImage(file='hc.png')
            image = canvas.create_image(0, 0, anchor='nw', image=imagefile)
            canvas.pack(side='top')

            self.mainframe1 = Frame(self.root, height=250, width=250, bg='white')
            canvas_frame = canvas.create_window((4, 4), window=self.mainframe1, anchor="nw")

            self.winid = self.mainframe1.winfo_id()

            if self.test_meeting == None:
                self.test_meeting = ReadFrame(self.winid)
                self.test_meeting.init()
                self.test_meeting.start()
            self.root.mainloop()

    def create_meeting(self):
        meeting_id = self.var_meeting_id.get()
        usr_name = self.var_usr_name.get()
        print("create_meeting: (meeting_id, usr_name)= ", (meeting_id, usr_name))
        # self.winid = self.image.winfo_id()
        self.winid = self.mainframe1.winfo_id()
        if self.test_meeting == None:
            self.test_meeting = ReadFrame(self.winid)
            self.test_meeting.init()
            self.test_meeting.start()

    def enter_meeting(self):
        meeting_id = self.var_meeting_id.get()
        usr_name = self.var_usr_name.get()
        print("enter_meeting: (meeting_id, usr_name)= ", (meeting_id, usr_name))
        if False:
            self.root.geometry('1280x720')
            self.canvas.delete("all")
            canvas = tk.Canvas(self.root, height=720, width=1280)
            self.canvas = canvas
            self.winid = canvas.winfo_id()
            imagefile = tk.PhotoImage(file='hc.png')
            image = canvas.create_image(0, 0, anchor='nw', image=imagefile)
            canvas.pack(side='top')
        elif True:
            self.openFrame()
        else:
            self.bt_login.destroy()
            del self.bt_login
            self.bt_logup.destroy()
            del self.bt_logup
            self.bt_logquit.destroy()
            del self.bt_logquit

    def watch_meeting(self):
        meeting_id = self.var_meeting_id.get()
        usr_name = self.var_usr_name.get()
        print("watch_meeting: (meeting_id, usr_name)= ", (meeting_id, usr_name))
        self.restart()

    def setting(self):
        meeting_id = self.var_meeting_id.get()
        usr_name = self.var_usr_name.get()
        print("seeting: (meeting_id, usr_name)= ", (meeting_id, usr_name))
        self.top.update()
        self.top.deiconify()


if __name__ == '__main__':
    print('Start pycall.')
    call = Conference()
    call.start()
    call.root_frame()

    ##call = Pyui()
    # call.FrameTest()
    ##call.FrameCallSDL()
    print('End pycall.')

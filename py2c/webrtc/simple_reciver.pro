TEMPLATE = app
CONFIG += console c++11
QT +=widgets network  websockets

DEFINES +=WEBRTC_POSIX
DEFINES +=WEBRTC_LINUX
DEFINES +=LINUX
DEFINES +=__LINUX__
DESTDIR = $$PWD/bin/
LIBS += -lpthread -ldl -lX11 -lboost_system

TARGET = reciverapp


#DEFINES += CMAKE_USE_OPENSSL=ON
#mediasoup使用libmediasoupclient库的方式-----------
#    -lgnustl_shared  -lc++_shared
#INCLUDEPATH +=/home/yang/libmediasoupclient.bak/include
#INCLUDEPATH +=/home/yang/libmediasoupclient.bak/nlohmann/
#INCLUDEPATH +=/home/yang/libmediasoupclient.bak/
#LIBS +=-L/home/yang/libmediasoupclient/build -lmediasoupclient
#INCLUDEPATH +=$$PWD/libsdptransform/include
#LIBS +=-L/home/yang/libmediasoupclient.bak/build/libsdptransform -lsdptransform
#======================================

#DEFINES +=MSC_LOG_TRACE

# -D_GLIBCXX_USE_CXX11_ABI=0
#####webrtc=====
webrtcroot=$$PWD/../webrtc-checkout/src/
INCLUDEPATH += $$webrtcroot
INCLUDEPATH += $$webrtcroot/third_party/abseil-cpp/
INCLUDEPATH += $$webrtcroot/third_party/libyuv/include/
#INCLUDEPATH +=/$$PWD/../webrtc/webrtc
LIBS += -L$$webrtcroot/out/h264_linux/obj -lwebrtc
LIBS += -lcurl
###socketio cpp
INCLUDEPATH +=/home/yang/code/socket.io-client-cpp/src/
LIBS += -L/home/yang/code/socket.io-client-cpp/build -lsioclient
#third_party
#INCLUDEPATH +=/home/yang/webrtc/agora/abseil/
#LIBS += -L/home/yang/webrtc/agora/abseil/lib/ -labsl_all


###old webrtc ===
#INCLUDEPATH +=/home/yang/webrtc/x64/webrtc/
#LIBS += -L/home/yang/webrtc/x64 -lwebrtc_full

message($$(HOME))
##mediasoup
#INCLUDEPATH +=/home/yang/libmediasoupclient/include
#LIBS +=-L/home/yang/libmediasoupclient/build -lmediasoupclient
#QMAKE_CFLAGS+=-fno-rtti
#QMAKE_CXXFLAGS+=-fno-rtti
include($$PWD/libmedia/libmedia.pri)
include($$PWD/libsdptransform/libsdptransform.pri)
include($$PWD/broadcaster-change/broadcaster.pri)
#include($$PWD/peer_client/peer_client.pri)
#include($$PWD/peer_server/peer_server.pri)
#include($$PWD/testClient/testClient.pri)
INCLUDEPATH +=$$PWD/jwt-cpp
LIBS += -L /home/yang/sdk/x264 -lx264
LIBS += -L/home/yang/datashare/jy/py2c -lhcsvc
INCLUDEPATH +=$$PWD/
HEADERS += $$PWD/PeerConnection.hpp\
           $$PWD/Handler.hpp \
           $$PWD/mediasoupclient.hpp \
           $$PWD/Transport.hpp \
           $$PWD/Device.hpp \
           $$PWD/createsessiondescriptionobserver.h \
           mediareciver.h \
           jhttpclient.h\
           simple/mediasimplereciver.h \
           render/video_render.h\
           render/rplaywidget.h
#           $$PWD/audiotalk.h

SOURCES += \
           $$PWD/main_reciever.cpp\
           $$PWD/PeerConnection.cpp\
           $$PWD/Handler.cpp \
            $$PWD/jhttpclient.cpp\
           $$PWD/mediasoupclient.cpp \
           $$PWD/Transport.cpp \
           $$PWD/Device.cpp \
           $$PWD/createsessiondescriptionobserver.cpp \
           mediareciver.cpp \
           simple/mediasimplereciver.cpp \
           render/video_render.cc\
           render/rplaywidget.cpp\
           $$PWD/simple/start_socket_io.cpp
#           $$PWD/audiotalk.cpp

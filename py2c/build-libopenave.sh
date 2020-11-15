#!/bin/sh

#sudo apt-get install alsa-base alsa-utils alsa-source libasound2-dev
#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/glibc-2.27/lib
#pyinstaller -F -w pyui.py

gcc -o libopenave.so -shared -fPIC  -Wformat=0 -Wl,-Bsymbolic -fvisibility=hidden -ldl -Wl,-rpath=. \
-Xlinker --unresolved-symbols=ignore-in-shared-libs \
./codec/decoding_encoding.c \
./codec/scaling_video.c \
./codec/rate_control.c \
./codec/nallayer.c \
./codec/simple_client.c \
./codec/simple_server.c \
./codec/utility.c \
./codec/utility_server.c \
./codec/capture.c \
./codec/render.c \
./codec/audio_capture.c \
./codec/audio_player.c \
./codec/cjson.c \
./codec/audiortp.c \
./codec/simple_osd.c \
-I../ffmpeg-linux/ffmpeg-2.8.7 \
-I../ffmpeg-linux/openh264-1.5.0/include \
-I../cJSON \
-I../openfec_v1.4.2 \
-L../ffmpeg-linux/mylib \
-L/usr/lib \
-L../openfec_v1.4.2/bin/Release \
-L../ffmpeg-linux/openssl-1.0.2d \
-lavformat -lavdevice -lavfilter -lavcodec -lavutil -lpostproc -lswresample -lswscale \
-laacplus -lx264 -lopenh264 -lcommon -lprocessing -lconsole_common \
-lwebrtc \
-lcjson -lopenfec -lpthread -ldl -lSDL2 -lSDL2_ttf -lsndio \
-lxcb-shm -lxcb-xfixes -lxcb-render -lxcb-shape -lxcb -lXau -lXdmcp -lX11 -lXext -lXv \
-lasound -lz -lstdc++ -lm -lbz2 -lva

#-lstdc++ 
#-lavformat -lavdevice  -lavcodec -lavutil  -lavfilter -lpostproc  -lswresample -lswscale \
#-lbz2
#-lavformat  -lavdevice -lavcodec  -lavutil -pthread  -ldl -lswscale -lSDL -lbz2  -lasound -lz -lm

#-lavformat -lavdevice -lavcodec -lavutil -lpthread -lswscale -lswresample
#-lswscale -lswresample -lavdevice -lavutil -lavcodec -lavformat -lavfilter -lpostproc \

#-L../ffmpeg-linux/openssl-1.0.2d \
#-lssl -lcrypto
#g++ -o libhcsvc.so -rdynamic -shared -fPIC -ldl
#gcc -o libhcsvc.so -shared -fPIC -Wl,-rpath=. -ldl \
#./codec/hcsvc.c

gcc -o libave.so -shared -fPIC -Wl,-Bsymbolic -g -fvisibility=hidden -ldl -Wl,--exclude-libs=ALL \
./codec/hcsvc.c

#-L../ffmpeg-linux/openssl-1.0.2d

#-lavformat -lavdevice -lavutil -lavcodec -lswresample -lswscale -lavfilter -lpostproc

#-lavdevice -lavformat -lavutil -lavcodec -lswresample -lswscale -lavfilter

#-Xlinker "-(" -lavdevice -lavformat -lavcodec -lavutil -lswresample -lswscale -lavfilter -lpostproc -Xlinker "-)"
#-Xlinker "-(" -lavdevice -lavformat -lavcodec -lavutil -lswresample -lswscale -lavfilter -Xlinker "-)"

#-lavformat -lavdevice -lavcodec -lavutil -lswresample -lswscale -lavfilter

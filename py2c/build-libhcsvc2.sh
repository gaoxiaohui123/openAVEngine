#!/bin/sh

#sudo apt-get install alsa-base alsa-utils alsa-source libasound2-dev
#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/glibc-2.27/lib

gcc -o libhcsvcapi.so -shared -fPIC -Wl,-Bsymbolic -g -fvisibility=hidden -ldl -Wl,-rpath=. \
./codec/decoding_encoding.c \
./codec/scaling_video.c \
./codec/rate_control.c \
./codec/nallayer.c \
./codec/simple_client.c \
./codec/simple_server.c \
./codec/utility.c \
-I../ffmpeg-linux/ffmpeg-2.8.7 \
-I../cJSON \
-I../openfec_v1.4.2 \
-L../ffmpeg-linux/mylib \
-L/usr/lib \
-L../openfec_v1.4.2/bin/Release \
-L../ffmpeg-linux/openssl-1.0.2d \
-lavformat -lavdevice -lavcodec -lavutil -lswresample -lswscale -lavfilter -lpostproc \
-laacplus -lx264 -lcjson -lopenfec -lpthread -lm -lstdc++ -lasound -lSDL2 -lSDL2_ttf -lssl -lcrypto


#-L../ffmpeg-linux/openssl-1.0.2d \
#-lssl -lcrypto
#g++ -o libhcsvc.so -rdynamic -shared -fPIC -ldl
#gcc -o libhcsvc.so -shared -fPIC -Wl,-rpath=. -ldl \
#./codec/hcsvc.c

gcc -o libhcsvc.so -shared -fPIC -Wl,-Bsymbolic -g -fvisibility=hidden -ldl -Wl,--exclude-libs=ALL \
./codec/hcsvc.c

#-L../ffmpeg-linux/openssl-1.0.2d

#-lavformat -lavdevice -lavutil -lavcodec -lswresample -lswscale -lavfilter -lpostproc

#-lavdevice -lavformat -lavutil -lavcodec -lswresample -lswscale -lavfilter

#-Xlinker "-(" -lavdevice -lavformat -lavcodec -lavutil -lswresample -lswscale -lavfilter -lpostproc -Xlinker "-)"
#-Xlinker "-(" -lavdevice -lavformat -lavcodec -lavutil -lswresample -lswscale -lavfilter -Xlinker "-)"

#-lavformat -lavdevice -lavcodec -lavutil -lswresample -lswscale -lavfilter
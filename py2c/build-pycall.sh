#!/bin/sh

gcc -o libpycall.so -shared -fPIC ./codec/pycall.c

gcc -o libhcsvc.so -shared -fPIC -Wl,-Bsymbolic -ldl \
./codec/decoding_encoding.c \
./codec/scaling_video.c \
./codec/rate_control.c \
./codec/nallayer.c \
./codec/simple_client.c \
./codec/simple_server.c \
-I../ffmpeg-linux/ffmpeg-2.8.7 \
-I../cJSON \
-I../openfec_v1.4.2 \
-L../ffmpeg-linux/mylib \
-L/usr/lib \
-L../openfec_v1.4.2/bin/Release \
-lavformat -lavdevice -lavcodec -lavutil -lswresample -lswscale -lavfilter \
-laacplus -lx264 -lcjson -lopenfec -lpthread -lm -lstdc++


#./codec/fec.c
#./codec/fectest.c
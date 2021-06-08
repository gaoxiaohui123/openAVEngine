#!/bin/sh

#git clone https://github.com/cisco/openh264.git
#git clone http://git.videolan.org/git/x264.git
#git clone git://source.ffmpeg.org/ffmpeg.git

#sudo ln -s /use/bin/yasm /use/bin/nasm

cd ./openh264-1.5.0
#make OS=linux ARCH=x86_64 SHARED= CFLAGS="-fPIC"
#make OS=linux ARCH=x86_64 CFLAGS="-fPIC"
#make SHARED= CFLAGS="-fPIC -fvisibility=hidden"
#make ARCH=x86_64 CFLAGS="-fPIC -fvisibility=hidden"
make ARCH=x86_32 CFLAGS="-fPIC -fvisibility=hidden"

#ldd libopenh264.so.6
#根目录下openh264.def添加自定义函数
#wels_enc_export.def

#!/bin/sh
#ffmpeg-2.8.7        发布日期：2016-02-29
#for linux
#sudo apt-get install automake faac fdk-aac lame libass libtool libvorbis libvpx libvo-aacenc opencore-amr openjpeg opus sdl schroedinger shtool speex texi2html theora wget x264 x265 xvid yasm

#zlib: CFLAGS="-fPIC" ./configure --static
#openssl: ./config -fPIC shared  ./Configure darwin64-x86_64-cc # make  # make test # make install
# cd rtmpdump-2.4 make CFLAGS="-fPIC -I/usr/local/include" #make install
#make SHARED= CFLAGS="-fPIC -DRTMPDUMP_VERSION=2.4 -DWO_CRYPTO -Wall -I/usr/local/ssl/include" LDFLAGS="-ldl -L/usr/local/ssl/lib" LIBZ="-lz -static"
#make SHARED= CRYPTO= XDEF=-DNO_SSL CFLAGS="-fPIC -DRTMPDUMP_VERSION=2.4 -I/home/gxh/works/huichang/svc_codec/bizconf_svc_codec/ffmpeg-linux/openssl-1.0.2d/include"

#mac下将文件/rtmpdump-2.4/librtmp/makefile中的-soname 改为 -dylib_install_name；
#libaacplus:    sed -i '.bck' -e 's/libtool/glibtool/' autogen.sh
#./autogen.sh #make && make install
#./autogen.sh --enable-static --disable-shared
#libaacplus: CFLAGS="-fPIC" ./configure --enable-static --disable-shared
#au_channel.h #define inline
#x264: CFLAGS="-fPIC -fvisibility=hidden" ./configure --enable-pic --enable-static --disable-cli --disable-opencl
#wget http://www.tortall.net/projects/yasm/releases/yasm-1.3.0.tar.gz
#tar xzvf yasm-1.3.0.tar.gz
#关于添加libopenh264：
#需要增加“-lstdc++ -lm”，否则configure会报welsEncoderExt.cpp:(.text+0x1a0): undefined reference to `operator delete(void*, unsigned long)'之类的错误
#因为libopenh264含cpp文件并由g++编译生成
#FFMPEG_ROOT=`pwd`/"thin"
FFMPEG_ROOT=`pwd`
export LIBVA_DRIVER_NAME=iHD
export MFX_HOME=/opt/intel/mediasdk
export PKG_CONFIG_PATH=$MFX_HOME/lib/pkgconfig/
#export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig #for windows
#export LIBVA_DRIVERS_PATH=/opt/intel/mediasdk/lib:/usr/lib/x86_64-linux-gnu/dri:/usr/local/lib
export LIBVA_DRIVERS_PATH=/opt/intel/mediasdk/lib:/usr/local/lib/dri:/usr/local/lib

export LD_LIBRARY_PATH=$LIBVA_DRIVERS_PATH
export QSVINC=/opt/intel/mediasdk/include
echo $PKG_CONFIG_PATH
echo $LD_LIBRARY_PATH

cd ffmpeg-4.3.2
CFLAGS="-fPIC" ./configure --enable-pic --enable-postproc \
--prefix=/usr/local \
--target-os=linux \
--disable-doc \
--disable-debug \
--enable-vaapi --enable-libmfx \
--enable-encoder=h264_qsv \
--enable-decoder=h264_qsv \
--enable-encoder=hevc_qsv \
--enable-decoder=hevc_qsv \
--enable-hwaccel=h264_vaapi \
--enable-encoder=libaacplus \
--enable-gpl --enable-nonfree --enable-librtmp \
--enable-static \
--enable-pthreads --enable-zlib \
--enable-encoder=libx264 --enable-libx264 \
--enable-encoder=libopenh264 --enable-libopenh264 --enable-muxer=h264 \
--extra-cflags="-fpic -I$FFMPEG_ROOT/rtmpdump-2.4 \
-I$FFMPEG_ROOT/libaacplus-2.0.2/include \
-I$FFMPEG_ROOT/x264 -I$FFMPEG_ROOT/openh264-1.5.0/include \
-I$FFMPEG_ROOT/zlib-1.2.8 -I$FFMPEG_ROOT/openssl-1.0.2d/include -I/usr/local/include \
-I$QSVINC " \
--extra-ldflags="-L/opt/intel/mediasdk/lib \
-L/usr/local/lib/dri \
-L$FFMPEG_ROOT/mylib \
-L/usr/local/lib" \
--extra-libs="-lmfx -laacplus -lx264 -lopenh264 -lcommon -lprocessing -lconsole_common -lz -ldl -lrtmp -lpthread -lstdc++ -lm"

cd ../

#-L/usr/lib/x86_64-linux-gnu/dri \
#--enable-libaacplus

#-lopenh264
#-lencoder -ldecoder -lopenh264 -lprocessing -lconsole_common -lcommon 
#--enable-memalign-hack
#-lasound 
#--enable-libpulse

#-L$FFMPEG_ROOT/openssl-1.0.2d 
#-lssl -lcrypto 
#--enable-openssl 
#-fvisibility=hidden
#--enable-shared
#make
#mkdir lib
#cp */*.so lib
#darwin-i386-cc(darwin64-x86_64-cc)
#-L/usr/local/lib
#install_name switch instead of soname
#注意：在mac下rtmpdump-2.4只能成功编译静态库，无法正确编译动态库库和可执行文件；
#在mac下openssl编译出的静态库安装到/usr/local/lib下会引起其他对openssl有依赖的项的错误
#在mac下该ffmpeg编译错误，请使用../works/tools/下的ffmpeg

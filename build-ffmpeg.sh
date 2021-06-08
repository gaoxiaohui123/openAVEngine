#!/bin/sh

#for linux
#sudo apt-get install automake faac fdk-aac lame libass libtool libvorbis libvpx libvo-aacenc opencore-amr openjpeg opus sdl schroedinger shtool speex texi2html theora wget x264 x265 xvid yasm

#zlib: CFLAGS="-fPIC" ./configure --static
#openssl: ./config -fPIC  ./Configure darwin64-x86_64-cc # make  # make test # make install
# cd rtmpdump-2.4 make CFLAGS="-fPIC -I/usr/local/include" #make install
#make SHARED= CFLAGS="-fPIC -DRTMPDUMP_VERSION=2.4 -DWO_CRYPTO -Wall -I/usr/local/ssl/include" LDFLAGS="-ldl -L/usr/local/ssl/lib" LIBZ="-lz -static"

#mac下将文件/rtmpdump-2.4/librtmp/makefile中的-soname 改为 -dylib_install_name；
#libaacplus:    sed -i '.bck' -e 's/libtool/glibtool/' autogen.sh
#./autogen.sh #make && make install
#./autogen.sh --enable-static --disable-shared
#libaacplus: CFLAGS="-fPIC" ./configure --enable-static --disable-shared
#au_channel.h #define inline
#x264: CFLAGS="-fPIC -fvisibility=hidden" ./configure --enable-pic --enable-static --disable-cli --disable-opencl
#wget http://www.tortall.net/projects/yasm/releases/yasm-1.3.0.tar.gz
#tar xzvf yasm-1.3.0.tar.gz

#FFMPEG_ROOT=`pwd`/"thin"
FFMPEG_ROOT=`pwd`

cd ffmpeg-2.8.7
CFLAGS="-fPIC -fvisibility=hidden" ./configure --enable-pic --enable-postproc \
--enable-openssl --prefix=/usr/local \
--target-os=linux \
--disable-doc \
--enable-encoder=libaacplus \
--enable-gpl --enable-nonfree --enable-libaacplus --enable-librtmp \
--enable-static \
--enable-pthreads --enable-zlib \
--enable-encoder=libx264 --enable-libx264 \
--extra-cflags="-fpic -fvisibility=hidden -I$FFMPEG_ROOT/rtmpdump-2.4 -I$FFMPEG_ROOT/libaacplus-2.0.2/include -I$FFMPEG_ROOT/x264 -I$FFMPEG_ROOT/zlib-1.2.8 -I$FFMPEG_ROOT/openssl-1.0.2d/include -I/usr/local/include" \
--extra-ldflags="-L$FFMPEG_ROOT/rtmpdump-2.4/librtmp -L$FFMPEG_ROOT/libaacplus-2.0.2/src/.libs -L$FFMPEG_ROOT/x264 -L$FFMPEG_ROOT/zlib-1.2.8 -L$FFMPEG_ROOT/openssl-1.0.2d -L$FFMPEG_ROOT/zlib-1.2.8 -L/usr/local/lib" \
--extra-libs="-laacplus -lx264 -lz -ldl -lssl -lcrypto -lrtmp"
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

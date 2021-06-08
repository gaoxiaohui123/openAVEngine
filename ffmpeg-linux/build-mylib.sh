#!/bin/sh

#git clone https://github.com/cisco/openh264.git
#git clone http://git.videolan.org/git/x264.git
#git clone git://source.ffmpeg.org/ffmpeg.git

#sudo ln -s /use/bin/yasm /use/bin/nasm
ROOT_PATH=`pwd`
FFMPEG_ROOT=`pwd`

echo 'input a number'
read Num
###############################
case $Num in
1)
  cd ./openh264-1.5.0
  make clean
  #make OS=linux ARCH=x86_64 SHARED= CFLAGS="-fPIC"
  #make OS=linux ARCH=x86_64 CFLAGS="-fPIC"
  #make SHARED= CFLAGS="-fPIC -fvisibility=hidden"
  make ARCH=x86_64 CFLAGS="-fPIC -fvisibility=hidden"
  #make ARCH=x86_32 CFLAGS="-fPIC -fvisibility=hidden"

  cp *.a ../mylib/
  make clean
  #ldd libopenh264.so.6
  #根目录下openh264.def添加自定义函数
  #wels_enc_export.def
  cd ../
  ################################
;;
2)
  cd ./x264
  CFLAGS="-fPIC -fvisibility=hidden" ./configure --enable-pic --enable-static --disable-cli --disable-opencl
  make clean
  make -j 4
  cp *.a ../mylib/
  make clean
  cd ../
  ###############################
;;
3)
  cd ./libaacplus-2.0.2
  make clean
  ./autogen.sh --enable-static --disable-shared
  CFLAGS="-fPIC" ./configure --enable-static --disable-shared
  make
  cp src/.libs/*.a ../mylib/
  make clean
  cd ../
;;
4)
  cd ./rtmpdump-2.4
  make SHARED= CRYPTO= XDEF=-DNO_SSL CFLAGS="-fPIC -DRTMPDUMP_VERSION=2.4 -I$ROOT_PATH/openssl-1.0.2d/include"
  cp ./librtmp/*.a ../mylib/
  make clean
  cd ../
;;
5)
  export LIBVA_DRIVER_NAME=iHD
  export MFX_HOME=/opt/intel/mediasdk
  export PKG_CONFIG_PATH=$MFX_HOME/lib/pkgconfig/
  #export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig #for windows
  export LIBVA_DRIVERS_PATH=/opt/intel/mediasdk/lib:/usr/lib/x86_64-linux-gnu/dri:/usr/local/lib #for uos
  #export LIBVA_DRIVERS_PATH=/opt/intel/mediasdk/lib:/usr/local/lib/dri:/usr/local/lib #for linux

  export LD_LIBRARY_PATH=$LIBVA_DRIVERS_PATH
  export QSVINC=/opt/intel/mediasdk/include
  echo $PKG_CONFIG_PATH
  echo $LD_LIBRARY_PATH
  #cd ffmpeg-2.8.7
  cd ffmpeg-4.3.2
  #cd ffmpeg-0
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
  --enable-gpl --enable-nonfree --enable-libaacplus --enable-librtmp \
  --enable-static \
  --enable-pthreads --enable-zlib \
  --enable-encoder=libx264 --enable-libx264 \
  --enable-encoder=libopenh264 --enable-libopenh264 --enable-muxer=h264 \
  --extra-cflags="-fpic -I$QSVINC -I$FFMPEG_ROOT/rtmpdump-2.4 -I$FFMPEG_ROOT/libaacplus-2.0.2/include -I$FFMPEG_ROOT/x264 -I$FFMPEG_ROOT/openh264-1.5.0/include -I$FFMPEG_ROOT/zlib-1.2.8 -I$FFMPEG_ROOT/openssl-1.0.2d/include -I/usr/local/include" \
  --extra-ldflags="-L/opt/intel/mediasdk/lib -L$FFMPEG_ROOT/mylib -L/usr/local/lib" \
  --extra-libs="-lmfx -laacplus -lx264 -lopenh264 -lcommon -lprocessing -lconsole_common -lz -ldl -lrtmp -lpthread -lstdc++ -lm"
  make clean
  make -j 4
  cp */*.a ../mylib/
  make clean
  cd ../
;;
6)
  cd ../cJSON
  mkdir build
  cd build
  make clean
  #cmake .. -DENABLE_CJSON_UTILS=Off -DENABLE_CJSON_TEST=On -DCMAKE_INSTALL_PREFIX=/usr #（生成bin+lib）
  cmake .. -DENABLE_CJSON_UTILS=Off -DENABLE_CJSON_TEST=On -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_SHARED_LIBS=Off #（生成bin）
  make
  cp *.a ../../ffmpeg-linux/mylib/
  make clean
  cd ../../ffmpeg-linux/
;;
7)
  cd ../openfec_v1.4.2
  rm -r build
  mkdir build
  cd build
  cmake .. -DDEBUG:STRING=OFF
  make
  cp ../bin/Release/*.a ../../ffmpeg-linux/mylib/
  make clean
  cd ../../ffmpeg-linux/
;;

esac


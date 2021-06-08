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
  #make ARCH=x86_64 CFLAGS="-fPIC -fvisibility=hidden"
  make ARCH=x86_32 CFLAGS="-fPIC -fvisibility=hidden"

  rm *.dll.a
  cp *.a ../mylib-win/
  make clean
  #ldd libopenh264.so.6
  #根目录下openh264.def添加自定义函数
  #wels_enc_export.def
  cd ../
  ################################
;;
2)
  cd ./x264
  CFLAGS="-fPIC -fvisibility=hidden" ./configure --enable-pic --enable-static \
  --disable-cli --disable-opencl --host=mingw32
  make clean
  make -j4
  cp *.a ../mylib-win/
  make clean
  cd ../
  ###############################
;;
3)
  cd ./libaacplus-2.0.2-win
  #make clean
  ./autogen.sh --enable-static --disable-shared
  CFLAGS="-fPIC" ./configure --enable-static --disable-shared
  make
  cp src/.libs/*.a ../mylib-win/
  make clean
  cd ../
;;
4)
  cd ./rtmpdump-2.4
  make SHARED= CRYPTO= XDEF=-DNO_SSL CFLAGS="-fPIC -DRTMPDUMP_VERSION=2.4 -I$ROOT_PATH/openssl-1.0.2d/include"
  cp ./librtmp/*.a ../mylib-win/
  make clean
  cd ../
;;
5)
  #windows 下必须指定--arch=x86_32
  export THIRD_PARTY_PATH=/mingw32/i686-w64-mingw32
  export PKG_CONFIG_PATH=$THIRD_PARTY_PATH/mingw32/lib/pkgconfig
  export QSVINC=$THIRD_PARTY_PATH/mingw32/include
  #cd ffmpeg-2.8.7
  cd ffmpeg-4.3.2
  CFLAGS="-fPIC" ./configure --enable-pic --enable-postproc \
  --prefix=/usr/local \
  --target-os=mingw32 \
  --arch=x86_32 \
  --disable-doc \
  --disable-debug \
  --enable-libmfx \
  --enable-encoder=h264_qsv \
  --enable-decoder=h264_qsv \
  --enable-encoder=hevc_qsv \
  --enable-decoder=hevc_qsv \
  --enable-encoder=libaacplus \
  --enable-gpl --enable-nonfree --enable-libaacplus --enable-librtmp \
  --enable-static \
  --enable-encoder=libx264 --enable-libx264 \
  --enable-encoder=libopenh264 --enable-libopenh264 --enable-muxer=h264 \
  --extra-cflags="-fpic -I$FFMPEG_ROOT/rtmpdump-2.4 -I$FFMPEG_ROOT/libaacplus-2.0.2/include -I$FFMPEG_ROOT/x264 -I$FFMPEG_ROOT/openh264-1.5.0/include -I$FFMPEG_ROOT/zlib-1.2.8 -I$FFMPEG_ROOT/openssl-1.0.2d/include -I/usr/local/include" \
  --extra-ldflags="-L$FFMPEG_ROOT/mylib-win -L$THIRD_PARTY_PATH/mingw32/lib" \
  --extra-libs="-lmfx -laacplus -lx264 -lopenh264 -lcommon -lprocessing -lconsole_common -lz -lrtmp -lstdc++ -lm"
  #make clean
  #make -j4
  #cp */*.a ../mylib-win/
  #make clean
  cd ../
;;
6)
 export THIRD_PARTY_PATH=/mingw32/i686-w64-mingw32
 export PKG_CONFIG_PATH=$THIRD_PARTY_PATH/mingw32/lib/pkgconfig
 export QSVINC=$THIRD_PARTY_PATH/mingw32/include

 #cp $THIRD_PARTY_PATH/lib/*pthread* /mingw32/lib/gcc/i686-w64-mingw32/10.2.0/
 #cp $THIRD_PARTY_PATH//include/*pthread* /mingw32/lib/gcc/i686-w64-mingw32/10.2.0/include/

 cd ffmpeg
 CFLAGS="-fPIC" ./configure \
 --enable-gpl --enable-nonfree \
 --enable-libmfx \
 --enable-encoder=h264_qsv \
 --enable-decoder=h264_qsv \
 --enable-encoder=hevc_qsv \
 --enable-decoder=hevc_qsv \
 --enable-libaacplus --enable-librtmp \
 --enable-encoder=libx264 --enable-libx264 \
 --extra-cflags="-fpic -I/usr/include -I$THIRD_PARTY_PATH/include -I$FFMPEG_ROOT/rtmpdump-2.4 -I$FFMPEG_ROOT/libaacplus-2.0.2/include -I$FFMPEG_ROOT/x264 -I$FFMPEG_ROOT/openh264-1.5.0/include -I$FFMPEG_ROOT/zlib-1.2.8 -I$FFMPEG_ROOT/openssl-1.0.2d/include" \
 --extra-ldflags="-L$FFMPEG_ROOT/mylib-win -L$THIRD_PARTY_PATH/mingw32/lib -L$THIRD_PARTY_PATH/lib -L/usr/lib" \
 --extra-libs="-laacplus -lx264 -lopenh264 -lcommon -lprocessing -lconsole_common -lrtmp -lmfx -lstdc++ -lm -lpthread"

 # -L$THIRD_PARTY_PATH/mingw32/lib  -L$THIRD_PARTY_PATH/lib -L/mingw32/i686-w64-mingw32/lib
 # -I/usr/local/include -I$THIRD_PARTY_PATH/include

 cd ../
;;
7)
 export THIRD_PARTY_PATH=/mingw32/i686-w64-mingw32
 export PKG_CONFIG_PATH=$THIRD_PARTY_PATH/mingw32/lib/pkgconfig
 export QSVINC=$THIRD_PARTY_PATH/mingw32/include

 #cp $THIRD_PARTY_PATH/lib/*pthread* /mingw32/lib/gcc/i686-w64-mingw32/10.2.0/
 #cp $THIRD_PARTY_PATH//include/*pthread* /mingw32/lib/gcc/i686-w64-mingw32/10.2.0/include/

 #cp $THIRD_PARTY_PATH/lib/*pthread* /mingw32/lib/
 #cp $THIRD_PARTY_PATH/include/*pthread* /mingw32/include/

 cd ffmpeg
 CFLAGS="-fPIC" ./configure \
 --enable-gpl --enable-nonfree \
 --enable-libmfx \
 --enable-encoder=h264_qsv \
 --enable-decoder=h264_qsv \
 --enable-encoder=hevc_qsv \
 --enable-decoder=hevc_qsv \
 --extra-ldflags="-L$FFMPEG_ROOT/mylib-win -L$THIRD_PARTY_PATH/mingw32/lib -L$THIRD_PARTY_PATH/lib -L/usr/lib" \
 --extra-libs="-lmfx"
 cd ../
;;
8)
  echo 'number is 6'
  cd ../cJSON
  mkdir build
  cd build
  #make clean
  #cmake .. -DENABLE_CJSON_UTILS=Off -DENABLE_CJSON_TEST=On -DCMAKE_INSTALL_PREFIX=/usr #（生成bin+lib）
  cmake .. -DENABLE_CJSON_UTILS=Off -DENABLE_CJSON_TEST=On -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_SHARED_LIBS=Off #（生成bin）
  make
  cp *.a ../../ffmpeg-linux/mylib-win/
  make clean
  cd ../../ffmpeg-linux/
;;
9)
  cd ../openfec_v1.4.2
  rm -r build
  mkdir build
  cd build
  cmake .. -DDEBUG:STRING=OFF
  make
  cp ../bin/Release/*.a ../../ffmpeg-linux/mylib-win/
  make clean
  cd ../../ffmpeg-linux/
;;

esac


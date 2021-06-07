#!/bin/sh

echo 'input a number'
read Num
###############################
case $Num in
1)
  export LIBVA_DRIVER_NAME=iHD
  export MFX_HOME=/opt/intel/mediasdk
  export PKG_CONFIG_PATH=$MFX_HOME/lib/pkgconfig/
  #use for uos
  #export LIBVA_DRIVERS_PATH=/opt/intel/mediasdk/lib:/usr/lib/x86_64-linux-gnu/dri:/usr/local/lib
  export LIBVA_DRIVERS_PATH=`pwd`/pycodec/dist
  #use for linux
  #export LIBVA_DRIVERS_PATH=/opt/intel/mediasdk/lib:/usr/local/lib/dri:/usr/local/lib

  export LD_LIBRARY_PATH=$LIBVA_DRIVERS_PATH
  export QSVINC=/opt/intel/mediasdk/include
  echo $PKG_CONFIG_PATH
  echo $LD_LIBRARY_PATH
  #sudo apt-get install alsa-base alsa-utils alsa-source libasound2-dev
  #export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/glibc-2.27/lib
  #pyinstaller -F -w pyui.py --add-binary ./libhcsvcapi.so:. --add-data ./icon/*.png:.
  #pyinstaller -F -w pyui.py
  gcc -o ./pycodec/libhcsvcapi.so -shared -fPIC  -Wformat=0 -Wl,-Bsymbolic -fvisibility=hidden -ldl -Wl,-rpath=. \
  -Xlinker --unresolved-symbols=ignore-in-shared-libs \
  -Wno-deprecated-declarations \
  -Wno-incompatible-pointer-types \
  -Wno-implicit-function-declaration \
  -Wno-int-conversion \
  -Wno-pointer-to-int-cast \
  -Wno-discarded-qualifiers \
  ./codec/decoding_encoding.c \
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
  ./codec/resample.c \
  ./codec/scale.c \
  ./codec/avstream.c \
  ./codec/udpbase.c \
  ./codec/udpbase_tasks.c \
  ./codec/udpbase_video.c \
  ./codec/udpbase_capture.c \
  ./codec/udpbase_render.c \
  ./codec/udpbase_netack.c \
  ./codec/udpbase_audio.c \
  ./codec/udpbase_player.c \
  ./codec/udpbase_audio_capture.c \
  ./codec/udpbase_server.c \
  ./codec/udpbase_mcu.c \
  ./codec/udpbase_stream.c \
  ./codec/hw_decode.c \
  ./codec/qsvdec.c \
  ./codec/vaapi_encode.c \
  ./codec/vaapi_transcode.c \
  -I../ffmpeg-linux/ffmpeg-4.3.2 \
  -I../ffmpeg-linux/openh264-1.5.0/include \
  -I../cJSON \
  -I../openfec_v1.4.2 \
  -I$QSVINC \
  -L../ffmpeg-linux/mylib \
  -L/usr/local/lib \
  -L/usr/lib \
  -lavformat -lavdevice -lavfilter -lavcodec -lavutil -lpostproc -lswresample -lswscale \
  -laacplus -lx264 -lwz264 -lopenh264 -lcommon -lprocessing -lconsole_common \
  -lwebrtc \
  -lmfx \
  -lcjson -lopenfec -lpthread -ldl -lSDL2 -lSDL2_ttf -lsndio \
  -lxcb-shm -lxcb-xfixes -lxcb-render -lxcb-shape -lxcb -lXau -lXdmcp -lX11 -lXext -lXv \
  -lasound -lz -lstdc++ -lm -lbz2 -lva -lva-drm -lva-x11 -lvdpau -llzma

  #-L/usr/lib/x86_64-linux-gnu/dri \

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
;;
2)
  #build libcjson.a
  cd ../cJSON
  mkdir build
  cd build
  cmake .. -DENABLE_CJSON_UTILS=Off -DENABLE_CJSON_TEST=On -DCMAKE_INSTALL_PREFIX=/usr #（生成bin+lib）
  #cmake .. -DENABLE_CJSON_UTILS=Off -DENABLE_CJSON_TEST=On -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_SHARED_LIBS=Off （生成bin）
  make
  #sudo make install
  cp *.a ../../ffmpeg-linux/mylib/
  make clean
  cd ../../py2c
;;
3)
  #build libopenfec.a
  #src/CMakeLists.txt
  #add_library(openfec  STATIC  ${openfec_sources})
  #./CMakeLists.txt
  #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  cd ../openfec_v1.4.2
  mkdir build
  cmake .. -DDEBUG:STRING=OFF
  make
  cp ./bin/Release/*.a ../ffmpeg-linux/mylib/
  make clean
  cd ../py2c
;;
4)

;;
5)
  gcc -o libhcsvc.so -shared -fPIC -Wl,-Bsymbolic -g -fvisibility=hidden -ldl -Wl,--exclude-libs=ALL \
  ./codec/hcsvc.c

  #-L../ffmpeg-linux/openssl-1.0.2d

  #-lavformat -lavdevice -lavutil -lavcodec -lswresample -lswscale -lavfilter -lpostproc

  #-lavdevice -lavformat -lavutil -lavcodec -lswresample -lswscale -lavfilter

  #-Xlinker "-(" -lavdevice -lavformat -lavcodec -lavutil -lswresample -lswscale -lavfilter -lpostproc -Xlinker "-)"
  #-Xlinker "-(" -lavdevice -lavformat -lavcodec -lavutil -lswresample -lswscale -lavfilter -Xlinker "-)"

  #-lavformat -lavdevice -lavcodec -lavutil -lswresample -lswscale -lavfilter
;;

esac
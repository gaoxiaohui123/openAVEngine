#!/bin/sh

#export PATH=$PATH:/usr/include
#sudo apt-get install alsa-base alsa-utils alsa-source libasound2-dev
#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/glibc-2.27/lib
#pyinstaller -F -w pyui.py --add-binary ./libhcsvcapi.so:. --add-data ./icon/*.png:.
#pyinstaller -F -w pyui.py

export THIRD_PARTY_PATH=/mingw32/i686-w64-mingw32
export QSVINC=$THIRD_PARTY_PATH/mingw32/include

echo 'input a number'
read Num
case $Num in
1)
	gcc -o ./pycodec/libhcsvcapi.dll -shared -fPIC \
	-Wl,--output-def,./pycodec/libhcsvcapi.def,--out-implib,./pycodec/libhcsvcapi.lib \
	-Wno-int-to-pointer-cast -Wno-pointer-to-int-cast \
	-Wformat=0 -Wl,-Bsymbolic -fvisibility=hidden -ldl -Wl,-rpath=. \
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
	-I/D/msys64/mingw32/include \
	-I/D/msys64/mingw32/i686-w64-mingw32/include \
	-I/usr/local/include \
	-I../ffmpeg-linux/ffmpeg-4.3.2 \
	-I../ffmpeg-linux/openh264-1.5.0/include \
	-I../cJSON \
	-I../openfec_v1.4.2 \
	-I$QSVINC \
	-L../ffmpeg-linux/mylib-win \
	-L/D/msys64/mingw32/lib \
	-L/D/msys64/mingw32/i686-w64-mingw32/lib \
	-L$THIRD_PARTY_PATH/mingw32/lib \
	-L/usr/lib \
	-L/usr/local/lib \
	-lavformat -lavdevice -lavfilter -lavcodec -lavutil -lpostproc -lswresample -lswscale \
	-laacplus -lx264 -lwz264 -lopenh264 -lcommon -lprocessing -lconsole_common \
	-lwebrtc -lwebrtc_base \
	-lmfx \
	-lcjson -lopenfec \
	-lz -lstdc++ -lm \
	-lpthread -lSDL2 -lws2_32 -ldl -llzma -lbz2 \
	-limagehlp -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 \
	-loleaut32 -lshell32 -lversion -luuid -lOpenGL32 \
	-lwinspool -lcomctl32 -lcomdlg32 -lctl3d32 -ladvapi32 -lwsock32 -lkernel32 -luser32 \
	-lrt -lstrmiids -lquartz -liconv -lvfw32 -lshlwapi \
	-lSecur32 -lBcrypt -lMfplat

# -I/D/msys64/usr/include
#-lbufferoverflow

#-lwebrtc
#-fno-stack-protector #-lssp	
#-lOpenGL32
#-Wl,--no-as-needed 
#-lpthread -ldl -lSDL2 -lSDL2_ttf -lsndio \
#-lxcb-shm -lxcb-xfixes -lxcb-render -lxcb-shape -lxcb -lXau -lXdmcp -lX11 -lXext -lXv \
#-lasound -lz -lstdc++ -lm -lbz2 -lva

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
	gcc -o libhcsvc.dll -shared -fPIC -Wl,-Bsymbolic -g -fvisibility=hidden -Wl,--exclude-libs=ALL \
	./codec/hcsvc.c \
	-Wl,--no-as-needed -ldl 
;;
3)
    gcc -o ./pycodec/libpycall.dll -shared -fPIC ./codec/pycall.c \
    -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast \
	-Wformat=0 -Wl,-Bsymbolic -fvisibility=hidden -ldl -Wl,-rpath=. \
	-Xlinker --unresolved-symbols=ignore-in-shared-libs \
	./codec/cjson.c \
	./codec/simple_osd.c \
	./codec/utility.c \
	./codec/utility_server.c \
	./codec/simple_client.c \
	./codec/simple_server.c \
	./codec/rate_control.c \
	./codec/nallayer.c \
	./codec/scaling_video.c \
	./codec/audiortp.c \
	./codec/decoding_encoding.c \
    ./codec/render.c \
    ./codec/audio_player.c \
    ./codec/capture.c \
    ./codec/audio_capture.c \
	-I/D/msys64/mingw32/include \
	-I/D/msys64/mingw32/i686-w64-mingw32/include \
	-I../ffmpeg-linux/ffmpeg-2.8.7 \
	-I../ffmpeg-linux/openh264-1.5.0/include \
	-I../cJSON \
	-I../openfec_v1.4.2 \
	-L../ffmpeg-linux/mylib-win \
	-L/D/msys64/mingw32/lib \
	-L/D/msys64/mingw32/i686-w64-mingw32/lib \
	-L/usr/lib \
	-lavdevice -lavfilter -lavformat -lavcodec -lavutil -lpostproc -lswresample -lswscale \
	-laacplus -lx264 -lopenh264 -lcommon -lprocessing -lconsole_common \
	-lcjson -lopenfec \
    -lz -lstdc++ -lm \
	-lpthread -lSDL2 -lws2_32 -ldl -llzma -lbz2 \
	-limagehlp -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 \
	-loleaut32 -lshell32 -lversion -luuid -lOpenGL32 \
	-lwinspool -lcomctl32 -lcomdlg32 -lctl3d32 -ladvapi32 -lwsock32 -lkernel32 -luser32 \
	-lrt -lstrmiids -lquartz -liconv -lvfw32 -lshlwapi \
	-lBcrypt

	#-ldxva2 -ld3d9 -ldwmapi -lcomsuppw -ld3d11 -ldxgi

	#-lDnsapi.-lSecur32 -lCrypt32
;;
4)
    gcc -o ./pycodec/libpycall.dll -shared -fPIC ./codec/pycall.c
;;
5)
    echo "create exe"
    gcc -o ./pycodec/maincall.exe -fPIC ./codec/maincall.c \
    -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast \
	-Wformat=0 -Wl,-Bsymbolic -fvisibility=hidden -ldl -Wl,-rpath=. \
	-Xlinker --unresolved-symbols=ignore-in-shared-libs \
	./codec/cjson.c \
	./codec/simple_osd.c \
	./codec/utility.c \
	./codec/utility_server.c \
	./codec/simple_client.c \
	./codec/simple_server.c \
	./codec/rate_control.c \
	./codec/nallayer.c \
	./codec/scaling_video.c \
	./codec/audiortp.c \
	 ./codec/decoding_encoding.c \
	 ./codec/render.c \
    ./codec/audio_player.c \
    ./codec/capture.c \
    ./codec/audio_capture.c \
	-I/D/msys64/mingw32/include \
	-I/D/msys64/mingw32/i686-w64-mingw32/include \
	-I../ffmpeg-linux/ffmpeg-2.8.7 \
	-I../ffmpeg-linux/openh264-1.5.0/include \
	-I../cJSON \
	-I../openfec_v1.4.2 \
	-L../ffmpeg-linux/mylib-win \
	-L/D/msys64/mingw32/lib \
	-L/D/msys64/mingw32/i686-w64-mingw32/lib \
	-L/usr/lib \
	-lavformat -lavdevice -lavfilter -lavcodec -lpostproc -lswresample -lswscale -lavutil \
	-laacplus -lx264 -lopenh264 -lcommon -lprocessing -lconsole_common \
	-lcjson -lopenfec \
    -lz -lstdc++ -lm \
	-lpthread -lSDL2 -lws2_32 -ldl -llzma -lbz2 \
	-limagehlp -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 \
	-loleaut32 -lshell32 -lversion -luuid -lOpenGL32 \
	-lwinspool -lcomctl32 -lcomdlg32 -lctl3d32 -ladvapi32 -lwsock32 -lkernel32 -luser32 \
	-lrt -lstrmiids -lquartz -liconv -lvfw32 -lshlwapi

	echo "create exe over"
;;
6)
    gcc -o ./pycodec/libhcsvcapi.lib -fPIC \
	-Wno-int-to-pointer-cast -Wno-pointer-to-int-cast \
	-Wformat=0 -Wl,-Bsymbolic -fvisibility=hidden -ldl -Wl,-rpath=. \
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
	-I/D/msys64/mingw32/include \
	-I/D/msys64/mingw32/i686-w64-mingw32/include \
	-I../ffmpeg-linux/ffmpeg-2.8.7 \
	-I../ffmpeg-linux/openh264-1.5.0/include \
	-I../cJSON \
	-I../openfec_v1.4.2 \
	-L../ffmpeg-linux/mylib-win \
	-L/D/msys64/mingw32/lib \
	-L/D/msys64/mingw32/i686-w64-mingw32/lib \
	-L/usr/lib \
	-lavformat -lavdevice -lavfilter -lavcodec -lavutil -lpostproc -lswresample -lswscale \
	-laacplus -lx264 -lopenh264 -lcommon -lprocessing -lconsole_common \
	-lcjson -lopenfec \
	-lwebrtc -lwebrtc_base \
	-lz -lstdc++ -lm \
	-lpthread -lSDL2 -lws2_32 -ldl -llzma -lbz2 \
	-limagehlp -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 \
	-loleaut32 -lshell32 -lversion -luuid -lOpenGL32 \
	-lwinspool -lcomctl32 -lcomdlg32 -lctl3d32 -ladvapi32 -lwsock32 -lkernel32 -luser32 \
	-lrt -lstrmiids -lquartz -liconv -lvfw32 -lshlwapi
    #ar -rc libxxx.a xxx.o
;;
esac


#-fgnu89-inline
#-L../ffmpeg-linux/openssl-1.0.2d

#-lavformat -lavdevice -lavutil -lavcodec -lswresample -lswscale -lavfilter -lpostproc

#-lavdevice -lavformat -lavutil -lavcodec -lswresample -lswscale -lavfilter

#-Xlinker "-(" -lavdevice -lavformat -lavcodec -lavutil -lswresample -lswscale -lavfilter -lpostproc -Xlinker "-)"
#-Xlinker "-(" -lavdevice -lavformat -lavcodec -lavutil -lswresample -lswscale -lavfilter -Xlinker "-)"

#-lavformat -lavdevice -lavcodec -lavutil -lswresample -lswscale -lavfilter

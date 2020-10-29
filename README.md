# openAVEngine
audio and video engine
1)the target library is libopenave.so, it depen on libs which locate on ./ffmpeg/mylib;
the libs' source code of mylib locate on ./ffmpeg, for example openh264-1.5.0,x264, 
and so;
2)libopenave.so can run under UOS at present, and whill be run under ubuntu, windows,
macOS, android, ios in future;
3)you can locate to ./py2c/pycodec and run it by 'python pyui.py';
4)please locate to ./ffmpeg and compile libraries,such as x264, openh264 and son;
5)please refer to the files: build-ffmpeg.sh and build-openh264.sh;
6)then you can comile the target library libopenave.so, refer to build-libopenave.sh which
is in ./py2c/;
7)audio and video denoise depend on libwebrtc, the source code will be offered later;



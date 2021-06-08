#!/bin/sh

SRC_DIR=/home/gxh/works/datashare
SRC_DIR1=$SRC_DIR/for_ENC/stream720p

IN_FILE=$SRC_DIR1/FVDO_Freeway_720p.264
IN_FILE2=$SRC_DIR/InToTree_1920x1080.yuv
IN_FILE3=$SRC_DIR/for_ENC/stream1080p/Speed_1080.wmv
WXH=1280x720
WXH2=1920x1080
OUT_FILE=/home/gxh/works/out.yuv
OUT_FILE1=/home/gxh/works/out.265
OUT_FILE2=/home/gxh/works/out2.mp4
OUT_FILE3=/home/gxh/works/out3.mp4
OUT_FILE4=/home/gxh/works/out.264

#export LIBVA_DRIVER_NAME=i965
##export LIBVA_DRIVER_NAME=iHD
#export MFX_HOME=/opt/intel/mediasdk
#export LIBVA_DRIVERS_PATH=/opt/intel/mediasdk/lib:/usr/lib/x86_64-linux-gnu/dri:/usr/local/lib
##export LIBVA_DRIVERS_PATH=/opt/intel/mediasdk/lib:/usr/local/lib/dri:/usr/local/lib
#export LD_LIBRARY_PATH=$LIBVA_DRIVERS_PATH:/opt/intel/mediasdk/share/mfx/samples
#export LD_LIBRARY_PATH=/opt/intel/mediasdk/lib:/usr/lib/x86_64-linux-gnu/dri:/usr/local/lib

#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/../mylib
##export LD_LIBRARY_PATH=$LIBVA_DRIVERS_PATH:`pwd`/../mylib
#linux-vdso
#ls /usr/intel/mediasdk/lib64
#ls /opt/intel/mediasdk/lib
#ls /usr/local/lib

vainfo

##echo $LIBVA_DRIVERS_PATH
##echo $LD_LIBRARY_PATH
#VA-API version 1.11.0

#rm $OUT_FILE
#rm $OUT_FILE1
#rm $OUT_FILE2
#rm $OUT_FILE3
#./ffmpeg -y -hwaccel qsv -c:v h264_qsv -i $IN_FILE -vf hwdownload,format=nv12  $OUT_FILE
##/opt/intel/mediasdk/share/mfx/samples/sample_decode h264 \
##-i /home/gxh/works/datashare/for_ENC/stream720p/FVDO_Freeway_720p.264 -o /home/gxh/works/out.yuv -vaapi
#./ffplay -f rawvideo -video_size 1280x720 /home/gxh/works/out.yuv
#./ffplay -i ../../../datashare/for_ENC/stream720p/FVDO_Freeway_720p.264
#./ffplay -i $IN_FILE3
#ffplay -i raw_out2.yuv -pix_fmt yuv422p -s 1280x720
#/opt/intel/mediasdk/share/mfx/samples/sample_multi_transcode -i::h264 /home/gxh/works/datashare/for_ENC/stream720p/FVDO_Freeway_720p.264 \
#-o::h265 /home/gxh/works/out.h265 -hw -la
#./sample_decode h264 -i sample_outdoor_car_1080p_10fps.h264 [-o /tmp/output.yuv] -vaapi

#Check supported qsv decoder list:
#./ffmpeg -decoders|grep qsv
#Check supported qsv decoder list:
#./ffmpeg -encoders|grep qsv
#ffmpeg -h encoder=h264_qsv
#./ffmpeg -f dshow -list_devices true -i dummy
#./ffmpeg -list_options true -f dshow -i video="罗技高清网络摄像机 C930c"
#./ffplay -showmode 0 -f dshow -i video="Integrated Camera"
#./ffplay -f dshow -video_size 1280x720 -i video="Integrated Camera"
#./ffplay -list_options true -f dshow -video_size 1280x720 -i video="USB  Live camera"
##./ffplay -f dshow -video_size 1280x720 -i video="USB  Live camera"
#./ffmpeg -f vfwcap -i 0 -r 25 -video_size 1280x720 -vcodec libx264 out.h264
#decode eg:
#./ffmpeg -y -hwaccel qsv -c:v h264_qsv -i $IN_FILE -vf hwdownload,format=nv12  $OUT_FILE
#./ffmpeg -hwaccel qsv -c:v h264_qsv -i $IN_FILE -vf hwdownload,format=nv12 -pix_fmt yuv420p $OUT_FILE
#./ffmpeg -hwaccel qsv -c:v h264_qsv -i $IN_FILE -vf hwdownload,format=nv12 -pix_fmt yuv420p -f sdl -
#.ffmpeg -hwaccel qsv -c:v h264_qsv -i $IN_FILE -f null -
#./ffmpeg -hwaccel qsv -c:v hevc_qsv -load_plugin hevc_hw -i $IN_FILE -vf hwdownload,format=p010 -pix_fmt p010le $OUT_FILE

#encode eg:
#./ffmpeg.exe -c:v h264_qsv -i /c/works/test/FVDO_Freeway_720p.264 -c:v h264_qsv -b:v 2M -preset veryfast /c/works/test/out_hw.264
./ffmpeg -hwaccel qsv -c:v h264_qsv -i $IN_FILE -c:v h264_qsv -b:v 2M -preset veryfast $OUT_FILE2
#./ffmpeg -hwaccel qsv -c:v h264_qsv -i $IN_FILE -c:v h264_qsv -b:v 2M -preset veryfast -max_slice_size 1100 -bsf h264_mp4toannexb $OUT_FILE4
#./ffmpeg -hwaccel qsv -c:v h264_qsv -i $IN_FILE -c:v h264_qsv -b:v 5M -look_ahead 1 $OUT_FILE2
#ffmpeg -v trace -vaapi_device /dev/dri/card0 -i 720p.mp4 -vf 'format=nv12,hwupload' -c:v h264_vaapi output.mp4
#./ffmpeg -init_hw_device qsv=hw -filter_hw_device hw -f rawvideo -pix_fmt yuv420p -s:v $WXH2 -i $IN_FILE2 -vf 'hwupload=extra_hw_frames=64,format=qsv' -c:v h264_qsv -b:v 5M $OUT_FILE2
#./ffmpeg -s 1920x1080 -pix_fmt yuv420p -i $IN_FILE2 -vcodec hevc_qsv $OUT_FILE1 -f sdl -
#./ffmpeg -hwaccel qsv -c:v hevc_qsv -i $OUT_FILE1 -vf 'vpp_qsv=framerate=60,scale_qsv=w=1920:h=1080' -c:v h264_qsv $OUT_FILE3

#./ffmpeg -init_hw_device qsv=hw -filter_hw_device hw -v verbose -f rawvideo -video_size $WXH2 -pix_fmt p010le -i $IN_FILE2 -an \
#-vf 'hwupload=extra_hw_frames=64,format=qsv' -c:v hevc_qsv -profile:v main10 $OUT_FILE2
#transcode eg: (MFE will be enabled by default if MSDK can support it)
#./ffmpeg -init_hw_device qsv=hw -filter_hw_device hw -i $OUT_FILE2 -vf hwupload=extra_hw_frames=64,format=qsv -c:v h264_qsv -b:v 5M -maxrate 5M $OUT_FILE3
#ffmpeg -init_hw_device qsv=hw -filter_hw_device hw -i input.mp4 -vf hwupload=extra_hw_frames=64,format=qsv -c:v h264_qsv -q 25 output.mp4
#./ffmpeg -hwaccel qsv -c:v h264_qsv -i $IN_FILE2 -c:v h264_qsv -b:v 5M -look_ahead 1 $OUT_FILE2
#./ffmpeg -hwaccel qsv -c:v h264_qsv -i $IN_FILE -c:v h264_qsv -global_quality 25 $OUT_FILE2
#./ffmpeg -hwaccel qsv -c:v h264_qsv -i $IN_FILE -c:v h264_qsv -global_quality 25 -look_ahead 1 $OUT_FILE2
#./ffmpeg -hwaccel qsv -c:v hevc_qsv -i $OUT_FILE2 -vf 'vpp_qsv=framerate=60,scale_qsv=w=1920:h=1080:format=rgb32,hwdownload,format=rgb32' -f sdl -
#1:N
#ffmpeg -hwaccel qsv -c:v h264_qsv -i input.mp4 \
#        -filter_complex "split=2[s1][s2]; [s1]scale_qsv=1280:720[o1];[s2]scale_qsv=960:540[o2]" \
#        -map [o1] -c:v h264_qsv -b:v 3200k 3200a.mp4 \
#        -map [o2] -c:v h264_qsv -b:v 1750k 1750a.264
#M:N
#ffmpeg -hwaccel qsv -c:v h264_qsv -i input1.mp4 -hwaccel qsv -c:v h264_qsv -i input2.mp4 \
#     -filter_complex '[0:v]split=2[out1][out2],[1:v]split=2[out3][out4]' \
#     -map '[out1]' -c:v h264_qsv output1.mp4 -map '[out2]' -c:v h264_qsv output2.mp4 \
#     -map '[out3]' -c:v h264_qsv output3.mp4 -map '[out4]' -c:v h264_qsv output4.mp4

#-qsv_device is an qsv customized option can be used to specify a hardware device and avoid the default device initialization failure when multiple devices usable (eg: an Intel integrated GPU and an AMD/Nvidia discrete graphics card). One example on Linux (more details please see ​https://trac.ffmpeg.org/ticket/7649)
#fmpeg -hwaccel qsv -qsv_device /dev/dri/renderD128 -c:v h264_qsv -i input.mp4 -c:v h264_qsv output.mp4

#Hybrid transcode eg:
#It is also possible to use "vaapi decode + vaapi scaling + qsv encode" (available on Linux platform)
#./ffmpeg -hwaccel vaapi -hwaccel_output_format vaapi -i $IN_FILE3 -vf 'scale_vaapi=1280:720,hwmap=derive_device=qsv,format=qsv' -c:v h264_qsv $OUT_FILE2

#Or use "dxva decode + qsv scaling + qsv encode" (available on Windows)
#./ffmpeg -hwaccel dxva2 -hwaccel_output_format dxva2_vld -i $IN_FILE3 -vf 'hwmap=derive_device=qsv,format=qsv,scale_qsv=w=1280:h=720' -c:v h264_qsv $OUT_FILE2
#./ffplay -f rawvideo -video_size $WXH2 $OUT_FILE
#./ffplay -i $OUT_FILE2
#./ffplay -i $OUT_FILE3
#./ffplay -i $OUT_FILE4
#./ffplay -i $OUT_FILE1

#卸载 apt-get purge ffmpeg
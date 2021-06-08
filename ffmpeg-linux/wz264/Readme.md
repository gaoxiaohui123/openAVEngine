# Visionular's H.264 Encoder Library

## 目录结构说明

* bin/wz264_sample_encoder.exe   可执行文件示例
* bin/wz264.dll                  编码器动态库文件(x86_64版)
* include/wz264/wz264enc.h       编码器的API 接口头文件
* lib/wz264.lib                  编码器动态库的索引库(x86_64版)
* samples/wz264_sample_encoder.c 编码器调用实例
* docs/wz264_encoder_sdk.md      编码器API 接口文档
* CMakeLists.txt                 编码器调用实例编译脚本 


## 编码器调用实例编译方法
* 使用cmake 生成Visual Studio的工程
```
mkdir build
cd build
cmake ../ -G  "Visual Studio 15 2017 Win64"
```
* 打开 wz264_sample_encoder.sln 编译生成即可

## 调用实例命令说明
```
Usage: wz264_sample_encoder.exe <preset> <tune> <width> <height> <fps-num> <in-yuv> <outfile> <keyframe-interval> <rc-type> <target-bitrate/crf> <threads>  <frames to encode> [debug-log]
presets: ultrafast superfast veryfast faster fast medium slow slower veryslow placebo
tune: film animation grain stillimage psnr ssim fastdecode screen zerolatency


cd bin/
wz264_sample_encoder.exe  ultrafast zerolatency 1280 720 60 Johnny_1280x720_60.yuv  out.264 100 2  100000  2  100 1
```

## 集成说明
参见 samples/wz264_sample_encoder.c 和 docs/wz264_encoder_sdk.md

#!/bin/sh

#gn gen out/linux_openssl --args=' is_debug=true target_os="linux" target_cpu="x64" is_clang=false ffmpeg_branding = "Chrome" treat_warnings_as_errors=false rtc_include_tests=false rtc_use_h264=true is_component_build=false use_custom_libcxx=false rtc_build_ssl=false  rtc_build_examples=false  rtc_build_tools=false rtc_enable_protobuf=false use_rtti=true proprietary_codecs=true  rtc_ssl_root="/usr/include/openssl/" ' --ide="qtcreator"

TOOLS_PATH=/home/gxh/works/

export PATH=$TOOLS_PATH/depot_tools:"$PATH"

ninja -C out/linux_openssl


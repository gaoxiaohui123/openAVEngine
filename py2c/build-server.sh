#!/usr/bin/env bash

#gcc -o libpycall.so -shared -fPIC ./codec/pycall.c

#!/bin/sh

a=`uname  -a`

b="Darwin"
c="centos"
d="ubuntu"

#if [[ $a =~ $b ]];then
#    echo "mac"
#elif [[ $a =~ $c ]];then
#    echo "centos"
#elif [[ $a =~ $d ]];then
#    echo "ubuntu"
#else
#    echo $a
#fi


#if ["$(expr substr $(uname -s) 1 5)" == "Linux"];then
#    echo "linux"
#    # GNU/Linux操作系统
#    echo "linux"

#elif["$(expr substr $(uname -s) 1 10)"="MINGW32_NT"];then
#    # Windows NT操作系统
#    echo "windows"

#fi

echo 'input a number'
read Num
case $Num in
1)
    gcc -o ./pycodec/librtpserver.so -shared -fPIC -Wl,-Bsymbolic -ldl \
    -Wno-incompatible-pointer-types \
    -Wno-pointer-to-int-cast \
    ./codec/utility_server.c \
    ./codec/udpbase.c \
    ./codec/udpbase_server.c \
    ./codec/cjson.c \
    -I../cJSON \
    -L./pycodec/ \
    -lcjson -lpthread -lm -lstdc++
;;
2)
    gcc -o ./pycodec/librtpserver.dll -shared -fPIC -Wl,-Bsymbolic -ldl \
    -Wno-incompatible-pointer-types \
    -Wno-pointer-to-int-cast \
    ./codec/utility_server.c \
    ./codec/udpbase.c \
    ./codec/udpbase_server.c \
    ./codec/cjson.c \
    -I../cJSON \
    -L./pycodec/ \
    -lws2_32 \
    -lcjson_win -lpthread -lm -lstdc++
;;
esac



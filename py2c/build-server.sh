#!/bin/sh

#gcc -o libpycall.so -shared -fPIC ./codec/pycall.c

gcc -o librtpserver.so -shared -fPIC -Wl,-Bsymbolic -ldl \
./codec/utility_server.c \
-I../cJSON \
-L./ \
-lcjson -lpthread -lm -lstdc++


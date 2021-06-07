CC        = gcc
AR        = ar
#-Wl,--exclude-libs=ALL
CFLAGS    = -fPIC -Wall -O -g -fvisibility=hidden -ldl -Wno-unused-variable -Wl,--version-script,version
CXXFLAGS  =
INCLUDES   = -I../ffmpeg-linux/ffmpeg-2.8.7 \
-I../cJSON \
-I../openfec_v1.4.2 

LIBVAR   = -lavformat -lavdevice -lavcodec -lavutil -lswresample -lswscale -lavfilter \
-laacplus -lx264 -lcjson -lopenfec -lpthread -lm -lstdc++
LIBPATH  = -L../ffmpeg-linux/mylib \
-L/usr/lib \
-L../openfec_v1.4.2/bin/Release 
 
TARGET    = libhcsvcapi.a
#LIBPATH   = ./

vpath %.h ./inc


SRCS      = ./codec/decoding_encoding.c \
./codec/scaling_video.c \
./codec/rate_control.c \
./codec/nallayer.c \
./codec/simple_client.c \
./codec/simple_server.c \
./codec/utility.c 

OBJS=$(SRCS:.c=.o)

.PHONY: all clean

all:$(TARGET)

$(TARGET):$(OBJS)
	#$(CC) $(CC_FLAGS) $(INCLUDES) -o $(TARGET) $^ $(LIBVAR) $(LIBPATH)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS) $(LIBVAR) $(LIBPATH)
%.o:%.c
	$(CC) -c $(CC_FLAGS) $(INCLUDES) $< -o $@ 
   
clean:
	rm -f $(SRCS:.c=.o) $(TARGET)
	
#$(OBJS):$(SRCS)
#	$(CC) $(CFLAGS) $(INCLUDE) -c $^

#all:$(OBJS) $(LIB)
	#ar rcs $(TARGET) $^          #打包 .o 文件到库文件 libmytest.a
	#mv $(TARGET) $(LIBPATH)
	#make all    #执行staticlib/Makefile 里的 make all
	#$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS) $(LIBVAR) $(LIBPATH)

#clean:
#	rm -f *.o
	#rm -f $(LIBPATH)*
#	rm -f $(TARGET)
#	make clean
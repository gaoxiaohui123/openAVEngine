#for win32 libwebrtc.a

#make -f Makefile-1

PWD := $(shell pwd)
SRC_ROOT = $(PWD)/../../../svc_webrtc/bizconf_rtc_endpoint/webrtc-checkout/src


# 1、准备工作，编译方式、目标文件名、依赖库路径的定义。
# 目标文件名
BIN = libwebrtc.so
TARGET = ../ffmpeg-linux/mylib-win/libwebrtc.a
CC = gcc  
CPP = g++
AR  = ar

#这里只加入库头文件路径及库路径  
INCS = -I/D/msys64/mingw32/include \
-I/D/msys64/mingw32/i686-w64-mingw32/include \
-I$(SRC_ROOT) \
-I$(SRC_ROOT)/third_party \
-I$(SRC_ROOT)/third_party/googletest/src/googlemock/include \
-I$(SRC_ROOT)/third_party/googletest/src/googletest/include \
-I$(SRC_ROOT)/third_party/abseil-cpp \
-I$(SRC_ROOT)/third_party/libyuv/include  

LIBS = -L/D/msys64/mingw32/lib \
-L/D/msys64/mingw32/i686-w64-mingw32/lib \
-L/usr/lib \
-L$(PWD)/ \
-L./ \
-L$(PWD)/../ffmpeg-linux/mylib-win/
SUBDIRS =  
#生成依赖信息时的搜索目录，比如到下列目录中搜索一个依赖文件(比如.h文件)，例如 -I"./***/" -I"./base/"  
DEFINC =  
#给INCS加上依赖搜索路径，分开写可能会产生不一致情况，而且繁琐  
INCS += $(DEFINC)  

#maintest.c tree/rbtree.c  多了子目录，那就直接添加 目录/*.c即可   所有的源文件--  .c文件列表  
CSRCS = $(wildcard $(SRC_ROOT)/modules/audio_processing/agc/legacy/analog_agc.c \
$(SRC_ROOT)/modules/audio_processing/agc/legacy/digital_agc.c \
$(SRC_ROOT)/modules/audio_processing/ns/noise_suppression.c \
$(SRC_ROOT)/modules/audio_processing/ns/ns_core.c)

CPPSRCS = $(wildcard $(SRC_ROOT)/modules/video_processing/util/denoiser_filter_c.cc \
$(SRC_ROOT)/modules/video_processing/util/denoiser_filter.cc \
$(SRC_ROOT)/modules/video_processing/util/noise_estimation.cc \
$(SRC_ROOT)/modules/video_processing/util/skin_detection.cc \
$(SRC_ROOT)/modules/video_processing/video_denoiser.cc \
$(SRC_ROOT)/modules/video_processing/util/denoiser_filter_sse2.cc \
$(SRC_ROOT)/modules/audio_processing/aec/aec_resampler.cc \
$(SRC_ROOT)/modules/audio_processing/aec/echo_cancellation.cc \
$(SRC_ROOT)/modules/audio_processing/aec/aec_core_sse2.cc \
$(SRC_ROOT)/modules/audio_processing/aec/aec_core.cc \
$(SRC_ROOT)/modules/audio_processing/aec_dump/null_aec_dump_factory.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/adaptive_fir_filter.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/aec_state.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/aec3_common.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/aec3_fft.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/api_call_jitter_metrics.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/block_delay_buffer.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/block_framer.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/block_processor_metrics.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/block_processor.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/cascaded_biquad_filter.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/clockdrift_detector.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/comfort_noise_generator.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/decimator.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/downsampled_render_buffer.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/echo_audibility.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/echo_canceller3.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/echo_path_delay_estimator.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/echo_path_variability.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/echo_remover_metrics.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/echo_remover.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/erl_estimator.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/erle_estimator.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/fft_buffer.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/filter_analyzer.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/frame_blocker.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/fullband_erle_estimator.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/main_filter_update_gain.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/matched_filter_lag_aggregator.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/matched_filter.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/matrix_buffer.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/moving_average.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/render_buffer.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/render_delay_buffer.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/render_delay_controller_metrics.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/render_delay_controller.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/render_reverb_model.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/render_signal_analyzer.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/residual_echo_estimator.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/reverb_decay_estimator.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/reverb_frequency_response.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/reverb_model_estimator.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/reverb_model_fallback.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/reverb_model.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/shadow_filter_update_gain.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/signal_dependent_erle_estimator.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/skew_estimator.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/stationarity_estimator.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/subband_erle_estimator.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/subtractor_output_analyzer.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/subtractor_output.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/subtractor.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/suppression_filter.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/suppression_gain.cc \
$(SRC_ROOT)/modules/audio_processing/aec3/vector_buffer.cc \
$(SRC_ROOT)/modules/audio_processing/aecm/aecm_core_c.cc \
$(SRC_ROOT)/modules/audio_processing/aecm/aecm_core.cc \
$(SRC_ROOT)/modules/audio_processing/aecm/echo_control_mobile.cc \
$(SRC_ROOT)/modules/audio_processing/agc/agc_manager_direct.cc \
$(SRC_ROOT)/modules/audio_processing/agc/agc.cc \
$(SRC_ROOT)/modules/audio_processing/agc/loudness_histogram.cc \
$(SRC_ROOT)/modules/audio_processing/agc/utility.cc \
$(SRC_ROOT)/modules/audio_processing/agc2/adaptive_agc.cc \
$(SRC_ROOT)/modules/audio_processing/agc2/adaptive_digital_gain_applier.cc \
$(SRC_ROOT)/modules/audio_processing/agc2/adaptive_mode_level_estimator.cc \
$(SRC_ROOT)/modules/audio_processing/agc2/saturation_protector.cc \
$(SRC_ROOT)/modules/audio_processing/agc2/biquad_filter.cc \
$(SRC_ROOT)/modules/audio_processing/agc2/agc2_common.cc \
$(SRC_ROOT)/modules/audio_processing/agc2/fixed_digital_level_estimator.cc \
$(SRC_ROOT)/modules/audio_processing/agc2/interpolated_gain_curve.cc \
$(SRC_ROOT)/modules/audio_processing/agc2/limiter.cc \
$(SRC_ROOT)/modules/audio_processing/agc2/gain_applier.cc \
$(SRC_ROOT)/modules/audio_processing/agc2/adaptive_mode_level_estimator_agc.cc \
$(SRC_ROOT)/modules/audio_processing/agc2/down_sampler.cc \
$(SRC_ROOT)/modules/audio_processing/agc2/noise_level_estimator.cc \
$(SRC_ROOT)/modules/audio_processing/agc2/noise_spectrum_estimator.cc \
$(SRC_ROOT)/modules/audio_processing/agc2/signal_classifier.cc \
$(SRC_ROOT)/modules/audio_processing/agc2/rnn_vad/features_extraction.cc \
$(SRC_ROOT)/modules/audio_processing/agc2/rnn_vad/fft_util.cc \
$(SRC_ROOT)/modules/audio_processing/agc2/rnn_vad/lp_residual.cc \
$(SRC_ROOT)/modules/audio_processing/agc2/rnn_vad/pitch_search_internal.cc \
$(SRC_ROOT)/modules/audio_processing/agc2/rnn_vad/pitch_search.cc \
$(SRC_ROOT)/modules/audio_processing/agc2/rnn_vad/rnn.cc \
$(SRC_ROOT)/modules/audio_processing/agc2/rnn_vad/spectral_features_internal.cc \
$(SRC_ROOT)/modules/audio_processing/agc2/rnn_vad/spectral_features.cc \
$(SRC_ROOT)/modules/audio_processing/agc2/vad_with_level.cc \
$(SRC_ROOT)/modules/audio_processing/include/audio_processing.cc \
$(SRC_ROOT)/modules/audio_processing/logging/apm_data_dumper.cc \
$(SRC_ROOT)/modules/audio_processing/audio_buffer.cc \
$(SRC_ROOT)/modules/audio_processing/splitting_filter.cc \
$(SRC_ROOT)/modules/audio_processing/three_band_filter_bank.cc \
$(SRC_ROOT)/modules/audio_processing/include/aec_dump.cc \
$(SRC_ROOT)/modules/audio_processing/audio_processing_impl.cc \
$(SRC_ROOT)/modules/audio_processing/echo_detector/circular_buffer.cc \
$(SRC_ROOT)/modules/audio_processing/echo_cancellation_impl.cc \
$(SRC_ROOT)/modules/audio_processing/echo_control_mobile_impl.cc \
$(SRC_ROOT)/modules/audio_processing/gain_control_for_experimental_agc.cc \
$(SRC_ROOT)/modules/audio_processing/gain_control_impl.cc \
$(SRC_ROOT)/modules/audio_processing/gain_controller2.cc \
$(SRC_ROOT)/modules/audio_processing/level_estimator_impl.cc \
$(SRC_ROOT)/modules/audio_processing/low_cut_filter.cc \
$(SRC_ROOT)/modules/audio_processing/echo_detector/mean_variance_estimator.cc \
$(SRC_ROOT)/modules/audio_processing/echo_detector/moving_max.cc \
$(SRC_ROOT)/modules/audio_processing/transient/moving_moments.cc \
$(SRC_ROOT)/modules/audio_processing/noise_suppression_impl.cc \
$(SRC_ROOT)/modules/audio_processing/echo_detector/normalized_covariance_estimator.cc \
$(SRC_ROOT)/modules/audio_processing/residual_echo_detector.cc \
$(SRC_ROOT)/modules/audio_processing/rms_level.cc \
$(SRC_ROOT)/modules/audio_processing/transient/transient_detector.cc \
$(SRC_ROOT)/modules/audio_processing/transient/transient_suppressor.cc \
$(SRC_ROOT)/modules/audio_processing/typing_detection.cc \
$(SRC_ROOT)/modules/audio_processing/voice_detection_impl.cc \
$(SRC_ROOT)/modules/audio_processing/transient/wpd_node.cc \
$(SRC_ROOT)/modules/audio_processing/transient/wpd_tree.cc \
$(SRC_ROOT)/modules/audio_processing/include/audio_processing_statistics.cc \
$(SRC_ROOT)/modules/audio_processing/include/config.cc \
$(SRC_ROOT)/modules/audio_processing/noise_suppression_proxy.cc \
$(SRC_ROOT)/modules/audio_processing/utility/block_mean_calculator.cc \
$(SRC_ROOT)/modules/audio_processing/utility/delay_estimator_wrapper.cc \
$(SRC_ROOT)/modules/audio_processing/utility/delay_estimator.cc \
$(SRC_ROOT)/modules/audio_processing/utility/ooura_fft_sse2.cc \
$(SRC_ROOT)/modules/audio_processing/utility/ooura_fft.cc \
$(SRC_ROOT)/modules/audio_processing/vad/gmm.cc \
$(SRC_ROOT)/modules/audio_processing/vad/pitch_based_vad.cc \
$(SRC_ROOT)/modules/audio_processing/vad/pitch_internal.cc \
$(SRC_ROOT)/modules/audio_processing/vad/pole_zero_filter.cc \
$(SRC_ROOT)/modules/audio_processing/vad/standalone_vad.cc \
$(SRC_ROOT)/modules/audio_processing/vad/vad_audio_proc.cc \
$(SRC_ROOT)/modules/audio_processing/vad/vad_circular_buffer.cc \
$(SRC_ROOT)/modules/audio_processing/vad/voice_activity_detector.cc)



#所有的.o文件列表 
COBJS := $(CSRCS:.c=.o)
CPPOBJS := $(CPPSRCS:.cc=.o)

#  
#生成依赖信息 -MM是只生成自己的头文件信息，-M 包含了标准库头文件信息。  
#-MT 或 -MQ都可以改变生成的依赖  xxx.o:src/xxx.h 为 src/xxx.o:src/xxx.h 当然。前面的 src/xxx.o需自己指定  
#格式为 -MM 输入.c或.cpp  查找依赖路径  -MT或-MQ  生成规则，比如src/xxx.o  
#MAKEDEPEND = gcc -MM -MT

#-g 生成调试信息  
#-pedantic参数与-ansi一起使用 会自动拒绝编译非ANSI程序  
#-fomit-frame-pointer 去除函数框架  
#-Wmissing-prototypes -Wstrict-prototypes 检查函数原型 
CFLAGS += $(INCS) 
#-Wall -O3 -std=c++0x 
#-O2 -Wall -fomit-frame-pointer -g -ansi
CFLAGS  += -O2 -Wall -g -fPIC \
-D WEBRTC_WIN \
-D WEBRTC_NS_FLOAT \
-D WEBRTC_APM_DEBUG_DUMP=0 \
-D MSYS2_GXH \
-mavx \
-fpermissive \
-mmmx \
-msse \
-msse2 \
-mssse3 \
-msse4.1 \
-msse4.2 \
-msse4

CPPFLAGS += $(INCS)  
CPPFLAGS += -O2 -Wall -g -fPIC \
-D WEBRTC_WIN \
-D WEBRTC_NS_FLOAT \
-D WEBRTC_APM_DEBUG_DUMP=0 \
-D MSYS2_GXH \
-mavx \
-fpermissive \
-mmmx \
-msse \
-msse2 \
-mssse3 \
-msse4.1 \
-msse4.2 \
-msse4

LDFLAGS += -shared -fPIC -lm -lrt -lz -lstdc++ -lwinmm \
-lwebrtc_base
 
#针对每个.c文件的.d依赖文件列表  
#CDEF = $(CSRCS:.c=.d)  
#CPPDEF = $(CPPSRCS:.cc=.d) 


all: $(TARGET) 

#生成.o的对自己目录中.h .c的依赖信息.d文件到.c所在的路径中  
#$(DEF)文件是.d文件名列表(含目录)，比如tree.d 匹配成功那么%就是tree，然后在尝试%.c，如果成功。则执行规则  
# $(<:.c=.o)是获取此.c文件的名字(含路径)，然后变为.o比如 src/xxx.o。 以形成如下  
# src/xxx.o : src/xxx.c ***.h  ***.h  最前面！！注意。    
# 此做法是每个.d都和生成他的.c在一个目录里，所以需要这样做。  
# $(<:.c=.o)之类的 。此时的<相当于变量$< 。切记  
# : : :  含义同下  
#$(CDEF) : %.d : %.c  
#	$(MAKEDEPEND) $(<:.c=.o) $< $(DEFINC) > $@  
#$(CPPDEF) : %.d : %.cc  
#	$(MAKEDEPEND) $(<:.cc=.o) $< $(DEFINC) > $@ 
	
#先删除依赖信息  
#重新生成依赖信息  
#这里出现了一个 $(MAKE) 没有定义的变量。这个变量是由 Make 自己定义的，它的值即为自己的位置，方便 Make 递归调用自己。  
#depend:  
#	rm $(CDEF)  
#	rm $(CPPDEF)  
#	$(MAKE) $(CDEF)  
#	$(MAKE) $(CPPDEF)  

#$(OBJS):%.o :%.c  先用$(OBJS)中的一项，比如foo.o: %.o : %.c  含义为:试着用%.o匹配foo.o。如果成功%就等于foo。如果不成功，  
# Make就会警告，然后。给foo.o添加依赖文件foo.c(用foo替换了%.c里的%)  
# 也可以不要下面的这个生成规则，因为下面的 include $(DEF)  就隐含了。此处为了明了，易懂。故留着  
$(COBJS) : %.o: %.c  
	$(CC) -c $< -o $@ $(CFLAGS)  
$(CPPOBJS) : %.o: %.cc  
	$(CPP) -c $< -o $@ $(CPPFLAGS)  


# $@--目标文件，$^--所有的依赖文件，$<--第一个依赖文件。每次$< $@ 代表的值就是列表中的  
#  
$(TARGET) : $(COBJS) $(CPPOBJS)  
	#$(CC) -o $(BIN) $(COBJS) $(CPPOBJS) $(LDFLAGS) $(LIBS)  
	ar rcs $(TARGET) $^
	rm $(COBJS)
	rm $(CPPOBJS)
#	rm $(CDEF)  
#	rm $(CPPDEF)  
# 链接为最终目标  
  
  
#引入了.o文件对.c和.h的依赖情况。以后.h被修改也会重新生成，可看看.d文件内容即知道为何  
#引入了依赖就相当于引入了一系列的规则，因为依赖内容例如： 目录/xxx.o:目录/xxx.c 目录/xxx.h 也相当于隐含的引入了生成规则  
#故上面不能在出现如： $(OBJS) : $(DEF)之类。切记  
#include $(CDEF)  
#include $(CPPDEF)  
.PHONY:clean cleanall  
  
#清除所有目标文件以及生成的最终目标文件  
clean:              
	rm $(TARGET) $(COBJS) $(CPPOBJS)  
#rm *.d  
cleanall:  
	#rm $(CDEF) $(CPPDEF)  
	rm  $(COBJS) $(CPPOBJS)
	#$(TARGET)
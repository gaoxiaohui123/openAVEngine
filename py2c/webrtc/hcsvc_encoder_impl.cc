/*
 *  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 *
 */

#include "hcsvc_encoder_impl.h"

#include <limits>
#include <string>

//#include "third_party/openh264/src/codec/api/svc/codec_api.h"
//#include "third_party/openh264/src/codec/api/svc/codec_app_def.h"
//#include "third_party/openh264/src/codec/api/svc/codec_def.h"
//#include "third_party/openh264/src/codec/api/svc/codec_ver.h"

#include "absl/strings/match.h"
#include "common_video/libyuv/include/webrtc_libyuv.h"
#include "modules/video_coding/utility/simulcast_rate_allocator.h"
#include "modules/video_coding/utility/simulcast_utility.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/time_utils.h"
#include "system_wrappers/include/metrics.h"
#include "third_party/libyuv/include/libyuv/convert.h"
#include "third_party/libyuv/include/libyuv/scale.h"
#include "SimpleLog.h"
namespace webrtc {

namespace {

const bool kOpenH264EncoderDetailedLogging = false;

// QP scaling thresholds.
static const int kLowH264QpThreshold = 24;
static const int kHighH264QpThreshold = 37;

// Used by histograms. Values of entries should not be changed.
enum HCSVCEncoderImplEvent {
    kH264EncoderEventInit = 0,
    kH264EncoderEventError = 1,
    kH264EncoderEventMax = 16,
};

int NumberOfThreads(int width, int height, int number_of_cores) {
    // TODO(hbos): In Chromium, multiple threads do not work with sandbox on Mac,
    // see crbug.com/583348. Until further investigated, only use one thread.
    if (width * height >= 1920 * 1080 && number_of_cores > 8) {
        return 8;  // 8 threads for 1080p on high perf machines.
    } else if (width * height > 1280 * 960 && number_of_cores >= 6) {
        return 3;  // 3 threads for 1080p.
    } else if (width * height > 640 * 480 && number_of_cores >= 3) {
        return 2;  // 2 threads for qHD/HD.
    } else {
        return 1;  // 1 thread for VGA or less.
    }
    // TODO(sprang): Also check sSliceArgument.uiSliceNum om GetEncoderPrams(),
    //               before enabling multithreading here.
    return 1;
}

//FrameType ConvertToVideoFrameType(int type) {
//  switch (type) {
//    case videoFrameTypeIDR:
//      return kVideoFrameKey;
//    case videoFrameTypeSkip:
//    case videoFrameTypeI:
//    case videoFrameTypeP:
//    case videoFrameTypeIPMixed:
//      return kVideoFrameDelta;
//    case videoFrameTypeInvalid:
//      break;
//  }
//  RTC_NOTREACHED() << "Unexpected/invalid frame type: " << type;
//  return kEmptyFrame;
//}

}  // namespace

// Helper method used by X264EncoderImpl::Encode.
// Copies the encoded bytes from |info| to |encoded_image| and updates the fragmentation information of |frag_header|. The |encoded_image->_buffer| may
// be deleted and reallocated if a bigger buffer is required.
//
// After OpenH264 encoding, the encoded bytes are stored in |info| spread out
// over a number of layers and "NAL units". Each NAL unit is a fragment starting
// with the four-byte start code {0,0,0,1}. All of this data (including the
// start codes) is copied to the |encoded_image->_buffer| and the |frag_header|
// is updated to point to each fragment, with offsets and lengths set as to
// exclude the start codes.
static void RtpFragmentize(EncodedImage* encoded_image,
                           const VideoFrameBuffer& frame_buffer,
                           HCSVCEncoderImpl::m_Encoder_ * encoders,
                           RTPFragmentationHeader* frag_header,
                           int nNal) {
    //    SimgpleLog log(__func__);
    // Calculate minimum buffer size required to hold encoded data.
    size_t required_capacity = 0;
    size_t fragments_count = 0;
#if 1
    //HCNOTE 测试内存分配方案 进行计时优化.
    //    for (int i = 0; i < nNal; i++) {
    //         required_capacity += encoders->nal[i].i_payload;
    //    }
    //    required_capacity +100;//多10个？
    //    if (encoded_image->capacity() < required_capacity) {
    // Increase buffer size. Allocate enough to hold an unencoded image, this
    // should be more than enough to hold any encoded data of future frames of
    // the same size (avoiding possible future reallocation due to variations in
    // required size).
    //    size_t new_capacity = CalcBufferSize(VideoType::kI420, frame_buffer.width(), frame_buffer.height());
    //        if (new_capacity < required_capacity) {
    //            // Encoded data > unencoded data. Allocate required bytes.
    //            RTC_LOG(LS_WARNING)
    //                    << "Encoding produced more bytes than the original image "
    //                    << "data! Original bytes: " << new_capacity
    //                    << ", encoded bytes: " << required_capacity << ".";
    //            new_capacity = required_capacity;
    //        }
    //    encoded_image->Allocate(new_capacity);
    //    }

    // Iterate layers and NAL units, note each NAL unit as a fragment and copy the data to |encoded_image->_buffer
    const uint8_t start_code[4] = {0, 0, 0, 1};
    frag_header->VerifyAndAllocateFragmentationHeader(nNal);
    size_t frag = 0;
    encoded_image->set_size(0);
    //=========拷贝输出的编码数据==================
    unsigned char * p_out =encoded_image->data();
    size_t layer_len = 0;
    for (int i = 0; i < nNal; ++i,++frag) {
        size_t startcodeleng = 3;
        if(encoders->outbuffer[layer_len + 0] == start_code[0]
                &&encoders->outbuffer[layer_len + 1] == start_code[1]
                &&encoders->outbuffer[layer_len + 2] == start_code[2]
                && encoders->outbuffer[layer_len + 3] == start_code[3]){
            startcodeleng = 4;
        }
//        std::cout<<"startcodeleng: "<<startcodeleng<<std::endl;
        size_t  nallength = (size_t)encoders->nal_size[i];
        memcpy(p_out, &encoders->outbuffer[layer_len], nallength);
        p_out += nallength;
        frag_header->fragmentationOffset[frag] = layer_len + startcodeleng;
        frag_header->fragmentationLength[frag] = nallength - startcodeleng;
        layer_len += nallength;
        //        std::cout<<"nallength:"<<nallength<<",frag:"<<frag_header->fragmentationOffset[frag]
        //        <<",layerlen:"<<layer_len<<std::endl;
    }
    encoded_image->set_size(layer_len);
    if(encoders->_test){
        //fwrite(encoded_image->data(),layer_len,1,encoders->_test);
    }
#endif
}

HCSVCEncoderImpl::HCSVCEncoderImpl(const cricket::VideoCodec& codec)
    : packetization_mode_(H264PacketizationMode::SingleNalUnit),
      max_payload_size_(0),
      number_of_cores_(0),
      encoded_image_callback_(nullptr),
      has_reported_init_(false),
      has_reported_error_(false),
      num_temporal_layers_(1),
      tl0sync_limit_(0) {
    SimgpleLog log(__func__);
    RTC_CHECK(absl::EqualsIgnoreCase(codec.name, cricket::kH264CodecName));
    std::string packetization_mode_string;
    if (codec.GetParam(cricket::kH264FmtpPacketizationMode,
                       &packetization_mode_string) &&
            packetization_mode_string == "1") {
        packetization_mode_ = H264PacketizationMode::NonInterleaved;
    }
    downscaled_buffers_.reserve(kMaxSimulcastStreams - 1);
    encoded_images_.reserve(kMaxSimulcastStreams);
    encoders_.reserve(kMaxSimulcastStreams);
    configurations_.reserve(kMaxSimulcastStreams);
}

HCSVCEncoderImpl::~HCSVCEncoderImpl() {
    SimgpleLog log(__func__);
    Release();
}

int32_t HCSVCEncoderImpl::InitEncode(const VideoCodec* inst,
                                    int32_t number_of_cores,
                                    size_t max_payload_size) {
    SimgpleLog log(__func__);

    if(!logfp_)
    {
        ///logfp_ = fopen("/home/gxh/hcsvc_enc_gxh.txt", "w");
    }
    if(logfp_)
    {
        fprintf(logfp_,"InitEncode: start \n");
        fflush(logfp_);
    }
    char current_absolute_path[256];
    //获取当前目录绝对路径
    if (NULL == getcwd(current_absolute_path, 256))
    {
        printf("***Error***\n");
        exit(-1);
    }
    printf("InitEncode: current absolute path:%s\n", current_absolute_path);
    //int ret = hcsvc_dll_init("/home/yang/datashare/jy/py2c/libhcsvcapi.so");
    //strcat(current_absolute_path, "/libhcsvcapi.so");
    //int ret = hcsvc_dll_init(current_absolute_path);
    //if(logfp_)
    //{
    //    fprintf(logfp_,"InitEncode: ret= %d \n", ret);
    //    fflush(logfp_);
    //}
    ReportInit();
    RTC_LOG(LS_INFO) << __LINE__<< __func__;
    if (!inst || inst->codecType != kVideoCodecH264) {
        ReportError();
        RTC_LOG(LS_INFO) <<"not h264";
        return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
    }
    if (inst->maxFramerate == 0) {
        ReportError();
        return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
    }
    if (inst->width < 1 || inst->height < 1) {
        ReportError();
        return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
    }

    int32_t release_ret = Release();
    if (release_ret != WEBRTC_VIDEO_CODEC_OK) {
        ReportError();
        return release_ret;
    }
    //simulcast的分层.
    size_t number_of_streams = SimulcastUtility::NumberOfSimulcastStreams(*inst);
    bool doing_simulcast = (number_of_streams > 1);
    if (doing_simulcast &&
            !SimulcastUtility::ValidSimulcastParameters(*inst, number_of_streams)) {
        return WEBRTC_VIDEO_CODEC_ERR_SIMULCAST_PARAMETERS_NOT_SUPPORTED;
    }
    if(logfp_)
    {
        fprintf(logfp_,"InitEncode: number_of_streams= %d \n", number_of_streams);
        fflush(logfp_);
    }
    downscaled_buffers_.resize(number_of_streams - 1);
    encoded_images_.resize(number_of_streams);
    encoders_.resize(number_of_streams);
    configurations_.resize(number_of_streams);
    number_of_cores_ = number_of_cores;
    max_payload_size_ = max_payload_size;
    codec_ = *inst;
    num_temporal_layers_ = codec_.H264()->numberOfTemporalLayers;
    // Code expects simulcastStream resolutions to be correct, make sure they are filled even when there are no simulcast layers.

    if (codec_.numberOfSimulcastStreams == 0) {
        codec_.simulcastStream[0].width = codec_.width;
        codec_.simulcastStream[0].height = codec_.height;
    }
    std::cout<<"number_of_streams:"<<number_of_streams<<std::endl;

    for (size_t i = 0, idx = number_of_streams - 1; i < number_of_streams;++i, --idx) {
        if(logfp_)
        {
            fprintf(logfp_,"InitEncode: i= %d \n", i);
            fflush(logfp_);
        }
        // Set internal settings from codec_settings
        configurations_[i].simulcast_idx = idx;
        //HCNOTE 默认是否应该全部发送.
        configurations_[i].sending = false;
        configurations_[i].width = codec_.simulcastStream[idx].width;
        configurations_[i].height = codec_.simulcastStream[idx].height;
        configurations_[i].max_frame_rate = /*static_cast<float>(codec_.maxFramerate)*/25;//30;
        configurations_[i].frame_dropping_on = codec_.H264()->frameDroppingOn;
        configurations_[i].key_frame_interval = codec_.H264()->keyFrameInterval;
        //    // Codec_settings uses kbits/second; encoder uses bits/second.
        configurations_[i].max_bps = codec_.maxBitrate * 1000;
        configurations_[i].target_bps = codec_.startBitrate * 1000;
        if(logfp_)
        {
            fprintf(logfp_,"InitEncode: codec_.maxBitrate= %d \n", codec_.maxBitrate);
            fprintf(logfp_,"InitEncode: codec_.startBitrate= %d \n", codec_.startBitrate);
            fprintf(logfp_,"InitEncode: configurations_[i].frame_dropping_on= %d \n", configurations_[i].frame_dropping_on);
            fflush(logfp_);
        }
        m_Encoder_ * tmphcsvcencoder = new m_Encoder_;
        char ctmp[32] = "";
        char pathname[256] = "/home/gxh/test_gxh_";
        sprintf(ctmp, "%d", (int)idx);
        strcat(pathname, ctmp);
        strcat(pathname, ".h264");

        tmphcsvcencoder->_test = fopen(pathname,"w");
        encoders_[i] = tmphcsvcencoder;
        //compress_begin(tmphcsvcencoder,configurations_[i].width,configurations_[i].height);
        // Create downscaled image buffers.
        if (i > 0) {
            downscaled_buffers_[i - 1] = I420Buffer::Create(
                        configurations_[i].width, configurations_[i].height,
                        configurations_[i].width, configurations_[i].width / 2,
                        configurations_[i].width / 2);
        }
        const size_t new_capacity =CalcBufferSize(VideoType::kI420, codec_.simulcastStream[idx].width, codec_.simulcastStream[idx].height);
        encoded_images_[i].Allocate(new_capacity);
        encoded_images_[i]._completeFrame = true;
        encoded_images_[i]._encodedWidth = codec_.simulcastStream[idx].width;
        encoded_images_[i]._encodedHeight = codec_.simulcastStream[idx].height;
        encoded_images_[i].set_size(0);
        //
        int w = configurations_[i].width;
        int h = configurations_[i].height;
        int frame_size = (w * h * 3) / 2;
        if(logfp_)
        {
            fprintf(logfp_,"InitEncode: frame_size= %d \n", frame_size);
            fflush(logfp_);
        }
        printf("InitEncode: codec_.startBitrate= %d \n", codec_.startBitrate);
        configurations_[i].seqnum = 0;//  # can be changed by fec
        configurations_[i].enable_fec = 0;
        configurations_[i].refresh_idr = 1;
        configurations_[i].lost_rate = 0.3;
        configurations_[i].code_rate = (1 - configurations_[i].lost_rate);
        configurations_[i].max_refs = 16;
        configurations_[i].mtu_size = 1400;
        configurations_[i].frame_rate = configurations_[i].max_frame_rate;
        configurations_[i].gop_size = configurations_[i].frame_rate << 1;
        configurations_[i].qp = 26;
        configurations_[i].refs = 16;//1;//16;//4;
        configurations_[i].bit_rate = codec_.startBitrate * 1000;
        configurations_[i].json = NULL;
        for(int l = 0; l < 4; l++)
        {
            tmphcsvcencoder->outparam[l] = NULL;
        }
        tmphcsvcencoder->obj_id = idx;
        tmphcsvcencoder->inbuffer = new uint8_t [frame_size];
        tmphcsvcencoder->outbuffer = new uint8_t [frame_size];
        compress_begin(tmphcsvcencoder, &configurations_[i]);
    }
    if(logfp_)
    {
        fprintf(logfp_,"InitEncode: end \n");
        fflush(logfp_);
    }
    SimulcastRateAllocator init_allocator(codec_);
    VideoBitrateAllocation allocation = init_allocator.GetAllocation(codec_.startBitrate * 1000, codec_.maxFramerate);
    std::cout<<"allocation"<<allocation.GetBitrate(1,1)<<std::endl;
    return SetRateAllocation(allocation, codec_.maxFramerate);
}

int32_t HCSVCEncoderImpl::Release() {
    while (!encoders_.empty()) {
        m_Encoder_*  encoder = encoders_.back();
        if(logfp_)
        {
            fprintf(logfp_,"Release: start \n");
            fflush(logfp_);
        }
        if(encoder->_test){
            fclose(encoder->_test);
        }
        if (encoder) {
            compress_end(encoder);
        }
        encoders_.pop_back();
        if(logfp_)
        {
            fclose(logfp_);
            logfp_ = NULL;
        }
    }
    downscaled_buffers_.clear();
    configurations_.clear();
    encoded_images_.clear();

    return WEBRTC_VIDEO_CODEC_OK;
}

int32_t HCSVCEncoderImpl::RegisterEncodeCompleteCallback(
        EncodedImageCallback* callback) {
    encoded_image_callback_ = callback;
    return WEBRTC_VIDEO_CODEC_OK;
}

int32_t HCSVCEncoderImpl::SetRateAllocation(
        const VideoBitrateAllocation& bitrate,
        uint32_t new_framerate) {
    SimgpleLog log(__func__);
#if 1
    if(logfp_)
    {
        fprintf(logfp_,"SetRateAllocation: start \n");
        fflush(logfp_);
    }
    if (encoders_.empty())
        return WEBRTC_VIDEO_CODEC_UNINITIALIZED;

    //HCNOTE 帧率改变:
    if (new_framerate < 1)
        return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;

    if (bitrate.get_sum_bps() == 0) {
        // Encoder paused, turn off all encoding.
        for (size_t i = 0; i < configurations_.size(); ++i)
            configurations_[i].SetStreamState(false);
        return WEBRTC_VIDEO_CODEC_OK;
    }
    // At this point, bitrate allocation should already match codec settings.
    if (codec_.maxBitrate > 0)
        RTC_DCHECK_LE(bitrate.get_sum_kbps(), codec_.maxBitrate);
    RTC_DCHECK_GE(bitrate.get_sum_kbps(), codec_.minBitrate);
    if (codec_.numberOfSimulcastStreams > 0)
        RTC_DCHECK_GE(bitrate.get_sum_kbps(), codec_.simulcastStream[0].minBitrate);
    codec_.maxFramerate = new_framerate;
    //设置码率  rc.i_rc_method
    size_t stream_idx = encoders_.size() - 1;
    for (size_t i = 0; i < encoders_.size(); ++i, --stream_idx) {
        // Update layer config.
        configurations_[i].target_bps = bitrate.GetSpatialLayerSum(stream_idx);
        printf("SetRateAllocation: configurations_[i].target_bps= %d \n", configurations_[i].target_bps);
        configurations_[i].max_frame_rate = static_cast<float>(new_framerate);

        if (configurations_[i].target_bps) {
            configurations_[i].SetStreamState(true);
            //HCNOTE Update h264 encoder.
            ///encoders_[i]->param->rc.i_rc_method = X264_RC_ABR;
            ///encoders_[i]->param->rc.i_vbv_max_bitrate=(int)((configurations_[i].max_frame_rate*1.2)) ;//
            ///encoders_[i]->param->rc.i_bitrate = configurations_[i].target_bps;//h264可能需要除以1000
        } else {
            configurations_[i].SetStreamState(false);
        }
    }
    if(logfp_)
    {
        fprintf(logfp_,"SetRateAllocation: end \n");
        fflush(logfp_);
    }
#endif
    return WEBRTC_VIDEO_CODEC_OK;
}

int32_t HCSVCEncoderImpl::Encode(const VideoFrame& input_frame,
                                const CodecSpecificInfo* codec_specific_info,
                                const std::vector<FrameType>* frame_types) {
    //    SimgpleLog log(__func__);
    if(logfp_)
    {
        fprintf(logfp_,"Encode: start \n");
        fflush(logfp_);
    }
    if (encoders_.empty()) {
        ReportError();
        return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
    }
#if 1
    if (!encoded_image_callback_) {
        RTC_LOG(LS_WARNING) << "InitEncode() has been called, but a callback function "<< "has not been set with RegisterEncodeCompleteCallback()";
        ReportError();
        return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
    }
    //获取输入的视频数据;
    rtc::scoped_refptr<const I420BufferInterface> frame_buffer =input_frame.video_frame_buffer()->ToI420();
    if(logfp_)
    {
        fprintf(logfp_,"Encode: configurations_.size()= %d \n", configurations_.size());
        fflush(logfp_);
    }
    //判断是不是需要立即编码I帧
    bool send_key_frame = false;
    for (size_t i = 0; i < configurations_.size(); ++i) {
        if (configurations_[i].key_frame_request && configurations_[i].sending) {
            send_key_frame = true;
            break;
        }
    }
    if (!send_key_frame && frame_types) {
        for (size_t i = 0; i < frame_types->size() && i < configurations_.size();
             ++i) {
            if ((*frame_types)[i] == kVideoFrameKey && configurations_[i].sending) {
                send_key_frame = true;
                break;
            }
        }
    }
    RTC_DCHECK_EQ(configurations_[0].width, frame_buffer->width());
    RTC_DCHECK_EQ(configurations_[0].height, frame_buffer->height());

    long long start_time = api_get_time_stamp_ll();
    if(start_time_)
    {
        int difftime = (int)(start_time - start_time_);
        //printf("Encode: interval= %d ms \n", difftime);
        if(logfp_ !=nullptr){

            fprintf(logfp_,"Encode: interval= %d \n", difftime);
            fflush(logfp_);
        }
    }
    start_time_ = start_time;
    // Encode image for each layer.
    for (size_t i = 0; i < encoders_.size(); ++i) {
        if (i == 0) {
#if 1
            int w = configurations_[i].width;
            int h = configurations_[i].height;
            uint8_t *dst_y = encoders_[i]->inbuffer;
            uint8_t *dst_u = &encoders_[i]->inbuffer[w * h];
            uint8_t *dst_v = &encoders_[i]->inbuffer[w * h + ((w >> 1) * (h >> 1))];
            uint8_t *src_y = const_cast<uint8_t*>(frame_buffer->DataY());
            uint8_t *src_u = const_cast<uint8_t*>(frame_buffer->DataU());
            uint8_t *src_v = const_cast<uint8_t*>(frame_buffer->DataV());
            memcpy(dst_y, src_y, w * h);
            memcpy(dst_u, src_u, ((w >> 1) * (h >> 1)));
            memcpy(dst_v, src_v, ((w >> 1) * (h >> 1)));
            if(logfp_)
            {
                fprintf(logfp_,"Encode: frame_buffer->StrideY()= %d \n", frame_buffer->StrideY());
                fflush(logfp_);
            }
#else
            encoders_[i]->picture->img.i_stride[0]= frame_buffer->StrideY();
            encoders_[i]->picture->img.i_stride[1]= frame_buffer->StrideU();
            encoders_[i]->picture->img.i_stride[2]= frame_buffer->StrideV();
            encoders_[i]->picture->img.plane[0] = const_cast<uint8_t*>(frame_buffer->DataY());
            encoders_[i]->picture->img.plane[1] = const_cast<uint8_t*>(frame_buffer->DataU());
            encoders_[i]->picture->img.plane[2] = const_cast<uint8_t*>(frame_buffer->DataV());
#endif
        } else {
            //downscaled_buffers_ 比encoders 少一层. 0在上面处理了。 原始图像.
#if 1
            int w0 = configurations_[i - 1].width;
            int h0 = configurations_[i - 1].height;
            int w = configurations_[i].width;
            int h = configurations_[i].height;
            //int stride_y = downscaled_buffers_[i - 1]->StrideY();
            //int stride_u = downscaled_buffers_[i - 1]->StrideU();
            //int stride_v = downscaled_buffers_[i - 1]->StrideV();
            uint8_t *dst_y = encoders_[i]->inbuffer;
            uint8_t *dst_u = &encoders_[i]->inbuffer[w * h];
            uint8_t *dst_v = &encoders_[i]->inbuffer[w * h + ((w >> 1) * (h >> 1))];
            uint8_t *src_y = const_cast<uint8_t*>(downscaled_buffers_[i - 1]->DataY());
            uint8_t *src_u = const_cast<uint8_t*>(downscaled_buffers_[i - 1]->DataU());
            uint8_t *src_v = const_cast<uint8_t*>(downscaled_buffers_[i - 1]->DataV());
            if(logfp_)
            {
                fprintf(logfp_,"Encode: downscaled_buffers_[i - 1]->StrideY()= %d \n", downscaled_buffers_[i - 1]->StrideY());
                fflush(logfp_);
            }
            libyuv::I420Scale(
                        src_y, w0,
                        src_u, w0 >> 1,
                        src_v, w0 >> 1,
                        w0, h0,
                        dst_y, w,
                        dst_u, w >> 1,
                        dst_v, w >> 1,
                        w, h,
                        libyuv::kFilterBilinear);
#else
            encoders_[i]->picture->img.i_stride[0]= downscaled_buffers_[i - 1]->StrideY();
            encoders_[i]->picture->img.i_stride[1]= downscaled_buffers_[i - 1]->StrideU();
            encoders_[i]->picture->img.i_stride[2]= downscaled_buffers_[i - 1]->StrideV();
            encoders_[i]->picture->img.plane[0] = const_cast<uint8_t*>(downscaled_buffers_[i - 1]->DataY());
            encoders_[i]->picture->img.plane[1] = const_cast<uint8_t*>(downscaled_buffers_[i - 1]->DataU());
            encoders_[i]->picture->img.plane[2] = const_cast<uint8_t*>(downscaled_buffers_[i - 1]->DataV());
            libyuv::I420Scale(
                        encoders_[i-1]->picture->img.plane[0], encoders_[i-1]->picture->img.i_stride[0],
                    encoders_[i-1]->picture->img.plane[1], encoders_[i-1]->picture->img.i_stride[1],
                    encoders_[i-1]->picture->img.plane[2], encoders_[i-1]->picture->img.i_stride[2],
                    configurations_[i - 1].width,configurations_[i - 1].height,
                    encoders_[i]->picture->img.plane[0], encoders_[i]->picture->img.i_stride[0],
                    encoders_[i]->picture->img.plane[1], encoders_[i]->picture->img.i_stride[1],
                    encoders_[i]->picture->img.plane[2], encoders_[i]->picture->img.i_stride[2],
                    configurations_[i].width,configurations_[i].height,
                    libyuv::kFilterBilinear);
#endif
        }

        if (!configurations_[i].sending) {
            continue;
        }
        if (frame_types != nullptr) {
            // Skip frame?
            if ((*frame_types)[i] == kEmptyFrame) {
                continue;
            }
        }
        ///encoders_[i]->picture->i_type = X264_TYPE_AUTO;
        if (send_key_frame) {
//            std::cout<<"send_key_frame"<<std::endl;
            //I帧率请求.API doc says ForceIntraFrame(false) does nothing, but calling this
            // function forces a key frame regardless of the |bIDR| argument's value.(If every frame is a key frame we get lag/delays.)
            ///encoders_[i]->picture->b_keyframe = true;
            configurations_[i].key_frame_request = false;
            ///encoders_[i]->picture->i_type = X264_TYPE_IDR;
//            encoders_[i]->picture->i_type = X264_TYPE_I;
        }
//        std::cout<<"encoders_[i]->picture->i_type:"<<encoders_[i]->picture->i_type<<std::endl;
        ///x264_picture_t pic_out;
        int nNal = 0;
        ///encoders_[i]->nal=nullptr;//???
#if 1
        LayerConfig *config = &configurations_[i];
        int gop_size = config->gop_size;
        int refs = config->refs;
        int max_refs = config->max_refs;
        int frame_idx = encoders_[i]->frame_idx;
        int ref_idx = 0;
        encoders_[i]->frame_idx++;
        if (frame_idx % gop_size)
        {
            int I = (frame_idx % gop_size);
            int j = (I - 1) % (refs);
            //I P0 P1 P2 P0 P1 P2 (ref=3)
            ref_idx = j + 1;
            if(I >= ((int)(gop_size / max_refs) * max_refs))
            {
                ref_idx = 1;
            }else if(((ref_idx & 1) == 0) &&
                     (ref_idx != refs) &&
                     (refs > 2))
            {
                ref_idx = 1;
            }
            else if(ref_idx == (refs - 1) && (refs > 4))
            {
                ref_idx = 1;
            }
        }
        else{
            ref_idx = 0;
            if(frame_idx > 0)
            {
                config->refresh_idr = 0;
                config->json = api_renew_json_int(config->json, "refresh_idr", config->refresh_idr);
            }
        }
        //if(logfp_)
        //{
        //    fprintf(logfp_,"Encode: gop_size= %d \n", gop_size);
        //    fprintf(logfp_,"Encode: ref_idx= %d \n", ref_idx);
        //    fflush(logfp_);
        //}
        config->json = api_renew_json_int(config->json, "frame_idx", frame_idx);
        //self.param.update({"seqnum": self.seqnum})
        if ((config->refs & 1) == 0)
        {
            //self.param.update({"ref_idx": ref_idx})
            config->json = api_renew_json_int(config->json, "ref_idx", ref_idx);
        }
        char* jsonstr = api_json2str(config->json);
        if(logfp_)
        {
            fprintf(logfp_,"Encode: jsonstr= %s \n", jsonstr);
            fflush(logfp_);
        }
        //self.outparam[0] = "test1"
        long long start_time = api_get_time_stamp_ll();
        int ret = api_video_encode_one_frame(encoders_[i]->obj_id,
                                            (char *)encoders_[i]->inbuffer,
                                            jsonstr,
                                            (char *)encoders_[i]->outbuffer,
                                            encoders_[i]->outparam);
        long long end_time = api_get_time_stamp_ll();
        int difftime = (int)(end_time - start_time);
        printf("Encode: difftime= %d ms \n", difftime);
        if(logfp_)
        {
            fprintf(logfp_,"Encode: ret= %d \n", ret);
            fprintf(logfp_,"Encode: difftime= %d \n", difftime);
            fflush(logfp_);
        }
        api_json2str_free(jsonstr);

        if(ret > 0)
        {
            if(encoders_[i]->_test){
                fwrite(encoders_[i]->outbuffer, ret,1, encoders_[i]->_test);
                fflush(encoders_[i]->_test);
            }
            if(!strcmp(encoders_[i]->outparam[0], "PIC_TYPE_KEYFRAME"))
            {
                encoded_images_[i]._frameType = kVideoFrameKey;
            }
            else{
                encoded_images_[i]._frameType = kVideoFrameDelta;
            }
            if(logfp_)
            {
                fprintf(logfp_,"Encode: encoders_[i]->outparam[0]= %s \n", encoders_[i]->outparam[0]);
                fflush(logfp_);
            }
            //int nal_num = 0;
            encoders_[i]->nal_size = api_get_array_by_str(encoders_[i]->outparam[1], ',', &nNal);
            //api_get_array_free(nal_size);
            if(logfp_)
            {
                fprintf(logfp_,"Encode: nNal= %d \n", nNal);
                fflush(logfp_);
            }
        }
#else
        if (x264_encoder_encode( encoders_[i]->handle, &( encoders_[i]->nal), &nNal,  encoders_[i]->picture, &pic_out) < 0) {
            return -1;
        }
#endif
        ///encoders_[i]->picture->i_pts++;

        encoded_images_[i]._encodedWidth = configurations_[i].width;
        encoded_images_[i]._encodedHeight = configurations_[i].height;
        encoded_images_[i].SetTimestamp(input_frame.timestamp());
        encoded_images_[i].ntp_time_ms_ = input_frame.ntp_time_ms();
        encoded_images_[i].capture_time_ms_ = input_frame.render_time_ms();
        encoded_images_[i].rotation_ = input_frame.rotation();
        encoded_images_[i].SetColorSpace(input_frame.color_space());
        //        encoded_images_[i].content_type_ =(codec_.mode == VideoCodecMode::kScreensharing) ? VideoContentType::SCREENSHARE : VideoContentType::UNSPECIFIED;
        encoded_images_[i].content_type_ = VideoContentType::UNSPECIFIED;
        encoded_images_[i].timing_.flags = VideoSendTiming::kInvalid;
        encoded_images_[i].SetSpatialIndex(configurations_[i].simulcast_idx);
#if 0
        switch (pic_out.i_type) {
        case X264_TYPE_AUTO:
            break;
        case X264_TYPE_IDR:
        case X264_TYPE_KEYFRAME:
        case X264_TYPE_I:
            encoded_images_[i]._frameType = kVideoFrameKey;
            break;
        case X264_TYPE_P:
        case X264_TYPE_B:
        case X264_TYPE_BREF:
            encoded_images_[i]._frameType = kVideoFrameDelta;
            break;
        default:
            encoded_images_[i]._frameType = kEmptyFrame;
            break;
        }
#endif
        //        std::cout<<"i: "<<i<<",configurations_[i].simulcast_idx: "<<configurations_[i].simulcast_idx<<std::endl;
//        std::cout<<"pic_out.i_type:"<<pic_out.i_type<<",encoded_images_[i]._frameType:"<<encoded_images_[i]._frameType<<std::endl;

        // Split encoded image up into fragments. This also updates |encoded_image_|.
        RTPFragmentationHeader frag_header;
        RtpFragmentize(&encoded_images_[i], *frame_buffer, encoders_[i], &frag_header,nNal);
        api_get_array_free(encoders_[i]->nal_size);
        if(logfp_)
        {
            fprintf(logfp_,"Encode: end \n");
            fflush(logfp_);
        }
        // Encoder can skip frames to save bandwidth in which case |encoded_images_[i]._length| == 0.
        if (encoded_images_[i].size() > 0) {
            // Parse QP.这两个方法计算编码帧的正确QP送出去。 平均QP一直居高不下（超过我们上面提到的QP阈值上限），所以一直在不断地请求下调分辨率
            h264_bitstream_parser_.ParseBitstream(encoded_images_[i].data(),encoded_images_[i].size());
            h264_bitstream_parser_.GetLastSliceQp(&encoded_images_[i].qp_);
            // Deliver encoded image. encoders_[i]->nal->i_type
            CodecSpecificInfo codec_specific;
            codec_specific.codecType = kVideoCodecH264;
            codec_specific.codecSpecific.H264.packetization_mode =packetization_mode_;
            codec_specific.codecSpecific.H264.temporal_idx = kNoTemporalIdx;
            ///codec_specific.codecSpecific.H264.idr_frame = encoders_[i]->nal->i_type== NAL_SLICE_IDR;//修改的判断
            codec_specific.codecSpecific.H264.base_layer_sync = false;
            if (num_temporal_layers_ > 1) {
                const uint8_t tid = 0;//从编码信息获取时间域的id  openh264 :info.sLayerInfo[0].uiTemporalId
                codec_specific.codecSpecific.H264.temporal_idx = tid;
                codec_specific.codecSpecific.H264.base_layer_sync =
                        tid > 0 && tid < tl0sync_limit_;
                if (codec_specific.codecSpecific.H264.base_layer_sync) {
                    tl0sync_limit_ = tid;
                }
                if (tid == 0) {
                    tl0sync_limit_ = num_temporal_layers_;
                }
            }
            //一帧数据进行缩放编码 实现simulcast //
            //编译完一帧发送一帧
            encoded_image_callback_->OnEncodedImage(encoded_images_[i],
                                                    &codec_specific, &frag_header);
        }
    }
#endif
    return WEBRTC_VIDEO_CODEC_OK;
}

// Initialization parameters.
// There are two ways to initialize. There is SEncParamBase (cleared with
// memset(&p, 0, sizeof(SEncParamBase)) used in Initialize, and SEncParamExt
// which is a superset of SEncParamBase (cleared with GetDefaultParams) used
// in InitializeExt.
//SEncParamExt X264EncoderImpl::CreateEncoderParams(size_t i) const {
//  SEncParamExt encoder_params;
//  encoders_[i]->GetDefaultParams(&encoder_params);
//  if (codec_.mode == VideoCodecMode::kRealtimeVideo) {
//    encoder_params.iUsageType = CAMERA_VIDEO_REAL_TIME;
//  } else if (codec_.mode == VideoCodecMode::kScreensharing) {
//    encoder_params.iUsageType = SCREEN_CONTENT_REAL_TIME;
//  } else {
//    RTC_NOTREACHED();
//  }
//  encoder_params.iPicWidth = configurations_[i].width;
//  encoder_params.iPicHeight = configurations_[i].height;
//  encoder_params.iTargetBitrate = configurations_[i].target_bps;
//  encoder_params.iMaxBitrate = configurations_[i].max_bps;
//  // Rate Control mode
//  encoder_params.iRCMode = RC_BITRATE_MODE;
//  encoder_params.fMaxFrameRate = configurations_[i].max_frame_rate;

//  // The following parameters are extension parameters (they're in SEncParamExt,
//  // not in SEncParamBase).
//  encoder_params.bEnableFrameSkip = configurations_[i].frame_dropping_on;
//  // |uiIntraPeriod|    - multiple of GOP size
//  // |keyFrameInterval| - number of frames
//  encoder_params.uiIntraPeriod = configurations_[i].key_frame_interval;
//  encoder_params.uiMaxNalSize = 0;
//  // Threading model: use auto.
//  //  0: auto (dynamic imp. internal encoder)
//  //  1: single thread (default value)
//  // >1: number of threads
//  encoder_params.iMultipleThreadIdc = NumberOfThreads(
//      encoder_params.iPicWidth, encoder_params.iPicHeight, number_of_cores_);
//  // The base spatial layer 0 is the only one we use.
//  encoder_params.sSpatialLayers[0].iVideoWidth = encoder_params.iPicWidth;
//  encoder_params.sSpatialLayers[0].iVideoHeight = encoder_params.iPicHeight;
//  encoder_params.sSpatialLayers[0].fFrameRate = encoder_params.fMaxFrameRate;
//  encoder_params.sSpatialLayers[0].iSpatialBitrate =
//      encoder_params.iTargetBitrate;
//  encoder_params.sSpatialLayers[0].iMaxSpatialBitrate =
//      encoder_params.iMaxBitrate;
//  encoder_params.iTemporalLayerNum = num_temporal_layers_;
//  if (encoder_params.iTemporalLayerNum > 1) {
//    encoder_params.iNumRefFrame = 1;
//  }
//  RTC_LOG(INFO) << "OpenH264 version is " << OPENH264_MAJOR << "."
//                << OPENH264_MINOR;
//  switch (packetization_mode_) {
//    case H264PacketizationMode::SingleNalUnit:
//      // Limit the size of the packets produced.
//      encoder_params.sSpatialLayers[0].sSliceArgument.uiSliceNum = 1;
//      encoder_params.sSpatialLayers[0].sSliceArgument.uiSliceMode =
//          SM_SIZELIMITED_SLICE;
//      encoder_params.sSpatialLayers[0].sSliceArgument.uiSliceSizeConstraint =
//          static_cast<unsigned int>(max_payload_size_);
//      RTC_LOG(INFO) << "Encoder is configured with NALU constraint: "
//                    << max_payload_size_ << " bytes";
//      break;
//    case H264PacketizationMode::NonInterleaved:
//      // When uiSliceMode = SM_FIXEDSLCNUM_SLICE, uiSliceNum = 0 means auto
//      // design it with cpu core number.
//      // TODO(sprang): Set to 0 when we understand why the rate controller borks
//      //               when uiSliceNum > 1.
//      encoder_params.sSpatialLayers[0].sSliceArgument.uiSliceNum = 1;
//      encoder_params.sSpatialLayers[0].sSliceArgument.uiSliceMode =
//          SM_FIXEDSLCNUM_SLICE;
//      break;
//  }
//  return encoder_params;
//}

void HCSVCEncoderImpl::ReportInit() {
    SimgpleLog log(__func__);
    if (has_reported_init_)
        return;
    RTC_HISTOGRAM_ENUMERATION("WebRTC.Video.HCSVCEncoderImpl.Event",
                              kH264EncoderEventInit, kH264EncoderEventMax);
    has_reported_init_ = true;
}

void HCSVCEncoderImpl::ReportError() {
    SimgpleLog log(__func__);
    if (has_reported_error_)
        return;
    RTC_HISTOGRAM_ENUMERATION("WebRTC.Video.HCSVCEncoderImpl.Event",
                              kH264EncoderEventError, kH264EncoderEventMax);
    has_reported_error_ = true;
}

VideoEncoder::EncoderInfo HCSVCEncoderImpl::GetEncoderInfo() const {
    EncoderInfo info;
    info.supports_native_handle = false;
    info.implementation_name = "libhcsvc";
    info.scaling_settings =
            VideoEncoder::ScalingSettings(kLowH264QpThreshold, kHighH264QpThreshold);
    info.is_hardware_accelerated = false;
    info.has_internal_source = false;
    return info;
}
#if 0
int HCSVCEncoderImpl::x264_param_apply_preset(x264_param_t *param, const char *preset)
{
    char *end;
    int i = strtol( preset, &end, 10 );
    if( *end == 0 && i >= 0 && i < (int)(sizeof(x264_preset_names)/sizeof(*x264_preset_names)-1) )
        preset = x264_preset_names[i];

    if( !strcasecmp( preset, "ultrafast" ) )
    {
        param->i_frame_reference = 1;
        param->i_scenecut_threshold = 0;
        param->b_deblocking_filter = 0;
        param->b_cabac = 0;
        param->i_bframe = 0;
        param->i_keyint_max = 240;//30帧里面有一个I帧
        param->i_keyint_min = 24;
        param->b_repeat_headers = 1;
        param->analyse.intra = 0;
        param->analyse.inter = 0;
        param->analyse.b_transform_8x8 = 0;
        param->analyse.i_me_method = X264_ME_DIA;
        param->analyse.i_subpel_refine = 0;
        param->rc.i_aq_mode = 0;
        param->analyse.b_mixed_references = 0;
        param->analyse.i_trellis = 0;
        param->i_bframe_adaptive = X264_B_ADAPT_NONE;
        param->rc.b_mb_tree = 0;
        param->analyse.i_weighted_pred = X264_WEIGHTP_NONE;
        param->analyse.b_weighted_bipred = 0;
        param->rc.i_lookahead = 0;
    }
    else if( !strcasecmp( preset, "superfast" ) )
    {
        param->analyse.inter = X264_ANALYSE_I8x8|X264_ANALYSE_I4x4;
        param->analyse.i_me_method = X264_ME_DIA;
        param->analyse.i_subpel_refine = 1;
        param->i_frame_reference = 1;
        param->analyse.b_mixed_references = 0;
        param->analyse.i_trellis = 0;
        param->rc.b_mb_tree = 0;
        param->analyse.i_weighted_pred = X264_WEIGHTP_SIMPLE;
        param->rc.i_lookahead = 0;
    }
    else if( !strcasecmp( preset, "veryfast" ) )
    {
        param->analyse.i_me_method = X264_ME_HEX;
        param->analyse.i_subpel_refine = 2;
        param->i_frame_reference = 1;
        param->analyse.b_mixed_references = 0;
        param->analyse.i_trellis = 0;
        param->analyse.i_weighted_pred = X264_WEIGHTP_SIMPLE;
        param->rc.i_lookahead = 10;
    }
    else if( !strcasecmp( preset, "faster" ) )
    {
        param->analyse.b_mixed_references = 0;
        param->i_frame_reference = 2;
        param->analyse.i_subpel_refine = 4;
        param->analyse.i_weighted_pred = X264_WEIGHTP_SIMPLE;
        param->rc.i_lookahead = 20;
    }
    else if( !strcasecmp( preset, "fast" ) )
    {
        param->i_frame_reference = 2;
        param->analyse.i_subpel_refine = 6;
        param->analyse.i_weighted_pred = X264_WEIGHTP_SIMPLE;
        param->rc.i_lookahead = 30;
    }
    else if( !strcasecmp( preset, "medium" ) )
    {
        /* Default is medium */
    }
    else if( !strcasecmp( preset, "slow" ) )
    {
        param->analyse.i_me_method = X264_ME_UMH;
        param->analyse.i_subpel_refine = 8;
        param->i_frame_reference = 5;
        param->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;
        param->analyse.i_direct_mv_pred = X264_DIRECT_PRED_AUTO;
        param->rc.i_lookahead = 50;
    }
    else if( !strcasecmp( preset, "slower" ) )
    {
        param->analyse.i_me_method = X264_ME_UMH;
        param->analyse.i_subpel_refine = 9;
        param->i_frame_reference = 8;
        param->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;
        param->analyse.i_direct_mv_pred = X264_DIRECT_PRED_AUTO;
        param->analyse.inter |= X264_ANALYSE_PSUB8x8;
        param->analyse.i_trellis = 2;
        param->rc.i_lookahead = 60;
    }
    else if( !strcasecmp( preset, "veryslow" ) )
    {
        param->analyse.i_me_method = X264_ME_UMH;
        param->analyse.i_subpel_refine = 10;
    }
    else
    {
        return -1;
    }

    return 0;
}
#endif
//可以增加参数 进行不同的设置。
void HCSVCEncoderImpl::compress_begin(HCSVCEncoderImpl::m_Encoder_ *en, LayerConfig *config)
{
    if(logfp_)
    {
        fprintf(logfp_,"compress_begin: start \n");
        fflush(logfp_);
    }
    config->json = api_renew_json_int(config->json, "seqnum", config->seqnum);
    config->json = api_renew_json_int(config->json, "enable_fec", config->enable_fec);
    config->json = api_renew_json_int(config->json, "refresh_idr", config->refresh_idr);
    config->json = api_renew_json_int(config->json, "width", config->width);
    config->json = api_renew_json_int(config->json, "height", config->height);
    config->json = api_renew_json_int(config->json, "bit_rate", config->bit_rate);
    config->json = api_renew_json_int(config->json, "gop_size", config->gop_size);
    config->json = api_renew_json_str(config->json, "preset", "superfast");
    config->json = api_renew_json_str(config->json, "tune", "zerolatency");
    if(config->mtu_size > 0)
    {
        char ctmp[32] = "";
        sprintf(ctmp, "%d", config->mtu_size);
        config->json = api_renew_json_str(config->json, "slice-max-size", ctmp);
        config->json = api_renew_json_int(config->json, "mtu_size", config->mtu_size);
    }
#if 1
    config->json = api_renew_json_int(config->json, "tff", 0);
    config->json = api_renew_json_int(config->json, "bff", 0);
    config->json = api_renew_json_int(config->json, "qmin", 20);
    config->json = api_renew_json_int(config->json, "qmax", 48);//
    config->json = api_renew_json_int(config->json, "qp", config->qp);
    config->json = api_renew_json_int(config->json, "max_b_frames", 0);
    config->json = api_renew_json_int(config->json, "coder_type", 2);// 1:cavlc/2:cabac
    config->json = api_renew_json_int(config->json, "refs", config->refs);
    config->json = api_renew_json_int(config->json, "frame_rate", config->frame_rate);
    config->json = api_renew_json_int(config->json, "thread_type", 1);//FF_THREAD_FRAME: 1 FF_THREAD_SLICE: 2
    config->json = api_renew_json_int(config->json, "thread_count", 1);
#endif
    char* jsonstr = api_json2str(config->json);
    if(logfp_)
    {
        fprintf(logfp_,"compress_begin: jsonstr= %s \n", jsonstr);
        fflush(logfp_);
    }
    api_video_encode_open(en->obj_id, jsonstr);
    api_json2str_free(jsonstr);
    if(logfp_)
    {
        fprintf(logfp_,"compress_begin: end \n");
        fflush(logfp_);
    }
}

#if 0
int HCSVCEncoderImpl::compress_frame(HCSVCEncoderImpl::m_Encoder_ *en, int type, uint8_t *in, uint8_t *out)
{
    //    std::cout<<"compress_frame"<<std::endl;
    int result = 0;
#if 0
    x264_picture_t pic_out;
    int nNal = 0;

    int i = 0 , j = 0 ;
    uint8_t *p_out = out;
    en->nal=NULL;
    uint8_t *p422;

    int ysize = en->param->i_width * en->param->i_height;
    uint8_t *y =  in;
    uint8_t *u = in + ysize;
    uint8_t *v =  u+ ysize/4;

    //不拷贝 直接赋值指针.
    memcpy(en->picture->img.plane[0],y,ysize);
    memcpy (en->picture->img.plane[1],u,ysize/4);
    memcpy (en->picture->img.plane[2],v,ysize/4);
    switch (type) {
    case 0:
        en->picture->i_type = X264_TYPE_P;
        break;
    case 1:
        en->picture->i_type = X264_TYPE_IDR;
        break;
    case 2:
        en->picture->i_type = X264_TYPE_I;
        break;
    default:
        en->picture->i_type = X264_TYPE_AUTO;
        break;
    }

    if (x264_encoder_encode(en->handle, &(en->nal), &nNal, en->picture,
                            &pic_out) < 0) {
        return -1;
    }
    en->picture->i_pts++;
    for (i = 0; i < nNal; i++) {
        memcpy(p_out, en->nal[i].p_payload, en->nal[i].i_payload);
        p_out += en->nal[i].i_payload;
        result += en->nal[i].i_payload;
    }
#endif
    //
    return result;
}
#endif
void HCSVCEncoderImpl::compress_end(HCSVCEncoderImpl::m_Encoder_ *en)
{
#if 1
    api_video_encode_close(en->obj_id);
    if(en->inbuffer)
    {
        delete en->inbuffer;
        en->inbuffer = NULL;
    }
    if(en->outbuffer)
    {
        delete en->outbuffer;
        en->outbuffer = NULL;
    }
#else
    if (en->picture) {
        //        x264_picture_clean(en->picture);
        free(en->picture);
        en->picture = 0;
    }
    if (en->param) {
        free(en->param);
        en->param = 0;
    }
    if (en->handle) {
        x264_encoder_close(en->handle);
    }
    free(en);
#endif
}

void HCSVCEncoderImpl::LayerConfig::SetStreamState(bool send_stream) {
    if (send_stream && !sending) {
        // Need a key frame if we have not sent this stream before.
        key_frame_request = true;
    }
    sending = send_stream;
}

}  // namespace webrtc

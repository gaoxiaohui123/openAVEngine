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

#ifndef HCSVC_MODULES_VIDEO_CODING_CODECS_H264_H264_ENCODER_IMPL_H_
#define HCSVC_MODULES_VIDEO_CODING_CODECS_H264_H264_ENCODER_IMPL_H_

#include <memory>
#include <vector>
#include <stdlib.h>
#include "api/video/i420_buffer.h"
#include "common_video/h264/h264_bitstream_parser.h"
#include "modules/video_coding/codecs/h264/include/h264.h"
#include "modules/video_coding/utility/quality_scaler.h"

//#include "third_party/openh264/src/codec/api/svc/codec_app_def.h"
//#include "third_party/x264/x264.h"
#include "third_party/hcsvc/hcsvc.h"
//#include "/home/yang/datashare/jy/py2c/webrtc/hcsvc.h"

//class ISVCEncoder;

namespace webrtc {
class HCSVCEncoderImpl;
class HCSVCEncoderImpl :   public H264Encoder {
public:
    struct LayerConfig {
        int simulcast_idx = 0;
        int width = -1;
        int height = -1;
        bool sending = true;
        bool key_frame_request = false;
        float max_frame_rate = 0;
        uint32_t target_bps = 0;
        uint32_t max_bps = 0;
        bool frame_dropping_on = false;
        int key_frame_interval = 0;
        //
        unsigned short seqnum = 0;//  # can be changed by fec
        int enable_fec = 0;
        int refresh_idr = 1;
        float lost_rate = 0.3;
        float code_rate = (1 - lost_rate);
        int max_refs = 16;
        int mtu_size = 1400;
        int frame_rate = 25;
        int gop_size = frame_rate << 1;
        int qp = 26;
        int refs = 4;
        int bit_rate = 640 * 1024;
        void *json = NULL;
        //
        void SetStreamState(bool send_stream);
    };

public:
    explicit HCSVCEncoderImpl(const cricket::VideoCodec& codec);
    ~HCSVCEncoderImpl() override;

    // |max_payload_size| is ignored.
    // The following members of |codec_settings| are used. The rest are ignored.
    // - codecType (must be kVideoCodecH264)
    // - targetBitrate
    // - maxFramerate
    // - width
    // - height
    int32_t InitEncode(const VideoCodec* codec_settings,
                       int32_t number_of_cores,
                       size_t max_payload_size) override;
    int32_t Release() override;

    int32_t RegisterEncodeCompleteCallback(
            EncodedImageCallback* callback) override;
    int32_t SetRateAllocation(const VideoBitrateAllocation& bitrate_allocation,
                              uint32_t framerate) override;

    // The result of encoding - an EncodedImage and RTPFragmentationHeader - are
    // passed to the encode complete callback.
    int32_t Encode(const VideoFrame& frame,
                   const CodecSpecificInfo* codec_specific_info,
                   const std::vector<FrameType>* frame_types) override;

    EncoderInfo GetEncoderInfo() const override;

    // Exposed for testing.
    H264PacketizationMode PacketizationModeForTesting() const {
        return packetization_mode_;
    }

    typedef struct {
        //x264_param_t *param;
        //x264_t *handle;
        //x264_picture_t *picture; //说明一个视频序列中每帧特点
        //x264_nal_t *nal;
        char obj_id = 0;
        int frame_idx = 0;
        char *outparam[4];
        int* nal_size;
        uint8_t *inbuffer = NULL;
        uint8_t * outbuffer = NULL;
        std::string  _saveFile;
        FILE * _test=nullptr;
    } m_Encoder_;
private:
    //=========h264的实现方法.


    //int x264_param_apply_preset( x264_param_t *param, const char *preset );
    //初始化编码器，并返回一个编码器对象
    void compress_begin(m_Encoder_ *en, LayerConfig *config);
    //编码一帧
    //int compress_frame(m_Encoder_ *en, int type, uint8_t *in, uint8_t *out);
    //int compress_frame(Encoder *en, int type, uint8_t *yin, uint8_t *u,uint8_t *v,uint8_t *out);

    //释放内存
    void compress_end(m_Encoder_ *en);


private:
//    SEncParamExt CreateEncoderParams(size_t i) const;

    webrtc::H264BitstreamParser h264_bitstream_parser_;
    // Reports statistics with histograms.
    void ReportInit();
    void ReportError();

//    std::vector<ISVCEncoder*> encoders_;
    std::vector<m_Encoder_ *> encoders_;
//    std::vector<SSourcePicture> pictures_;
    //HCNOTE 定义x264的input pictrues
    std::vector<rtc::scoped_refptr<I420Buffer>> downscaled_buffers_;
    std::vector<LayerConfig> configurations_;
    std::vector<EncodedImage> encoded_images_;

    VideoCodec codec_;
    H264PacketizationMode packetization_mode_;
    size_t max_payload_size_;
    int32_t number_of_cores_;
    EncodedImageCallback* encoded_image_callback_;

    bool has_reported_init_;
    bool has_reported_error_;

    int num_temporal_layers_;
    uint8_t tl0sync_limit_;
    FILE * logfp_ = nullptr;
    long long start_time_ = 0;//gxh
};

}  // namespace webrtc

#endif  // HCSVC_MODULES_VIDEO_CODING_CODECS_H264_H264_ENCODER_IMPL_H_

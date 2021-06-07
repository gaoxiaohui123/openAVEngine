#!/bin/sh

echo 'input a number'
read Num
case $Num in
1)
    #for uos
    SRC_ROOT=/home/gxh/works/huichang/svc_webrtc/bizconf_rtc_endpoint/webrtc-checkout/src/out/linux_openssl/obj/
    DST_ROOT=`pwd`/../ffmpeg-linux/mylib/

    ar rcs $DST_ROOT/libwebrtc.a \
    $SRC_ROOT/api/video/video_frame_i420/i420_buffer.o \
    $SRC_ROOT/api/video/video_frame_i420/i420_buffer.dwo \
    $SRC_ROOT/api/video/video_frame/color_space.o \
    $SRC_ROOT/api/video/video_frame/color_space.dwo \
    $SRC_ROOT/api/video/video_frame/hdr_metadata.o \
    $SRC_ROOT/api/video/video_frame/hdr_metadata.dwo \
    $SRC_ROOT/api/video/video_frame/video_content_type.o \
    $SRC_ROOT/api/video/video_frame/video_content_type.dwo \
    $SRC_ROOT/api/video/video_frame/video_frame_buffer.o \
    $SRC_ROOT/api/video/video_frame/video_frame_buffer.dwo \
    $SRC_ROOT/api/video/video_frame/video_frame.o \
    $SRC_ROOT/api/video/video_frame/video_frame.dwo \
    $SRC_ROOT/api/video/video_frame/video_source_interface.o \
    $SRC_ROOT/api/video/video_frame/video_source_interface.dwo \
    $SRC_ROOT/api/video/video_frame/video_timing.o \
    $SRC_ROOT/api/video/video_frame/video_timing.dwo \
    $SRC_ROOT/common_video/common_video/i420_buffer_pool.o \
    $SRC_ROOT/common_video/common_video/i420_buffer_pool.dwo \
    $SRC_ROOT/modules/video_processing/video_processing/denoiser_filter_c.o \
    $SRC_ROOT/modules/video_processing/video_processing/denoiser_filter_c.dwo \
    $SRC_ROOT/modules/video_processing/video_processing/denoiser_filter.o \
    $SRC_ROOT/modules/video_processing/video_processing/denoiser_filter.dwo \
    $SRC_ROOT/modules/video_processing/video_processing/noise_estimation.o \
    $SRC_ROOT/modules/video_processing/video_processing/noise_estimation.dwo \
    $SRC_ROOT/modules/video_processing/video_processing/skin_detection.o \
    $SRC_ROOT/modules/video_processing/video_processing/skin_detection.dwo \
    $SRC_ROOT/modules/video_processing/video_processing/video_denoiser.o \
    $SRC_ROOT/modules/video_processing/video_processing/video_denoiser.dwo \
    $SRC_ROOT/modules/video_processing/video_processing_sse2/denoiser_filter_sse2.o \
    $SRC_ROOT/modules/video_processing/video_processing_sse2/denoiser_filter_sse2.dwo \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/compare_common.o \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/compare_common.dwo \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/compare_gcc.o \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/compare_gcc.dwo \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/compare.o \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/compare.dwo \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/convert_argb.o \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/convert_argb.dwo \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/convert_from_argb.o \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/convert_from_argb.dwo \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/convert_from.o \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/convert_from.dwo \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/convert_jpeg.o \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/convert_jpeg.dwo \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/convert_to_argb.o \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/convert_to_argb.dwo \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/convert_to_i420.o \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/convert_to_i420.dwo \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/convert.o \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/convert.dwo \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/cpu_id.o \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/cpu_id.dwo \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/mjpeg_decoder.o \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/mjpeg_decoder.dwo \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/mjpeg_validate.o \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/mjpeg_validate.dwo \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/planar_functions.o \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/planar_functions.dwo \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/rotate_any.o \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/rotate_any.dwo \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/rotate_argb.o \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/rotate_argb.dwo \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/rotate_common.o \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/rotate_common.dwo \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/rotate_gcc.o \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/rotate_gcc.dwo \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/rotate.o \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/rotate.dwo \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/row_any.o \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/row_any.dwo \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/row_common.o \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/row_common.dwo \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/row_gcc.o \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/row_gcc.dwo \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/scale_any.o \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/scale_any.dwo \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/scale_argb.o \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/scale_argb.dwo \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/scale_common.o \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/scale_common.dwo \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/scale_gcc.o \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/scale_gcc.dwo \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/scale.o \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/scale.dwo \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/video_common.o \
    $SRC_ROOT/third_party/libyuv/libyuv_internal/video_common.dwo \
    $SRC_ROOT/rtc_base/memory/aligned_malloc/aligned_malloc.o \
    $SRC_ROOT/rtc_base/memory/aligned_malloc/aligned_malloc.dwo \
    $SRC_ROOT/rtc_base/checks/checks.o \
    $SRC_ROOT/rtc_base/checks/checks.dwo \
    $SRC_ROOT/rtc_base/rtc_base_approved/race_checker.o \
    $SRC_ROOT/rtc_base/rtc_base_approved/race_checker.dwo \
    $SRC_ROOT/rtc_base/platform_thread_types/platform_thread_types.o \
    $SRC_ROOT/rtc_base/platform_thread_types/platform_thread_types.dwo \
    $SRC_ROOT/modules/audio_processing/aec/aec/aec_resampler.o \
    $SRC_ROOT/modules/audio_processing/aec/aec/aec_resampler.dwo \
    $SRC_ROOT/modules/audio_processing/aec/aec/echo_cancellation.o \
    $SRC_ROOT/modules/audio_processing/aec/aec/echo_cancellation.dwo \
    $SRC_ROOT/modules/audio_processing/aec/aec_core/aec_core_sse2.o \
    $SRC_ROOT/modules/audio_processing/aec/aec_core/aec_core_sse2.dwo \
    $SRC_ROOT/modules/audio_processing/aec/aec_core/aec_core.o \
    $SRC_ROOT/modules/audio_processing/aec/aec_core/aec_core.dwo \
    $SRC_ROOT/modules/audio_processing/aec_dump/null_aec_dump_factory/null_aec_dump_factory.o \
    $SRC_ROOT/modules/audio_processing/aec_dump/null_aec_dump_factory/null_aec_dump_factory.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/adaptive_fir_filter.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/adaptive_fir_filter.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/aec_state.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/aec_state.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/aec3_common.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/aec3_common.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/aec3_fft.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/aec3_fft.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/api_call_jitter_metrics.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/api_call_jitter_metrics.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/block_delay_buffer.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/block_delay_buffer.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/block_framer.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/block_framer.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/block_processor_metrics.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/block_processor_metrics.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/block_processor.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/block_processor.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/cascaded_biquad_filter.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/cascaded_biquad_filter.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/clockdrift_detector.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/clockdrift_detector.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/comfort_noise_generator.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/comfort_noise_generator.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/decimator.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/decimator.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/downsampled_render_buffer.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/downsampled_render_buffer.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/echo_audibility.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/echo_audibility.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/echo_canceller3.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/echo_canceller3.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/echo_path_delay_estimator.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/echo_path_delay_estimator.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/echo_path_variability.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/echo_path_variability.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/echo_remover_metrics.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/echo_remover_metrics.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/echo_remover.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/echo_remover.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/erl_estimator.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/erl_estimator.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/erle_estimator.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/erle_estimator.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/fft_buffer.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/fft_buffer.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/filter_analyzer.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/filter_analyzer.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/frame_blocker.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/frame_blocker.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/fullband_erle_estimator.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/fullband_erle_estimator.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/main_filter_update_gain.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/main_filter_update_gain.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/matched_filter_lag_aggregator.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/matched_filter_lag_aggregator.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/matched_filter.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/matched_filter.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/matrix_buffer.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/matrix_buffer.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/moving_average.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/moving_average.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/render_buffer.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/render_buffer.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/render_delay_buffer.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/render_delay_buffer.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/render_delay_controller_metrics.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/render_delay_controller_metrics.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/render_delay_controller.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/render_delay_controller.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/render_reverb_model.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/render_reverb_model.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/render_signal_analyzer.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/render_signal_analyzer.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/residual_echo_estimator.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/residual_echo_estimator.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/reverb_decay_estimator.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/reverb_decay_estimator.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/reverb_frequency_response.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/reverb_frequency_response.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/reverb_model_estimator.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/reverb_model_estimator.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/reverb_model_fallback.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/reverb_model_fallback.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/reverb_model.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/reverb_model.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/shadow_filter_update_gain.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/shadow_filter_update_gain.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/signal_dependent_erle_estimator.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/signal_dependent_erle_estimator.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/skew_estimator.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/skew_estimator.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/stationarity_estimator.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/stationarity_estimator.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/subband_erle_estimator.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/subband_erle_estimator.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/subtractor_output_analyzer.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/subtractor_output_analyzer.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/subtractor_output.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/subtractor_output.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/subtractor.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/subtractor.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/suppression_filter.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/suppression_filter.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/suppression_gain.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/suppression_gain.dwo \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/vector_buffer.o \
    $SRC_ROOT/modules/audio_processing/aec3/aec3/vector_buffer.dwo \
    $SRC_ROOT/modules/audio_processing/aecm/aecm_core/aecm_core_c.o \
    $SRC_ROOT/modules/audio_processing/aecm/aecm_core/aecm_core_c.dwo \
    $SRC_ROOT/modules/audio_processing/aecm/aecm_core/aecm_core.o \
    $SRC_ROOT/modules/audio_processing/aecm/aecm_core/aecm_core.dwo \
    $SRC_ROOT/modules/audio_processing/aecm/aecm_core/echo_control_mobile.o \
    $SRC_ROOT/modules/audio_processing/aecm/aecm_core/echo_control_mobile.dwo \
    $SRC_ROOT/modules/audio_processing/agc/agc/agc_manager_direct.o \
    $SRC_ROOT/modules/audio_processing/agc/agc/agc_manager_direct.dwo \
    $SRC_ROOT/modules/audio_processing/agc/agc_legacy_c/analog_agc.o \
    $SRC_ROOT/modules/audio_processing/agc/agc_legacy_c/analog_agc.dwo \
    $SRC_ROOT/modules/audio_processing/agc/agc_legacy_c/digital_agc.o \
    $SRC_ROOT/modules/audio_processing/agc/agc_legacy_c/digital_agc.dwo \
    $SRC_ROOT/modules/audio_processing/agc/level_estimation/agc.o \
    $SRC_ROOT/modules/audio_processing/agc/level_estimation/agc.dwo \
    $SRC_ROOT/modules/audio_processing/agc/level_estimation/loudness_histogram.o \
    $SRC_ROOT/modules/audio_processing/agc/level_estimation/loudness_histogram.dwo \
    $SRC_ROOT/modules/audio_processing/agc/level_estimation/utility.o \
    $SRC_ROOT/modules/audio_processing/agc/level_estimation/utility.dwo \
    $SRC_ROOT/modules/audio_processing/agc2/adaptive_digital/adaptive_agc.o \
    $SRC_ROOT/modules/audio_processing/agc2/adaptive_digital/adaptive_agc.dwo \
    $SRC_ROOT/modules/audio_processing/agc2/adaptive_digital/adaptive_digital_gain_applier.o \
    $SRC_ROOT/modules/audio_processing/agc2/adaptive_digital/adaptive_digital_gain_applier.dwo \
    $SRC_ROOT/modules/audio_processing/agc2/adaptive_digital/adaptive_mode_level_estimator.o \
    $SRC_ROOT/modules/audio_processing/agc2/adaptive_digital/adaptive_mode_level_estimator.dwo \
    $SRC_ROOT/modules/audio_processing/agc2/adaptive_digital/saturation_protector.o \
    $SRC_ROOT/modules/audio_processing/agc2/adaptive_digital/saturation_protector.dwo \
    $SRC_ROOT/modules/audio_processing/agc2/biquad_filter/biquad_filter.o \
    $SRC_ROOT/modules/audio_processing/agc2/biquad_filter/biquad_filter.dwo \
    $SRC_ROOT/modules/audio_processing/agc2/common/agc2_common.o \
    $SRC_ROOT/modules/audio_processing/agc2/common/agc2_common.dwo \
    $SRC_ROOT/modules/audio_processing/agc2/fixed_digital/fixed_digital_level_estimator.o \
    $SRC_ROOT/modules/audio_processing/agc2/fixed_digital/fixed_digital_level_estimator.dwo \
    $SRC_ROOT/modules/audio_processing/agc2/fixed_digital/interpolated_gain_curve.o \
    $SRC_ROOT/modules/audio_processing/agc2/fixed_digital/interpolated_gain_curve.dwo \
    $SRC_ROOT/modules/audio_processing/agc2/fixed_digital/limiter.o \
    $SRC_ROOT/modules/audio_processing/agc2/fixed_digital/limiter.dwo \
    $SRC_ROOT/modules/audio_processing/agc2/gain_applier/gain_applier.o \
    $SRC_ROOT/modules/audio_processing/agc2/gain_applier/gain_applier.dwo \
    $SRC_ROOT/modules/audio_processing/agc2/level_estimation_agc/adaptive_mode_level_estimator_agc.o \
    $SRC_ROOT/modules/audio_processing/agc2/level_estimation_agc/adaptive_mode_level_estimator_agc.dwo \
    $SRC_ROOT/modules/audio_processing/agc2/noise_level_estimator/down_sampler.o \
    $SRC_ROOT/modules/audio_processing/agc2/noise_level_estimator/down_sampler.dwo \
    $SRC_ROOT/modules/audio_processing/agc2/noise_level_estimator/noise_level_estimator.o \
    $SRC_ROOT/modules/audio_processing/agc2/noise_level_estimator/noise_level_estimator.dwo \
    $SRC_ROOT/modules/audio_processing/agc2/noise_level_estimator/noise_spectrum_estimator.o \
    $SRC_ROOT/modules/audio_processing/agc2/noise_level_estimator/noise_spectrum_estimator.dwo \
    $SRC_ROOT/modules/audio_processing/agc2/noise_level_estimator/signal_classifier.o \
    $SRC_ROOT/modules/audio_processing/agc2/noise_level_estimator/signal_classifier.dwo \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/rnn_vad/features_extraction.o \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/rnn_vad/features_extraction.dwo \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/rnn_vad/fft_util.o \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/rnn_vad/fft_util.dwo \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/rnn_vad/lp_residual.o \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/rnn_vad/lp_residual.dwo \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/rnn_vad/pitch_search_internal.o \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/rnn_vad/pitch_search_internal.dwo \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/rnn_vad/pitch_search.o \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/rnn_vad/pitch_search.dwo \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/rnn_vad/rnn.o \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/rnn_vad/rnn.dwo \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/rnn_vad/spectral_features_internal.o \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/rnn_vad/spectral_features_internal.dwo \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/rnn_vad/spectral_features.o \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/rnn_vad/spectral_features.dwo \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad_with_level/vad_with_level.o \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad_with_level/vad_with_level.dwo \
    $SRC_ROOT/modules/audio_processing/api/audio_processing.o \
    $SRC_ROOT/modules/audio_processing/api/audio_processing.dwo \
    $SRC_ROOT/modules/audio_processing/apm_logging/apm_data_dumper.o \
    $SRC_ROOT/modules/audio_processing/apm_logging/apm_data_dumper.dwo \
    $SRC_ROOT/modules/audio_processing/audio_buffer/audio_buffer.o \
    $SRC_ROOT/modules/audio_processing/audio_buffer/audio_buffer.dwo \
    $SRC_ROOT/modules/audio_processing/audio_buffer/splitting_filter.o \
    $SRC_ROOT/modules/audio_processing/audio_buffer/splitting_filter.dwo \
    $SRC_ROOT/modules/audio_processing/audio_buffer/three_band_filter_bank.o \
    $SRC_ROOT/modules/audio_processing/audio_buffer/three_band_filter_bank.dwo \
    $SRC_ROOT/modules/audio_processing/audio_processing/aec_dump.o \
    $SRC_ROOT/modules/audio_processing/audio_processing/aec_dump.dwo \
    $SRC_ROOT/modules/audio_processing/audio_processing/audio_processing_impl.o \
    $SRC_ROOT/modules/audio_processing/audio_processing/audio_processing_impl.dwo \
    $SRC_ROOT/modules/audio_processing/audio_processing/circular_buffer.o \
    $SRC_ROOT/modules/audio_processing/audio_processing/circular_buffer.dwo \
    $SRC_ROOT/modules/audio_processing/audio_processing/echo_cancellation_impl.o \
    $SRC_ROOT/modules/audio_processing/audio_processing/echo_cancellation_impl.dwo \
    $SRC_ROOT/modules/audio_processing/audio_processing/echo_control_mobile_impl.o \
    $SRC_ROOT/modules/audio_processing/audio_processing/echo_control_mobile_impl.dwo \
    $SRC_ROOT/modules/audio_processing/audio_processing/gain_control_for_experimental_agc.o \
    $SRC_ROOT/modules/audio_processing/audio_processing/gain_control_for_experimental_agc.dwo \
    $SRC_ROOT/modules/audio_processing/audio_processing/gain_control_impl.o \
    $SRC_ROOT/modules/audio_processing/audio_processing/gain_control_impl.dwo \
    $SRC_ROOT/modules/audio_processing/audio_processing/gain_controller2.o \
    $SRC_ROOT/modules/audio_processing/audio_processing/gain_controller2.dwo \
    $SRC_ROOT/modules/audio_processing/audio_processing/level_estimator_impl.o \
    $SRC_ROOT/modules/audio_processing/audio_processing/level_estimator_impl.dwo \
    $SRC_ROOT/modules/audio_processing/audio_processing/low_cut_filter.o \
    $SRC_ROOT/modules/audio_processing/audio_processing/low_cut_filter.dwo \
    $SRC_ROOT/modules/audio_processing/audio_processing/mean_variance_estimator.o \
    $SRC_ROOT/modules/audio_processing/audio_processing/mean_variance_estimator.dwo \
    $SRC_ROOT/modules/audio_processing/audio_processing/moving_max.o \
    $SRC_ROOT/modules/audio_processing/audio_processing/moving_max.dwo \
    $SRC_ROOT/modules/audio_processing/audio_processing/moving_moments.o \
    $SRC_ROOT/modules/audio_processing/audio_processing/moving_moments.dwo \
    $SRC_ROOT/modules/audio_processing/audio_processing/noise_suppression_impl.o \
    $SRC_ROOT/modules/audio_processing/audio_processing/noise_suppression_impl.dwo \
    $SRC_ROOT/modules/audio_processing/audio_processing/normalized_covariance_estimator.o \
    $SRC_ROOT/modules/audio_processing/audio_processing/normalized_covariance_estimator.dwo \
    $SRC_ROOT/modules/audio_processing/audio_processing/residual_echo_detector.o \
    $SRC_ROOT/modules/audio_processing/audio_processing/residual_echo_detector.dwo \
    $SRC_ROOT/modules/audio_processing/audio_processing/rms_level.o \
    $SRC_ROOT/modules/audio_processing/audio_processing/rms_level.dwo \
    $SRC_ROOT/modules/audio_processing/audio_processing/transient_detector.o \
    $SRC_ROOT/modules/audio_processing/audio_processing/transient_detector.dwo \
    $SRC_ROOT/modules/audio_processing/audio_processing/transient_suppressor.o \
    $SRC_ROOT/modules/audio_processing/audio_processing/transient_suppressor.dwo \
    $SRC_ROOT/modules/audio_processing/audio_processing/typing_detection.o \
    $SRC_ROOT/modules/audio_processing/audio_processing/typing_detection.dwo \
    $SRC_ROOT/modules/audio_processing/audio_processing/voice_detection_impl.o \
    $SRC_ROOT/modules/audio_processing/audio_processing/voice_detection_impl.dwo \
    $SRC_ROOT/modules/audio_processing/audio_processing/wpd_node.o \
    $SRC_ROOT/modules/audio_processing/audio_processing/wpd_node.dwo \
    $SRC_ROOT/modules/audio_processing/audio_processing/wpd_tree.o \
    $SRC_ROOT/modules/audio_processing/audio_processing/wpd_tree.dwo \
    $SRC_ROOT/modules/audio_processing/audio_processing_c/noise_suppression.o \
    $SRC_ROOT/modules/audio_processing/audio_processing_c/noise_suppression.dwo \
    $SRC_ROOT/modules/audio_processing/audio_processing_c/ns_core.o \
    $SRC_ROOT/modules/audio_processing/audio_processing_c/ns_core.dwo \
    $SRC_ROOT/modules/audio_processing/audio_processing_statistics/audio_processing_statistics.o \
    $SRC_ROOT/modules/audio_processing/audio_processing_statistics/audio_processing_statistics.dwo \
    $SRC_ROOT/modules/audio_processing/config/config.o \
    $SRC_ROOT/modules/audio_processing/config/config.dwo \
    $SRC_ROOT/modules/audio_processing/noise_suppression_proxy/noise_suppression_proxy.o \
    $SRC_ROOT/modules/audio_processing/noise_suppression_proxy/noise_suppression_proxy.dwo \
    $SRC_ROOT/modules/audio_processing/utility/block_mean_calculator/block_mean_calculator.o \
    $SRC_ROOT/modules/audio_processing/utility/block_mean_calculator/block_mean_calculator.dwo \
    $SRC_ROOT/modules/audio_processing/utility/legacy_delay_estimator/delay_estimator_wrapper.o \
    $SRC_ROOT/modules/audio_processing/utility/legacy_delay_estimator/delay_estimator_wrapper.dwo \
    $SRC_ROOT/modules/audio_processing/utility/legacy_delay_estimator/delay_estimator.o \
    $SRC_ROOT/modules/audio_processing/utility/legacy_delay_estimator/delay_estimator.dwo \
    $SRC_ROOT/modules/audio_processing/utility/ooura_fft/ooura_fft_sse2.o \
    $SRC_ROOT/modules/audio_processing/utility/ooura_fft/ooura_fft_sse2.dwo \
    $SRC_ROOT/modules/audio_processing/utility/ooura_fft/ooura_fft.o \
    $SRC_ROOT/modules/audio_processing/utility/ooura_fft/ooura_fft.dwo \
    $SRC_ROOT/modules/audio_processing/vad/vad/gmm.o \
    $SRC_ROOT/modules/audio_processing/vad/vad/gmm.dwo \
    $SRC_ROOT/modules/audio_processing/vad/vad/pitch_based_vad.o \
    $SRC_ROOT/modules/audio_processing/vad/vad/pitch_based_vad.dwo \
    $SRC_ROOT/modules/audio_processing/vad/vad/pitch_internal.o \
    $SRC_ROOT/modules/audio_processing/vad/vad/pitch_internal.dwo \
    $SRC_ROOT/modules/audio_processing/vad/vad/pole_zero_filter.o \
    $SRC_ROOT/modules/audio_processing/vad/vad/pole_zero_filter.dwo \
    $SRC_ROOT/modules/audio_processing/vad/vad/standalone_vad.o \
    $SRC_ROOT/modules/audio_processing/vad/vad/standalone_vad.dwo \
    $SRC_ROOT/modules/audio_processing/vad/vad/vad_audio_proc.o \
    $SRC_ROOT/modules/audio_processing/vad/vad/vad_audio_proc.dwo \
    $SRC_ROOT/modules/audio_processing/vad/vad/vad_circular_buffer.o \
    $SRC_ROOT/modules/audio_processing/vad/vad/vad_circular_buffer.dwo \
    $SRC_ROOT/modules/audio_processing/vad/vad/voice_activity_detector.o \
    $SRC_ROOT/modules/audio_processing/vad/vad/voice_activity_detector.dwo \
    $SRC_ROOT/audio/utility/audio_frame_operations/audio_frame_operations.o \
    $SRC_ROOT/audio/utility/audio_frame_operations/audio_frame_operations.dwo \
    $SRC_ROOT/rtc_base/rtc_base_approved/event_tracer.o \
    $SRC_ROOT/rtc_base/rtc_base_approved/event_tracer.dwo \
    $SRC_ROOT/rtc_base/criticalsection/critical_section.o \
    $SRC_ROOT/rtc_base/criticalsection/critical_section.dwo \
    $SRC_ROOT/rtc_base/logging/logging.o \
    $SRC_ROOT/rtc_base/logging/logging.dwo \
    $SRC_ROOT/rtc_base/timeutils/time_utils.o \
    $SRC_ROOT/rtc_base/timeutils/time_utils.dwo \
    $SRC_ROOT/rtc_base/platform_thread/platform_thread.o \
    $SRC_ROOT/rtc_base/platform_thread/platform_thread.dwo \
    $SRC_ROOT/rtc_base/rtc_event/event.o \
    $SRC_ROOT/rtc_base/rtc_event/event.dwo \
    $SRC_ROOT/rtc_base/thread_checker/thread_checker_impl.o \
    $SRC_ROOT/rtc_base/thread_checker/thread_checker_impl.dwo \
    $SRC_ROOT/rtc_base/stringutils/string_builder.o \
    $SRC_ROOT/rtc_base/stringutils/string_builder.dwo \
    $SRC_ROOT/rtc_base/stringutils/string_encode.o \
    $SRC_ROOT/rtc_base/stringutils/string_encode.dwo \
    $SRC_ROOT/rtc_base/stringutils/string_to_number.o \
    $SRC_ROOT/rtc_base/stringutils/string_to_number.dwo \
    $SRC_ROOT/rtc_base/stringutils/string_utils.o \
    $SRC_ROOT/rtc_base/stringutils/string_utils.dwo \
    $SRC_ROOT/api/audio/audio_frame_api/audio_frame.o \
    $SRC_ROOT/api/audio/audio_frame_api/audio_frame.dwo \
    $SRC_ROOT/api/audio/aec3_config/echo_canceller3_config.o \
    $SRC_ROOT/api/audio/aec3_config/echo_canceller3_config.dwo \
    $SRC_ROOT/system_wrappers/metrics/metrics.o \
    $SRC_ROOT/system_wrappers/metrics/metrics.dwo \
    $SRC_ROOT/system_wrappers/system_wrappers/cpu_features.o \
    $SRC_ROOT/system_wrappers/system_wrappers/cpu_features.dwo \
    $SRC_ROOT/system_wrappers/system_wrappers/cpu_info.o \
    $SRC_ROOT/system_wrappers/system_wrappers/cpu_info.dwo \
    $SRC_ROOT/system_wrappers/field_trial/field_trial.o \
    $SRC_ROOT/system_wrappers/field_trial/field_trial.dwo \
    $SRC_ROOT/common_audio/common_audio/audio_converter.o \
    $SRC_ROOT/common_audio/common_audio/audio_converter.dwo \
    $SRC_ROOT/common_audio/common_audio/audio_util.o \
    $SRC_ROOT/common_audio/common_audio/audio_util.dwo \
    $SRC_ROOT/common_audio/common_audio/channel_buffer.o \
    $SRC_ROOT/common_audio/common_audio/channel_buffer.dwo \
    $SRC_ROOT/common_audio/common_audio/push_resampler.o \
    $SRC_ROOT/common_audio/common_audio/push_resampler.dwo \
    $SRC_ROOT/common_audio/common_audio/push_sinc_resampler.o \
    $SRC_ROOT/common_audio/common_audio/push_sinc_resampler.dwo \
    $SRC_ROOT/common_audio/common_audio/real_fourier_ooura.o \
    $SRC_ROOT/common_audio/common_audio/real_fourier_ooura.dwo \
    $SRC_ROOT/common_audio/common_audio/real_fourier.o \
    $SRC_ROOT/common_audio/common_audio/real_fourier.dwo \
    $SRC_ROOT/common_audio/common_audio/resampler.o \
    $SRC_ROOT/common_audio/common_audio/resampler.dwo \
    $SRC_ROOT/common_audio/common_audio/sinc_resampler.o \
    $SRC_ROOT/common_audio/common_audio/sinc_resampler.dwo \
    $SRC_ROOT/common_audio/common_audio/smoothing_filter.o \
    $SRC_ROOT/common_audio/common_audio/smoothing_filter.dwo \
    $SRC_ROOT/common_audio/common_audio/sparse_fir_filter.o \
    $SRC_ROOT/common_audio/common_audio/sparse_fir_filter.dwo \
    $SRC_ROOT/common_audio/common_audio/vad.o \
    $SRC_ROOT/common_audio/common_audio/vad.dwo \
    $SRC_ROOT/common_audio/common_audio/wav_file.o \
    $SRC_ROOT/common_audio/common_audio/wav_file.dwo \
    $SRC_ROOT/common_audio/common_audio/wav_header.o \
    $SRC_ROOT/common_audio/common_audio/wav_header.dwo \
    $SRC_ROOT/common_audio/common_audio/window_generator.o \
    $SRC_ROOT/common_audio/common_audio/window_generator.dwo \
    $SRC_ROOT/common_audio/common_audio_c/auto_corr_to_refl_coef.o \
    $SRC_ROOT/common_audio/common_audio_c/auto_corr_to_refl_coef.dwo \
    $SRC_ROOT/common_audio/common_audio_c/auto_correlation.o \
    $SRC_ROOT/common_audio/common_audio_c/auto_correlation.dwo \
    $SRC_ROOT/common_audio/common_audio_c/complex_bit_reverse.o \
    $SRC_ROOT/common_audio/common_audio_c/complex_bit_reverse.dwo \
    $SRC_ROOT/common_audio/common_audio_c/complex_fft.o \
    $SRC_ROOT/common_audio/common_audio_c/complex_fft.dwo \
    $SRC_ROOT/common_audio/common_audio_c/copy_set_operations.o \
    $SRC_ROOT/common_audio/common_audio_c/copy_set_operations.dwo \
    $SRC_ROOT/common_audio/common_audio_c/cross_correlation.o \
    $SRC_ROOT/common_audio/common_audio_c/cross_correlation.dwo \
    $SRC_ROOT/common_audio/common_audio_c/division_operations.o \
    $SRC_ROOT/common_audio/common_audio_c/division_operations.dwo \
    $SRC_ROOT/common_audio/common_audio_c/downsample_fast.o \
    $SRC_ROOT/common_audio/common_audio_c/downsample_fast.dwo \
    $SRC_ROOT/common_audio/common_audio_c/energy.o \
    $SRC_ROOT/common_audio/common_audio_c/energy.dwo \
    $SRC_ROOT/common_audio/common_audio_c/filter_ar_fast_q12.o \
    $SRC_ROOT/common_audio/common_audio_c/filter_ar_fast_q12.dwo \
    $SRC_ROOT/common_audio/common_audio_c/filter_ar.o \
    $SRC_ROOT/common_audio/common_audio_c/filter_ar.dwo \
    $SRC_ROOT/common_audio/common_audio_c/filter_ma_fast_q12.o \
    $SRC_ROOT/common_audio/common_audio_c/filter_ma_fast_q12.dwo \
    $SRC_ROOT/common_audio/common_audio_c/get_hanning_window.o \
    $SRC_ROOT/common_audio/common_audio_c/get_hanning_window.dwo \
    $SRC_ROOT/common_audio/common_audio_c/get_scaling_square.o \
    $SRC_ROOT/common_audio/common_audio_c/get_scaling_square.dwo \
    $SRC_ROOT/common_audio/common_audio_c/ilbc_specific_functions.o \
    $SRC_ROOT/common_audio/common_audio_c/ilbc_specific_functions.dwo \
    $SRC_ROOT/common_audio/common_audio_c/levinson_durbin.o \
    $SRC_ROOT/common_audio/common_audio_c/levinson_durbin.dwo \
    $SRC_ROOT/common_audio/common_audio_c/lpc_to_refl_coef.o \
    $SRC_ROOT/common_audio/common_audio_c/lpc_to_refl_coef.dwo \
    $SRC_ROOT/common_audio/common_audio_c/min_max_operations.o \
    $SRC_ROOT/common_audio/common_audio_c/min_max_operations.dwo \
    $SRC_ROOT/common_audio/common_audio_c/randomization_functions.o \
    $SRC_ROOT/common_audio/common_audio_c/randomization_functions.dwo \
    $SRC_ROOT/common_audio/common_audio_c/real_fft.o \
    $SRC_ROOT/common_audio/common_audio_c/real_fft.dwo \
    $SRC_ROOT/common_audio/common_audio_c/refl_coef_to_lpc.o \
    $SRC_ROOT/common_audio/common_audio_c/refl_coef_to_lpc.dwo \
    $SRC_ROOT/common_audio/common_audio_c/resample_48khz.o \
    $SRC_ROOT/common_audio/common_audio_c/resample_48khz.dwo \
    $SRC_ROOT/common_audio/common_audio_c/resample_by_2_internal.o \
    $SRC_ROOT/common_audio/common_audio_c/resample_by_2_internal.dwo \
    $SRC_ROOT/common_audio/common_audio_c/resample_by_2.o \
    $SRC_ROOT/common_audio/common_audio_c/resample_by_2.dwo \
    $SRC_ROOT/common_audio/common_audio_c/resample_fractional.o \
    $SRC_ROOT/common_audio/common_audio_c/resample_fractional.dwo \
    $SRC_ROOT/common_audio/common_audio_c/resample.o \
    $SRC_ROOT/common_audio/common_audio_c/resample.dwo \
    $SRC_ROOT/common_audio/common_audio_c/ring_buffer.o \
    $SRC_ROOT/common_audio/common_audio_c/ring_buffer.dwo \
    $SRC_ROOT/common_audio/common_audio_c/spl_init.o \
    $SRC_ROOT/common_audio/common_audio_c/spl_init.dwo \
    $SRC_ROOT/common_audio/common_audio_c/spl_inl.o \
    $SRC_ROOT/common_audio/common_audio_c/spl_inl.dwo \
    $SRC_ROOT/common_audio/common_audio_c/spl_sqrt.o \
    $SRC_ROOT/common_audio/common_audio_c/spl_sqrt.dwo \
    $SRC_ROOT/common_audio/common_audio_c/splitting_filter.o \
    $SRC_ROOT/common_audio/common_audio_c/splitting_filter.dwo \
    $SRC_ROOT/common_audio/common_audio_c/sqrt_of_one_minus_x_squared.o \
    $SRC_ROOT/common_audio/common_audio_c/sqrt_of_one_minus_x_squared.dwo \
    $SRC_ROOT/common_audio/common_audio_c/vad_core.o \
    $SRC_ROOT/common_audio/common_audio_c/vad_core.dwo \
    $SRC_ROOT/common_audio/common_audio_c/vad_filterbank.o \
    $SRC_ROOT/common_audio/common_audio_c/vad_filterbank.dwo \
    $SRC_ROOT/common_audio/common_audio_c/vad_gmm.o \
    $SRC_ROOT/common_audio/common_audio_c/vad_gmm.dwo \
    $SRC_ROOT/common_audio/common_audio_c/vad_sp.o \
    $SRC_ROOT/common_audio/common_audio_c/vad_sp.dwo \
    $SRC_ROOT/common_audio/common_audio_c/vector_scaling_operations.o \
    $SRC_ROOT/common_audio/common_audio_c/vector_scaling_operations.dwo \
    $SRC_ROOT/common_audio/common_audio_c/webrtc_vad.o \
    $SRC_ROOT/common_audio/common_audio_c/webrtc_vad.dwo \
    $SRC_ROOT/common_audio/common_audio_cc/dot_product_with_scale.o \
    $SRC_ROOT/common_audio/common_audio_cc/dot_product_with_scale.dwo \
    $SRC_ROOT/common_audio/common_audio_sse2/fir_filter_sse.o \
    $SRC_ROOT/common_audio/common_audio_sse2/fir_filter_sse.dwo \
    $SRC_ROOT/common_audio/common_audio_sse2/sinc_resampler_sse.o \
    $SRC_ROOT/common_audio/common_audio_sse2/sinc_resampler_sse.dwo \
    $SRC_ROOT/common_audio/fir_filter_factory/fir_filter_c.o \
    $SRC_ROOT/common_audio/fir_filter_factory/fir_filter_c.dwo \
    $SRC_ROOT/common_audio/fir_filter_factory/fir_filter_factory.o \
    $SRC_ROOT/common_audio/fir_filter_factory/fir_filter_factory.dwo \
    $SRC_ROOT/common_audio/third_party/fft4g/fft4g/fft4g.o \
    $SRC_ROOT/common_audio/third_party/fft4g/fft4g/fft4g.dwo \
    $SRC_ROOT/common_audio/third_party/spl_sqrt_floor/spl_sqrt_floor/spl_sqrt_floor.o \
    $SRC_ROOT/common_audio/third_party/spl_sqrt_floor/spl_sqrt_floor/spl_sqrt_floor.dwo \
    $SRC_ROOT/third_party/rnnoise/kiss_fft/kiss_fft.o \
    $SRC_ROOT/third_party/rnnoise/kiss_fft/kiss_fft.dwo \
    $SRC_ROOT/third_party/rnnoise/rnn_vad/rnn_vad_weights.o \
    $SRC_ROOT/third_party/rnnoise/rnn_vad/rnn_vad_weights.dwo \
    $SRC_ROOT/third_party/abseil-cpp/absl/types/bad_optional_access/bad_optional_access.o \
    $SRC_ROOT/third_party/abseil-cpp/absl/types/bad_optional_access/bad_optional_access.dwo \
    $SRC_ROOT/third_party/abseil-cpp/absl/base/base/raw_logging.o \
    $SRC_ROOT/third_party/abseil-cpp/absl/base/base/raw_logging.dwo \
    $SRC_ROOT/modules/audio_coding/isac/audio_decoder_isac.o \
    $SRC_ROOT/modules/audio_coding/isac/audio_decoder_isac.dwo \
    $SRC_ROOT/modules/audio_coding/isac/audio_encoder_isac.o \
    $SRC_ROOT/modules/audio_coding/isac/audio_encoder_isac.dwo \
    $SRC_ROOT/modules/audio_coding/isac_common/locked_bandwidth_info.o \
    $SRC_ROOT/modules/audio_coding/isac_common/locked_bandwidth_info.dwo \
    $SRC_ROOT/modules/audio_coding/isac_vad/filter_functions.o \
    $SRC_ROOT/modules/audio_coding/isac_vad/filter_functions.dwo \
    $SRC_ROOT/modules/audio_coding/isac_vad/isac_vad.o \
    $SRC_ROOT/modules/audio_coding/isac_vad/isac_vad.dwo \
    $SRC_ROOT/modules/audio_coding/isac_vad/pitch_estimator.o \
    $SRC_ROOT/modules/audio_coding/isac_vad/pitch_estimator.dwo \
    $SRC_ROOT/modules/audio_coding/isac_vad/pitch_filter.o \
    $SRC_ROOT/modules/audio_coding/isac_vad/pitch_filter.dwo \
    $SRC_ROOT/modules/audio_coding/isac_c/arith_routines_hist.o \
    $SRC_ROOT/modules/audio_coding/isac_c/arith_routines_hist.dwo \
    $SRC_ROOT/modules/audio_coding/isac_c/arith_routines_logist.o \
    $SRC_ROOT/modules/audio_coding/isac_c/arith_routines_logist.dwo \
    $SRC_ROOT/modules/audio_coding/isac_c/arith_routines.o \
    $SRC_ROOT/modules/audio_coding/isac_c/arith_routines.dwo \
    $SRC_ROOT/modules/audio_coding/isac_c/bandwidth_estimator.o \
    $SRC_ROOT/modules/audio_coding/isac_c/bandwidth_estimator.dwo \
    $SRC_ROOT/modules/audio_coding/isac_c/crc.o \
    $SRC_ROOT/modules/audio_coding/isac_c/crc.dwo \
    $SRC_ROOT/modules/audio_coding/isac_c/decode_bwe.o \
    $SRC_ROOT/modules/audio_coding/isac_c/decode_bwe.dwo \
    $SRC_ROOT/modules/audio_coding/isac_c/decode.o \
    $SRC_ROOT/modules/audio_coding/isac_c/decode.dwo \
    $SRC_ROOT/modules/audio_coding/isac_c/encode_lpc_swb.o \
    $SRC_ROOT/modules/audio_coding/isac_c/encode_lpc_swb.dwo \
    $SRC_ROOT/modules/audio_coding/isac_c/encode.o \
    $SRC_ROOT/modules/audio_coding/isac_c/encode.dwo \
    $SRC_ROOT/modules/audio_coding/isac_c/entropy_coding.o \
    $SRC_ROOT/modules/audio_coding/isac_c/entropy_coding.dwo \
    $SRC_ROOT/modules/audio_coding/isac_c/filterbanks.o \
    $SRC_ROOT/modules/audio_coding/isac_c/filterbanks.dwo \
    $SRC_ROOT/modules/audio_coding/isac_c/intialize.o \
    $SRC_ROOT/modules/audio_coding/isac_c/intialize.dwo \
    $SRC_ROOT/modules/audio_coding/isac_c/isac.o \
    $SRC_ROOT/modules/audio_coding/isac_c/isac.dwo \
    $SRC_ROOT/modules/audio_coding/isac_c/lattice.o \
    $SRC_ROOT/modules/audio_coding/isac_c/lattice.dwo \
    $SRC_ROOT/modules/audio_coding/isac_c/lpc_analysis.o \
    $SRC_ROOT/modules/audio_coding/isac_c/lpc_analysis.dwo \
    $SRC_ROOT/modules/audio_coding/isac_c/lpc_gain_swb_tables.o \
    $SRC_ROOT/modules/audio_coding/isac_c/lpc_gain_swb_tables.dwo \
    $SRC_ROOT/modules/audio_coding/isac_c/lpc_shape_swb12_tables.o \
    $SRC_ROOT/modules/audio_coding/isac_c/lpc_shape_swb12_tables.dwo \
    $SRC_ROOT/modules/audio_coding/isac_c/lpc_shape_swb16_tables.o \
    $SRC_ROOT/modules/audio_coding/isac_c/lpc_shape_swb16_tables.dwo \
    $SRC_ROOT/modules/audio_coding/isac_c/lpc_tables.o \
    $SRC_ROOT/modules/audio_coding/isac_c/lpc_tables.dwo \
    $SRC_ROOT/modules/audio_coding/isac_c/pitch_gain_tables.o \
    $SRC_ROOT/modules/audio_coding/isac_c/pitch_gain_tables.dwo \
    $SRC_ROOT/modules/audio_coding/isac_c/pitch_lag_tables.o \
    $SRC_ROOT/modules/audio_coding/isac_c/pitch_lag_tables.dwo \
    $SRC_ROOT/modules/audio_coding/isac_c/spectrum_ar_model_tables.o \
    $SRC_ROOT/modules/audio_coding/isac_c/spectrum_ar_model_tables.dwo \
    $SRC_ROOT/modules/audio_coding/isac_c/transform.o \
    $SRC_ROOT/modules/audio_coding/isac_c/transform.dwo \
;;
2)
    #for win32
    SRC_ROOT=`pwd`/../../../svc_webrtc/bizconf_rtc_endpoint/webrtc-checkout/src/out/windows_openssl/obj/
    DST_ROOT=`pwd`/../ffmpeg-linux/mylib-win/
    echo $SRC_ROOT
    mkdir tmp
    cd tmp
    cp $SRC_ROOT/api/video/video_frame_i420/i420_buffer.obj ./
    cp $SRC_ROOT/api/video/video_frame/color_space.obj ./
    cp $SRC_ROOT/api/video/video_frame/hdr_metadata.obj ./
    cp $SRC_ROOT/api/video/video_frame/video_content_type.obj ./
    cp $SRC_ROOT/api/video/video_frame/video_frame_buffer.obj ./
    cp $SRC_ROOT/api/video/video_frame/video_frame.obj ./
    cp $SRC_ROOT/api/video/video_frame/video_source_interface.obj ./
    cp $SRC_ROOT/api/video/video_frame/video_timing.obj ./
    cp $SRC_ROOT/common_video/common_video/i420_buffer_pool.obj ./
    cp $SRC_ROOT/modules/video_processing/video_processing/denoiser_filter_c.obj ./
    cp $SRC_ROOT/modules/video_processing/video_processing/denoiser_filter.obj ./
    cp $SRC_ROOT/modules/video_processing/video_processing/noise_estimation.obj ./
    cp $SRC_ROOT/modules/video_processing/video_processing/skin_detection.obj ./
    cp $SRC_ROOT/modules/video_processing/video_processing/video_denoiser.obj ./
    cp $SRC_ROOT/modules/video_processing/video_processing_sse2/denoiser_filter_sse2.obj ./
    cp $SRC_ROOT/third_party/libyuv/libyuv_internal/compare_common.obj ./
    cp $SRC_ROOT/third_party/libyuv/libyuv_internal/compare_gcc.obj ./
    cp $SRC_ROOT/third_party/libyuv/libyuv_internal/compare.obj ./
    cp $SRC_ROOT/third_party/libyuv/libyuv_internal/convert_argb.obj ./
    cp $SRC_ROOT/third_party/libyuv/libyuv_internal/convert_from_argb.obj ./
    cp $SRC_ROOT/third_party/libyuv/libyuv_internal/convert_from.obj ./
    cp $SRC_ROOT/third_party/libyuv/libyuv_internal/convert_jpeg.obj ./
    cp $SRC_ROOT/third_party/libyuv/libyuv_internal/convert_to_argb.obj ./
    cp $SRC_ROOT/third_party/libyuv/libyuv_internal/convert_to_i420.obj ./
    cp $SRC_ROOT/third_party/libyuv/libyuv_internal/convert.obj ./
    cp $SRC_ROOT/third_party/libyuv/libyuv_internal/cpu_id.obj ./
    cp $SRC_ROOT/third_party/libyuv/libyuv_internal/mjpeg_decoder.obj ./
    cp $SRC_ROOT/third_party/libyuv/libyuv_internal/mjpeg_validate.obj ./
    cp $SRC_ROOT/third_party/libyuv/libyuv_internal/planar_functions.obj ./
    cp $SRC_ROOT/third_party/libyuv/libyuv_internal/rotate_any.obj ./
    cp $SRC_ROOT/third_party/libyuv/libyuv_internal/rotate_argb.obj ./
    cp $SRC_ROOT/third_party/libyuv/libyuv_internal/rotate_common.obj ./
    cp $SRC_ROOT/third_party/libyuv/libyuv_internal/rotate_gcc.obj ./
    cp $SRC_ROOT/third_party/libyuv/libyuv_internal/rotate.obj ./
    cp $SRC_ROOT/third_party/libyuv/libyuv_internal/row_any.obj ./
    cp $SRC_ROOT/third_party/libyuv/libyuv_internal/row_common.obj ./
    cp $SRC_ROOT/third_party/libyuv/libyuv_internal/row_gcc.obj ./
    cp $SRC_ROOT/third_party/libyuv/libyuv_internal/scale_any.obj ./
    cp $SRC_ROOT/third_party/libyuv/libyuv_internal/scale_argb.obj ./
    cp $SRC_ROOT/third_party/libyuv/libyuv_internal/scale_common.obj ./
    cp $SRC_ROOT/third_party/libyuv/libyuv_internal/scale_gcc.obj ./
    cp $SRC_ROOT/third_party/libyuv/libyuv_internal/scale.obj ./
    cp $SRC_ROOT/third_party/libyuv/libyuv_internal/video_common.obj ./
    cp $SRC_ROOT/rtc_base/memory/aligned_malloc/aligned_malloc.obj ./
    cp $SRC_ROOT/rtc_base/checks/checks.obj ./
    cp $SRC_ROOT/rtc_base/rtc_base_approved/race_checker.obj ./
    cp $SRC_ROOT/rtc_base/platform_thread_types/platform_thread_types.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec/aec/aec_resampler.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec/aec/echo_cancellation.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec/aec_core/aec_core_sse2.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec/aec_core/aec_core.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec_dump/null_aec_dump_factory/null_aec_dump_factory.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/adaptive_fir_filter.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/aec_state.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/aec3_common.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/aec3_fft.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/api_call_jitter_metrics.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/block_delay_buffer.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/block_framer.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/block_processor_metrics.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/block_processor.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/cascaded_biquad_filter.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/clockdrift_detector.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/comfort_noise_generator.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/decimator.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/downsampled_render_buffer.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/echo_audibility.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/echo_canceller3.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/echo_path_delay_estimator.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/echo_path_variability.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/echo_remover_metrics.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/echo_remover.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/erl_estimator.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/erle_estimator.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/fft_buffer.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/filter_analyzer.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/frame_blocker.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/fullband_erle_estimator.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/main_filter_update_gain.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/matched_filter_lag_aggregator.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/matched_filter.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/matrix_buffer.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/moving_average.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/render_buffer.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/render_delay_buffer.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/render_delay_controller_metrics.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/render_delay_controller.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/render_reverb_model.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/render_signal_analyzer.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/residual_echo_estimator.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/reverb_decay_estimator.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/reverb_frequency_response.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/reverb_model_estimator.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/reverb_model_fallback.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/reverb_model.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/shadow_filter_update_gain.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/signal_dependent_erle_estimator.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/skew_estimator.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/stationarity_estimator.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/subband_erle_estimator.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/subtractor_output_analyzer.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/subtractor_output.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/subtractor.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/suppression_filter.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/suppression_gain.obj ./
    cp $SRC_ROOT/modules/audio_processing/aec3/aec3/vector_buffer.obj ./
    cp $SRC_ROOT/modules/audio_processing/aecm/aecm_core/aecm_core_c.obj ./
    cp $SRC_ROOT/modules/audio_processing/aecm/aecm_core/aecm_core.obj ./
    cp $SRC_ROOT/modules/audio_processing/aecm/aecm_core/echo_control_mobile.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc/agc/agc_manager_direct.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc/agc_legacy_c/analog_agc.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc/agc_legacy_c/digital_agc.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc/level_estimation/agc.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc/level_estimation/loudness_histogram.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc/level_estimation/utility.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc2/adaptive_digital/adaptive_agc.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc2/adaptive_digital/adaptive_digital_gain_applier.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc2/adaptive_digital/adaptive_mode_level_estimator.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc2/adaptive_digital/saturation_protector.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc2/biquad_filter/biquad_filter.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc2/common/agc2_common.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc2/fixed_digital/fixed_digital_level_estimator.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc2/fixed_digital/interpolated_gain_curve.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc2/fixed_digital/limiter.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc2/gain_applier/gain_applier.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc2/level_estimation_agc/adaptive_mode_level_estimator_agc.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc2/noise_level_estimator/down_sampler.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc2/noise_level_estimator/noise_level_estimator.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc2/noise_level_estimator/noise_spectrum_estimator.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc2/noise_level_estimator/signal_classifier.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/rnn_vad/features_extraction.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/rnn_vad/fft_util.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/rnn_vad/lp_residual.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/rnn_vad/pitch_search_internal.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/rnn_vad/pitch_search.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/rnn_vad/rnn.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/rnn_vad/spectral_features_internal.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/rnn_vad/spectral_features.obj ./
    cp $SRC_ROOT/modules/audio_processing/agc2/rnn_vad_with_level/vad_with_level.obj ./
    cp $SRC_ROOT/modules/audio_processing/api/audio_processing.obj ./
    cp $SRC_ROOT/modules/audio_processing/apm_logging/apm_data_dumper.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_buffer/audio_buffer.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_buffer/splitting_filter.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_buffer/three_band_filter_bank.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_processing/aec_dump.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_processing/audio_processing_impl.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_processing/circular_buffer.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_processing/echo_cancellation_impl.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_processing/echo_control_mobile_impl.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_processing/gain_control_for_experimental_agc.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_processing/gain_control_impl.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_processing/gain_controller2.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_processing/level_estimator_impl.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_processing/low_cut_filter.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_processing/mean_variance_estimator.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_processing/moving_max.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_processing/moving_moments.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_processing/noise_suppression_impl.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_processing/normalized_covariance_estimator.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_processing/residual_echo_detector.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_processing/rms_level.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_processing/transient_detector.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_processing/transient_suppressor.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_processing/typing_detection.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_processing/voice_detection_impl.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_processing/wpd_node.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_processing/wpd_tree.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_processing_c/noise_suppression.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_processing_c/ns_core.obj ./
    cp $SRC_ROOT/modules/audio_processing/audio_processing_statistics/audio_processing_statistics.obj ./
    cp $SRC_ROOT/modules/audio_processing/config/config.obj ./
    cp $SRC_ROOT/modules/audio_processing/noise_suppression_proxy/noise_suppression_proxy.obj ./
    cp $SRC_ROOT/modules/audio_processing/utility/block_mean_calculator/block_mean_calculator.obj ./
    cp $SRC_ROOT/modules/audio_processing/utility/legacy_delay_estimator/delay_estimator_wrapper.obj ./
    cp $SRC_ROOT/modules/audio_processing/utility/legacy_delay_estimator/delay_estimator.obj ./
    cp $SRC_ROOT/modules/audio_processing/utility/ooura_fft/ooura_fft_sse2.obj ./
    cp $SRC_ROOT/modules/audio_processing/utility/ooura_fft/ooura_fft.obj ./
    cp $SRC_ROOT/modules/audio_processing/vad/vad/gmm.obj ./
    cp $SRC_ROOT/modules/audio_processing/vad/vad/pitch_based_vad.obj ./
    cp $SRC_ROOT/modules/audio_processing/vad/vad/pitch_internal.obj ./
    cp $SRC_ROOT/modules/audio_processing/vad/vad/pole_zero_filter.obj ./
    cp $SRC_ROOT/modules/audio_processing/vad/vad/standalone_vad.obj ./
    cp $SRC_ROOT/modules/audio_processing/vad/vad/vad_audio_proc.obj ./
    cp $SRC_ROOT/modules/audio_processing/vad/vad/vad_circular_buffer.obj ./
    cp $SRC_ROOT/modules/audio_processing/vad/vad/voice_activity_detector.obj ./
    cp $SRC_ROOT/audio/utility/audio_frame_operations/audio_frame_operations.obj ./
    cp $SRC_ROOT/rtc_base/rtc_base_approved/event_tracer.obj ./
    cp $SRC_ROOT/rtc_base/criticalsection/critical_section.obj ./
    cp $SRC_ROOT/rtc_base/logging/logging.obj ./
    cp $SRC_ROOT/rtc_base/timeutils/time_utils.obj ./
    cp $SRC_ROOT/rtc_base/platform_thread/platform_thread.obj ./
    cp $SRC_ROOT/rtc_base/rtc_event/event.obj ./
    cp $SRC_ROOT/rtc_base/thread_checker/thread_checker_impl.obj ./
    cp $SRC_ROOT/rtc_base/stringutils/string_builder.obj ./
    cp $SRC_ROOT/rtc_base/stringutils/string_encode.obj ./
    cp $SRC_ROOT/rtc_base/stringutils/string_to_number.obj ./
    cp $SRC_ROOT/rtc_base/stringutils/string_utils.obj ./
    cp $SRC_ROOT/api/audio/audio_frame_api/audio_frame.obj ./
    cp $SRC_ROOT/api/audio/aec3_config/echo_canceller3_config.obj ./
    cp $SRC_ROOT/system_wrappers/metrics/metrics.obj ./
    cp $SRC_ROOT/system_wrappers/system_wrappers/cpu_features.obj ./
    cp $SRC_ROOT/system_wrappers/system_wrappers/cpu_info.obj ./
    cp $SRC_ROOT/system_wrappers/field_trial/field_trial.obj ./
    cp $SRC_ROOT/common_audio/common_audio/audio_converter.obj ./
    cp $SRC_ROOT/common_audio/common_audio/audio_util.obj ./
    cp $SRC_ROOT/common_audio/common_audio/channel_buffer.obj ./
    cp $SRC_ROOT/common_audio/common_audio/push_resampler.obj ./
    cp $SRC_ROOT/common_audio/common_audio/push_sinc_resampler.obj ./
    cp $SRC_ROOT/common_audio/common_audio/real_fourier_ooura.obj ./
    cp $SRC_ROOT/common_audio/common_audio/real_fourier.obj ./
    cp $SRC_ROOT/common_audio/common_audio/resampler.obj ./
    cp $SRC_ROOT/common_audio/common_audio/sinc_resampler.obj ./
    cp $SRC_ROOT/common_audio/common_audio/smoothing_filter.obj ./
    cp $SRC_ROOT/common_audio/common_audio/sparse_fir_filter.obj ./
    cp $SRC_ROOT/common_audio/common_audio/vad.obj ./
    cp $SRC_ROOT/common_audio/common_audio/wav_file.obj ./
    cp $SRC_ROOT/common_audio/common_audio/wav_header.obj ./
    cp $SRC_ROOT/common_audio/common_audio/window_generator.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/auto_corr_to_refl_coef.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/auto_correlation.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/complex_bit_reverse.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/complex_fft.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/copy_set_operations.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/cross_correlation.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/division_operations.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/downsample_fast.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/energy.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/filter_ar_fast_q12.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/filter_ar.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/filter_ma_fast_q12.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/get_hanning_window.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/get_scaling_square.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/ilbc_specific_functions.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/levinson_durbin.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/lpc_to_refl_coef.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/min_max_operations.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/randomization_functions.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/real_fft.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/refl_coef_to_lpc.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/resample_48khz.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/resample_by_2_internal.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/resample_by_2.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/resample_fractional.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/resample.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/ring_buffer.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/spl_init.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/spl_inl.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/spl_sqrt.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/splitting_filter.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/sqrt_of_one_minus_x_squared.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/vad_core.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/vad_filterbank.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/vad_gmm.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/vad_sp.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/vector_scaling_operations.obj ./
    cp $SRC_ROOT/common_audio/common_audio_c/webrtc_vad.obj ./
    cp $SRC_ROOT/common_audio/common_audio_cc/dot_product_with_scale.obj ./
    cp $SRC_ROOT/common_audio/common_audio_sse2/fir_filter_sse.obj ./
    cp $SRC_ROOT/common_audio/common_audio_sse2/sinc_resampler_sse.obj ./
    cp $SRC_ROOT/common_audio/fir_filter_factory/fir_filter_c.obj ./
    cp $SRC_ROOT/common_audio/fir_filter_factory/fir_filter_factory.obj ./
    cp $SRC_ROOT/common_audio/third_party/fft4g/fft4g/fft4g.obj ./
    cp $SRC_ROOT/common_audio/third_party/spl_sqrt_floor/spl_sqrt_floor/spl_sqrt_floor.obj ./
    cp $SRC_ROOT/third_party/rnnoise/kiss_fft/kiss_fft.obj ./
    cp $SRC_ROOT/third_party/rnnoise/rnn_vad/rnn_vad_weights.obj ./
    cp $SRC_ROOT/third_party/abseil-cpp/absl/types/bad_optional_access/bad_optional_access.obj ./
    cp $SRC_ROOT/third_party/abseil-cpp/absl/base/base/raw_logging.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac/audio_decoder_isac.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac/audio_encoder_isac.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac_common/locked_bandwidth_info.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac_vad/filter_functions.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac_vad/isac_vad.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac_vad/pitch_estimator.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac_vad/pitch_filter.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac_c/arith_routines_hist.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac_c/arith_routines_logist.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac_c/arith_routines.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac_c/bandwidth_estimator.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac_c/crc.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac_c/decode_bwe.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac_c/decode.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac_c/encode_lpc_swb.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac_c/encode.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac_c/entropy_coding.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac_c/filterbanks.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac_c/intialize.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac_c/isac.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac_c/lattice.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac_c/lpc_analysis.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac_c/lpc_gain_swb_tables.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac_c/lpc_shape_swb12_tables.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac_c/lpc_shape_swb16_tables.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac_c/lpc_tables.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac_c/pitch_gain_tables.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac_c/pitch_lag_tables.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac_c/spectrum_ar_model_tables.obj ./
    cp $SRC_ROOT/modules/audio_coding/isac_c/transform.obj ./


    ar rcs $DST_ROOT/libwebrtc.a *.obj
    cd ../
    ##rm -r tmp
;;
3)
    #for win32 audo_process test
    SRC_ROOT=`pwd`/../../../svc_webrtc/bizconf_rtc_endpoint/webrtc-checkout/src
    g++ -o libwebrtc.lib -fPIC \
    -D WEBRTC_WIN \
    -D WEBRTC_NS_FLOAT \
    -D WEBRTC_APM_DEBUG_DUMP=0 \
    -D MSYS2_GXH \
    -mavx \
    -fpermissive \
    $SRC_ROOT/rtc_base/platform_file.cc \
    $SRC_ROOT/modules/audio_processing/aec/aec_resampler.cc \
    $SRC_ROOT/modules/audio_processing/aec/echo_cancellation.cc \
    $SRC_ROOT/modules/audio_processing/aec/aec_core_sse2.cc \
    $SRC_ROOT/modules/audio_processing/aec/aec_core.cc \
    $SRC_ROOT/modules/audio_processing/aec_dump/null_aec_dump_factory.cc \
    $SRC_ROOT/modules/audio_processing/aec3/adaptive_fir_filter.cc \
    $SRC_ROOT/modules/audio_processing/aec3/aec_state.cc \
    $SRC_ROOT/modules/audio_processing/aec3/aec3_common.cc \
    $SRC_ROOT/modules/audio_processing/aec3/aec3_fft.cc \
    $SRC_ROOT/modules/audio_processing/aec3/api_call_jitter_metrics.cc \
    $SRC_ROOT/modules/audio_processing/aec3/block_delay_buffer.cc \
    $SRC_ROOT/modules/audio_processing/aec3/block_framer.cc \
    $SRC_ROOT/modules/audio_processing/aec3/block_processor_metrics.cc \
    $SRC_ROOT/modules/audio_processing/aec3/block_processor.cc \
    $SRC_ROOT/modules/audio_processing/aec3/cascaded_biquad_filter.cc \
    $SRC_ROOT/modules/audio_processing/aec3/clockdrift_detector.cc \
    $SRC_ROOT/modules/audio_processing/aec3/comfort_noise_generator.cc \
    $SRC_ROOT/modules/audio_processing/aec3/decimator.cc \
    $SRC_ROOT/modules/audio_processing/aec3/downsampled_render_buffer.cc \
    $SRC_ROOT/modules/audio_processing/aec3/echo_audibility.cc \
    $SRC_ROOT/modules/audio_processing/aec3/echo_canceller3.cc \
    $SRC_ROOT/modules/audio_processing/aec3/echo_path_delay_estimator.cc \
    $SRC_ROOT/modules/audio_processing/aec3/echo_path_variability.cc \
    $SRC_ROOT/modules/audio_processing/aec3/echo_remover_metrics.cc \
    $SRC_ROOT/modules/audio_processing/aec3/echo_remover.cc \
    $SRC_ROOT/modules/audio_processing/aec3/erl_estimator.cc \
    $SRC_ROOT/modules/audio_processing/aec3/erle_estimator.cc \
    $SRC_ROOT/modules/audio_processing/aec3/fft_buffer.cc \
    $SRC_ROOT/modules/audio_processing/aec3/filter_analyzer.cc \
    $SRC_ROOT/modules/audio_processing/aec3/frame_blocker.cc \
    $SRC_ROOT/modules/audio_processing/aec3/fullband_erle_estimator.cc \
    $SRC_ROOT/modules/audio_processing/aec3/main_filter_update_gain.cc \
    $SRC_ROOT/modules/audio_processing/aec3/matched_filter_lag_aggregator.cc \
    $SRC_ROOT/modules/audio_processing/aec3/matched_filter.cc \
    $SRC_ROOT/modules/audio_processing/aec3/matrix_buffer.cc \
    $SRC_ROOT/modules/audio_processing/aec3/moving_average.cc \
    $SRC_ROOT/modules/audio_processing/aec3/render_buffer.cc \
    $SRC_ROOT/modules/audio_processing/aec3/render_delay_buffer.cc \
    $SRC_ROOT/modules/audio_processing/aec3/render_delay_controller_metrics.cc \
    $SRC_ROOT/modules/audio_processing/aec3/render_delay_controller.cc \
    $SRC_ROOT/modules/audio_processing/aec3/render_reverb_model.cc \
    $SRC_ROOT/modules/audio_processing/aec3/render_signal_analyzer.cc \
    $SRC_ROOT/modules/audio_processing/aec3/residual_echo_estimator.cc \
    $SRC_ROOT/modules/audio_processing/aec3/reverb_decay_estimator.cc \
    $SRC_ROOT/modules/audio_processing/aec3/reverb_frequency_response.cc \
    $SRC_ROOT/modules/audio_processing/aec3/reverb_model_estimator.cc \
    $SRC_ROOT/modules/audio_processing/aec3/reverb_model_fallback.cc \
    $SRC_ROOT/modules/audio_processing/aec3/reverb_model.cc \
    $SRC_ROOT/modules/audio_processing/aec3/shadow_filter_update_gain.cc \
    $SRC_ROOT/modules/audio_processing/aec3/signal_dependent_erle_estimator.cc \
    $SRC_ROOT/modules/audio_processing/aec3/skew_estimator.cc \
    $SRC_ROOT/modules/audio_processing/aec3/stationarity_estimator.cc \
    $SRC_ROOT/modules/audio_processing/aec3/subband_erle_estimator.cc \
    $SRC_ROOT/modules/audio_processing/aec3/subtractor_output_analyzer.cc \
    $SRC_ROOT/modules/audio_processing/aec3/subtractor_output.cc \
    $SRC_ROOT/modules/audio_processing/aec3/subtractor.cc \
    $SRC_ROOT/modules/audio_processing/aec3/suppression_filter.cc \
    $SRC_ROOT/modules/audio_processing/aec3/suppression_gain.cc \
    $SRC_ROOT/modules/audio_processing/aec3/vector_buffer.cc \
    $SRC_ROOT/modules/audio_processing/aecm/aecm_core_c.cc \
    $SRC_ROOT/modules/audio_processing/aecm/aecm_core.cc \
    $SRC_ROOT/modules/audio_processing/aecm/echo_control_mobile.cc \
    $SRC_ROOT/modules/audio_processing/agc/agc_manager_direct.cc \
    $SRC_ROOT/modules/audio_processing/agc/legacy/analog_agc.c \
    $SRC_ROOT/modules/audio_processing/agc/legacy/digital_agc.c \
    $SRC_ROOT/modules/audio_processing/agc/agc.cc \
    $SRC_ROOT/modules/audio_processing/agc/loudness_histogram.cc \
    $SRC_ROOT/modules/audio_processing/agc/utility.cc \
    $SRC_ROOT/modules/audio_processing/agc2/adaptive_agc.cc \
    $SRC_ROOT/modules/audio_processing/agc2/adaptive_digital_gain_applier.cc \
    $SRC_ROOT/modules/audio_processing/agc2/adaptive_mode_level_estimator.cc \
    $SRC_ROOT/modules/audio_processing/agc2/saturation_protector.cc \
    $SRC_ROOT/modules/audio_processing/agc2/biquad_filter.cc \
    $SRC_ROOT/modules/audio_processing/agc2/agc2_common.cc \
    $SRC_ROOT/modules/audio_processing/agc2/fixed_digital_level_estimator.cc \
    $SRC_ROOT/modules/audio_processing/agc2/interpolated_gain_curve.cc \
    $SRC_ROOT/modules/audio_processing/agc2/limiter.cc \
    $SRC_ROOT/modules/audio_processing/agc2/gain_applier.cc \
    $SRC_ROOT/modules/audio_processing/agc2/adaptive_mode_level_estimator_agc.cc \
    $SRC_ROOT/modules/audio_processing/agc2/down_sampler.cc \
    $SRC_ROOT/modules/audio_processing/agc2/noise_level_estimator.cc \
    $SRC_ROOT/modules/audio_processing/agc2/noise_spectrum_estimator.cc \
    $SRC_ROOT/modules/audio_processing/agc2/signal_classifier.cc \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/features_extraction.cc \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/fft_util.cc \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/lp_residual.cc \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/pitch_search_internal.cc \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/pitch_search.cc \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/rnn.cc \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/spectral_features_internal.cc \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/spectral_features.cc \
    $SRC_ROOT/modules/audio_processing/agc2/vad_with_level.cc \
    $SRC_ROOT/modules/audio_processing/include/audio_processing.cc \
    $SRC_ROOT/modules/audio_processing/logging/apm_data_dumper.cc \
    $SRC_ROOT/modules/audio_processing/audio_buffer.cc \
    $SRC_ROOT/modules/audio_processing/splitting_filter.cc \
    $SRC_ROOT/modules/audio_processing/three_band_filter_bank.cc \
    $SRC_ROOT/modules/audio_processing/include/aec_dump.cc \
    $SRC_ROOT/modules/audio_processing/audio_processing_impl.cc \
    $SRC_ROOT/modules/audio_processing/echo_detector/circular_buffer.cc \
    $SRC_ROOT/modules/audio_processing/echo_cancellation_impl.cc \
    $SRC_ROOT/modules/audio_processing/echo_control_mobile_impl.cc \
    $SRC_ROOT/modules/audio_processing/gain_control_for_experimental_agc.cc \
    $SRC_ROOT/modules/audio_processing/gain_control_impl.cc \
    $SRC_ROOT/modules/audio_processing/gain_controller2.cc \
    $SRC_ROOT/modules/audio_processing/level_estimator_impl.cc \
    $SRC_ROOT/modules/audio_processing/low_cut_filter.cc \
    $SRC_ROOT/modules/audio_processing/echo_detector/mean_variance_estimator.cc \
    $SRC_ROOT/modules/audio_processing/echo_detector/moving_max.cc \
    $SRC_ROOT/modules/audio_processing/transient/moving_moments.cc \
    $SRC_ROOT/modules/audio_processing/noise_suppression_impl.cc \
    $SRC_ROOT/modules/audio_processing/echo_detector/normalized_covariance_estimator.cc \
    $SRC_ROOT/modules/audio_processing/residual_echo_detector.cc \
    $SRC_ROOT/modules/audio_processing/rms_level.cc \
    $SRC_ROOT/modules/audio_processing/transient/transient_detector.cc \
    $SRC_ROOT/modules/audio_processing/transient/transient_suppressor.cc \
    $SRC_ROOT/modules/audio_processing/typing_detection.cc \
    $SRC_ROOT/modules/audio_processing/voice_detection_impl.cc \
    $SRC_ROOT/modules/audio_processing/transient/wpd_node.cc \
    $SRC_ROOT/modules/audio_processing/transient/wpd_tree.cc \
    $SRC_ROOT/modules/audio_processing/ns/noise_suppression.c \
    $SRC_ROOT/modules/audio_processing/ns/ns_core.c \
    $SRC_ROOT/modules/audio_processing/include/audio_processing_statistics.cc \
    $SRC_ROOT/modules/audio_processing/include/config.cc \
    $SRC_ROOT/modules/audio_processing/noise_suppression_proxy.cc \
    $SRC_ROOT/modules/audio_processing/utility/block_mean_calculator.cc \
    $SRC_ROOT/modules/audio_processing/utility/delay_estimator_wrapper.cc \
    $SRC_ROOT/modules/audio_processing/utility/delay_estimator.cc \
    $SRC_ROOT/modules/audio_processing/utility/ooura_fft_sse2.cc \
    $SRC_ROOT/modules/audio_processing/utility/ooura_fft.cc \
    $SRC_ROOT/modules/audio_processing/vad/gmm.cc \
    $SRC_ROOT/modules/audio_processing/vad/pitch_based_vad.cc \
    $SRC_ROOT/modules/audio_processing/vad/pitch_internal.cc \
    $SRC_ROOT/modules/audio_processing/vad/pole_zero_filter.cc \
    $SRC_ROOT/modules/audio_processing/vad/standalone_vad.cc \
    $SRC_ROOT/modules/audio_processing/vad/vad_audio_proc.cc \
    $SRC_ROOT/modules/audio_processing/vad/vad_circular_buffer.cc \
    $SRC_ROOT/modules/audio_processing/vad/voice_activity_detector.cc \
    \
    $SRC_ROOT/rtc_base/memory/aligned_malloc.cc \
    $SRC_ROOT/rtc_base/checks.cc \
    $SRC_ROOT/rtc_base/race_checker.cc \
    $SRC_ROOT/rtc_base/platform_thread_types.cc \
    $SRC_ROOT/rtc_base/event_tracer.cc \
    $SRC_ROOT/rtc_base/critical_section.cc \
    \
    $SRC_ROOT/rtc_base/time_utils.cc \
    $SRC_ROOT/rtc_base/platform_thread.cc \
    $SRC_ROOT/rtc_base/event.cc \
    $SRC_ROOT/rtc_base/thread_checker_impl.cc \
    $SRC_ROOT/rtc_base/strings/string_builder.cc \
    $SRC_ROOT/rtc_base/string_encode.cc \
    $SRC_ROOT/rtc_base/string_to_number.cc \
    $SRC_ROOT/rtc_base/string_utils.cc \
    $SRC_ROOT/system_wrappers/source/metrics.cc \
    $SRC_ROOT/system_wrappers/source/cpu_features.cc \
    $SRC_ROOT/system_wrappers/source/cpu_info.cc \
    $SRC_ROOT/system_wrappers/source/field_trial.cc \
    \
    $SRC_ROOT/common_audio/audio_converter.cc \
    $SRC_ROOT/common_audio/audio_util.cc \
    $SRC_ROOT/common_audio/channel_buffer.cc \
    $SRC_ROOT/common_audio/resampler/push_resampler.cc \
    $SRC_ROOT/common_audio/resampler/push_sinc_resampler.cc \
    $SRC_ROOT/common_audio/real_fourier_ooura.cc \
    $SRC_ROOT/common_audio/real_fourier.cc \
    $SRC_ROOT/common_audio/resampler/resampler.cc \
    $SRC_ROOT/common_audio/resampler/sinc_resampler.cc \
    $SRC_ROOT/common_audio/smoothing_filter.cc \
    $SRC_ROOT/common_audio/sparse_fir_filter.cc \
    $SRC_ROOT/common_audio/vad/vad.cc \
    $SRC_ROOT/common_audio/wav_file.cc \
    $SRC_ROOT/common_audio/wav_header.cc \
    $SRC_ROOT/common_audio/window_generator.cc \
    $SRC_ROOT/common_audio/signal_processing/auto_corr_to_refl_coef.c \
    $SRC_ROOT/common_audio/signal_processing/auto_correlation.c \
    $SRC_ROOT/common_audio/signal_processing/complex_bit_reverse.c \
    $SRC_ROOT/common_audio/signal_processing/complex_fft.c \
    $SRC_ROOT/common_audio/signal_processing/copy_set_operations.c \
    $SRC_ROOT/common_audio/signal_processing/cross_correlation.c \
    $SRC_ROOT/common_audio/signal_processing/division_operations.c \
    $SRC_ROOT/common_audio/signal_processing/downsample_fast.c \
    $SRC_ROOT/common_audio/signal_processing/energy.c \
    $SRC_ROOT/common_audio/signal_processing/filter_ar_fast_q12.c \
    $SRC_ROOT/common_audio/signal_processing/filter_ar.c \
    $SRC_ROOT/common_audio/signal_processing/filter_ma_fast_q12.c \
    $SRC_ROOT/common_audio/signal_processing/get_hanning_window.c \
    $SRC_ROOT/common_audio/signal_processing/get_scaling_square.c \
    $SRC_ROOT/common_audio/signal_processing/ilbc_specific_functions.c \
    $SRC_ROOT/common_audio/signal_processing/levinson_durbin.c \
    $SRC_ROOT/common_audio/signal_processing/lpc_to_refl_coef.c \
    $SRC_ROOT/common_audio/signal_processing/min_max_operations.c \
    $SRC_ROOT/common_audio/signal_processing/randomization_functions.c \
    $SRC_ROOT/common_audio/signal_processing/real_fft.c \
    $SRC_ROOT/common_audio/signal_processing/refl_coef_to_lpc.c \
    $SRC_ROOT/common_audio/signal_processing/resample_48khz.c \
    $SRC_ROOT/common_audio/signal_processing/resample_by_2_internal.c \
    $SRC_ROOT/common_audio/signal_processing/resample_by_2.c \
    $SRC_ROOT/common_audio/signal_processing/resample_fractional.c \
    $SRC_ROOT/common_audio/signal_processing/resample.c \
    $SRC_ROOT/common_audio/ring_buffer.cc \
    $SRC_ROOT/common_audio/signal_processing/spl_init.c \
    $SRC_ROOT/common_audio/signal_processing/spl_inl.c \
    $SRC_ROOT/common_audio/signal_processing/spl_sqrt.c \
    $SRC_ROOT/common_audio/signal_processing/splitting_filter.c \
    $SRC_ROOT/common_audio/signal_processing/sqrt_of_one_minus_x_squared.c \
    $SRC_ROOT/common_audio/vad/vad_core.cc \
    $SRC_ROOT/common_audio/vad/vad_filterbank.cc \
    $SRC_ROOT/common_audio/vad/vad_gmm.cc \
    $SRC_ROOT/common_audio/vad/vad_sp.cc \
    $SRC_ROOT/common_audio/signal_processing/vector_scaling_operations.c \
    $SRC_ROOT/common_audio/vad/webrtc_vad.cc \
    $SRC_ROOT/common_audio/signal_processing/dot_product_with_scale.cc \
    $SRC_ROOT/common_audio/fir_filter_sse.cc \
    $SRC_ROOT/common_audio/resampler/sinc_resampler_sse.cc \
    $SRC_ROOT/common_audio/fir_filter_c.cc \
    $SRC_ROOT/common_audio/fir_filter_factory.cc \
    $SRC_ROOT/common_audio/third_party/fft4g/fft4g.cc \
    $SRC_ROOT/common_audio/third_party/spl_sqrt_floor/spl_sqrt_floor.cc \
    -I/D/msys64/mingw32/include \
	  -I/D/msys64/mingw32/i686-w64-mingw32/include \
    -I$SRC_ROOT \
    -I$SRC_ROOT/third_party \
    -I$SRC_ROOT/third_party/abseil-cpp \
    -L/D/msys64/mingw32/lib \
	  -L/D/msys64/mingw32/i686-w64-mingw32/lib \
	  -L/usr/lib \
    -lz -lstdc++ -lm \
	  -lpthread -lSDL2 -lws2_32 -ldl -llzma -lbz2 \
	  -limagehlp -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 \
	  -loleaut32 -lshell32 -lversion -luuid -lOpenGL32 \
	  -lwinspool -lcomctl32 -lcomdlg32 -lctl3d32 -ladvapi32 -lwsock32 -lkernel32 -luser32 \
	  -lrt -lstrmiids -lquartz -liconv -lvfw32 -lshlwapi

	#$SRC_ROOT/rtc_base/logging.cc
;;
4)
    echo "adio processing"

    SRC_ROOT=`pwd`/../../../svc_webrtc/bizconf_rtc_endpoint/webrtc-checkout/src
    gcc -o libwebrtc_base.dll  -shared -fPIC \
    -Wl,--output-def,./libwebrtc_base.def,--out-implib,./libwebrtc_base.lib \
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
    -msse4 \
    $SRC_ROOT/rtc_base/platform_file.cc \
    $SRC_ROOT/rtc_base/memory/aligned_malloc.cc \
    $SRC_ROOT/rtc_base/checks.cc \
    $SRC_ROOT/rtc_base/race_checker.cc \
    $SRC_ROOT/rtc_base/platform_thread_types.cc \
    $SRC_ROOT/rtc_base/event_tracer.cc \
    $SRC_ROOT/rtc_base/critical_section.cc \
    $SRC_ROOT/rtc_base/logging.cc \
    $SRC_ROOT/rtc_base/time_utils.cc \
    $SRC_ROOT/rtc_base/platform_thread.cc \
    $SRC_ROOT/rtc_base/event.cc \
    $SRC_ROOT/rtc_base/thread_checker_impl.cc \
    $SRC_ROOT/rtc_base/strings/string_builder.cc \
    $SRC_ROOT/rtc_base/string_encode.cc \
    $SRC_ROOT/rtc_base/string_to_number.cc \
    $SRC_ROOT/rtc_base/string_utils.cc \
    $SRC_ROOT/system_wrappers/source/metrics.cc \
    $SRC_ROOT/system_wrappers/source/cpu_features.cc \
    $SRC_ROOT/system_wrappers/source/cpu_info.cc \
    $SRC_ROOT/system_wrappers/source/field_trial.cc \
    $SRC_ROOT/third_party/rnnoise/src/kiss_fft.cc \
    $SRC_ROOT/third_party/rnnoise/src/rnn_vad_weights.cc \
    $SRC_ROOT/third_party/abseil-cpp/absl/types/bad_optional_access.cc \
    $SRC_ROOT/third_party/abseil-cpp/absl/base/internal/raw_logging.cc \
    $SRC_ROOT/third_party/libyuv/source/compare_common.cc \
    $SRC_ROOT/third_party/libyuv/source/compare_gcc.cc \
    $SRC_ROOT/third_party/libyuv/source/compare.cc \
    $SRC_ROOT/third_party/libyuv/source/convert_argb.cc \
    $SRC_ROOT/third_party/libyuv/source/convert_from_argb.cc \
    $SRC_ROOT/third_party/libyuv/source/convert_from.cc \
    $SRC_ROOT/third_party/libyuv/source/convert_jpeg.cc \
    $SRC_ROOT/third_party/libyuv/source/convert_to_argb.cc \
    $SRC_ROOT/third_party/libyuv/source/convert_to_i420.cc \
    $SRC_ROOT/third_party/libyuv/source/convert.cc \
    $SRC_ROOT/third_party/libyuv/source/cpu_id.cc \
    $SRC_ROOT/third_party/libyuv/source/mjpeg_decoder.cc \
    $SRC_ROOT/third_party/libyuv/source/mjpeg_validate.cc \
    $SRC_ROOT/third_party/libyuv/source/planar_functions.cc \
    $SRC_ROOT/third_party/libyuv/source/rotate_any.cc \
    $SRC_ROOT/third_party/libyuv/source/rotate_argb.cc \
    $SRC_ROOT/third_party/libyuv/source/rotate_common.cc \
    $SRC_ROOT/third_party/libyuv/source/rotate_gcc.cc \
    $SRC_ROOT/third_party/libyuv/source/rotate.cc \
    $SRC_ROOT/third_party/libyuv/source/row_any.cc \
    $SRC_ROOT/third_party/libyuv/source/row_common.cc \
    $SRC_ROOT/third_party/libyuv/source/row_gcc.cc \
    $SRC_ROOT/third_party/libyuv/source/scale_any.cc \
    $SRC_ROOT/third_party/libyuv/source/scale_argb.cc \
    $SRC_ROOT/third_party/libyuv/source/scale_common.cc \
    $SRC_ROOT/third_party/libyuv/source/scale_gcc.cc \
    $SRC_ROOT/third_party/libyuv/source/scale.cc \
    $SRC_ROOT/third_party/libyuv/source/video_common.cc \
    $SRC_ROOT/api/video/i420_buffer.cc \
    $SRC_ROOT/api/video/color_space.cc \
    $SRC_ROOT/api/video/hdr_metadata.cc \
    $SRC_ROOT/api/video/video_content_type.cc \
    $SRC_ROOT/api/video/video_frame_buffer.cc \
    $SRC_ROOT/api/video/video_frame.cc \
    $SRC_ROOT/api/video/video_source_interface.cc \
    $SRC_ROOT/api/video/video_timing.cc \
    $SRC_ROOT/common_video/i420_buffer_pool.cc \
    $SRC_ROOT/modules/third_party/fft/fft.c \
    $SRC_ROOT/api/audio/audio_frame.cc \
    $SRC_ROOT/api/audio/echo_canceller3_config.cc \
    $SRC_ROOT/api/audio_codecs/audio_encoder.cc \
    $SRC_ROOT/api/audio_codecs/audio_decoder.cc \
    $SRC_ROOT/audio/utility/audio_frame_operations.cc \
    $SRC_ROOT/common_audio/third_party/fft4g/fft4g.c \
    $SRC_ROOT/common_audio/third_party/spl_sqrt_floor/spl_sqrt_floor.c \
    $SRC_ROOT/common_audio/audio_converter.cc \
    $SRC_ROOT/common_audio/audio_util.cc \
    $SRC_ROOT/common_audio/channel_buffer.cc \
    $SRC_ROOT/common_audio/resampler/push_resampler.cc \
    $SRC_ROOT/common_audio/resampler/push_sinc_resampler.cc \
    $SRC_ROOT/common_audio/real_fourier_ooura.cc \
    $SRC_ROOT/common_audio/real_fourier.cc \
    $SRC_ROOT/common_audio/resampler/resampler.cc \
    $SRC_ROOT/common_audio/resampler/sinc_resampler.cc \
    $SRC_ROOT/common_audio/smoothing_filter.cc \
    $SRC_ROOT/common_audio/sparse_fir_filter.cc \
    $SRC_ROOT/common_audio/vad/vad.cc \
    $SRC_ROOT/common_audio/wav_file.cc \
    $SRC_ROOT/common_audio/wav_header.cc \
    $SRC_ROOT/common_audio/window_generator.cc \
    $SRC_ROOT/common_audio/signal_processing/auto_corr_to_refl_coef.c \
    $SRC_ROOT/common_audio/signal_processing/auto_correlation.c \
    $SRC_ROOT/common_audio/signal_processing/complex_bit_reverse.c \
    $SRC_ROOT/common_audio/signal_processing/complex_fft.c \
    $SRC_ROOT/common_audio/signal_processing/copy_set_operations.c \
    $SRC_ROOT/common_audio/signal_processing/cross_correlation.c \
    $SRC_ROOT/common_audio/signal_processing/division_operations.c \
    $SRC_ROOT/common_audio/signal_processing/downsample_fast.c \
    $SRC_ROOT/common_audio/signal_processing/energy.c \
    $SRC_ROOT/common_audio/signal_processing/filter_ar_fast_q12.c \
    $SRC_ROOT/common_audio/signal_processing/filter_ar.c \
    $SRC_ROOT/common_audio/signal_processing/filter_ma_fast_q12.c \
    $SRC_ROOT/common_audio/signal_processing/get_hanning_window.c \
    $SRC_ROOT/common_audio/signal_processing/get_scaling_square.c \
    $SRC_ROOT/common_audio/signal_processing/ilbc_specific_functions.c \
    $SRC_ROOT/common_audio/signal_processing/levinson_durbin.c \
    $SRC_ROOT/common_audio/signal_processing/lpc_to_refl_coef.c \
    $SRC_ROOT/common_audio/signal_processing/min_max_operations.c \
    $SRC_ROOT/common_audio/signal_processing/randomization_functions.c \
    $SRC_ROOT/common_audio/signal_processing/real_fft.c \
    $SRC_ROOT/common_audio/signal_processing/refl_coef_to_lpc.c \
    $SRC_ROOT/common_audio/signal_processing/resample_48khz.c \
    $SRC_ROOT/common_audio/signal_processing/resample_by_2_internal.c \
    $SRC_ROOT/common_audio/signal_processing/resample_by_2.c \
    $SRC_ROOT/common_audio/signal_processing/resample_fractional.c \
    $SRC_ROOT/common_audio/signal_processing/resample.c \
    $SRC_ROOT/common_audio/ring_buffer.c \
    $SRC_ROOT/common_audio/signal_processing/spl_init.c \
    $SRC_ROOT/common_audio/signal_processing/spl_inl.c \
    $SRC_ROOT/common_audio/signal_processing/spl_sqrt.c \
    $SRC_ROOT/common_audio/signal_processing/splitting_filter.c \
    $SRC_ROOT/common_audio/signal_processing/sqrt_of_one_minus_x_squared.c \
    $SRC_ROOT/common_audio/vad/vad_core.c \
    $SRC_ROOT/common_audio/vad/vad_filterbank.c \
    $SRC_ROOT/common_audio/vad/vad_gmm.c \
    $SRC_ROOT/common_audio/vad/vad_sp.c \
    $SRC_ROOT/common_audio/signal_processing/vector_scaling_operations.c \
    $SRC_ROOT/common_audio/vad/webrtc_vad.c \
    $SRC_ROOT/common_audio/signal_processing/dot_product_with_scale.cc \
    $SRC_ROOT/common_audio/fir_filter_sse.cc \
    $SRC_ROOT/common_audio/resampler/sinc_resampler_sse.cc \
    $SRC_ROOT/common_audio/fir_filter_c.cc \
    $SRC_ROOT/common_audio/fir_filter_factory.cc \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/audio_decoder_isac.cc \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/audio_encoder_isac.cc \
    $SRC_ROOT/modules/audio_coding/codecs/isac/locked_bandwidth_info.cc \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/filter_functions.c \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/isac_vad.c \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/pitch_estimator.c \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/pitch_filter.c \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/arith_routines_hist.c \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/arith_routines_logist.c \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/arith_routines.c \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/bandwidth_estimator.c \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/crc.c \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/decode_bwe.c \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/decode.c \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/encode_lpc_swb.c \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/encode.c \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/entropy_coding.c \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/filterbanks.c \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/intialize.c \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/isac.c \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/lattice.c \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/lpc_analysis.c \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/lpc_gain_swb_tables.c \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/lpc_shape_swb12_tables.c \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/lpc_shape_swb16_tables.c \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/lpc_tables.c \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/pitch_gain_tables.c \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/pitch_lag_tables.c \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/spectrum_ar_model_tables.c \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/transform.c \
    -I/D/msys64/mingw32/include \
	-I/D/msys64/mingw32/i686-w64-mingw32/include \
    -I$SRC_ROOT \
    -I$SRC_ROOT/third_party \
    -I$SRC_ROOT/third_party/googletest/src/googlemock/include \
    -I$SRC_ROOT/third_party/googletest/src/googletest/include \
    -I$SRC_ROOT/third_party/abseil-cpp \
    -I$SRC_ROOT/third_party/libyuv/include \
    -L/D/msys64/mingw32/lib \
	-L/D/msys64/mingw32/i686-w64-mingw32/lib \
	-L/usr/lib \
    -lz -lstdc++ -lm  -lwinmm
	#-lpthread -lSDL2 -lws2_32 -ldl -llzma -lbz2 \
	#-limagehlp -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32  -limm32 -lole32 \
	#-loleaut32 -lshell32 -lversion -luuid -lOpenGL32 \
	#-lwinspool -lcomctl32 -lcomdlg32 -lctl3d32 -ladvapi32 -lwsock32 -lkernel32 -luser32 \
	#-lrt -lstrmiids -lquartz -liconv -lvfw32 -lshlwapi

	#-msse3
    #-m3dnow
;;
5)
    #for av process test
    SRC_ROOT=`pwd`/../../../svc_webrtc/bizconf_rtc_endpoint/webrtc-checkout/src
    gcc -o libwebrtc.dll  -shared -fPIC \
    -Wl,--output-def,./libwebrtc.def,--out-implib,./libwebrtc.lib \
    -D WEBRTC_WIN \
    -D WEBRTC_NS_FLOAT \
    -D WEBRTC_APM_DEBUG_DUMP=0 \
    -D MSYS2_GXH \
    -mavx \
    -fpermissive \
    $SRC_ROOT/modules/video_processing/util/denoiser_filter_c.cc \
    $SRC_ROOT/modules/video_processing/util/denoiser_filter.cc \
    $SRC_ROOT/modules/video_processing/util/noise_estimation.cc \
    $SRC_ROOT/modules/video_processing/util/skin_detection.cc \
    $SRC_ROOT/modules/video_processing/video_denoiser.cc \
    $SRC_ROOT/modules/video_processing/util/denoiser_filter_sse2.cc \
    $SRC_ROOT/modules/audio_processing/aec/aec_resampler.cc \
    $SRC_ROOT/modules/audio_processing/aec/echo_cancellation.cc \
    $SRC_ROOT/modules/audio_processing/aec/aec_core_sse2.cc \
    $SRC_ROOT/modules/audio_processing/aec/aec_core.cc \
    $SRC_ROOT/modules/audio_processing/aec_dump/null_aec_dump_factory.cc \
    $SRC_ROOT/modules/audio_processing/aec3/adaptive_fir_filter.cc \
    $SRC_ROOT/modules/audio_processing/aec3/aec_state.cc \
    $SRC_ROOT/modules/audio_processing/aec3/aec3_common.cc \
    $SRC_ROOT/modules/audio_processing/aec3/aec3_fft.cc \
    $SRC_ROOT/modules/audio_processing/aec3/api_call_jitter_metrics.cc \
    $SRC_ROOT/modules/audio_processing/aec3/block_delay_buffer.cc \
    $SRC_ROOT/modules/audio_processing/aec3/block_framer.cc \
    $SRC_ROOT/modules/audio_processing/aec3/block_processor_metrics.cc \
    $SRC_ROOT/modules/audio_processing/aec3/block_processor.cc \
    $SRC_ROOT/modules/audio_processing/aec3/cascaded_biquad_filter.cc \
    $SRC_ROOT/modules/audio_processing/aec3/clockdrift_detector.cc \
    $SRC_ROOT/modules/audio_processing/aec3/comfort_noise_generator.cc \
    $SRC_ROOT/modules/audio_processing/aec3/decimator.cc \
    $SRC_ROOT/modules/audio_processing/aec3/downsampled_render_buffer.cc \
    $SRC_ROOT/modules/audio_processing/aec3/echo_audibility.cc \
    $SRC_ROOT/modules/audio_processing/aec3/echo_canceller3.cc \
    $SRC_ROOT/modules/audio_processing/aec3/echo_path_delay_estimator.cc \
    $SRC_ROOT/modules/audio_processing/aec3/echo_path_variability.cc \
    $SRC_ROOT/modules/audio_processing/aec3/echo_remover_metrics.cc \
    $SRC_ROOT/modules/audio_processing/aec3/echo_remover.cc \
    $SRC_ROOT/modules/audio_processing/aec3/erl_estimator.cc \
    $SRC_ROOT/modules/audio_processing/aec3/erle_estimator.cc \
    $SRC_ROOT/modules/audio_processing/aec3/fft_buffer.cc \
    $SRC_ROOT/modules/audio_processing/aec3/filter_analyzer.cc \
    $SRC_ROOT/modules/audio_processing/aec3/frame_blocker.cc \
    $SRC_ROOT/modules/audio_processing/aec3/fullband_erle_estimator.cc \
    $SRC_ROOT/modules/audio_processing/aec3/main_filter_update_gain.cc \
    $SRC_ROOT/modules/audio_processing/aec3/matched_filter_lag_aggregator.cc \
    $SRC_ROOT/modules/audio_processing/aec3/matched_filter.cc \
    $SRC_ROOT/modules/audio_processing/aec3/matrix_buffer.cc \
    $SRC_ROOT/modules/audio_processing/aec3/moving_average.cc \
    $SRC_ROOT/modules/audio_processing/aec3/render_buffer.cc \
    $SRC_ROOT/modules/audio_processing/aec3/render_delay_buffer.cc \
    $SRC_ROOT/modules/audio_processing/aec3/render_delay_controller_metrics.cc \
    $SRC_ROOT/modules/audio_processing/aec3/render_delay_controller.cc \
    $SRC_ROOT/modules/audio_processing/aec3/render_reverb_model.cc \
    $SRC_ROOT/modules/audio_processing/aec3/render_signal_analyzer.cc \
    $SRC_ROOT/modules/audio_processing/aec3/residual_echo_estimator.cc \
    $SRC_ROOT/modules/audio_processing/aec3/reverb_decay_estimator.cc \
    $SRC_ROOT/modules/audio_processing/aec3/reverb_frequency_response.cc \
    $SRC_ROOT/modules/audio_processing/aec3/reverb_model_estimator.cc \
    $SRC_ROOT/modules/audio_processing/aec3/reverb_model_fallback.cc \
    $SRC_ROOT/modules/audio_processing/aec3/reverb_model.cc \
    $SRC_ROOT/modules/audio_processing/aec3/shadow_filter_update_gain.cc \
    $SRC_ROOT/modules/audio_processing/aec3/signal_dependent_erle_estimator.cc \
    $SRC_ROOT/modules/audio_processing/aec3/skew_estimator.cc \
    $SRC_ROOT/modules/audio_processing/aec3/stationarity_estimator.cc \
    $SRC_ROOT/modules/audio_processing/aec3/subband_erle_estimator.cc \
    $SRC_ROOT/modules/audio_processing/aec3/subtractor_output_analyzer.cc \
    $SRC_ROOT/modules/audio_processing/aec3/subtractor_output.cc \
    $SRC_ROOT/modules/audio_processing/aec3/subtractor.cc \
    $SRC_ROOT/modules/audio_processing/aec3/suppression_filter.cc \
    $SRC_ROOT/modules/audio_processing/aec3/suppression_gain.cc \
    $SRC_ROOT/modules/audio_processing/aec3/vector_buffer.cc \
    $SRC_ROOT/modules/audio_processing/aecm/aecm_core_c.cc \
    $SRC_ROOT/modules/audio_processing/aecm/aecm_core.cc \
    $SRC_ROOT/modules/audio_processing/aecm/echo_control_mobile.cc \
    $SRC_ROOT/modules/audio_processing/agc/agc_manager_direct.cc \
    $SRC_ROOT/modules/audio_processing/agc/legacy/analog_agc.c \
    $SRC_ROOT/modules/audio_processing/agc/legacy/digital_agc.c \
    $SRC_ROOT/modules/audio_processing/agc/agc.cc \
    $SRC_ROOT/modules/audio_processing/agc/loudness_histogram.cc \
    $SRC_ROOT/modules/audio_processing/agc/utility.cc \
    $SRC_ROOT/modules/audio_processing/agc2/adaptive_agc.cc \
    $SRC_ROOT/modules/audio_processing/agc2/adaptive_digital_gain_applier.cc \
    $SRC_ROOT/modules/audio_processing/agc2/adaptive_mode_level_estimator.cc \
    $SRC_ROOT/modules/audio_processing/agc2/saturation_protector.cc \
    $SRC_ROOT/modules/audio_processing/agc2/biquad_filter.cc \
    $SRC_ROOT/modules/audio_processing/agc2/agc2_common.cc \
    $SRC_ROOT/modules/audio_processing/agc2/fixed_digital_level_estimator.cc \
    $SRC_ROOT/modules/audio_processing/agc2/interpolated_gain_curve.cc \
    $SRC_ROOT/modules/audio_processing/agc2/limiter.cc \
    $SRC_ROOT/modules/audio_processing/agc2/gain_applier.cc \
    $SRC_ROOT/modules/audio_processing/agc2/adaptive_mode_level_estimator_agc.cc \
    $SRC_ROOT/modules/audio_processing/agc2/down_sampler.cc \
    $SRC_ROOT/modules/audio_processing/agc2/noise_level_estimator.cc \
    $SRC_ROOT/modules/audio_processing/agc2/noise_spectrum_estimator.cc \
    $SRC_ROOT/modules/audio_processing/agc2/signal_classifier.cc \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/features_extraction.cc \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/fft_util.cc \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/lp_residual.cc \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/pitch_search_internal.cc \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/pitch_search.cc \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/rnn.cc \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/spectral_features_internal.cc \
    $SRC_ROOT/modules/audio_processing/agc2/rnn_vad/spectral_features.cc \
    $SRC_ROOT/modules/audio_processing/agc2/vad_with_level.cc \
    $SRC_ROOT/modules/audio_processing/include/audio_processing.cc \
    $SRC_ROOT/modules/audio_processing/logging/apm_data_dumper.cc \
    $SRC_ROOT/modules/audio_processing/audio_buffer.cc \
    $SRC_ROOT/modules/audio_processing/splitting_filter.cc \
    $SRC_ROOT/modules/audio_processing/three_band_filter_bank.cc \
    $SRC_ROOT/modules/audio_processing/include/aec_dump.cc \
    $SRC_ROOT/modules/audio_processing/audio_processing_impl.cc \
    $SRC_ROOT/modules/audio_processing/echo_detector/circular_buffer.cc \
    $SRC_ROOT/modules/audio_processing/echo_cancellation_impl.cc \
    $SRC_ROOT/modules/audio_processing/echo_control_mobile_impl.cc \
    $SRC_ROOT/modules/audio_processing/gain_control_for_experimental_agc.cc \
    $SRC_ROOT/modules/audio_processing/gain_control_impl.cc \
    $SRC_ROOT/modules/audio_processing/gain_controller2.cc \
    $SRC_ROOT/modules/audio_processing/level_estimator_impl.cc \
    $SRC_ROOT/modules/audio_processing/low_cut_filter.cc \
    $SRC_ROOT/modules/audio_processing/echo_detector/mean_variance_estimator.cc \
    $SRC_ROOT/modules/audio_processing/echo_detector/moving_max.cc \
    $SRC_ROOT/modules/audio_processing/transient/moving_moments.cc \
    $SRC_ROOT/modules/audio_processing/noise_suppression_impl.cc \
    $SRC_ROOT/modules/audio_processing/echo_detector/normalized_covariance_estimator.cc \
    $SRC_ROOT/modules/audio_processing/residual_echo_detector.cc \
    $SRC_ROOT/modules/audio_processing/rms_level.cc \
    $SRC_ROOT/modules/audio_processing/transient/transient_detector.cc \
    $SRC_ROOT/modules/audio_processing/transient/transient_suppressor.cc \
    $SRC_ROOT/modules/audio_processing/typing_detection.cc \
    $SRC_ROOT/modules/audio_processing/voice_detection_impl.cc \
    $SRC_ROOT/modules/audio_processing/transient/wpd_node.cc \
    $SRC_ROOT/modules/audio_processing/transient/wpd_tree.cc \
    $SRC_ROOT/modules/audio_processing/ns/noise_suppression.c \
    $SRC_ROOT/modules/audio_processing/ns/ns_core.c \
    $SRC_ROOT/modules/audio_processing/include/audio_processing_statistics.cc \
    $SRC_ROOT/modules/audio_processing/include/config.cc \
    $SRC_ROOT/modules/audio_processing/noise_suppression_proxy.cc \
    $SRC_ROOT/modules/audio_processing/utility/block_mean_calculator.cc \
    $SRC_ROOT/modules/audio_processing/utility/delay_estimator_wrapper.cc \
    $SRC_ROOT/modules/audio_processing/utility/delay_estimator.cc \
    $SRC_ROOT/modules/audio_processing/utility/ooura_fft_sse2.cc \
    $SRC_ROOT/modules/audio_processing/utility/ooura_fft.cc \
    $SRC_ROOT/modules/audio_processing/vad/gmm.cc \
    $SRC_ROOT/modules/audio_processing/vad/pitch_based_vad.cc \
    $SRC_ROOT/modules/audio_processing/vad/pitch_internal.cc \
    $SRC_ROOT/modules/audio_processing/vad/pole_zero_filter.cc \
    $SRC_ROOT/modules/audio_processing/vad/standalone_vad.cc \
    $SRC_ROOT/modules/audio_processing/vad/vad_audio_proc.cc \
    $SRC_ROOT/modules/audio_processing/vad/vad_circular_buffer.cc \
    $SRC_ROOT/modules/audio_processing/vad/voice_activity_detector.cc \
    -I/D/msys64/mingw32/include \
	-I/D/msys64/mingw32/i686-w64-mingw32/include \
    -I$SRC_ROOT \
    -I$SRC_ROOT/third_party \
    -I$SRC_ROOT/third_party/abseil-cpp \
    -I$SRC_ROOT/third_party/libyuv/include \
    -L./ \
    -L/D/msys64/mingw32/lib \
	-L/D/msys64/mingw32/i686-w64-mingw32/lib \
	-L/usr/lib \
	-lwebrtc_base \
    -lz -lstdc++ -lm  -lwinmm
	#-lpthread -lSDL2 -lws2_32 -ldl -llzma -lbz2 \
	#-limagehlp -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32  -limm32 -lole32 \
	#-loleaut32 -lshell32 -lversion -luuid -lOpenGL32 \
	#-lwinspool -lcomctl32 -lcomdlg32 -lctl3d32 -ladvapi32 -lwsock32 -lkernel32 -luser32 \
	#-lrt -lstrmiids -lquartz -liconv -lvfw32 -lshlwapi

	#
;;
6)
    #test
    gcc -o ./pycodec/libwebrtc.dll -shared -fPIC \
    -Wl,--output-def,./pycodec/libwebrtc.def,--out-implib,./pycodec/libwebrtc.lib \
    ./tmp/*.obj \
    -L/D/msys64/mingw32/lib \
	-L/D/msys64/mingw32/i686-w64-mingw32/lib \
	-L/usr/lib \
    -lz -lstdc++ -lm \
	-lpthread -lSDL2 -lws2_32 -ldl -llzma -lbz2 \
	-limagehlp -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 \
	-loleaut32 -lshell32 -lversion -luuid -lOpenGL32 \
	-lwinspool -lcomctl32 -lcomdlg32 -lctl3d32 -ladvapi32 -lwsock32 -lkernel32 -luser32 \
	-lrt -lstrmiids -lquartz -liconv -lvfw32 -lshlwapi
;;
7)
    #test
    SRC_ROOT=`pwd`/../../../svc_webrtc/bizconf_rtc_endpoint/webrtc-checkout/src
    gcc -o libwebrtc_base.dll  -shared -fPIC \

    # 1、准备工作，编译方式、目标文件名、依赖库路径的定义。
    CC = g++
    CFLAGS  := -Wall -O3 -std=c++0x \
    -Wl,--output-def,./libwebrtc_base.def,--out-implib,./libwebrtc_base.lib \
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

    # opencv 头文件和lib路径
    AVPROCESSING_INC_ROOT = /D/msys64/mingw32/include \
	/D/msys64/mingw32/i686-w64-mingw32/include \
    $SRC_ROOT \
    $SRC_ROOT/third_party \
    $SRC_ROOT/third_party/googletest/src/googlemock/include \
    $SRC_ROOT/third_party/googletest/src/googletest/include \
    $SRC_ROOT/third_party/abseil-cpp \
    $SRC_ROOT/third_party/libyuv/include

    AVPROCESSING_LIB_ROOT = /D/msys64/mingw32/lib \
	/D/msys64/mingw32/i686-w64-mingw32/lib \
	/usr/lib


    OBJS = $SRC_ROOT/rtc_base/platform_file.o \
    $SRC_ROOT/rtc_base/memory/aligned_malloc.o \
    $SRC_ROOT/rtc_base/checks.o \
    $SRC_ROOT/rtc_base/race_checker.o \
    $SRC_ROOT/rtc_base/platform_thread_types.o \
    $SRC_ROOT/rtc_base/event_tracer.o \
    $SRC_ROOT/rtc_base/critical_section.o \
    $SRC_ROOT/rtc_base/logging.o \
    $SRC_ROOT/rtc_base/time_utils.o \
    $SRC_ROOT/rtc_base/platform_thread.o \
    $SRC_ROOT/rtc_base/event.o \
    $SRC_ROOT/rtc_base/thread_checker_impl.o \
    $SRC_ROOT/rtc_base/strings/string_builder.o \
    $SRC_ROOT/rtc_base/string_encode.o \
    $SRC_ROOT/rtc_base/string_to_number.o \
    $SRC_ROOT/rtc_base/string_utils.o \
    $SRC_ROOT/system_wrappers/source/metrics.o \
    $SRC_ROOT/system_wrappers/source/cpu_features.o \
    $SRC_ROOT/system_wrappers/source/cpu_info.o \
    $SRC_ROOT/system_wrappers/source/field_trial.o \
    $SRC_ROOT/third_party/rnnoise/src/kiss_fft.o \
    $SRC_ROOT/third_party/rnnoise/src/rnn_vad_weights.o \
    $SRC_ROOT/third_party/abseil-cpp/absl/types/bad_optional_access.o \
    $SRC_ROOT/third_party/abseil-cpp/absl/base/internal/raw_logging.o \
    $SRC_ROOT/third_party/libyuv/source/compare_common.o \
    $SRC_ROOT/third_party/libyuv/source/compare_gcc.o \
    $SRC_ROOT/third_party/libyuv/source/compare.o \
    $SRC_ROOT/third_party/libyuv/source/convert_argb.o \
    $SRC_ROOT/third_party/libyuv/source/convert_from_argb.o \
    $SRC_ROOT/third_party/libyuv/source/convert_from.o \
    $SRC_ROOT/third_party/libyuv/source/convert_jpeg.o \
    $SRC_ROOT/third_party/libyuv/source/convert_to_argb.o \
    $SRC_ROOT/third_party/libyuv/source/convert_to_i420.o \
    $SRC_ROOT/third_party/libyuv/source/convert.o \
    $SRC_ROOT/third_party/libyuv/source/cpu_id.o \
    $SRC_ROOT/third_party/libyuv/source/mjpeg_decoder.o \
    $SRC_ROOT/third_party/libyuv/source/mjpeg_validate.o \
    $SRC_ROOT/third_party/libyuv/source/planar_functions.o \
    $SRC_ROOT/third_party/libyuv/source/rotate_any.o \
    $SRC_ROOT/third_party/libyuv/source/rotate_argb.o \
    $SRC_ROOT/third_party/libyuv/source/rotate_common.o \
    $SRC_ROOT/third_party/libyuv/source/rotate_gcc.o \
    $SRC_ROOT/third_party/libyuv/source/rotate.o \
    $SRC_ROOT/third_party/libyuv/source/row_any.o \
    $SRC_ROOT/third_party/libyuv/source/row_common.o \
    $SRC_ROOT/third_party/libyuv/source/row_gcc.o \
    $SRC_ROOT/third_party/libyuv/source/scale_any.o \
    $SRC_ROOT/third_party/libyuv/source/scale_argb.o \
    $SRC_ROOT/third_party/libyuv/source/scale_common.o \
    $SRC_ROOT/third_party/libyuv/source/scale_gcc.o \
    $SRC_ROOT/third_party/libyuv/source/scale.o \
    $SRC_ROOT/third_party/libyuv/source/video_common.o \
    $SRC_ROOT/api/video/i420_buffer.o \
    $SRC_ROOT/api/video/color_space.o \
    $SRC_ROOT/api/video/hdr_metadata.o \
    $SRC_ROOT/api/video/video_content_type.o \
    $SRC_ROOT/api/video/video_frame_buffer.o \
    $SRC_ROOT/api/video/video_frame.o \
    $SRC_ROOT/api/video/video_source_interface.o \
    $SRC_ROOT/api/video/video_timing.o \
    $SRC_ROOT/common_video/i420_buffer_pool.o \
    $SRC_ROOT/modules/third_party/fft/fft.o \
    $SRC_ROOT/api/audio/audio_frame.o \
    $SRC_ROOT/api/audio/echo_canceller3_config.o \
    $SRC_ROOT/api/audio_codecs/audio_encoder.o \
    $SRC_ROOT/api/audio_codecs/audio_decoder.o \
    $SRC_ROOT/audio/utility/audio_frame_operations.o \
    $SRC_ROOT/common_audio/third_party/fft4g/fft4g.o \
    $SRC_ROOT/common_audio/third_party/spl_sqrt_floor/spl_sqrt_floor.o \
    $SRC_ROOT/common_audio/audio_converter.o \
    $SRC_ROOT/common_audio/audio_util.o \
    $SRC_ROOT/common_audio/channel_buffer.o \
    $SRC_ROOT/common_audio/resampler/push_resampler.o \
    $SRC_ROOT/common_audio/resampler/push_sinc_resampler.o \
    $SRC_ROOT/common_audio/real_fourier_ooura.o \
    $SRC_ROOT/common_audio/real_fourier.o \
    $SRC_ROOT/common_audio/resampler/resampler.o \
    $SRC_ROOT/common_audio/resampler/sinc_resampler.o \
    $SRC_ROOT/common_audio/smoothing_filter.o \
    $SRC_ROOT/common_audio/sparse_fir_filter.o \
    $SRC_ROOT/common_audio/vad/vad.o \
    $SRC_ROOT/common_audio/wav_file.o \
    $SRC_ROOT/common_audio/wav_header.o \
    $SRC_ROOT/common_audio/window_generator.o \
    $SRC_ROOT/common_audio/signal_processing/auto_corr_to_refl_coef.o \
    $SRC_ROOT/common_audio/signal_processing/auto_correlation.o \
    $SRC_ROOT/common_audio/signal_processing/complex_bit_reverse.o \
    $SRC_ROOT/common_audio/signal_processing/complex_fft.o \
    $SRC_ROOT/common_audio/signal_processing/copy_set_operations.o \
    $SRC_ROOT/common_audio/signal_processing/cross_correlation.o \
    $SRC_ROOT/common_audio/signal_processing/division_operations.o \
    $SRC_ROOT/common_audio/signal_processing/downsample_fast.o \
    $SRC_ROOT/common_audio/signal_processing/energy.o \
    $SRC_ROOT/common_audio/signal_processing/filter_ar_fast_q12.o \
    $SRC_ROOT/common_audio/signal_processing/filter_ar.o \
    $SRC_ROOT/common_audio/signal_processing/filter_ma_fast_q12.o \
    $SRC_ROOT/common_audio/signal_processing/get_hanning_window.o \
    $SRC_ROOT/common_audio/signal_processing/get_scaling_square.o \
    $SRC_ROOT/common_audio/signal_processing/ilbc_specific_functions.o \
    $SRC_ROOT/common_audio/signal_processing/levinson_durbin.o \
    $SRC_ROOT/common_audio/signal_processing/lpc_to_refl_coef.o \
    $SRC_ROOT/common_audio/signal_processing/min_max_operations.o \
    $SRC_ROOT/common_audio/signal_processing/randomization_functions.o \
    $SRC_ROOT/common_audio/signal_processing/real_fft.o \
    $SRC_ROOT/common_audio/signal_processing/refl_coef_to_lpc.o \
    $SRC_ROOT/common_audio/signal_processing/resample_48khz.o \
    $SRC_ROOT/common_audio/signal_processing/resample_by_2_internal.o \
    $SRC_ROOT/common_audio/signal_processing/resample_by_2.o \
    $SRC_ROOT/common_audio/signal_processing/resample_fractional.o \
    $SRC_ROOT/common_audio/signal_processing/resample.o \
    $SRC_ROOT/common_audio/ring_buffer.o \
    $SRC_ROOT/common_audio/signal_processing/spl_init.o \
    $SRC_ROOT/common_audio/signal_processing/spl_inl.o \
    $SRC_ROOT/common_audio/signal_processing/spl_sqrt.o \
    $SRC_ROOT/common_audio/signal_processing/splitting_filter.o \
    $SRC_ROOT/common_audio/signal_processing/sqrt_of_one_minus_x_squared.o \
    $SRC_ROOT/common_audio/vad/vad_core.o \
    $SRC_ROOT/common_audio/vad/vad_filterbank.o \
    $SRC_ROOT/common_audio/vad/vad_gmm.o \
    $SRC_ROOT/common_audio/vad/vad_sp.o \
    $SRC_ROOT/common_audio/signal_processing/vector_scaling_operations.o \
    $SRC_ROOT/common_audio/vad/webrtc_vad.o \
    $SRC_ROOT/common_audio/signal_processing/dot_product_with_scale.o \
    $SRC_ROOT/common_audio/fir_filter_sse.o \
    $SRC_ROOT/common_audio/resampler/sinc_resampler_sse.o \
    $SRC_ROOT/common_audio/fir_filter_c.o \
    $SRC_ROOT/common_audio/fir_filter_factory.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/audio_decoder_isac.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/audio_encoder_isac.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/locked_bandwidth_info.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/filter_functions.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/isac_vad.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/pitch_estimator.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/pitch_filter.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/arith_routines_hist.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/arith_routines_logist.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/arith_routines.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/bandwidth_estimator.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/crc.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/decode_bwe.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/decode.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/encode_lpc_swb.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/encode.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/entropy_coding.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/filterbanks.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/intialize.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/isac.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/lattice.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/lpc_analysis.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/lpc_gain_swb_tables.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/lpc_shape_swb12_tables.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/lpc_shape_swb16_tables.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/lpc_tables.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/pitch_gain_tables.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/pitch_lag_tables.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/spectrum_ar_model_tables.o \
    $SRC_ROOT/modules/audio_coding/codecs/isac/main/source/transform.o \

    LIB = libwebrtc_base.a # 目标文件名

    AVPROCESSING_INC= -I $(AVPROCESSING_INC_ROOT)

    INCLUDE_PATH = $(AVPROCESSING_INC)

    LIB_PATH = -L $(AVPROCESSING_LIB_ROOT)

    # 依赖的lib名称
    AVPROCESSING_LIB = -lz -lstdc++ -lm  -lwinmm

    all : $(LIB)

    # 2. 生成.o文件
    %.o : %.cc %c
        $(CC) $(CFLAGS) -c $< -o $@ $(INCLUDE_PATH) $(LIB_PATH) $(AVPROCESSING_LIB)

    # 3. 生成静态库文件
    $(LIB) : $(OBJS)
        rm -f $@
        ar cr $@ $(OBJS)
        rm -f $(OBJS)

    tags :
         ctags -R *

    # 4. 删除中间过程生成的文件
    clean:
        rm -f $(OBJS) $(TARGET) $(LIB)
;;
esac
















// Sample Encoder
// ==============
//
// This is an sample of a simple encoder loop. It takes an input file in
// YV12 format, passes it through the encoder, and writes the compressed
// frames to disk.
//
// Standard Includes
// -----------------
// `wz264enc.h`
//
// Initializing The codec config
// Declare an instance of wz264_encoder_config_t, and set fields according
// to your encode speed and quality requirements.
//
// Create a codec
// ----------------------
// Call wz264encoder_open with the config filled above
//
// Encoding A Frame
// ----------------
// Fill YUV data in a wz264_video_sample_t object, and call wz264encoder_encode_frame
//
// Processing The Encoded Data
// ---------------------------
// The value of `pkt_cnt` indicates the number of nals in `pks`.
// We write raw data to output file.
//
// Cleanup
// -------
// The `wz264encoder_close` call frees any memory allocated by the codec.
//
#include "wz264/wz264enc.h"
#include "wz264_sample_utils.h"

static INLINE int wz_img_read(wz264_encoder_config_t *cfg, wz264_video_sample_t *sample,
                              FILE *file) {
  wz264_image_t *img = &sample->img;
  for (int plane = 0; plane < img->i_plane; ++plane) {
    unsigned char *buf = img->plane[plane];
    const int stride = img->i_stride[plane];
    const int w = cfg->width >> (plane ? 1 : 0);
    const int h = cfg->height >> (plane ? 1 : 0);
    for (int y = 0; y < h; ++y) {
      if (fread(buf, 1, w, file) != (size_t)w) return WZ_FAILED;
      buf += stride;
    }
  }
  return WZ_OK;
}

static INLINE void write_nals_to_file(wz264_nal_t *nals, int nal_cnt, wz264_video_sample_t *outpic,
                                      FILE *outfile, int debug_log) {
  if (debug_log && nal_cnt && outpic->is_keyframe) wz_printf(" +++key+++ \n");
  for (int i = 0; i < nal_cnt; ++i) {
    if (debug_log) {
      wz_printf("%6lld size %5d, ", outpic->pts, nals[i].i_payload);
    }
    fwrite(nals[i].p_payload, 1, nals[i].i_payload, outfile);
  }
}

int wz264_sample_encoder(int argc, char **argv) {
  const char *infile_arg = NULL;
  const char *outfile_arg = NULL;
  int argv_idx = 1;
  wz264_encoder_config_t cfg;
  wz264encoder_config_default(&cfg);

  cfg.preset = get_char_arg(argv, &argv_idx);
  if (!strncasecmp(argv[argv_idx], "0", 1)) {
    argv_idx++;
  } else {
    cfg.tune = get_char_arg(argv, &argv_idx);
  }

  cfg.width = get_int_arg(argv, &argv_idx);
  cfg.height = get_int_arg(argv, &argv_idx);
  const int fps = get_int_arg(argv, &argv_idx);
  infile_arg = get_char_arg(argv, &argv_idx);
  outfile_arg = get_char_arg(argv, &argv_idx);
  int keyframe_interval = get_int_arg(argv, &argv_idx);
  if (keyframe_interval < 0) wz_exit_on_err("Invalid key frame interval value.");
  cfg.keyint_max = keyframe_interval;

  cfg.rc_type = get_int_arg(argv, &argv_idx);
  if (cfg.rc_type == 0 || cfg.rc_type == 2) {
    int rc_target = get_int_arg(argv, &argv_idx);
    cfg.bitrate = cfg.qp = rc_target;
  } else if (cfg.rc_type == 1) {
    cfg.rf_constant = get_float_arg(argv, &argv_idx);
  }

  cfg.vbv_max_bitrate = get_int_arg(argv, &argv_idx);

  cfg.vbv_buffer_size = get_int_arg(argv, &argv_idx);

  cfg.bframe = get_int_arg(argv, &argv_idx);

  cfg.visual_opt = get_int_arg(argv, &argv_idx);

  cfg.threads = get_int_arg(argv, &argv_idx);
  const int max_frames = get_int_arg(argv, &argv_idx);
  const int min_argc = usage_msg ? count_usage_args(usage_msg) : argc - 1;
  const int debug_log = argc > min_argc ? get_int_arg(argv, &argv_idx) : 0;

  cfg.frame_rate_num = fps;
  cfg.frame_rate_den = 1;
  cfg.time_base_num = 1;
  cfg.time_base_den = fps;

  FILE *infile = fopen(infile_arg, "rb");
  if (!infile) {
    wz_exit_on_err("Failed to open %s for reading.", infile_arg);
  }
  FILE *outfile = fopen(outfile_arg, "wb");
  if (!outfile) {
    wz_exit_on_err("Failed to open %s for writing.", outfile_arg);
  }

  wz264_video_sample_t inpic;
  if (wz264_video_sample_alloc(&inpic, WZ264_CSP_I420, cfg.width, cfg.height) != WZ_OK) {
    wz_exit_on_err("Failed to allocate image.");
  }
  int32_t errcode;
  void *codec = wz264encoder_open(&cfg, &errcode);
  if (codec == NULL) {
    wz264_video_sample_free(&inpic);
    wz_exit_on_err("Failed to open codec %d ", errcode);
  }
  int64_t i_start = wz264_get_msec();
  wz264_nal_t *pkts = NULL;
  int32_t pkt_cnt = 0;
  // Get golbal header
  int ret = wz264encoder_headers(codec, &pkts, &pkt_cnt);
  if (ret < 0) {
    wz264_video_sample_free(&inpic);
    wz264encoder_close(codec);
    return EXIT_FAILURE;
  }
  // Encode frames.
  int frame_count = 0;
  int frames_encoded = 0;

  wz264_video_sample_t outpic;
  while (wz_img_read(&cfg, &inpic, infile) == WZ_OK) {
    frame_count += 1;
    inpic.pts += 1;
    int ret = wz264encoder_encode_frame(codec, &pkts, &pkt_cnt, &inpic, &outpic);
    if (ret < 0) {
      wz264encoder_close(codec);
      wz264_video_sample_free(&inpic);
      return EXIT_FAILURE;
    }
    if (debug_log) wz_printf("\n%3d: bytes %d  pkts %d ", frames_encoded, ret, pkt_cnt);
    write_nals_to_file(pkts, pkt_cnt, &outpic, outfile, debug_log);
    frames_encoded++;
    if (max_frames > 0 && frames_encoded >= max_frames) break;
  }

  // Flush encoder.
  do {
    ret = wz264encoder_encode_frame(codec, &pkts, &pkt_cnt, NULL, &outpic);
    if (debug_log)
      wz_printf("\nflushing: bytes %d  pkts %d  out pic %d ", ret, pkt_cnt, outpic.slice_type);
    if (pkt_cnt == 0) {
      break;
    }
    write_nals_to_file(pkts, pkt_cnt, &outpic, outfile, debug_log);
  } while (ret > WZ_OK);

  int64_t i_end = wz264_get_msec();
  double speed_fps = (double)frame_count * (double)1000000 / (double)(i_end - i_start);

  if (debug_log) wz_printf("\n");
  fclose(infile);
  fclose(outfile);
  wz_printf("Processed %d frames. %.2f fps\n", frame_count, speed_fps);
  wz264encoder_close(codec);
  wz264_video_sample_free(&inpic);
  return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
  exec_name = argv[0];
  usage_msg =
    "<preset> <tune> <width> <height> <fps-num> <in-yuv> <outfile> <keyframe-interval> "
    "<rc-type> <target-bitrate/crf/qp> <vbv-maxrate> <vbv-bufsize> <bframes> "
    "<visual-opt> <threads> <frames to encode> [debug-log]\n";
  int min_argc = count_usage_args(usage_msg);
  if (argc < min_argc) {
    wz_exit_on_err("Invalid number of arguments");
  }
  wz_printf("Welcome to use Visionular's H264 encoder %s %s\n", wz264_version_str(),
            wz264_version_extra_str());
  return wz264_sample_encoder(argc, argv);
}

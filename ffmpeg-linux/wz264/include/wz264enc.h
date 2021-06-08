#ifndef __WZ264_ENC_H__
#define __WZ264_ENC_H__
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif  //__cplusplus

#if defined(WIN32) || defined(_MSC_VER) || defined(_WIN32)
#define _h_dll_export __declspec(dllexport)
#else
#define _h_dll_export __attribute__((visibility("default")))
#endif

enum wz264_colorspace_e {
  WZ264_CSP_NONE = 0x0000, /* Invalid mode     */
  WZ264_CSP_I420 = 0x0001, /* yuv 4:2:0 planar */
  WZ264_CSP_YV12 = 0x0002, /* yvu 4:2:0 planar */
  WZ264_CSP_NV12 = 0x0003, /* yuv 4:2:0, with one y plane and one packed u+v */
  WZ264_CSP_NV21 = 0x0004, /* yuv 4:2:0, with one y plane and one packed v+u */
  WZ264_CSP_I422 = 0x0005, /* yuv 4:2:2 planar */
  WZ264_CSP_YV16 = 0x0006, /* yvu 4:2:2 planar */
  WZ264_CSP_NV16 = 0x0007, /* yuv 4:2:2, with one y plane and one packed u+v */
  WZ264_CSP_YUYV = 0x0008, /* yuyv 4:2:2 packed */
  WZ264_CSP_UYVY = 0x0009, /* uyvy 4:2:2 packed */
  WZ264_CSP_V210 = 0x000a, /* 10-bit yuv 4:2:2 packed in 32 */
  WZ264_CSP_I444 = 0x000b, /* yuv 4:4:4 planar */
  WZ264_CSP_YV24 = 0x000c, /* yvu 4:4:4 planar */
  WZ264_CSP_BGR = 0x000d,  /* packed bgr 24bits */
  WZ264_CSP_BGRA = 0x000e, /* packed bgr 32bits */
  WZ264_CSP_RGB = 0x000f,  /* packed rgb 24bits */
  WZ264_CSP_MAX = 0x0010,  /* end of list */
};

enum wz264_slice_type_e {
  WZ264_TYPE_AUTO = 0x0000, /* Let wz264 choose the right type */
  WZ264_TYPE_IDR = 0x0001,
  WZ264_TYPE_I = 0x0002,
  WZ264_TYPE_P = 0x0003,
  WZ264_TYPE_BREF = 0x0004, /* Non-disposable B-frame */
  WZ264_TYPE_B = 0x0005,
  WZ264_TYPE_KEYFRAME = 0x0006, /* IDR or I depending on b_open_gop option */
};

typedef struct wz264_image_t {
  int i_csp;         /* Colorspace */
  int i_plane;       /* Number of image planes */
  int i_stride[4];   /* Strides for each plane */
  uint8_t *plane[4]; /* Pointers to each plane */
} wz264_image_t;

typedef struct wz264_image_properties_t {
  /* All arrays of data here are ordered as follows:
   * each array contains one offset per macroblock, in raster scan order.  In interlaced
   * mode, top-field MBs and bottom-field MBs are interleaved at the row level.
   * Macroblocks are 16x16 blocks of pixels (with respect to the luma plane).  For the
   * purposes of calculating the number of macroblocks, width and height are rounded up to
   * the nearest 16.  If in interlaced mode, height is rounded up to the nearest 32 instead. */

  /* In: an array of quantizer offsets to be applied to this image during encoding.
   *     These are added on top of the decisions made by wz264.
   *     Offsets can be fractional; they are added before QPs are rounded to integer.
   *     Adaptive quantization must be enabled to use this feature.  Behavior if quant
   *     offsets differ between encoding passes is undefined. */
  float *quant_offsets;
  /* In: optional callback to free quant_offsets when used.
   *     Useful if one wants to use a different quant_offset array for each frame. */
  void (*quant_offsets_free)(void *);

  /* In: optional array of flags for each macroblock.
   *     Allows specifying additional information for the encoder such as which macroblocks
   *     remain unchanged.  Usable flags are listed below.
   *     wz264_param_t.analyse.b_mb_info must be set to use this, since wz264 needs to track
   *     extra data internally to make full use of this information.
   *
   * Out: if b_mb_info_update is set, wz264 will update this array as a result of encoding.
   *
   *      For "MBINFO_CONSTANT", it will remove this flag on any macroblock whose decoded
   *      pixels have changed.  This can be useful for e.g. noting which areas of the
   *      frame need to actually be blitted. Note: this intentionally ignores the effects
   *      of deblocking for the current frame, which should be fine unless one needs exact
   *      pixel-perfect accuracy.
   *
   *      Results for MBINFO_CONSTANT are currently only set for P-frames, and are not
   *      guaranteed to enumerate all blocks which haven't changed.  (There may be false
   *      negatives, but no false positives.)
   */
  uint8_t *mb_info;
  /* In: optional callback to free mb_info when used. */
  void (*mb_info_free)(void *);

/* The macroblock is constant and remains unchanged from the previous frame. */
#define WZ264_MBINFO_CONSTANT (1 << 0)
  /* More flags may be added in the future. */

  /* Out: SSIM of the the frame luma (if wz264_param_t.b_ssim is set) */
  double f_ssim;
  /* Out: Average PSNR of the frame (if wz264_param_t.b_psnr is set) */
  double f_psnr_avg;
  /* Out: PSNR of Y, U, and V (if wz264_param_t.b_psnr is set) */
  double f_psnr[3];

  /* Out: Average effective CRF of the encoded frame */
  double f_crf_avg;
} wz264_image_properties_t;

// input frame data and info
typedef struct wz264_video_sample_t {
  int slice_type;   // specified by output pictures
  int is_keyframe;  // Out: whether this frame is a key-frame.
  /* Out: when temporal scalable is enable, this is used to indicate temporal layer
     0 is the most important layer */
  int i_tempolayer;
  long long pts;
  long long dts;
  wz264_image_t img;
  wz264_image_properties_t prop;
  // private user data. copied from input to output frames.
  void *opaque;
} wz264_video_sample_t;

/**
 *  Alloc data for a video sample. Must call wz264_video_sample_free before exit.
 * @param sample   pointer to video sample
 * @param csp  colorspace
 * @param width  width of this video sample
 * @param height height of this video sample
 * @return  0 on success, or -1 on malloc failure or invalid colorspace.
 */
_h_dll_export int wz264_video_sample_alloc(wz264_video_sample_t *sample, int csp, int width,
                                           int height);

/* wz264_video_sample_free:
 *  free associated resource for a wz264_video_sample_t allocated with
 *  wz264_video_sample_alloc ONLY */

/**
 *  Free associated resource for a wz264_video_sample_t allocated with
 *  wz264_video_sample_alloc ONLY!
 * @param sample   pointer to video sample
 */
_h_dll_export void wz264_video_sample_free(wz264_video_sample_t *sample);

enum nal_unit_type_e {
  NAL_UNKNOWN = 0,
  NAL_SLICE = 1,
  NAL_SLICE_DPA = 2,
  NAL_SLICE_DPB = 3,
  NAL_SLICE_DPC = 4,
  NAL_SLICE_IDR = 5, /* ref_idc != 0 */
  NAL_SEI = 6,       /* ref_idc == 0 */
  NAL_SPS = 7,
  NAL_PPS = 8,
  NAL_AUD = 9,
  NAL_FILLER = 12,
  /* ref_idc == 0 for 6,9,10,11,12 */
};

enum nal_priority_e {
  NAL_PRIORITY_DISPOSABLE = 0,
  NAL_PRIORITY_LOW = 1,
  NAL_PRIORITY_HIGH = 2,
  NAL_PRIORITY_HIGHEST = 3,
};

/* The data within the payload is already NAL-encapsulated; the ref_idc and type
 * are merely in the struct for easy access by the calling application.
 * All data returned in an wz264_nal_t, including the data in p_payload, is no longer
 * valid after the next call to wz264encoder_encode.  Thus it must be used or copied
 * before calling wz264encoder_encode or wz264_encoder_headers again. */
typedef struct wz264_nal_t {
  int i_ref_idc; /* nal_priority_e */
  int i_type;    /* nal_unit_type_e */
  int b_long_startcode;
  int i_first_mb; /* If this NAL is a slice, the index of the first MB in the slice. */
  int i_last_mb;  /* If this NAL is a slice, the index of the last MB in the slice. */

  /* Size of payload (including any padding) in bytes. */
  int i_payload;
  /* If config->annexb is set, Annex-B bytestream with startcode.
   * Otherwise, startcode is replaced with a 4-byte size(equal to i_payload-4) */
  uint8_t *p_payload;

  /* Size of padding in bytes. */
  int i_padding;
} wz264_nal_t;

typedef struct wz264_vui_t {
  /* they will be reduced to be 0 < x <= 65535 and prime */
  int i_sar_height;
  int i_sar_width;

  int i_overscan; /* 0=undef, 1=no overscan, 2=overscan */

  /* see h264 annex E for the values of the following */
  int i_vidformat;
  int b_fullrange;
  int i_colorprim;
  int i_transfer;
  int i_colmatrix;
  int i_chroma_loc; /* both top & bottom */
} wz264_vui_t;

enum wz264_rc_type_e {
  WZ264_RC_CQP = 0,
  WZ264_RC_CRF = 1,
  WZ264_RC_ABR = 2,
};

enum wz264_log_level_e {
  WZ264_LOG_NONE = (-1),
  WZ264_LOG_ERROR = 0,
  WZ264_LOG_WARNING = 1,
  WZ264_LOG_INFO = 2,
  WZ264_LOG_DEBUG = 3,
};

enum {
  WZ_OK = (0x00000000),             // Success codes
  WZ_NO_MORE_FRAME = (0x00000001),  // No more cached frame

  WZ_FAILED = (0xFFFFFFFF),        //  Unspecified error
  WZ_FAIL = (0xFFFFFFFF),          //  Unspecified error
  WZ_OUTOFMEMORY = (0x80000002),   //  Ran out of memory
  WZ_POINTER = (0x80000003),       //  Invalid pointer
  WZ_NOTSUPPORTED = (0x80000004),  //  NOT support feature
  WZ_EXPIRED = (0x80000005),       //  SDK expired, please renew

  // wz264encoder_config_parse error
  WZ264_PARAM_BAD_NAME = (0x80000006),
  WZ264_PARAM_BAD_VALUE = (0x80000007),
};

/*!\brief Return the version information (as a string)
 *
 * Returns a printable string containing the full library version number.
 */
_h_dll_export const char *wz264_version_str(void);

/*!\brief Return extra version information (as a string)
 *
 * Returns a printable "extra string".
 * This is the short commit id of current version.
 *
 */
_h_dll_export const char *wz264_version_extra_str(void);

static const char *const wz264_preset_names[] = { "ultrafast", "superfast", "veryfast", "faster",
                                                  "fast",      "medium",    "slow",     "slower",
                                                  "veryslow",  "superslow", "placebo",  0 };

/* Multiple tunings can be used if separated by a delimiter in ",./-+" */
static const char *const wz264_tune_names[] = { "film",       "animation",   "grain",
                                                "stillimage", "psnr",        "ssim",
                                                "fastdecode", "screen",      "semi_screen",
                                                "arm",        "zerolatency", 0 };

/* Currently available profiles are, from most to least restrictive: */
static const char *const wz264_profile_names[] = { "baseline", "main",    "high", "high10",
                                                   "high422",  "high444", 0 };

typedef struct WZ264_ENCODER_CONFIG {
  const char *preset;   // codec preset for speed level
  const char *tune;     // codec tune for scene
  const char *profile;  //  Applies the restrictions of the given profile(can be NULL)

  int bitdepth;
  int csp;
  int width;   // image origin width
  int height;  // image origin height
  wz264_vui_t vui;

  int threads;              // encoder threads number
  int vfr_input;            // VFR. In rate-control 1: use timebase and pts 0(default): use fps only
  uint32_t frame_rate_num;  // frame rate numerator
  uint32_t frame_rate_den;  // frame rate denominator,default should be 1
  uint32_t time_base_num;   // time base numerator, default should be 1
  uint32_t time_base_den;   // time base denominator, time scale of pts

  int rc_type;            // rate control method type (see wz264_rc_type_e).
  float rf_constant;      // 1pass VBR, nominal QP
  float rf_constant_max;  // In CRF mode, maximum CRF as caused by VBV
  float rate_tolerance;
  int bitrate;            // bit rate(kbps), valid when rc type is WZ264_RC_ABR
  int vbv_max_bitrate;    // max rate(kbps) of vbv, 0 means vbv not enabled,default 0
  int vbv_buffer_size;    // buffer size(kbit) of vbv, 0 means vbv not enabled,default 0
  float vbv_buffer_init;  // <=1: fraction of buffer_size. >1: kbit
  int tempo_scalable;     /* enable temporal scalability up to 4 layers video
                           * sequence, nal_ref_idc 3 to 0 means highest to lowest
                           * layer, combining with slice's nal_type can solely
                           * identify target frame, used with some constrains.
                           * 0 - bypass, 1: 2 layers, 2: 3 layers, 3: 4 layers */
  float iboost;
  float ipratio;

  int qp;     // qp , only valid when rc_type is WZ264_RC_CQP
  int qpmin;  // minimal qp, valid when rc_type is not WZ264_RC_CQP
  int qpmax;  // maximal qp, valid when rc_type is not WZ264_RC_CQP

  int keyint_max;      // max dist between key frames
  int keyint_min;      // min dist between key frames
  int repeat_headers;  // put SPS/PPS before each keyframe
  int annexb;          // 1: place start codes (4 bytes) before NAL units,
                       // 0: place size (4 bytes) before NAL units.

  int bframe;     // number of b frames [0, 16]
  int lookahead;  // lookahead depth [0, 65]
  int open_gop;
  int intra_refresh;

  // analyser
  int subpel_refine;       // subpixel motion estimation quality
  int visual_opt;          // 0, 1, 2
  int log2_max_frame_num;  // [4, 16]

  // debug
  int b_psnr;  // compute and print PSNR stats
  int log_level;
} wz264_encoder_config_t;

/**
 * Fill config with default values.
 * @param cfg   config to be filled
 */
_h_dll_export void wz264encoder_config_default(wz264_encoder_config_t *cfg);

/**
 * Set one config by name.
 * @param encoder   handle of encoder
 * @param name   name of the config item
 * @param value  value of the config item
 * @return Returns 0 on success, or returns BAD_NAME or BAD_VALUE
 *  note: BAD_VALUE occurs only if it can't even parse the value,
 *  numerical range is not checked until wz264encoder_update_config().
 *  value=NULL means "true" for boolean options, but is a BAD_VALUE for non-booleans.
 */
_h_dll_export int wz264encoder_config_parse(void *encoder, const char *name, const char *value);

/**
 * create encoder
 * @param cfg : base config of encoder
 * @param errorCode: error code
 * @return encoder handle
 */
_h_dll_export void *wz264encoder_open(wz264_encoder_config_t *cfg, int32_t *errorCode);

/**
 * destroy encoder
 * @param encoder   handle of encoder
 * @return error code
 */
_h_dll_export int32_t wz264encoder_close(void *encoder);

/**
 * parse extra string config
 *
 * @param encoder   handle of encoder
 * @param cfg : base config of encoder
 * @return if succeed, return 0; if failed, return the error code
 */
_h_dll_export int32_t wz264encoder_update_config(void *encoder, wz264_encoder_config_t *cfg);

/**
 *  Get the SPS and PPS that will be used for the whole stream.
 *
 * @param encoder   handle of encoder
 * @param nals : pointer array of output nal units
 * @param nal_cnt : is the number of NAL units outputted in nals.
 * @return the number of bytes in the returned NALs.
 *      returns negative on error.
 */
_h_dll_export int wz264encoder_headers(void *encoder, wz264_nal_t **nals, int *nal_cnt);

/**
 * Encode one frame
 *
 * @param encoder   handle of encoder
 * @param nals      pointer array of output nal units (the payloads of all output NALs are
 * guaranteed to be sequential in memory.)
 * @param nal_cnt   output nal unit count
 * @param inpic    input frame, could be NULL for flush data
 * @param outpic
 * @return  Returns the number of bytes in the returned NALs.
 *          Returns negative on error.
 *          Returns zero if no NAL units returned.
 */
_h_dll_export int wz264encoder_encode_frame(void *encoder, wz264_nal_t **nals, int *nal_cnt,
                                            wz264_video_sample_t *inpic,
                                            wz264_video_sample_t *outpic);

/**
 * Request an intra refresh
 *
 *      If an intra refresh is not in progress, begin one with the next P-frame.
 *      If an intra refresh is in progress, begin one as soon as the current one finishes.
 */
_h_dll_export void wz264encoder_intra_refresh(void *encoder);

/**
 * Query then number of currently delayed (buffered) frames
 *
 * @param encoder: error code
 * @return delayed frames
 */
_h_dll_export int wz264encoder_delayed_frames(void *encoder);

/**
 *  Query the maximum number of delayed (buffered) frames that can occur with the current
 * parameters.
 *
 * @param errorCode: error code
 * @return maximum delayed frames
 */
_h_dll_export int wz264encoder_maximum_delayed_frames(void *encoder);

#if defined(__cplusplus)
}
#endif  //__cplusplus
#endif  //__WZ264_ENC_H__

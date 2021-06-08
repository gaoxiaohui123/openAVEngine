# WZ264编码器SDK文档

---

[TOC]


## 生成默认配置参数

```c
void wz264encoder_config_default(wz264_encoder_config_t *cfg);
```

- 功能：初始化默认配置参数
- 返回值: 无
- 参数:
- wz264_encoder_config_t* cfg: 基本配置参数指针

##  创建编码器

```c
void *wz264encoder_open(wz264_encoder_config_t *cfg, int32_t *errorCode);
```

- 功能：创建一个编码器实例
- 返回值：编码器实例的指针
- 参数:
   - cfg 编码器基本配置参数
   - errorCode 返回错误码

注意: 调用该接口前, 传入的cfg需要：
1. 通过调用`wz264encoder_config_default`填入默认值；
2. 根据输入图像的信息修改宽高、帧率等信息；
3. 码控模式和目标码率等信息
4. 根据应用需要填写preset和tune
5. 其他的建议保持默认值

### 编码器基本配置参数

```c
enum wz264_rc_type_e {
  WZ264_RC_CQP = 0,
  WZ264_RC_CRF = 1,
  WZ264_RC_ABR = 2,
};


static const char *const wz264_preset_names[] = {
  "ultrafast", "superfast", "veryfast", "faster", "fast", "medium", "slow", "slower", "veryslow", "placebo", 0
};

static const char * const wz264_tune_names[] = { 
  "film", "animation", "grain", "stillimage", "psnr", "ssim", "fastdecode", "screen", "zerolatency", 0 
};

/* Currently available profiles are, from most to least restrictive: */
static const char *const wz264_profile_names[] = {"baseline", "main", "high", "high10", "high422", "high444", 0};

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
  int subpel_refine;  // subpixel motion estimation quality
  int visual_opt;     // 0, 1, 2

  int log_level;
} wz264_encoder_config_t;
```

preset: 速度档次. 支持从ultrafast到placebo, 默认值为medium 
tune: 应用场景. 多个tune字符串可以使用",./-+"分隔符拼接起来
tempo_scalable: 大于0表示打开时间分级SVC，1表示有两级(temporal layer分别为0和1的图像层级)，2表示有3级，3表示有4级。
log_level: 日志等级。默认为WZ264_LOG_INFO
```c
enum wz264_log_level_e {
  WZ264_LOG_NONE = (-1),
  WZ264_LOG_ERROR = 0,
  WZ264_LOG_WARNING = 1,
  WZ264_LOG_INFO = 2,
  WZ264_LOG_DEBUG = 3,
};
```

## 改变编码配置参数

```c
int32_t wz264encoder_update_config(void *encoder, wz264_encoder_config_t *cfg);
```

- 功能：改变基本配置参数，主要用于码率的调整。
- 返回值：错误码, 0表示正常


## 销毁编码器

```c
int32_t wz264encoder_close(void *encoder);
```


## 编码一帧图像

```c
int wz264encoder_encode_frame(void *encoder, wz264_nal_t **nals, int *nal_cnt, wz264_video_sample_t *inpic, wz264_video_sample_t *outpic);
```

- 功能：编码一帧YUV图像
- 返回值：编码输出的byte数。等于0表示没有输出。负值表示发生错误或者编码结束。
- 参数:
- void *encoder:  编码器实例的指针
- wz264_nal_t **nals: 编码生成的NAL单元的指针, nals[i]表示第i个NAL单元的指针。当有多个nal单元时，数据在内存中保持连续。
- int *nal_cnt:  编码生成的NAL单元的数目
- wz264_video_sample_t *inpic: 输入YUV图像结构体指针, 与输出nals不同步。inpic为NULL时，表示flush编码缓存中还没有编完的数据
- wz264_video_sample_t *outpic: 输出图像帧信息, 作为输出nals的辅助信息, 与输出nals同步. 仅为参考使用, 丢弃不影响正常编码

### 输出NAL单元

```c
typedef struct wz264_nal_t {
  int i_ref_idc;  /* nal_priority_e */
  int i_type;     /* nal_unit_type_e */
  int b_long_startcode;
  int i_first_mb; /* If this NAL is a slice, the index of the first MB in the slice. */
  int i_last_mb;  /* If this NAL is a slice, the index of the last MB in the slice. */

  /* Size of payload (including any padding) in bytes. */
  int     i_payload;
  /* If config->annexb is set, Annex-B bytestream with startcode.
    * Otherwise, startcode is replaced with a 4-byte size(equal to i_payload-4) */
  uint8_t *p_payload;

  /* Size of padding in bytes. */
  int i_padding;
} wz264_nal_t;
```

### 图像帧信息

```c

typedef struct wz264_image_t {
  int i_csp;         /* Colorspace */
  int i_plane;       /* Number of image planes */
  int i_stride[4];   /* Strides for each plane */
  uint8_t *plane[4]; /* Pointers to each plane */
} wz264_image_t;

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
  // private user data. copied from input to output frames.
  void *opaque;
} wz264_video_sample_t;
```

unsigned char* plane[3]：YUV数据Buffer的起始地址
int i_stride[3]：YUV数据Buffer像素行间的步长, 大于等于宽度
int slice_type：图像slice类型, 输入时不用指定, 默认为0即可
int i_tempolayer: 当tempo_scalable大于0时，输出图像的temporal layer属性。0为最重要的层级，1次之，如此类推。
unsigned int pts：显示时间标签
unsigned int dts：解码时间标签, 输入时不用指定
void* opaque: 置为NULL即可

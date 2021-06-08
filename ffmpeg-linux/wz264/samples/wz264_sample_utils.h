#ifndef __WZ264_SAMPLE_UTILS_H__
#define __WZ264_SAMPLE_UTILS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdarg.h>
#include <assert.h>

#if _WIN32
#include <sys/types.h>
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif
#include <time.h>

#if defined(__cplusplus)
extern "C" {
#endif  //__cplusplus

#if __ANDROID_LOG__
#include <android/log.h>
#define wz_printf(fmt, ...) __android_log_print(ANDROID_LOG_INFO, "wz264jni", fmt, ##__VA_ARGS__);
#define wz_error(fmt, ...) __android_log_print(ANDROID_LOG_ERROR, "wz264jni", fmt, ##__VA_ARGS__);
#define wz_exit_on_err(fmt, ...)                                            \
  do {                                                                      \
    __android_log_print(ANDROID_LOG_ERROR, "wz264jni", fmt, ##__VA_ARGS__); \
    usage();                                                                \
  } while (0);
#else
#define wz_printf(fmt, ...) printf(fmt, ##__VA_ARGS__);
#define wz_error(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__);
#define wz_exit_on_err(fmt, ...)         \
  do {                                   \
    fprintf(stderr, fmt, ##__VA_ARGS__); \
    fprintf(stderr, "\n");               \
    usage();                             \
  } while (0);

#endif

#define INLINE __inline
#define WZ_ALIGN(x, n) (((x) + (n)-1) & ~((n)-1))

#ifdef _MSC_VER
#define strncasecmp _strnicmp
#else
#include <strings.h>
#endif

static const char *exec_name;
static const char *usage_msg;
static INLINE void usage() {
  wz_error("Usage: %s %s", exec_name, usage_msg);

  char buf[2048] = { 0 };
  for (size_t i = 0; wz264_preset_names[i]; i++) {
    sprintf(buf, "%s %s", buf, wz264_preset_names[i]);
  }
  wz_error("\npresets: %s ", buf);
  memset(buf, 0, sizeof(buf));
  for (size_t i = 0; wz264_tune_names[i]; i++) {
    sprintf(buf, "%s %s", buf, wz264_tune_names[i]);
  }
  wz_error("\ntune: %s ", buf);
  exit(EXIT_FAILURE);
}

static INLINE int count_usage_args(const char *s) {
  int i;
  for (i = 0; s[i];) s[i] == '<' ? i++ : *s++;
  return i + 1;
}

static INLINE int get_int_arg(char **argv, int *idx) {
  return (int)strtol(argv[(*idx)++], NULL, 0);
}
static INLINE float get_float_arg(char **argv, int *idx) {
  return (float)strtod(argv[(*idx)++], NULL);
}

static INLINE const char *get_char_arg(char **argv, int *idx) { return argv[(*idx)++]; }

static INLINE int64_t wz264_get_msec(void) {
#if _WIN32
  struct timeb tb;
  ftime(&tb);
  return ((int64_t)tb.time * 1000 + (int64_t)tb.millitm) * 1000;
#else
  struct timeval tv_date;
  gettimeofday(&tv_date, NULL);
  return (int64_t)tv_date.tv_sec * 1000000 + (int64_t)tv_date.tv_usec;
#endif
}

#if defined(__cplusplus)
}
#endif  //__cplusplus
#endif  //__WZ264_SAMPLE_UTILS_H__

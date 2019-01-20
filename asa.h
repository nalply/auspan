#include <fftw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/stat.h>


// TODO: autogenerate
#define VERSION "0.0.1"

typedef int16_t s16le_t;

typedef struct asa_struct_t {
  int in_fd;
  int out_fd;
  size_t size;
  size_t num_spans;
  size_t num_bands;
  double *bands;
  double max;
  void *pcm_s16le_buffer;
  void (*window)(double*);
  double *r;
  fftw_complex *c;
  fftw_plan plan;
} *asa_t;

extern void asa_init_fft(asa_t asa);

extern void asa_run_fft(asa_t asa);

extern int asa_read_span(asa_t asa);

extern void asa_convert_real(asa_t asa);

extern void asa_bands(asa_t asa);

extern void asa_write_spectrum(asa_t asa);

extern void asa_cleanup(asa_t asa);

extern int asa_dbg_enabled, asa_trc_enabled;

extern void asa_init_dbg();

extern char *asa_time();

#define _asa_out(ansi_color, code, time, fmt, ...) { \
  fprintf(stderr, "\e[%sm%s\e[m %s%s:%d ", \
    ansi_color, code, time, __FILE__, __LINE__); \
  fprintf(stderr, fmt, ## __VA_ARGS__); \
  fputc('\n', stderr); \
}

#define asa_trc(fmt, ...) { if (asa_trc_enabled) \
  _asa_out("36", "T", asa_time(), fmt, ## __VA_ARGS__); \
}

#define asa_dbg(fmt, ...) { if (asa_dbg_enabled) \
  _asa_out("33", "D", asa_time(), fmt, ## __VA_ARGS__); \
}

#define asa_info(fmt, ...) { \
  asa_dbg("invocation site of asa_info():"); \
  _asa_out("32", "Info", "", fmt, ## __VA_ARGS__); \
}

#define asa_error(fmt, ...) { \
  asa_dbg("invocation site of asa_error():"); \
  _asa_out("31", "Error", "", fmt, ## __VA_ARGS__); \
  exit(1); \
}

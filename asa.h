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
#define PROGRAM "asa-s16le"
#define VERSION "0.0.3"
#define COMPILE PROGRAM " " VERSION " compiled at " __DATE__ " " __TIME__

#define W_BOXCAR         0
#define W_HANN           1
#define W_FLATTOP        2
#define W_BLACKMANHARRIS 3
#define W_FIRST          W_BOXCAR
#define W_LAST           W_BLACKMANHARRIS

extern const char* window_names[];

extern const int x;

#define C_SIZE 1000

typedef struct asa_param_t { //                          limits
  int s;         // number of samples in a sequence      1 <= s <= x
  int n;         // fft input size                       s <= n <= x 
  int m;         // fft output size                      m = 1 + n / 2
  int b0;        // index of first bin                   0 <= b0 <= b1
  int b1;        // index of last bin                    b0 <= b1 <= m
  int b;         // number of bins in output             b = b1 - b0
  int r;         // number of sequences per spectrum     1 <= r <= x
  int d;         // distance between sequence starts     1 <= d <= x
  int w;         // window function
  int 
} asa_param_t;


typedef struct asa_struct_t {
  asa_param_t param;
  int fd_in;              // file descriptor of input (PCM s16le)
  int fd_out;             // file descriptor of output (u8 spectrum data)
  int num_in;             // how many sequences of s s16le samples read 
  int num_out;            // how many spectrums of b u8 magnitudes written
  int16_t *s16le;         // buffer for a sequence of s s16le samples
  double *d;              // input for fft, output of asa_spectrum()
  double max_mag;         // maximum magnitude after asa_spectrum()
  fftw_complex *c;        // output of fft
  fftw_plan plan;         // fftw3 plan
} *asa_t;

extern void asa_init_dbg();

extern void asa_init_fft(asa_t asa);

extern int asa_read(asa_t asa);

extern void asa_pad_and_window(asa_t asa);

extern void asa_run_fft(asa_t asa);

extern void asa_spectrum(asa_t asa);

extern void asa_write(asa_t asa);

extern void asa_cleanup(asa_t asa);

extern int asa_dbg_enabled, asa_trc_enabled;

extern char *asa_time();


#define ANSI(x) "\e[" x "m"
#define NORMAL ""
#define T_CLR "34;1"
#define D_CLR "36;1"
#define I_CLR "32;1"
#define W_CLR "33;1"
#define E_CLR "31;1"


#define _stringify0(line) ":" #line
#define _stringify(line) _stringify0(line)
#define _line _stringify(__LINE__)
#define _asa_out(ansi_color, code, t, fmt, ...) { \
  fprintf(stderr, ANSI("%s") "%s" ANSI(NORMAL) " %s%s%s%s", ansi_color, code, \
    t ? t : "", t ? __FILE__ : "", t ? _line : "", t ? " " : ""); \
  fprintf(stderr, fmt, ## __VA_ARGS__); \
  fputc('\n', stderr); \
}

#define asa_trc(fmt, ...) { if (asa_trc_enabled) \
  _asa_out(T_CLR, "T", asa_time(), fmt, ## __VA_ARGS__); \
}

#define asa_dbg(fmt, ...) { if (asa_dbg_enabled) \
  _asa_out(D_CLR, "D", asa_time(), fmt, ## __VA_ARGS__); \
}

#define asa_info(fmt, ...) { \
  asa_dbg("invocation site of " ANSI(I_CLR) "Info" ANSI(NORMAL) " below:"); \
  _asa_out(I_CLR, "Info", NULL, fmt, ## __VA_ARGS__); \
}

#define asa_warn(fmt, ...) { \
  asa_dbg("invocation site of " ANSI(W_CLR) "Warn" ANSI(NORMAL) " below:"); \
  _asa_out(W_CLR, "Warn", NULL, fmt, ## __VA_ARGS__); \
}

#define asa_error(fmt, ...) { \
  asa_dbg("invocation site of " ANSI(E_CLR) "Error" ANSI(NORMAL) " below:"); \
  _asa_out(E_CLR, "Error", NULL, fmt, ## __VA_ARGS__); \
  exit(1); \
}

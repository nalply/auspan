#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include "asa.h"
#include "asa_dbg.h"


const char* window_names[] = { 
  "boxcar", "hann", "flattop", "blackmanharris"
};

const int x = 1 << 20;


void asa_init_fft(asa_t asa) {
  asa->d = fftw_alloc_real(asa->param.n);
  asa->c = fftw_alloc_complex(asa->param.m);
  if (!asa->d || !asa->c) asa_oom();
  asa->plan = fftw_plan_dft_r2c_1d(
    asa->param.n, asa->d, asa->c, FFTW_ESTIMATE);
  if (!asa->plan) asa_error("fftw plan failed");
}


void asa_run_fft(asa_t asa) {
  fftw_execute(asa->plan);
}


#define min(a, b) ({ \
  __typeof__ (a) _a = (a); \
  __typeof__ (b) _b = (b); \
  _a < _b ? _a : _b; \
})


#define s16 sizeof(int16_t)

// not reentrant!
int asa_read(asa_t asa) {
  int size = s16 * asa->param.s;
  char *p = (char*)asa->s16le;
  
  // Overlap? Copy rest of s16le buffer to front
  if (asa->num_in && asa->param.s > asa->param.d) {
    int overlap = s16 * (asa->param.s - asa->param.d);
    asa_trc("overlap %d", overlap);
    memmove(asa->s16le, asa->s16le + asa->param.d, overlap);
    size -= overlap;
    p += overlap;
  }

  const int in = asa->fd_in;

  // Skip? Dummy read (seek doesn't work on pipes)
  if (asa->num_in && asa->param.s < asa->param.d) {
    int skip = s16 * (asa->param.d - asa->param.s);
    while (1) {
      char dummy[2000];
      int size = min(skip, sizeof(dummy));
      ssize_t len = read(in, dummy, size);
      asa_trc("skip: read(%d, dummy, %d): %ld", in, size, len);

      if (len == size) break;
      if (len == 0) return 0; // End of file
      if (len == -1) asa_error("skipping: %m");

      size -= len;
    }
  }

  while (1) {
    assert(p + size == (char*)(asa->s16le + asa->param.s));

    ssize_t len = read(in, p, size);
    asa_trc("seq #%d: read(%d, p, %d): %ld", asa->num_in, in, size, len);

    // Done!
    if (len == size) {
      asa->num_in++;
      return 1;
    }

    // End of file!
    if (len == 0) {
      asa_info("end of file after reading %i sequences(s)", asa->num_in);
      ssize_t unused = p - (char*)asa->s16le;
      if (unused) asa_warn("%ld bytes discarded", unused);
      return 0;
    }

    // Error!
    if (len == -1) asa_error("read s16le: %m");
    
    size -= len;
    p += len;
  }
}


void asa_pad_and_window(asa_t asa) {
  memset(asa->d, 0, sizeof(*asa->d) * asa->param.n);

  const double N = M_PI / (asa->param.n - 1);
  const int i0 = (asa->param.n - asa->param.s) / 2;
  const int i1 = i0 + asa->param.s;

  assert(asa->param.w >= W_FIRST && asa->param.w <= W_LAST);
  switch (asa->param.w) {

    #define WINDOW(f) \
      for (int i = i0; i < i1; i++) asa->d[i] = asa->s16le[i] * (f)
    #define COS2 cos(2 * i * N)
    #define COS4 cos(4 * i * N)
    #define COS6 cos(6 * i * N)
    #define COS8 cos(8 * i * N)

    case W_BOXCAR: {
      WINDOW(1);
    } break;

    case W_HANN: {
      // wikipedia.org/wiki/Hann_function
      const double a0=.5;
      WINDOW(a0 - a0 * COS2);
    } break;

    case W_FLATTOP: {
      // wikipedia.org/wiki/Window_function#Flat_top_window
      const double a0=.21557895, a1=.41663158, a2=.277263158,
        a3=.083578947, a4=.006947368;
      WINDOW(a0 - a1 * COS2 + a2 * COS4 - a3 * COS6 + a4 * COS8);
    }

    case W_BLACKMANHARRIS: {
      // wikipedia.org/wiki/Window_function#Blackman–Harris_window
      const double a0=0.35875, a1=0.48829, a2=0.14128, a3=0.01168;
      WINDOW(a0 - a1 * COS2 + a2 * COS4 - a3 * COS6);
    }
  }
}


void asa_spectrum(asa_t asa) {
  assert(asa->param.b0 <= asa->param.b1);

  double *const d = asa->d;
  fftw_complex *const c = asa->c;

  asa->max_mag = 0;
  for (int i = asa->param.b0; i < asa->param.b1; i++) {
    d[i] = hypot(c[i][0], c[i][1]); // assume hypot >= 0
    if (asa->max_mag < d[i]) asa->max_mag = d[i];
  }
}


void asa_write(asa_t asa) {
  assert(asa->param.b0 <= asa->param.b1);

  const int b0 = asa->param.b0;
  const int b = asa->param.b;
  const int fd = asa->fd_out;
  uint8_t spectrum[b];

  for (int i = 0; i < b; i++) {
    spectrum[i] = (uint8_t)(255 * asa->d[b0 + i] / asa->max_mag);
  }

  ssize_t result = write(fd, spectrum, b);
  asa_trc("spectrum #%d write(%d, <p>, %d): %ld", asa->num_out, fd, b, result);
  if (result == -1) {
    asa_error("output error");
  }
  asa->num_out++;
}


void asa_cleanup(asa_t asa) {
  if (asa->d) fftw_free(asa->d);
  if (asa->c) fftw_free(asa->c);
  if (asa->plan) fftw_destroy_plan(asa->plan);
  fftw_cleanup();

  asa = (asa_t){ 0 };
}


#define DATE_TIME_BUF_SIZE 25

char *asa_time() {
  static char buf[DATE_TIME_BUF_SIZE];
  time_t now = time(NULL);
  struct tm *local = localtime(&now);
  strftime(buf, DATE_TIME_BUF_SIZE, "%F %T ", local);
  return buf;
}



int asa_dbg_enabled = 0;
int asa_trc_enabled = 0;

void asa_init_dbg() {
  char* s = getenv("ASA_DBG");
  if (s == NULL) return;

  asa_dbg_enabled = s[0] == 'D' || s[0] == 'T';
  asa_trc_enabled = s[0] == 'T';
  asa_dbg("%s enabled", asa_trc_enabled ? "trace" : "debug");
  asa_dbg("software %s", COMPILE);
}

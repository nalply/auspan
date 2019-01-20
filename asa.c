#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include "asa.h"


void asa_init_fft(asa_t asa) {
  asa->r = fftw_alloc_real(asa->size);
  asa->c = fftw_alloc_complex(1 + asa->size / 2);
  if (!asa->r || !asa->c) asa_error("out of memory");
  asa->plan = fftw_plan_dft_r2c_1d(
    asa->size, asa->r, asa->c, FFTW_ESTIMATE);
  if (!asa->plan) asa_error("fftw plan failed");
}


void asa_run_fft(asa_t asa) {
  fftw_execute(asa->plan);
}


int asa_read_span(asa_t asa) {
  void *p = asa->pcm_s16le_buffer;
  size_t size = asa->size * sizeof(int16_t);

  while (1) {
    ssize_t len = read(asa->in_fd, p, size);
    asa_trc("read(%d, %lu): %ld", asa->in_fd, size, len);

    if (len == -1) asa_error("read pcm: %s", strerror(errno));
    if (len == size) return 1;
    if (len == 0) break;
    
    size -= len;
    p += size;
  }
  asa_info("end of file");
  return 0;
}


static inline double as_s16le(void *buf, size_t offset) {
  return *(int16_t*)(buf + offset);
}


void asa_convert_real(asa_t asa) {
  for (int i = 0; i < asa->size; i++) {
    asa->r[i] = as_s16le(asa->pcm_s16le_buffer, i << 1);
  }
}


void asa_bands(asa_t asa) {
  const int n = asa->size / 2;
  const double s = (double)n / (double)asa->num_bands;
  double *r = asa->r;
  fftw_complex *c = asa->c;
  double *bands = asa->bands;
  double sum = 0, max = 0;

  // TODO: apply cutoff frequencies and perhaps interleave with next for loop
  for (int i = 0; i < n; i++) {
    // TODO perhaps use 20 * log10(magnitude)
    sum += r[i] = hypot(c[i][0], c[i][1]);
    if (max < r[i]) max = r[i];
  }
  asa_trc("magnitudes: sum %.2f max %.2f  s %.1f", sum, max, s);

  max = 0;
  for (int b = 0; b < asa->num_bands; b++) {
    const size_t j0 = 1 + round(b * s);
    const size_t j1 = round((b + 1) * s);
    asa_trc("b %d j0 %lu j1 %lu j1 - j0 %ld", b, j0, j1, j1 - j0)
    assert(j1 <= n);

    double band = 0;
    for (int j = j0; j <= j1; j++) {
      band += r[j];
    }
    bands[b] = band / (j1 - j0);
    if (max < bands[b]) max = bands[b];

    asa_trc("bands[%d] %.1f band %.1f", b, bands[b], band)
  }

  asa->max = max;
}


void asa_write_spectrum(asa_t asa) {
  const size_t b = asa->num_bands;
  uint8_t spectrum[b];

  for (int i = 0; i < asa->num_bands; i++) {
    spectrum[i] = (uint8_t)(255 * asa->bands[i] / asa->max);
  }

  ssize_t result = write(asa->out_fd, spectrum, b);
  asa_trc("write(%d, %p, %lu): %ld", asa->out_fd, spectrum, b, result);
  if (result == -1) {
    asa_error("output error");
  }
}


void asa_cleanup(asa_t asa) {
  if (asa->r) fftw_free(asa->r);
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
}

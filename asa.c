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
    if (len == size) {
      asa->num_spans_read++;
      return 1;
    }
    if (len == 0) {
      asa_info("end of file after reading %lu span(s)", asa->num_spans_read);
      return 0;
    }
    
    size -= len;
    p += size;
  }
}


static inline double as_s16le(void *buf, size_t offset) {
  return *(int16_t*)(buf + offset);
}


void asa_convert_real(asa_t asa) {
  for (int i = 0; i < asa->size; i++) {
    asa->r[i] = as_s16le(asa->pcm_s16le_buffer, i << 1);
  }
}


#define TRC_BUF_SIZE 1000
static char trc_buf[TRC_BUF_SIZE + 4];
static char *trc_buf_ptr;
static void trc_append_d(double d) {
  int size = TRC_BUF_SIZE - (trc_buf_ptr - trc_buf);
  int len = snprintf(trc_buf_ptr, size, "%.1f ", d);
  trc_buf_ptr += len;
  trc_buf[TRC_BUF_SIZE + 0]  = '.';
  trc_buf[TRC_BUF_SIZE + 1]  = '.';
  trc_buf[TRC_BUF_SIZE + 2]  = '.';
  trc_buf[TRC_BUF_SIZE + 3]  = 0;
}


void asa_bands(asa_t asa) {
  const int n = 1 + asa->size / 2;
  double *r = asa->r;
  fftw_complex *c = asa->c;
  double magn_sum = 0, magn_max = 0;

  // TODO: apply cutoff frequencies and perhaps interleave with next for loop
  for (int i = 0; i < n; i++) {
    const double magn = hypot(c[i][0], c[i][1]);
    if (magn < 0) asa_trc("negative magnitude %.1f", magn); 
    magn_sum += r[i] = magn < 0 ? 0 : magn;
    if (magn_max < r[i]) magn_max = r[i];
  }

  const double s = (double)n / (double)asa->num_bands;
  asa_trc("magnitudes: sum %.1f max %.1f s %.2f", magn_sum, magn_max, s);

  double band_max = 0, band_sum = 0;
  double *bands = asa->bands;
  int ri = 0;

  for (int b = 0; b < asa->num_bands; b++) {
    const double b0 = b * s;
    const double b1 = (b + 1) * s;
    const size_t j0 = floor(b0);
    const size_t j1 = floor(b1);
    assert(j1 <= n);

    double band = r[j0] * (j0 - b0 + 1);
    for (int j = j0 + 1; j < j1; j++) band += r[j];
    if (j1 > j0) band += r[j1] * (b1 - j1);

    bands[b] = band / (b1 - b0);
    band_sum += bands[b];

    trc_buf_ptr = trc_buf;
    while (ri < j1) trc_append_d(r[ri++]);
    asa_trc("bands[%d] %.1f j (%6.1f %6.1f) %s", b, bands[b], b0, b1, trc_buf);

    if (band_max < bands[b]) band_max = bands[b];
  }
  asa_trc("magnitudes sum %.1f ≈ %.1f bands sum * %.2f", 
    magn_sum, s * band_sum, s);
  assert(magn_sum - s * band_sum < 0.01);

  asa->max = band_max;
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
  asa->num_spectrums_written++;
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
  asa_dbg("software %s", COMPILE);
}

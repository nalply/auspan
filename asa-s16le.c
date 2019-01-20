#include "asa.h"


#define PROGRAM "asa-s16le"


__attribute__((noreturn)) 
static void version() {
  printf(PROGRAM " " VERSION "\n");
  exit(0);
}


__attribute__((noreturn))
static void usage(char* msg) {
  fprintf(stderr,
    "Error: %s\n"
    "\n" 
    PROGRAM " analyses audio and generates spectrums.\n"
    "Usage: " PROGRAM " [options] [input [output]]\n"
    "  where input is PCM s16le und output 8bit spectrum data.\n"
    "\n"
    "Options:\n"
    "  -v print version\n"
    "  -n fft size (default 2048, range 2-65536, preferably powers of two)\n"
    "  -b number of spectrum bands (default 32, range 1-256)\n"
    "  -s number of spans for a spectrum (default 64, range 1-100000)\n"
    "     spectrums are generated this many times per second:\n"
    "     sampling frequency / fft size (-n) / number of spans (-s)\n"
    "", msg
  );
  exit(127);
}


static void parse_args(int argc, char **argv, asa_t asa) {
  asa->size = 2048;
  asa->num_bands = 32;
  asa->num_spans = 64;

  char opt;
  unsigned long result;

  while (-1 != (opt = getopt (argc, argv, "n:b:s:v"))) {
    switch (opt) {
      case 'v': version();

      case 'n':
        result = strtoll(optarg, NULL, 10);
        if (result < 2 || result > 65536) usage("option -n out of range");
        asa->size = result;
        break;

      case 'b':
        result = strtoll(optarg, NULL, 10);
        if (result < 1 || result > 256) usage("option -b out of range");
        asa->num_bands = result;
        break;

      case 's':
        result = strtoll(optarg, NULL, 10);
        if (result < 100 || result > 100000) usage("option -s out of range");
        asa->num_spans = result;
        break;
      
      default: usage("invalid option(s)");
    }
  } 

  if (argc - optind > 3) usage("too many parameters");

  if (argc - optind == 0) {
    asa->in_fd = STDIN_FILENO;
    asa_info("stdin used as input");
  }
  else {
    char *in = argv[optind + 0];
    asa->in_fd = open(in, O_RDONLY);
    if (asa->in_fd == -1) asa_error("input: %s", strerror(errno));
    asa_dbg("'%s' opened readonly, fd %d", in, asa->in_fd);
  }

  if (argc - optind <= 1) {
    asa->out_fd = STDOUT_FILENO;
    asa_info("stdout used as output");
  }
  else {
    char *out = argv[optind + 1];
    asa->out_fd = open(out, O_WRONLY|O_CREAT|O_APPEND, 0666);
    if (asa->out_fd == -1) asa_error("output: %s", strerror(errno));
    asa_dbg("'%s' opened writeonly, fd %d", out, asa->in_fd);
  }

  asa_dbg("size %lu num_bands %lu num_spans %lu",
    asa->size, asa->num_bands, asa->num_spans);
}


void window_square(double *in) {
  // do nothing (aka square window)
}



static struct asa_struct_t static_asa = {};


void exit_handler(void) {
  asa_dbg("cleaning up");
  if (static_asa.pcm_s16le_buffer) free(static_asa.pcm_s16le_buffer);
  if (static_asa.bands) free(static_asa.bands);
  asa_cleanup(&static_asa);
}


int main(int argc, char **argv) {
  asa_init_dbg();
  atexit(exit_handler);

  asa_t asa = &static_asa;
  parse_args(argc, argv, asa);
  asa_init_fft(asa);

  asa->pcm_s16le_buffer = malloc(sizeof(s16le_t) * asa->size);
  asa->bands = malloc(sizeof(double) * asa->num_bands);
  if (!asa->pcm_s16le_buffer || !asa->bands) asa_error("out of memory");
  asa->max = 1.0;
  asa->window = window_square;

  size_t num_spans = 0, num_spectrums = 0;
  while (asa_read_span(asa)) {
    num_spans++;
    asa_trc("span #%lu read", num_spans);

    asa_convert_real(asa); // Todo renormalise
    asa->window(asa->r);
    asa_run_fft(asa);
    asa_bands(asa);
    // TODO use step (framerate)

    asa_write_spectrum(asa);
    num_spectrums++;
    asa_trc("spectrum #%lu written", num_spectrums);
  }

  asa_dbg("Number of spans read: %lu", num_spans);
  asa_dbg("Number of spectrums written: %lu", num_spectrums);
}

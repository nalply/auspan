#include "asa.h"
#include "assert.h"


__attribute__((noreturn)) 
static void version() {
  fprintf(stderr, "%s\n", COMPILE);
  exit(0);
}


__attribute__((noreturn))
static void usage(char* msg) {
  if (msg) fprintf(stderr, "\e[31;1mError: %s\e[m\n", msg);
  fputs(
    "Analyse audio and generate spectrums\n"
    "Usage: " PROGRAM " [options] [input [output]]\n"
    "  where input is s16le pcm und output u8 spectrum data,\n"
    "  s16le is signed 16-bit little endian and u8 unsigned 8-bit integer\n"
    "\n"
    "Options:                            (x = 2^20 = 1048576, m = 1 + n / 2)\n"
    "  -v print version                       default   limits\n"
    "  -s number of samples taken in a sequence    32   1 <= s <= x\n"
    "  -n fft size with zero padding                s   s <= n <= x\n"
    "  -b b0,b1 use bins from b0 to b1          1,m-2   0 <= b0 <= b1 <= m-1\n"
    "     spectrums contain b = b1 - b0 unsigned 8-bit bins\n"
    "  -c c0,c1,...,ck combine bins     don't combine   sum(c0,...,ck) = b\n"
    "         or c0_c1 combine logarithmically\n"
    "  -r number of sequences per spectrum          1   1 <= r <= 999\n"
    "  -d distance between sequence starts          s   1 <= d <= x\n"
    "         or in % of s                       100%   1% <= d <= 10000%\n"
    "     spectrums output at frequency: sampling frequency / d / r\n"
    "  -w window function, one of: boxcar hann flattop blackmanharris,\n"
    "     default hann\n"
    "", stderr
  );
  exit(127);
}


static void parse_args(int argc, char **argv, asa_t asa) {
  asa_param_t p = { 
    .s = 32, .n = 32, .m = 17, .b0 = 1, .b1 = 15, .b = 15, .r = 1, .d = 32,
    .w = W_HANN,
  };

  asa_trc("default s %d n %d m %d b0 %d b1 %d b %d r %d d %d w %s",
    p.s, p.n, p.m, p.b0, p.b1, p.b, p.r, p.d, window_names[p.w]);

  char opt;
  unsigned long result;
  int s_set = 0, n_set = 0, d_set = 0, b_set = 0;

  while (-1 != (opt = getopt (argc, argv, "vhs:n:b:c:r:d:w:"))) {
    asa_trc("opt %c optarg '%s' optind %d", opt, optarg, optind);
    switch (opt) {
      case 'v': version();

      case 'h': usage(NULL);

      case 's': {
        result = strtoull(optarg, NULL, 10);
        if (result < 1 || result > x) usage("-s out of limit");
        p.s = result;
        if (!n_set) p.n = p.s;
        if (!d_set) p.d = p.s;
        s_set = 1;
      } break;

      case 'n': {
        result = strtoll(optarg, NULL, 10);
        p.n = result;
        p.m = 1 + p.n / 2;
        n_set = 1;
      } break;
      
      case 'b': {
        int num = sscanf(optarg, "%u,%u", &p.b0, &p.b1);
        if (num != 2) usage("-b malformed");
        p.c[0] = -1;
        b_set = 1;
      } break;

      case 'c': {
        if (!b_set) usage("specify -b before -c"); // it's easier this way :-/

        unsigned a, b;
        int num = sscanf(optarg, "%u_%u", &a, &b);
        if (num == 2) usage("-c logarithmic combine not implemented");

        int i = 0, sum = 0;
        char *tok = strtok(optarg, ",");
        while (tok != NULL) {
          if (i >= C_SIZE) usage("-c too man elements");
          result = strtoull(tok, NULL, 10);
          if (result <= 0 || result > p.b)
            usage("-c combine element > b or invalid");
          sum += asa->param.c[i++] = result;
          if (sum > p.b) usage("-c sum of elements > b");
          tok = strtok(NULL, ",");
        }
        if (sum < p.b) usage("-c sum of elements < b");
      }

      case 'r': {
        result = strtoll(optarg, NULL, 10);
        if (result < 1 || result > 999) usage("-r out of limit");
        p.r = result;
      } break;

      case 'd': {
        if (!s_set) usage("specify -s before -d"); // it's easier this way :-/
        char *tail;
        result = strtoull(optarg, &tail, 10);
        if (tail[0] == '%') {
          if (result > 10000) usage("-d out of limit");
          p.d = result / 100.0 * p.s;
        }
        else {
          if (result < 1 || result > x) usage("-d out of limit");
          p.d = result;
        }
      } break;
      
      case 'w': {
        for (p.w = W_FIRST; p.w <= W_LAST; p.w++)
          if (0 == strcmp(optarg, window_names[p.w])) break;
        if (p.w > W_LAST) usage("-d invalid window type");
      } break;

      default: usage("invalid option(s) or missing argument(s)");
    }
  }

  p.m = 1 + p.n / 2;
  if (!b_set) p.b1 = p.m - 2;
  p.b = 1 + p.b1 - p.b0;

  if (p.n < p.s || p.n > x) usage("-n out of limit");
  if (p.b0 > p.b1) usage("-b rule b0 <= b1 broken");
  if (p.b1 > p.m - 1) usage("-b rule b1 <= m-1 broken");
  if (argc - optind > 3) usage("too many parameters");

  if (argc - optind == 0) {
    asa->fd_in = STDIN_FILENO;
    asa_info("stdin used as input");
  }
  else {
    char *in = argv[optind + 0];
    asa->fd_in = open(in, O_RDONLY);
    if (asa->fd_in == -1) asa_error("input: %s", strerror(errno));
    asa_dbg("'%s' opened readonly, fd %d", in, asa->fd_in);
  }

  if (argc - optind <= 1) {
    asa->fd_out = STDOUT_FILENO;
    asa_info("stdout used as output");
  }
  else {
    char *out = argv[optind + 1];
    asa->fd_out = open(out, O_WRONLY|O_CREAT|O_APPEND, 0666);
    if (asa->fd_out == -1) asa_error("output: %s", strerror(errno));
    asa_dbg("'%s' opened writeonly append, fd %d", out, asa->fd_out);
  }

  asa->param = p;

  

  asa_info(""
    "Running with these parameters:                  (f: sampling frequency)\n"
    "  w %-14s window function"
    "  s %6d         number of samples in a sequence%s\n"
    "  n %6d         fft input size; frequency resolution is f / n, for\n"
    "                   44.1 kHz it's %.3f Hz\n"
    "  r %6d         number of sequences used per generated spectrum\n"
    "  d %6d         distance between sequence starts; spectrums come at\n"
    "                   f / r / d, for 44.1 kHz at %.3f Hz\n"
    "  m %6d         fft output size; using bins %d to %d, %d total\n"
    ""
      , window_names[p.w]
      , p.s , p.n > p.s ? ", sequence zero-padded" : ""
      , p.n, 44100.0 / p.n
      , p.r, p.d
      , 44100.0 / p.r / p.d
      , p.m, p.b0, p.b1, p.b
  );
}


static struct asa_struct_t static_asa = { 0 };


void exit_handler(void) {
  asa_dbg("cleaning up");
  if (static_asa.s16le) free(static_asa.s16le);
  asa_cleanup(&static_asa);
}


int main(int argc, char **argv) {
  asa_init_dbg();
  atexit(exit_handler);

  asa_t asa = &static_asa;
  parse_args(argc, argv, asa);
  asa_init_fft(asa);
  asa_param_t p = asa->param;

  asa->s16le = malloc(sizeof(int16_t) * p.s);
  if (!asa->s16le) asa_error("out of memory");

  while (asa_read(asa)) {
    asa_pad_and_window(asa);
    asa_run_fft(asa);
    asa_spectrum(asa);
    asa_write(asa);
  }

  asa_dbg("number of sequences read: %d", asa->num_in);
  asa_dbg("number of spectrums written: %d", asa->num_out);
}

#include "../asa.h"

#define N 100

__attribute__((noreturn))
static void usage() {
  fputs(
    "dump_window n window\n"
    "  where n <= 10000 and window one of:\n"
    "    boxcar hann flattop blackmanharris\n"
    "output: n space-separated numbers up to 10000"
    "", stderr
  );
  exit(1);
}

int main(int argc, char **argv) {
  if (argc != 3) usage();
  unsigned int n = strtoul(argv[1], NULL, 10);
  if (n > 10000) usage();
  int w;
  for (w = W_FIRST; w <= W_LAST; w++)
    if (0 == strcmp(argv[2], window_names[w])) break;
  if (w > W_LAST) usage();

  int16_t s16le[n];
  double d[n];

  struct asa_struct_t asa = { 
    .s16le = s16le, 
    .d = d, 
    .param.w = w,
    .param.n = n,
    .param.s = n,
  };

  for (int i = 0; i < N; i++) s16le[i] = 10000;

  asa_pad_and_window(&asa);

  for (int i = 0; i < n; i++) printf("%0.0f ", asa.d[i]);
  puts("");
}

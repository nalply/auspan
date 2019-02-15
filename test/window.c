#include <asa.h>
#include <stdlib.h>
#include <string.h>

#define Y_DBG_MAIN
#include <y_dbg.h>

__attribute__((noreturn))
static void usage() {
  fprintf(stderr, "Usage: window <n> <window>\n"
      "  where: 1 <= n <= %d; window one of:", x);
  for (int i = 0; i <= W_LAST; i++)
    fprintf(stderr, " %s", window_names[i]);
  fputs("\n", stderr);
  exit(1);
}

int main(int argc, char **argv) {
  if (argc != 3) usage();
  unsigned int n = strtoul(argv[1], NULL, 10);
  if (n > x) usage();
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

  for (int i = 0; i < n; i++) s16le[i] = 10000;

  asa_pad_and_window(&asa);

  printf("%d %s:", n, window_names[w]);
  for (int i = 0; i < n; i++) printf(" %.0f", asa.d[i]);
  puts("");
}

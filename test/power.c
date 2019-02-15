#include <asa.h>
#include <stdlib.h>
#include <string.h>

#define Y_DBG_MAIN
#include <y_dbg.h>

__attribute__((noreturn))
static void usage() {
  fprintf(stderr, "Usage: power <b> <l> <p>\n"
      "  where: integer 1 <= l <= b <= %d; real 1 <= p <= 2\n", x);
  exit(1);
}

int main(int argc, char **argv) {
  if (argc != 4) usage();
  int l = strtoul(argv[2], NULL, 10);
  if (l < 1 || l > x ) usage();
  int b = strtoul(argv[1], NULL, 10);
  if (b < l || b > x ) usage();
  double p = strtod(argv[3], NULL);

  int *lines = asa_distribute_bins(l, b, p);

  printf("%d %d %g:", b, l, p);
  for (int i = 0; i < l; i++) printf(" %d", lines[i]);
  printf(" %d", sum(lines, l));
  puts("");
}

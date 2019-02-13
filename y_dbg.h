/* A macro only source file to implement colorful logging with levels:
 *   trc: output if Y_DBG=T, blue T 
 *   dbg: output if Y_DBG=D or Y_DBG=T cyan D
 *   info: output with green Info
 *   warn: output with yellow Warn
 *   error: output with red Error and exit with error code 1
 *   abort: output with magenta Abort and abort like assert
 * The macros return the bytes written so that one can implement word-breaking
 * together with the y_*_o() variants which allow logging in parts.
 *
 * Example log output (T, D, Info, Warn are colored)
 * T 2019-02-07 15:02:17 asa.c:78 seq #0: read(0, p, 8192): 2
 * T 2019-02-07 15:02:18 asa.c:78 seq #0: read(0, p, 8190): 0
 * D 2019-02-07 15:02:18 asa.c:88 invocation site of Info below:
 * Info end of file after reading 0 sequences(s)
 * D 2019-02-07 15:02:18 asa.c:90 invocation site of Warn below:
 * Warn 2 bytes discarded
 * D 2019-02-07 15:02:18 asa-s16le.c:279 number of sequences read: 0
 * D 2019-02-07 15:02:18 asa-s16le.c:280 number of spectrums written: 0
 * D 2019-02-07 15:02:18 asa-s16le.c:254 cleaning up
 */
#ifndef Y_DBG_H
#define Y_DBG_H

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define Y_DATE_TIME_BUF_SIZE 25

static inline char *y_time() {
  static char buf[Y_DATE_TIME_BUF_SIZE];
  time_t now = time(NULL);
  struct tm *local = localtime(&now);
  strftime(buf, Y_DATE_TIME_BUF_SIZE, "%F %T ", local);
  return buf;
}

#define Y_TRC  5
#define Y_DBG  4
#define Y_INFO 3
#define Y_WARN 2

#define Y_ANSI(x) "\e[" x "m"
#define Y_RESET ""
#define Y_TCLR "34;1"
#define Y_DCLR "36;1"
#define Y_ICLR "32;1"
#define Y_WCLR "33;1"
#define Y_ECLR "31;1"
#define Y_ACLR "35;1"

#define Y_OUT 0
#define Y_OUT_START 1
#define Y_OUT_END 2
#define Y_OUT_CONT 3

#define Y_STRINGIFY0(line) ":" #line
#define Y_STRINGIFY(line) Y_STRINGIFY0(line)
#define Y_LINE_S Y_STRINGIFY(__LINE__)

#define _Y_OUT(ansi_color, code, t, out, fmt, ...) ({ \
  (out <= Y_OUT_START ? \
    fprintf(stderr, \
      Y_ANSI("%s") "%s" Y_ANSI(Y_RESET) " %s%s%s%s", \
      ansi_color, code, \
      t ? t : "", \
      t ? __FILE__ : "", \
      t ? Y_LINE_S : "", \
      t ? " " : "" \
    ) : 0) \
  + fprintf(stderr, fmt, ##__VA_ARGS__) \
  + ((out == Y_OUT || out == Y_OUT_END) ? (fputc('\n', stderr), 1) : 0); \
})

#define y_trc_o(out, fmt, ...) ({ y_level() >= Y_TRC \
  ? _Y_OUT(Y_TCLR, "T", y_time(), out, fmt, ##__VA_ARGS__) : 0; \
})

#define y_trc(fmt, ...) ({ y_trc_o(Y_OUT, fmt, ##__VA_ARGS__); })

#define y_dbg_o(out, fmt, ...) ({ y_level() >= Y_DBG \
  ? _Y_OUT(Y_DCLR, "D", y_time(), out, fmt, ##__VA_ARGS__) : 0; \
})

#define y_dbg(fmt, ...) ({ y_dbg_o(Y_OUT, fmt, ##__VA_ARGS__); })

#define _Y_INVOCATION_SITE(ansi, code) ({ \
  y_dbg("invocation site of " Y_ANSI(ansi) code Y_ANSI(Y_RESET) " below:"); \
})

#define y_info_o(out, fmt, ...) ({ y_level() >= Y_INFO ? ( \
  (out <= Y_OUT_START ? _Y_INVOCATION_SITE(Y_ICLR, "Info") : 0) \
  + _Y_OUT(Y_ICLR, "Info", NULL, out, fmt, ##__VA_ARGS__)) : 0; \
})

#define y_info(fmt, ...) ({ y_info_o(Y_OUT, fmt, ##__VA_ARGS__); })

// TODO: y_warn_o()

#define y_warn(fmt, ...) { \
  _Y_INVOCATION_SITE(Y_WCLR, "Warn"); \
  _Y_OUT(Y_WCLR, "Warn", NULL, Y_OUT, fmt, ##__VA_ARGS__); \
}

#define y_error_o(out, fmt, ...) { \
  (out <= Y_OUT_START ? _Y_INVOCATION_SITE(Y_ECLR, "Error") : 0) \
  + _Y_OUT(Y_ECLR, "Error", NULL, out, fmt, ##__VA_ARGS__); \
  if (out == Y_OUT || out == Y_OUT_END) exit(1); \
}

#define y_error(fmt, ...) ({ y_error_o(Y_OUT, fmt, ##__VA_ARGS__); })

#define y_abort_o(out, fmt, ...) { \
  if (out <= Y_OUT_START) fputc('\n', stderr); \
  _Y_OUT(Y_ACLR, "Abort", y_time(), out, fmt, ##__VA_ARGS__); \
  if (out == Y_OUT || out == Y_OUT_END) abort(); \
}

#define y_abort(fmt, ...) y_abort_o(Y_OUT, fmt, ##__VA_ARGS__)

#define y_oom() y_abort("out of memory")

#define _Y_ASSERT(expr, fmt, ...) { \
  if (expr) /*empty*/; else { \
    y_abort_o(Y_OUT_START, "assert " #expr " failed"); \
    if (fmt[0]) y_abort_o(Y_OUT_CONT, ": "); \
    y_abort_o(Y_OUT_END, fmt, ##__VA_ARGS__); \
  } \
}

#define y_assert(expr) _Y_ASSERT(expr, "")

#define y_assert_e(expr, fmt, ...) _Y_ASSERT(expr, fmt, ##__VA_ARGS__)

static inline int y_level() {
  static unsigned int level = 0;

  if (level == 0) {
    char *y_log_level = getenv("Y_LOG");
    if (y_log_level) {
      if (y_log_level[0] == 'T')        level = Y_TRC;
      else if (y_log_level[0] == 'D')   level = Y_DBG;
      else if (y_log_level[0] == 'I')   level = Y_INFO;
      else                              level = Y_WARN;
      y_dbg("Y_LOG environment variable: '%s'", y_log_level);
    }
    else {
      level = Y_INFO;
    }
    if (level > Y_TRC) abort();
    y_info("log level set to %c", "AEWIDT"[level]);
  }

  return level;
}

#endif

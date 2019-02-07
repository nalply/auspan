/* A macro only source file to implement colorful logging with levels:
 *   trc: (o) output if ASA_DBG=T, blue code T 
 *   dbg: (o) output if ASA_DBG=D or ASA_DBG=T cyan code D
 *   info: (o) output with green code Info
 *   warn: output with yellow code Warn
 *   error: output with red code Error and exit with error code 1
 * (o) is the option to partially output a logging message with out as:
 *   OUT: log whole message at once
 *   OUT_START: start logging one message
 *   OUT_CONT: continue logging the started message
 *   OUT_END: end logging the started message
 * by using the *_o() macro, for example asa_trc_o(). The _o() macros return
 * the bytes written so that one can implement word-breaking.
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
#ifndef ASA_DBG_H
#define ASA_DBG_H

#include <stdio.h>

#define ANSI(x) "\e[" x "m"
#define RESET ""
#define T_CLR "34;1"
#define D_CLR "36;1"
#define I_CLR "32;1"
#define W_CLR "33;1"
#define E_CLR "31;1"

#define OUT 0
#define OUT_START 1
#define OUT_END 2
#define OUT_CONT 3

#define STRINGIFY0(line) ":" #line
#define STRINGIFY(line) STRINGIFY0(line)
#define _ASA_LINE STRINGIFY(__LINE__)

#define _ASA_OUT(ansi_color, code, t, out, fmt, ...) ({ \
  (out <= OUT_START ? \
    fprintf(stderr, \
      ANSI("%s") "%s" ANSI(RESET) " %s%s%s%s", \
      ansi_color, code, \
      t ? t : "", \
      t ? __FILE__ : "", \
      t ? _ASA_LINE : "", \
      t ? " " : "" \
    ) : 0) \
  + fprintf(stderr, fmt, ##__VA_ARGS__) \
  + ((out == OUT || out == OUT_END) ? (fputc('\n', stderr), 1) : 0); \
})

#define asa_trc_o(out, fmt, ...) ({ asa_trc_enabled \
  ? _ASA_OUT(T_CLR, "T", asa_time(), out, fmt, ##__VA_ARGS__) : 0; \
})

#define asa_trc(fmt, ...) ({ asa_trc_o(OUT, fmt, ##__VA_ARGS__); })

#define asa_dbg(fmt, ...) ({ asa_dbg_enabled \
  ? _ASA_OUT(D_CLR, "D", asa_time(), OUT, fmt, ##__VA_ARGS__) : 0; \
})

#define _ASA_INVOCATION_SITE(ansi_color, code) ({ \
  asa_dbg("invocation site of " ANSI(ansi_color) code ANSI(RESET) " below:"); \
})

#define asa_info_o(out, fmt, ...) ({ \
  (out <= OUT_START ? _ASA_INVOCATION_SITE(I_CLR, "Info") : 0) \
  + _ASA_OUT(I_CLR, "Info", NULL, out, fmt, ##__VA_ARGS__); \
})

#define asa_info(fmt, ...) ({ asa_info_o(OUT, fmt, ##__VA_ARGS__); })

#define asa_warn(fmt, ...) { \
  _ASA_INVOCATION_SITE(W_CLR, "Warn"); \
  _ASA_OUT(W_CLR, "Warn", NULL, OUT, fmt, ##__VA_ARGS__); \
}

#define asa_error(fmt, ...) { \
  _ASA_INVOCATION_SITE(E_CLR, "Error"); \
  _ASA_OUT(E_CLR, "Error", NULL, OUT, fmt, ##__VA_ARGS__); \
  exit(1); \
}

#define asa_oom() asa_error("out of memory")

#endif

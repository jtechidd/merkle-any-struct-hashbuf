#ifndef common_h
#define common_h

#define TRY(expr)                                              \
  do {                                                         \
    int _try_err = (expr);                                     \
    if (_try_err < 0) {                                        \
      fprintf(stderr, "[ERROR] %s failed at %s:%d (err=%d)\n", \
              #expr, __FILE__, __LINE__, _try_err);            \
      return _try_err;                                         \
    }                                                          \
  } while (0);

#endif
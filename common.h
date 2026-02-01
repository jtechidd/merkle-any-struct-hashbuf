#ifndef common_h
#define common_h

#define RETURN_IF_ERROR(expr)                                  \
  do {                                                         \
    int _error_code = (expr);                                  \
    if (_error_code < 0) {                                     \
      fprintf(stderr, "[ERROR] %s failed at %s:%d (err=%d)\n", \
              #expr, __FILE__, __LINE__, _error_code);         \
      return _error_code;                                      \
    }                                                          \
  } while (0);

#endif
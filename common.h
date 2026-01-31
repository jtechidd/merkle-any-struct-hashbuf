#ifndef common_h
#define common_h

#define RETURN_IF_NEG(expr)                                                    \
  if ((expr) < 0) {                                                            \
    return -1;                                                                 \
  }

#endif
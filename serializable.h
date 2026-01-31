#ifndef serializable_h
#define serializable_h

typedef struct buffer_t buffer_t;
typedef struct serializable_t serializable_t;

typedef int (*serialize_t)(serializable_t *, buffer_t *);

typedef struct serializable_t {
  serialize_t serialize;
} serializable_t;

#endif
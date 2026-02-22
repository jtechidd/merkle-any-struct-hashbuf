#ifndef serializable_h
#define serializable_h

typedef struct buffer_t buffer_t;
typedef struct serializable_t serializable_t;

typedef void serialize_fn(serializable_t *, buffer_t *);

typedef struct serializable_t {
  serialize_fn* serialize;
} serializable_t;

#endif
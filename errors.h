#ifndef ERRORS_H
#define ERRORS_H

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

typedef int status_t;

enum
{
  LF_OK,
  LF_INTERNAL_ERROR,
  LF_SYS_ERR_START = 1000
};

#define LF_FROM_SYS_ERROR(e) (LF_SYS_ERR_START + e)
#define LF_TO_SYS_ERROR(e) (e - LF_SYS_ERR_START)
#define LF_IS_SYS_ERROR(e) (e >= LF_SYS_ERR_START)

#define FAIL_PRED(pred, status) \
  do { if (pred) { _status = status; goto fail; } } while(0)

#define FAIL_SYS(pred) \
  do { if (pred) { _status = LF_FROM_SYS_ERROR(errno); goto fail; } } while(0)

#define FAIL_FORWARD(expr) \
  do { _status = expr; if (_status != LF_OK) goto fail; } while(0)

#define FAIL_PRED_MSG(pred, status, msg) \
  do { if (pred) { _status = status; fprintf(stderr, "%s\n", msg); goto fail; } } while(0)

#define FAIL_SYS_MSG(pred, msg) \
  do { if (pred) { _status = LF_FROM_SYS_ERROR(errno); fprintf(stderr, "%s\n", msg); goto fail; } } while(0)

#define FAIL_FORWARD_MSG(expr, msg) \
  do { _status = expr; if (_status != LF_OK) { fprintf(stderr, "%s\n", msg); goto fail; } } while(0)

void lf_strerror(int error_num, char *buffer, size_t buffer_length);

#endif

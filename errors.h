/* Copyright (c) 2012 Francis Russell <francis@unchartedbackwaters.co.uk>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

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
  LF_INVALID_WINDOW_SIZE,
  LF_INVALID_COMMAND_LINE_OPTION,
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

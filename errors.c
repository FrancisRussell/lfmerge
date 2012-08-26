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

#include "errors.h"
#include <string.h>

struct error_string
{
  int code;
  const char *message;
};

static struct error_string error_strings[] = {
  { LF_OK,             "No error encountered." },
  { LF_INTERNAL_ERROR, "An internal error occured. Please report." },
  { LF_INVALID_WINDOW_SIZE, "Invalid checksum window size." },
  { LF_INVALID_COMMAND_LINE_OPTION, "Invalid command line option." }
};

void lf_strerror(const int status, char *const buffer, const size_t buffer_length)
{
  if (LF_IS_SYS_ERROR(status))
  {
    strerror_r(LF_TO_SYS_ERROR(status), buffer, buffer_length);
  }
  else
  {
    for(size_t i=0; i < sizeof(error_strings) / sizeof(struct error_string); ++i)
    {
      if (error_strings[i].code == status)
      {
          strncpy(buffer, error_strings[i].message, buffer_length);
          return;
      }
    }
    strncpy(buffer, "An invalid or unhandled error code was encountered. Please report.", buffer_length);
  }
}

#include "errors.h"
#include <string.h>

struct error_string
{
  int code;
  const char* message;
};

static struct error_string error_strings[] = {
  { LF_OK,             "No error encountered." },
  { LF_INTERNAL_ERROR, "An internal error occured. Please report." }
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


#include <stdlib.h>
#include <stdio.h>
#include "memory.h"

void *lmalloc(size_t size)
{
  void* const data = malloc(size);

  if (data == NULL)
  {
    fprintf(stderr, "Failed to allocate region of %li bytes.\n", size);
    exit(EXIT_FAILURE);
  }

  return data;
}

void lfree(void *const data)
{
  free(data);
}

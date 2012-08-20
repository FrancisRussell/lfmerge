#ifndef MEMORY_H
#define MEMORY_H

#include <stdlib.h>

void *lmalloc(size_t size);
void lfree(void *data);

#endif

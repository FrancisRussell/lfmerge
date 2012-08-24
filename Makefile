LFS_CFLAGS:=$(shell getconf LFS_CFLAGS)
LFS_LDFLAGS:=$(shell getconf LFS_LDFLAGS)

CFLAGS=-O3 -Wall -pedantic -std=c99 -D_POSIX_C_SOURCE=200112l ${LFS_CFLAGS}
LDFLAGS=${LFS_LDFLAGS}

all: lfmerge

lfmerge.o: file_info.h checksum.h errors.h

file_info.o: file_info.h checksum.h errors.h

checksum.o: checksum.h

errors.o: errors.h

lfmerge: file_info.o checksum.o errors.o

clean:
	rm -f lfmerge checksum.o file_info.o lfmerge.o memory.o errors.o

.PHONY: clean all

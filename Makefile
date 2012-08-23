LFS_CFLAGS:=$(shell getconf LFS_CFLAGS)
LFS_LDFLAGS:=$(shell getconf LFS_LDFLAGS)

CFLAGS=-O3 -Wall -pedantic -std=c99 -D_POSIX_C_SOURCE=200112l ${LFS_CFLAGS}
LDFLAGS=${LFS_LDFLAGS}

all: lfmerge

lfmerge.o: file_info.h checksum.h

file_info.o: file_info.h checksum.h memory.h

checksum.o: checksum.h

memory.o: memory.h

lfmerge: file_info.o checksum.o memory.o

clean:
	rm -f lfmerge checksum.o  file_info.o  lfmerge.o  memory.o

.PHONY: clean all

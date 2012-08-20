CFLAGS=-O2 -Wall -pedantic -std=c99 -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64

all: lfmerge

lfmerge.o: file_info.h checksum.h

file_info.o: file_info.h checksum.h memory.h

checksum.o: checksum.h

memory.o: memory.h

lfmerge: file_info.o checksum.o memory.o

clean:
	rm -f lfmerge checksum.o  file_info.o  lfmerge.o  memory.o

.PHONY: clean all

#ifndef fILE_INFO_H
#define fILE_INFO_H

#include <stdio.h>
#include "checksum.h"

static const int BUFFER_SIZE = 4 * 1048576;

typedef struct
{
  FILE *file;
  long total_length;
  long block_offset;
  long internal_offset;
  long buffer_use;
  checksum_t checksum;
  unsigned char *buffer;

} file_info_t;

int open_input_file(const char *path, file_info_t *info);
int close_input_file(file_info_t *info);
int hit_file_end(const file_info_t *file);
int hit_buffer_end(const file_info_t * info);
void populate_forwards(file_info_t *file);
void populate_backwards(file_info_t *info);
int find_checksum_match(file_info_t *f1_info, file_info_t *f2_info);
void advance_location(file_info_t *file);
int validate_match(file_info_t *f1_info, file_info_t *f2_info);
int write_merged_file(file_info_t *f1_info, file_info_t *f2_info, FILE *out);
int find_overlap_start(file_info_t *f1_info, file_info_t *f2_info);
long characters_handled(file_info_t *info);

#endif

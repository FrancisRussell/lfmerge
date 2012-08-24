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

#ifndef FILE_INFO_H
#define FILE_INFO_H

#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include "checksum.h"
#include "errors.h"

static const size_t BUFFER_SIZE = 4 * 1048576;

typedef struct
{
  FILE   *file;
  off_t  total_length;
  off_t  block_offset;
  long internal_offset;
  long buffer_use;
  checksum_t checksum;
  unsigned char *prev_buffer;
  unsigned char *buffer;

} file_info_t;

typedef struct
{
  off_t matching_bytes;
  off_t total_bytes;
} match_info_t;

status_t open_input_file(file_info_t *info, const char *path, size_t checksum_length);
status_t close_input_file(file_info_t *info);
status_t seek_file(file_info_t *info, off_t offset);
off_t file_length(const file_info_t *info);
int hit_file_end(const file_info_t *file);
int hit_buffer_end(const file_info_t * info);
status_t populate_forwards(file_info_t *file);
status_t find_checksum_match(const file_info_t *f1_info, file_info_t *f2_info, int *result);
status_t advance_location(file_info_t *file);
status_t validate_match(file_info_t *f1_info, file_info_t *f2_info, int *result);
status_t write_merged_file(file_info_t *f1_info, file_info_t *f2_info, FILE *out);
off_t characters_handled(file_info_t *info);
status_t compute_match_info(FILE *f1, FILE *f2, match_info_t *info);
status_t get_match_info(file_info_t *f1_info, file_info_t *f2_info, match_info_t *info);

static inline unsigned char get_byte(file_info_t *const info, const long offset)
{
  const long local_offset = info->internal_offset + offset;

  assert(offset <= 0);
  assert(local_offset + BUFFER_SIZE >= 0);

  if (local_offset >= 0)
    return info->buffer[local_offset];
  else
    return info->prev_buffer[BUFFER_SIZE + local_offset];
}



#endif

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
#include "checksum.h"

static const size_t BUFFER_SIZE = 4 * 1048576;

typedef struct
{
  FILE   *file;
  off_t  total_length;
  off_t  block_offset;
  size_t internal_offset;
  size_t buffer_use;
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
off_t characters_handled(file_info_t *info);

#endif

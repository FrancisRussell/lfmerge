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

#include "file_info.h"
#include "memory.h"
#include "checksum.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

static void reverse_bytes(unsigned char *data, size_t length);

int open_input_file(const char *const path, file_info_t *const info)
{
  info->file = fopen(path, "rb");
  if (info->file == NULL)
  {
    fprintf(stderr, "Unable to open input file %s: ", path);
    perror(NULL);
    exit(EXIT_FAILURE);
  }

  fseek(info->file, 0, SEEK_END);
  info->total_length = ftell(info->file);

  info->buffer = lmalloc(BUFFER_SIZE);
  info->buffer_use = 0;
  info->block_offset = 0;
  info->internal_offset = 0;
  init_checksum(&info->checksum);

  return 1;
}

int close_input_file(file_info_t *const info)
{
  lfree(info->buffer);
  return (fclose(info->file) == 0);
}

int hit_file_end(const file_info_t *const file)
{
  return file->total_length == file->internal_offset + file->block_offset;
}

int hit_buffer_end(const file_info_t *const info)
{
  return info->internal_offset == info->buffer_use;
}

void populate_forwards(file_info_t *const file)
{
  if (!hit_buffer_end(file))
    return;

  file->block_offset += file->buffer_use;
  file->internal_offset = 0;

  fseek(file->file, file->block_offset, SEEK_SET);
  file->buffer_use = fread(file->buffer, 1, BUFFER_SIZE, file->file);
}

void reverse_bytes(unsigned char* const data, const size_t length)
{
  for(size_t i=0; i<(length/2); ++i)
  {
    const unsigned char tmp = data[i];
    data[i] = data[length - i - 1];
    data[length - i - 1] = tmp;
  }
}

void populate_backwards(file_info_t *const info)
{
  if (!hit_buffer_end(info))
    return;

  info->block_offset += info->buffer_use; 
  info->buffer_use = (info->total_length - info->block_offset > BUFFER_SIZE ? BUFFER_SIZE : info->total_length - info->block_offset);
  info->internal_offset = 0;

  fseek(info->file, -(info->block_offset + info->buffer_use), SEEK_END);
  const long read = fread(info->buffer, 1, info->buffer_use, info->file);
  assert(read == info->buffer_use);

  // Reverse contents of buffer
  reverse_bytes(info->buffer, info->buffer_use);
}

int find_checksum_match(file_info_t *const f1_info, file_info_t *const f2_info)
{
  assert(!hit_buffer_end(f1_info) && !hit_buffer_end(f2_info));

  do
  {
    add_char_checksum(&f1_info->checksum, f1_info->buffer[f1_info->internal_offset++]);
    add_char_checksum(&f2_info->checksum, f2_info->buffer[f2_info->internal_offset++]);

    if (checksum_equal(&f1_info->checksum, &f2_info->checksum))
      return 1;
  }
  while(!hit_buffer_end(f1_info) && !hit_buffer_end(f2_info));

  return 0;
}

void advance_location(file_info_t *const file)
{
  ++file->internal_offset;
}

int validate_match(file_info_t *const f1_info, file_info_t *const f2_info)
{
  fseek(f1_info->file, -(f1_info->block_offset + f1_info->internal_offset), SEEK_END);
  fseek(f2_info->file, 0, SEEK_SET);

  unsigned char *buffer1 = lmalloc(BUFFER_SIZE);
  unsigned char *buffer2 = lmalloc(BUFFER_SIZE);

  int result = 1;
  while(result == 1 && !feof(f1_info->file) && !feof(f2_info->file))
  {
    const long read1 = fread(buffer1, 1, BUFFER_SIZE, f1_info->file);
    const long read2 = fread(buffer2, 1, BUFFER_SIZE, f2_info->file);
    const long length = (read1 < read2 ? read1 : read2); 

    if (memcmp(buffer1, buffer2, length) != 0)
      result = 0;
  }

  lfree(buffer1);
  lfree(buffer2);

  return result;
}

int write_merged_file(file_info_t *const f1_info, file_info_t *const f2_info, FILE *const out)
{
  fseek(f1_info->file, 0, SEEK_SET);
  unsigned char *const buffer = lmalloc(BUFFER_SIZE);

  long read;
  do
  {
    read = fread(buffer, 1, BUFFER_SIZE, f1_info->file);
    const long written = fwrite(buffer, 1, read, out);

    if (written != read)
    {
      lfree(buffer);
      return 0;
    }
  }
  while(read != 0);

  fseek(f2_info->file, f2_info->block_offset + f2_info->internal_offset, SEEK_SET);
  do
  {
    read = fread(buffer, 1, BUFFER_SIZE, f2_info->file);
    const long written = fwrite(buffer, 1, read, out);

    if (written != read)
    {
      lfree(buffer);
      return 0;
    }
  }
  while(read != 0);

  lfree(buffer);

  return 1;
}

int find_overlap_start(file_info_t *const f1_info, file_info_t *const f2_info)
{
  int candidate_found = 0;

  while(candidate_found == 0 && !hit_file_end(f1_info) && !hit_file_end(f2_info))
  {
    populate_backwards(f1_info);
    populate_forwards(f2_info);
    candidate_found = find_checksum_match(f1_info, f2_info);
  }

  return candidate_found;
}

long characters_handled(file_info_t *const info)
{
  return info->block_offset + info->internal_offset;
}

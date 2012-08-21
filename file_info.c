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
#include <sys/types.h>

int open_input_file(file_info_t *const info, 
                    const char *const path, 
                    const size_t checksum_length)
{
  info->file = fopen(path, "rb");
  if (info->file == NULL)
  {
    fprintf(stderr, "Unable to open input file %s: ", path);
    perror(NULL);
    exit(EXIT_FAILURE);
  }

  fseeko(info->file, 0, SEEK_END);
  info->total_length = ftello(info->file);

  info->prev_buffer = lmalloc(BUFFER_SIZE);
  info->buffer      = lmalloc(BUFFER_SIZE);

  init_checksum(&info->checksum, checksum_length);

  return seek_file(info, 0);
}

int seek_file(file_info_t *const info, const off_t offset)
{
  info->block_offset = offset;
  info->buffer_use = 0;
  info->internal_offset = 0;
  memset(info->prev_buffer, 0, BUFFER_SIZE);
  reset_checksum(&info->checksum);
  return fseeko(info->file, offset, SEEK_SET) == 0;
}

int close_input_file(file_info_t *const info)
{
  lfree(info->buffer);
  return (fclose(info->file) == 0);
}

off_t file_length(const file_info_t *info)
{
  return info->total_length;
}

int hit_file_end(const file_info_t *const file)
{
  return file->total_length == file->internal_offset + file->block_offset;
}

int hit_buffer_end(const file_info_t *const info)
{
  return info->internal_offset >= info->buffer_use;
}

void populate_forwards(file_info_t *const file)
{
  if (!hit_buffer_end(file))
    return;

  file->block_offset += file->buffer_use;
  file->internal_offset = 0;

  unsigned char *const new_prev_buffer = file->buffer;
  file->buffer = file->prev_buffer;
  file->prev_buffer = new_prev_buffer;

  fseeko(file->file, file->block_offset, SEEK_SET);
  file->buffer_use = fread(file->buffer, 1, BUFFER_SIZE, file->file);
}

int find_checksum_match(const file_info_t *const f1_info, file_info_t *const f2_info)
{
  do
  {
    advance_location(f2_info);

    if (checksum_equal(&f1_info->checksum, &f2_info->checksum))
      return 1;
  }
  while(!hit_file_end(f2_info));

  return 0;
}

int advance_location(file_info_t *const file)
{
  if (hit_file_end(file))
    return 0;

  populate_forwards(file);

  add_char_checksum(&file->checksum, 
    get_byte(file, -checksum_length(&file->checksum)),
    get_byte(file, 0));

  ++file->internal_offset;

  return 1;
}

int validate_match(file_info_t *const f1_info, file_info_t *const f2_info)
{
  const size_t cs_length = checksum_length(&f1_info->checksum);
  assert(cs_length ==  checksum_length(&f2_info->checksum));

  fseeko(f1_info->file, f1_info->block_offset + f1_info->internal_offset - cs_length, SEEK_SET);
  fseeko(f2_info->file, f2_info->block_offset + f2_info->internal_offset - cs_length, SEEK_SET);

  unsigned char *const buffer1 = lmalloc(BUFFER_SIZE);
  unsigned char *const buffer2 = lmalloc(BUFFER_SIZE);

  int result = 1;
  while(result == 1 && !feof(f1_info->file) && !feof(f2_info->file))
  {
    const size_t read1 = fread(buffer1, 1, BUFFER_SIZE, f1_info->file);
    const size_t read2 = fread(buffer2, 1, BUFFER_SIZE, f2_info->file);
    const size_t length = (read1 < read2 ? read1 : read2);
 
    if (memcmp(buffer1, buffer2, length) != 0)
      result = 0;
   }

  lfree(buffer1);
  lfree(buffer2);

  return result;
}

int write_merged_file(file_info_t *const f1_info, file_info_t *const f2_info, FILE *const out)
{
  fseeko(f1_info->file, 0, SEEK_SET);
  unsigned char *const buffer = lmalloc(BUFFER_SIZE);

  size_t read;
  do
  {
    read = fread(buffer, 1, BUFFER_SIZE, f1_info->file);
    const size_t written = fwrite(buffer, 1, read, out);

    if (written != read)
    {
      lfree(buffer);
      return 0;
    }
  }
  while(read != 0);

  fseeko(f2_info->file, f2_info->block_offset + f2_info->internal_offset, SEEK_SET);
  do
  {
    read = fread(buffer, 1, BUFFER_SIZE, f2_info->file);
    const size_t written = fwrite(buffer, 1, read, out);

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

off_t characters_handled(file_info_t *const info)
{
  return info->block_offset + info->internal_offset;
}

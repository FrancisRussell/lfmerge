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
#include "checksum.h"
#include "errors.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>

static int hit_buffer_end(const file_info_t *info);

inline int hit_buffer_end(const file_info_t *const info)
{
  return info->internal_offset >= info->buffer_use;
}

status_t open_input_file(file_info_t *const info, 
                         const char *const path, 
                         const size_t checksum_length)
{
  status_t _status = LF_INTERNAL_ERROR;
  FAIL_PRED(checksum_length > BUFFER_SIZE, LF_INVALID_WINDOW_SIZE);
  info->prev_buffer = info->buffer = NULL;

  FAIL_SYS((info->prev_buffer = malloc(BUFFER_SIZE)) == NULL);
  FAIL_SYS((info->buffer = malloc(BUFFER_SIZE)) == NULL);
  init_checksum(&info->checksum, checksum_length);

  info->file = fopen(path, "rb");
  FAIL_SYS(info->file == NULL);
  FAIL_SYS(fseeko(info->file, 0, SEEK_END) == -1);
  info->total_length = ftello(info->file);

  FAIL_FORWARD(seek_file(info, 0));
  return LF_OK;

fail:
  free(info->buffer);
  free(info->prev_buffer);
  return _status;
}

status_t seek_file(file_info_t *const info, const off_t offset)
{
  status_t _status = LF_INTERNAL_ERROR;
  info->block_offset = offset;
  info->buffer_use = 0;
  info->internal_offset = 0;
  memset(info->prev_buffer, 0, BUFFER_SIZE);
  memset(info->buffer, 0, BUFFER_SIZE);
  reset_checksum(&info->checksum);
  FAIL_SYS(fseeko(info->file, offset, SEEK_SET) == -1);

  return LF_OK;

fail:
  return _status;
}

status_t close_input_file(file_info_t *const info)
{
  status_t _status = LF_INTERNAL_ERROR;
  free(info->prev_buffer);
  free(info->buffer);
  
  FAIL_SYS(fclose(info->file) == EOF);
  return LF_OK;

fail:
  return _status;
}

status_t populate_forwards(file_info_t *const file)
{
  status_t _status = LF_INTERNAL_ERROR;
  if (!hit_buffer_end(file))
    return LF_OK;

  file->block_offset += file->buffer_use;
  file->internal_offset = 0;

  unsigned char *const new_prev_buffer = file->buffer;
  file->buffer = file->prev_buffer;
  file->prev_buffer = new_prev_buffer;

  FAIL_SYS(fseeko(file->file, file->block_offset, SEEK_SET) == -1);
  file->buffer_use = fread(file->buffer, 1, BUFFER_SIZE, file->file);
  FAIL_SYS(file->buffer_use != BUFFER_SIZE && ferror(file->file));
  return LF_OK;

fail:
  return _status;
}

status_t find_checksum_match(const file_info_t *const f1_info, file_info_t *const f2_info, int *const success)
{
  *success = 0;
  do
  {
    const status_t status = advance_location(f2_info);
    if (status != LF_OK)
      return status;

    if (checksum_equal(&f1_info->checksum, &f2_info->checksum))
    {
      *success = 1;
      return LF_OK;
    }
  }
  while(!hit_file_end(f2_info));

  return LF_OK;
}

status_t advance_location(file_info_t *const file)
{
  status_t _status = LF_INTERNAL_ERROR;
  FAIL_PRED(hit_file_end(file), LF_INTERNAL_ERROR);

  if (hit_buffer_end(file))
    FAIL_FORWARD(populate_forwards(file));

  add_char_checksum(&file->checksum, 
    get_byte(file, -checksum_length(&file->checksum)),
    get_byte(file, 0));

  ++file->internal_offset;
  return LF_OK;

fail:
  return _status;
}

status_t validate_match(file_info_t *const f1_info, file_info_t *const f2_info, int *const is_valid)
{
  status_t _status = LF_INTERNAL_ERROR;
  const size_t cs_length = checksum_length(&f1_info->checksum);
  assert(cs_length ==  checksum_length(&f2_info->checksum));

  FAIL_SYS(fseeko(f1_info->file, f1_info->block_offset + f1_info->internal_offset - cs_length, SEEK_SET) == -1);
  FAIL_SYS(fseeko(f2_info->file, f2_info->block_offset + f2_info->internal_offset - cs_length, SEEK_SET) == -1);

  match_info_t match_info;
  FAIL_FORWARD(compute_match_info(f1_info->file, f2_info->file, &match_info));
  *is_valid = (match_info.matching_bytes == match_info.total_bytes);
  return LF_OK;

fail:
  return _status;
}

status_t get_match_info(file_info_t *const f1_info, file_info_t *const f2_info, match_info_t *const info)
{
  status_t _status = LF_INTERNAL_ERROR;
  const off_t f2_offset = characters_handled(f2_info);
  const off_t f1_start = f1_info->total_length - (f2_offset > f1_info->total_length ? f1_info->total_length : f2_offset);
  const off_t f2_start = f2_offset - (f1_info->total_length - f1_start);

  FAIL_SYS(fseeko(f1_info->file, f1_start, SEEK_SET) == -1);
  FAIL_SYS(fseeko(f2_info->file, f2_start, SEEK_SET) == -1);
  FAIL_FORWARD(compute_match_info(f1_info->file, f2_info->file, info));
  return LF_OK;

fail:
  return _status;
}

status_t compute_match_info(FILE *const f1, FILE *const f2, match_info_t *const info)
{
  status_t _status = LF_INTERNAL_ERROR;
  info->matching_bytes = 0;
  info->total_bytes = 0;

  unsigned char *buffer1 = NULL;
  unsigned char *buffer2 = NULL;
  FAIL_SYS((buffer1 = malloc(BUFFER_SIZE)) == NULL);
  FAIL_SYS((buffer2 = malloc(BUFFER_SIZE)) == NULL);

  while(!feof(f1) && !feof(f2))
  {
    const size_t read1 = fread(buffer1, 1, BUFFER_SIZE, f1);
    FAIL_SYS(read1 != BUFFER_SIZE && ferror(f1));
    const size_t read2 = fread(buffer2, 1, BUFFER_SIZE, f2);
    FAIL_SYS(read2 != BUFFER_SIZE && ferror(f2));
    const size_t length = (read1 < read2 ? read1 : read2);
 
    info->total_bytes += length;
    for(size_t i=0; i<length; ++i)
    {
      if (buffer1[i] == buffer2[i])
        ++info->matching_bytes;
      else
        info->matching_bytes = 0;
    }
  }
  _status = LF_OK;

fail:
  free(buffer1);
  free(buffer2);
  return _status;
}

status_t write_merged_file(file_info_t *const f1_info, file_info_t *const f2_info, FILE *const out)
{
  status_t _status = LF_INTERNAL_ERROR;
  unsigned char *buffer = NULL;
  FAIL_SYS(fseeko(f1_info->file, 0, SEEK_SET) == -1);
  FAIL_SYS((buffer = malloc(BUFFER_SIZE)) == NULL);

  size_t read;
  do
  {
    read = fread(buffer, 1, BUFFER_SIZE, f1_info->file);
    FAIL_SYS(read != BUFFER_SIZE && ferror(f1_info->file));
    const size_t written = fwrite(buffer, 1, read, out);
    FAIL_SYS(written != read);
  }
  while(read != 0);

  FAIL_SYS(fseeko(f2_info->file, f2_info->block_offset + f2_info->internal_offset, SEEK_SET) == -1);
  do
  {
    read = fread(buffer, 1, BUFFER_SIZE, f2_info->file);
    FAIL_SYS(read != BUFFER_SIZE && ferror(f2_info->file));
    const size_t written = fwrite(buffer, 1, read, out);
    FAIL_SYS(written != read);
  }
  while(read != 0);

  _status=LF_OK;

fail:
  free(buffer);
  return _status;
}

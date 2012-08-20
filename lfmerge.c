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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_info.h"
#include "checksum.h"

static const char *desc_string = "\
Searches for the offset of the overlap between \"file1\" and \"file2\".\n\
If the overlap is found it is printed and the merged file written\n\
to \"merged\", if supplied.";
static const char *copyright = "\
Copyright (c) 2012 Francis Russell <francis@unchartedbackwaters.co.uk>";

static const int MINIMUM_OVERLAP = 1024;

void usage()
{
  fprintf(stderr, "Usage: lfmerge file1 file2 [merged]\n\n");
  fprintf(stderr, "%s\n\n", desc_string);
  fprintf(stderr, 
    "This build was configured for a minimum overlap of %i bytes.\n\n", 
    MINIMUM_OVERLAP);
  fprintf(stderr, "%s\n", copyright);
}

int main(const int argc, char **const argv)
{
  if (argc != 3 && argc != 4)
  {
    usage();
    exit(EXIT_FAILURE);
  }

  if (argc == 4 && (strcmp(argv[1], argv[3])==0 || strcmp(argv[2], argv[3])==0))
  {
    fprintf(stderr, "Output file cannot also be one of the input files.\n");
    exit(EXIT_FAILURE);
  }

  file_info_t f1_info, f2_info;
  open_input_file(argv[1], &f1_info);
  open_input_file(argv[2], &f2_info);

  int found = 0;
  while(!found && !hit_file_end(&f1_info) && !hit_file_end(&f2_info))
  {
    found = find_overlap_start(&f1_info, &f2_info) &&
      validate_match(&f1_info, &f2_info) &&
      characters_handled(&f1_info) > MINIMUM_OVERLAP;
  }

  if (found != 0)
  {
    const long overlap = characters_handled(&f1_info);
    printf("Found overlap of %li bytes at offset of %li bytes.\n", overlap, f1_info.total_length - overlap);
  }
  else
  {
    printf("Failed to find overlap.\n");
  }

  if (argc == 4)
  {
    FILE *const out = fopen(argv[3], "wb");
    if (out == NULL)
    {
      fprintf(stderr, "Unable to open output file %s: ", argv[3]);
      perror(NULL);
      exit(EXIT_FAILURE);
    }
 
    const int result = write_merged_file(&f1_info, &f2_info, out);
    if (result == 0)
    {
      fprintf(stderr, "Failed to write merged file: ");
      perror(NULL);
      exit(EXIT_FAILURE);
    }
    else
    {
      printf("Wrote merged file %s.\n", argv[3]);
    }
    fclose(out);
  }
  else
  {
    printf("Not writing output file since none supplied.\n");
  }

  close_input_file(&f1_info);
  close_input_file(&f2_info);
 
  exit(EXIT_SUCCESS);
}

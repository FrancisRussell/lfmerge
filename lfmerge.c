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
#include <sys/types.h>
#include "file_info.h"
#include "checksum.h"

static const char *desc_string = "\
Searches for the offset of an overlap between the footer of \"file1\"\n\
and any region in \"file2\". Since the overlap can occur at any point\n\
in \"file2\", this is useful for instances where \"file2\" has headers\n\
that must be discarded. If the overlap is found it is printed and the\n\
merged file written to \"merged\", if supplied.";

static const char *copyright = "\
Copyright (c) 2012 Francis Russell <francis@unchartedbackwaters.co.uk>";

static const int OVERLAP_SIZE = 4 * 1024;

void usage()
{
  fprintf(stderr, "Usage: lfmerge file1 file2 [merged]\n\n");
  fprintf(stderr, "%s\n\n", desc_string);
  fprintf(stderr, 
    "This build was configured with an overlap size of %i bytes.\n\n", 
    OVERLAP_SIZE);
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
  open_input_file(&f1_info, argv[1], OVERLAP_SIZE);
  open_input_file(&f2_info, argv[2], OVERLAP_SIZE);

  // Checksum end of first file
  if (seek_file(&f1_info, file_length(&f1_info) - OVERLAP_SIZE))
  {
    while(advance_location(&f1_info));
  }
  else
  {
    fprintf(stderr, "Unable to checksum footer of first file. Probably too small.\n");
    exit(EXIT_FAILURE);
  }

  int found = 0;
  while(!found && !hit_file_end(&f2_info))
  {
    found = find_checksum_match(&f1_info, &f2_info) &&
      characters_handled(&f2_info) >= OVERLAP_SIZE && 
      validate_match(&f1_info, &f2_info);
  }

  if (found != 0)
  {
    const off_t join_location = characters_handled(&f2_info);
    printf("Found join location at offset of %ju bytes into second file.\n", join_location);

    match_info_t match_info;
    get_match_info(&f1_info, &f2_info, &match_info);
    const double match_percentage = 
      (100.0 * match_info.matching_bytes)/match_info.total_bytes;

    printf("Of the overlapping region of size %ju, the final %ju (%.2f%%) bytes matched exactly.\n", 
      match_info.total_bytes, match_info.matching_bytes, match_percentage);

    if (match_info.total_bytes < join_location)
      printf("Warning: This merge completely subsumes the first file. Mostly like the output will be useless.\n");

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
  }
  else
  {
    printf("Failed to find overlap.\n");
  }

  close_input_file(&f1_info);
  close_input_file(&f2_info);
 
  exit(EXIT_SUCCESS);
}

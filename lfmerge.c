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
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include "file_info.h"
#include "checksum.h"
#include "errors.h"

static const char *desc_string = "\
Searches for the offset of an overlap between the footer of \"file1\"\n\
and any region in \"file2\". Since the overlap can occur at any point\n\
in \"file2\", this is useful for instances where \"file2\" has headers\n\
that must be discarded. If the overlap is found it is printed and the\n\
merged file written to \"merged\", if supplied.";

static const char *copyright = "\
Copyright (c) 2012 Francis Russell <francis@unchartedbackwaters.co.uk>";

static const long DEFAULT_OVERLAP_SIZE = 4 * 1024;

struct option_values
{
  long window_size;
  int  first_index;
  int  arg_count;
};

static void init_default_option_values(struct option_values *const options)
{
  options->window_size = DEFAULT_OVERLAP_SIZE;
  options->first_index = 0;
  options->arg_count = 0;
}

static status_t parse_options(struct option_values *const options, const int argc, char **const argv)
{
  status_t _status = LF_INTERNAL_ERROR;
  int opt;
  while((opt = getopt(argc, argv, "w:")) != -1)
  {
    switch(opt)
    {
      case 'w': 
      {
        char *endptr;
        errno = 0;
        const long size = strtol(optarg, &endptr, 10);
        FAIL_SYS(errno != 0);
        FAIL_PRED(endptr == optarg || *endptr != '\0', LF_INVALID_COMMAND_LINE_OPTION);
        options->window_size = size;
        break;
      }
      default:
      {
        FAIL_PRED(1, LF_INVALID_COMMAND_LINE_OPTION);
      }
    }
  }

  options->first_index = optind;
  options->arg_count = argc - options->first_index;
  return LF_OK;

fail:
  return _status;
}

static void usage()
{
  fprintf(stderr, "Usage: lfmerge [-w overlap_window_size] file1 file2 [merged]\n\n");
  fprintf(stderr, "%s\n\n", desc_string);
  fprintf(stderr, 
    "This build was configured with a default overlap size of %li bytes.\n\n", 
    DEFAULT_OVERLAP_SIZE);
  fprintf(stderr, "%s\n", copyright);
}

int main(const int argc, char **const argv)
{
  status_t _status;
  struct option_values options;
  init_default_option_values(&options);
  FAIL_FORWARD(parse_options(&options, argc, argv));

  if (options.arg_count != 2 && options.arg_count != 3)
  {
    usage();
    exit(EXIT_FAILURE);
  }

  if (options.window_size <= 0 || options.window_size > BUFFER_SIZE)
  {
    fprintf(stderr, "Overlap window size must be between 1 and %li bytes inclusive.\n", BUFFER_SIZE);
    exit(EXIT_FAILURE);
  }

  const char *const file1 = argv[options.first_index];
  const char *const file2 = argv[options.first_index+1];
  const char *const file3 = (options.arg_count == 3 ? argv[options.first_index+2] : NULL);

  if (options.arg_count == 3 && (strcmp(file1, file3)==0 || strcmp(file2, file3)==0))
  {
    fprintf(stderr, "Output file cannot also be one of the input files.\n");
    exit(EXIT_FAILURE);
  }

  file_info_t f1_info, f2_info;
  FAIL_FORWARD_MSG(open_input_file(&f1_info, file1, options.window_size), "Couldn't open first file.");

  if (file_length(&f1_info) < options.window_size)
  {
    fprintf(stderr, "First file needs to be at least %li bytes long.\n", options.window_size);
    FAIL_FORWARD(close_input_file(&f1_info));
    exit(EXIT_FAILURE);
  }

  FAIL_FORWARD_MSG(open_input_file(&f2_info, file2, options.window_size), "Couldn't open second file.");

  // Checksum end of first file
  FAIL_FORWARD_MSG(seek_file(&f1_info, file_length(&f1_info) - options.window_size), "Couldn't seek to footer of first file.");

  printf("Performing search using overlap window of %li bytes.\n", options.window_size);

  while(!hit_file_end(&f1_info))
    FAIL_FORWARD(advance_location(&f1_info));

  int found = 0;
  while(!found && !hit_file_end(&f2_info))
  {
    FAIL_FORWARD(find_checksum_match(&f1_info, &f2_info, &found));
    found = found && (characters_handled(&f2_info) >= options.window_size);

    if (found)
      FAIL_FORWARD(validate_match(&f1_info, &f2_info, &found));
  }

  if (found != 0)
  {
    const off_t join_location = characters_handled(&f2_info);
    printf("Found join location at offset of %ju bytes into second file.\n", join_location);

    match_info_t match_info;
    FAIL_FORWARD(get_match_info(&f1_info, &f2_info, &match_info));

    const double match_percentage = 
      (100.0 * match_info.matching_bytes)/match_info.total_bytes;

    printf("Of the overlapping region of size %ju bytes, the final %ju (%.2f%%) matched exactly.\n", 
      match_info.total_bytes, match_info.matching_bytes, match_percentage);

    if (match_info.total_bytes < join_location)
      printf("Warning: This merge will produce a file shorter than the second. Mostly likely the output will be useless.\n");

    if (options.arg_count == 3)
    {
      FILE *const out = fopen(file3, "wb");
      FAIL_SYS_MSG(out == NULL, "Failed to open output file.");
   
      FAIL_FORWARD_MSG(write_merged_file(&f1_info, &f2_info, out), "Couldn't write output file.");
      FAIL_SYS_MSG(fclose(out) == EOF, "Failed to close output file after write.");
      printf("Wrote merged file %s.\n", file3);
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

  FAIL_FORWARD_MSG(close_input_file(&f1_info), "Error closing first input file.");
  FAIL_FORWARD_MSG(close_input_file(&f2_info), "Error closing second input file.");
  exit(found != 0 ? EXIT_SUCCESS : EXIT_FAILURE);

fail:;
  char buffer[256];
  lf_strerror(_status, buffer, sizeof(buffer));
  fprintf(stderr, "An error occured: %s\n", buffer);
  exit(EXIT_FAILURE);
}

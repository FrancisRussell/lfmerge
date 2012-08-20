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

#ifndef CHECKSUM_H
#define CHECKSUM_H

#include <stdlib.h>

/* This number must be prime, and must be smaller than 
 * (2^64)/(2^8) otherwise the multiplication may
 * overflow the >= 64-bit integer.
 */
typedef unsigned long long checksum_integer_t;
static const checksum_integer_t LCG_A = 6364136223846793005ull;

typedef struct
{
  size_t length;
  checksum_integer_t lcg_ak;
  checksum_integer_t byte_product;
  checksum_integer_t byte_sum;
} checksum_t;

void init_checksum(checksum_t *c, size_t length);
void reset_checksum(checksum_t *c);
int checksum_equal(const checksum_t *c1, const checksum_t *c2);
size_t checksum_length(const checksum_t* c);
void add_char_checksum(checksum_t *checksum, unsigned char out, unsigned char in);

#endif

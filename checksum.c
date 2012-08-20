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

#include "checksum.h"
#include <assert.h>
#include <stdlib.h>

static checksum_integer_t checksum_pow(const checksum_integer_t x, 
                                       const checksum_integer_t y)
{
  if (y == 0)
    return 1;

  const checksum_integer_t sub_pow = checksum_pow(x, y/2);
  
  if (y % 2 == 0)
    return sub_pow * sub_pow;
  else
    return sub_pow * sub_pow * x;
}

void init_checksum(checksum_t *const c, const size_t length)
{
  c->length = length;
  c->lcg_ak = checksum_pow(LCG_A, length);
  c->byte_product = 0;
}

void reset_checksum(checksum_t *const c)
{
  c->byte_product = 0;
}

int checksum_equal(const checksum_t *const c1, const checksum_t *const c2)
{
  return c1->byte_product == c2->byte_product;
}

void add_char_checksum(checksum_t *const checksum, const unsigned char out, const unsigned char in) 
{
  checksum->byte_product *= LCG_A;
  checksum->byte_product -= checksum->lcg_ak * out;
  checksum->byte_product += in;
}

size_t checksum_length(const checksum_t *const c)
{
  return c->length;
}



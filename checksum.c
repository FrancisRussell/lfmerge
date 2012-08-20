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

void init_checksum(checksum_t *const c)
{
  c->byte_product = 1;
  c->byte_sum = 0;
}

int checksum_equal(const checksum_t *const c1, const checksum_t *const c2)
{
  return c1->byte_product == c2->byte_product &&
         c1->byte_sum == c2->byte_sum;
}

void add_char_checksum(checksum_t *const checksum, const unsigned char in) 
{
  // Since we traverse both forwards and backwards, our checksum consists only
  // of commutative and associative operations. Specifically, addition modulo
  // the range of checksum_integer_t and multiplication modulo a prime.
  checksum->byte_sum += ((checksum_integer_t) in);

  // Zero is not a member of our multiplicative group.
  checksum->byte_product *= ((checksum_integer_t) in) + 1;
  checksum->byte_product %= PRIME;

  // We should *never* generate 0 via these operations.
  assert(checksum->byte_product != 0);
}



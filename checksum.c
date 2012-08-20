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



#ifndef CHECKSUN_h
#define CHECKSUN_h

/* This number must be prime, and must be smaller than 
 * (2^64)/(2^8) otherwise the multiplication may
 * overflow the >= 64-bit integer.
 */
typedef unsigned long long checksum_integer_t;
static const checksum_integer_t PRIME = 36028797018963913ull;

typedef struct
{
  checksum_integer_t byte_product;
  checksum_integer_t byte_sum;
} checksum_t;

void init_checksum(checksum_t* c);
int checksum_equal(const checksum_t *c1, const checksum_t *c2);
void add_char_checksum(checksum_t *checksum, const unsigned char in);

#endif

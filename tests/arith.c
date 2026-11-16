// Integer arithmetic at every width the 6502 has to synthesize: 8, 16, 32
// and 64 bits, signed and unsigned, with values the optimizer cannot fold.
#include <inttypes.h>
#include <stdio.h>

static volatile uint32_t seed = 0x12345678;

static uint32_t next(void) {
  uint32_t x = seed;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  seed = x;
  return x;
}

int main(void) {
  uint64_t h = 0;
  for (int i = 0; i < 64; i++) {
    uint32_t a = next(), b = next() | 1;
    uint8_t a8 = a, b8 = b;
    int16_t a16 = a, b16 = b;
    int32_t a32 = a, b32 = b;
    uint64_t a64 = ((uint64_t)a << 32) | next(), b64 = ((uint64_t)b << 29) | 1;
    h = h * 31 + (uint8_t)(a8 * b8) + a8 / b8 + a8 % b8 + (uint8_t)(a8 << (b8 & 7));
    h = h * 31 + (uint16_t)(a16 * b16) + (uint16_t)(a16 / b16) +
        (uint16_t)(a16 % b16) + (uint16_t)(a16 >> (b & 15));
    h = h * 31 + (uint32_t)(a32 * b32) + (uint32_t)(a32 / b32) +
        (uint32_t)(a32 % b32) + (uint32_t)(a32 >> (b & 31)) + (a >> (b & 31));
    h = h * 31 + a64 * b64 + a64 / b64 + a64 % b64 + (a64 >> (b & 63)) +
        (uint64_t)((int64_t)a64 >> (b & 63));
    h ^= (a32 < b32) | (a < b) << 1 | (a16 > b16) << 2 | (a8 >= b8) << 3;
    if (i % 16 == 15)
      printf("%2d %08" PRIx32 " %016" PRIx64 "\n", i, a, h);
  }
  return 0;
}

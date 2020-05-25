#include "hash.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "exception.h"

#define RightRotate(x, n) (((x) >> (n)) | ((x) << ((sizeof(x) << 3) - (n))))
static void ChunkProcess(const uint8_t *msg, uint32_t *h) {
  // Initialize array of round constants (first 32 bits of the fractional parts
  // of the cube roots of the first 64 primes)
  static uint32_t k[64] = {
      0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
      0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
      0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
      0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
      0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
      0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
      0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
      0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
      0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
      0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
      0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

  uint32_t w[64], new_h[8];

  // copy chunk into first 16 words w[0..15] of the message schedule array
  for (int i = 0, j = 0; i < 16; i++, j += 4)
    w[i] = msg[j] << 24 | msg[j + 1] << 16 | msg[j + 2] << 8 | msg[j + 3];

  // extend the first 16 words into the remaining 48 words w[16..63] of the
  // message schedule array
  for (int i = 16; i < 64; i++) {
    const uint32_t s0 = RightRotate(w[i - 15], 7) ^ RightRotate(w[i - 15], 18) ^
                        (w[i - 15] >> 3);
    const uint32_t s1 = RightRotate(w[i - 2], 17) ^ RightRotate(w[i - 2], 19) ^
                        (w[i - 2] >> 10);
    w[i] = w[i - 16] + s0 + w[i - 7] + s1;
  }

  // initialize working variables to current hash value
  memcpy(new_h, h, sizeof(new_h));

  // compression function main loop:
  for (int i = 0; i < 64; i++) {
    const uint32_t s1 = RightRotate(new_h[4], 6) ^ RightRotate(new_h[4], 11) ^
                        RightRotate(new_h[4], 25);
    const uint32_t ch = (new_h[4] & new_h[5]) ^ (~new_h[4] & new_h[6]);
    const uint32_t t1 = new_h[7] + s1 + ch + k[i] + w[i];

    const uint32_t s0 = RightRotate(new_h[0], 2) ^ RightRotate(new_h[0], 13) ^
                        RightRotate(new_h[0], 22);
    const uint32_t maj =
        (new_h[0] & new_h[1]) ^ (new_h[0] & new_h[2]) ^ (new_h[1] & new_h[2]);
    const uint32_t t2 = s0 + maj;

    for (int j = 7; j >= 1; j--) new_h[j] = new_h[j - 1];
    new_h[4] += t1;
    new_h[0] = t1 + t2;
  }

  // add the compressed chunk to the current hash value
  for (int i = 0; i < 8; i++) h[i] += new_h[i];
}
#undef RightRotate

#define SHA256_BLOCK_SIZE 64  // 512 / 8
void Sha256Sum(uint32_t *const dst, const uint8_t *const src,
               const uint32_t len) {
  // Pre-processing (Padding)
  uint32_t n = len / SHA256_BLOCK_SIZE;
  uint32_t m = len % SHA256_BLOCK_SIZE;
  uint32_t cover_size = SHA256_BLOCK_SIZE;
  if (m >= 56) cover_size += SHA256_BLOCK_SIZE;
  uint8_t cover_data[SHA256_BLOCK_SIZE * 2];
  memset(cover_data, 0, SHA256_BLOCK_SIZE * 2);
  memcpy(cover_data, src + (n * SHA256_BLOCK_SIZE), m);
  cover_data[m] = 0x80;        // append a single '1' bit
  for (int i = 0; i < 4; i++)  // append len as a 64-bit big-endian integer
    cover_data[cover_size - 1 - i] = ((len * 8) & (0xff << (i * 2))) >> (i * 8);

  // Initialize hash values (first 32 bits of the fractional parts of the square
  // roots of the first 8 primes)
  uint32_t h[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                   0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

  // break message into 512-bit chunks and process each chunk
  const uint8_t *now = src;
  for (uint32_t i = 0; i < n; i++, now += SHA256_BLOCK_SIZE)
    ChunkProcess(now, h);
  now = cover_data;
  for (uint8_t i = cover_size / SHA256_BLOCK_SIZE; i;
       i--, now += SHA256_BLOCK_SIZE)
    ChunkProcess(now, h);

  if (!dst) Error("SHA-256 Dst is a nullptr");
  memcpy(dst, h, 256 / 8);
}

void RandStr(char *const dst, const unsigned len) {
  srand(time(NULL));
  for (unsigned i = 0; i != len; i++) dst[i] = '!' + rand() % 94;
  dst[len] = '\0';
}

#undef SHA256_BLOCK_SIZE

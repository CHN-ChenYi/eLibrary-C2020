#ifndef HASH_H
#define HASH_H

#include <stdint.h>

// Calculate the SHA-256 hash of src and store the result in dst
// Note that dst must has at least 32 Byte
// Example:
//   uint32_t sha[8];
//   char str[] = "Hello World!";
//   try {
//     Sha256Sum(sha, (unsigned char *)str, strlen(str));
//     except(ErrorException) puts("oops");
//   } endtry
//   for (int i = 0; i < 8; i++) printf("%08x", sha[i]);
void Sha256Sum(uint32_t *const dst, const uint8_t *const src,
               const uint32_t len);

#endif  // HASH_H
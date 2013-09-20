/* This code is public-domain - it is based on libcrypt 
 * placed in the public domain by Wei Dai and other contributors.
 */

#ifndef INTERNAL_SHA1_H
#define INTERNAL_SHA1_H

#include <sys/types.h>
#include <stdint.h>

#define SHA1_LENGTH 20

struct SHA1_CTX;

#ifdef __cplusplus
extern "C" {
#endif

int SHA1_Init(struct SHA1_CTX **s);
void SHA1_Update(struct SHA1_CTX *s, const void *data, size_t len);
void SHA1_Final(struct SHA1_CTX *s, uint8_t *result);
void SHA1_Free(struct SHA1_CTX **s);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* INTERNAL_SHA1_H */

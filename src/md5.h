/*
 * This is an OpenSSL-compatible implementation of the RSA Data Security, Inc.
 * MD5 Message-Digest Algorithm (RFC 1321).
 *
 * Homepage:
 * http://openwall.info/wiki/people/solar/software/public-domain-source-code/md5
 *
 * Author:
 * Alexander Peslyak, better known as Solar Designer <solar at openwall.com>
 *
 * This software was written by Alexander Peslyak in 2001.  No copyright is
 * claimed, and the software is hereby placed in the public domain.
 * In case this attempt to disclaim copyright and place the software in the
 * public domain is deemed null and void, then the software is
 * Copyright (c) 2001 Alexander Peslyak and it is hereby released to the
 * general public under the following terms:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted.
 *
 * There's ABSOLUTELY NO WARRANTY, express or implied.
 *
 * See md5.c for more information.
 */

#ifndef INTERNAL_MD5_H
#define INTERNAL_MD5_H

#include <sys/types.h>
#include <stdint.h>

struct MD5_CTX;

#define MD5_LENGTH 16

#ifdef __cplusplus
extern "C" {
#endif
int MD5_Init(struct MD5_CTX **ctx);
void MD5_Update(struct MD5_CTX *ctx, const void *data, size_t size);
void MD5_Final(struct MD5_CTX *ctx, uint8_t *result);
void MD5_Free(struct MD5_CTX **ctx);
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* INTERNAL_MD5_H */

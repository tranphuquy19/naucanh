#ifndef _CRYPT_PORT_H
#define _CRYPT_PORT_H 1

/* Enable algorithms - THIS IS CRITICAL */
#define INCLUDE_md5crypt 1
#define INCLUDE_sha256crypt 1
#define INCLUDE_sha512crypt 1

/* Standalone build config */
#define HAVE_CONFIG_H 1

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

/* System compatibility */
#ifdef __THROW
#undef __THROW
#endif
#define __THROW

/* Endianness */
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__)
# if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#  define WORDS_BIGENDIAN 0
# elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# define WORDS_BIGENDIAN 0
#endif

/* Feature macros */
#define HAVE_EXPLICIT_BZERO 1
#define HAVE_STRTOUL 1
#define HAVE_SNPRINTF 1
#define INCLUDE_explicit_bzero 1

/* Buffer sizes */
#define CRYPT_OUTPUT_SIZE 512
#define ALG_SPECIFIC_SIZE 8192
#define SALT_LEN_MAX 16

/* Utility macros */
#define ARG_UNUSED(x) x __attribute__ ((__unused__))
#define MIN_SIZE(x) (x)

/* Disable static_assert for compatibility */
#define static_assert(expr, msg) /* disabled */

/* Symbol versioning - disabled for standalone */
#define SYMVER_crypt_sha512crypt_rn
#define SYMVER_crypt_sha256crypt_rn
#define SYMVER_crypt_md5crypt_rn

/* Base64 table */
extern const unsigned char ascii64[65];
#define b64t ((const char *) ascii64)

/* Function prototypes */
extern void make_failure_token(const char *setting, char *output, int size);
extern void gensalt_sha_rn(const char *tag, size_t maxsalt, unsigned long defcount,
                           unsigned long mincount, unsigned long maxcount,
                           unsigned long count, const uint8_t *rbytes, size_t nrbytes,
                           uint8_t *output, size_t output_size);
extern void _crypt_explicit_bzero(void *s, size_t len);
extern size_t _crypt_strcpy_or_abort(void *dst, size_t d_size, const void *src);
extern bool get_random_bytes(void *buf, size_t buflen);

/* explicit_bzero mapping */
#if INCLUDE_explicit_bzero
#define explicit_bzero _crypt_explicit_bzero
#endif

#endif
#ifndef _CRYPT_H
#define _CRYPT_H 1

#include <stddef.h>
#include <stdint.h>

/* These are the actual function names from the source files */
extern void crypt_md5crypt_rn(const char *phrase, size_t phr_size,
                              const char *setting, size_t set_size,
                              uint8_t *output, size_t out_size,
                              void *scratch, size_t scr_size);

extern void crypt_sha256crypt_rn(const char *phrase, size_t phr_size,
                                 const char *setting, size_t set_size,
                                 uint8_t *output, size_t out_size,
                                 void *scratch, size_t scr_size);

extern void crypt_sha512crypt_rn(const char *phrase, size_t phr_size,
                                 const char *setting, size_t set_size,
                                 uint8_t *output, size_t out_size,
                                 void *scratch, size_t scr_size);

#endif
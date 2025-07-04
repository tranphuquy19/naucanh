#include "crypt-port.h"

/* Base64 encoding table - exact copy from libxcrypt */
const unsigned char ascii64[65] = 
    "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

/* Generate failure token */
void make_failure_token(const char *setting, char *output, int size) {
    if (size >= 3) {
        strcpy(output, "*0");
    }
}

/* Generate salt for SHA algorithms */
void gensalt_sha_rn(const char *tag, size_t maxsalt, unsigned long defcount,
                    unsigned long mincount, unsigned long maxcount,
                    unsigned long count, const uint8_t *rbytes, size_t nrbytes,
                    uint8_t *output, size_t output_size) {
    if (count != defcount) {
        snprintf((char*)output, output_size, "$%s$rounds=%lu$", tag, count);
    } else {
        snprintf((char*)output, output_size, "$%s$", tag);
    }
}

/* Secure zero memory */
void _crypt_explicit_bzero(void *s, size_t len) {
    volatile unsigned char *p = (volatile unsigned char *)s;
    while (len--) {
        *p++ = 0;
    }
}

/* Safe string copy with overflow protection */
size_t _crypt_strcpy_or_abort(void *dst, size_t d_size, const void *src) {
    size_t len = strlen((const char*)src);
    if (len + 1 > d_size) {
        fprintf(stderr, "Buffer overflow in strcpy_or_abort\n");
        abort();
    }
    memcpy(dst, src, len + 1);
    return len;
}

/* Simple random bytes generator */
bool get_random_bytes(void *buf, size_t buflen) {
    unsigned char *p = (unsigned char *)buf;
    static int seeded = 0;
    
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = 1;
    }
    
    for (size_t i = 0; i < buflen; i++) {
        p[i] = (unsigned char)(rand() & 0xFF);
    }
    return true;
}
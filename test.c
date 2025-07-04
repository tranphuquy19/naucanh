#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "crypt-port.h"
#include "crypt.h"

/* Crypt wrapper function */
char* my_crypt(const char* password, const char* salt) {
    static char output[CRYPT_OUTPUT_SIZE];
    static char scratch[ALG_SPECIFIC_SIZE];
    
    /* Initialize buffers */
    memset(output, 0, sizeof(output));
    memset(scratch, 0, sizeof(scratch));
    
    /* Set failure token as default */
    make_failure_token(salt, output, sizeof(output));
    
    /* Call the appropriate crypt function */
    if (strncmp(salt, "$1$", 3) == 0) {
        crypt_md5crypt_rn(password, strlen(password), salt, strlen(salt),
                         (uint8_t*)output, sizeof(output), scratch, sizeof(scratch));
    } else if (strncmp(salt, "$5$", 3) == 0) {
        crypt_sha256crypt_rn(password, strlen(password), salt, strlen(salt),
                            (uint8_t*)output, sizeof(output), scratch, sizeof(scratch));
    } else if (strncmp(salt, "$6$", 3) == 0) {
        crypt_sha512crypt_rn(password, strlen(password), salt, strlen(salt),
                            (uint8_t*)output, sizeof(output), scratch, sizeof(scratch));
    } else {
        strcpy(output, "*UNSUPPORTED");
    }
    
    return output;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <type> <salt> <password>\n", argv[0]);
        fprintf(stderr, "Types: -1 (MD5), -5 (SHA-256), -6 (SHA-512)\n");
        return 1;
    }
    
    char salt_string[256];
    char *type = argv[1];
    
    /* Build salt string */
    if (!strcmp(type, "-1")) {
        snprintf(salt_string, sizeof(salt_string), "$1$%s", argv[2]);
    } else if (!strcmp(type, "-5")) {
        snprintf(salt_string, sizeof(salt_string), "$5$%s", argv[2]);
    } else if (!strcmp(type, "-6")) {
        snprintf(salt_string, sizeof(salt_string), "$6$%s", argv[2]);
    } else {
        fprintf(stderr, "Unsupported type: %s\n", type);
        return 1;
    }
    
    /* Get hash and print result */
    char *result = my_crypt(argv[3], salt_string);
    puts(result);
    return 0;
}
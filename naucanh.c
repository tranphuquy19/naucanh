#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "crypt-port.h"
#include "crypt.h"

/* Optimized crypt wrapper - reuse single buffer */
static char output[CRYPT_OUTPUT_SIZE];
static char scratch[ALG_SPECIFIC_SIZE];

char* my_crypt(const char* password, const char* salt) {
    memset(output, 0, CRYPT_OUTPUT_SIZE);
    memset(scratch, 0, ALG_SPECIFIC_SIZE);
    
    make_failure_token(salt, output, CRYPT_OUTPUT_SIZE);
    
    size_t plen = strlen(password);
    size_t slen = strlen(salt);
    
    if (salt[1] == '1') {
        crypt_md5crypt_rn(password, plen, salt, slen, (uint8_t*)output, CRYPT_OUTPUT_SIZE, scratch, ALG_SPECIFIC_SIZE);
    } else if (salt[1] == '5') {
        crypt_sha256crypt_rn(password, plen, salt, slen, (uint8_t*)output, CRYPT_OUTPUT_SIZE, scratch, ALG_SPECIFIC_SIZE);
    } else if (salt[1] == '6') {
        crypt_sha512crypt_rn(password, plen, salt, slen, (uint8_t*)output, CRYPT_OUTPUT_SIZE, scratch, ALG_SPECIFIC_SIZE);
    } else {
        strcpy(output, "*UNSUPPORTED");
    }
    
    return output;
}

/* Optimized hash retrieval - single function for both files */
char* get_hash(const char* username) {
    static char hash[512];
    FILE *fp;
    char line[1024];
    char *p, *h;
    
    /* Try /etc/shadow first */
    fp = fopen("/etc/shadow", "r");
    if (fp) {
        while (fgets(line, 1024, fp)) {
            line[strcspn(line, "\n")] = 0;
            p = strchr(line, ':');
            if (p && (p - line) == strlen(username) && !strncmp(line, username, p - line)) {
                h = p + 1;
                p = strchr(h, ':');
                if (p) *p = 0;
                if (*h) {
                    strcpy(hash, h);
                    fclose(fp);
                    return hash;
                }
            }
        }
        fclose(fp);
    }
    
    /* Try /etc/passwd */
    fp = fopen("/etc/passwd", "r");
    if (fp) {
        while (fgets(line, 1024, fp)) {
            line[strcspn(line, "\n")] = 0;
            p = strchr(line, ':');
            if (p && (p - line) == strlen(username) && !strncmp(line, username, p - line)) {
                h = p + 1;
                p = strchr(h, ':');
                if (p) *p = 0;
                if (*h) {
                    strcpy(hash, h);
                    fclose(fp);
                    return hash;
                }
            }
        }
        fclose(fp);
    }
    
    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        return 1;
    }
    
    char *user_hash = get_hash(argv[1]);
    
    if (!user_hash || !*user_hash) {
        fprintf(stderr, "Error: User %s not found or has no password hash\n", argv[1]);
        return 1;
    }
    
    /* Quick format check - only check second character */
    if (user_hash[0] != '$' || (user_hash[1] != '1' && user_hash[1] != '5' && user_hash[1] != '6')) {
        fprintf(stderr, "Error: Unsupported hash format for user %s\n", argv[1]);
        return 1;
    }
    
    FILE *fp = fopen(argv[2], "r");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open password file %s\n", argv[2]);
        return 1;
    }
    
    char password[256];
    while (fgets(password, 256, fp)) {
        password[strcspn(password, "\n\r")] = 0;
        
        if (!*password) continue;
        
        if (!strcmp(my_crypt(password, user_hash), user_hash)) {
            printf("%s FOUND!!! is the password!\n", password);
            fclose(fp);
            return 0;
        } else {
            printf("%s is not the password\n", password);
        }
    }
    
    fclose(fp);
    printf("Password not found in the provided list.\n");
    return 1;
}
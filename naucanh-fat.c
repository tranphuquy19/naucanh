#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "crypt-port.h"
#include "crypt.h"

/* Debug macro */
#define DEBUG_PRINT(fmt, ...) \
    do { \
        if (getenv("DEBUG") && strcmp(getenv("DEBUG"), "1") == 0) { \
            fprintf(stderr, "[DEBUG] " fmt "\n", ##__VA_ARGS__); \
        } \
    } while(0)

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

/* Parse /etc/passwd to find user's password hash */
char* get_user_hash(const char* username) {
    static char hash[512];
    FILE *fp;
    char line[1024];
    char *token;
    int field;
    
    memset(hash, 0, sizeof(hash));
    
    DEBUG_PRINT("Trying to open /etc/passwd");
    
    /* Open /etc/passwd */
    fp = fopen("/etc/passwd", "r");
    if (!fp) {
        DEBUG_PRINT("Failed to open /etc/passwd: %s", strerror(errno));
        return NULL;
    }
    
    DEBUG_PRINT("Successfully opened /etc/passwd");
    
    /* Read line by line */
    while (fgets(line, sizeof(line), fp)) {
        DEBUG_PRINT("Read line: %s", line);
        
        /* Remove newline */
        line[strcspn(line, "\n")] = 0;
        
        /* Parse fields separated by ':' */
        field = 0;
        token = strtok(line, ":");
        
        while (token != NULL) {
            if (field == 0) {
                /* Username field */
                DEBUG_PRINT("Checking username: '%s' against '%s'", token, username);
                if (strcmp(token, username) == 0) {
                    DEBUG_PRINT("Found user %s in /etc/passwd", username);
                    /* Found user, get password hash */
                    token = strtok(NULL, ":");
                    if (token != NULL) {
                        /* Copy password hash */
                        DEBUG_PRINT("Password hash from /etc/passwd: '%s'", token);
                        strncpy(hash, token, sizeof(hash) - 1);
                        hash[sizeof(hash) - 1] = '\0';
                        fclose(fp);
                        return hash;
                    } else {
                        DEBUG_PRINT("No password hash found for user %s", username);
                    }
                }
                break;
            }
            token = strtok(NULL, ":");
            field++;
        }
    }
    
    DEBUG_PRINT("User %s not found in /etc/passwd", username);
    fclose(fp);
    return NULL;
}

/* Parse /etc/shadow to find user's password hash (more likely location) */
char* get_user_shadow_hash(const char* username) {
    static char hash[512];
    FILE *fp;
    char line[1024];
    char *token;
    
    memset(hash, 0, sizeof(hash));
    
    DEBUG_PRINT("Trying to open /etc/shadow");
    
    /* Try to open /etc/shadow */
    fp = fopen("/etc/shadow", "r");
    if (!fp) {
        DEBUG_PRINT("Failed to open /etc/shadow: %s", strerror(errno));
        return NULL;
    }
    
    DEBUG_PRINT("Successfully opened /etc/shadow");
    
    /* Read line by line */
    while (fgets(line, sizeof(line), fp)) {
        DEBUG_PRINT("Read shadow line: %s", line);
        
        /* Remove newline */
        line[strcspn(line, "\n")] = 0;
        
        /* Parse username (first field) */
        token = strtok(line, ":");
        if (token != NULL) {
            DEBUG_PRINT("Checking shadow username: '%s' against '%s'", token, username);
            if (strcmp(token, username) == 0) {
                DEBUG_PRINT("Found user %s in /etc/shadow", username);
                /* Found user, get password hash (second field) */
                token = strtok(NULL, ":");
                if (token != NULL) {
                    DEBUG_PRINT("Password hash from /etc/shadow: '%s'", token);
                    strncpy(hash, token, sizeof(hash) - 1);
                    hash[sizeof(hash) - 1] = '\0';
                    fclose(fp);
                    return hash;
                } else {
                    DEBUG_PRINT("No password hash found for user %s in shadow", username);
                }
            }
        }
    }
    
    DEBUG_PRINT("User %s not found in /etc/shadow", username);
    fclose(fp);
    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <username> <password_list_file>\n", argv[0]);
        return 1;
    }
    
    char *username = argv[1];
    char *password_file = argv[2];
    char *user_hash = NULL;
    FILE *fp;
    char password[256];
    char *computed_hash;
    
    DEBUG_PRINT("Starting password cracker for user: %s", username);
    DEBUG_PRINT("Password file: %s", password_file);
    
    /* Get user's password hash from /etc/shadow first, then /etc/passwd */
    DEBUG_PRINT("Attempting to get user hash from /etc/shadow");
    user_hash = get_user_shadow_hash(username);
    
    if (!user_hash) {
        DEBUG_PRINT("Failed to get hash from /etc/shadow, trying /etc/passwd");
        user_hash = get_user_hash(username);
    }
    
    /* Check if user exists */
    if (!user_hash) {
        DEBUG_PRINT("User %s not found in either /etc/shadow or /etc/passwd", username);
        fprintf(stderr, "Error: User %s not found in system\n", username);
        return 1;
    }
    
    if (strlen(user_hash) == 0) {
        DEBUG_PRINT("User %s found but has empty password hash", username);
        printf("1\n");  /* Empty hash */
        return 1;
    }
    
    DEBUG_PRINT("Found user %s with hash: %s", username, user_hash);
    
    /* Check if password hash is in supported format */
    if (strncmp(user_hash, "$1$", 3) != 0 &&
        strncmp(user_hash, "$5$", 3) != 0 &&
        strncmp(user_hash, "$6$", 3) != 0) {
        DEBUG_PRINT("Unsupported hash format for user %s: %s", username, user_hash);
        printf(stderr, "Error: Unsupported hash format for user %s\n", username);
        printf("1\n");  /* Unsupported hash format */
        return 1;
    }
    
    DEBUG_PRINT("Hash format is supported: %s", user_hash);
    
    /* Open password list file */
    DEBUG_PRINT("Opening password file: %s", password_file);
    fp = fopen(password_file, "r");
    if (!fp) {
        DEBUG_PRINT("Failed to open password file %s: %s", password_file, strerror(errno));
        fprintf(stderr, "Error: Cannot open password file %s\n", password_file);
        return 2;
    }
    
    DEBUG_PRINT("Successfully opened password file");
    
    /* Try each password in the list */
    int password_count = 0;
    while (fgets(password, sizeof(password), fp)) {
        password_count++;
        
        /* Remove newline */
        password[strcspn(password, "\n\r")] = 0;
        
        /* Skip empty lines */
        if (strlen(password) == 0) {
            DEBUG_PRINT("Skipping empty password line %d", password_count);
            continue;
        }
        
        DEBUG_PRINT("Testing password %d: '%s'", password_count, password);
        
        /* Compute hash for this password */
        computed_hash = my_crypt(password, user_hash);
        
        DEBUG_PRINT("Computed hash: %s", computed_hash);
        DEBUG_PRINT("Target hash:   %s", user_hash);
        
        /* Compare with user's hash */
        if (strcmp(computed_hash, user_hash) == 0) {
            DEBUG_PRINT("Password match found!");
            printf("%s FOUND!!! is the password!\n", password);
            fclose(fp);
            return 0;
        } else {
            printf("%s is not the password\n", password);
        }
    }
    
    DEBUG_PRINT("Tested %d passwords, no match found", password_count);
    fclose(fp);
    printf("Password not found in the provided list.\n");
    return 1;
}
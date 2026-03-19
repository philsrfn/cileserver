#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include "../include/auth.h"
#include "../include/logger.h"
#include "../include/protocol.h"

#define MAX_USERS 100
#define LINE_BUFFER_SIZE 256
#define SALT_SIZE 16

static pthread_mutex_t auth_mutex = PTHREAD_MUTEX_INITIALIZER;

// Simple salted hash function for demonstration (should use bcrypt/SHA-256 in production)
static void hash_password(const char *password, const char *salt, char *output, size_t output_size) {
    unsigned char hash[32] = {0};
    size_t pass_len = strlen(password);
    size_t salt_len = strlen(salt);
    
    // Mix password and salt
    for (int round = 0; round < 1000; round++) {
        for (size_t i = 0; i < 32; i++) {
            unsigned char p_char = (pass_len > 0) ? password[(i + round) % pass_len] : 0;
            unsigned char s_char = (salt_len > 0) ? salt[(i * round) % salt_len] : 0;
            hash[i] = (hash[i] + p_char + s_char + round) % 256;
            // Additional mixing
            hash[i] ^= (hash[(i + 1) % 32] >> 1) | (hash[(i + 2) % 32] << 7);
        }
    }
    
    // Convert to hex string
    for (size_t i = 0; i < 32 && i*2+1 < output_size; i++) {
        sprintf(output + i*2, "%02x", hash[i]);
    }
    output[output_size-1] = '\0';
}

// Generate random salt
static void generate_salt(char *salt_hex) {
    unsigned char salt[SALT_SIZE];
    for (int i = 0; i < SALT_SIZE; i++) {
        salt[i] = random() % 256;
    }
    for (int i = 0; i < SALT_SIZE; i++) {
        sprintf(salt_hex + i*2, "%02x", salt[i]);
    }
    salt_hex[SALT_SIZE*2] = '\0';
}

static user_t users[MAX_USERS];
// Add a parallel array for salts to keep structure intact or embed in hash field
// We'll embed it as salt$hash to keep the single password_hash field format
// password_hash will now be: 32 chars salt + '$' + 64 chars hash (total < 100 chars, password_hash is 256)
static int num_users = 0;
static char auth_file_path[1024] = "";

int init_auth(const char *auth_file) {
    FILE *file;
    char line[LINE_BUFFER_SIZE];
    char *username, *password_hash, *role_str, *saveptr;
    
    if (auth_file == NULL) {
        log_error("No authentication file specified");
        return -1;
    }
    
    srandom(time(NULL) ^ getpid()); // seeded for salt gen
    
    pthread_mutex_lock(&auth_mutex);
    strncpy(auth_file_path, auth_file, sizeof(auth_file_path) - 1);
    auth_file_path[sizeof(auth_file_path) - 1] = '\0';
    
    // Try to open auth file
    file = fopen(auth_file_path, "r");
    if (file == NULL) {
        log_warning("Authentication file %s not found, creating new file", auth_file_path);
        
        // Temporarily unlock to avoid deadlock with add_user
        pthread_mutex_unlock(&auth_mutex);
        add_user("admin", "admin", ROLE_ADMIN);
        save_auth_file();
        return 0;
    }
    
    num_users = 0;
    // Parse each line (format: username:password_hash:role)
    while (fgets(line, sizeof(line), file) != NULL && num_users < MAX_USERS) {
        if (line[0] == '#' || line[0] == '\n') continue;
        
        line[strcspn(line, "\n")] = 0;
        
        username = strtok_r(line, ":", &saveptr);
        password_hash = strtok_r(NULL, ":", &saveptr);
        role_str = strtok_r(NULL, ":", &saveptr);
        
        if (username && password_hash && role_str) {
            strncpy(users[num_users].username, username, sizeof(users[num_users].username) - 1);
            strncpy(users[num_users].password_hash, password_hash, sizeof(users[num_users].password_hash) - 1);
            users[num_users].role = (user_role_t)atoi(role_str);
            num_users++;
        }
    }
    
    fclose(file);
    pthread_mutex_unlock(&auth_mutex);
    
    log_info("Loaded %d users from authentication file", num_users);
    return 0;
}

int authenticate_user(const char *username, const char *password, user_role_t *role) {
    if (username == NULL || password == NULL || role == NULL) {
        return -1;
    }
    
    pthread_mutex_lock(&auth_mutex);
    for (int i = 0; i < num_users; i++) {
        if (strcmp(users[i].username, username) == 0) {
            // Check if hash contains salt
            char *salt_sep = strchr(users[i].password_hash, '$');
            if (salt_sep != NULL) {
                // Salted hash
                char salt[64];
                size_t salt_len = salt_sep - users[i].password_hash;
                strncpy(salt, users[i].password_hash, salt_len);
                salt[salt_len] = '\0';
                
                char hash_out[128];
                hash_password(password, salt, hash_out, sizeof(hash_out));
                
                if (strcmp(salt_sep + 1, hash_out) == 0) {
                    *role = users[i].role;
                    pthread_mutex_unlock(&auth_mutex);
                    return 0;
                }
            } else {
                // Legacy unsalted logic (just in case they have an old file)
                char hash_out[128];
                hash_password(password, "", hash_out, sizeof(hash_out));
                if (strcmp(users[i].password_hash, hash_out) == 0) {
                    *role = users[i].role;
                    pthread_mutex_unlock(&auth_mutex);
                    return 0;
                }
            }
            break;
        }
    }
    pthread_mutex_unlock(&auth_mutex);
    
    log_warning("Authentication failed for user %s", username);
    return -1;
}

int add_user(const char *username, const char *password, user_role_t role) {
    if (username == NULL || password == NULL) return -1;
    
    pthread_mutex_lock(&auth_mutex);
    if (num_users >= MAX_USERS) {
        pthread_mutex_unlock(&auth_mutex);
        return -1;
    }
    
    // Check if user already exists
    for (int i = 0; i < num_users; i++) {
        if (strcmp(users[i].username, username) == 0) {
            pthread_mutex_unlock(&auth_mutex);
            return -1;
        }
    }
    
    // Add new user
    strncpy(users[num_users].username, username, sizeof(users[num_users].username) - 1);
    
    char salt[64];
    generate_salt(salt);
    
    char hash_out[128];
    hash_password(password, salt, hash_out, sizeof(hash_out));
    
    snprintf(users[num_users].password_hash, sizeof(users[num_users].password_hash), "%s$%s", salt, hash_out);
    users[num_users].role = role;
    num_users++;
    
    pthread_mutex_unlock(&auth_mutex);
    log_info("Added user %s with role %d", username, role);
    return 0;
}

int remove_user(const char *username) {
    if (username == NULL) return -1;
    
    pthread_mutex_lock(&auth_mutex);
    int found = -1;
    for (int i = 0; i < num_users; i++) {
        if (strcmp(users[i].username, username) == 0) {
            found = i;
            break;
        }
    }
    
    if (found < 0) {
        pthread_mutex_unlock(&auth_mutex);
        return -1;
    }
    
    for (int i = found; i < num_users - 1; i++) {
        memcpy(&users[i], &users[i+1], sizeof(user_t));
    }
    
    num_users--;
    pthread_mutex_unlock(&auth_mutex);
    
    return 0;
}

int save_auth_file(void) {
    pthread_mutex_lock(&auth_mutex);
    if (auth_file_path[0] == '\0') {
        pthread_mutex_unlock(&auth_mutex);
        return -1;
    }
    
    FILE *file = fopen(auth_file_path, "w");
    if (file == NULL) {
        pthread_mutex_unlock(&auth_mutex);
        return -1;
    }
    
    fprintf(file, "# CileServer Authentication File\n");
    fprintf(file, "# Format: username:password_hash:role\n");
    
    for (int i = 0; i < num_users; i++) {
        fprintf(file, "%s:%s:%d\n", 
                users[i].username, 
                users[i].password_hash, 
                users[i].role);
    }
    
    fclose(file);
    pthread_mutex_unlock(&auth_mutex);
    return 0;
}

int check_permission(user_role_t role, int operation) {
    if (role == ROLE_ADMIN) return 1;
    
    if (role == ROLE_USER) {
        switch (operation) {
            case CMD_LIST:
            case CMD_GET:
            case CMD_PUT:
            case CMD_DELETE:
            case CMD_MKDIR:
            case CMD_INFO:
                return 1;
            default:
                return 0;
        }
    }
    
    if (role == ROLE_GUEST) {
        switch (operation) {
            case CMD_LIST:
            case CMD_GET:
            case CMD_INFO:
                return 1;
            default:
                return 0;
        }
    }
    
    return 0;
}

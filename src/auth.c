#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "../include/auth.h"
#include "../include/logger.h"
#include "../include/protocol.h"

#define MAX_USERS 100
#define LINE_BUFFER_SIZE 256
#define SALT_SIZE 16

// Simple hash function (SHA-256 would be better in a real implementation)
static void hash_password(const char *password, char *output, size_t output_size) {
    unsigned char hash[32];
    unsigned int i;
    size_t password_len = strlen(password);
    
    // This is a simple hash for demonstration purposes
    // In a real implementation, use a secure hash function like SHA-256 with a salt
    for (i = 0; i < 32; i++) {
        hash[i] = (password[i % password_len] + i) % 256;
    }
    
    // Convert to hex string
    for (i = 0; i < 32 && i*2+1 < output_size; i++) {
        sprintf(output + i*2, "%02x", hash[i]);
    }
    output[output_size-1] = '\0';
    
    log_debug("Generated hash for password '%s': %s", password, output);
}

static user_t users[MAX_USERS];
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
    
    strncpy(auth_file_path, auth_file, sizeof(auth_file_path) - 1);
    auth_file_path[sizeof(auth_file_path) - 1] = '\0';
    
    // Try to open auth file
    file = fopen(auth_file_path, "r");
    if (file == NULL) {
        log_warning("Authentication file %s not found, creating new file", auth_file_path);
        
        // Create default admin user if file doesn't exist
        add_user("admin", "admin", ROLE_ADMIN);
        save_auth_file();
        
        return 0;
    }
    
    // Parse each line (format: username:password_hash:role)
    while (fgets(line, sizeof(line), file) != NULL && num_users < MAX_USERS) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n') {
            continue;
        }
        
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        // Split line
        username = strtok_r(line, ":", &saveptr);
        password_hash = strtok_r(NULL, ":", &saveptr);
        role_str = strtok_r(NULL, ":", &saveptr);
        
        if (username && password_hash && role_str) {
            strncpy(users[num_users].username, username, sizeof(users[num_users].username) - 1);
            strncpy(users[num_users].password_hash, password_hash, sizeof(users[num_users].password_hash) - 1);
            users[num_users].role = (user_role_t)atoi(role_str);
            log_debug("Loaded user: %s, hash: %s, role: %d", 
                      users[num_users].username, 
                      users[num_users].password_hash, 
                      users[num_users].role);
            num_users++;
        }
    }
    
    fclose(file);
    log_info("Loaded %d users from authentication file", num_users);
    
    return 0;
}

int authenticate_user(const char *username, const char *password, user_role_t *role) {
    char hash[128];
    int i;
    
    if (username == NULL || password == NULL || role == NULL) {
        log_error("Invalid authentication parameters");
        return -1;
    }
    
    log_debug("Attempting authentication for user: %s with password: %s", username, password);
    
    hash_password(password, hash, sizeof(hash));
    log_debug("Calculated hash: %s", hash);
    
    log_debug("Number of users in database: %d", num_users);
    for (i = 0; i < num_users; i++) {
        log_debug("Checking against user[%d]: %s, hash: %s", i, users[i].username, users[i].password_hash);
        
        if (strcmp(users[i].username, username) == 0) {
            log_debug("Username match found for %s", username);
            
            if (strcmp(users[i].password_hash, hash) == 0) {
                *role = users[i].role;
                log_info("User %s authenticated successfully with role %d", username, *role);
                return 0;
            } else {
                log_warning("Authentication failed for user %s: incorrect password (expected hash: %s, got: %s)", 
                           username, users[i].password_hash, hash);
                return -1;
            }
        }
    }
    
    log_warning("Authentication failed: user %s not found", username);
    return -1;
}

int add_user(const char *username, const char *password, user_role_t role) {
    int i;
    
    if (username == NULL || password == NULL || num_users >= MAX_USERS) {
        return -1;
    }
    
    // Check if user already exists
    for (i = 0; i < num_users; i++) {
        if (strcmp(users[i].username, username) == 0) {
            log_warning("User %s already exists", username);
            return -1;
        }
    }
    
    // Add new user
    strncpy(users[num_users].username, username, sizeof(users[num_users].username) - 1);
    hash_password(password, users[num_users].password_hash, sizeof(users[num_users].password_hash));
    users[num_users].role = role;
    num_users++;
    
    log_info("Added user %s with role %d", username, role);
    return 0;
}

int remove_user(const char *username) {
    int i, found = -1;
    
    if (username == NULL) {
        return -1;
    }
    
    // Find user
    for (i = 0; i < num_users; i++) {
        if (strcmp(users[i].username, username) == 0) {
            found = i;
            break;
        }
    }
    
    if (found < 0) {
        log_warning("User %s not found", username);
        return -1;
    }
    
    // Remove user by shifting array
    for (i = found; i < num_users - 1; i++) {
        memcpy(&users[i], &users[i+1], sizeof(user_t));
    }
    
    num_users--;
    log_info("Removed user %s", username);
    
    return 0;
}

int save_auth_file(void) {
    FILE *file;
    int i;
    
    if (auth_file_path[0] == '\0') {
        log_error("Authentication file path not set");
        return -1;
    }
    
    file = fopen(auth_file_path, "w");
    if (file == NULL) {
        log_error("Failed to open authentication file for writing");
        return -1;
    }
    
    fprintf(file, "# CileServer Authentication File\n");
    fprintf(file, "# Format: username:password_hash:role\n");
    
    for (i = 0; i < num_users; i++) {
        fprintf(file, "%s:%s:%d\n", 
                users[i].username, 
                users[i].password_hash, 
                users[i].role);
    }
    
    fclose(file);
    log_info("Saved %d users to authentication file", num_users);
    
    return 0;
}

int check_permission(user_role_t role, int operation) {
    // Admin can do everything
    if (role == ROLE_ADMIN) {
        return 1;
    }
    
    // Users can do most operations
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
    
    // Guests can only read
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

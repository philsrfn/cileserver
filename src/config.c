#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/config.h"
#include "../include/logger.h"

#define LINE_BUFFER_SIZE 1024
#define DEFAULT_PORT 8080
#define DEFAULT_MAX_CONNECTIONS 100
#define DEFAULT_LOG_LEVEL 1  // INFO

static server_config_t config;
static int config_loaded = 0;
static char config_file_path[MAX_PATH_LENGTH] = CONFIG_FILENAME;

// Initialize config with default values
static void init_default_config(void) {
    char cwd[MAX_PATH_LENGTH];
    
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        strncpy(config.root_directory, cwd, sizeof(config.root_directory) - 1);
    } else {
        strncpy(config.root_directory, ".", sizeof(config.root_directory) - 1);
    }
    
    config.max_connections = DEFAULT_MAX_CONNECTIONS;
    config.port = DEFAULT_PORT;
    config.log_level = DEFAULT_LOG_LEVEL;
    config.enable_auth = 0;
    strncpy(config.auth_file, "users.auth", sizeof(config.auth_file) - 1);
}

int set_config_path(const char *path) {
    if (path == NULL) {
        return -1;
    }
    
    strncpy(config_file_path, path, MAX_PATH_LENGTH - 1);
    config_file_path[MAX_PATH_LENGTH - 1] = '\0';
    
    return 0;
}

int load_config(void) {
    FILE *file;
    char line[LINE_BUFFER_SIZE];
    char *key, *value, *saveptr;
    
    // Initialize with defaults first
    init_default_config();
    
    // Try to open config file
    file = fopen(config_file_path, "r");
    if (file == NULL) {
        log_warning("Configuration file %s not found, using defaults", config_file_path);
        config_loaded = 1;
        return 0;
    }
    
    // Parse each line
    while (fgets(line, sizeof(line), file) != NULL) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n') {
            continue;
        }
        
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        // Split into key and value
        key = strtok_r(line, "=", &saveptr);
        value = strtok_r(NULL, "=", &saveptr);
        
        if (key != NULL && value != NULL) {
            // Trim whitespace
            while (*key == ' ') key++;
            while (*value == ' ') value++;
            
            // Set configuration value
            set_config_value(key, value);
        }
    }
    
    fclose(file);
    config_loaded = 1;
    log_info("Configuration loaded from %s", config_file_path);
    
    return 0;
}

int save_config(void) {
    FILE *file;
    
    file = fopen(config_file_path, "w");
    if (file == NULL) {
        log_error("Failed to open configuration file for writing");
        return -1;
    }
    
    fprintf(file, "# CileServer Configuration File\n\n");
    fprintf(file, "root_directory=%s\n", config.root_directory);
    fprintf(file, "max_connections=%d\n", config.max_connections);
    fprintf(file, "port=%d\n", config.port);
    fprintf(file, "log_level=%d\n", config.log_level);
    fprintf(file, "enable_auth=%d\n", config.enable_auth);
    fprintf(file, "auth_file=%s\n", config.auth_file);
    
    fclose(file);
    log_info("Configuration saved to %s", config_file_path);
    
    return 0;
}

server_config_t *get_config(void) {
    if (!config_loaded) {
        load_config();
    }
    
    return &config;
}

int set_config_value(const char *name, const char *value) {
    if (name == NULL || value == NULL) {
        return -1;
    }
    
    if (strcmp(name, "root_directory") == 0) {
        strncpy(config.root_directory, value, sizeof(config.root_directory) - 1);
    } else if (strcmp(name, "max_connections") == 0) {
        config.max_connections = atoi(value);
    } else if (strcmp(name, "port") == 0) {
        config.port = atoi(value);
    } else if (strcmp(name, "log_level") == 0) {
        config.log_level = atoi(value);
        set_log_level((log_level_t)config.log_level);
    } else if (strcmp(name, "enable_auth") == 0) {
        config.enable_auth = atoi(value);
    } else if (strcmp(name, "auth_file") == 0) {
        strncpy(config.auth_file, value, sizeof(config.auth_file) - 1);
    } else {
        log_warning("Unknown configuration parameter: %s", name);
        return -1;
    }
    
    return 0;
} 

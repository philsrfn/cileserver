#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include "../include/file_ops.h"
#include "../include/config.h"
#include "../include/logger.h"

#define MAX_PATH_SIZE 2048

int get_full_path(const char *relative_path, char *out_path, size_t out_size) {
    server_config_t *config = get_config();
    char temp_path[MAX_PATH_SIZE];
    
    // Start with the root directory
    strncpy(temp_path, config->root_directory, sizeof(temp_path) - 1);
    temp_path[sizeof(temp_path) - 1] = '\0';
    
    // Skip leading slash in relative path if present
    const char *rel = relative_path;
    while (*rel == '/') {
        rel++;
    }
    
    // Append the relative path
    if (*rel != '\0') {
        size_t len = strlen(temp_path);
        if (len > 0 && temp_path[len - 1] != '/') {
            strncat(temp_path, "/", sizeof(temp_path) - strlen(temp_path) - 1);
        }
        strncat(temp_path, rel, sizeof(temp_path) - strlen(temp_path) - 1);
    }
    
    // Secure Path Resolution against Traversal
    char resolved_path[PATH_MAX];
    if (realpath(temp_path, resolved_path) == NULL) {
        // If file doesn't exist yet (for write/mkdir), check its parent directory
        char parent_path[MAX_PATH_SIZE];
        strncpy(parent_path, temp_path, sizeof(parent_path) - 1);
        parent_path[sizeof(parent_path) - 1] = '\0';
        
        char *last_slash = strrchr(parent_path, '/');
        if (last_slash != NULL) {
            *last_slash = '\0'; // split parent and filename
            
            if (realpath(parent_path, resolved_path) == NULL) {
                return -1; // Parent directory also does not exist
            }
            
            // Re-append the filename to the resolved parent path
            strncat(resolved_path, "/", sizeof(resolved_path) - strlen(resolved_path) - 1);
            strncat(resolved_path, last_slash + 1, sizeof(resolved_path) - strlen(resolved_path) - 1);
        } else {
            return -1;
        }
    }
    
    // Get absolute path of root directory
    char resolved_root[PATH_MAX];
    if (realpath(config->root_directory, resolved_root) == NULL) {
        return -1;
    }
    
    // Verify that resolved_path starts with resolved_root
    size_t root_len = strlen(resolved_root);
    if (strncmp(resolved_path, resolved_root, root_len) != 0) {
        log_error("Path traversal attempt blocked: %s", relative_path);
        return -1; // Outside root directory
    }
    
    // If the path continues after root_len, it must be a separator or we are at exactly the root
    if (resolved_path[root_len] != '\0' && resolved_path[root_len] != '/') {
        log_error("Path traversal attempt blocked (boundary): %s", relative_path);
        return -1;
    }
    
    strncpy(out_path, resolved_path, out_size - 1);
    out_path[out_size - 1] = '\0';
    return 0;
}

int init_file_ops(void) {
    server_config_t *config = get_config();
    
    // Ensure root directory exists
    struct stat st;
    if (stat(config->root_directory, &st) != 0) {
        log_warning("Root directory %s does not exist, creating it", config->root_directory);
        if (mkdir(config->root_directory, 0755) != 0) {
            log_error("Failed to create root directory: %s", strerror(errno));
            return -1;
        }
    } else if (!S_ISDIR(st.st_mode)) {
        log_error("Root directory path exists but is not a directory");
        return -1;
    }
    
    char resolved_root[PATH_MAX];
    if (realpath(config->root_directory, resolved_root) != NULL) {
        log_info("File operations initialized with absolute root directory: %s", resolved_root);
    }
    return 0;
}

int cleanup_file_ops(void) {
    // Nothing to clean up for now
    return 0;
}

int is_path_valid(const char *path) {
    if (path == NULL || *path == '\0') {
        return 0;
    }
    // Deep path traversal is prevented in get_full_path via realpath logic.
    return 1;
}

int read_file(const char *path, void *buffer, size_t size, size_t *bytes_read) {
    if (!is_path_valid(path)) {
        log_error("Invalid path: %s", path);
        return -1;
    }
    
    char full_path[MAX_PATH_SIZE];
    if (get_full_path(path, full_path, sizeof(full_path)) != 0) {
        log_error("Failed to resolve path safely: %s", path);
        return -1;
    }
    
    FILE *file = fopen(full_path, "rb");
    if (file == NULL) {
        log_error("Failed to open file %s for reading: %s", full_path, strerror(errno));
        return -1;
    }
    
    *bytes_read = fread(buffer, 1, size, file);
    if (*bytes_read == 0 && ferror(file)) {
        log_error("Error reading file %s: %s", full_path, strerror(errno));
        fclose(file);
        return -1;
    }
    
    fclose(file);
    return 0;
}

int write_file(const char *path, const void *buffer, size_t size) {
    if (!is_path_valid(path)) {
        log_error("Invalid path: %s", path);
        return -1;
    }
    
    char full_path[MAX_PATH_SIZE];
    if (get_full_path(path, full_path, sizeof(full_path)) != 0) {
        log_error("Failed to resolve path safely for write: %s", path);
        return -1;
    }
    
    // Check if path is a directory
    struct stat st;
    if (stat(full_path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            log_error("Cannot write to %s: Is a directory", full_path);
            return -1;
        }
    }
    
    FILE *file = fopen(full_path, "wb");
    if (file == NULL) {
        log_error("Failed to open file %s for writing: %s", full_path, strerror(errno));
        return -1;
    }
    
    size_t bytes_written = fwrite(buffer, 1, size, file);
    if (bytes_written != size) {
        log_error("Error writing file %s: %s", full_path, strerror(errno));
        fclose(file);
        return -1;
    }
    
    fclose(file);
    log_info("File %s written successfully (%zu bytes)", path, size);
    return 0;
}

int delete_file(const char *path) {
    if (!is_path_valid(path)) {
        log_error("Invalid path: %s", path);
        return -1;
    }
    
    char full_path[MAX_PATH_SIZE];
    if (get_full_path(path, full_path, sizeof(full_path)) != 0) {
        return -1;
    }
    
    struct stat st;
    if (stat(full_path, &st) != 0) {
        log_error("File %s does not exist", full_path);
        return -1;
    }
    
    if (S_ISDIR(st.st_mode)) {
        if (rmdir(full_path) != 0) {
            log_error("Failed to delete directory %s: %s", full_path, strerror(errno));
            return -1;
        }
    } else {
        if (unlink(full_path) != 0) {
            log_error("Failed to delete file %s: %s", full_path, strerror(errno));
            return -1;
        }
    }
    
    log_info("Deleted %s", path);
    return 0;
}

int list_directory(const char *path, file_info_t *entries, int max_entries, int *num_entries) {
    if (!is_path_valid(path)) {
        log_error("Invalid path: %s", path);
        return -1;
    }
    
    char full_path[MAX_PATH_SIZE];
    if (get_full_path(path, full_path, sizeof(full_path)) != 0) {
        return -1;
    }
    
    DIR *dir = opendir(full_path);
    if (dir == NULL) {
        log_error("Failed to open directory %s: %s", full_path, strerror(errno));
        return -1;
    }
    
    struct dirent *entry;
    int count = 0;
    
    while ((entry = readdir(dir)) != NULL && count < max_entries) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char entry_path[MAX_PATH_SIZE];
        snprintf(entry_path, sizeof(entry_path), "%s/%s", full_path, entry->d_name);
        
        struct stat st;
        if (stat(entry_path, &st) != 0) {
            log_warning("Failed to get info for %s: %s", entry_path, strerror(errno));
            continue;
        }
        
        strncpy(entries[count].name, entry->d_name, sizeof(entries[count].name) - 1);
        entries[count].name[sizeof(entries[count].name) - 1] = '\0';
        entries[count].size = st.st_size;
        entries[count].is_directory = S_ISDIR(st.st_mode) ? 1 : 0;
        entries[count].modified_time = st.st_mtime;
        
        count++;
    }
    
    closedir(dir);
    *num_entries = count;
    
    log_debug("Listed %d entries in directory %s", count, path);
    return 0;
}

int create_directory(const char *path) {
    if (!is_path_valid(path)) {
        log_error("Invalid path: %s", path);
        return -1;
    }
    
    char full_path[MAX_PATH_SIZE];
    if (get_full_path(path, full_path, sizeof(full_path)) != 0) {
        return -1;
    }
    
    if (mkdir(full_path, 0755) != 0) {
        log_error("Failed to create directory %s: %s", full_path, strerror(errno));
        return -1;
    }
    
    log_info("Created directory %s", path);
    return 0;
}

int get_file_info(const char *path, file_info_t *info) {
    if (!is_path_valid(path)) {
        log_error("Invalid path: %s", path);
        return -1;
    }
    
    char full_path[MAX_PATH_SIZE];
    if (get_full_path(path, full_path, sizeof(full_path)) != 0) {
        return -1;
    }
    
    struct stat st;
    if (stat(full_path, &st) != 0) {
        log_error("Failed to get info for %s: %s", full_path, strerror(errno));
        return -1;
    }
    
    const char *filename = strrchr(path, '/');
    if (filename == NULL) {
        filename = path;
    } else {
        filename++;
    }
    
    strncpy(info->name, filename, sizeof(info->name) - 1);
    info->name[sizeof(info->name) - 1] = '\0';
    info->size = st.st_size;
    info->is_directory = S_ISDIR(st.st_mode) ? 1 : 0;
    info->modified_time = st.st_mtime;
    
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include "../include/protocol.h"
#include "../include/file_ops.h"
#include "../include/logger.h"
#include "../include/auth.h"
#include "../include/config.h"

#define BUFFER_SIZE 4096
#define MAX_PATH_LENGTH 1024
#define MAX_ENTRIES 100
#define MAX_USERNAME_LENGTH 64
#define MAX_PASSWORD_LENGTH 64

// Protocol message header
typedef struct {
    uint8_t command;
    uint16_t path_length;
    uint32_t data_length;
} __attribute__((packed)) message_header_t;

// Response header
typedef struct {
    uint8_t status;
    uint32_t data_length;
} __attribute__((packed)) response_header_t;

// Auth message structure
typedef struct {
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
} __attribute__((packed)) auth_message_t;

// Function prototypes for handlers with streaming support
int handle_put_streaming(int client_fd, const char *path, const char *initial_data, size_t initial_len, uint32_t total_len, user_role_t user_role);
int handle_get_streaming(int client_fd, const char *path, user_role_t user_role);

int process_request(int client_fd, const char *buffer, size_t size, user_role_t *user_role) {
    if (size < sizeof(message_header_t)) {
        log_error("Request too small to contain header");
        return -1;
    }
    
    const message_header_t *header = (const message_header_t *)buffer;
    uint8_t command = header->command;
    uint16_t path_length = ntohs(header->path_length);
    uint32_t data_length = ntohl(header->data_length);
    
    // Validate that we at least have the path in this initial buffer
    if (size < sizeof(message_header_t) + path_length) {
        log_error("Incomplete request message (path missing)");
        return -1;
    }
    
    // Extract path
    char path[MAX_PATH_LENGTH];
    if (path_length >= MAX_PATH_LENGTH) {
        log_error("Path too long");
        return -1;
    }
    
    memcpy(path, buffer + sizeof(message_header_t), path_length);
    path[path_length] = '\0';
    
    // Calculate how much actual data is in the initial buffer
    size_t header_and_path_len = sizeof(message_header_t) + path_length;
    size_t initial_data_len = size - header_and_path_len;
    const char *initial_data = buffer + header_and_path_len;
    
    if (initial_data_len > data_length) {
        initial_data_len = data_length;
    }
    
    log_debug("Received command %d for path %s (data_length=%u, initial_read=%zu)", 
              command, path, data_length, initial_data_len);
    
    // Check if authentication is required
    server_config_t *config = get_config();
    if (config->enable_auth && command != CMD_AUTH && *user_role == ROLE_GUEST) {
        log_warning("Authentication required for command %d", command);
        return send_response(client_fd, RESP_AUTH_REQUIRED, "Authentication required", 23);
    }
    
    // Process command
    switch (command) {
        case CMD_AUTH:
            if (initial_data_len >= sizeof(auth_message_t)) {
                const auth_message_t *auth_data = (const auth_message_t *)initial_data;
                return handle_auth_command(client_fd, auth_data->username, auth_data->password, user_role);
            } else {
                log_error("Invalid authentication data");
                return send_response(client_fd, RESP_ERROR, "Invalid authentication data", 27);
            }
        
        case CMD_LOGOUT:
            return handle_logout_command(client_fd, user_role);
            
        case CMD_LIST:
            return handle_list_command(client_fd, path, *user_role);
        
        case CMD_GET:
            return handle_get_streaming(client_fd, path, *user_role);
        
        case CMD_PUT:
            return handle_put_streaming(client_fd, path, initial_data, initial_data_len, data_length, *user_role);
        
        case CMD_DELETE:
            return handle_delete_command(client_fd, path, *user_role);
        
        case CMD_MKDIR:
            return handle_mkdir_command(client_fd, path, *user_role);
        
        case CMD_INFO:
            return handle_info_command(client_fd, path, *user_role);
        
        default:
            log_error("Unknown command: %d", command);
            return send_response(client_fd, RESP_ERROR, "Unknown command", 15);
    }
}

int send_response(int client_fd, int status, const void *data, size_t data_size) {
    response_header_t header;
    header.status = status;
    header.data_length = htonl(data_size);
    
    // Send header
    if (write(client_fd, &header, sizeof(header)) != sizeof(header)) {
        log_error("Failed to send response header");
        return -1;
    }
    
    // Send data if present
    if (data != NULL && data_size > 0) {
        size_t written = 0;
        while (written < data_size) {
            ssize_t res = write(client_fd, (const char *)data + written, data_size - written);
            if (res < 0) {
                if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
                    usleep(1000);
                    continue;
                }
                log_error("Failed to send response data");
                return -1;
            }
            written += res;
        }
    }
    
    return 0;
}

int handle_auth_command(int client_fd, const char *username, const char *password, user_role_t *user_role) {
    int result;
    
    log_info("Authentication attempt for user %s", username);
    
    result = authenticate_user(username, password, user_role);
    if (result == 0) {
        char response[64];
        snprintf(response, sizeof(response), "Authenticated as %s (role %d)", username, *user_role);
        return send_response(client_fd, RESP_OK, response, strlen(response));
    } else {
        return send_response(client_fd, RESP_ERROR, "Authentication failed", 21);
    }
}

int handle_logout_command(int client_fd, user_role_t *user_role) {
    *user_role = ROLE_GUEST;
    log_info("User logged out, role set to guest");
    return send_response(client_fd, RESP_OK, "Logged out", 10);
}

int handle_list_command(int client_fd, const char *path, user_role_t user_role) {
    file_info_t entries[MAX_ENTRIES];
    int num_entries;
    
    if (!check_permission(user_role, CMD_LIST)) {
        return send_response(client_fd, RESP_ERROR, "Permission denied", 17);
    }
    
    if (list_directory(path, entries, MAX_ENTRIES, &num_entries) != 0) {
        return send_response(client_fd, RESP_ERROR, "Failed to list directory", 24);
    }
    
    size_t response_size = num_entries * sizeof(file_info_t);
    return send_response(client_fd, RESP_OK, entries, response_size);
}

int handle_get_streaming(int client_fd, const char *path, user_role_t user_role) {
    if (!check_permission(user_role, CMD_GET)) {
        return send_response(client_fd, RESP_ERROR, "Permission denied", 17);
    }

    file_info_t info;
    if (get_file_info(path, &info) != 0 || info.is_directory) {
        return send_response(client_fd, RESP_ERROR, "Failed to read file", 19);
    }
    
    // We send RESP_OK with data_length = file size
    response_header_t header;
    header.status = RESP_OK;
    header.data_length = htonl((uint32_t)info.size);
    if (write(client_fd, &header, sizeof(header)) != sizeof(header)) {
        return -1;
    }
    
    char resolved_path[1024];
    if (get_full_path(path, resolved_path, sizeof(resolved_path)) != 0) return -1;
    
    FILE *f = fopen(resolved_path, "rb");
    if (!f) return -1;
    
    char stream_buf[BUFFER_SIZE];
    size_t sent = 0;
    while (sent < info.size) {
        size_t count = fread(stream_buf, 1, sizeof(stream_buf), f);
        if (count == 0) break;
        size_t written = 0;
        while (written < count) {
            ssize_t w = write(client_fd, stream_buf + written, count - written);
            if (w < 0) {
                if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
                    usleep(1000);
                    continue;
                }
                fclose(f);
                return -1;
            }
            written += w;
        }
        sent += count;
    }
    fclose(f);
    return 0;
}

int handle_put_streaming(int client_fd, const char *path, const char *initial_data, size_t initial_len, uint32_t total_len, user_role_t user_role) {
    if (!check_permission(user_role, CMD_PUT)) {
        return send_response(client_fd, RESP_ERROR, "Permission denied", 17);
    }
    
    char full_path[1024];
    if (get_full_path(path, full_path, sizeof(full_path)) != 0) {
        return send_response(client_fd, RESP_ERROR, "Invalid path", 12);
    }
    
    FILE *f = fopen(full_path, "wb");
    if (!f) {
        return send_response(client_fd, RESP_ERROR, "Failed to write file", 20);
    }
    
    // Write initial data
    if (initial_len > 0) {
        fwrite(initial_data, 1, initial_len, f);
    }
    
    // Read the rest from socket
    uint32_t remaining = total_len - initial_len;
    char stream_buf[BUFFER_SIZE];
    while (remaining > 0) {
        size_t to_read = remaining < BUFFER_SIZE ? remaining : BUFFER_SIZE;
        ssize_t r = read(client_fd, stream_buf, to_read);
        if (r < 0) {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(1000); // Back off
                continue;
            }
            fclose(f);
            return -1;
        } else if (r == 0) {
            // connection closed prematurely
            break;
        }
        fwrite(stream_buf, 1, r, f);
        remaining -= r;
    }
    fclose(f);
    
    if (remaining == 0) {
        return send_response(client_fd, RESP_OK, "File written successfully", 25);
    } else {
        return -1; // disconnected early
    }
}

// Stubs for remaining since handle_put_command was redefined over old one
int handle_put_command(int client_fd, const char *path, const void *data, size_t data_size, user_role_t user_role) {
    return handle_put_streaming(client_fd, path, data, data_size, data_size, user_role);
}
int handle_get_command(int client_fd, const char *path, user_role_t user_role) {
    return handle_get_streaming(client_fd, path, user_role);
}

int handle_delete_command(int client_fd, const char *path, user_role_t user_role) {
    if (!check_permission(user_role, CMD_DELETE)) return send_response(client_fd, RESP_ERROR, "Permission denied", 17);
    if (delete_file(path) != 0) return send_response(client_fd, RESP_ERROR, "Failed to delete file", 21);
    return send_response(client_fd, RESP_OK, "File deleted successfully", 25);
}

int handle_mkdir_command(int client_fd, const char *path, user_role_t user_role) {
    if (!check_permission(user_role, CMD_MKDIR)) return send_response(client_fd, RESP_ERROR, "Permission denied", 17);
    if (create_directory(path) != 0) return send_response(client_fd, RESP_ERROR, "Failed to create dir", 20);
    return send_response(client_fd, RESP_OK, "Directory created successfully", 30);
}

int handle_info_command(int client_fd, const char *path, user_role_t user_role) {
    file_info_t info;
    if (!check_permission(user_role, CMD_INFO)) return send_response(client_fd, RESP_ERROR, "Permission denied", 17);
    if (get_file_info(path, &info) != 0) return send_response(client_fd, RESP_ERROR, "Failed to get file info", 23);
    return send_response(client_fd, RESP_OK, &info, sizeof(info));
}

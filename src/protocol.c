#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
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

int process_request(int client_fd, const char *buffer, size_t size, user_role_t *user_role) {
    if (size < sizeof(message_header_t)) {
        log_error("Request too small to contain header");
        return -1;
    }
    
    const message_header_t *header = (const message_header_t *)buffer;
    uint8_t command = header->command;
    uint16_t path_length = ntohs(header->path_length);
    uint32_t data_length = ntohl(header->data_length);
    
    // Validate message size
    if (size < sizeof(message_header_t) + path_length + data_length) {
        log_error("Incomplete request message");
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
    
    // Extract data if present
    const void *data = NULL;
    if (data_length > 0) {
        data = buffer + sizeof(message_header_t) + path_length;
    }
    
    log_debug("Received command %d for path %s", command, path);
    
    // Check if authentication is required
    server_config_t *config = get_config();
    if (config->enable_auth && command != CMD_AUTH && *user_role == ROLE_GUEST) {
        log_warning("Authentication required for command %d", command);
        return send_response(client_fd, RESP_AUTH_REQUIRED, "Authentication required", 24);
    }
    
    // Process command
    switch (command) {
        case CMD_AUTH:
            if (data_length >= sizeof(auth_message_t)) {
                const auth_message_t *auth_data = (const auth_message_t *)data;
                return handle_auth_command(client_fd, auth_data->username, auth_data->password, user_role);
            } else {
                log_error("Invalid authentication data");
                return send_response(client_fd, RESP_ERROR, "Invalid authentication data", 25);
            }
        
        case CMD_LOGOUT:
            return handle_logout_command(client_fd, user_role);
            
        case CMD_LIST:
            return handle_list_command(client_fd, path, *user_role);
        
        case CMD_GET:
            return handle_get_command(client_fd, path, *user_role);
        
        case CMD_PUT:
            return handle_put_command(client_fd, path, data, data_length, *user_role);
        
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
        ssize_t bytes_written = write(client_fd, data, data_size);
        if (bytes_written < 0 || (size_t)bytes_written != data_size) {
            log_error("Failed to send response data");
            return -1;
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
        return send_response(client_fd, RESP_ERROR, "Authentication failed", 20);
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
    
    // Check permissions
    if (!check_permission(user_role, CMD_LIST)) {
        log_warning("Permission denied for LIST command");
        return send_response(client_fd, RESP_ERROR, "Permission denied", 17);
    }
    
    if (list_directory(path, entries, MAX_ENTRIES, &num_entries) != 0) {
        return send_response(client_fd, RESP_ERROR, "Failed to list directory", 23);
    }
    
    // Calculate response size
    size_t response_size = num_entries * sizeof(file_info_t);
    
    // Send response
    return send_response(client_fd, RESP_OK, entries, response_size);
}

int handle_get_command(int client_fd, const char *path, user_role_t user_role) {
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    
    // Check permissions
    if (!check_permission(user_role, CMD_GET)) {
        log_warning("Permission denied for GET command");
        return send_response(client_fd, RESP_ERROR, "Permission denied", 17);
    }
    
    if (read_file(path, buffer, BUFFER_SIZE, &bytes_read) != 0) {
        return send_response(client_fd, RESP_ERROR, "Failed to read file", 18);
    }
    
    // Send response
    return send_response(client_fd, RESP_OK, buffer, bytes_read);
}

int handle_put_command(int client_fd, const char *path, const void *data, size_t data_size, user_role_t user_role) {
    // Check permissions
    if (!check_permission(user_role, CMD_PUT)) {
        log_warning("Permission denied for PUT command");
        return send_response(client_fd, RESP_ERROR, "Permission denied", 17);
    }
    
    if (write_file(path, data, data_size) != 0) {
        return send_response(client_fd, RESP_ERROR, "Failed to write file", 19);
    }
    
    // Send success response
    return send_response(client_fd, RESP_OK, "File written successfully", 24);
}

int handle_delete_command(int client_fd, const char *path, user_role_t user_role) {
    // Check permissions
    if (!check_permission(user_role, CMD_DELETE)) {
        log_warning("Permission denied for DELETE command");
        return send_response(client_fd, RESP_ERROR, "Permission denied", 17);
    }
    
    if (delete_file(path) != 0) {
        return send_response(client_fd, RESP_ERROR, "Failed to delete file", 20);
    }
    
    // Send success response
    return send_response(client_fd, RESP_OK, "File deleted successfully", 24);
}

int handle_mkdir_command(int client_fd, const char *path, user_role_t user_role) {
    // Check permissions
    if (!check_permission(user_role, CMD_MKDIR)) {
        log_warning("Permission denied for MKDIR command");
        return send_response(client_fd, RESP_ERROR, "Permission denied", 17);
    }
    
    if (create_directory(path) != 0) {
        return send_response(client_fd, RESP_ERROR, "Failed to create directory", 25);
    }
    
    // Send success response
    return send_response(client_fd, RESP_OK, "Directory created successfully", 29);
}

int handle_info_command(int client_fd, const char *path, user_role_t user_role) {
    file_info_t info;
    
    // Check permissions
    if (!check_permission(user_role, CMD_INFO)) {
        log_warning("Permission denied for INFO command");
        return send_response(client_fd, RESP_ERROR, "Permission denied", 17);
    }
    
    if (get_file_info(path, &info) != 0) {
        return send_response(client_fd, RESP_ERROR, "Failed to get file info", 22);
    }
    
    // Send response
    return send_response(client_fd, RESP_OK, &info, sizeof(info));
} 

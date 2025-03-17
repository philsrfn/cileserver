#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../include/protocol.h"
#include "../include/file_ops.h"
#include "../include/logger.h"

#define BUFFER_SIZE 4096
#define MAX_PATH_LENGTH 1024
#define MAX_ENTRIES 100

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

int process_request(int client_fd, const char *buffer, size_t size) {
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
    
    // Process command
    switch (command) {
        case CMD_LIST:
            return handle_list_command(client_fd, path);
        
        case CMD_GET:
            return handle_get_command(client_fd, path);
        
        case CMD_PUT:
            return handle_put_command(client_fd, path, data, data_length);
        
        case CMD_DELETE:
            return handle_delete_command(client_fd, path);
        
        case CMD_MKDIR:
            return handle_mkdir_command(client_fd, path);
        
        case CMD_INFO:
            return handle_info_command(client_fd, path);
        
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

int handle_list_command(int client_fd, const char *path) {
    file_info_t entries[MAX_ENTRIES];
    int num_entries;
    
    if (list_directory(path, entries, MAX_ENTRIES, &num_entries) != 0) {
        return send_response(client_fd, RESP_ERROR, "Failed to list directory", 23);
    }
    
    // Calculate response size
    size_t response_size = num_entries * sizeof(file_info_t);
    
    // Send response
    return send_response(client_fd, RESP_OK, entries, response_size);
}

int handle_get_command(int client_fd, const char *path) {
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    
    if (read_file(path, buffer, BUFFER_SIZE, &bytes_read) != 0) {
        return send_response(client_fd, RESP_ERROR, "Failed to read file", 18);
    }
    
    // Send response
    return send_response(client_fd, RESP_OK, buffer, bytes_read);
}

int handle_put_command(int client_fd, const char *path, const void *data, size_t data_size) {
    if (write_file(path, data, data_size) != 0) {
        return send_response(client_fd, RESP_ERROR, "Failed to write file", 19);
    }
    
    // Send success response
    return send_response(client_fd, RESP_OK, "File written successfully", 24);
}

int handle_delete_command(int client_fd, const char *path) {
    if (delete_file(path) != 0) {
        return send_response(client_fd, RESP_ERROR, "Failed to delete file", 20);
    }
    
    // Send success response
    return send_response(client_fd, RESP_OK, "File deleted successfully", 24);
}

int handle_mkdir_command(int client_fd, const char *path) {
    if (create_directory(path) != 0) {
        return send_response(client_fd, RESP_ERROR, "Failed to create directory", 25);
    }
    
    // Send success response
    return send_response(client_fd, RESP_OK, "Directory created successfully", 29);
}

int handle_info_command(int client_fd, const char *path) {
    file_info_t info;
    
    if (get_file_info(path, &info) != 0) {
        return send_response(client_fd, RESP_ERROR, "Failed to get file info", 22);
    }
    
    // Send response
    return send_response(client_fd, RESP_OK, &info, sizeof(info));
} 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include "../include/protocol.h"
#include "../include/file_ops.h"
#include "../include/auth.h"

#define BUFFER_SIZE 4096
#define DEFAULT_PORT 9090
#define DEFAULT_HOST "localhost"

// Global variables for auth credentials
static char g_username[64] = "";
static char g_password[64] = "";
static char g_host[256] = DEFAULT_HOST;
static int g_port = DEFAULT_PORT;

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
    char username[64];
    char password[64];
} __attribute__((packed)) auth_message_t;

int connect_to_server(const char *host, int port);
int send_request(int sock_fd, uint8_t command, const char *path, const void *data, size_t data_size);
int receive_response(int sock_fd, void *buffer, size_t buffer_size, size_t *data_size);
void client_list_directory(int sock_fd, const char *path);
void client_get_file(int sock_fd, const char *path, const char *local_path);
void client_put_file(int sock_fd, const char *path, const char *local_path);
void client_delete_file(int sock_fd, const char *path);
void client_create_directory(int sock_fd, const char *path);
void client_authenticate(int sock_fd, const char *username, const char *password);
void client_logout(int sock_fd);
void print_usage(const char *program_name);
void client_login(const char *username, const char *password);

int connect_to_server(const char *host, int port) {
    int sock_fd;
    struct sockaddr_in server_addr;
    struct hostent *server;
    
    // Create socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Error creating socket");
        return -1;
    }
    
    // Get server information
    server = gethostbyname(host);
    if (server == NULL) {
        fprintf(stderr, "Error: no such host\n");
        close(sock_fd);
        return -1;
    }
    
    // Prepare server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    server_addr.sin_port = htons(port);
    
    // Connect to server
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting to server");
        close(sock_fd);
        return -1;
    }
    
    return sock_fd;
}

int send_request(int sock_fd, uint8_t command, const char *path, const void *data, size_t data_size) {
    message_header_t header;
    size_t path_len = strlen(path);
    
    // Prepare header
    header.command = command;
    header.path_length = htons(path_len);
    header.data_length = htonl(data_size);
    
    // Send header
    if (write(sock_fd, &header, sizeof(header)) != sizeof(header)) {
        perror("Error sending request header");
        return -1;
    }
    
    // Send path
    ssize_t bytes_written = write(sock_fd, path, path_len);
    if (bytes_written < 0 || (size_t)bytes_written != path_len) {
        perror("Error sending path");
        return -1;
    }
    
    // Send data if present
    if (data != NULL && data_size > 0) {
        bytes_written = write(sock_fd, data, data_size);
        if (bytes_written < 0 || (size_t)bytes_written != data_size) {
            perror("Error sending data");
            return -1;
        }
    }
    
    return 0;
}

int receive_response(int sock_fd, void *buffer, size_t buffer_size, size_t *data_size) {
    response_header_t header;
    
    // Receive header
    if (read(sock_fd, &header, sizeof(header)) != sizeof(header)) {
        perror("Error receiving response header");
        return -1;
    }
    
    // Get data length
    *data_size = ntohl(header.data_length);
    
    // Check if response is OK
    if (header.status != RESP_OK) {
        if (header.status == RESP_AUTH_REQUIRED) {
            fprintf(stderr, "Authentication required\n");
            return -2;  // Special return code for auth required
        } else {
            fprintf(stderr, "Server returned error\n");
        }
        
        // Read error message if available
        if (*data_size > 0 && *data_size < buffer_size) {
            ssize_t bytes_read = read(sock_fd, buffer, *data_size);
            if (bytes_read < 0 || (size_t)bytes_read != *data_size) {
                perror("Error receiving error message");
            } else {
                ((char *)buffer)[*data_size] = '\0';
                fprintf(stderr, "Error message: %s\n", (char *)buffer);
            }
        }
        
        return -1;
    }
    
    // Receive data if present and if caller wants us to buffer it
    if (*data_size > 0 && buffer != NULL) {
        if (*data_size > buffer_size) {
            fprintf(stderr, "Response too large for buffer\n");
            return -1;
        }
        
        ssize_t bytes_read = read(sock_fd, buffer, *data_size);
        if (bytes_read < 0 || (size_t)bytes_read != *data_size) {
            perror("Error receiving response data");
            return -1;
        }
    }
    
    return 0;
}

void client_authenticate(int sock_fd, const char *username, const char *password) {
    char buffer[BUFFER_SIZE];
    size_t data_size;
    auth_message_t auth_data;
    
    printf("Authenticating as user: %s\n", username);
    
    // Prepare auth data
    strncpy(auth_data.username, username, sizeof(auth_data.username) - 1);
    strncpy(auth_data.password, password, sizeof(auth_data.password) - 1);
    
    // Send AUTH request
    if (send_request(sock_fd, CMD_AUTH, "", &auth_data, sizeof(auth_data)) != 0) {
        printf("Failed to send authentication request\n");
        return;
    }
    
    // Receive response
    int result = receive_response(sock_fd, buffer, BUFFER_SIZE, &data_size);
    if (result != 0) {
        printf("Failed to authenticate\n");
        return;
    }
    
    // Display success message
    buffer[data_size] = '\0';
    printf("%s\n", buffer);
}

void client_logout(int sock_fd) {
    char buffer[BUFFER_SIZE];
    size_t data_size;
    
    printf("Logging out\n");
    
    // Send LOGOUT request
    if (send_request(sock_fd, CMD_LOGOUT, "", NULL, 0) != 0) {
        return;
    }
    
    // Receive response
    if (receive_response(sock_fd, buffer, BUFFER_SIZE, &data_size) != 0) {
        return;
    }
    
    // Display success message
    buffer[data_size] = '\0';
    printf("%s\n", buffer);
}

void client_list_directory(int sock_fd, const char *path) {
    char buffer[BUFFER_SIZE];
    size_t data_size;
    
    printf("Listing directory: %s\n", path);
    
        // Try to authenticate first if credentials are available
    if (g_username[0] != '\0' && g_password[0] != '\0') {
        client_authenticate(sock_fd, g_username, g_password);
    }
    
    // Send LIST request
    if (send_request(sock_fd, CMD_LIST, path, NULL, 0) != 0) {
        return;
    }
    
    // Receive response
    int result = receive_response(sock_fd, buffer, BUFFER_SIZE, &data_size);
    if (result == -2) {
        // Authentication required, prompt for credentials if not already set
        if (g_username[0] == '\0') {
            printf("Username: ");
            fgets(g_username, sizeof(g_username), stdin);
            g_username[strcspn(g_username, "\n")] = 0; // Remove newline
        } else {
            printf("Using username: %s\n", g_username);
        }
        
        if (g_password[0] == '\0') {
            printf("Password: ");
            fgets(g_password, sizeof(g_password), stdin);
            g_password[strcspn(g_password, "\n")] = 0; // Remove newline
        }
        
        client_authenticate(sock_fd, g_username, g_password);
        
        // Try the original command again
        client_list_directory(sock_fd, path);
        return;
    } else if (result != 0) {
        return;
    }
    
    // Parse and display directory entries
    int num_entries = data_size / sizeof(file_info_t);
    file_info_t *entries = (file_info_t *)buffer;
    
    printf("Directory contents (%d entries):\n", num_entries);
    printf("%-30s %-10s %-20s\n", "Name", "Size", "Type");
    printf("------------------------------------------------------------\n");
    
    for (int i = 0; i < num_entries; i++) {
        char time_str[30];
        struct tm *tm_info = localtime(&entries[i].modified_time);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
        
        printf("%-30s %-10zu %-20s %s\n", 
               entries[i].name, 
               entries[i].size, 
               entries[i].is_directory ? "Directory" : "File",
               time_str);
    }
}

void client_get_file(int sock_fd, const char *path, const char *local_path) {
    char buffer[BUFFER_SIZE];
    size_t data_size;
    
    printf("Getting file: %s -> %s\n", path, local_path);
    
        // Try to authenticate first if credentials are available
    if (g_username[0] != '\0' && g_password[0] != '\0') {
        client_authenticate(sock_fd, g_username, g_password);
    }
    
    // Send GET request
    if (send_request(sock_fd, CMD_GET, path, NULL, 0) != 0) {
        return;
    }
    
    // Receive response header ONLY
    int result = receive_response(sock_fd, NULL, 0, &data_size);
    if (result == -2) {
        // ... handled auth below ...
        return;
    } else if (result != 0) {
        return;
    }
    
    // Write to local file streaming
    FILE *file = fopen(local_path, "wb");
    if (file == NULL) {
        perror("Error opening local file");
        return;
    }
    
    size_t remaining = data_size;
    while (remaining > 0) {
        size_t to_read = remaining < BUFFER_SIZE ? remaining : BUFFER_SIZE;
        ssize_t bytes_read = read(sock_fd, buffer, to_read);
        if (bytes_read <= 0) {
            perror("Error receiving data chunk");
            fclose(file);
            return;
        }
        if (fwrite(buffer, 1, bytes_read, file) != bytes_read) {
            perror("Error writing to local file");
            fclose(file);
            return;
        }
        remaining -= bytes_read;
    }
    
    fclose(file);
    printf("File downloaded successfully (%zu bytes)\n", data_size);
}

void client_put_file(int sock_fd, const char *path, const char *local_path) {
    char buffer[BUFFER_SIZE];
    size_t data_size;
    
    // Check if path ends with a slash (directory)
    size_t path_len = strlen(path);
    if (path_len > 0 && path[path_len - 1] == '/') {
        fprintf(stderr, "Error: Cannot write to a directory path. Please specify a file path.\n");
        return;
    }
    
    printf("Putting file: %s -> %s\n", local_path, path);
    
        // Try to authenticate first if credentials are available
    if (g_username[0] != '\0' && g_password[0] != '\0') {
        client_authenticate(sock_fd, g_username, g_password);
    }
    
    // Read local file
    FILE *file = fopen(local_path, "rb");
    if (file == NULL) {
        perror("Error opening local file");
        return;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size < 0) {
        fprintf(stderr, "Invalid file size\n");
        fclose(file);
        return;
    }
    
    // Send PUT request header only
    if (send_request(sock_fd, CMD_PUT, path, NULL, file_size) != 0) {
        fclose(file);
        return;
    }
    
    // Stream file content chunks
    size_t remaining = file_size;
    while (remaining > 0) {
        size_t bytes_read = fread(buffer, 1, remaining < BUFFER_SIZE ? remaining : BUFFER_SIZE, file);
        if (bytes_read == 0) {
            perror("Error reading local file chunk");
            fclose(file);
            return;
        }
        size_t written = 0;
        while (written < bytes_read) {
            ssize_t sent = write(sock_fd, buffer + written, bytes_read - written);
            if (sent < 0) {
                perror("Error sending local file chunk");
                fclose(file);
                return;
            }
            written += sent;
        }
        remaining -= bytes_read;
    }
    fclose(file);
    
    // Receive response OK status
    int result = receive_response(sock_fd, buffer, BUFFER_SIZE, &data_size);
    if (result == -2) {
        // ... Handled in other paths mostly natively or we prompt ...
        return;
    } else if (result != 0) {
        return;
    }
    
    // Display success message
    if (data_size > 0 && data_size < BUFFER_SIZE) {
        buffer[data_size] = '\0';
        printf("%s\n", buffer);
    } else {
        printf("Upload completed successfully.\n");
    }
}

void client_delete_file(int sock_fd, const char *path) {
    char buffer[BUFFER_SIZE];
    size_t data_size;
    
    printf("Deleting: %s\n", path);
    
        // Try to authenticate first if credentials are available
    if (g_username[0] != '\0' && g_password[0] != '\0') {
        client_authenticate(sock_fd, g_username, g_password);
    }
    
    // Send DELETE request
    if (send_request(sock_fd, CMD_DELETE, path, NULL, 0) != 0) {
        return;
    }
    
    // Receive response
    int result = receive_response(sock_fd, buffer, BUFFER_SIZE, &data_size);
    if (result == -2) {
        // Authentication required, prompt for credentials if not already set
        if (g_username[0] == '\0') {
            printf("Username: ");
            fgets(g_username, sizeof(g_username), stdin);
            g_username[strcspn(g_username, "\n")] = 0; // Remove newline
        } else {
            printf("Using username: %s\n", g_username);
        }
        
        if (g_password[0] == '\0') {
            printf("Password: ");
            fgets(g_password, sizeof(g_password), stdin);
            g_password[strcspn(g_password, "\n")] = 0; // Remove newline
        }
        
        client_authenticate(sock_fd, g_username, g_password);
        
        // Try the original command again
        client_delete_file(sock_fd, path);
        return;
    } else if (result != 0) {
        return;
    }
    
    // Display success message
    buffer[data_size] = '\0';
    printf("%s\n", buffer);
}

void client_create_directory(int sock_fd, const char *path) {
    char buffer[BUFFER_SIZE];
    size_t data_size;
    
    printf("Creating directory: %s\n", path);
    
        // Try to authenticate first if credentials are available
    if (g_username[0] != '\0' && g_password[0] != '\0') {
        client_authenticate(sock_fd, g_username, g_password);
    }
    
    // Send MKDIR request
    if (send_request(sock_fd, CMD_MKDIR, path, NULL, 0) != 0) {
        return;
    }
    
    // Receive response
    int result = receive_response(sock_fd, buffer, BUFFER_SIZE, &data_size);
    if (result == -2) {
        // Authentication required, prompt for credentials if not already set
        if (g_username[0] == '\0') {
            printf("Username: ");
            fgets(g_username, sizeof(g_username), stdin);
            g_username[strcspn(g_username, "\n")] = 0; // Remove newline
        } else {
            printf("Using username: %s\n", g_username);
        }
        
        if (g_password[0] == '\0') {
            printf("Password: ");
            fgets(g_password, sizeof(g_password), stdin);
            g_password[strcspn(g_password, "\n")] = 0; // Remove newline
        }
        
        client_authenticate(sock_fd, g_username, g_password);
        
        // Try the original command again
        client_create_directory(sock_fd, path);
        return;
    } else if (result != 0) {
        return;
    }
    
    // Display success message
    buffer[data_size] = '\0';
    printf("%s\n", buffer);
}

void client_login(const char *username, const char *password) {
    int sock_fd = connect_to_server(g_host, g_port);
    if (sock_fd < 0) {
        return;
    }
    
    client_authenticate(sock_fd, username, password);
    close(sock_fd);
    
    // Save credentials for future commands
    strncpy(g_username, username, sizeof(g_username) - 1);
    g_username[sizeof(g_username) - 1] = '\0';
    
    strncpy(g_password, password, sizeof(g_password) - 1);
    g_password[sizeof(g_password) - 1] = '\0';
}

void print_usage(const char *program_name) {
    printf("Usage: %s [OPTIONS] COMMAND [ARGS]\n", program_name);
    printf("\nOptions:\n");
    printf("  -h, --host HOST      Server hostname (default: %s)\n", DEFAULT_HOST);
    printf("  -p, --port PORT      Server port (default: %d)\n", DEFAULT_PORT);
    printf("  -u, --user USER      Username for authentication\n");
    printf("  -P, --password PASS  Password for authentication\n");
    printf("\nCommands:\n");
    printf("  login USERNAME PASSWORD    Authenticate with the server\n");
    printf("  logout                     Log out from the server\n");
    printf("  list PATH                  List directory contents\n");
    printf("  get REMOTE_PATH LOCAL_PATH Download a file\n");
    printf("  put REMOTE_PATH LOCAL_PATH Upload a file\n");
    printf("  delete PATH                Delete a file or directory\n");
    printf("  mkdir PATH                 Create a directory\n");
}

int main(int argc, char *argv[]) {
    int i;
    
    // Parse options
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--host") == 0) {
            if (i + 1 < argc) {
                strncpy(g_host, argv[i + 1], sizeof(g_host) - 1);
                i++;
            }
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0) {
            if (i + 1 < argc) {
                g_port = atoi(argv[i + 1]);
                i++;
            }
        } else if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--user") == 0) {
            if (i + 1 < argc) {
                strncpy(g_username, argv[i + 1], sizeof(g_username) - 1);
                i++;
            }
        } else if (strcmp(argv[i], "-P") == 0 || strcmp(argv[i], "--password") == 0) {
            if (i + 1 < argc) {
                strncpy(g_password, argv[i + 1], sizeof(g_password) - 1);
                i++;
            }
        } else {
            break;  // End of options
        }
    }
    
    // Check if command is provided
    if (i >= argc) {
        print_usage(argv[0]);
        return 1;
    }
    
    // Connect to server
    int sock_fd = connect_to_server(g_host, g_port);
    if (sock_fd < 0) {
        return 1;
    }
    
    // Process command
    const char *command = argv[i++];
    
    if (strcmp(command, "login") == 0) {
        if (i + 1 < argc) {
            client_login(argv[i], argv[i + 1]);
        } else {
            fprintf(stderr, "Error: login command requires USERNAME and PASSWORD\n");
        }
    } else if (strcmp(command, "logout") == 0) {
        client_logout(sock_fd);
        // Clear credentials
        g_username[0] = '\0';
        g_password[0] = '\0';
    } else if (strcmp(command, "list") == 0) {
        if (i < argc) {
            client_list_directory(sock_fd, argv[i]);
        } else {
            client_list_directory(sock_fd, "/");
        }
    } else if (strcmp(command, "get") == 0) {
        if (i + 1 < argc) {
            client_get_file(sock_fd, argv[i], argv[i + 1]);
        } else {
            fprintf(stderr, "Error: get command requires REMOTE_PATH and LOCAL_PATH\n");
        }
    } else if (strcmp(command, "put") == 0) {
        if (i + 1 < argc) {
            client_put_file(sock_fd, argv[i], argv[i + 1]);
        } else {
            fprintf(stderr, "Error: put command requires REMOTE_PATH and LOCAL_PATH\n");
        }
    } else if (strcmp(command, "delete") == 0) {
        if (i < argc) {
            client_delete_file(sock_fd, argv[i]);
        } else {
            fprintf(stderr, "Error: delete command requires PATH\n");
        }
    } else if (strcmp(command, "mkdir") == 0) {
        if (i < argc) {
            client_create_directory(sock_fd, argv[i]);
        } else {
            fprintf(stderr, "Error: mkdir command requires PATH\n");
        }
    } else {
        fprintf(stderr, "Error: unknown command: %s\n", command);
        print_usage(argv[0]);
    }
    
    // Close connection
    close(sock_fd);
    
    return 0;
} 

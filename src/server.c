#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include "../include/server.h"
#include "../include/logger.h"
#include "../include/file_ops.h"
#include "../include/protocol.h"

#define MAX_CLIENTS 100
#define BUFFER_SIZE 4096

static int server_fd = -1;
static struct sockaddr_in server_addr;
static pthread_t client_threads[MAX_CLIENTS];
static int client_fds[MAX_CLIENTS];
static int num_clients = 0;
static pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

static void *client_handler(void *arg);

int init_server(int port, int backlog) {
    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        log_error("Failed to create socket: %s", strerror(errno));
        return -1;
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        log_error("Failed to set socket options: %s", strerror(errno));
        close(server_fd);
        return -1;
    }
    
    // Prepare server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    log_info("Attempting to bind to port %d", port);
    
    // Bind socket to address
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        log_error("Failed to bind socket: %s (errno=%d)", strerror(errno), errno);
        close(server_fd);
        return -1;
    }
    
    // Listen for connections
    if (listen(server_fd, backlog) < 0) {
        log_error("Failed to listen on socket: %s", strerror(errno));
        close(server_fd);
        return -1;
    }
    
    // Set non-blocking mode
    int flags = fcntl(server_fd, F_GETFL, 0);
    if (flags < 0) {
        log_error("Failed to get socket flags: %s", strerror(errno));
        close(server_fd);
        return -1;
    }
    
    if (fcntl(server_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        log_error("Failed to set non-blocking mode: %s", strerror(errno));
        close(server_fd);
        return -1;
    }
    
    // Initialize client tracking
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_fds[i] = -1;
    }
    
    log_info("Server initialized on port %d", port);
    return 0;
}

int server_process(void) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    // Accept new connections
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            log_error("Failed to accept connection: %s", strerror(errno));
            return -1;
        }
    } else {
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        log_info("New connection from %s:%d", client_ip, ntohs(client_addr.sin_port));
        
        // Add client to tracking
        pthread_mutex_lock(&clients_mutex);
        if (num_clients < MAX_CLIENTS) {
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_fds[i] == -1) {
                    client_fds[i] = client_fd;
                    
                    // Create thread to handle client
                    if (pthread_create(&client_threads[i], NULL, client_handler, (void *)(intptr_t)i) != 0) {
                        log_error("Failed to create client thread: %s", strerror(errno));
                        close(client_fd);
                        client_fds[i] = -1;
                    } else {
                        num_clients++;
                        log_info("Client %d connected, total clients: %d", i, num_clients);
                    }
                    break;
                }
            }
        } else {
            log_error("Maximum number of clients reached, connection rejected");
            close(client_fd);
        }
        pthread_mutex_unlock(&clients_mutex);
    }
    
    return 0;
}

int shutdown_server(void) {
    if (server_fd >= 0) {
        // Close server socket
        close(server_fd);
        server_fd = -1;
        
        // Wait for client threads to finish
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_fds[i] >= 0) {
                close(client_fds[i]);
                client_fds[i] = -1;
                pthread_join(client_threads[i], NULL);
            }
        }
        num_clients = 0;
        pthread_mutex_unlock(&clients_mutex);
        
        log_info("Server shutdown complete");
    }
    
    return 0;
}

int handle_client(int client_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    
    // Read client request
    bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
    if (bytes_read <= 0) {
        if (bytes_read < 0) {
            // Don't treat EAGAIN/EWOULDBLOCK as errors in non-blocking mode
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return 0;  // Try again later
            }
            log_error("Error reading from client: %s", strerror(errno));
        }
        return -1;
    }
    
    buffer[bytes_read] = '\0';
    
    // Process request based on protocol
    return process_request(client_fd, buffer, bytes_read);
}

static void *client_handler(void *arg) {
    int index = (intptr_t)arg;
    int client_fd = client_fds[index];
    
    // Handle client until disconnection
    while (client_fd >= 0) {
        if (handle_client(client_fd) != 0) {
            break;
        }
    }
    
    // Clean up client resources
    pthread_mutex_lock(&clients_mutex);
    if (client_fds[index] >= 0) {
        close(client_fds[index]);
        client_fds[index] = -1;
        num_clients--;
        log_info("Client %d disconnected, total clients: %d", index, num_clients);
    }
    pthread_mutex_unlock(&clients_mutex);
    
    return NULL;
} 

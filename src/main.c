#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include "../include/server.h"
#include "../include/config.h"
#include "../include/logger.h"
#include "../include/auth.h"

#define DEFAULT_PORT 9090
#define DEFAULT_BACKLOG 10
#define DEFAULT_CONFIG_PATH "config/cileserver.conf"
#define DEFAULT_AUTH_FILE "config/users.auth"

static volatile int keep_running = 1;

void handle_signal(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        keep_running = 0;
        log_info("Shutdown signal received, stopping server...");
    }
}

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;
    int port_specified = 0;
    int auth_enabled = -1;  // -1 means use config file setting
    char config_path[MAX_PATH_LENGTH] = DEFAULT_CONFIG_PATH;
    char auth_file[MAX_PATH_LENGTH] = DEFAULT_AUTH_FILE;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0) {
            if (i + 1 < argc) {
                port = atoi(argv[i + 1]);
                port_specified = 1;
                i++;
            }
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config") == 0) {
            if (i + 1 < argc) {
                strncpy(config_path, argv[i + 1], MAX_PATH_LENGTH - 1);
                i++;
            }
        } else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--auth") == 0) {
            auth_enabled = 1;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                strncpy(auth_file, argv[i + 1], MAX_PATH_LENGTH - 1);
                i++;
            }
        } else if (strcmp(argv[i], "--no-auth") == 0) {
            auth_enabled = 0;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [OPTIONS]\n", argv[0]);
            printf("Options:\n");
            printf("  -p, --port PORT      Port to listen on (default: from config or %d)\n", DEFAULT_PORT);
            printf("  -c, --config PATH    Path to config file (default: %s)\n", DEFAULT_CONFIG_PATH);
            printf("  -a, --auth [FILE]    Enable authentication (optional auth file path)\n");
            printf("  --no-auth            Disable authentication\n");
            printf("  -h, --help           Display this help message\n");
            return 0;
        } else {
            // If no flag is specified, assume it's the config file path
            strncpy(config_path, argv[i], MAX_PATH_LENGTH - 1);
        }
    }
    
    // Initialize logger
    if (init_logger() != 0) {
        fprintf(stderr, "Failed to initialize logger\n");
        return 1;
    }
    
    // Override CONFIG_FILENAME with the specified path
    if (set_config_path(config_path) != 0) {
        log_error("Failed to set configuration path");
        return 1;
    }
    
    // Load configuration
    if (load_config() != 0) {
        log_error("Failed to load configuration");
        return 1;
    }
    
    // Override config settings if specified in command line
    server_config_t *config = get_config();
    
    if (!port_specified) {
        port = config->port;
        log_info("Using port %d from configuration", port);
    }
    
    // Update auth settings if specified in command line
    if (auth_enabled == 1) {
        config->enable_auth = 1;
        if (auth_file[0] != '\0') {
            strncpy(config->auth_file, auth_file, MAX_PATH_LENGTH - 1);
        }
        log_info("Authentication enabled, using file: %s", config->auth_file);
    } else if (auth_enabled == 0) {
        config->enable_auth = 0;
        log_info("Authentication disabled by command line argument");
    }
    
    // Set up signal handlers
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    // Initialize server
    if (init_server(port, DEFAULT_BACKLOG) != 0) {
        log_error("Failed to initialize server");
        return 1;
    }
    
    log_info("Server started on port %d", port);
    if (config->enable_auth) {
        log_info("Authentication enabled");
    } else {
        log_info("Authentication disabled");
    }
    
    // Main server loop
    while (keep_running) {
        server_process();
        usleep(10000); // 10ms sleep to prevent CPU hogging
    }
    
    // Cleanup
    shutdown_server();
    cleanup_logger();
    
    log_info("Server shutdown complete");
    return 0;
} 

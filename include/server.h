#ifndef SERVER_H
#define SERVER_H

/**
 * Initialize the server with the given port and backlog
 * 
 * @param port Port number to listen on
 * @param backlog Maximum length of the queue of pending connections
 * @return 0 on success, non-zero on failure
 */
int init_server(int port, int backlog);

/**
 * Process incoming connections and handle client requests
 * 
 * @return 0 on success, non-zero on failure
 */
int server_process(void);

/**
 * Shutdown the server and clean up resources
 * 
 * @return 0 on success, non-zero on failure
 */
int shutdown_server(void);

/**
 * Handle a client connection
 * 
 * @param client_fd Client socket file descriptor
 * @return 0 on success, non-zero on failure
 */
int handle_client(int client_fd);

#endif /* SERVER_H */ 

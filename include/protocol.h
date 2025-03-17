#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stddef.h>

// Command codes
#define CMD_LIST    0x01
#define CMD_GET     0x02
#define CMD_PUT     0x03
#define CMD_DELETE  0x04
#define CMD_MKDIR   0x05
#define CMD_INFO    0x06

// Response codes
#define RESP_OK     0x00
#define RESP_ERROR  0x01

/**
 * Process a client request
 * 
 * @param client_fd Client socket file descriptor
 * @param buffer Request buffer
 * @param size Size of the request
 * @return 0 on success, non-zero on failure
 */
int process_request(int client_fd, const char *buffer, size_t size);

/**
 * Send a response to the client
 * 
 * @param client_fd Client socket file descriptor
 * @param status Response status code
 * @param data Response data
 * @param data_size Size of the response data
 * @return 0 on success, non-zero on failure
 */
int send_response(int client_fd, int status, const void *data, size_t data_size);

/**
 * Handle a LIST command
 * 
 * @param client_fd Client socket file descriptor
 * @param path Directory path to list
 * @return 0 on success, non-zero on failure
 */
int handle_list_command(int client_fd, const char *path);

/**
 * Handle a GET command
 * 
 * @param client_fd Client socket file descriptor
 * @param path File path to get
 * @return 0 on success, non-zero on failure
 */
int handle_get_command(int client_fd, const char *path);

/**
 * Handle a PUT command
 * 
 * @param client_fd Client socket file descriptor
 * @param path File path to put
 * @param data File data
 * @param data_size Size of the file data
 * @return 0 on success, non-zero on failure
 */
int handle_put_command(int client_fd, const char *path, const void *data, size_t data_size);

/**
 * Handle a DELETE command
 * 
 * @param client_fd Client socket file descriptor
 * @param path Path to delete
 * @return 0 on success, non-zero on failure
 */
int handle_delete_command(int client_fd, const char *path);

/**
 * Handle a MKDIR command
 * 
 * @param client_fd Client socket file descriptor
 * @param path Directory path to create
 * @return 0 on success, non-zero on failure
 */
int handle_mkdir_command(int client_fd, const char *path);

/**
 * Handle an INFO command
 * 
 * @param client_fd Client socket file descriptor
 * @param path Path to get info for
 * @return 0 on success, non-zero on failure
 */
int handle_info_command(int client_fd, const char *path);

#endif /* PROTOCOL_H */ 

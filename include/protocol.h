#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stddef.h>
#include "auth.h"

// Command codes
#define CMD_LIST    0x01
#define CMD_GET     0x02
#define CMD_PUT     0x03
#define CMD_DELETE  0x04
#define CMD_MKDIR   0x05
#define CMD_INFO    0x06
#define CMD_AUTH    0x07  // New authentication command
#define CMD_LOGOUT  0x08  // New logout command

// Response codes
#define RESP_OK     0x00
#define RESP_ERROR  0x01
#define RESP_AUTH_REQUIRED 0x02  // New response code for authentication required

/**
 * Process a client request
 * 
 * @param client_fd Client socket file descriptor
 * @param buffer Request buffer
 * @param size Size of the request
 * @param user_role User role for permission checking
 * @return 0 on success, non-zero on failure
 */
int process_request(int client_fd, const char *buffer, size_t size, user_role_t *user_role);

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
 * Handle an AUTH command
 * 
 * @param client_fd Client socket file descriptor
 * @param username Username
 * @param password Password
 * @param user_role Pointer to store the user's role if authentication succeeds
 * @return 0 on success, non-zero on failure
 */
int handle_auth_command(int client_fd, const char *username, const char *password, user_role_t *user_role);

/**
 * Handle a LIST command
 * 
 * @param client_fd Client socket file descriptor
 * @param path Directory path to list
 * @param user_role User role for permission checking
 * @return 0 on success, non-zero on failure
 */
int handle_list_command(int client_fd, const char *path, user_role_t user_role);

/**
 * Handle a GET command
 * 
 * @param client_fd Client socket file descriptor
 * @param path File path to get
 * @param user_role User role for permission checking
 * @return 0 on success, non-zero on failure
 */
int handle_get_command(int client_fd, const char *path, user_role_t user_role);

/**
 * Handle a PUT command
 * 
 * @param client_fd Client socket file descriptor
 * @param path File path to put
 * @param data File data
 * @param data_size Size of the file data
 * @param user_role User role for permission checking
 * @return 0 on success, non-zero on failure
 */
int handle_put_command(int client_fd, const char *path, const void *data, size_t data_size, user_role_t user_role);

/**
 * Handle a DELETE command
 * 
 * @param client_fd Client socket file descriptor
 * @param path Path to delete
 * @param user_role User role for permission checking
 * @return 0 on success, non-zero on failure
 */
int handle_delete_command(int client_fd, const char *path, user_role_t user_role);

/**
 * Handle a MKDIR command
 * 
 * @param client_fd Client socket file descriptor
 * @param path Directory path to create
 * @param user_role User role for permission checking
 * @return 0 on success, non-zero on failure
 */
int handle_mkdir_command(int client_fd, const char *path, user_role_t user_role);

/**
 * Handle an INFO command
 * 
 * @param client_fd Client socket file descriptor
 * @param path Path to get info for
 * @param user_role User role for permission checking
 * @return 0 on success, non-zero on failure
 */
int handle_info_command(int client_fd, const char *path, user_role_t user_role);

/**
 * Handle a LOGOUT command
 * 
 * @param client_fd Client socket file descriptor
 * @param user_role Pointer to the user's role (will be reset to ROLE_GUEST)
 * @return 0 on success, non-zero on failure
 */
int handle_logout_command(int client_fd, user_role_t *user_role);

#endif /* PROTOCOL_H */ 

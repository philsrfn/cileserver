#ifndef AUTH_H
#define AUTH_H

#include <stddef.h>

/**
 * User role types
 */
typedef enum {
    ROLE_GUEST = 0,    // Limited read-only access
    ROLE_USER = 1,     // Normal user with read/write access
    ROLE_ADMIN = 2     // Admin with full access
} user_role_t;

/**
 * User data structure
 */
typedef struct {
    char username[64];
    char password_hash[128];
    user_role_t role;
} user_t;

/**
 * Initialize the authentication system
 * 
 * @param auth_file Path to the authentication file
 * @return 0 on success, non-zero on failure
 */
int init_auth(const char *auth_file);

/**
 * Authenticate a user with the given credentials
 * 
 * @param username Username
 * @param password Password (plaintext)
 * @param role Pointer to store the user's role if authentication succeeds
 * @return 0 on success, non-zero on failure
 */
int authenticate_user(const char *username, const char *password, user_role_t *role);

/**
 * Add a new user to the authentication system
 * 
 * @param username Username
 * @param password Password (plaintext)
 * @param role User role
 * @return 0 on success, non-zero on failure
 */
int add_user(const char *username, const char *password, user_role_t role);

/**
 * Remove a user from the authentication system
 * 
 * @param username Username
 * @return 0 on success, non-zero on failure
 */
int remove_user(const char *username);

/**
 * Save the current user database to the auth file
 * 
 * @return 0 on success, non-zero on failure
 */
int save_auth_file(void);

/**
 * Check if a user has permission to perform an operation
 * 
 * @param role User role
 * @param operation Operation code (from protocol.h)
 * @return 1 if allowed, 0 if not allowed
 */
int check_permission(user_role_t role, int operation);

#endif /* AUTH_H */ 

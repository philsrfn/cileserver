#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_FILENAME "config/cileserver.conf"
#define MAX_PATH_LENGTH 1024

typedef struct {
    char root_directory[MAX_PATH_LENGTH];
    int max_connections;
    int port;
    int log_level;
    int enable_auth;
    char auth_file[MAX_PATH_LENGTH];
} server_config_t;

/**
 * Set the path to the configuration file
 * 
 * @param path Path to the configuration file
 * @return 0 on success, non-zero on failure
 */
int set_config_path(const char *path);

/**
 * Load server configuration from file
 * 
 * @return 0 on success, non-zero on failure
 */
int load_config(void);

/**
 * Save current configuration to file
 * 
 * @return 0 on success, non-zero on failure
 */
int save_config(void);

/**
 * Get the current server configuration
 * 
 * @return Pointer to the server configuration
 */
server_config_t *get_config(void);

/**
 * Set a configuration value by name
 * 
 * @param name Configuration parameter name
 * @param value Configuration parameter value
 * @return 0 on success, non-zero on failure
 */
int set_config_value(const char *name, const char *value);

#endif /* CONFIG_H */ 

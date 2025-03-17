#ifndef FILE_OPS_H
#define FILE_OPS_H

#include <stddef.h>

typedef struct {
    char name[256];
    size_t size;
    int is_directory;
    time_t modified_time;
} file_info_t;

/**
 * Initialize the file operations module
 * 
 * @return 0 on success, non-zero on failure
 */
int init_file_ops(void);

/**
 * Clean up file operations resources
 * 
 * @return 0 on success, non-zero on failure
 */
int cleanup_file_ops(void);

/**
 * Read a file from the server's root directory
 * 
 * @param path Relative path to the file
 * @param buffer Buffer to store the file contents
 * @param size Size of the buffer
 * @param bytes_read Pointer to store the number of bytes read
 * @return 0 on success, non-zero on failure
 */
int read_file(const char *path, void *buffer, size_t size, size_t *bytes_read);

/**
 * Write a file to the server's root directory
 * 
 * @param path Relative path to the file
 * @param buffer Buffer containing the data to write
 * @param size Size of the data to write
 * @return 0 on success, non-zero on failure
 */
int write_file(const char *path, const void *buffer, size_t size);

/**
 * Delete a file from the server's root directory
 * 
 * @param path Relative path to the file
 * @return 0 on success, non-zero on failure
 */
int delete_file(const char *path);

/**
 * List files in a directory
 * 
 * @param path Relative path to the directory
 * @param entries Array to store file information
 * @param max_entries Maximum number of entries to retrieve
 * @param num_entries Pointer to store the number of entries retrieved
 * @return 0 on success, non-zero on failure
 */
int list_directory(const char *path, file_info_t *entries, int max_entries, int *num_entries);

/**
 * Create a directory
 * 
 * @param path Relative path to the directory
 * @return 0 on success, non-zero on failure
 */
int create_directory(const char *path);

/**
 * Get information about a file
 * 
 * @param path Relative path to the file
 * @param info Pointer to store file information
 * @return 0 on success, non-zero on failure
 */
int get_file_info(const char *path, file_info_t *info);

/**
 * Check if a path is valid and within the server's root directory
 * 
 * @param path Relative path to check
 * @return 1 if valid, 0 if invalid
 */
int is_path_valid(const char *path);

#endif /* FILE_OPS_H */ 

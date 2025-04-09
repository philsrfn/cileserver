# File Operations

## Overview

The file operations module (`src/file_ops.c`) provides functions for interacting with the file system. It handles operations such as reading, writing, deleting, and listing files and directories.

## API Functions

### File Information

#### `int get_file_info(const char *path, file_info_t *info)`

Gets information about a file or directory.

Parameters:
- `path`: Path to the file or directory
- `info`: Pointer to a file_info_t structure to store the information

Returns:
- 0 on success
- -1 on failure

### Directory Operations

#### `int list_directory(const char *path, file_info_t **entries, int *num_entries)`

Lists the contents of a directory.

Parameters:
- `path`: Path to the directory
- `entries`: Pointer to an array of file_info_t structures (will be allocated by the function)
- `num_entries`: Pointer to an integer to store the number of entries

Returns:
- 0 on success
- -1 on failure

#### `int create_directory(const char *path)`

Creates a new directory.

Parameters:
- `path`: Path to the new directory

Returns:
- 0 on success
- -1 on failure

### File Operations

#### `int read_file(const char *path, void **data, size_t *size)`

Reads the contents of a file.

Parameters:
- `path`: Path to the file
- `data`: Pointer to a buffer to store the file contents (will be allocated by the function)
- `size`: Pointer to a size_t to store the file size

Returns:
- 0 on success
- -1 on failure

#### `int write_file(const char *path, const void *data, size_t size)`

Writes data to a file.

Parameters:
- `path`: Path to the file
- `data`: Pointer to the data to write
- `size`: Size of the data in bytes

Returns:
- 0 on success
- -1 on failure

#### `int delete_file(const char *path)`

Deletes a file or directory.

Parameters:
- `path`: Path to the file or directory

Returns:
- 0 on success
- -1 on failure

### Path Handling

#### `int get_full_path(const char *path, char *full_path, size_t max_len)`

Converts a relative path to an absolute path within the server's root directory.

Parameters:
- `path`: Relative path
- `full_path`: Buffer to store the absolute path
- `max_len`: Size of the buffer

Returns:
- 0 on success
- -1 on failure

## Data Structures

### file_info_t

```c
typedef struct {
    char name[256];
    size_t size;
    time_t modified_time;
    int is_directory;
} file_info_t;
```

- `name`: File or directory name
- `size`: File size in bytes (0 for directories)
- `modified_time`: Last modification time
- `is_directory`: 1 if directory, 0 if file

## Error Handling

File operations functions return 0 on success and -1 on failure. When a failure occurs, the function sets `errno` to indicate the error. Common errors include:

- `ENOENT`: No such file or directory
- `EACCES`: Permission denied
- `EISDIR`: Is a directory
- `ENOTDIR`: Not a directory
- `EIO`: Input/output error

## Implementation Details

The file operations module:
- Uses the configuration system to determine the root directory
- Ensures that all file operations are confined to the root directory
- Handles path normalization and validation
- Uses standard C file I/O functions (fopen, fread, fwrite, etc.)
- Uses POSIX directory functions (opendir, readdir, etc.) 
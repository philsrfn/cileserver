# File Operations

## Overview

The file operations module provides a robust and secure interface for file system interactions. Implemented in `src/file_ops.c`, it offers a comprehensive set of functions for managing files and directories while ensuring security and proper error handling.

## Key Features

- 🔒 **Security**: Path validation and root directory confinement
- 📁 **Comprehensive**: Full suite of file and directory operations
- ⚡ **Efficient**: Optimized for performance
- 🛡️ **Safe**: Proper error handling and resource management
- 🔄 **Thread-safe**: Safe for concurrent access

## API Reference

### File Information

#### `int get_file_info(const char *path, file_info_t *info)`

Retrieves detailed information about a file or directory.

Parameters:
- `path`: Path to the file or directory
- `info`: Pointer to a file_info_t structure to store the information

Returns:
- `0`: Success
- `-1`: Failure (check errno for details)

Example:
```c
file_info_t info;
if (get_file_info("/documents/report.pdf", &info) == 0) {
    printf("File: %s\nSize: %zu bytes\n", info.name, info.size);
}
```

### Directory Operations

#### `int list_directory(const char *path, file_info_t *entries, int max_entries, int *num_entries)`

Lists the contents of a directory.

Parameters:
- `path`: Path to the directory
- `entries`: Array of file_info_t structures
- `max_entries`: Maximum number of entries to return
- `num_entries`: Pointer to store the actual number of entries

Returns:
- `0`: Success
- `-1`: Failure (check errno for details)

Example:
```c
file_info_t entries[100];
int num_entries;
if (list_directory("/documents", entries, 100, &num_entries) == 0) {
    for (int i = 0; i < num_entries; i++) {
        printf("%s\n", entries[i].name);
    }
}
```

#### `int create_directory(const char *path)`

Creates a new directory with appropriate permissions.

Parameters:
- `path`: Path to the new directory

Returns:
- `0`: Success
- `-1`: Failure (check errno for details)

Example:
```c
if (create_directory("/documents/backup") == 0) {
    printf("Directory created successfully\n");
}
```

### File Operations

#### `int read_file(const char *path, void *buffer, size_t buffer_size, size_t *bytes_read)`

Reads the contents of a file into a buffer.

Parameters:
- `path`: Path to the file
- `buffer`: Buffer to store the file contents
- `buffer_size`: Size of the buffer
- `bytes_read`: Pointer to store the number of bytes read

Returns:
- `0`: Success
- `-1`: Failure (check errno for details)

Example:
```c
char buffer[4096];
size_t bytes_read;
if (read_file("/documents/report.txt", buffer, sizeof(buffer), &bytes_read) == 0) {
    printf("Read %zu bytes\n", bytes_read);
}
```

#### `int write_file(const char *path, const void *data, size_t size)`

Writes data to a file, creating it if necessary.

Parameters:
- `path`: Path to the file
- `data`: Data to write
- `size`: Size of the data in bytes

Returns:
- `0`: Success
- `-1`: Failure (check errno for details)

Example:
```c
const char *data = "Hello, world!";
if (write_file("/documents/greeting.txt", data, strlen(data)) == 0) {
    printf("File written successfully\n");
}
```

#### `int delete_file(const char *path)`

Deletes a file or directory.

Parameters:
- `path`: Path to the file or directory

Returns:
- `0`: Success
- `-1`: Failure (check errno for details)

Example:
```c
if (delete_file("/documents/old_file.txt") == 0) {
    printf("File deleted successfully\n");
}
```

### Path Handling

#### `char *get_full_path(const char *relative_path)`

Converts a relative path to an absolute path within the server's root directory.

Parameters:
- `relative_path`: Relative path

Returns:
- Pointer to the full path (statically allocated)
- NULL on error

Example:
```c
const char *full_path = get_full_path("documents/report.txt");
if (full_path != NULL) {
    printf("Full path: %s\n", full_path);
}
```

## Data Structures

### file_info_t

```c
typedef struct {
    char name[256];        // File/directory name (null-terminated)
    size_t size;           // File size in bytes (0 for directories)
    time_t modified_time;  // Last modification timestamp
    int is_directory;      // 1 if directory, 0 if file
} file_info_t;
```

## Error Handling

The module uses standard Unix error codes through `errno`. Common errors include:

- `ENOENT`: File or directory not found
- `EACCES`: Permission denied
- `EISDIR`: Path is a directory
- `ENOTDIR`: Path component is not a directory
- `EIO`: I/O error
- `ENOMEM`: Out of memory
- `ENAMETOOLONG`: Path too long

## Implementation Details

### Security Features

1. **Path Validation**
   - Prevents directory traversal attacks
   - Ensures paths stay within root directory
   - Handles symbolic links safely

2. **Resource Management**
   - Proper file descriptor handling
   - Memory allocation safety
   - Cleanup on errors

3. **Permission Handling**
   - Respects file permissions
   - Creates files with safe defaults
   - Handles umask properly

### Performance Considerations

1. **Buffer Management**
   - Efficient memory usage
   - Appropriate buffer sizes
   - Reuse of static buffers where safe

2. **File Operations**
   - Optimized read/write operations
   - Efficient directory scanning
   - Minimal system calls

3. **Thread Safety**
   - Safe for concurrent access
   - Proper synchronization
   - No global state 
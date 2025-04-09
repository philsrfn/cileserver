# Protocol

## Overview

CileServer implements a custom binary protocol optimized for efficient file operations over TCP/IP. The protocol is designed to be lightweight, fast, and reliable, with built-in support for error handling and extensibility.

## Features

- 🚀 **Efficient**: Binary format for minimal overhead
- 🔒 **Reliable**: Built-in error detection and handling
- 📦 **Compact**: Optimized message structure
- 🔄 **Extensible**: Easy to add new commands
- ⚡ **Fast**: Minimal parsing overhead

## Format

### Request

```
+--------+------------+------------+------+------+
| Command| Path Length| Data Length| Path | Data |
| (1B)   | (2B)       | (4B)       | (var)| (var)|
+--------+------------+------------+------+------+
```

Field Descriptions:
- **Command** (1 byte): Operation identifier
- **Path Length** (2 bytes): Length of path string (network byte order)
- **Data Length** (4 bytes): Length of data payload (network byte order)
- **Path** (variable): UTF-8 encoded path string
- **Data** (variable): Optional payload for operations like PUT

### Response

```
+--------+------------+------+
| Status | Data Length| Data |
| (1B)   | (4B)       | (var)|
+--------+------------+------+
```

Field Descriptions:
- **Status** (1 byte): Operation result code
- **Data Length** (4 bytes): Length of response data (network byte order)
- **Data** (variable): Response payload or error message

## Commands

| Command | Value | Description                   | Request Data                | Response Data               |
|---------|-------|-------------------------------|----------------------------|----------------------------|
| LIST    | 0x01  | List directory contents       | None                       | Array of file_info_t       |
| GET     | 0x02  | Get file contents             | None                       | File contents              |
| PUT     | 0x03  | Upload file                   | File contents              | Success message            |
| DELETE  | 0x04  | Delete file or directory      | None                       | Success message            |
| MKDIR   | 0x05  | Create directory              | None                       | Success message            |
| INFO    | 0x06  | Get file information          | None                       | file_info_t                |

## Status

| Status | Value | Description                   |
|--------|-------|-------------------------------|
| OK     | 0x00  | Operation successful          |
| ERROR  | 0x01  | Operation failed              |

## Data

### file_info_t

```c
typedef struct {
    char name[256];        // File/directory name (null-terminated)
    size_t size;           // File size in bytes (0 for directories)
    time_t modified_time;  // Last modification timestamp
    int is_directory;      // 1 if directory, 0 if file
} file_info_t;
```

## Flow

### Success

1. Client sends request with command and parameters
2. Server processes request
3. Server sends response with status OK and result data
4. Client processes response

### Errors

1. Client sends request
2. Server detects error
3. Server sends response with status ERROR and error message
4. Client handles error appropriately

## Examples

### List

Request:
```c
struct {
    uint8_t command = 0x01;        // LIST
    uint16_t path_len = 0x000B;    // "/documents"
    uint32_t data_len = 0x00000000;
    char path[] = "/documents";
} request;
```

Response:
```c
struct {
    uint8_t status = 0x00;         // OK
    uint32_t data_len = 0x00000100;
    file_info_t entries[] = {
        { "file1.txt", 1024, 1234567890, 0 },
        { "subdir", 0, 1234567890, 1 }
    };
} response;
```

### Error

Response:
```c
struct {
    uint8_t status = 0x01;         // ERROR
    uint32_t data_len = 0x0000000D;
    char message[] = "File not found";
} response;
```

## Best Practices

1. **Errors**
   - Always check response status
   - Handle network errors gracefully
   - Implement retry logic for transient failures

2. **Performance**
   - Use appropriate buffer sizes
   - Implement connection pooling
   - Consider compression for large files

3. **Security**
   - Validate all input data
   - Implement proper authentication
   - Use secure connections (TLS)

4. **Implementation**
   - Use network byte order for multi-byte values
   - Handle partial reads/writes
   - Implement proper timeouts 
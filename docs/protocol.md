# Protocol

## Overview

CileServer uses a custom binary protocol for communication between the client and server. The protocol is designed to be simple, efficient, and extensible.

## Message Format

### Request Format

```
+--------+------------+------------+------+------+
| Command| Path Length| Data Length| Path | Data |
| (1B)   | (2B)       | (4B)       | (var)| (var)|
+--------+------------+------------+------+------+
```

- **Command** (1 byte): The operation to perform
- **Path Length** (2 bytes): Length of the path string in bytes
- **Data Length** (4 bytes): Length of the data in bytes
- **Path** (variable): The file or directory path (not null-terminated)
- **Data** (variable): Optional data for operations like PUT

### Response Format

```
+--------+------------+------+
| Status | Data Length| Data |
| (1B)   | (4B)       | (var)|
+--------+------------+------+
```

- **Status** (1 byte): Result of the operation (0 = success, 1 = error)
- **Data Length** (4 bytes): Length of the data in bytes
- **Data** (variable): Response data or error message

## Commands

| Command | Value | Description                   | Request Data                | Response Data               |
|---------|-------|-------------------------------|----------------------------|----------------------------|
| LIST    | 0x01  | List directory contents       | None                       | Array of file_info_t       |
| GET     | 0x02  | Get file contents             | None                       | File contents              |
| PUT     | 0x03  | Upload file                   | File contents              | Success message            |
| DELETE  | 0x04  | Delete file or directory      | None                       | Success message            |
| MKDIR   | 0x05  | Create directory              | None                       | Success message            |
| INFO    | 0x06  | Get file information          | None                       | file_info_t                |

## Status Codes

| Status | Value | Description                   |
|--------|-------|-------------------------------|
| OK     | 0x00  | Operation successful          |
| ERROR  | 0x01  | Operation failed              |

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

- **name**: File or directory name
- **size**: File size in bytes (0 for directories)
- **modified_time**: Last modification time
- **is_directory**: 1 if directory, 0 if file

## Examples

### List Directory

Request:
- Command: 0x01 (LIST)
- Path: "/documents"
- Data: None

Response:
- Status: 0x00 (OK)
- Data: Array of file_info_t structures

### Get File

Request:
- Command: 0x02 (GET)
- Path: "/documents/file.txt"
- Data: None

Response:
- Status: 0x00 (OK)
- Data: Contents of file.txt

### Put File

Request:
- Command: 0x03 (PUT)
- Path: "/documents/newfile.txt"
- Data: Contents of newfile.txt

Response:
- Status: 0x00 (OK)
- Data: "File written successfully"

### Error Response

Response:
- Status: 0x01 (ERROR)
- Data: Error message (e.g., "File not found") 
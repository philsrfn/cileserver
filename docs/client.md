# CileClient: Command-Line File Management

## Overview

CileClient is a powerful command-line interface for interacting with CileServer. Designed for both interactive use and scripting, it provides a comprehensive set of commands for managing files and directories on the server. Whether you're a system administrator or a developer, CileClient offers a flexible and efficient way to handle remote file operations.

## Quick Start

```bash
# Connect to a remote server
./builddir/cileclient -h myserver.com -p 8080 list /

# Upload a file
./builddir/cileclient put /remote/path/file.txt local_file.txt

# Download a file
./builddir/cileclient get /remote/path/file.txt local_file.txt
```

## Command Reference

### Basic Usage

```
./builddir/cileclient [OPTIONS] COMMAND [ARGS]
```

### Connection Options

| Option           | Description                                | Default Value |
|------------------|--------------------------------------------|---------------|
| -h, --host HOST  | Server hostname                           | localhost     |
| -p, --port PORT  | Server port                               | 8080          |

### Available Commands

#### List Directory Contents

```bash
./builddir/cileclient list [PATH]
```

Lists files and directories at the specified path. If PATH is omitted, lists the root directory.

Examples:
```bash
# List root directory
./builddir/cileclient list /

# List specific directory
./builddir/cileclient list /documents
```

#### Download Files

```bash
./builddir/cileclient get REMOTE_PATH LOCAL_PATH
```

Downloads a file from the server to your local system.

Examples:
```bash
# Download a file
./builddir/cileclient get /documents/report.pdf ~/Downloads/report.pdf

# Download to current directory
./builddir/cileclient get /data/sample.txt .
```

#### Upload Files

```bash
./builddir/cileclient put REMOTE_PATH LOCAL_PATH
```

Uploads a file from your local system to the server.

Examples:
```bash
# Upload a file
./builddir/cileclient put /backup/data.txt ~/Documents/data.txt

# Upload from current directory
./builddir/cileclient put /uploads/image.jpg ./photo.jpg
```

#### Create Directories

```bash
./builddir/cileclient mkdir PATH
```

Creates a new directory on the server.

Examples:
```bash
# Create a directory
./builddir/cileclient mkdir /projects/new_project

# Create nested directories
./builddir/cileclient mkdir /data/2024/04
```

#### Delete Files or Directories

```bash
./builddir/cileclient delete PATH
```

Removes a file or directory from the server.

Examples:
```bash
# Delete a file
./builddir/cileclient delete /temp/old_file.txt

# Delete a directory
./builddir/cileclient delete /backup/old_version
```

## Advanced Usage

### Scripting Examples

```bash
#!/bin/bash

# Upload multiple files
for file in *.txt; do
    ./builddir/cileclient put "/backup/$file" "$file"
done

# Backup directory
./builddir/cileclient mkdir "/backup/$(date +%Y-%m-%d)"
./builddir/cileclient put "/backup/$(date +%Y-%m-%d)/data.tar.gz" "data.tar.gz"
```

### Error Handling

The client provides detailed error messages for various scenarios:

- **Connection Issues**
  - Server unreachable
  - Authentication failures
  - Connection timeouts

- **File Operations**
  - File not found
  - Permission denied
  - Disk space issues
  - Invalid paths

- **Protocol Errors**
  - Invalid responses
  - Timeout errors
  - Protocol version mismatches

All errors are logged with appropriate exit codes for scripting purposes.

## Implementation Details

The client is implemented in `src/client.c` and uses a robust binary protocol for efficient communication. Key features include:

- Connection pooling for better performance
- Automatic retry on transient failures
- Progress reporting for large file transfers
- Thread-safe operation for concurrent use

For protocol details, see the [protocol documentation](protocol.md). 
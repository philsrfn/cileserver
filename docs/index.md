# Documentation

## Introduction

CileServer is a multi-threaded file server implemented in C. It provides basic file operations (read, write, delete, list) and directory operations (create, list) over a custom binary protocol.

## Table of Contents

- [Architecture](architecture.md) - Overview of the server architecture
- [Server](server.md) - Server implementation details
- [Client](client.md) - Client usage and implementation
- [Protocol](protocol.md) - Binary protocol specification
- [File Operations](file_operations.md) - File system operations
- [Configuration](configuration.md) - Configuration system
- [Logging](logging.md) - Logging system

## Getting Started

### Building

To build the server and client:

```bash
./install.sh
```

Or manually:

```bash
meson setup builddir
meson compile -C builddir
```

### Running the Server

```bash
./builddir/cileserver
```

### Using the Client

```bash
# List files
./builddir/cileclient list /

# Upload a file
./builddir/cileclient put /remote_file.txt local_file.txt

# Download a file
./builddir/cileclient get /remote_file.txt local_file.txt

# Create a directory
./builddir/cileclient mkdir /new_directory

# Delete a file
./builddir/cileclient delete /remote_file.txt
```

## Project Structure

```
cileserver/
├── config/              # Configuration files
│   ├── cileserver.conf
│   └── cileserver.conf.example
├── docs/                # Documentation
├── include/             # Header files
├── logs/                # Log files
├── src/                 # Source code
├── test_files/          # Test files
├── tests/               # Test scripts
├── .gitignore
├── install.sh           # Installation script
├── Makefile             # Legacy build system
├── meson.build          # Meson build system
└── README.md
```

## License

This project is licensed under the MIT License - see the LICENSE file for details. 
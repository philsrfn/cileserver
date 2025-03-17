# CileServer - C File Server

A simple and efficient file server implemented in C.

## Features

- Multi-threaded server architecture
- File operations: read, write, delete, list
- Directory operations: create, list
- Configuration system
- Logging system
- Custom binary protocol

## Project Structure

```
cileserver/
├── config/              # Configuration files
│   ├── cileserver.conf
│   └── cileserver.conf.example
├── docs/                # Documentation
│   └── TODO.md
├── include/             # Header files
├── logs/                # Log files
├── src/                 # Source code
├── test_files/          # Test files
├── tests/               # Test scripts
├── .gitignore
├── install.sh           # Installation script
├── Makefile             # Legacy build system
├── meson.build          # Meson build system
└── README.md            # This file
```

## Building

### Using Meson (Recommended)

To build the server and client using Meson:

```bash
# Run the installation script
./install.sh
```

Or manually:

```bash
# Setup build directory
meson setup builddir

# Build the project
meson compile -C builddir
```

This will create the executables in the `builddir` directory.

### Using Make (Legacy)

To build using the legacy Makefile:

```bash
make
```

This will create the executables in the `bin` directory.

## Running

### Server

To run the server:

```bash
./builddir/cileserver config/cileserver.conf
```

### Client

To run the client:

```bash
./builddir/cileclient [COMMAND] [OPTIONS]
```

### Server Options

- `-p, --port PORT`: Port to listen on (default: from config or 8080)
- `-c, --config PATH`: Path to config file (default: config/cileserver.conf)
- `-h, --help`: Display help message

You can also specify the config file path directly as the first argument:
```bash
./builddir/cileserver config/cileserver.conf
```

## Configuration

The server can be configured using the `config/cileserver.conf` file. If this file doesn't exist, default values will be used.

Example configuration:

```
root_directory=/path/to/files
max_connections=100
port=8080
log_level=1
enable_auth=0
auth_file=users.auth
```

### Configuration Parameters

- `root_directory`: Directory to serve files from
- `max_connections`: Maximum number of concurrent connections
- `port`: Port to listen on
- `log_level`: Logging level (0=DEBUG, 1=INFO, 2=WARNING, 3=ERROR)
- `enable_auth`: Enable authentication (0=disabled, 1=enabled)
- `auth_file`: File containing user credentials

## Protocol

The server uses a simple binary protocol for communication:

### Request Format

```
+--------+------------+------------+------+------+
| Command| Path Length| Data Length| Path | Data |
| (1B)   | (2B)       | (4B)       | (var)| (var)|
+--------+------------+------------+------+------+
```

### Response Format

```
+--------+------------+------+
| Status | Data Length| Data |
| (1B)   | (4B)       | (var)|
+--------+------------+------+
```

### Commands

- `0x01`: LIST - List directory contents
- `0x02`: GET - Get file contents
- `0x03`: PUT - Upload file
- `0x04`: DELETE - Delete file or directory
- `0x05`: MKDIR - Create directory
- `0x06`: INFO - Get file information

### Status Codes

- `0x00`: OK - Operation successful
- `0x01`: ERROR - Operation failed

## License

This project is licensed under the MIT License - see the LICENSE file for details. 
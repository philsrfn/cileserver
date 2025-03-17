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
│   ├── architecture.md
│   ├── client.md
│   ├── configuration.md
│   ├── file_operations.md
│   ├── index.md
│   ├── logging.md
│   ├── protocol.md
│   ├── server.md
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
./builddir/cileserver
```

### Client

To run the client:

```bash
./builddir/cileclient [COMMAND] [ARGS]
```

Available commands:
- `list PATH` - List directory contents
- `get REMOTE_PATH LOCAL_PATH` - Download a file
- `put REMOTE_PATH LOCAL_PATH` - Upload a file
- `delete PATH` - Delete a file or directory
- `mkdir PATH` - Create a directory

### Server Options

- `-p, --port PORT`: Port to listen on (default: from config or 9090)
- `-c, --config PATH`: Path to config file (default: config/cileserver.conf)
- `-h, --help`: Display help message

You can also specify the config file path directly as the first argument:
```bash
./builddir/cileserver config/cileserver.conf
```

### Client Options

- `-h, --host HOST`: Server hostname (default: localhost)
- `-p, --port PORT`: Server port (default: 9090)

## Configuration

The server can be configured using the `config/cileserver.conf` file. If this file doesn't exist, default values will be used.

Example configuration:

```
# Directory to serve files from
root_directory=/tmp/cileserver

# Maximum number of concurrent connections
max_connections=100

# Port to listen on
port=9090

# Logging level (0=DEBUG, 1=INFO, 2=WARNING, 3=ERROR)
log_level=1

# Enable authentication (0=disabled, 1=enabled)
enable_auth=0

# File containing user credentials
auth_file=users.auth
```

## Documentation

Comprehensive documentation is available in the `docs` directory:

- [Architecture Overview](docs/architecture.md)
- [Server Implementation](docs/server.md)
- [Client Usage](docs/client.md)
- [Protocol Specification](docs/protocol.md)
- [File Operations](docs/file_operations.md)
- [Configuration System](docs/configuration.md)
- [Logging System](docs/logging.md)
- [Project TODO List](docs/TODO.md)

## Examples

### List files in the root directory
```bash
./builddir/cileclient list /
```

### Upload a file
```bash
./builddir/cileclient put /remote_file.txt local_file.txt
```

### Download a file
```bash
./builddir/cileclient get /remote_file.txt downloaded_file.txt
```

### Create a directory
```bash
./builddir/cileclient mkdir /new_directory
```

### Delete a file
```bash
./builddir/cileclient delete /remote_file.txt
```

## License

This project is licensed under the MIT License - see the LICENSE file for details. 
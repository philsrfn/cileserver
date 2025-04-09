# CileServer

## Intro

CileServer is a high-performance, multi-threaded file server written in C. Designed with efficiency and reliability in mind, it provides a robust platform for file operations over a custom binary protocol. Whether you need to share files across a network or build a distributed file system, CileServer offers the perfect foundation.

## Features

- 🚀 **High Performance**: Multi-threaded architecture for optimal throughput
- 🔒 **Security**: Optional authentication and secure file operations
- 📝 **Logging**: Comprehensive logging system with configurable levels
- ⚙️ **Configurable**: Extensive configuration options for fine-tuning
- 📁 **File Operations**: Full suite of file and directory operations
- 🔄 **Binary Protocol**: Efficient, custom protocol for fast transfers

## Docs

- [Architecture](architecture.md) - System design and components
- [Server](server.md) - Server implementation and operation
- [Client](client.md) - Client usage and features
- [Protocol](protocol.md) - Binary protocol specification
- [File Operations](file_operations.md) - File system operations
- [Configuration](configuration.md) - Configuration system
- [Logging](logging.md) - Logging system

## Quick Start

### Prerequisites

- C compiler (GCC or Clang)
- Meson build system
- POSIX-compliant operating system

### Build

```bash
# Quick install
./install.sh

# Manual build
meson setup builddir
meson compile -C builddir
```

### Run Server

```bash
# Start with default configuration
./builddir/cileserver

# Start on a specific port
./builddir/cileserver -p 8080

# Use custom configuration
./builddir/cileserver -c /path/to/config.conf
```

### Use Client

```bash
# List directory contents
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

## Structure

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

## Contributing

We welcome contributions! Please see our [Contributing Guidelines](CONTRIBUTING.md) for details.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details. 
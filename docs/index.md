# CileServer

## About

CileServer is a high-performance, multi-threaded file server written in C. It provides a robust platform for file operations over a custom binary protocol, perfect for file sharing and distributed file systems.

## Features

- 🚀 **High Performance**: Multi-threaded architecture
- 🔒 **Security**: Optional authentication
- 📝 **Logging**: Comprehensive logging system
- ⚙️ **Configurable**: Extensive configuration options
- 📁 **File Operations**: Full suite of operations
- 🔄 **Binary Protocol**: Efficient transfers

## Install

### Prerequisites

- C compiler (GCC or Clang)
- Meson build system
- POSIX-compliant OS

### Quick Install

```bash
./install.sh
```

### Manual Build

```bash
# Setup build
meson setup builddir

# Build
meson compile -C builddir
```

## Start

### Server

```bash
# Default config
./builddir/cileserver

# Custom port
./builddir/cileserver -p 8080

# Custom config
./builddir/cileserver -c /path/to/config.conf
```

### Client

```bash
# List files
./builddir/cileclient list /

# Upload
./builddir/cileclient put /remote.txt local.txt

# Download
./builddir/cileclient get /remote.txt local.txt
```

## Docs

- [Server](server.md) - Server details and configuration
- [Client](client.md) - Client usage and commands
- [Protocol](protocol.md) - Binary protocol specification
- [File Operations](file_operations.md) - File system operations
- [Configuration](configuration.md) - Configuration options
- [Logging](logging.md) - Logging system
- [Architecture](architecture.md) - System design

## Structure

```
cileserver/
├── config/              # Configuration files
├── docs/                # Documentation
├── include/             # Header files
├── logs/                # Log files
├── src/                 # Source code
├── test_files/          # Test files
├── tests/               # Test scripts
├── install.sh           # Installation script
├── meson.build          # Build system
└── README.md
```

## Contributing

We welcome contributions! See [Contributing Guidelines](CONTRIBUTING.md) for details.

## License

MIT License - see [LICENSE](LICENSE) for details. 
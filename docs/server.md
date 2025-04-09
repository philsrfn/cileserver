# CileServer: A High-Performance File Server

## Overview

CileServer is a robust, multi-threaded file server designed for efficient file operations over a network. Built with performance and reliability in mind, it handles client connections, processes requests according to a well-defined protocol, and performs file operations with proper error handling and logging.

## Quick Start

```bash
# Start the server with default configuration
./builddir/cileserver

# Start on a specific port
./builddir/cileserver -p 8080

# Use a custom configuration file
./builddir/cileserver -c /path/to/config.conf
```

## Command-Line Options

| Option              | Description                                | Default Value |
|---------------------|--------------------------------------------|---------------|
| -p, --port PORT     | Port to listen on                          | 8080          |
| -c, --config PATH   | Path to config file                        | config/cileserver.conf |
| -h, --help          | Display help message                       | -             |

You can also specify the config file path directly as the first argument:
```bash
./builddir/cileserver config/cileserver.conf
```

## Architecture

### Server Lifecycle

1. **Initialization**
   - Parse command-line arguments
   - Initialize logger
   - Load configuration
   - Set up signal handlers
   - Initialize server (create socket, bind, listen)

2. **Main Loop**
   - Accept incoming connections
   - Create a new thread for each client
   - Process client requests
   - Respond to clients

3. **Shutdown**
   - Handle SIGINT/SIGTERM signals
   - Close all client connections
   - Close server socket
   - Clean up resources

### Threading Model

The server employs a thread-per-connection model for optimal performance:
- Main thread: Listens for new connections
- Worker threads: Handle individual client connections
- Synchronization: Thread-safe logging and configuration access

### Signal Handling

The server gracefully handles the following signals:
- SIGINT (Ctrl+C): Initiates a clean shutdown
- SIGTERM: Triggers graceful termination

## Implementation Details

### Core Components

1. **Main (`src/main.c`)**
   - Entry point and lifecycle management
   - Command-line argument parsing
   - Signal handling
   - Server initialization and shutdown

2. **Server (`src/server.c`)**
   - Socket management
   - Connection handling
   - Thread management
   - Request processing

3. **Client Handling**
   - Request parsing
   - Operation execution
   - Response generation
   - Connection cleanup

### Error Handling

The server implements comprehensive error handling:
- Socket operations (bind, listen, accept)
- Thread management
- File operations
- Protocol violations
- Resource exhaustion

All errors are logged with appropriate severity levels.

### Logging System

The server features a robust logging system:
- Log file: `logs/cileserver.log`
- Configurable log levels
- Thread-safe logging
- Timestamp and severity information

## Performance Considerations

1. **Connection Management**
   - Maximum concurrent connections configurable
   - Efficient thread pool management
   - Connection timeouts

2. **Resource Usage**
   - Memory-efficient buffer management
   - File descriptor limits
   - Thread stack size optimization

3. **Security**
   - Optional authentication
   - Path traversal prevention
   - File permission enforcement

## Configuration

The server's behavior can be customized through the configuration file. See [configuration documentation](configuration.md) for detailed information. 
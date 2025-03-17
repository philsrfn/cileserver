# CileServer Server

## Overview

The CileServer server (`cileserver`) is a multi-threaded file server that allows clients to perform file operations over a network. It handles client connections, processes requests according to the protocol, and performs file operations.

## Usage

```
./builddir/cileserver [OPTIONS]
```

## Options

| Option              | Description                                | Default Value |
|---------------------|--------------------------------------------|---------------|
| -p, --port PORT     | Port to listen on                          | 9090          |
| -c, --config PATH   | Path to config file                        | config/cileserver.conf |
| -h, --help          | Display help message                       | -             |

You can also specify the config file path directly as the first argument:
```
./builddir/cileserver config/cileserver.conf
```

## Server Lifecycle

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

## Threading Model

The server uses a thread-per-connection model:
- The main thread listens for new connections
- Each client connection is handled in a separate thread
- Thread synchronization is used for shared resources (logging, configuration)

## Signal Handling

The server handles the following signals:
- SIGINT (Ctrl+C): Graceful shutdown
- SIGTERM: Graceful shutdown

## Implementation Details

### Main (`src/main.c`)

The main entry point for the server. It:
- Parses command-line arguments
- Initializes the logger
- Loads the configuration
- Sets up signal handlers
- Initializes the server
- Runs the main server loop
- Handles shutdown

### Server (`src/server.c`)

Manages the server socket and client connections. It:
- Creates the server socket
- Binds to the specified port
- Listens for connections
- Accepts client connections
- Creates threads to handle clients
- Processes client requests
- Cleans up resources on shutdown

### Client Handling

Each client connection is handled in a separate thread. The client handler:
- Reads the request from the client
- Parses the request according to the protocol
- Performs the requested operation
- Sends the response back to the client
- Closes the connection when done

## Error Handling

The server handles various error conditions:
- Socket errors (e.g., bind, listen, accept)
- Thread creation errors
- File operation errors
- Protocol errors

Errors are logged to the log file and, if severe, may cause the server to shut down.

## Logging

The server logs events to the log file (`logs/cileserver.log`). The log level can be configured in the configuration file.

## Configuration

The server can be configured using the configuration file. See [configuration documentation](configuration.md) for details. 
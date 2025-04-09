# Architecture

## Overview

CileServer is a multi-threaded file server implemented in C. It provides basic file operations (read, write, delete, list) and directory operations (create, list) over a custom binary protocol.

## Components

The server is organized into several modular components:

1. **Main** (`src/main.c`): Entry point that handles command-line arguments, initializes components, and runs the main server loop.

2. **Server** (`src/server.c`): Manages socket connections, client handling, and the server lifecycle.

3. **Protocol** (`src/protocol.c`): Implements the binary protocol for client-server communication.

4. **File Operations** (`src/file_ops.c`): Handles file system operations (read, write, delete, list).

5. **Configuration** (`src/config.c`): Manages server configuration from the config file.

6. **Logger** (`src/logger.c`): Provides logging functionality.

7. **Client** (`src/client.c`): A command-line client for interacting with the server.

## Data Flow

1. The server listens for incoming connections on the configured port.
2. When a client connects, a new thread is created to handle the client.
3. The client sends requests using the binary protocol.
4. The server processes the request and performs the requested file operation.
5. The server sends a response back to the client.
6. The client displays the result to the user.

## Threading Model

The server uses a thread-per-connection model:
- The main thread listens for new connections
- Each client connection is handled in a separate thread
- Thread synchronization is used for shared resources (logging, configuration)

## Directory Structure

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
# Logging System

## Overview

The logging system provides a robust and flexible way to track server activity, debug issues, and monitor system health. Implemented in `src/logger.c`, it offers configurable log levels, thread-safe operation, and both file and console output.

## Key Features

- 📝 **Multiple Outputs**: File and console logging
- 🔒 **Thread-Safe**: Safe for concurrent access
- ⚡ **Efficient**: Minimal performance impact
- 📊 **Configurable**: Adjustable log levels
- 🕒 **Timestamped**: Precise timing information

## Log Levels

The system supports four log levels, each serving a specific purpose:

| Level   | Value | Description                                      | Typical Use Case |
|---------|-------|--------------------------------------------------|------------------|
| DEBUG   | 0     | Detailed debugging information                   | Development      |
| INFO    | 1     | General operational information                  | Normal operation |
| WARNING | 2     | Potential issues that don't prevent operation    | Monitoring       |
| ERROR   | 3     | Critical errors that may affect operation        | Troubleshooting  |

## API Reference

### Initialization and Cleanup

#### `int init_logger(void)`

Initializes the logging system. Creates the log file and sets up necessary resources.

Returns:
- `0`: Success
- `-1`: Failure (unable to create/open log file)

Example:
```c
if (init_logger() != 0) {
    fprintf(stderr, "Failed to initialize logger\n");
    return 1;
}
```

#### `int cleanup_logger(void)`

Cleans up logging resources and closes the log file.

Returns:
- `0`: Success
- `-1`: Failure

Example:
```c
cleanup_logger();
```

### Log Level Control

#### `void set_log_level(log_level_t level)`

Sets the current log level. Messages below this level will be ignored.

Parameters:
- `level`: The new log level (DEBUG, INFO, WARNING, or ERROR)

Example:
```c
set_log_level(LOG_INFO);  // Show INFO and above
```

### Logging Functions

#### `void log_debug(const char *format, ...)`

Logs a debug message. Only shown when log level is DEBUG.

Parameters:
- `format`: Printf-style format string
- `...`: Format arguments

Example:
```c
log_debug("Processing request from %s", client_ip);
```

#### `void log_info(const char *format, ...)`

Logs an informational message. Shown for INFO and above.

Parameters:
- `format`: Printf-style format string
- `...`: Format arguments

Example:
```c
log_info("Server started on port %d", port);
```

#### `void log_warning(const char *format, ...)`

Logs a warning message. Shown for WARNING and above.

Parameters:
- `format`: Printf-style format string
- `...`: Format arguments

Example:
```c
log_warning("Disk space below 10%%");
```

#### `void log_error(const char *format, ...)`

Logs an error message. Always shown.

Parameters:
- `format`: Printf-style format string
- `...`: Format arguments

Example:
```c
log_error("Failed to open file: %s", strerror(errno));
```

## Log Format

Log messages follow a consistent format:

```
[TIMESTAMP] [LEVEL] MESSAGE
```

Example:
```
[2024-04-09 14:30:45] [INFO] Server started on port 8080
[2024-04-09 14:31:02] [WARNING] Client connection timeout
[2024-04-09 14:31:15] [ERROR] Failed to write file: Permission denied
```

## Configuration

The logging system can be configured through:

1. **Configuration File**
   - Set log level
   - Configure log file path
   - Enable/disable console output

2. **Runtime**
   - Change log level dynamically
   - Enable/disable specific log types

## Implementation Details

### Thread Safety

The logging system ensures thread safety through:
- Mutex protection for file access
- Atomic operations for log level changes
- Thread-local storage for formatting

### Performance Optimization

1. **Message Buffering**
   - Batch writes to reduce I/O
   - Efficient string formatting
   - Minimal locking duration

2. **Resource Management**
   - Automatic file rotation
   - Memory-efficient buffer usage
   - Proper cleanup on shutdown

3. **Error Handling**
   - Graceful degradation
   - Fallback to stderr
   - Automatic recovery

## Best Practices

1. **Log Level Selection**
   - Use DEBUG for development
   - Use INFO for normal operation
   - Use WARNING for potential issues
   - Use ERROR for critical problems

2. **Message Content**
   - Be specific and descriptive
   - Include relevant context
   - Use consistent formatting
   - Avoid sensitive information

3. **Performance**
   - Minimize debug logging in production
   - Use appropriate log levels
   - Consider log rotation
   - Monitor log file size 
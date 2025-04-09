# Logging

## Overview

The logging module (`src/logger.c`) provides functions for logging messages at different severity levels. It writes log messages to a log file and optionally to stderr for warnings and errors.

## Log Levels

The logging system supports four log levels:

| Level   | Value | Description                                      |
|---------|-------|--------------------------------------------------|
| DEBUG   | 0     | Detailed information for debugging               |
| INFO    | 1     | General information about normal operation       |
| WARNING | 2     | Potential issues that don't prevent operation    |
| ERROR   | 3     | Errors that may prevent proper operation         |

The current log level can be configured in the configuration file. Messages with a level lower than the current log level are not logged.

## API Functions

### Initialization and Cleanup

#### `int init_logger(void)`

Initializes the logging system.

Returns:
- 0 on success
- -1 on failure

#### `int cleanup_logger(void)`

Cleans up the logging system.

Returns:
- 0 on success
- -1 on failure

### Log Level Control

#### `void set_log_level(log_level_t level)`

Sets the current log level.

Parameters:
- `level`: The new log level

### Logging Functions

#### `void log_debug(const char *format, ...)`

Logs a debug message.

Parameters:
- `format`: Printf-style format string
- `...`: Format arguments

#### `void log_info(const char *format, ...)`

Logs an info message.

Parameters:
- `format`: Printf-style format string
- `...`: Format arguments

#### `void log_warning(const char *format, ...)`

Logs a warning message.

Parameters:
- `format`: Printf-style format string
- `...`: Format arguments

#### `void log_error(const char *format, ...)`

Logs an error message.

Parameters:
- `format`: Printf-style format string
- `...`: Format arguments

## Log Format

Log messages are formatted as follows:

```
[TIMESTAMP] [LEVEL] MESSAGE
```

Example:
```
[2023-03-17 10:49:40] [INFO] Server started on port 9090
```

## Log File

By default, log messages are written to `logs/cileserver.log`. The log file is opened in append mode, so logs are preserved across server restarts.

## Thread Safety

The logging system is thread-safe. It uses a mutex to synchronize access to the log file.

## Implementation Details

The logging module:
- Uses a mutex to ensure thread safety
- Formats log messages with timestamps
- Writes to a log file
- Writes warnings and errors to stderr
- Supports variable arguments using va_list
- Automatically initializes if not already initialized 
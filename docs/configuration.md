# Configuration

## Overview

CileServer is a highly configurable file server that uses a simple yet powerful configuration system. The default configuration file is located at `config/cileserver.conf`, but you can specify a custom path using command-line arguments.

## Configuration File Format

The configuration file uses a straightforward key-value format:

```
key=value
```

Lines starting with `#` are treated as comments and ignored.

## Configuration Parameters

| Parameter       | Description                                      | Default Value    |
|-----------------|--------------------------------------------------|------------------|
| root_directory  | Directory to serve files from                    | Current directory|
| max_connections | Maximum number of concurrent connections         | 100              |
| port            | Port to listen on                                | 8080             |
| log_level       | Logging level (0=DEBUG, 1=INFO, 2=WARNING, 3=ERROR) | 1 (INFO)     |
| enable_auth     | Enable authentication (0=disabled, 1=enabled)    | 0 (disabled)     |
| auth_file       | File containing user credentials                 | users.auth       |

## Example Configuration

```
# CileServer Configuration File
# This file controls the behavior of your CileServer instance

# Directory to serve files from
root_directory=/tmp/cileserver

# Maximum number of concurrent connections
max_connections=100

# Port to listen on
port=8080

# Logging level (0=DEBUG, 1=INFO, 2=WARNING, 3=ERROR)
log_level=1

# Enable authentication (0=disabled, 1=enabled)
enable_auth=0

# File containing user credentials
auth_file=users.auth
```

## Command-Line Overrides

The following configuration parameters can be overridden via command-line arguments:

| Command-Line Option | Configuration Parameter | Description |
|---------------------|-------------------------|-------------|
| -p, --port PORT     | port                    | Override the port number |
| -c, --config PATH   | (path to config file)   | Specify a custom config file path |

## Configuration API

The configuration system provides a robust API for programmatic access:

### `int load_config(void)`

Loads the configuration from the configuration file. Returns 0 on success, non-zero on failure.

### `int save_config(void)`

Saves the current configuration to the configuration file. Returns 0 on success, non-zero on failure.

### `server_config_t *get_config(void)`

Returns a pointer to the current server configuration structure.

### `int set_config_value(const char *name, const char *value)`

Sets a configuration value by name. Returns 0 on success, non-zero on failure.

### `int set_config_path(const char *path)`

Sets the path to the configuration file. Returns 0 on success, non-zero on failure.

## Best Practices

1. Always use absolute paths for `root_directory` to avoid confusion
2. Set appropriate file permissions for the `auth_file` when authentication is enabled
3. Consider using a higher log level in production environments
4. Keep the configuration file secure as it may contain sensitive information 
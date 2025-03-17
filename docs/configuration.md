# CileServer Configuration

## Overview

CileServer uses a configuration file to customize its behavior. The default configuration file is located at `config/cileserver.conf`.

## Configuration File Format

The configuration file uses a simple key-value format:

```
key=value
```

Lines starting with `#` are treated as comments.

## Configuration Parameters

| Parameter       | Description                                      | Default Value    |
|-----------------|--------------------------------------------------|------------------|
| root_directory  | Directory to serve files from                    | Current directory|
| max_connections | Maximum number of concurrent connections         | 100              |
| port            | Port to listen on                                | 9090             |
| log_level       | Logging level (0=DEBUG, 1=INFO, 2=WARNING, 3=ERROR) | 1 (INFO)     |
| enable_auth     | Enable authentication (0=disabled, 1=enabled)    | 0 (disabled)     |
| auth_file       | File containing user credentials                 | users.auth       |

## Example Configuration

```
# CileServer Configuration File

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

## Command-Line Overrides

The following configuration parameters can be overridden via command-line arguments:

| Command-Line Option | Configuration Parameter |
|---------------------|-------------------------|
| -p, --port PORT     | port                    |
| -c, --config PATH   | (path to config file)   |

## Configuration API

The configuration system provides the following functions:

### `int load_config(void)`

Loads the configuration from the configuration file.

### `int save_config(void)`

Saves the current configuration to the configuration file.

### `server_config_t *get_config(void)`

Returns a pointer to the current server configuration.

### `int set_config_value(const char *name, const char *value)`

Sets a configuration value by name.

### `int set_config_path(const char *path)`

Sets the path to the configuration file. 
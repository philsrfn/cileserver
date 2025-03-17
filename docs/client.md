# CileServer Client

## Overview

The CileServer client (`cileclient`) is a command-line tool for interacting with the CileServer. It provides commands for listing directories, uploading and downloading files, creating directories, and deleting files.

## Usage

```
./builddir/cileclient [OPTIONS] COMMAND [ARGS]
```

## Options

| Option           | Description                                | Default Value |
|------------------|--------------------------------------------|---------------|
| -h, --host HOST  | Server hostname                           | localhost     |
| -p, --port PORT  | Server port                               | 9090          |

## Commands

### List Directory

Lists the contents of a directory on the server.

```
./builddir/cileclient list [PATH]
```

If PATH is not specified, it defaults to the root directory (`/`).

Example:
```
./builddir/cileclient list /
./builddir/cileclient list /documents
```

### Download File

Downloads a file from the server to the local file system.

```
./builddir/cileclient get REMOTE_PATH LOCAL_PATH
```

Example:
```
./builddir/cileclient get /documents/file.txt ~/Downloads/file.txt
```

### Upload File

Uploads a file from the local file system to the server.

```
./builddir/cileclient put REMOTE_PATH LOCAL_PATH
```

Example:
```
./builddir/cileclient put /documents/newfile.txt ~/Documents/myfile.txt
```

### Delete File or Directory

Deletes a file or directory on the server.

```
./builddir/cileclient delete PATH
```

Example:
```
./builddir/cileclient delete /documents/oldfile.txt
```

### Create Directory

Creates a new directory on the server.

```
./builddir/cileclient mkdir PATH
```

Example:
```
./builddir/cileclient mkdir /documents/newdir
```

## Examples

### List the root directory
```
./builddir/cileclient list /
```

### Upload a file
```
./builddir/cileclient put /test.txt test_files/test.txt
```

### Download a file
```
./builddir/cileclient get /test.txt test_files/downloaded.txt
```

### Create a directory
```
./builddir/cileclient mkdir /test_dir
```

### List a directory
```
./builddir/cileclient list /test_dir
```

### Delete a file
```
./builddir/cileclient delete /test.txt
```

## Error Handling

If an error occurs, the client will display an error message and exit with a non-zero status code. Error messages may include:

- Connection errors (e.g., "Error connecting to server")
- File errors (e.g., "Error opening local file")
- Server errors (e.g., "Server returned error: File not found")

## Implementation Details

The client is implemented in `src/client.c` and uses the same protocol as the server. It communicates with the server using TCP sockets and follows the protocol described in the [protocol documentation](protocol.md). 
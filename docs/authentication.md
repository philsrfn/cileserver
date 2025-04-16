# Authentication System

CileServer now supports user authentication, allowing you to restrict access to files and operations based on user roles.

## Configuration

Authentication can be enabled in the `cileserver.conf` file:

```
# Authentication settings
enable_auth=1
auth_file=config/users.auth
```

Or via command line arguments:

```
./cileserver --auth [path/to/auth/file]
```

To disable authentication:

```
./cileserver --no-auth
```

## User Roles

The authentication system supports three user roles:

1. **Guest (Role 0)** - Limited read-only access
   - Can list directories
   - Can read files
   - Can get file information

2. **User (Role 1)** - Normal user with read/write access
   - All guest permissions
   - Can write files
   - Can delete files
   - Can create directories

3. **Admin (Role 2)** - Administrator with full access
   - All user permissions
   - Plus future administrative operations

## Authentication File Format

The authentication file uses a simple text format:

```
# Format: username:password_hash:role
admin:9d8a1f36cdcb0e1dca728a30153dbabd:2
user:b8e5bf4f32daedc4509c539d53c4fd97:1
guest:2c4f09fe5a64b67cbfa1b62e287a7f0c:0
```

When a new authentication file is created, a default admin user is added:
- Username: `admin`
- Password: `admin`
- Role: Admin (2)

**Important:** For security reasons, you should change the default admin password after installation.

## Protocol

Authentication has been added to the protocol with two new commands:

1. **AUTH (0x07)** - Authenticate a user
   - Sends username and password
   - Receives role on success

2. **LOGOUT (0x08)** - Log out the current user
   - Resets user role to guest

A new response code has also been added:

- **AUTH_REQUIRED (0x02)** - Indicates that authentication is required

## Usage Flow

1. Client connects to server
2. Server assigns guest role by default
3. If authentication is enabled, operations will return AUTH_REQUIRED
4. Client sends AUTH command with credentials
5. If successful, client can perform operations according to role
6. Client can LOGOUT to return to guest role

## Security Considerations

The current implementation uses a simple hash function for passwords. In a production environment, you should replace this with a more secure hashing algorithm like bcrypt or Argon2.

The authentication file should have appropriate permissions to prevent unauthorized access. 
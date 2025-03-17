project('cileserver', 'c',
  version : '0.1.0',
  default_options : ['warning_level=3', 'c_std=c11'])

# Dependencies
threads_dep = dependency('threads')

# Include directories
inc_dir = include_directories('include')

# Server executable
server_sources = [
  'src/main.c',
  'src/server.c',
  'src/file_ops.c',
  'src/protocol.c',
  'src/config.c',
  'src/logger.c'
]

server = executable('cileserver',
  server_sources,
  include_directories : inc_dir,
  dependencies : threads_dep,
  install : true)

# Client executable
client_sources = [
  'src/client.c',
  'src/protocol.c',
  'src/logger.c',
  'src/file_ops.c',
  'src/config.c'
]

client = executable('cileclient',
  client_sources,
  include_directories : inc_dir,
  dependencies : threads_dep,
  install : true) 
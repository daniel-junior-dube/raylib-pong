project('raylib-pong', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

project_dependencies = [
	dependency('raylib', version : '3.0.0')
]

exe = executable('raylib_pong', 'raylib_pong.c',
  install : true,
  dependencies : project_dependencies,
  include_directories : include_directories('include'))

test('basic', exe)

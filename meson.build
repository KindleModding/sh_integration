project('sh_integration', 'c', version : '1.0.0', default_options: ['cpp_std=c++17'])

# Fetch our dependencies
cjson_dep = subproject('cjson', default_options: ['default_library=static']).get_variable('libcjson_dep')
lipc_dep = dependency('lipc')

subdir('./stubs')
subdir('./launcher')
subdir('./extractor')

##sources = files(
##    './src/sh_runner.cpp'
##)
##include_dir = include_directories(
##    './src/includes'
##)
##
##executable('sh_runner', sources, include_directories: include_dir, dependencies: [ lipc_stub_dep ##])
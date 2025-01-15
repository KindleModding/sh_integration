project('sh_integration', 'c', 'cpp', version : '1.0.0', default_options: ['cpp_std=c++17'])

# Fetch our dependencies
cjson_dep = dependency('cjson')
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
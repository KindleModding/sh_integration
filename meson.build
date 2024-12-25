project('sh_integration', 'c', version : '1.0.0')#, default_options: ['cpp_std=c++17' ])

cjson_dep = dependency('libcjson')

subdir('./stubs')
subdir('./launcher')
subdir('./parser')

##sources = files(
##    './src/sh_runner.cpp'
##)
##include_dir = include_directories(
##    './src/includes'
##)
##
##executable('sh_runner', sources, include_directories: include_dir, dependencies: [ lipc_stub_dep ##])
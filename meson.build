project('sh_integration', 'c', 'cpp', version : '1.0.0', default_options: ['default_library=static', 'c++_std=17'])

cmake = import('cmake')

opt_var = cmake.subproject_options()
opt_var.add_cmake_defines({'ENABLE_CJSON_VERSION_SO': false})
opt_var.set_override_option('BUILD_SHARED_LIBS', 'on')
opt_var.set_override_option('BUILD_STATIC_LIBS', 'off')

# Configure the CMake project
cjson = cmake.subproject('cjson')

# Fetch the dependency object
cjson_dep = cjson.dependency('cjson')

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
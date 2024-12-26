sources = files(
    './main.cpp'
)
#include_dir = include_directories(
#    './src/includes'
#)

executable('sh_integration_launcher', sources, dependencies: [ lipc_stub_dep ], cpp_args: ['-static-libstdc++'], link_args: ['-static-libstdc++'])
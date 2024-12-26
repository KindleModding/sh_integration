sources = files(
    './main.cpp'
)
#include_dir = include_directories(
#    './src/includes'
#)

library('sh_integration_parser', sources, include_directories: './', dependencies: [ cjson_dep, scanner_stub_dep, libxml2_stub_dep ], name_prefix: '', cpp_args: ['-static-libstdc++'], link_args: ['-static-libstdc++'])
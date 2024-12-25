sources = files(
    './main.c',
    './utils.c',
    './simple_sh_parser.c'
)
#include_dir = include_directories(
#    './src/includes'
#)

library('sh_integration_parser', sources, include_directories: './', dependencies: [ cjson_dep, scanner_stub_dep, libxml2_stub_dep ], name_prefix: '')
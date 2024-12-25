sources = files(
    './main.c',
    './utils.c',
    './simple_sh_extractor.c'
)
#include_dir = include_directories(
#    './src/includes'
#)

library('sh_integration_extractor', sources, include_directories: './', dependencies: [ lipc_stub_dep, scanner_stub_dep, libxml2_stub_dep ], name_prefix: '')
sources = files(
    './main.cpp'
)

library('sh_integration_extractor', sources, include_directories: './', dependencies: [ cjson_dep, scanner_stub_dep ], name_prefix: '', cpp_args: ['-static-libstdc++'], link_args: ['-static-libstdc++'])
scanner_stub = library(
    'scanner',
    'scanner/scanner.cpp',
    include_directories: ['scanner/'],
    name_prefix: 'lib',
    dependencies: [cjson_dep]
)

scanner_stub_dep = declare_dependency(
    link_with: scanner_stub,
    include_directories: ['scanner/'],
    dependencies: [cjson_dep]
)
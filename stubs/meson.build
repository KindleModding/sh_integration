scanner_stub = library(
    'scanner',
    'scanner/scanner.c',
    include_directories: ['scanner/'],
    name_prefix: 'lib',
    dependencies: [cjson_dep]
)

scanner_stub_dep = declare_dependency(
    link_with: scanner_stub,
    include_directories: ['scanner/'],
    dependencies: [cjson_dep]
)

lipc_stub = library(
    'lipc',
    'lipc/lipc.c',
    include_directories: ['lipc/'],
    name_prefix: 'lib'
)

lipc_stub_dep = declare_dependency(
    link_with: lipc_stub,
    include_directories: ['lipc/']
)
lipc_stub = library(
    'lipc',
    'lipc/lipc.cpp',
    include_directories: ['../openlipc/include/'],
    name_prefix: 'lib'
)

lipc_stub_dep = declare_dependency(
    link_with: lipc_stub,
    include_directories: ['../openlipc/include/']
)

libxml2_stub = library(
    'libxml2',
    'libxml2/libxml2.cpp',
    include_directories: ['libxml2/'],
    name_prefix: ''
)

libxml2_stub_dep = declare_dependency(
    link_with: libxml2_stub,
    include_directories: ['libxml2/']
)

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
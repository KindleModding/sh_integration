project('sh_integration', 'c', version : '1.0.0', default_options: ['warning_level=everything'])

# Fetch our dependencies
subproject('cjson', default_options: ['default_library=static']).get_variable('libcjson_dep')
cjson_dep = dependency('libcjson')

subdir('./stubs')
lipc_dep = lipc_stub_dep#dependency('lipc')

subdir('./launcher')
subdir('./extractor')
subdir('./tests')
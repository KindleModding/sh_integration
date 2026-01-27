project('sh_integration', 'c', version : '3.1.0', default_options: ['warning_level=everything'])

arch = meson.get_external_property('arch', 'armhf')
if arch == 'armhf'
    add_project_arguments('-DARCH_ARMHF', language: 'c')
endif

# Fetch our dependencies
subproject('cjson', default_options: ['default_library=static']).get_variable('libcjson_dep')
cjson_dep = dependency('libcjson')

subdir('./stubs')
lipc_dep = lipc_stub_dep

subdir('./launcher')
subdir('./extractor')
subdir('./tests')
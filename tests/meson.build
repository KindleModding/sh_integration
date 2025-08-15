utils_test = executable('utils_test', ['utils_test.c', '../utils/utils.c'], include_directories: ['../utils', '.'])
test('Test Utils', utils_test, is_parallel : false)

extractor_test_change_request = executable('extractor_test_change_request', ['extractor_test_change_request.c', '../extractor/main.c', '../utils/utils.c'], include_directories: ['../utils', '.'], dependencies: [ cjson_dep, scanner_stub_dep ])
test('Test Extractor Change Request Generator', extractor_test_change_request, is_parallel : false)

extractor_test_add_file = executable('extractor_test_add_file', ['extractor_test_add_file.c', '../extractor/main.c', '../utils/utils.c'], include_directories: ['../utils', '.'], dependencies: [ cjson_dep, scanner_stub_dep ])
configure_file(
  input: 'test_image.png',
  output: 'example_icon.png',
  copy: true
)
test('Test Extractor Add File', extractor_test_add_file, is_parallel : false)

extractor_test_remove_file = executable('extractor_test_remove_file', ['extractor_test_remove_file.c', '../extractor/main.c', '../utils/utils.c'], include_directories: ['../utils', '.'], dependencies: [ cjson_dep, scanner_stub_dep ])
test('Test Extractor Remove File', extractor_test_remove_file, is_parallel : false)

launcher_test_generate_command = executable('launcher_test_generate_command', ['launcher_test_generate_command.c', '../launcher/main.c', '../utils/utils.c'], include_directories: ['../utils', '.'], dependencies: [ lipc_dep, scanner_stub_dep ], c_args: ['-DLAUNCHER_TESTING'])
test('Test Launcher Generate Command', launcher_test_generate_command, is_parallel: false)
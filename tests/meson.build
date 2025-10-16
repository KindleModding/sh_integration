configure_file(
  input: 'test.sh',
  output: 'test.sh',
  copy: true
)
configure_file(
  input: 'test_fbink.sh',
  output: 'test_fbink.sh',
  copy: true
)
configure_file(
  input: 'test_hooks_fbink.sh',
  output: 'test_hooks_fbink.sh',
  copy: true
)
configure_file(
  input: 'test_hooks.sh',
  output: 'test_hooks.sh',
  copy: true
)
configure_file(
  input: 'test_noheader.sh',
  output: 'test_noheader.sh',
  copy: true
)
configure_file(
  input: 'test_noicon.sh',
  output: 'test_noicon.sh',
  copy: true
)

configure_file(
  input: 'test_image.png',
  output: 'example_icon.png',
  copy: true
)

###
# Edge Cases
###
configure_file(
  input: 'KindleCraft.sh',
  output: 'KindleCraft.sh',
  copy: true
)

utils_test = executable('utils_test', ['utils_test.c', '../utils/utils.c'], include_directories: ['../utils', '.'])
test('Test Utils', utils_test, is_parallel : false)

extractor_test_change_request = executable('extractor_test_change_request', ['extractor_test_change_request.c', '../extractor/main.c', '../utils/utils.c'], include_directories: ['../utils', '.'], dependencies: [ cjson_dep, scanner_stub_dep ])
test('Test Extractor Change Request Generator', extractor_test_change_request, is_parallel : false)

extractor_test_unknown_event = executable('extractor_test_unknown_event', ['extractor_test_unknown_event.c', '../extractor/main.c', '../utils/utils.c'], include_directories: ['../utils', '.'], dependencies: [ cjson_dep, scanner_stub_dep ])
test('Test Extractor Unknown Event', extractor_test_unknown_event, is_parallel : false)

extractor_test_add_file = executable('extractor_test_add_file', ['extractor_test_add_file.c', '../extractor/main.c', '../utils/utils.c'], include_directories: ['../utils', '.'], dependencies: [ cjson_dep, scanner_stub_dep ])
test('Test Extractor Add File', extractor_test_add_file, is_parallel : false)


extractor_test_update_file = executable('extractor_test_update_file', ['extractor_test_update_file.c', '../extractor/main.c', '../utils/utils.c'], include_directories: ['../utils', '.'], dependencies: [ cjson_dep, scanner_stub_dep ])
test('Test Extractor Update File', extractor_test_update_file, is_parallel : false)

extractor_test_remove_file = executable('extractor_test_remove_file', ['extractor_test_remove_file.c', '../extractor/main.c', '../utils/utils.c'], include_directories: ['../utils', '.'], dependencies: [ cjson_dep, scanner_stub_dep ])
test('Test Extractor Remove File', extractor_test_remove_file, is_parallel : false)

extractor_test_add_noicon_file = executable('extractor_test_add_noicon_file', ['extractor_test_add_noicon_file.c', '../extractor/main.c', '../utils/utils.c'], include_directories: ['../utils', '.'], dependencies: [ cjson_dep, scanner_stub_dep ])
test('Test Extractor Add Noicon File', extractor_test_add_noicon_file, is_parallel : false)

extractor_test_edgecases = executable('extractor_test_edgecases', ['extractor_test_edgecases.c', '../extractor/main.c', '../utils/utils.c'], include_directories: ['../utils', '.'], dependencies: [ cjson_dep, scanner_stub_dep ])
test('Test Extractor Edge Cases', extractor_test_edgecases, is_parallel : false)

launcher_test_generate_command = executable('launcher_test_generate_command', ['launcher_test_generate_command.c', '../launcher/main.c', '../utils/utils.c'], include_directories: ['../utils', '.'], dependencies: [ lipc_dep, scanner_stub_dep ], c_args: ['-DLAUNCHER_TESTING'])
test('Test Launcher Generate Command', launcher_test_generate_command, is_parallel: false)
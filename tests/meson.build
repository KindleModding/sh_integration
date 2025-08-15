testUtils = executable('testUtils', 'testUtils.c', include_directories: ['../utils', '.'])
test('Test Utils', testUtils, is_parallel : false)

testChangeRequest = executable('testChangeRequest', ['testChangeRequest.c', '../extractor/main.c'], include_directories: ['../utils', '.'], dependencies: [ cjson_dep, scanner_stub_dep ])
test('Test Change Request Generator', testChangeRequest, is_parallel : false)

testIndexFile = executable('testIndexFile', ['testIndexFile.c', '../extractor/main.c'], include_directories: ['../utils', '.'], dependencies: [ cjson_dep, scanner_stub_dep ])
configure_file(
  input: 'test_image.png',
  output: 'example_icon.png',
  copy: true
)
test('Test Index File', testIndexFile, is_parallel : false)

testRemoveFile = executable('testRemoveFile', ['testRemoveFile.c', '../extractor/main.c'], include_directories: ['../utils', '.'], dependencies: [ cjson_dep, scanner_stub_dep ])
test('Test Remove File', testRemoveFile, is_parallel : false)


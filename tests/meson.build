testUtils = executable('testUtils', 'testUtils.c', include_directories: ['../utils', '.'])
test('Test Utils', testUtils)

testChangeRequest = executable('testChangeRequest', ['testChangeRequest.c', '../extractor/main.c'], include_directories: ['../utils', '.'], dependencies: [ cjson_dep, scanner_stub_dep ])
test('Test Change Request Generator', testChangeRequest)

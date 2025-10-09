#include "launcher.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    char* command = getScriptCommand("./tests/test.sh");
    assert(command != NULL);
    printf("test.sh - %s\n", command);
    assert(strcmp(command, "sh -l \"./tests/test.sh\"") == 0);

    free(command);
    command = getScriptCommand("./tests/test_fbink.sh");
    assert(command != NULL);
    printf("test_fbink.sh - %s\n", command);
    assert(strcmp(command, "/mnt/us/libkh/bin/fbink -k; sh -l \"./tests/test_fbink.sh\" 2>&1 | /mnt/us/libkh/bin/fbink -y 5 -r") == 0);

    free(command);
    command = getScriptCommand("./tests/test_hooks_fbink.sh");
    assert(command != NULL);
    printf("test_fbink.sh - %s\n", command);
    assert(strcmp(command, "/mnt/us/libkh/bin/fbink -k; sh -l -c \"source \\\"./tests/test_hooks_fbink.sh\\\"; on_run;\" 2>&1 | /mnt/us/libkh/bin/fbink -y 5 -r") == 0);

    free(command);
    command = getScriptCommand("./tests/test_hooks.sh");
    assert(command != NULL);
    printf("test_hooks.sh - %s\n", command);
    assert(strcmp(command, "sh -l -c \"source \\\"./tests/test_hooks.sh\\\"; on_run;\"") == 0);
    free(command);
    return 0;
}
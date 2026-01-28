#include "launcher.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    char* command = getScriptCommand("./tests/Check OTA status v1.1.sh");
    assert(command != NULL);
    printf("Check OTA status v1.1.sh - %s\n", command);
    assert(strcmp(command, "/mnt/us/libkh/bin/fbink -k; sh -l \"./tests/Check OTA status v1.1.sh\" 2>&1 | /mnt/us/libkh/bin/fbink -y 5 -r") == 0);
    free(command);

    command = getScriptCommand("./tests/KindleCraft.sh");
    assert(command != NULL);
    printf("KindleCraft.sh - %s\n", command);
    assert(strcmp(command, "sh -l \"./tests/KindleCraft.sh\"") == 0);
    free(command);

    printf("\n\n");
    const char* test = "4:app://com.notmarek.shell_integration.launcher./tests/KindleCraft.sh";
    go_callback(NULL, "test1", test, NULL);
    printf("\n\n");

    test = "4:app://com.notmarek.shell_integration.launcher./tests/Check OTA status v1.1.sh";
    go_callback(NULL, "test2", test, NULL);
    return 0;
}
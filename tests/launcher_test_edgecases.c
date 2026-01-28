#include "launcher.h"
#include "utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    assert(hexDecode('0') == 0);
    assert(hexDecode('1') == 1);
    assert(hexDecode('2') == 2);
    assert(hexDecode('3') == 3);
    assert(hexDecode('4') == 4);
    assert(hexDecode('5') == 5);
    assert(hexDecode('6') == 6);
    assert(hexDecode('7') == 7);
    assert(hexDecode('8') == 8);
    assert(hexDecode('9') == 9);
    assert(hexDecode('a') == 10);
    assert(hexDecode('b') == 11);
    assert(hexDecode('c') == 12);
    assert(hexDecode('d') == 13);
    assert(hexDecode('e') == 14);
    assert(hexDecode('f') == 15);
    assert(hexDecode('A') == 10);
    assert(hexDecode('B') == 11);
    assert(hexDecode('C') == 12);
    assert(hexDecode('D') == 13);
    assert(hexDecode('E') == 14);
    assert(hexDecode('F') == 15);


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
    char* test = strdup("4:app://com.notmarek.shell_integration.launcher./tests/KindleCraft.sh");
    go_callback(NULL, "test1", test, NULL);
    printf("\n\n");
    free(test);

    test = strdup("4:app://com.notmarek.shell_integration.launcher.%2Ftests%2FCheck%20OTA%20status%20v1.1.sh");
    go_callback(NULL, "test2", test, NULL);
    free(test);
    return 0;
}
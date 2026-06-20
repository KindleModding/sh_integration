#include "launcher.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    char* test = "4:app://tech.hackerdude.shell_integration.launcher./tests/test.sh";
    go_callback(NULL, "test", test, NULL);
    return 0;
}
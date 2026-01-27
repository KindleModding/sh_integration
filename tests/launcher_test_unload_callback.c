#include "launcher.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    char* test = "4:app://com.notmarek.shell_integration.launcherhelp";
    unload_callback(NULL, "test", test, NULL);
    return 0;
}
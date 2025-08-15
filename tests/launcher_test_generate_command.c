#include "launcher.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main()
{
    char* command = getScriptCommand("./test.sh");
    assert(strcmp(command, "sh -c \"source \\\"./test.sh\\\"; on_run;\"") == 0);
    
    return 0;
}
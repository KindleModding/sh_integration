#include "utils.h"
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

int main()
{
    char* command = buildCommand("test %s testing", "hello");
    assert(strcmp(command, "test hello testing") == 0);

    fprintf(stderr, "Creating test.sh\n");
    FILE* file = fopen("./test.sh", "w+");
    assert(file != NULL);
    fputs("#!/bin/sh\n", file);
    fputs("# Name: TestName\n", file);
    fputs("# Author: TestAuthor\n", file);
    fputs("# Icon: data:image/png;base64,hello\n", file);
    fputs("# UseHooks\n", file);
    fputs("# DontUseFBInk\n", file);
    fputs("#BLANK\n#BLANK\n#BLANK\n#BLANK\n#BLANK\n#BLANK\n", file);
    fseek(file, 0, SEEK_SET);

    fprintf(stderr, "Reading file header\n");
    struct ScriptHeader header;
    readScriptHeader(file, &header);
    fprintf(stderr, "Name: %s\n", header.name);
    assert(strcmp(header.name, "TestName") == 0);
    fprintf(stderr, "Author: %s\n", header.name);
    assert(strcmp(header.author, "TestAuthor") == 0);
    fprintf(stderr, "Icon: %s\n", header.name);
    assert(strcmp(header.icon, "data:image/png;base64,hello") == 0);
    fprintf(stderr, "UseHooks: %i\n", header.useHooks);
    assert(header.useHooks);
    fprintf(stderr, "UseFBInk: %i\n", header.useFBInk);
    assert(!header.useFBInk);

    fprintf(stderr, "Freeing header strings\n");
    freeScriptHeader(&header);
}
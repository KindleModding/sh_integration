#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

char* urlDecode(char* raw);

void recursiveDelete(char* path);

char* buildCommand(char* command, char* sub);

struct ScriptHeader
{
    char* name;
    char* author;
    char* icon;
    bool useHooks;
    bool useFBInk;
};

void freeScriptHeader(struct ScriptHeader* header);

void readScriptHeader(FILE* file, struct ScriptHeader* header);
#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

int hexDecode(char c);
char* urlDecode(char* raw);
void strip(char** string);

void recursiveDelete(char* path);

char* buildCommand(const char* command, const char* sub);

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
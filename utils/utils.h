#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdarg.h>

char* vasprintf_hd(const char * format, va_list args);
char* asprintf_hd(const char * format, ...) __attribute__((format(printf, 1, 2)));

int hexDecode(char c);
char* urlDecode(char* raw);
void strip(char** string);

void rmdir_r(char* path);

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
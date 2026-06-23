#include "utils.h"
#include <ctype.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define __USE_XOPEN_EXTENDED   /* See feature_test_macros(7) */
#include <ftw.h>

/**
 * @brief vasprintf implementation
 * 
 * @param format 
 * @param args
 * @return char* 
 */
char* vasprintf_hd(const char * format, va_list args)
{
    va_list args2;
    va_copy (args2, args);
    size_t size = vsnprintf(NULL, 0, format, args) + 1;
    char* str = (char*) malloc(size);
    vsnprintf(str, size, format, args2);
    va_end(args2);
    return str;
}

/**
 * @brief asprintf implementation
 * 
 * @param format 
 * @param ... 
 * @return char* 
 */
char* asprintf_hd(const char * format, ...)
{
    va_list args;
    va_start(args, format);
    char* result = vasprintf_hd(format, args);
    va_end(args);
    return result;
}

int hexDecode(const char c)
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }
    else if (c >= 'a' && c <= 'z')
    {
        return c - 'a' + 10;
    }
    else if (c >= 'A' && c <= 'Z')
    {
        return c - 'A' + 10;
    }
    return 0;
}

char* urlDecode(char* raw)
{
    char* result = (char*) malloc(strlen(raw) + 1); // URLEncoded string will NEVER be longer decoded
    int currentFilepathLen = 0;

    for (size_t i=0; i < strlen(raw); i++) {
        if (raw[i] == '%') {
            result[currentFilepathLen++] = (char)((hexDecode(raw[i+1]) << 4) + hexDecode(raw[i+2]));
            i += 2;
        } else {
            result[currentFilepathLen++] = raw[i];
        }
    }
    result[currentFilepathLen] = '\0';
    return result;
}

void strip(char** string)
{
    char* strippedString = *string;
    while (isspace(*strippedString)) { (strippedString)++; };
    char* back = strippedString + strlen(strippedString);
    while(back > strippedString && isspace(*(--back))) { printf("Back: [%c]\n", *back); *back = '\0'; };
    
    char* finalString = strdup(strippedString);
    free(*string);
    *string = finalString;
}

int internal_delete(const char* fpath, const struct stat*, int typeflag, struct FTW*)
{
    switch (typeflag)
    {
        case FTW_F:
            remove(fpath);
            break;
        case FTW_D:
        case FTW_DP:
            rmdir(fpath);
            break;
        default:
            break;
    }
    return 0;
}

/**
 * @brief Deletes a folder recursively, equivalent to "rm -rf"
 * 
 * @param path 
 */
void rmdir_r(char* path)
{
    nftw(path, &internal_delete, 256, FTW_DEPTH);
    rmdir(path);
}

void freeScriptHeader(struct ScriptHeader* header)
{
    free(header->name);
    free(header->author);
    free(header->icon);

    header->name = NULL;
    header->author = NULL;
    header->icon = NULL;
}

void readScriptHeader(FILE* file, struct ScriptHeader* header)
{
    header->author=NULL;
    header->icon=NULL;
    header->name=NULL;
    header->useFBInk=true;
    header->useHooks=false;
    
    size_t bufferSize = 1024;
    char* buffer = (char*) malloc(bufferSize);
    fseek(file, 0, SEEK_SET);
    for (int i=0; i < 6; i++) {
        buffer[0] = '\0';
        size_t lineLength = 0;
        int c;
        while ((c = fgetc(file)) != EOF)
        {
            if (c == '\n' || c == '\r')
            {
                break;
            }

            if (lineLength + 2 >= bufferSize)
            {
                buffer = (char*) realloc(buffer, bufferSize+=(lineLength + 2));
                if (buffer == NULL)
                {
                    printf("FATAL - FAILED TO REALLOC BUFFER!!!\n");
                    return; //@TODO: We don't really have a good way of dealing with this
                }
            }

            buffer[lineLength++] = (char) c;
        }
        buffer[lineLength] = '\0';
        
        // Start reading the header
        if (strncmp(buffer, "# Name: ", strlen("# Name: ")) == 0)
        {
            header->name = strdup(buffer + strlen("# Name: "));
            strip(&header->name);
            if (strlen(header->name) == 0)
            {
                free(header->name);
                header->name = NULL;
            }
        }
        else if (strncmp(buffer, "# Author: ", strlen("# Author: ")) == 0)
        {
            header->author = strdup(buffer + strlen("# Author: "));
            strip(&header->author);
            if (strlen(header->author) == 0)
            {
                free(header->author);
                header->author = NULL;
            }
        }
        else if (strncmp(buffer, "# Icon: ", strlen("# Icon: ")) == 0)
        {
            header->icon = strdup(buffer + strlen("# Icon: "));
            strip(&header->icon);
            if (strlen(header->icon) == 0)
            {
                free(header->icon);
                header->icon = NULL;
            }
        }
        else if (strncmp(buffer, "# UseHooks", strlen("# UseHooks")) == 0)
        {
            header->useHooks = true;
        }
        else if (strncmp(buffer, "# DontUseFBInk", strlen("# DontUseFBInk")) == 0)
        {
            header->useFBInk = false;
        }

        if (c == EOF)
        {
            break;
        }
    }
    free(buffer);
}
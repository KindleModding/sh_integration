#include "utils.h"
#include <stdlib.h>
#include <string.h>

char* urlDecode(char* raw)
{
    char* result = malloc(strlen(raw) + 1); // URLEncoded string will NEVER be longer decoded
    int currentFilepathLen = 0;

    for (size_t i=0; i < strlen(raw); i++) {
        if (raw[i] == '%') {
            result[currentFilepathLen++] = (char)(((raw[i+1] - '0') << 4) + (raw[i+2] - '0'));
            i += 2;
        } else {
            result[currentFilepathLen++] = raw[i];
        }
    }
    result[currentFilepathLen] = '\0';
    return result;
}

void recursiveDelete(char* path)
{
    DIR* dir = opendir(path);
    struct dirent* item;
    while ((item = readdir(dir)) != NULL)
    {
        if (strcmp(item->d_name, ".") == 0 || strcmp(item->d_name, "..") == 0)
        {
            continue;
        }
        char* itemPath = malloc(strlen(path) + 1 + strlen(item->d_name) + 1);
        sprintf(itemPath, "%s/%s", path, item->d_name);
        if (item->d_type == DT_DIR)
        {
            recursiveDelete(itemPath);
        }
        remove(itemPath);
        free(itemPath);
    }
    remove(path);
    closedir(dir);
}

char* buildCommand(char* command, char* sub)
{
    char* builtCommand = malloc(strlen(command) + strlen(sub) + 1);
    sprintf(builtCommand, command, sub);
    return builtCommand;
}

void freeScriptHeader(struct ScriptHeader* header)
{
    free(header->name);
    free(header->author);
    free(header->icon);

    header->name = NULL;
    header->author = NULL;
    header->icon = NULL;
};

void readScriptHeader(FILE* file, struct ScriptHeader* header)
{
    header->author=NULL;
    header->icon=NULL;
    header->name=NULL;
    header->useFBInk=true;
    header->useHooks=false;
    
    int bufferSize = 1024;
    char* buffer = malloc(bufferSize);
    fseek(file, 0, SEEK_SET);
    for (int i=0; i < 6; i++) {
        buffer[0] = '\0';
        int lineLength = 0;
        char c;
        while ((c = fgetc(file)) != EOF)
        {
            if (c == '\n' || c == '\r')
            {
                break;
            }

            if (lineLength + 2 >= bufferSize)
            {
                buffer = realloc(buffer, bufferSize+=1024);
            }

            buffer[lineLength++] = c;
        }
        buffer[lineLength] = '\0';

        // Start reading the header
        if (strncmp(buffer, "# Name: ", strlen("# Name: ")) == 0)
        {
            header->name = strdup(buffer + strlen("# Name: "));
        }
        else if (strncmp(buffer, "# Author: ", strlen("# Author: ")) == 0)
        {
            header->author = strdup(buffer + strlen("# Author: "));
        }
        else if (strncmp(buffer, "# Icon: ", strlen("# Icon: ")) == 0)
        {
            header->icon = strdup(buffer + strlen("# Icon: "));
        }
        else if (strncmp(buffer, "# UseHooks", strlen("# UseHooks")) == 0)
        {
            header->useHooks = true;
        }
        else if (strncmp(buffer, "# DontUseFBInk", strlen("# DontUseFBInk")) == 0)
        {
            header->useFBInk = false;
        }
    }
    free(buffer);
};
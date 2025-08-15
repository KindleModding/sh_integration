#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* buildCommand(char* command, char* sub)
{
    char* builtCommand = malloc(strlen(command) + strlen(sub) + 1);
    sprintf(builtCommand, command, sub);
    return builtCommand;
}

struct ScriptHeader
{
    char* name;
    char* author;
    char* icon;
    bool useHooks;
    bool useFBInk;
};

void freeScriptHeader(struct ScriptHeader* header)
{
    free(header->name);
    free(header->author);
    free(header->icon);
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
        while (!feof(file))
        {
            char character = fgetc(file);
            if (character == '\n' || character == '\r')
            {
                break;
            }

            if (lineLength + 2 >= bufferSize)
            {
                buffer = realloc(buffer, bufferSize+=1024);
            }

            buffer[lineLength++] = character;
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
#include "scanner.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>
#include "cjson/cJSON.h"
#include "utils.h"

void Log(const char* format, ...)
{
    va_list args;
    va_start (args, format);
    vprintf (format, args);
    printf("\n");
    vsyslog(LOG_INFO, format, args);
    va_end (args);
}

cJSON* generateChangeRequest(cJSON* json, char* filePath, char* uuid, char* name_string, char* author_string, char* icon_string, bool new) {
    struct stat st;
    stat(filePath, &st);

    cJSON* changeRequestString = cJSON_CreateString("ChangeRequest");
    cJSON_AddItemToObject(json, "type", changeRequestString);
    cJSON* commands = cJSON_CreateArray();
    cJSON* command = cJSON_CreateObject();
    cJSON* insert = cJSON_CreateObject();
    cJSON* jsonUUID = cJSON_CreateString(uuid);
    cJSON_AddItemToObject(insert, "uuid", jsonUUID);
    cJSON* jsonLocation = cJSON_CreateString(filePath);
    cJSON_AddItemToObject(insert, "location", jsonLocation);
    
    cJSON* entryType = cJSON_CreateString("Entry:Item");
    cJSON_AddItemToObject(insert, "type", entryType);

    cJSON* modificationTime = cJSON_CreateNumber(st.st_mtim.tv_sec);
    cJSON_AddItemToObject(insert, "modificationTime", modificationTime);

    cJSON* diskUsage = cJSON_CreateNumber(st.st_size);
    cJSON_AddItemToObject(insert, "diskUsage", diskUsage);

    cJSON* contentSize = cJSON_CreateNumber(st.st_size);
    cJSON_AddItemToObject(insert, "contentSize", contentSize);

    cJSON* mimeType = cJSON_CreateString("text/x-shellscript");
    cJSON_AddItemToObject(insert, "mimeType", mimeType);

    cJSON* cdeKey = cJSON_CreateString(getSha1Hash(filePath));
    cJSON_AddItemToObject(insert, "cdeKey", cdeKey);

    cJSON* cdeType = cJSON_CreateString("PDOC");
    cJSON_AddItemToObject(insert, "cdeType", cdeType);

    if (new)
    {
        const char* tag = "NEW";
        cJSON* displayTags = cJSON_CreateStringArray(&tag, 1);
        cJSON_AddItemToObject(insert, "displayTags", displayTags);
    }
    else
    {
        cJSON* percentFinished = cJSON_CreateNumber(1337);
        cJSON_AddItemToObject(insert, "percentFinished", percentFinished);
    }

    cJSON* isVisibleInHome = cJSON_CreateTrue();
    cJSON_AddItemToObject(insert, "isVisibleInHome", isVisibleInHome);

    cJSON* isArchived = cJSON_CreateFalse();
    cJSON_AddItemToObject(insert, "isArchived", isArchived);

    cJSON* displayObjects = cJSON_CreateArray();
    cJSON* titleDisplayObject = cJSON_CreateObject();
    cJSON* titleRef = cJSON_CreateString("titles");
    cJSON* creditsDisplayObject = cJSON_CreateObject();
    cJSON* creditsRef = cJSON_CreateString("credits");
    cJSON_AddItemToObject(titleDisplayObject, "ref", titleRef);
    cJSON_AddItemToObject(creditsDisplayObject, "ref", creditsRef);
    cJSON_AddItemToArray(displayObjects, titleDisplayObject);
    cJSON_AddItemToArray(displayObjects, creditsDisplayObject);
    cJSON_AddItemToObject(insert, "displayObjects", displayObjects);

    cJSON* credits = cJSON_CreateArray();
    cJSON* credit = cJSON_CreateObject();
    cJSON* kind = cJSON_CreateString("Author");
    cJSON* name = cJSON_CreateObject();
    
    cJSON* author_display;
    if (author_string)
    {
        author_display = cJSON_CreateString(author_string);
    }
    else
    {
        author_display = cJSON_CreateString("Unknown");
    }

    cJSON_AddItemToObject(name, "display", author_display);
    cJSON_AddItemToObject(credit, "kind", kind);
    cJSON_AddItemToObject(credit, "name", name);
    cJSON_AddItemToArray(credits, credit);
    cJSON_AddItemToObject(insert, "credits", credits);

    cJSON* titles = cJSON_CreateArray();
    cJSON* title_display = cJSON_CreateObject();
    cJSON* title;
    if (name_string)
    {
        title = cJSON_CreateString(name_string);
    }
    else
    {
        title = cJSON_CreateString(basename(filePath));
    }

    if (icon_string)
    {
        cJSON* thumbnail = cJSON_CreateString(icon_string);
        cJSON_AddItemToObject(insert, "thumbnail", thumbnail);
    }
    
    cJSON_AddItemToObject(title_display, "display", title);
    cJSON_AddItemToArray(titles, title_display);
    cJSON_AddItemToObject(insert, "titles", titles);
    cJSON_AddItemToObject(command, "insert", insert);
    cJSON_AddItemToArray(commands, command);
    cJSON_AddItemToObject(json, "commands", commands);

    return json;
}

void index_file(char *path, char* filename, bool new) {
    Log("Indexing file: %s/%s\n", path, filename);

    char* full_path = malloc(strlen(path) + 1 + strlen(filename) + 1);
    sprintf(full_path, "%s/%s", path, filename);

    // Generate UUID
    char uuid[37] = {0};
    scanner_gen_uuid(uuid, 37);
    Log("Generated UUID: %s", uuid);

    // Read data from file header
    FILE* file = fopen(full_path, "r");
    if (!file) {
        Log("Failed to open file");
        free(full_path);
        return;
    }

    Log("Reading header.");
    struct ScriptHeader header;
    readScriptHeader(file, &header);
    fclose(file);

    bool validIcon = header.icon != NULL && strncmp(header.icon, "data:image", strlen("data:image")) == 0;
    if (validIcon || header.useHooks) {
        Log("Valid icon OR uses hooks");
        // Create sdr folder
        char* sdr_path = malloc(strlen(full_path) + strlen(".sdr") + 1);
        sprintf(sdr_path, "%s.sdr", full_path);
        mkdir(sdr_path, 0755);

        if (validIcon) {
            Log("Valid icon detected, attempting to extract it");
            //data:image/jpeg;base64,iVBORw0KGgoAAAANSUhEUgAAA
            char* fileTypePointer = strchr(header.icon, '/')+1;
            char* fileTypeEndPointer = strchr(header.icon, ';');
            char* b64Pointer = strchr(header.icon, ',')+1;
            if (fileTypeEndPointer == NULL)
            {
                fileTypeEndPointer = b64Pointer;
            }

            char* fileType = malloc(fileTypeEndPointer - fileTypePointer + 1);
            strncpy(fileType, fileTypePointer, fileTypeEndPointer - fileTypePointer);
            fileType[fileTypeEndPointer - fileTypePointer] = '\0';

            char* icon_sdr_path = malloc(strlen(sdr_path) + strlen("/icon.") + strlen(fileType) + 1);
            sprintf(icon_sdr_path, "%s/icon.%s", sdr_path, fileType);

            // Parse the base64
            FILE* file = fopen(icon_sdr_path, "wb");

            char currentByte = 0;
            int processedBits = 0;
            for (int i=(b64Pointer - header.icon); i < strlen(header.icon); i++) {
                // Convert the base64 character to binary
                char value = 0;
                if (header.icon[i] >= 'A' && header.icon[i] <= 'Z') {
                    value = header.icon[i] - 'A';
                } else if (header.icon[i] >= 'a' && header.icon[i] <= 'z') {
                    value = (header.icon[i] - 'a') + 26;
                } else if (header.icon[i] >= '0' && header.icon[i] <= '9') {
                    value = (header.icon[i] - '0') + 52;
                } else if (header.icon[i] == '+') {
                    value = 62;
                } else if (header.icon[i] == '/') {
                    value = 63;
                } else if (header.icon[i] != '=') {
                    Log("Invalid B64 at position %i", i); // Warn
                }

                // Add data to the currentByte
                currentByte |= (value << 2) >> processedBits; // Shift back by two because base64 chars only represent 6 bits
                int consumedBits = (8 - processedBits) < 6 ? 8 - processedBits : 6; // We process a maximum of 6 bits at once
                processedBits += consumedBits;

                if (processedBits >= 8) {
                    if (header.icon[i] == '=')
                    {
                        break; // We terminate on padding since the last bits needed are already 0
                    }

                    putc(currentByte, file);
                    processedBits -= 8;

                    // Set new currentByte to leftover bits
                    currentByte = 0;
                    currentByte |= value << (2 + consumedBits);
                    processedBits = 6 - consumedBits;
                }
            }
            fclose(file);

            free(header.icon);
            header.icon = strdup(icon_sdr_path);//realloc(header.icon, strlen(icon_sdr_path) + 1);
            free(icon_sdr_path);
            free(fileType);
        }

        if (header.useHooks) {
            Log("Script uses hooks!");
            // If the file is functional, run install hook
            char* escapedPath = malloc(strlen(full_path)*2 + 1);
            int escapedPathLength = 0;
            for (int i=0; i < strlen(full_path); i++) {
                if (full_path[i] == '"') {
                    escapedPath[escapedPathLength++] = '\\';
                }
                escapedPath[escapedPathLength++] = full_path[i];
            }
            escapedPath[escapedPathLength] = '\0';

            Log("Running install event");
            const int pid = fork();
            if (pid == 0) {
                Log("Hello from fork!");
                char* command = malloc(escapedPathLength + 32);
                sprintf(command, "source \"%s\"; on_install;", escapedPath);
                Log("Executing command: %s", command);
                execl("/var/local/mkk/su", "su", "-c", command, NULL);
            } else {
                Log("Waiting for install to complete...");
                waitpid(pid, NULL, 0);
                Log("Done!");
            }

            free(escapedPath);

            char* sdrFilePath = malloc(strlen(sdr_path) + strlen("/script.sh") + 1);
            sprintf(sdrFilePath, "%s/script.sh", sdr_path);
            Log("Writing script to %s\n", sdrFilePath);
            FILE* scriptFile = fopen(full_path, "r");
            FILE* sdrFile = fopen(sdrFilePath, "w");
            char c;
            while ((c = getc(scriptFile)) != EOF) {
                putc(c, sdrFile);
            }
            Log("Done!");
            fclose(scriptFile);
            fclose(sdrFile);
            free(sdrFilePath);
        }

        free(sdr_path);
    }


    // Create JSON objects
    cJSON* json = cJSON_CreateObject();
    if (!json) {
        fprintf(stderr, "Failed to create a JSON object");
        return;
    }
    
    generateChangeRequest(json, full_path, uuid, header.name, header.author, header.icon, new);
    

    const int result = scanner_post_change(json);
    char* stringJSON = cJSON_Print(json);
    Log("Indexing json:\n%s\n\n", stringJSON);
    free(stringJSON);
    Log("ccat error: %d\n", result);
    //printf("Json: %s\n", cJSON_Print(json));

    if (json)
    {
        cJSON_Delete(json);
    }
    // Can you believe that cJSON deleting causes issues
    freeScriptHeader(&header);
    free(full_path);
}

void remove_file(const char* path, const char* filename, char* uuid) {
    Log("Removing file: %s/%s", path, filename);
    char* filePath = malloc(strlen(path) + 1 + strlen(filename) + 1);
    sprintf(filePath, "%s/%s", path, filename);

    char* sdrPath = malloc(strlen(filePath) + strlen(".sdr") + 1);
    sprintf(sdrPath, "%s.sdr", filePath);
    free(filePath);
    
    char* sdrScriptPath = malloc(strlen(sdrPath) + strlen("/script.sh") + 1);
    sprintf(sdrScriptPath, "%s/script.sh", sdrPath);
    
    Log("Loading file");
    FILE* file = fopen(sdrScriptPath, "r");
    if (file) {
        struct ScriptHeader header;
        readScriptHeader(file, &header);
        fclose(file);

        // If the file is uses hooks, run removal hook
        if (header.useHooks) {
            Log("Script uses hooks!");
            char* escapedPath = malloc(strlen(sdrScriptPath)*2 + 1);
            int escapedPathLength = 0;
            for (int i=0; i < strlen(sdrScriptPath); i++) {
                if (sdrScriptPath[i] == '"') {
                    escapedPath[escapedPathLength++] = '\\';
                }
                escapedPath[escapedPathLength++] = sdrScriptPath[i];
            }
            escapedPath[escapedPathLength] = '\0';

            Log("Running remove event");
            const int pid = fork();
            if (pid == 0) {
                Log("Hello from fork!");
                char* command = buildCommand("source \"%s\"; on_remove;", escapedPath);
                Log("Executing command: %s", command);
                free(escapedPath);
                execl("/var/local/mkk/su", command, NULL);
                free(command);
            } else {
                Log("Hello from parent! Waiting.");
                free(escapedPath);
                waitpid(pid, NULL, 0);
                Log("Wait complete.");
            }
        }
        freeScriptHeader(&header);
        Log("Removing: %s\n", sdrPath);
    }

    if (access(sdrPath, R_OK|W_OK) == F_OK)
    {
        Log("SDR exists - deleting");
        recursiveDelete(sdrPath);
    }
    
    free(sdrPath);
    free(sdrScriptPath);
    Log("Removing ccat entry.");
    scanner_delete_ccat_entry(uuid);
}

int extractor(const struct scanner_event* event) {
    Log("Extractor called with event type %i\n", event->event_type);
    switch (event->event_type) {
        case SCANNER_ADD:
            index_file(event->path, event->filename, true);
            break;
        case SCANNER_DELETE:
            remove_file(event->path, event->filename, event->uuid);
            break;
        case SCANNER_UPDATE:
            remove_file(event->path, event->filename, event->uuid); // Remove SDR and entry
            index_file(event->path, event->filename, false); // Re-index with new metadata and such
            break;
        default:
            // Don't run install hooks and such willy-nilly
            //index_file(event->path, event->filename);
            Log("Received unknown event: %i\n", event->event_type);
            return 1;
    }

    return 0;
}

__attribute__((__visibility__("default"))) int load_extractor(ScannerEventHandler** handler, int *unk1) {
    Log("Extractor initialised.\n");
    openlog("org.kindlemodding.shell_integration.extractor", LOG_PID, LOG_DAEMON);
    *handler = extractor;
    *unk1 = 0;
    return 0;
}
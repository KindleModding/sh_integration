#include "scanner.h"
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

#define __USE_XOPEN_EXTENDED
#include <ftw.h>

cJSON* generateChangeRequest(char* filePath, char* uuid, char* name_string, char* author_string, char* icon_string) {
    syslog(LOG_INFO, "Generating change request for %s\n", filePath);

    struct stat st;
    stat(filePath, &st);

    cJSON* json = cJSON_CreateObject();
    cJSON_AddItemToObject(json, "uuid", cJSON_CreateString(uuid));
    cJSON_AddItemToObject(json, "location", cJSON_CreateString(filePath));
    cJSON_AddItemToObject(json, "type", cJSON_CreateString("Entry:Item"));
    cJSON_AddItemToObject(json, "cdeType", cJSON_CreateString("PDOC"));
    cJSON_AddItemToObject(json, "cdeKey", cJSON_CreateString(getSha1Hash(filePath)));
    cJSON_AddItemToObject(json, "modificationTime", cJSON_CreateNumber(st.st_mtim.tv_sec));
    cJSON_AddItemToObject(json, "diskUsage", cJSON_CreateNumber(st.st_size));
    cJSON_AddItemToObject(json, "isVisibleInHome", cJSON_CreateTrue());
    cJSON_AddItemToObject(json, "isArchived", cJSON_CreateFalse());
    cJSON_AddItemToObject(json, "mimeType", cJSON_CreateString("text/x-shellscript"));
    cJSON *authors = cJSON_CreateArray();
    cJSON *author = cJSON_CreateObject();
    cJSON_AddItemToObject(author, "kind", cJSON_CreateString("Author"));
    cJSON *author_name = cJSON_CreateObject();
    cJSON_AddItemToObject(author_name, "display", cJSON_CreateString((const char *)(author_string != NULL ? author_string : "Unknown")));
    cJSON_AddItemToObject(author, "name", author_name);
    cJSON_AddItemToArray(authors, author);
    cJSON_AddItemToObject(json, "credits", authors);
    cJSON_AddItemToObject(json, "publisher", cJSON_CreateString("KMC"));
    cJSON *refs = cJSON_CreateArray();
    cJSON *titles_ref = cJSON_CreateObject();
    cJSON_AddItemToObject(titles_ref, "ref", cJSON_CreateString("titles"));
    cJSON_AddItemToArray(refs, titles_ref);
    cJSON *authors_ref = cJSON_CreateObject();
    cJSON_AddItemToObject(authors_ref, "ref", cJSON_CreateString("credits"));
    cJSON_AddItemToArray(refs, authors_ref);
    cJSON_AddItemToObject(json, "displayObjects", refs);
    const char *tags[] = {"NEW"};
    cJSON_AddItemToObject(json, "displayTags", cJSON_CreateStringArray(tags, 1));
    if (icon_string != NULL) {
        cJSON_AddItemToObject(json, "thumbnail",
                            cJSON_CreateString(icon_string));
    }
    cJSON *titles = cJSON_CreateArray();
    cJSON *title = cJSON_CreateObject();
    cJSON_AddItemToObject(
        title, "display",
        cJSON_CreateString((const char *)(name_string != NULL ? name_string : basename(filePath))));
    cJSON_AddItemToArray(titles, title);
    cJSON_AddItemToObject(json, "titles", titles);

    return json;
}

char* buildCommand(char* command, char* sub)
{
    char* builtCommand = malloc(strlen(command) + strlen(sub) + 1);
    sprintf(builtCommand, command, sub);
    return builtCommand;
}

typedef cJSON* (ChangeRequestGenerator)(const char* file_path, const char* uuid);
void index_file(char *path, char* filename) {
    syslog(LOG_INFO, "Indexing file: %s/%s", path, filename);

    char* full_path = malloc(strlen(path) + 1 + strlen(filename) + 1);
    sprintf(full_path, "%s/%s", path, filename);

    // Generate UUID
    char uuid[37] = {0};
    scanner_gen_uuid(uuid, 37);

    bool useHooks = false;

    // Read data from file header
    FILE* file = fopen(full_path, "r");
    if (!file) {
        free(full_path);
        return;
    }

    struct ScriptHeader header;
    readScriptHeader(file, &header);
    fclose(file);

    bool validIcon = header.icon != NULL && strncmp(header.icon, "data:image", strlen("data:image"));
    if (validIcon || useHooks) {
        // Create sdr folder
        char* sdr_path = strdup(full_path);
        strcat(sdr_path, ".sdr");
        mkdir(sdr_path, 0755);

        if (validIcon) {
            //data:image/jpeg;base64,iVBORw0KGgoAAAANSUhEUgAAA
            char* fileTypePointer = strchr(header.icon, '/')+1;
            char* fileTypeEndPointer = strchr(header.icon, ';')-1;
            char* b64Pointer = strchr(header.icon, ',')+1;
            if (fileTypeEndPointer == NULL)
            {
                fileTypeEndPointer = b64Pointer-1;
            }

            char* fileType = malloc(fileTypeEndPointer - fileTypePointer + 1);
            strncpy(fileType, fileTypePointer, fileTypeEndPointer - fileTypePointer);
            fileType[fileTypeEndPointer - fileTypePointer] = '\0';

            char* icon_sdr_path = malloc(strlen(full_path) + strlen(".sdr/icon.") + strlen(fileType) + 1);
            icon_sdr_path[0] = '\0';
            strcat(icon_sdr_path, full_path);
            strcat(icon_sdr_path, ".sdr/icon.");
            strcat(icon_sdr_path, fileType);

            // Parse the base64
            FILE* file = fopen(icon_sdr_path, "wb");

            char currentByte = 0;
            int processedBits = 0;
            for (int i=0; i < (strlen(header.icon) - strlen(b64Pointer)); i++) {
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
                    syslog(LOG_WARNING, "Invalid B64 at position %i", i); // Warn
                }

                // Add data to the currentByte
                currentByte |= (value << 2) >> processedBits;
                int consumedBits = (8 - processedBits) < 6 ? 8 - processedBits : 6;
                processedBits += consumedBits;

                if (processedBits >= 8) {
                    putc(currentByte, file);
                    processedBits -= 8;

                    if (header.icon[i] == '=') {
                        break; // Terminate on padding
                    }

                    // Set new currentByte to leftover bits
                    currentByte = 0;
                    currentByte |= value << (2 + consumedBits);
                    processedBits = 6 - consumedBits;
                }
            }
            fclose(file);

            free(header.icon); // Shennanigans
            header.icon = icon_sdr_path;
            free(sdr_path);
        }

        if (useHooks) {
            // If the file is functional, run install hook
            if (useHooks) {
                char* escapedPath = malloc(strlen(full_path)*2 + 1);
                int escapedPathLength = 0;
                for (int i=0; i < strlen(full_path); i++) {
                    if (full_path[i] == '"') {
                        escapedPath[escapedPathLength++] = '\\';
                    }
                    escapedPath[escapedPathLength++] = full_path[i];
                }
                escapedPath[escapedPathLength] = '\0';

                syslog(LOG_INFO, "Running install event");
                const int pid = fork();
                if (pid == 0) {
                    char* command = malloc(escapedPathLength + 32);
                    sprintf(command, "source \"%s\"; on_install;", escapedPath);
                    syslog(LOG_INFO, "Executing command: %s", command);
                    execl("/var/local/mkk/su", "su", "-c", command, NULL);
                } else {
                    waitpid(pid, NULL, 0);
                }

                free(escapedPath);

                FILE* scriptFile = fopen(full_path, "rb");
                char* sdrFilePath = malloc(strlen(full_path) + strlen("/script.sh") + 1);
                sdrFilePath[0] = '\0';
                strcat(sdrFilePath, full_path);
                strcat(sdrFilePath, "/script.sh");
                FILE* sdrFile = fopen(sdrFilePath, "wb");
                while (!feof(scriptFile)) {
                    putc(getc(scriptFile), sdrFile);
                }
            }
        }
    }


    // Create JSON objects
    cJSON* json = cJSON_CreateObject();
    if (!json) {
        syslog(LOG_CRIT, "Failed to create a JSON object");
        return;
    }

    cJSON_AddItemToObject(json, "type", cJSON_CreateString("ChangeRequest"));

    cJSON* array = cJSON_CreateArray();
    if (!array) {
        syslog(LOG_CRIT, "Failed to create cJSON array");
        cJSON_Delete(json);
        return;
    }

    cJSON* what = cJSON_CreateObject();
    if (!what) {
        syslog(LOG_CRIT, "Failed to create a JSON object");
        cJSON_Delete(json);
        cJSON_Delete(array);
        return;
    }

    cJSON* change = generateChangeRequest(full_path, uuid, header.name, header.author, header.icon);
    if (!change) {
        syslog(LOG_CRIT, "Failed to generate a change request...");
        cJSON_Delete(json);
        cJSON_Delete(array);
        cJSON_Delete(what);
        freeScriptHeader(&header);
        return;
    }

    cJSON* location_filter = cJSON_CreateObject();
    if (!location_filter) {
        syslog(LOG_CRIT, "Failed to create JSON object");
        cJSON_Delete(json);
        cJSON_Delete(array);
        cJSON_Delete(what);
        freeScriptHeader(&header);
        cJSON_Delete(change);
        return;
    }
    cJSON_AddItemToObject(location_filter, "path", cJSON_CreateString("location"));
    cJSON_AddItemToObject(location_filter, "value", cJSON_CreateString(full_path));

    cJSON* Equals = cJSON_CreateObject();
    if (!Equals) {
        syslog(LOG_CRIT, "Failed to create JSON object");
        cJSON_Delete(json);
        cJSON_Delete(array);
        cJSON_Delete(what);
        freeScriptHeader(&header);
        cJSON_Delete(change);
        cJSON_Delete(location_filter);
        return;
    }
    cJSON_AddItemToObject(Equals, "Equals", location_filter);

    cJSON*filter = cJSON_CreateObject();
    if (!filter) {
        syslog(LOG_CRIT, "Failed to create JSON object");
        cJSON_Delete(json);
        cJSON_Delete(array);
        cJSON_Delete(what);
        freeScriptHeader(&header);
        cJSON_Delete(change);
        cJSON_Delete(location_filter);
        cJSON_Delete(Equals);
        return;
    }
    cJSON_AddItemToObject(filter, "onConflict", cJSON_CreateString("REPLACE"));
    cJSON_AddItemToObject(filter, "filter", Equals);
    cJSON_AddItemToObject(filter, "entry", change);
    
    cJSON_AddItemToObject(what, "insertOr", filter);
    cJSON_AddItemToArray(array, what);
    cJSON_AddItemToObject(json, "commands", array);

    const int result = scanner_post_change(json);
    syslog(LOG_INFO, "ccat error: %d.", result);

    cJSON_Delete(json);
    freeScriptHeader(&header);
}

int remove_callback(const char* pathname, const struct stat* statBuffer, int objInfo, struct FTW* ftw)
{
    remove(pathname);
    return 0;
}

void remove_file(const char* path, const char* filename, char* uuid) {
    syslog(LOG_INFO, "Removing file: %s/%s", path, filename);
    char* filePath = malloc(strlen(path) + 1 + strlen(filename) + 1);
    sprintf(filePath, "%s/%s", path, filename);

    char* sdrPath = malloc(strlen(filePath) + strlen(".sdr") + 1);
    sprintf(sdrPath, "%s.sdr", filePath);
    free(filePath);
    
    char* sdrScriptPath = malloc(strlen(sdrPath) + strlen("/script.sh") + 1);
    sprintf(sdrScriptPath, "%s/script.sh", sdrPath);

    FILE* file = fopen(sdrScriptPath, "r");
    if (!file) {
        free(sdrPath);
        free(sdrScriptPath);
        return;
    }

    struct ScriptHeader header;
    readScriptHeader(file, &header);
    fclose(file);

    // If the file is uses hooks, run removal hook
    if (header.useHooks) {
        char* escapedPath = malloc(strlen(sdrScriptPath)*2 + 1);
        int escapedPathLength = 0;
        for (int i=0; i < strlen(sdrScriptPath); i++) {
            if (sdrScriptPath[i] == '"') {
                escapedPath[escapedPathLength++] = '\\';
            }
            escapedPath[escapedPathLength++] = sdrScriptPath[i];
        }
        escapedPath[escapedPathLength] = '\0';
        free(sdrScriptPath);

        syslog(LOG_INFO, "Running remove event");
        const int pid = fork();
        if (pid == 0) {
            char* command = buildCommand("source \"%s\"; on_remove;", escapedPath);
            free(escapedPath);
            execl("/var/local/mkk/su", command, NULL);
        } else {
            free(escapedPath);
            waitpid(pid, NULL, 0);
        }
    }

    // Actually remove the file (and the entry)
    nftw(sdrPath, remove_callback, INT_MAX, FTW_DEPTH);
    free(sdrPath);
    scanner_delete_ccat_entry(uuid);
}

int extractor(const struct scanner_event* event) {
    switch (event->event_type) {
        case SCANNER_ADD:
            index_file(event->path, event->filename);
            break;
        case SCANNER_DELETE:
            remove_file(event->path, event->filename, event->uuid);
            break;
        case SCANNER_UPDATE:
            remove_file(event->path, event->filename, event->uuid); // Remove SDR and entry
            index_file(event->path, event->filename); // Re-index with new metadata and such
            break;
        default:
            // Don't run install hooks and such willy-nilly
            //index_file(event->path, event->filename);
            syslog(LOG_INFO, "Received unknown event: %i", event->event_type);
            break;
    }

    return 0;
}

__attribute__((__visibility__("default"))) int load_extractor(ScannerEventHandler** handler, int *unk1) {
  openlog("com.notmarek.shell_integration.extractor", LOG_PID, LOG_DAEMON);
  *handler = extractor;
  *unk1 = 0;
  return 0;
}
#include "scanner.h"
#include <cstring>
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
#include "cJSON.h"

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

typedef cJSON* (ChangeRequestGenerator)(const char* file_path, const char* uuid);
void index_file(char *path, char* filename) {
    syslog(LOG_INFO, "Indexing file: %s/%s", path, filename);

    char* full_path = malloc(strlen(path) + 1 + strlen(filename));
    full_path[0] = '\0';
    strcat(full_path, path);
    strcat(full_path, "/");
    strcat(full_path, filename);

    // Generate UUID
    char uuid[37] = {0};
    scanner_gen_uuid(uuid, 37);

    // Parse file data
    char* name_string = NULL;
    char* author_string = NULL;
    char* icon_string = NULL;

    char* line = malloc(1024);

    bool useHooks = false;

    // Read data from file header
    FILE* file = fopen(full_path, "r");
    if (file) {
        for (int i=0; i < 6; i++) {
            if (!std::getline(file, line)) {
                break;
            }

            // Start reading the header
            if (line.substr(0, 8) == "# Name: ") {
                name_string = line.substr(8);
            } else if (line.substr(0, 10) == "# Author: ") {
                author_string = line.substr(10);
            } else if (line.substr(0, 8) == "# Icon: ") {
                icon_string = line.substr(8);
            } else if (line.substr(0, 10) == "# UseHooks") {
                useHooks = true;
            }
        }
        fclose(file); // We are done reading the file
    }

    if (icon_string.substr(0, 10) == "data:image" || useHooks) {
        // Create sdr folder
        const std::string sdr_path = full_path.string() + ".sdr";
        std::filesystem::create_directory(sdr_path);

        if (icon_string.substr(0, 10) == "data:image") {
            //data:image/jpeg;base64,iVBORw0KGgoAAAANSUhEUgAAA
            const int fileTypeIndex = icon_string.find('/') + 1;
            const int b64StartIndex = icon_string.find(',') + 1;
            int fileTypeEndIndex = icon_string.find(';');
            if (fileTypeEndIndex == -1) {
                fileTypeEndIndex = b64StartIndex - 1;
            }

            const std::string fileType = icon_string.substr(fileTypeIndex, fileTypeEndIndex - fileTypeIndex);
            const std::string icon_sdr_path = full_path.string() + ".sdr/icon." + fileType;

            // Parse the base64
            std::ofstream file;
            file.open(icon_sdr_path, std::ios::out | std::ios::binary);

            char currentByte = 0;
            int processedBits = 0;
            for (int i=b64StartIndex; i < icon_string.length(); i++) {
                // Convert the base64 character to binary
                char value = 0;
                if (icon_string[i] >= 'A' && icon_string[i] <= 'Z') {
                    value = icon_string[i] - 'A';
                } else if (icon_string[i] >= 'a' && icon_string[i] <= 'z') {
                    value = (icon_string[i] - 'a') + 26;
                } else if (icon_string[i] >= '0' && icon_string[i] <= '9') {
                    value = (icon_string[i] - '0') + 52;
                } else if (icon_string[i] == '+') {
                    value = 62;
                } else if (icon_string[i] == '/') {
                    value = 63;
                } else if (icon_string[i] != '=') {
                    syslog(LOG_WARNING, "Invalid B64 at position %i", i); // Warn
                }

                // Add data to the currentByte
                currentByte |= (value << 2) >> processedBits;
                int consumedBits = std::min(8 - processedBits, 6);
                processedBits += consumedBits;

                if (processedBits >= 8) {
                    file.put(currentByte);
                    processedBits -= 8;

                    if (icon_string[i] == '=') {
                        break; // Terminate on padding
                    }

                    // Set new currentByte to leftover bits
                    currentByte = 0;
                    currentByte |= value << (2 + consumedBits);
                    processedBits = 6 - consumedBits;
                }
            }
            file.close();

            icon_string = icon_sdr_path;
        }

        if (useHooks) {
            // If the file is functional, run install hook
            if (useHooks) {
                std::string escapedPath = full_path.string();
                for (int i=0; i < escapedPath.length(); i++) {
                    if (escapedPath[i] == '"') {
                        escapedPath.insert(i, "\\");
                        i += 1; // Skip character
                    }
                }

                syslog(LOG_INFO, "Running install event");
                const int pid = fork();
                if (pid == 0) {
                    syslog(LOG_INFO, "Executing command: %s", ("source \"" + escapedPath + "\"; on_install;").c_str());
                    execl("/var/local/mkk/su", "su", "-c", ("source \"" + escapedPath + "\"; on_install;").c_str(), NULL);
                } else {
                    waitpid(pid, NULL, 0);
                }

                std::filesystem::copy_file(full_path, sdr_path + "/script.sh"); // We copy the script to sdr folder in case we need 
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

    cJSON* change = generateChangeRequest(full_path, uuid, name_string, author_string, icon_string);
    if (!change) {
        syslog(LOG_CRIT, "Failed to generate a change request...");
        cJSON_Delete(json);
        cJSON_Delete(array);
        cJSON_Delete(what);
        return;
    }

    cJSON* location_filter = cJSON_CreateObject();
    if (!location_filter) {
        syslog(LOG_CRIT, "Failed to create JSON object");
        cJSON_Delete(json);
        cJSON_Delete(array);
        cJSON_Delete(what);
        cJSON_Delete(change);
        return;
    }
    cJSON_AddItemToObject(location_filter, "path", cJSON_CreateString("location"));
    cJSON_AddItemToObject(location_filter, "value", cJSON_CreateString(full_path.c_str()));

    cJSON* Equals = cJSON_CreateObject();
    if (!Equals) {
        syslog(LOG_CRIT, "Failed to create JSON object");
        cJSON_Delete(json);
        cJSON_Delete(array);
        cJSON_Delete(what);
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
}

void remove_file(const char* path, const char* filename, char* uuid) {
    syslog(LOG_INFO, "Removing file: %s/%s", path, filename);
    std::filesystem::path filePath;
    filePath += path;
    filePath += "/";
    filePath += filename;
    std::string sdrScriptPath = filePath.string() + ".sdr/script.sh";

    std::string icon_string;
    bool useHooks = false;

    // Read data from file header
    std::string line;
    std::ifstream file(sdrScriptPath);
    if (file.is_open()) {
        for (int i=0; i < 6; i++) {
            if (!std::getline(file, line)) {
                break;
            }
            
            // Look for Functional flag
            if (line.substr(0, 10) == "# UseHooks") {
                useHooks = true;
            }
        }
        file.close(); // We are done reading the file
    } else {
        syslog(LOG_INFO, "Unable to open file %s", filePath.string().c_str());
    }

    // If the file is uses hooks, run removal hook
    if (useHooks) {
        std::string escapedScriptPath = sdrScriptPath;
        for (int i=0; i < escapedScriptPath.length(); i++) {
            if (escapedScriptPath[i] == '"') { // Who thought this was a good idea???
                escapedScriptPath.insert(i, "\\");
                i += 1; // Skip character
            }
        }

        syslog(LOG_INFO, "Running remove event");
        const int pid = fork();
        if (pid == 0) {
            syslog(LOG_INFO, "Executing command: %s", ("source \"" + escapedScriptPath + "\"; on_remove;").c_str());
            execl("/var/local/mkk/su", "su", "-c", ("source \"" + escapedScriptPath + "\"; on_remove;").c_str(), NULL);
        } else {
            waitpid(pid, NULL, 0);
        }
    }

    // Actually remove the file (and the entry)
    //std::filesystem::remove(filePath); // No this isn't needed
    std::filesystem::remove_all(filePath.string() + ".sdr");
    scanner_delete_ccat_entry(uuid);
}

int extractor(const struct scanner_event* event) {
    std::filesystem::path appPath;

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

extern "C" __attribute__((__visibility__("default"))) int load_extractor(ScannerEventHandler** handler, int *unk1) {
  openlog("com.notmarek.shell_integration.extractor", LOG_PID, LOG_DAEMON);
  *handler = extractor;
  *unk1 = 0;
  return 0;
}
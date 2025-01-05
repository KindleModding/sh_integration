#include "scanner.h"
#include <filesystem>
#include <fstream>
#include <sys/syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include "cJSON.h"

void index_file(char *path, char* filename) {
    syslog(LOG_INFO, "Indexing file: %s/%s", path, filename);

    std::filesystem::path full_path;
    full_path += path;
    full_path += "/";
    full_path += filename;

    // Generate UUID
    char uuid[37] = {0};
    scanner_gen_uuid(uuid, 37);

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

    /**
     * Generate the (huge) change request JSON
     * 
     */
    syslog(LOG_INFO, "Generating change request for %s\n", full_path.c_str());
    std::string name_string;
    std::string author_string;
    std::string icon_string;

    std::string line;

    bool useHooks = false;

    // Read data from file header
    std::ifstream file(full_path);
    if (file.is_open()) {
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
        file.close(); // We are done reading the file
    }

    // If the file uses hooks, run install hook
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

        syslog(LOG_INFO, "Creating sdr folder...");
        std::string sdr_path = full_path.string() + ".sdr";
        std::filesystem::remove_all(sdr_path); // Delete previous sdr folder
        std::filesystem::create_directory(sdr_path); // Create the sdr folder
        std::filesystem::copy(full_path, sdr_path + "/uninstall.sh");
    }

    struct stat st;
    stat(full_path.c_str(), &st);

    cJSON* change = cJSON_CreateObject();
    cJSON_AddItemToObject(json, "uuid", cJSON_CreateString(uuid));
    cJSON_AddItemToObject(json, "location", cJSON_CreateString(full_path.c_str()));
    cJSON_AddItemToObject(json, "type", cJSON_CreateString("Entry:Item"));
    cJSON_AddItemToObject(json, "cdeType", cJSON_CreateString("PDOC"));
    cJSON_AddItemToObject(json, "cdeKey", cJSON_CreateString(getSha1Hash(full_path.c_str())));
    cJSON_AddItemToObject(json, "modificationTime", cJSON_CreateNumber(st.st_mtim.tv_sec));
    cJSON_AddItemToObject(json, "diskUsage", cJSON_CreateNumber(st.st_size));
    cJSON_AddItemToObject(json, "isVisibleInHome", cJSON_CreateTrue());
    cJSON_AddItemToObject(json, "isArchived", cJSON_CreateFalse());
    cJSON_AddItemToObject(json, "mimeType", cJSON_CreateString("text/x-shellscript"));
    cJSON *authors = cJSON_CreateArray();
    cJSON *author = cJSON_CreateObject();
    cJSON_AddItemToObject(author, "kind", cJSON_CreateString("Author"));
    cJSON *author_name = cJSON_CreateObject();
    cJSON_AddItemToObject(author_name, "display", cJSON_CreateString((const char *)(author_string.length() > 0 ? author_string.c_str() : "Unknown")));
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
    if (icon_string.length() > 0) {
        cJSON_AddItemToObject(json, "thumbnail",
                            cJSON_CreateString(icon_string.c_str()));
    }
    cJSON *titles = cJSON_CreateArray();
    cJSON *title = cJSON_CreateObject();
    cJSON_AddItemToObject(
        title, "display",
        cJSON_CreateString((const char *)(name_string.length() > 0 ? name_string.c_str() : full_path.filename().c_str())));
    cJSON_AddItemToArray(titles, title);
    cJSON_AddItemToObject(json, "titles", titles);
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

    std::string sdr_path = filePath.string() + ".sdr";

    // Read data from file header
    bool useHooks = false;
    std::string line;
    std::ifstream file(sdr_path + "/uninstall.sh"); // We use an sdr folder to store a copy of the script as the file is deleted before remove_file is called
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
        syslog(LOG_INFO, "Unable to open file %s", (sdr_path + "/uninstall.sh").c_str());
    }

    // If the file is functional, run install hook
    if (useHooks) {
        std::string escapedPath = filePath.string();
        for (int i=0; i < escapedPath.length(); i++) {
            if (escapedPath[i] == '"') {
                escapedPath.insert(i, "\\");
                i += 1; // Skip character
            }
        }

        syslog(LOG_INFO, "Running remove event");
        const int pid = fork();
        if (pid == 0) {
            syslog(LOG_INFO, "Executing command: %s", ("source \"" + escapedPath + "\"; on_remove;").c_str());
            execl("/var/local/mkk/su", "su", "-c", ("source \"" + escapedPath + "\"; on_remove;").c_str(), NULL);
        } else {
            waitpid(pid, NULL, 0);
        }
    }

    // Actually remove the file (and the entry)
    //std::filesystem::remove(filePath); // This is handled externally, actually
    std::filesystem::remove_all(sdr_path); // Delete sdr folder
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
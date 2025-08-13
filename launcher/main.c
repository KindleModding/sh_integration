#include "lipc.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <stdlib.h>

#define SERVICE_NAME "com.notmarek.shell_integration.launcher"

pid_t app_pid = -1;
bool shouldExit = false;

LIPCcode stub(LIPC* lipc, const char* property, void* value, void* data) {
    syslog(LOG_INFO, "Stub called for \"%s\" with value \"%s\"", property, (char*) value);
    
    const int segment_length = (strchr(value, ':') - (char*) value);
    const int response_size = segment_length + strlen(":0:") + 1;
    char* response = malloc(response_size); // +1 ftw
    memset(response, 0, response_size);
    strncpy(response, value, segment_length);
    strcat(response, ":0:");
    syslog(LOG_INFO, "Replying with %s", response);

    char* target = malloc(strlen(property) + strlen("result") + 1); // +1 bc null termination ofc
    strcpy(target, property);
    strcat(target, "result");
    LipcSetStringProperty(lipc, "com.lab126.appmgrd", target, response);

    free(target);
    free(response);

    return LIPC_OK;
}

LIPCcode pause(LIPC* lipc, const char* property, void* value, void* data) {
    return stub(lipc, property,value, data);
}

LIPCcode unload(LIPC* lipc, const char* property, void* value, void* data) {
    syslog(LOG_INFO, "Unloading shell integration launcher");
    
    // Kill the app if it's running
    if (app_pid > 0) {
        char command[48];
        sprintf(command, "/var/local/mkk/su -c \"kill -9 %i\"", app_pid);
        syslog(LOG_INFO, "Killing with: %s", command);
        system(command);
    }
    
    const LIPCcode result = stub(lipc, property, value, data);
    // Quit app
    shouldExit = true;
    return result;
}

LIPCcode go(LIPC* lipc, const char* property, void* value, void* data) {
    const std::string uri(static_cast<char*>(value));
    std::string rawFilePath = uri.substr(uri.find(':') + 6 + strlen(SERVICE_NAME) + 1);
    const int queryIndex = rawFilePath.find('?');
    if (queryIndex != -1) {
        rawFilePath = rawFilePath.substr(0, queryIndex);
    }

    std::string filePath;

    syslog(LOG_INFO, "Raw path: \"%s\"", rawFilePath.c_str());

    // Parse the filePath as it is urlencoded
    for (size_t i=0; i < rawFilePath.length(); i++) {
        if (rawFilePath[i] == '%') {
            filePath += (char)((rawFilePath[i+1] - '0') * 16 + (rawFilePath[i+2] - '0'));
            i += 2;
        } else {
            filePath += rawFilePath[i];
        }
    }

    bool useFBInk = true;
    bool useHooks = false;
    std::string line;
    std::ifstream file(filePath);
    if (file.is_open()) {
        for (int i=0; i < 6; i++) {
            if (!std::getline(file, line)) {
                break;
            }

            // Start reading the header
            if (line.substr(0, 14) == "# DontUseFBInk") {
                useFBInk = false;
            } else if (line.substr(0, 10) == "# UseHooks") {
                useHooks = true;
            }
        }
        file.close(); // We are done reading the file
    }

    std::string escapedPath = filePath;
    for (int i=0; i < escapedPath.length(); i++) {
        if (escapedPath[i] == '"') {
            escapedPath.insert(i, "\\");
            i += 1; // Skip character
        }
    }

    std::string command = "sh \"" + escapedPath + '"';
    if (useHooks) { // useHooks script - source it and use `on_run`
        command = "sh -c \"source \\\"" + escapedPath + "\\\"; on_run;\"";
    }
    if (useFBInk) {
        command = "/mnt/us/libkh/bin/fbink -k; " + command + " 2>&1 | /mnt/us/libkh/bin/fbink -y 5 -r"; // Send output to fbink
    }

    syslog(LOG_INFO, "Invoking app using \"%s\"", command.c_str());
    // Run the app on a background thread
    app_pid = fork();
    syslog(LOG_INFO, "Our app PID \"%d\"", app_pid);
    if (app_pid == 0) {
        // we are running as framework call gandalf for help
        execl("/var/local/mkk/su", "su", "-c", command.c_str(), NULL); // su is first arg bc it expects to be run from a shell ($1 = name of command, duh)
    }

    return stub(lipc, property, value, data);
}

int main(void) {
    openlog(SERVICE_NAME, LOG_PID, LOG_DAEMON);

    LIPCcode code;
    LIPC* lipc = LipcOpenEx(SERVICE_NAME, &code);
    if (code != LIPC_OK)
        return 1;

    LipcRegisterStringProperty(lipc, "load", NULL, stub, NULL);
    LipcRegisterStringProperty(lipc, "unload", NULL, unload, NULL);
    LipcRegisterStringProperty(lipc, "pause", NULL, pause, NULL);
    LipcRegisterStringProperty(lipc, "go", NULL, go, NULL);
    LipcSetStringProperty(lipc, "com.lab126.appmgrd", "runresult",  "0:" SERVICE_NAME);

    while (!shouldExit) {
        sleep(1);

        if (app_pid > 0) { // This is the parent process AND we have spwned the child
            // Wait for child process to quit
            waitpid(app_pid, NULL, 0);

            app_pid = -1; // So we know the program has quit

            // Calls unload - gracefully exits
            char* value;
            LipcGetStringProperty(lipc, "com.lab126.appmgrd", "popAppHistory", &value);
            LipcFreeString(value);
        }
    }

    syslog(LOG_INFO, "Running exit routine with PID: %d", app_pid);
    LipcClose(lipc);
    return 0;
}
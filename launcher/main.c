#include "openlipc.h"
#include "unistd.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <stdlib.h>
#include "utils.h"

#define SERVICE_NAME "com.notmarek.shell_integration.launcher"

pid_t app_pid = -1;
bool shouldExit = false;


LIPCcode stub(LIPC* lipc, const char* property, void* value, void* data) {
    syslog(LOG_INFO, "Stub called for \"%s\" with value \"%s\"", property, (char*) value);
    
    const int segment_length = (strchr(value, ':') - (char*) value);
    const int response_size = segment_length + strlen(":0:") + 1;
    char* response = malloc(response_size + 1);
    strncpy(response, value, segment_length);
    response[segment_length] = '\0';
    strcat(response, ":0:");
    syslog(LOG_INFO, "Replying with %s", response);

    char* target = malloc(strlen(property) + strlen("result") + 1);
    target[0] = '\0';
    strcat(target, property);
    strcat(target, "result");
    LipcSetStringProperty(lipc, "com.lab126.appmgrd", target, response);

    free(target);
    free(response);

    return LIPC_OK;
}

LIPCcode pause_callback(LIPC* lipc, const char* property, void* value, void* data) {
    return stub(lipc, property,value, data);
}

LIPCcode unload_callback(LIPC* lipc, const char* property, void* value, void* data) {
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

char* getScriptCommand(char* scriptPath)
{
    FILE* file = fopen(scriptPath, "r");
    if (!file) {
        return NULL;
    }

    struct ScriptHeader header;
    readScriptHeader(file, &header);

    char* escapedPath = malloc((strlen(scriptPath) * 2) + 1);
    int escapedPathLength = 0;
    for (int i=0; i < strlen(scriptPath); i++) {
        if (scriptPath[i] == '"') {
            escapedPath[escapedPathLength++] = '\\';
        }
        escapedPath[escapedPathLength++] = scriptPath[i];
    }
    escapedPath[escapedPathLength] = '\0';

    char* command = buildCommand("sh \"%s\"", escapedPath);

    if (header.useHooks) { // useHooks script - source it and use `on_run`
        free(command);
        command = buildCommand("sh -c \"source \\\"%s\\\"; on_run;\"", escapedPath);
    }
    if (header.useFBInk) {
        char* old_command = strdup(command);
        free(command);
        command = buildCommand("/mnt/us/libkh/bin/fbink -k; %s 2>&1 | /mnt/us/libkh/bin/fbink -y 5 -r", old_command);
        free(old_command);
    }

    freeScriptHeader(&header);
    free(escapedPath);
    return command;
}

LIPCcode go_callback(LIPC* lipc, const char* property, void* value, void* data) {
    char* rawFilePath = strchr(value, ':') + 6 + strlen(SERVICE_NAME) + 1;
    char* query = strchr(rawFilePath, '?');
    if (query != NULL) {
        query[0] = 0;
    }

    syslog(LOG_INFO, "Raw path: \"%s\"", rawFilePath);

    // Parse the filePath as it is urlencoded
    char* filePath = malloc(strlen(rawFilePath) + 1); // URLEncoded string will NEVER be longer decoded
    int currentFilepathLen = 0;

    for (size_t i=0; i < strlen(rawFilePath); i++) {
        if (rawFilePath[i] == '%') {
            filePath[currentFilepathLen++] = (char)(((rawFilePath[i+1] - '0') << 4) + (rawFilePath[i+2] - '0'));
            i += 2;
        } else {
            filePath[currentFilepathLen++] = rawFilePath[i];
        }
    }
    filePath[currentFilepathLen] = '\0';
    
    char* command = getScriptCommand(filePath);
    if (command == NULL)
    {
        return stub(lipc, property, value, data);
    }

    syslog(LOG_INFO, "Invoking app using \"%s\"", command);
    utimensat(NULL, filePath, UTIME_NOW, 0);
    // Run the app on a background thread
    app_pid = fork();
    syslog(LOG_INFO, "Our app PID \"%d\"", app_pid);
    if (app_pid == 0) {
        // we are running as framework call gandalf for help
        execl("/var/local/mkk/su", "su", "-c", command, NULL); // su is first arg bc it expects to be run from a shell ($1 = name of command, duh)
    }

    free(filePath);
    free(command);
    return stub(lipc, property, value, data);
}

#ifndef LAUNCHER_TESTING
int main(void) {
    openlog(SERVICE_NAME, LOG_PID, LOG_DAEMON);

    LIPCcode code;
    LIPC* lipc = LipcOpenEx(SERVICE_NAME, &code);
    if (code != LIPC_OK)
        return 1;

    LipcRegisterStringProperty(lipc, "load", NULL, stub, NULL);
    LipcRegisterStringProperty(lipc, "unload", NULL, unload_callback, NULL);
    LipcRegisterStringProperty(lipc, "pause", NULL, pause_callback, NULL);
    LipcRegisterStringProperty(lipc, "go", NULL, go_callback, NULL);
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
#endif
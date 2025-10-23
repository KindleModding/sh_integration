#include "openlipc.h"
#include "unistd.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"

#define SERVICE_NAME "com.notmarek.shell_integration.launcher"

pid_t app_pid = -1;
bool shouldExit = false;

LIPCcode stub(LIPC *lipc, const char *property, void *value, void*) {
    syslog(LOG_INFO, "Stub called for \"%s\" with value \"%s\"", property,
           (char *)value);
    char *id = strtok((char *)value, ":");
    char *response = (char*) malloc(strlen(id) + 3 + 1);
    snprintf(response, strlen(id) + 3 + 1, "%s:0:", id);
    syslog(LOG_INFO, "Replying with %s", response);
    char *target = (char*) malloc(strlen(property) + 6 + 1);
    snprintf(target, strlen(property) + 6 + 1, "%sresult", property);
    syslog(LOG_INFO, "Replying with %s, %s", target, response);
    LipcSetStringProperty(lipc, "com.lab126.appmgrd", target, response);
    free(response);
    free(target);
  
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
    fclose(file);

    char* escapedPath = (char*) malloc((strlen(scriptPath) * 2) + 1);
    int escapedPathLength = 0;
    for (size_t i=0; i < strlen(scriptPath); i++) {
        if (scriptPath[i] == '"') {
            escapedPath[escapedPathLength++] = '\\';
        }
        escapedPath[escapedPathLength++] = scriptPath[i];
    }
    escapedPath[escapedPathLength] = '\0';

    char* command = buildCommand("sh -l \"%s\"", escapedPath);

    if (header.useHooks) { // useHooks script - source it and use `on_run`
        free(command);
        command = buildCommand("sh -l -c \"source \\\"%s\\\"; on_run;\"", escapedPath);
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
    char* rawFilePath = strchr((const char*)value, ':') + 6 + strlen(SERVICE_NAME) + 1;
    char* query = strchr(rawFilePath, '?');
    if (query != NULL) {
        query[0] = 0;
    }

    syslog(LOG_INFO, "Raw path: \"%s\"", rawFilePath);

    // Parse the filePath as it is urlencoded
    char* filePath = urlDecode(rawFilePath);
    
    char* command = getScriptCommand(filePath);
    if (command == NULL)
    {
        return stub(lipc, property, value, data);
    }

    syslog(LOG_INFO, "Invoking app using \"%s\"", command);
    struct timespec time[2] = {{
        .tv_nsec = UTIME_NOW,
        .tv_sec = UTIME_NOW
    }, {
        .tv_nsec = UTIME_NOW,
        .tv_sec = UTIME_NOW
    }};

    // Update the item so it is bought to the front
    LipcSetIntProperty(lipc, "com.lab126.scanner", "doFullScan", 1);
    
    utimensat(0, filePath, time, 0);
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
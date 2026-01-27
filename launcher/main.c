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
#include <syslog.h>

#define SERVICE_NAME "com.notmarek.shell_integration.launcher"

void Log(const char* format, ...)
{
    va_list args;
    va_start (args, format);
    va_list args2;
    va_copy (args2, args);
    int size = vsnprintf (NULL, 0, format, args) + 1;
    char* buffer = malloc((unsigned long) size);
    vsnprintf (buffer, (unsigned long) size, format, args2);
    printf("%s\n", buffer);
    syslog(LOG_INFO, buffer);
    free(buffer);
    va_end (args);
    va_end (args2);
}


pid_t app_pid = -1;
bool shouldExit = false;

LIPCcode stub(LIPC *lipc, const char *property, void *value, void*) {
    Log("Stub called for \"%s\" with value \"%s\"", property,
           (char *)value);
    char* mutValue = strdup(value);
    char *id = strtok((char *)mutValue, ":");
    int bufSize = snprintf(NULL, 0, "%s:0:", id) + 1;
    char *response = (char*) malloc((unsigned long) bufSize + 1);
    snprintf(response, (unsigned long) bufSize, "%s:0:", id);
    Log("Replying with %s", response);
    bufSize = snprintf(NULL, 0, "%sresult", property) + 1;
    char *target = (char*) malloc((unsigned long) bufSize);
    snprintf(target, (unsigned long) bufSize, "%sresult", property);
    Log("Replying with %s, %s", target, response);
    LipcSetStringProperty(lipc, "com.lab126.appmgrd", target, response);
    free(response);
    free(target);
    free(mutValue);
  
    return LIPC_OK;
}

LIPCcode pause_callback(LIPC* lipc, const char* property, void* value, void* data) {
    return stub(lipc, property,value, data);
}

LIPCcode unload_callback(LIPC* lipc, const char* property, void* value, void* data) {
    Log("unload_callback");
    
    // Kill the app if it's running
    if (app_pid > 0) {
        char command[48];
        sprintf(command, "/var/local/mkk/su -c \"kill -9 %i\"", app_pid);
        Log("Killing with: %s", command);
        system(command);
    }
    
    const LIPCcode result = stub(lipc, property, value, data);
    // Quit app
    shouldExit = true;
    return result;
}

char* getScriptCommand(char* scriptPath)
{
    Log("Loading script file");
    FILE* file = fopen(scriptPath, "r");
    if (!file) {
        return NULL;
    }

    Log("Reading header");
    struct ScriptHeader header;
    readScriptHeader(file, &header);
    fclose(file);

    Log("Escaping path");
    char* escapedPath = (char*) malloc((strlen(scriptPath) * 2) + 1);
    int escapedPathLength = 0;
    for (size_t i=0; i < strlen(scriptPath); i++) {
        if (scriptPath[i] == '"') {
            escapedPath[escapedPathLength++] = '\\';
        }
        escapedPath[escapedPathLength++] = scriptPath[i];
    }
    escapedPath[escapedPathLength] = '\0';

    Log("Building command");
    char* command = buildCommand("sh -l \"%s\"", escapedPath);

    if (header.useHooks) { // useHooks script - source it and use `on_run`
        Log("Script uses hooks!");
        free(command);
        command = buildCommand("sh -l -c \"source \\\"%s\\\"; on_run;\"", escapedPath);
    }
    if (header.useFBInk) {
        Log("Script uses FBInk!");
        char* old_command = strdup(command);
        free(command);
        command = buildCommand("/mnt/us/libkh/bin/fbink -k; %s 2>&1 | /mnt/us/libkh/bin/fbink -y 5 -r", old_command);
        free(old_command);
    }

    Log("Finished generating command.");
    freeScriptHeader(&header);
    free(escapedPath);
    return command;
}

LIPCcode go_callback(LIPC* lipc, const char* property, void* value, void* data) {
    Log("go_callback");
    char* rawFilePath = strchr((const char*)value, ':') + 6 + strlen(SERVICE_NAME) + 1;
    char* query = strchr(rawFilePath, '?');
    if (query != NULL) {
        query[0] = 0;
    }

    Log("Raw path: \"%s\"", rawFilePath);

    // Parse the filePath as it is urlencoded
    char* filePath = urlDecode(rawFilePath);
    
    char* command = getScriptCommand(filePath);
    if (command == NULL)
    {
        Log("Could not get script command!");
        free(filePath);
        return stub(lipc, property, value, data);
    }

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
    Log("Invoking app using \"%s\"", command);
    // Run the app on a background thread
    app_pid = fork();
    Log("Our app PID \"%d\"", app_pid);
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
    openlog(SERVICE_NAME, LOG_CONS|LOG_NDELAY|LOG_PID, LOG_USER);
    Log("SH_INTEGRATION LAUNCHER START!");
    LIPCcode code;
    LIPC* lipc = LipcOpenEx(SERVICE_NAME, &code);
    if (code != LIPC_OK)
        return 1;
    
    Log("Registering properties");
    LipcRegisterStringProperty(lipc, "load", NULL, stub, NULL);
    LipcRegisterStringProperty(lipc, "unload", NULL, unload_callback, NULL);
    LipcRegisterStringProperty(lipc, "pause", NULL, pause_callback, NULL);
    LipcRegisterStringProperty(lipc, "go", NULL, go_callback, NULL);
    LipcSetStringProperty(lipc, "com.lab126.appmgrd", "runresult",  "0:" SERVICE_NAME);

    Log("Waiting to exit...");
    while (!shouldExit) {
        sleep(1);

        if (app_pid > 0) { // This is the parent process AND we have spwned the child
            Log("Child spawned, waiting to quit");
            // Wait for child process to quit
            waitpid(app_pid, NULL, 0);

            app_pid = -1; // So we know the program has quit

            Log("Exiting");
            
            #ifdef ARCH_ARMHF
                char* value;
                LipcGetStringProperty(lipc, "com.lab126.appmgrd", "popAppHistory", &value);
                LipcFreeString(value);
            #else
                LipcSetStringProperty(lipc, "com.lab126.appmgrd", "start", "app://com.lab126.booklet.home");
            #endif
        }
    }

    Log("Running exit routine with PID: %d", app_pid);
    LipcClose(lipc);
    closelog();
    return 0;
}
#endif
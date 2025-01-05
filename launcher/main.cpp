#include "openlipc.h"
#include <cerrno>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string>

#define SERVICE_NAME "com.notmarek.shell_integration.launcher"

int childStatus;
pid_t app_pid = -1;
bool shouldExit = false;

LIPCcode stub(LIPC* lipc, const char* property, void* value, void* data) {
    syslog(LOG_INFO, "Stub called for \"%s\" with value \"%s\"", property, static_cast<char*>(value));
    std::string uri(static_cast<char*>(value));

    const int idLoc = uri.find(':');
    std::string response = uri.substr(0, idLoc) + ":0:";
    syslog(LOG_INFO, "Replying with %s", response.c_str());

    std::string target = property;
    target += "result";
    LipcSetStringProperty(lipc, "com.lab126.appmgrd", target.c_str(), response.c_str());

    return LIPC_OK;
}

LIPCcode pause(LIPC* lipc, const char* property, void* value, void* data) {
    return stub(lipc, property,value, data);
}

LIPCcode unload(LIPC* lipc, const char* property, void* value, void* data) {
    syslog(LOG_INFO, "Unloading shell integration launcher");
    
    // Kill the app if it's running
    if (app_pid > 0) {
        const std::string command = ("/var/local/mkk/su -c \"kill -9 " + std::to_string(app_pid) + "\"");
        syslog(LOG_INFO, "Killing with: %s", command.c_str());
        system(command.c_str());
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
    std::string line;
    std::ifstream file(filePath);
    if (file.is_open()) {
        for (int i=0; i < 5; i++) {
            if (!std::getline(file, line)) {
                break;
            }

            // Start reading the header
            if (line.substr(0, 14) == "# DontUseFBInk") {
                useFBInk = false;
            }
        }
        file.close(); // We are done reading the file
    }

    std::string command = "sh \"" + filePath + '"';
    if (useFBInk) {
        command = "/mnt/us/libkh/bin/fbink -k; " + command + " 2>&1 | /mnt/us/libkh/bin/fbink -y 5"; // Send output to fbink
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
            waitpid(app_pid, &childStatus, 0);

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
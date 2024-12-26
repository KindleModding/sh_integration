#include "openlipc.h"
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <sys/syslog.h>
#include <unistd.h>
#include <string>

#define SERVICE_NAME "com.notmarek.shell_integration.launcher"

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

LIPCcode unload(LIPC* lipc, const char* property, void* value, void* data) {
    syslog(LOG_INFO, "Unloading shell integration launcher");
    
    // Kill the app if it's running
    if (app_pid > 0) {
        kill(app_pid, SIGKILL);
    }

    const LIPCcode result = stub(lipc, property, value, data);
    // Quit app
    return result;
}

LIPCcode go(LIPC* lipc, const char* property, void* value, void* data) {
    std::string uri(static_cast<char*>(value));
    // uri format: "2:app://com.notmarek.shell_integration.launcher/mnt/us/documents/run_bridge.sh"
    std::string command = uri.substr(uri.find(':') + 6 + strlen(SERVICE_NAME) + 1);

    syslog(LOG_INFO, "Invoking app using \"%s\"", command.c_str());
    // Run the app on a background thread
    app_pid = fork();
    if (app_pid == 0) {
        // we are runnniang as framework call gandalf for help
        execl("/var/local/mkk/su", "su", "-c", command.c_str(), NULL);
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
    LipcRegisterStringProperty(lipc, "pause", NULL, stub, NULL);
    LipcRegisterStringProperty(lipc, "go", NULL, go, NULL);
    LipcSetStringProperty(lipc, "com.lab126.appmgrd", "runresult",  "0:" SERVICE_NAME);

    while (!shouldExit) {
        sleep(1);

        if (app_pid > 0) { // This is the parent process
            if (kill(app_pid, 0) != 0) {
                // PID does not exist, likely shell has quit
                shouldExit = true;
            };
        }
    }

    // As we exit - read from the appmgr to quit this app
    char* value;
    LipcGetStringProperty(lipc, "com.lab126.appmgrd", "popAppHistory", &value);
    LipcFreeString(value);
    LipcClose(lipc);
    return 0;
}
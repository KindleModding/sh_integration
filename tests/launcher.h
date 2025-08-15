#pragma once

#include "openlipc.h"
#include "unistd.h"
#include <sys/stat.h>
#include <sys/syslog.h>
#include <sys/wait.h>
#include <stdbool.h>
#include "utils.h"

#define SERVICE_NAME "com.notmarek.shell_integration.launcher"

LIPCcode stub(LIPC* lipc, const char* property, void* value, void* data);
LIPCcode pause_callback(LIPC* lipc, const char* property, void* value, void* data);
char* getScriptCommand(char* scriptPath);
LIPCcode go_callback(LIPC* lipc, const char* property, void* value, void* data);
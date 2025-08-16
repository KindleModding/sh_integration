#pragma once

#include <cjson/cJSON.h>
#include <stdbool.h>
#include "scanner.h"

cJSON* generateChangeRequest(cJSON* json, char* filePath, char* uuid, char* name_string, char* author_string, char* icon_string, bool new);
int load_file_extractor(ScannerEventHandler** handler, int *unk1);
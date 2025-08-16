#pragma once

#include <cjson/cJSON.h>
#include "scanner.h"

cJSON* generateChangeRequest(cJSON* json, char* filePath, char* uuid, char* name_string, char* author_string, char* icon_string);
typedef cJSON* (ChangeRequestGenerator)(const char* file_path, const char* uuid);
void index_file(char *path, char* filename);
void remove_file(const char* path, const char* filename, char* uuid);
int extractor(const struct scanner_event* event);
int load_file_extractor(ScannerEventHandler** handler, int *unk1);
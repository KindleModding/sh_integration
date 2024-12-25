#include <cjson/cJSON.h>
#include <libxml2.h>
#include <scanner.h>
#include <stdbool.h>

cJSON *generate_change_request(const char *mimetype, const char *uuid,
                               const char *file_path, int modification_time,
                               int file_size, const char *creator,
                               const char *publisher, const char *book_title,
                               const char *thumbnail_path);

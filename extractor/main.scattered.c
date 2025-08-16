#include "scanner.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>
#include "cjson/cJSON.h"

#include "utils.h"

#define __USE_XOPEN_EXTENDED
#include <ftw.h>

cJSON* generateChangeRequest(cJSON* json, char* filePath, char* uuid, char* name_string, char* author_string, char* icon_string) {
printf("SCATTERGUN_LOG - 21\n");
    syslog(LOG_INFO, "Generating change request for %s\n", filePath);
printf("SCATTERGUN_LOG - 23\n");

printf("SCATTERGUN_LOG - 25\n");
    struct stat st;
printf("SCATTERGUN_LOG - 27\n");
    stat(filePath, &st);
printf("SCATTERGUN_LOG - 29\n");

printf("SCATTERGUN_LOG - 31\n");
    cJSON *authors = cJSON_CreateArray();
printf("SCATTERGUN_LOG - 33\n");
    cJSON *author = cJSON_CreateObject();
printf("SCATTERGUN_LOG - 35\n");
    cJSON *author_name = cJSON_CreateObject();
printf("SCATTERGUN_LOG - 37\n");
    cJSON *refs = cJSON_CreateArray();
printf("SCATTERGUN_LOG - 39\n");
    cJSON *titles_ref = cJSON_CreateObject();
printf("SCATTERGUN_LOG - 41\n");
    cJSON *authors_ref = cJSON_CreateObject();
printf("SCATTERGUN_LOG - 43\n");
    cJSON *titles = cJSON_CreateArray();
printf("SCATTERGUN_LOG - 45\n");
    cJSON *title = cJSON_CreateObject();
printf("SCATTERGUN_LOG - 47\n");

printf("SCATTERGUN_LOG - 49\n");

printf("SCATTERGUN_LOG - 51\n");
    cJSON_AddItemToObject(json, "uuid", cJSON_CreateString(uuid));
printf("SCATTERGUN_LOG - 53\n");
    cJSON_AddItemToObject(json, "location", cJSON_CreateString(filePath));
printf("SCATTERGUN_LOG - 55\n");
    cJSON_AddItemToObject(json, "type", cJSON_CreateString("Entry:Item"));
printf("SCATTERGUN_LOG - 57\n");
    cJSON_AddItemToObject(json, "cdeType", cJSON_CreateString("PDOC"));
printf("SCATTERGUN_LOG - 59\n");
    cJSON_AddItemToObject(json, "cdeKey", cJSON_CreateString(getSha1Hash(filePath)));
printf("SCATTERGUN_LOG - 61\n");
    cJSON_AddItemToObject(json, "modificationTime", cJSON_CreateNumber(st.st_mtim.tv_sec));
printf("SCATTERGUN_LOG - 63\n");
    cJSON_AddItemToObject(json, "diskUsage", cJSON_CreateNumber(st.st_size));
printf("SCATTERGUN_LOG - 65\n");
    cJSON_AddItemToObject(json, "isVisibleInHome", cJSON_CreateTrue());
printf("SCATTERGUN_LOG - 67\n");
    cJSON_AddItemToObject(json, "isArchived", cJSON_CreateFalse());
printf("SCATTERGUN_LOG - 69\n");
    cJSON_AddItemToObject(json, "mimeType", cJSON_CreateString("text/x-shellscript"));
printf("SCATTERGUN_LOG - 71\n");
    cJSON_AddItemToObject(json, "displayObjects", refs);
printf("SCATTERGUN_LOG - 73\n");
    cJSON_AddItemToObject(json, "credits", authors);
printf("SCATTERGUN_LOG - 75\n");
    cJSON_AddItemToObject(json, "publisher", cJSON_CreateString("KMC"));
printf("SCATTERGUN_LOG - 77\n");
    const char *tags[] = {"NEW"};
printf("SCATTERGUN_LOG - 79\n");
    cJSON_AddItemToObject(json, "displayTags", cJSON_CreateStringArray(tags, 1));
printf("SCATTERGUN_LOG - 81\n");
    if (icon_string != NULL) {
printf("SCATTERGUN_LOG - 83\n");
        cJSON_AddItemToObject(json, "thumbnail",
                            cJSON_CreateString(icon_string));
printf("SCATTERGUN_LOG - 86\n");
    }
printf("SCATTERGUN_LOG - 88\n");
    cJSON_AddItemToObject(json, "titles", titles);
printf("SCATTERGUN_LOG - 90\n");

printf("SCATTERGUN_LOG - 92\n");
    cJSON_AddItemToArray(refs, titles_ref);
printf("SCATTERGUN_LOG - 94\n");
    cJSON_AddItemToArray(refs, authors_ref);
printf("SCATTERGUN_LOG - 96\n");
    cJSON_AddItemToObject(titles_ref, "ref", cJSON_CreateString("titles"));
printf("SCATTERGUN_LOG - 98\n");
    cJSON_AddItemToObject(authors_ref, "ref", cJSON_CreateString("credits"));
printf("SCATTERGUN_LOG - 100\n");

printf("SCATTERGUN_LOG - 102\n");
    cJSON_AddItemToArray(authors, author);
printf("SCATTERGUN_LOG - 104\n");
    cJSON_AddItemToObject(author, "kind", cJSON_CreateString("Author"));
printf("SCATTERGUN_LOG - 106\n");
    cJSON_AddItemToObject(author, "name", author_name);
printf("SCATTERGUN_LOG - 108\n");
    cJSON_AddItemToObject(author_name, "display", cJSON_CreateString((const char *)(author_string != NULL ? author_string : "Unknown")));
printf("SCATTERGUN_LOG - 110\n");
    
printf("SCATTERGUN_LOG - 112\n");
    cJSON_AddItemToArray(titles, title);
printf("SCATTERGUN_LOG - 114\n");
    cJSON_AddItemToObject(
        title, "display",
        cJSON_CreateString((const char *)(name_string != NULL ? name_string : basename(filePath))));
printf("SCATTERGUN_LOG - 118\n");

printf("SCATTERGUN_LOG - 120\n");
    return json;
printf("SCATTERGUN_LOG - 122\n");
}

typedef cJSON* (ChangeRequestGenerator)(const char* file_path, const char* uuid);
void index_file(char *path, char* filename) {
printf("SCATTERGUN_LOG - 127\n");
    printf("Indexing file: %s/%s\n", path, filename);
printf("SCATTERGUN_LOG - 129\n");
    syslog(LOG_INFO, "Indexing file: %s/%s", path, filename);
printf("SCATTERGUN_LOG - 131\n");

printf("SCATTERGUN_LOG - 133\n");
    char* full_path = malloc(strlen(path) + 1 + strlen(filename) + 1);
printf("SCATTERGUN_LOG - 135\n");
    sprintf(full_path, "%s/%s", path, filename);
printf("SCATTERGUN_LOG - 137\n");

printf("SCATTERGUN_LOG - 139\n");
    // Generate UUID
printf("SCATTERGUN_LOG - 141\n");
    char uuid[37] = {0};
printf("SCATTERGUN_LOG - 143\n");
    scanner_gen_uuid(uuid, 37);
printf("SCATTERGUN_LOG - 145\n");

printf("SCATTERGUN_LOG - 147\n");
    // Read data from file header
printf("SCATTERGUN_LOG - 149\n");
    FILE* file = fopen(full_path, "r");
printf("SCATTERGUN_LOG - 151\n");
    if (!file) {
printf("SCATTERGUN_LOG - 153\n");
        free(full_path);
printf("SCATTERGUN_LOG - 155\n");
        return;
printf("SCATTERGUN_LOG - 157\n");
    }
printf("SCATTERGUN_LOG - 159\n");

printf("SCATTERGUN_LOG - 161\n");
    struct ScriptHeader header;
printf("SCATTERGUN_LOG - 163\n");
    readScriptHeader(file, &header);
printf("SCATTERGUN_LOG - 165\n");
    fclose(file);
printf("SCATTERGUN_LOG - 167\n");

printf("SCATTERGUN_LOG - 169\n");
    bool validIcon = header.icon != NULL && strncmp(header.icon, "data:image", strlen("data:image")) == 0;
printf("SCATTERGUN_LOG - 171\n");
    if (validIcon || header.useHooks) {
printf("SCATTERGUN_LOG - 173\n");
        // Create sdr folder
printf("SCATTERGUN_LOG - 175\n");
        char* sdr_path = malloc(strlen(full_path) + strlen(".sdr") + 1);
printf("SCATTERGUN_LOG - 177\n");
        sprintf(sdr_path, "%s.sdr", full_path);
printf("SCATTERGUN_LOG - 179\n");
        mkdir(sdr_path, 0755);
printf("SCATTERGUN_LOG - 181\n");

printf("SCATTERGUN_LOG - 183\n");
        if (validIcon) {
printf("SCATTERGUN_LOG - 185\n");
            //data:image/jpeg;base64,iVBORw0KGgoAAAANSUhEUgAAA
printf("SCATTERGUN_LOG - 187\n");
            char* fileTypePointer = strchr(header.icon, '/')+1;
printf("SCATTERGUN_LOG - 189\n");
            char* fileTypeEndPointer = strchr(header.icon, ';');
printf("SCATTERGUN_LOG - 191\n");
            char* b64Pointer = strchr(header.icon, ',')+1;
printf("SCATTERGUN_LOG - 193\n");
            if (fileTypeEndPointer == NULL)
printf("SCATTERGUN_LOG - 195\n");
            {
printf("SCATTERGUN_LOG - 197\n");
                fileTypeEndPointer = b64Pointer;
printf("SCATTERGUN_LOG - 199\n");
            }
printf("SCATTERGUN_LOG - 201\n");

printf("SCATTERGUN_LOG - 203\n");
            char* fileType = malloc(fileTypeEndPointer - fileTypePointer + 1);
printf("SCATTERGUN_LOG - 205\n");
            strncpy(fileType, fileTypePointer, fileTypeEndPointer - fileTypePointer);
printf("SCATTERGUN_LOG - 207\n");
            fileType[fileTypeEndPointer - fileTypePointer] = '\0';
printf("SCATTERGUN_LOG - 209\n");

printf("SCATTERGUN_LOG - 211\n");
            char* icon_sdr_path = malloc(strlen(sdr_path) + strlen("/icon.") + strlen(fileType) + 1);
printf("SCATTERGUN_LOG - 213\n");
            sprintf(icon_sdr_path, "%s/icon.%s", sdr_path, fileType);
printf("SCATTERGUN_LOG - 215\n");

printf("SCATTERGUN_LOG - 217\n");
            // Parse the base64
printf("SCATTERGUN_LOG - 219\n");
            FILE* file = fopen(icon_sdr_path, "wb");
printf("SCATTERGUN_LOG - 221\n");

printf("SCATTERGUN_LOG - 223\n");
            char currentByte = 0;
printf("SCATTERGUN_LOG - 225\n");
            int processedBits = 0;
printf("SCATTERGUN_LOG - 227\n");
            for (int i=(b64Pointer - header.icon); i < strlen(header.icon); i++) {
printf("SCATTERGUN_LOG - 229\n");
                // Convert the base64 character to binary
printf("SCATTERGUN_LOG - 231\n");
                char value = 0;
printf("SCATTERGUN_LOG - 233\n");
                if (header.icon[i] >= 'A' && header.icon[i] <= 'Z') {
printf("SCATTERGUN_LOG - 235\n");
                    value = header.icon[i] - 'A';
printf("SCATTERGUN_LOG - 237\n");
                } else if (header.icon[i] >= 'a' && header.icon[i] <= 'z') {
printf("SCATTERGUN_LOG - 239\n");
                    value = (header.icon[i] - 'a') + 26;
printf("SCATTERGUN_LOG - 241\n");
                } else if (header.icon[i] >= '0' && header.icon[i] <= '9') {
printf("SCATTERGUN_LOG - 243\n");
                    value = (header.icon[i] - '0') + 52;
printf("SCATTERGUN_LOG - 245\n");
                } else if (header.icon[i] == '+') {
printf("SCATTERGUN_LOG - 247\n");
                    value = 62;
printf("SCATTERGUN_LOG - 249\n");
                } else if (header.icon[i] == '/') {
printf("SCATTERGUN_LOG - 251\n");
                    value = 63;
printf("SCATTERGUN_LOG - 253\n");
                } else if (header.icon[i] != '=') {
printf("SCATTERGUN_LOG - 255\n");
                    syslog(LOG_WARNING, "Invalid B64 at position %i", i); // Warn
printf("SCATTERGUN_LOG - 257\n");
                }
printf("SCATTERGUN_LOG - 259\n");

printf("SCATTERGUN_LOG - 261\n");
                // Add data to the currentByte
printf("SCATTERGUN_LOG - 263\n");
                currentByte |= (value << 2) >> processedBits; // Shift back by two because base64 chars only represent 6 bits
printf("SCATTERGUN_LOG - 265\n");
                int consumedBits = (8 - processedBits) < 6 ? 8 - processedBits : 6; // We process a maximum of 6 bits at once
printf("SCATTERGUN_LOG - 267\n");
                processedBits += consumedBits;
printf("SCATTERGUN_LOG - 269\n");

printf("SCATTERGUN_LOG - 271\n");
                if (processedBits >= 8) {
printf("SCATTERGUN_LOG - 273\n");
                    if (header.icon[i] == '=')
printf("SCATTERGUN_LOG - 275\n");
                    {
printf("SCATTERGUN_LOG - 277\n");
                        break; // We terminate on padding since the last bits needed are already 0
printf("SCATTERGUN_LOG - 279\n");
                    }
printf("SCATTERGUN_LOG - 281\n");

printf("SCATTERGUN_LOG - 283\n");
                    putc(currentByte, file);
printf("SCATTERGUN_LOG - 285\n");
                    processedBits -= 8;
printf("SCATTERGUN_LOG - 287\n");

printf("SCATTERGUN_LOG - 289\n");
                    // Set new currentByte to leftover bits
printf("SCATTERGUN_LOG - 291\n");
                    currentByte = 0;
printf("SCATTERGUN_LOG - 293\n");
                    currentByte |= value << (2 + consumedBits);
printf("SCATTERGUN_LOG - 295\n");
                    processedBits = 6 - consumedBits;
printf("SCATTERGUN_LOG - 297\n");
                }
printf("SCATTERGUN_LOG - 299\n");
            }
printf("SCATTERGUN_LOG - 301\n");
            fclose(file);
printf("SCATTERGUN_LOG - 303\n");

printf("SCATTERGUN_LOG - 305\n");
            free(header.icon);
printf("SCATTERGUN_LOG - 307\n");
            header.icon = strdup(icon_sdr_path);//realloc(header.icon, strlen(icon_sdr_path) + 1);
printf("SCATTERGUN_LOG - 309\n");
            free(icon_sdr_path);
printf("SCATTERGUN_LOG - 311\n");
            free(fileType);
printf("SCATTERGUN_LOG - 313\n");
        }
printf("SCATTERGUN_LOG - 315\n");

printf("SCATTERGUN_LOG - 317\n");
        if (header.useHooks) {
printf("SCATTERGUN_LOG - 319\n");
            // If the file is functional, run install hook
printf("SCATTERGUN_LOG - 321\n");
            char* escapedPath = malloc(strlen(full_path)*2 + 1);
printf("SCATTERGUN_LOG - 323\n");
            int escapedPathLength = 0;
printf("SCATTERGUN_LOG - 325\n");
            for (int i=0; i < strlen(full_path); i++) {
printf("SCATTERGUN_LOG - 327\n");
                if (full_path[i] == '"') {
printf("SCATTERGUN_LOG - 329\n");
                    escapedPath[escapedPathLength++] = '\\';
printf("SCATTERGUN_LOG - 331\n");
                }
printf("SCATTERGUN_LOG - 333\n");
                escapedPath[escapedPathLength++] = full_path[i];
printf("SCATTERGUN_LOG - 335\n");
            }
printf("SCATTERGUN_LOG - 337\n");
            escapedPath[escapedPathLength] = '\0';
printf("SCATTERGUN_LOG - 339\n");

printf("SCATTERGUN_LOG - 341\n");
            syslog(LOG_INFO, "Running install event");
printf("SCATTERGUN_LOG - 343\n");
            const int pid = fork();
printf("SCATTERGUN_LOG - 345\n");
            if (pid == 0) {
printf("SCATTERGUN_LOG - 347\n");
                char* command = malloc(escapedPathLength + 32);
printf("SCATTERGUN_LOG - 349\n");
                sprintf(command, "source \"%s\"; on_install;", escapedPath);
printf("SCATTERGUN_LOG - 351\n");
                syslog(LOG_INFO, "Executing command: %s", command);
printf("SCATTERGUN_LOG - 353\n");
                execl("/var/local/mkk/su", "su", "-c", command, NULL);
printf("SCATTERGUN_LOG - 355\n");
            } else {
printf("SCATTERGUN_LOG - 357\n");
                waitpid(pid, NULL, 0);
printf("SCATTERGUN_LOG - 359\n");
            }
printf("SCATTERGUN_LOG - 361\n");

printf("SCATTERGUN_LOG - 363\n");
            free(escapedPath);
printf("SCATTERGUN_LOG - 365\n");

printf("SCATTERGUN_LOG - 367\n");
            char* sdrFilePath = malloc(strlen(sdr_path) + strlen("/script.sh") + 1);
printf("SCATTERGUN_LOG - 369\n");
            sprintf(sdrFilePath, "%s/%s", sdr_path, "/script.sh");
printf("SCATTERGUN_LOG - 371\n");
            printf("Writing script to %s\n", sdrFilePath);
printf("SCATTERGUN_LOG - 373\n");
            FILE* scriptFile = fopen(full_path, "r");
printf("SCATTERGUN_LOG - 375\n");
            FILE* sdrFile = fopen(sdrFilePath, "w");
printf("SCATTERGUN_LOG - 377\n");
            char c;
printf("SCATTERGUN_LOG - 379\n");
            while ((c = getc(scriptFile)) != EOF) {
printf("SCATTERGUN_LOG - 381\n");
                putc(c, sdrFile);
printf("SCATTERGUN_LOG - 383\n");
            }
printf("SCATTERGUN_LOG - 385\n");
            fclose(scriptFile);
printf("SCATTERGUN_LOG - 387\n");
            fclose(sdrFile);
printf("SCATTERGUN_LOG - 389\n");
            free(sdrFilePath);
printf("SCATTERGUN_LOG - 391\n");
        }
printf("SCATTERGUN_LOG - 393\n");

printf("SCATTERGUN_LOG - 395\n");
        free(sdr_path);
printf("SCATTERGUN_LOG - 397\n");
    }
printf("SCATTERGUN_LOG - 399\n");

printf("SCATTERGUN_LOG - 401\n");

printf("SCATTERGUN_LOG - 403\n");
    // Create JSON objects
printf("SCATTERGUN_LOG - 405\n");
    cJSON* json = cJSON_CreateObject();
printf("SCATTERGUN_LOG - 407\n");
    if (!json) {
printf("SCATTERGUN_LOG - 409\n");
        fprintf(stderr, "Failed to create a JSON object");
printf("SCATTERGUN_LOG - 411\n");
        return;
printf("SCATTERGUN_LOG - 413\n");
    }
printf("SCATTERGUN_LOG - 415\n");

printf("SCATTERGUN_LOG - 417\n");
    cJSON* array = cJSON_CreateArray();
printf("SCATTERGUN_LOG - 419\n");
    cJSON* what = cJSON_CreateObject();
printf("SCATTERGUN_LOG - 421\n");
    cJSON* location_filter = cJSON_CreateObject();
printf("SCATTERGUN_LOG - 423\n");
    cJSON* Equals = cJSON_CreateObject();
printf("SCATTERGUN_LOG - 425\n");
    cJSON* filter = cJSON_CreateObject();
printf("SCATTERGUN_LOG - 427\n");
    cJSON* change = cJSON_CreateObject();
printf("SCATTERGUN_LOG - 429\n");

printf("SCATTERGUN_LOG - 431\n");
    cJSON_AddItemToObject(json, "type", cJSON_CreateString("ChangeRequest"));
printf("SCATTERGUN_LOG - 433\n");
    cJSON_AddItemToObject(json, "commands", array);
printf("SCATTERGUN_LOG - 435\n");
    cJSON_AddItemToArray(array, what);
printf("SCATTERGUN_LOG - 437\n");
    cJSON_AddItemToObject(what, "insertOr", filter);
printf("SCATTERGUN_LOG - 439\n");
    cJSON_AddItemToObject(filter, "filter", Equals);
printf("SCATTERGUN_LOG - 441\n");
    cJSON_AddItemToObject(filter, "onConflict", cJSON_CreateString("REPLACE"));
printf("SCATTERGUN_LOG - 443\n");
    cJSON_AddItemToObject(filter, "entry", change);
printf("SCATTERGUN_LOG - 445\n");
    generateChangeRequest(change, full_path, uuid, header.name, header.author, header.icon);
printf("SCATTERGUN_LOG - 447\n");
    cJSON_AddItemToObject(Equals, "Equals", location_filter);
printf("SCATTERGUN_LOG - 449\n");
    cJSON_AddItemToObject(location_filter, "path", cJSON_CreateString("location"));
printf("SCATTERGUN_LOG - 451\n");
    cJSON_AddItemToObject(location_filter, "value", cJSON_CreateString(full_path));
printf("SCATTERGUN_LOG - 453\n");

printf("SCATTERGUN_LOG - 455\n");

printf("SCATTERGUN_LOG - 457\n");
    
printf("SCATTERGUN_LOG - 459\n");

printf("SCATTERGUN_LOG - 461\n");
    const int result = scanner_post_change(json);
printf("SCATTERGUN_LOG - 463\n");
    syslog(LOG_INFO, "ccat error: %d.", result);
printf("SCATTERGUN_LOG - 465\n");
    //printf("Json: %s\n", cJSON_Print(json));
printf("SCATTERGUN_LOG - 467\n");

printf("SCATTERGUN_LOG - 469\n");
    /*if (json)
printf("SCATTERGUN_LOG - 471\n");
    {
printf("SCATTERGUN_LOG - 473\n");
        cJSON_Delete(json);
printf("SCATTERGUN_LOG - 475\n");
    }*/
printf("SCATTERGUN_LOG - 477\n");
    // Can you believe that cJSON deleting causes issues
printf("SCATTERGUN_LOG - 479\n");
    freeScriptHeader(&header);
printf("SCATTERGUN_LOG - 481\n");
    free(full_path);
printf("SCATTERGUN_LOG - 483\n");
}

void remove_file(const char* path, const char* filename, char* uuid) {
printf("SCATTERGUN_LOG - 487\n");
    syslog(LOG_INFO, "Removing file: %s/%s", path, filename);
printf("SCATTERGUN_LOG - 489\n");
    printf("Removing file: %s/%s\n", path, filename);
printf("SCATTERGUN_LOG - 491\n");
    char* filePath = malloc(strlen(path) + 1 + strlen(filename) + 1);
printf("SCATTERGUN_LOG - 493\n");
    sprintf(filePath, "%s/%s", path, filename);
printf("SCATTERGUN_LOG - 495\n");

printf("SCATTERGUN_LOG - 497\n");
    char* sdrPath = malloc(strlen(filePath) + strlen(".sdr") + 1);
printf("SCATTERGUN_LOG - 499\n");
    sprintf(sdrPath, "%s.sdr", filePath);
printf("SCATTERGUN_LOG - 501\n");
    free(filePath);
printf("SCATTERGUN_LOG - 503\n");
    
printf("SCATTERGUN_LOG - 505\n");
    char* sdrScriptPath = malloc(strlen(sdrPath) + strlen("/script.sh") + 1);
printf("SCATTERGUN_LOG - 507\n");
    sprintf(sdrScriptPath, "%s/script.sh", sdrPath);
printf("SCATTERGUN_LOG - 509\n");
    
printf("SCATTERGUN_LOG - 511\n");
    FILE* file = fopen(sdrScriptPath, "r");
printf("SCATTERGUN_LOG - 513\n");
    if (file) {
printf("SCATTERGUN_LOG - 515\n");
        struct ScriptHeader header;
printf("SCATTERGUN_LOG - 517\n");
        readScriptHeader(file, &header);
printf("SCATTERGUN_LOG - 519\n");
        fclose(file);
printf("SCATTERGUN_LOG - 521\n");

printf("SCATTERGUN_LOG - 523\n");
        // If the file is uses hooks, run removal hook
printf("SCATTERGUN_LOG - 525\n");
        if (header.useHooks) {
printf("SCATTERGUN_LOG - 527\n");
            char* escapedPath = malloc(strlen(sdrScriptPath)*2 + 1);
printf("SCATTERGUN_LOG - 529\n");
            int escapedPathLength = 0;
printf("SCATTERGUN_LOG - 531\n");
            for (int i=0; i < strlen(sdrScriptPath); i++) {
printf("SCATTERGUN_LOG - 533\n");
                if (sdrScriptPath[i] == '"') {
printf("SCATTERGUN_LOG - 535\n");
                    escapedPath[escapedPathLength++] = '\\';
printf("SCATTERGUN_LOG - 537\n");
                }
printf("SCATTERGUN_LOG - 539\n");
                escapedPath[escapedPathLength++] = sdrScriptPath[i];
printf("SCATTERGUN_LOG - 541\n");
            }
printf("SCATTERGUN_LOG - 543\n");
            escapedPath[escapedPathLength] = '\0';
printf("SCATTERGUN_LOG - 545\n");
            free(sdrScriptPath);
printf("SCATTERGUN_LOG - 547\n");

printf("SCATTERGUN_LOG - 549\n");
            syslog(LOG_INFO, "Running remove event");
printf("SCATTERGUN_LOG - 551\n");
            const int pid = fork();
printf("SCATTERGUN_LOG - 553\n");
            if (pid == 0) {
printf("SCATTERGUN_LOG - 555\n");
                char* command = buildCommand("source \"%s\"; on_remove;", escapedPath);
printf("SCATTERGUN_LOG - 557\n");
                free(escapedPath);
printf("SCATTERGUN_LOG - 559\n");
                execl("/var/local/mkk/su", command, NULL);
printf("SCATTERGUN_LOG - 561\n");
            } else {
printf("SCATTERGUN_LOG - 563\n");
                free(escapedPath);
printf("SCATTERGUN_LOG - 565\n");
                waitpid(pid, NULL, 0);
printf("SCATTERGUN_LOG - 567\n");
            }
printf("SCATTERGUN_LOG - 569\n");
        }
printf("SCATTERGUN_LOG - 571\n");
        printf("Removing: %s\n", sdrPath);
printf("SCATTERGUN_LOG - 573\n");

printf("SCATTERGUN_LOG - 575\n");
        if (access(sdrPath, R_OK|W_OK) == F_OK)
printf("SCATTERGUN_LOG - 577\n");
        {
printf("SCATTERGUN_LOG - 579\n");
            recursiveDelete(sdrPath);
printf("SCATTERGUN_LOG - 581\n");
        }
printf("SCATTERGUN_LOG - 583\n");
    }
printf("SCATTERGUN_LOG - 585\n");
    free(sdrPath);
printf("SCATTERGUN_LOG - 587\n");
    free(sdrScriptPath);
printf("SCATTERGUN_LOG - 589\n");
    scanner_delete_ccat_entry(uuid);
printf("SCATTERGUN_LOG - 591\n");
}

int extractor(const struct scanner_event* event) {
    switch (event->event_type) {
printf("SCATTERGUN_LOG - 596\n");
        case SCANNER_ADD:
printf("SCATTERGUN_LOG - 598\n");
            index_file(event->path, event->filename);
printf("SCATTERGUN_LOG - 600\n");
            break;
printf("SCATTERGUN_LOG - 602\n");
        case SCANNER_DELETE:
printf("SCATTERGUN_LOG - 604\n");
            remove_file(event->path, event->filename, event->uuid);
printf("SCATTERGUN_LOG - 606\n");
            break;
printf("SCATTERGUN_LOG - 608\n");
        case SCANNER_UPDATE:
printf("SCATTERGUN_LOG - 610\n");
            remove_file(event->path, event->filename, event->uuid); // Remove SDR and entry
printf("SCATTERGUN_LOG - 612\n");
            index_file(event->path, event->filename); // Re-index with new metadata and such
printf("SCATTERGUN_LOG - 614\n");
            break;
printf("SCATTERGUN_LOG - 616\n");
        default:
printf("SCATTERGUN_LOG - 618\n");
            // Don't run install hooks and such willy-nilly
printf("SCATTERGUN_LOG - 620\n");
            //index_file(event->path, event->filename);
printf("SCATTERGUN_LOG - 622\n");
            syslog(LOG_INFO, "Received unknown event: %i", event->event_type);
printf("SCATTERGUN_LOG - 624\n");
            break;
printf("SCATTERGUN_LOG - 626\n");
    }

    return 0;
}

__attribute__((__visibility__("default"))) int load_file_extractor(ScannerEventHandler** handler, int *unk1) {
printf("SCATTERGUN_LOG - 633\n");
  openlog("com.notmarek.shell_integration.extractor", LOG_PID, LOG_DAEMON);
printf("SCATTERGUN_LOG - 635\n");
  *handler = extractor;
printf("SCATTERGUN_LOG - 637\n");
  *unk1 = 0;
printf("SCATTERGUN_LOG - 639\n");
  return 0;
printf("SCATTERGUN_LOG - 641\n");
}
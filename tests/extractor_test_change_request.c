#include "extractor.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main()
{
    char uuid[37];
    scanner_gen_uuid(uuid, 37);
    cJSON* changeRequest = cJSON_CreateObject();
    generateChangeRequest(changeRequest, "./tests/test_hooks.sh", uuid, "TestName", "TestAuthor", "TestIcon", false);
    fprintf(stderr, "Testing Normal\n%s", cJSON_Print(changeRequest));

    fprintf(stderr, "Checking uuid...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "commands"), 0), "insert"), "uuid")), uuid) == 0);
    fprintf(stderr, "Checking path...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "commands"), 0), "insert"), "location")), "./tests/test_hooks.sh") == 0);
    fprintf(stderr, "Checking thumbnail...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "commands"), 0), "insert"), "thumbnail")), "TestIcon") == 0);
    fprintf(stderr, "Checking author...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "commands"), 0), "insert"), "credits"), 0), "name"), "display")), "TestAuthor") == 0);
    fprintf(stderr, "Checking title...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "commands"), 0), "insert"), "titles"), 0), "display")), "TestName") == 0);

    cJSON_Delete(changeRequest);
    changeRequest = cJSON_CreateObject();
    generateChangeRequest(changeRequest, "./tests/test_hooks.sh", uuid, NULL, "TestAuthor", "TestIcon", false);
    fprintf(stderr, "Testing NULL name\n%s", cJSON_Print(changeRequest));

    fprintf(stderr, "Checking uuid...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "commands"), 0), "insert"), "uuid")), uuid) == 0);
    fprintf(stderr, "Checking path...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "commands"), 0), "insert"), "location")), "./tests/test_hooks.sh") == 0);
    fprintf(stderr, "Checking thumbnail...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "commands"), 0), "insert"), "thumbnail")), "TestIcon") == 0);
    fprintf(stderr, "Checking author...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "commands"), 0), "insert"), "credits"), 0), "name"), "display")), "TestAuthor") == 0);
    fprintf(stderr, "Checking title...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "commands"), 0), "insert"), "titles"), 0), "display")), "test_hooks.sh") == 0);

    cJSON_Delete(changeRequest);
    changeRequest = cJSON_CreateObject();
    generateChangeRequest(changeRequest, "./tests/test_hooks.sh", uuid, NULL, NULL, "TestIcon", false);
    fprintf(stderr, "Testing NULL author\n%s", cJSON_Print(changeRequest));

    fprintf(stderr, "Checking uuid...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "commands"), 0), "insert"), "uuid")), uuid) == 0);
    fprintf(stderr, "Checking path...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "commands"), 0), "insert"), "location")), "./tests/test_hooks.sh") == 0);
    fprintf(stderr, "Checking thumbnail...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "commands"), 0), "insert"), "thumbnail")), "TestIcon") == 0);
    fprintf(stderr, "Checking author...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "commands"), 0), "insert"), "credits"), 0), "name"), "display")), "Unknown") == 0);
    fprintf(stderr, "Checking title...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "commands"), 0), "insert"), "titles"), 0), "display")), "test_hooks.sh") == 0);
    fprintf(stderr, "Checking tags...\n");
    assert(!cJSON_HasObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "commands"), 0), "insert"), "displayTags"));

    cJSON_Delete(changeRequest);
    changeRequest = cJSON_CreateObject();
    generateChangeRequest(changeRequest, "./tests/test_hooks.sh", uuid, NULL, NULL, NULL, true);
    fprintf(stderr, "Testing NULL icon\n%s", cJSON_Print(changeRequest));

    fprintf(stderr, "Checking uuid...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "commands"), 0), "insert"), "uuid")), uuid) == 0);
    fprintf(stderr, "Checking path...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "commands"), 0), "insert"), "location")), "./tests/test_hooks.sh") == 0);
    fprintf(stderr, "Checking thumbnail...\n");
    assert(!cJSON_HasObjectItem(changeRequest, "thumbnail"));
    fprintf(stderr, "Checking author...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "commands"), 0), "insert"), "credits"), 0), "name"), "display")), "Unknown") == 0);
    fprintf(stderr, "Checking title...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "commands"), 0), "insert"), "titles"), 0), "display")), "test_hooks.sh") == 0);
    fprintf(stderr, "Checking tags...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetArrayItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "commands"), 0), "insert"), "displayTags"), 0)), "NEW") == 0);
}
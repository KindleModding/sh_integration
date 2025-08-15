#include "extractor.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main()
{
    char uuid[37];
    scanner_gen_uuid(uuid, 37);
    cJSON* changeRequest = generateChangeRequest("./test.sh", uuid, "TestName", "TestAuthor", "TestIcon");
    fprintf(stderr, "Testing Normal\n%s", cJSON_Print(changeRequest));

    fprintf(stderr, "Checking uuid...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(changeRequest, "uuid")), uuid) == 0);
    fprintf(stderr, "Checking path...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(changeRequest, "location")), "./test.sh") == 0);
    fprintf(stderr, "Checking thumbnail...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(changeRequest, "thumbnail")), "TestIcon") == 0);
    fprintf(stderr, "Checking author...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "credits"), 0), "name"), "display")), "TestAuthor") == 0);
    fprintf(stderr, "Checking title...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "titles"), 0), "display")), "TestName") == 0);

    cJSON_Delete(changeRequest);
    changeRequest = generateChangeRequest("./test.sh", uuid, NULL, "TestAuthor", "TestIcon");
    fprintf(stderr, "Testing NULL name\n%s", cJSON_Print(changeRequest));

    fprintf(stderr, "Checking uuid...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(changeRequest, "uuid")), uuid) == 0);
    fprintf(stderr, "Checking path...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(changeRequest, "location")), "./test.sh") == 0);
    fprintf(stderr, "Checking thumbnail...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(changeRequest, "thumbnail")), "TestIcon") == 0);
    fprintf(stderr, "Checking author...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "credits"), 0), "name"), "display")), "TestAuthor") == 0);
    fprintf(stderr, "Checking title...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "titles"), 0), "display")), "test.sh") == 0);

    cJSON_Delete(changeRequest);
    changeRequest = generateChangeRequest("./test.sh", uuid, NULL, NULL, "TestIcon");
    fprintf(stderr, "Testing NULL author\n%s", cJSON_Print(changeRequest));

    fprintf(stderr, "Checking uuid...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(changeRequest, "uuid")), uuid) == 0);
    fprintf(stderr, "Checking path...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(changeRequest, "location")), "./test.sh") == 0);
    fprintf(stderr, "Checking thumbnail...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(changeRequest, "thumbnail")), "TestIcon") == 0);
    fprintf(stderr, "Checking author...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "credits"), 0), "name"), "display")), "Unknown") == 0);
    fprintf(stderr, "Checking title...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "titles"), 0), "display")), "test.sh") == 0);

    cJSON_Delete(changeRequest);
    changeRequest = generateChangeRequest("./test.sh", uuid, NULL, NULL, NULL);
    fprintf(stderr, "Testing NULL icon\n%s", cJSON_Print(changeRequest));

    fprintf(stderr, "Checking uuid...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(changeRequest, "uuid")), uuid) == 0);
    fprintf(stderr, "Checking path...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(changeRequest, "location")), "./test.sh") == 0);
    fprintf(stderr, "Checking thumbnail...\n");
    assert(!cJSON_HasObjectItem(changeRequest, "thumbnail"));
    fprintf(stderr, "Checking author...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "credits"), 0), "name"), "display")), "Unknown") == 0);
    fprintf(stderr, "Checking title...\n");
    assert(strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(changeRequest, "titles"), 0), "display")), "test.sh") == 0);
}
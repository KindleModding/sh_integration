#include "extractor.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>

int main()
{
    ScannerEventHandler* handler;
    int unk1;
    load_extractor(&handler, &unk1);    

    assert(unk1 == 0);

    char uuid[37];
    scanner_gen_uuid(uuid, 37);

    struct scanner_event event = {
        .event_type = SCANNER_ADD,
        .path = ".",
        .lipchandle = NULL,
        .filename = "./tests/KindleCraft.sh",
        .uuid = uuid,
        .glob = ""
    };

    assert(handler(&event) == 0);

    return 0;
}
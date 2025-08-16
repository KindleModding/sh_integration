#include "extractor.h"
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>

int main()
{
    // MUST be run after the index file stuff has been tested
    struct stat stats;
    assert(stat("./tests/test_hooks.sh.sdr/icon.png", &stats) == 0);
    assert(stat("./tests/test_hooks.sh.sdr", &stats) == 0);

    ScannerEventHandler* handler;
    int unk1;
    load_file_extractor(&handler, &unk1);    

    assert(unk1 == 0);

    char uuid[37];
    scanner_gen_uuid(uuid, 37);

    struct scanner_event event = {
        .event_type = SCANNER_DELETE,
        .path = ".",
        .lipchandle = NULL,
        .filename = "./tests/test_hooks.sh",
        .uuid = uuid,
        .glob = ""
    };

    assert(handler(&event) == 0);
    
    assert(stat("./tests/test_hooks.sh.sdr/icon.png", &stats) != 0);
    assert(stat("./tests/test_hooks.sh.sdr", &stats) != 0);
    
    return 0;
}
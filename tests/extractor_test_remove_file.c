#include "extractor.h"
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>

int main()
{
    // MUST be run after the index file stuff has been tested
    struct stat stats;
    assert(stat("./test.sh.sdr/icon.png", &stats) == 0);
    assert(stat("./test.sh.sdr", &stats) == 0);

    ScannerEventHandler* handler;
    int unk1;
    load_extractor(&handler, &unk1);    

    assert(unk1 == 0);

    char uuid[37];
    scanner_gen_uuid(uuid, 37);

    struct scanner_event event = {
        .event_type = SCANNER_DELETE,
        .path = ".",
        .lipchandle = NULL,
        .filename = "test.sh",
        .uuid = uuid,
        .glob = ""
    };

    handler(&event);
    
    assert(stat("./test.sh.sdr/icon.png", &stats) != 0);
    assert(stat("./test.sh.sdr", &stats) != 0);
    
    return 0;
}
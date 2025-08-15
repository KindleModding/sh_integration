#include "extractor.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

int main()
{
    char uuid[37];
    scanner_gen_uuid(uuid, 37);
    remove_file(".", "test.sh", uuid);
    struct stat stats;
    assert(stat("./test.sh.sdr/icon.png", &stats) != 0);
    assert(stat("./test.sh.sdr", &stats) != 0);
    
    return 0;
}
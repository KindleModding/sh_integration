#include "extractor.h"
#include "scanner.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>

int compareFiles(FILE* a, FILE* b)
{
    int index=-1;
    int char_a;
    int char_b;
    while (true)
    {
        char_a = fgetc(a);
        char_b = fgetc(b);
        index++;

        if (char_a == EOF || char_b == EOF)
        {
            if (char_a != char_b)
            {
                fprintf(stderr, "Early EOF in file %s at byte %i\n", char_a == EOF ? "file A" : "file B", index);
                return 1;
            }
            break;
        }

        if (char_a == char_b)
        {
            continue;
        }
        fprintf(stderr, "Read mismatch at byte %i\n", index);
        return 1;
    }

    return 0;
}

int main()
{
    ScannerEventHandler* handler;
    int unk1;
    load_extractor(&handler, &unk1);    

    assert(unk1 == 0);

    char uuid[37];
    scanner_gen_uuid(uuid, 37);

    struct scanner_event event = {
        .event_type = SCANNER_UPDATE,
        .path = ".",
        .lipchandle = NULL,
        .filename = "test_hooks.sh",
        .uuid = uuid,
        .glob = ""
    };

    assert(handler(&event) == 0);

    struct stat stats;
    assert(stat("./tests/test_hooks.sh.sdr/icon.png", &stats) == 0);

    fprintf(stderr, "Checking if icon was copied\n");
    FILE* icon = fopen("./tests/test_hooks.sh.sdr/icon.png", "r");
    FILE* icon_truth = fopen("./tests/example_icon.png", "r");
    
    assert(compareFiles(icon_truth, icon) == 0);

    fclose(icon);
    fclose(icon_truth);


    fprintf(stderr, "Checking if script was copied\n");
    assert(stat("./tests/test_hooks.sh.sdr/script.sh", &stats) == 0);
    FILE* script = fopen("./tests/test_hooks.sh.sdr/script.sh", "r");
    FILE* script_truth = fopen("./tests/test_hooks.sh", "r");

    assert(compareFiles(script_truth, script) == 0);

    fclose(script);
    fclose(script_truth);

    return 0;
}
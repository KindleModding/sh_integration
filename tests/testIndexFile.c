#include "extractor.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>

int compareFiles(FILE* a, FILE* b)
{
    int index=-1;
    char char_a;
    char char_b;
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
    index_file(".", "test.sh");
    struct stat stats;
    assert(stat("./test.sh.sdr/icon.png", &stats) == 0);

    fprintf(stderr, "Checking if icon was copied\n");
    FILE* icon = fopen("./test.sh.sdr/icon.png", "r");
    FILE* icon_truth = fopen("./tests/example_icon.png", "r");
    
    assert(compareFiles(icon_truth, icon) == 0);

    fclose(icon);
    fclose(icon_truth);


    fprintf(stderr, "Checking if script was copied\n");
    assert(stat("./test.sh.sdr/script.sh", &stats) == 0);
    FILE* script = fopen("./test.sh.sdr/script.sh", "r");
    FILE* script_truth = fopen("./test.sh", "r");

    assert(compareFiles(script_truth, script) == 0);

    fclose(script);
    fclose(script_truth);

    return 0;
}
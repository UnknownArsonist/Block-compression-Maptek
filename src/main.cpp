#include "StreamProcessor.h"
#include "DisplayOutput.h"


/* NOTES:
    Could use fwrite from <cstdio> instead of printf for output to stdout (more efficient)
*/

int main()
{
    StreamProcessor processor;
    FILE *input_file = fopen("test_input.txt", "r");
    processor.startProcessing(input_file);
    fclose(input_file);
    processor.printHeader();
    return 0;
}

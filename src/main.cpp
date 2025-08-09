#include "StreamProcessor.h"
#include "DisplayOutput.h"


/* NOTES:
    Could use fwrite from <cstdio> instead of printf for output to stdout (more efficient)
*/

int main()
{
    StreamProcessor processor;
    processor.startProcessing();
    processor.printHeader();
    return 0;
}

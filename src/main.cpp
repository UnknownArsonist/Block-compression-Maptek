#include "StreamProcessor.h"


/* NOTES:
    Could use fwrite from <cstdio> instead of printf for output to stdout (more efficient)
*/

int main()
{
    StreamProcessor processor;
    processor.start();
    processor.getInputStreamReader()->printHeader();
    processor.getCompressor()->compressStream();
    return 0;
}

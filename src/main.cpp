#include "StreamProcessor.h"

/* NOTES:
    Could use fwrite from <cstdio> instead of printf for output to stdout (more efficient)
*/

int main()
{
    StreamProcessor processor;
    Compressor compressor;
    //processor.setVerbose(true);
    processor.start();
    processor.end();
    // processor.getInputStreamReader()->printHeader();
    // processor.getCompressor()->compressStream();
    // compressor.compressParentBlock();
    return 0;
}

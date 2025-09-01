#include "StreamProcessor.h"
/* NOTES:
    Could use fwrite from <cstdio> instead of printf for output to stdout (more efficient)
*/

int main()
{
    StreamProcessor processor;
    processor.setVerbose(true);
    // Enable the actual compression pipeline
    processor.start();
    
    // Uncomment to test with hardcoded data
    // compressor.compressParentBlock();
    
    processor.end();
    return 0;
}

#include "StreamProcessor.h"
/* NOTES:
    Could use fwrite from <cstdio> instead of printf for output to stdout (more efficient)
*/

int main(int argc, char **argv) {
    int num_compress_threads = 8;
    if (argc > 1) {
        int c = atoi(argv[1]);
        if (c >= 1 && c <= 16) {
            num_compress_threads = c;
        }
    }
    StreamProcessor *processor = new StreamProcessor(num_compress_threads);
    
    processor->setVerbose(true);
    // Enable the actual compression pipeline
    processor->start();
    
    // Uncomment to test with hardcoded data
    // compressor.compressParentBlock();
    delete processor;
    return 0;
}

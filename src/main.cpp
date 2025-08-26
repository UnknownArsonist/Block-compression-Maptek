#include "StreamProcessor.h"
<<<<<<< HEAD

=======
#include <signal.h>
#include <execinfo.h>
>>>>>>> 7a84fbe7280b4e0869f090e21fdebabaeccc7e4e
/* NOTES:
    Could use fwrite from <cstdio> instead of printf for output to stdout (more efficient)
*/

<<<<<<< HEAD
int main()
{
    StreamProcessor processor;
    Compressor compressor;
    
    // Enable the actual compression pipeline
    processor.start();
    
    // Uncomment to test with hardcoded data
    // compressor.compressParentBlock();
    
=======
void signal_handler(int sig)
{
    void *array[10];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 10);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}

int main()
{
    // Install signal handlers for debugging
    signal(SIGSEGV, signal_handler);
    signal(SIGABRT, signal_handler);

    StreamProcessor processor;
    // processor.setVerbose(true);
    processor.start();
>>>>>>> 7a84fbe7280b4e0869f090e21fdebabaeccc7e4e
    processor.end();
    return 0;
}

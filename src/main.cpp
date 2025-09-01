#include "../include/StreamProcessor.h"
#include "../include/InputStreamReader.h"
#include <signal.h>
/* NOTES:
    Could use fwrite from <cstdio> instead of printf for output to stdout (more efficient)
*/

void signal_handler(int sig)
{
 
    // get void*'s for all entries on the stack
    //size = backtrace(array, 10);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    //backtrace_symbols_fd(array, size, STDERR_FILENO);
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
    processor.end();
    return 0;
}
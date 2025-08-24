#include "../include/StreamProcessor.h"
#include <signal.h>
#include <cstdio>
#include <windows.h>

#ifdef _WIN32
void signal_handler(int sig)
{
    // Allocate memory to hold the stack trace
    const int max_frames = 10;
    void* stack[max_frames];
    unsigned short frames;

    // Capture the stack trace (addresses only, no symbols)
    frames = CaptureStackBackTrace(0, max_frames, stack, NULL);

    // Print stack trace (addresses only)
    fprintf(stderr, "Error: signal %d:\n", sig);
    for (int i = 0; i < frames; ++i) {
        fprintf(stderr, "Frame %d: 0x%p\n", i, stack[i]);
    }
    exit(1);
}
#endif

int main()
{
    // Install signal handlers for debugging on Windows
    #ifdef _WIN32
    signal(SIGSEGV, signal_handler);
    signal(SIGABRT, signal_handler);
    #endif

    StreamProcessor processor;
    // processor.setVerbose(true);
    processor.start();
    processor.end();
    return 0;
}

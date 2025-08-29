#include "StreamProcessor.h"

StreamProcessor::StreamProcessor() {
    inputStreamReader = new InputStreamReader();
    compressor = new Compressor();
    displayOutput = new DisplayOutput();
    inputToCompressorBuffer = new StreamBuffer();
    compressorToOutputBuffer = new StreamBuffer();
}

StreamProcessor::~StreamProcessor() {
    free(inputStreamReader);
    free(compressor);
    free(displayOutput);
    free(inputToCompressorBuffer);
    free(compressorToOutputBuffer);
}

void StreamProcessor::setup() {
    inputToCompressorBuffer->setSize(1024);
    compressorToOutputBuffer->setSize(8192);
    inputStreamReader->passValues(this);
    inputStreamReader->getHeader();
    compressor->passValues(this);
    displayOutput->passValues(this);
}

void StreamProcessor::start() {
    setup();
    if (verbose) {
        fprintf(stderr, "[SP] Setup Complete\n");
        started = std::chrono::high_resolution_clock::now();
    }
    // inputStreamReader.printHeader();
    compressorThread = std::thread(&Compressor::compressStream, compressor);
    //compressorThread2 = std::thread(&Compressor::compressStream, compressor);
    displayOutputThread = std::thread(&DisplayOutput::displayBlocks, displayOutput);
    inputStreamReader->processStream();
    if (verbose) {
        auto end = std::chrono::high_resolution_clock::now();
        std::cerr << "InputStreamReader Runtime:\n  " << std::chrono::duration_cast<std::chrono::milliseconds>(end - started).count() << std::endl;
    }
}

void StreamProcessor::end()
{
    compressorThread.join();
    if (verbose) {
        auto end = std::chrono::high_resolution_clock::now();
        std::cerr << "Compressor1 Runtime:\n  " << std::chrono::duration_cast<std::chrono::milliseconds>(end - started).count() << std::endl;
    }
    /*
    compressorThread2.join();
    if (verbose) {
        auto end = std::chrono::high_resolution_clock::now();
        std::cerr << "Compressor2 Runtime:\n  " << std::chrono::duration_cast<std::chrono::milliseconds>(end - started).count() << std::endl;
    }
    */
    displayOutputThread.join();
    if (verbose) {
        auto end = std::chrono::high_resolution_clock::now();
        std::cerr << "Output Runtime:\n  " << std::chrono::duration_cast<std::chrono::milliseconds>(end - started).count() << std::endl;
    }
}

void StreamProcessor::setVerbose(bool c_v) {
    verbose = c_v;
}
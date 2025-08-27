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
    inputStreamReaderThread = std::thread(&InputStreamReader::processStream, inputStreamReader);
    compressorThread = std::thread(&Compressor::compressStream, compressor);
    displayOutputThread = std::thread(&DisplayOutput::displayBlocks, displayOutput);
}

void StreamProcessor::end()
{
    inputStreamReaderThread.join();
    if (verbose) {
        auto end = std::chrono::high_resolution_clock::now();
        std::cerr << "InputStreamReader Runtime:\n  " << std::chrono::duration_cast<std::chrono::milliseconds>(end - started).count() << std::endl;
    }
    compressorThread.join();
    if (verbose) {
        auto end = std::chrono::high_resolution_clock::now();
        std::cerr << "Compressor Runtime:\n  " << std::chrono::duration_cast<std::chrono::milliseconds>(end - started).count() << std::endl;
    }
    displayOutputThread.join();
    if (verbose) {
        auto end = std::chrono::high_resolution_clock::now();
        std::cerr << "Output Runtime:\n  " << std::chrono::duration_cast<std::chrono::milliseconds>(end - started).count() << std::endl;
    }
}

void StreamProcessor::setVerbose(bool c_v) {
    verbose = c_v;
}
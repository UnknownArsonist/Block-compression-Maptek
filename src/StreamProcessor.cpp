#include "StreamProcessor.h"

StreamProcessor::StreamProcessor(int c_num_compressor_threads, int c_itoc_buf_size, int c_ctoo_buf_size) {
    inputStreamReader = new InputStreamReader();
    compressor = new Compressor();
    displayOutput = new DisplayOutput();
    inputToCompressorBuffer = new StreamBuffer();
    compressorToOutputBuffer = new StreamBuffer(c_num_compressor_threads);
    num_compressor_threads = c_num_compressor_threads;
    compressorThreads = (std::thread**)malloc(num_compressor_threads * sizeof(std::thread*));
}

StreamProcessor::StreamProcessor(int c_num_compressor_threads) : StreamProcessor(c_num_compressor_threads, 1024, 8192) {}

StreamProcessor::StreamProcessor() : StreamProcessor(1) {}

StreamProcessor::~StreamProcessor() {
    for (int i = 0; i < num_compressor_threads; i++) {
        compressorThreads[i]->join();
        delete compressorThreads[i];
    }
    if (verbose) {
        auto end = std::chrono::high_resolution_clock::now();
        std::cerr << "Compressor Runtime:\n  " << std::chrono::duration_cast<std::chrono::milliseconds>(end - started).count() << std::endl;
    }
    displayOutputThread.join();
    if (verbose) {
        auto end = std::chrono::high_resolution_clock::now();
        std::cerr << "Output Runtime:\n  " << std::chrono::duration_cast<std::chrono::milliseconds>(end - started).count() << std::endl;
    }
    free(compressorThreads);
    free(inputStreamReader);
    free(compressor);
    free(displayOutput);
    free(inputToCompressorBuffer);
    free(compressorToOutputBuffer);
}

void StreamProcessor::setup() {
    inputStreamReader->passValues(this);
    inputStreamReader->getHeader();
    if (verbose)
        inputStreamReader->printHeader();
    compressor->passValues(this);
    displayOutput->passValues(this);
    int itoc_buf_size = (x_count / parent_x) * (y_count / parent_y) * (z_count / parent_z);
    if (itoc_buf_size > 512)
        itoc_buf_size = 512;
    inputToCompressorBuffer->setSize(itoc_buf_size);
    compressorToOutputBuffer->setSize(itoc_buf_size * 256);
}

void StreamProcessor::start() {
    setup();
    if (verbose) {
        fprintf(stderr, "[SP] Setup Complete\n");
        started = std::chrono::high_resolution_clock::now();
    }
    // inputStreamReader.printHeader();
    if (verbose) {
        fprintf(stderr, "Starting %d Compressor Threads\n", num_compressor_threads);
        started = std::chrono::high_resolution_clock::now();
    }
    for (int i = 0; i < num_compressor_threads; i++) {
        compressorThreads[i] = new std::thread(&Compressor::compressStream, compressor);
    }
    if (verbose) {
        fprintf(stderr, "Starting Display Thread\n");
        started = std::chrono::high_resolution_clock::now();
    }
    displayOutputThread = std::thread(&DisplayOutput::displayBlocks, displayOutput);
    inputStreamReader->processStream();
    if (verbose) {
        auto end = std::chrono::high_resolution_clock::now();
        std::cerr << "InputStreamReader Runtime:\n  " << std::chrono::duration_cast<std::chrono::milliseconds>(end - started).count() << std::endl;
    }
}

void StreamProcessor::setVerbose(bool c_v) {
    verbose = c_v;
}
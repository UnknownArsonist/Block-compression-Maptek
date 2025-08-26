#include "StreamProcessor.h"

StreamProcessor::StreamProcessor()
{
}

StreamProcessor::~StreamProcessor()
{
}

void StreamProcessor::setup()
{
    inputToCompressorBuffer.setSize(1024);
    compressorToOutputBuffer.setSize(8192);
    inputStreamReader.passValues(&x_count, &y_count, &z_count, &parent_x, &parent_y, &parent_z, &tag_table);
    inputStreamReader.passBuffers(&inputToCompressorBuffer);
    inputStreamReader.getHeader();
<<<<<<< HEAD
    compressor.passValues(&parent_x, &parent_y, &parent_z, &tag_table);
=======
    compressor.passValues(&parent_x, &parent_y, &parent_z, &tag_table, x_count, y_count, z_count);
>>>>>>> 7a84fbe7280b4e0869f090e21fdebabaeccc7e4e
    compressor.passBuffers(&inputToCompressorBuffer, &compressorToOutputBuffer);
    displayOutput.passValues(&tag_table);
    displayOutput.passBuffers(&compressorToOutputBuffer);
}

void StreamProcessor::start()
{
    setup();
    if (verbose)
<<<<<<< HEAD
        fprintf(stderr, "[SP] Setup Complete\n");
    //inputStreamReader.printHeader();
=======
    {
        fprintf(stderr, "[SP] Setup Complete\n");
        inputStreamReader.printHeader(stderr);
        started = std::chrono::high_resolution_clock::now();
    }
    // inputStreamReader.printHeader();
>>>>>>> 7a84fbe7280b4e0869f090e21fdebabaeccc7e4e
    inputStreamReaderThread = std::thread(&InputStreamReader::processStream, &inputStreamReader);
    compressorThread = std::thread(&Compressor::compressStream, &compressor);
    displayOutputThread = std::thread(&DisplayOutput::displayBlocks, &displayOutput);
}

<<<<<<< HEAD
void StreamProcessor::end() {
    inputStreamReaderThread.join();
    compressorThread.join();
    displayOutputThread.join();
}

void StreamProcessor::setVerbose(bool c_v) {
=======
void StreamProcessor::end()
{
    inputStreamReaderThread.join();
    if (verbose)
    {
        auto end = std::chrono::high_resolution_clock::now();
        std::cerr << "InputStreamReader Runtime:\n  " << std::chrono::duration_cast<std::chrono::milliseconds>(end - started).count() << std::endl;
    }
    compressorThread.join();
    if (verbose)
    {
        auto end = std::chrono::high_resolution_clock::now();
        std::cerr << "Compressor Runtime:\n  " << std::chrono::duration_cast<std::chrono::milliseconds>(end - started).count() << std::endl;
    }
    displayOutputThread.join();
    if (verbose)
    {
        auto end = std::chrono::high_resolution_clock::now();
        std::cerr << "Output Runtime:\n  " << std::chrono::duration_cast<std::chrono::milliseconds>(end - started).count() << std::endl;
    }
}

void StreamProcessor::setVerbose(bool c_v)
{
>>>>>>> 7a84fbe7280b4e0869f090e21fdebabaeccc7e4e
    verbose = c_v;
}

InputStreamReader *StreamProcessor::getInputStreamReader() { return &inputStreamReader; }
Compressor *StreamProcessor::getCompressor() { return &compressor; }
DisplayOutput *StreamProcessor::getDisplayOutput() { return &displayOutput; }
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
    compressor.passValues(&parent_x, &parent_y, &parent_z, &tag_table);
    compressor.passBuffers(&inputToCompressorBuffer, &compressorToOutputBuffer);
    displayOutput.passValues(&tag_table);
    displayOutput.passBuffers(&compressorToOutputBuffer);
}

void StreamProcessor::start()
{
    setup();
    if (verbose)
        fprintf(stderr, "[SP] Setup Complete\n");
    //inputStreamReader.printHeader();
    inputStreamReaderThread = std::thread(&InputStreamReader::processStream, &inputStreamReader);
    compressorThread = std::thread(&Compressor::compressStream, &compressor);
    displayOutputThread = std::thread(&DisplayOutput::displayBlocks, &displayOutput);
}

void StreamProcessor::end() {
    inputStreamReaderThread.join();
    compressorThread.join();
    displayOutputThread.join();
}

void StreamProcessor::setVerbose(bool c_v) {
    verbose = c_v;
}

InputStreamReader *StreamProcessor::getInputStreamReader() { return &inputStreamReader; }
Compressor *StreamProcessor::getCompressor() { return &compressor; }
DisplayOutput *StreamProcessor::getDisplayOutput() { return &displayOutput; }
#include "StreamProcessor.h"

StreamProcessor::StreamProcessor()
{
}

StreamProcessor::~StreamProcessor()
{
}

void StreamProcessor::setup()
{
    inputStreamReader.passValues(&x_count, &y_count, &z_count, &parent_x, &parent_y, &parent_z, &tag_table);
    inputStreamReader.passBuffers(&inputToCompressorBuffer);
    inputStreamReader.getHeader();
    compressor.passValues(&parent_x, &parent_y, &parent_z, &tag_table);
    compressor.passBuffers(&inputToCompressorBuffer, &compressorToOutputBuffer);
    inputToCompressorBuffer.setSize(64);
    compressorToOutputBuffer.setSize(64);
}

void StreamProcessor::start()
{
    setup();
    inputStreamReader.printHeader();
    inputStreamReader.processStream();
    compressor.compressStream();
}

InputStreamReader *StreamProcessor::getInputStreamReader() { return &inputStreamReader; }
Compressor *StreamProcessor::getCompressor() { return &compressor; }
DisplayOutput *StreamProcessor::getDisplayOutput() { return &displayOutput; }
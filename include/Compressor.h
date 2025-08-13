#pragma once
#include "header.h"
#include "StreamBuffer.h"

class Compressor
{
private:
    // Parent Block dimensions
    int *parent_x;
    int *parent_y;
    int *parent_z;

    StreamBuffer *input_stream;
    StreamBuffer *output_stream;

public:
    Compressor();
    ~Compressor();

    void compressParentBlock();
    void processParentBlocks(const std::vector<std::vector<std::vector<char>>> &sub_blocks);
    void printParentBlock(const std::vector<std::vector<std::vector<std::vector<char>>>> &parent_blocks);
    void compressStream();
    void passValues(int *c_parent_x, int *c_parent_y, int *c_parent_z);
    void passBuffers(StreamBuffer *c_input_stream, StreamBuffer *c_output_stream);
};
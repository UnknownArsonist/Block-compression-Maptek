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
    std::unordered_map<char, std::string> *tagTable;

    StreamBuffer *input_stream;
    StreamBuffer *output_stream;

public:
    Compressor();
    ~Compressor();

    // helper function

    void compressParentBlock();
    void processParentBlocks(ParentBlock *parent_block);
    void printParentBlock(const std::vector<std::vector<std::vector<std::vector<char>>>> &parent_blocks);
    void compressStream();
    void passValues(int *c_parent_x, int *c_parent_y, int *c_parent_z, std::unordered_map<char, std::string> *tagtable);
    void passBuffers(StreamBuffer *c_input_stream, StreamBuffer *c_output_stream);
};
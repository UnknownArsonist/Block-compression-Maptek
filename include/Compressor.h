#pragma once
#include "header.h"
#include "StreamBuffer.h"
<<<<<<< HEAD
=======
#include "OctTreeNode.h"
>>>>>>> 7a84fbe7280b4e0869f090e21fdebabaeccc7e4e

class Compressor
{
private:
    // Parent Block dimensions
    int *parent_x;
    int *parent_y;
    int *parent_z;
<<<<<<< HEAD
    std::unordered_map<char, std::string> *tagTable;

    StreamBuffer *input_stream;
    StreamBuffer *output_stream;
=======

    int mx;
    int my;
    int mz;

    StreamBuffer *input_stream;
    StreamBuffer *output_stream;
    OctTreeNode octTree;
    std::unordered_map<char, std::string> *tagTable;
>>>>>>> 7a84fbe7280b4e0869f090e21fdebabaeccc7e4e

public:
    Compressor();
    ~Compressor();

<<<<<<< HEAD
    // helper function

=======
    // algorithms
    void OctreeCompression(ParentBlock *parent_block);

    // helper function
>>>>>>> 7a84fbe7280b4e0869f090e21fdebabaeccc7e4e
    void compressParentBlock();
    void processParentBlocks(ParentBlock *parent_block);
    void printParentBlock(const std::vector<std::vector<std::vector<std::vector<char>>>> &parent_blocks);
    void compressStream();
<<<<<<< HEAD
    void passValues(int *c_parent_x, int *c_parent_y, int *c_parent_z, std::unordered_map<char, std::string> *tagtable);
=======
    void passValues(int *c_parent_x, int *c_parent_y, int *c_parent_z, std::unordered_map<char, std::string> *tagtable, int mx_count, int my_count, int mz_count);
>>>>>>> 7a84fbe7280b4e0869f090e21fdebabaeccc7e4e
    void passBuffers(StreamBuffer *c_input_stream, StreamBuffer *c_output_stream);
};
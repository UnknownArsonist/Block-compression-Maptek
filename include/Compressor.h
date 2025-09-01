#pragma once
#include "header.h"
#include "StreamBuffer.h"
#include "OctTreeNode.h"

struct Cuboid
{
    int x, y, z; // bottom-left-front corner
    int w, h, d; // width, height, depth
    char label;  // block label
};

class Compressor
{
private:
    // Parent Block dimensions
    int *parent_x;
    int *parent_y;
    int *parent_z;

    int mx;
    int my;
    int mz;

    StreamBuffer *input_stream;
    StreamBuffer *output_stream;
    OctTreeNode octTree;
    std::unordered_map<char, std::string> *tagTable;
    std::vector<SubBlock *> blocks;

public:
    Compressor();
    ~Compressor();
    void compressStream();
    void passValues(int *c_parent_x, int *c_parent_y, int *c_parent_z, std::unordered_map<char, std::string> *tagtable, int mx_count, int my_count, int mz_count);
    void passBuffers(StreamBuffer *c_input_stream, StreamBuffer *c_output_stream);
    // --------------------------// -------------------------

    std::vector<Cuboid> compressParentBlock(ParentBlock *pb,
                                            int parent_x, int parent_y, int parent_z);
    void printCuboidsWithLegend(std::vector<Cuboid> &cuboids,
                                     std::unordered_map<char, std::string> &legend);
    
                                     void validateCoverage(const std::vector<Cuboid>& cuboids, ParentBlock* pb,
                     int parent_x, int parent_y, int parent_z);
};
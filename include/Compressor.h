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

    // algorithms
    void OctreeCompression(ParentBlock *parent_block);

    bool isUniform(ParentBlock *parent_block, int z_index);
    void blockRect3D(ParentBlock *pb);
    void base_algorithms(ParentBlock *parent_block);

    // helper function
<<<<<<< HEAD
    // void compressParentBlock();
=======
    static inline char voxel(ParentBlock *pb, int x, int y, int z, int strideX, int strideY) {
        return pb->block[x * strideX + y * strideY + z];
    }

    void compressParentBlock();
>>>>>>> 42b1140eec2fe31997d77995c38aec7f593bdfe0
    void processParentBlocks(ParentBlock *parent_block);
    void printParentBlock(const std::vector<std::vector<std::vector<std::vector<char>>>> &parent_blocks);
    void compressStream();
    void passValues(int *c_parent_x, int *c_parent_y, int *c_parent_z, std::unordered_map<char, std::string> *tagtable, int mx_count, int my_count, int mz_count);
    void passBuffers(StreamBuffer *c_input_stream, StreamBuffer *c_output_stream);
    // --------------------------// -------------------------

    std::vector<Cuboid> compressParentBlock(const ParentBlock *pb,
                                            int parent_x, int parent_y, int parent_z);
    void printCuboidsWithLegend(const std::vector<Cuboid> &cuboids,
                                const std::unordered_map<char, std::string> &legend);
};
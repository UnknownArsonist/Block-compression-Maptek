#pragma once
#include "header.h"

class OctTreeNode
{
private:
    // the octTree Node stores the tag to represent the entire uniform block
    char tag;

    // used for checking if the current octTreeNode is uniform or not
    //  if not, then we sub-divide the Node into 8 subNodes
    bool isleaf;

    // starting co-ordinates of the block (parentBlock)
    int x0;
    int y0;
    int z0;

    // dimension of the 3D block of the block (parentBlock)
    int sizeX;
    int sizeY;
    int sizeZ;

    // OctTreeNode contains pointer 8 children
    OctTreeNode *children[8];
    std::vector<OctTreeNode *> childrenVector;

public:
    // Constructors & deconstructors
    OctTreeNode(/* args */);
    ~OctTreeNode();

    // main functions
    OctTreeNode *build(ParentBlock &grid, int x0, int y0, int z0,
                       int sizeX, int sizeY, int sizeZ);
    OctTreeNode *buildContentDriven3D(ParentBlock &grid,
                                      int x0, int y0, int z0,
                                      int sizeX, int sizeY, int sizeZ);

    // helper functions
    bool isUniform(ParentBlock *parent_block, int x0, int y0, int z0,
                   int sizeX, int sizeY, int sizeZ, char &outTag);

    void collectSubBlocks(OctTreeNode *node, std::vector<SubBlock> &blocks,
                          std::unordered_map<char, std::string> *tag_table, int originX, int originY, int originZ);

    std::vector<SubBlock> mergeSubBlocks(const std::vector<SubBlock> &blocks);
    void deleteTree(OctTreeNode *node);
};

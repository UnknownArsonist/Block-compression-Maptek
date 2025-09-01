#pragma once
#include <vector>
#include <cstddef> // for size_t

// Forward declaration
class OctTreeNode;

class OctreeNodePool {
    std::vector<OctTreeNode*> pool;
    size_t index = 0;

public:
    OctreeNodePool(size_t capacity = 10000);
    ~OctreeNodePool();

    OctTreeNode* get();
};



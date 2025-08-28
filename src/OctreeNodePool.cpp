#include "../include/OctreeNodePool.h"
#include "../include/OctTreeNode.h" 

OctreeNodePool::OctreeNodePool(size_t capacity ): index(0) { // default here
    pool.reserve(capacity);
    for (size_t i = 0; i < capacity; i++)
        pool.push_back(new OctTreeNode());
    index = 0;
}

OctreeNodePool::~OctreeNodePool() {
    for (auto node : pool) delete node;
}

OctTreeNode* OctreeNodePool::get() {
    if (index >= pool.size()) {
        pool.push_back(new OctTreeNode());
    }
    return pool[index++];
}

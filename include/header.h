#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <cstdio>
#include <thread>
#include <cstring>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm> // For std::min
#include <tuple>     // For std::tie
#ifdef WIN32
#include <windows.h>
#endif

struct SubBlock {
    int x;
    int y;
    int z;
    int l;
    int w;
    int h;
    char tag;
};

struct ParentBlock {
    int x;
    int y;
    int z;
    char *block;
    char first;
    SubBlock** sub_blocks;
    int sub_block_num;
};

struct Chunk {
    int id;
    char *block;
    SubBlock** sub_blocks;
    int sub_block_num;
};
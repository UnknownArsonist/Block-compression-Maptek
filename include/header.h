#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <cstdio>
#include <thread>
#include <cstring>
#include <mutex>
#include <stack>
#include <algorithm>

#include <condition_variable>
#include <chrono>
#ifdef WIN32
#include <windows.h>
#endif

struct SubBlock
{
    int x;
    int y;
    int z;
    int l;
    int w;
    int h;
    char tag;
};

struct ParentBlock
{
    int x;
    int y;
    int z;
    char *block;
    char first;
    SubBlock **sub_blocks;
    int sub_block_num;
};

struct Rect
{
    int x, y, w, h;
    char label;
};

struct Run
{
    int x, len;
    char label;
};

struct OptimalRect
{
    int x, y, w, h;
    char label;
};

struct Cuboid
{
    int x, y, z; // bottom-left-front corner
    int w, h, d; // width, height, depth
    char label;  // block label
};

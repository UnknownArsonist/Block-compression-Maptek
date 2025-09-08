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
#include <algorithm>
#include <condition_variable>
#include <chrono>
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
    SubBlock **sub_blocks;
    int sub_block_num;
};

struct encode {
    char character;
    int length;

    //used for comparing two encode
    bool operator==(const encode &other) const {
        return character == other.character && length == other.length;
    }
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

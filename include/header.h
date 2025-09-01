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
#include <chrono>

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
};
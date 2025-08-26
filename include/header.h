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

<<<<<<< HEAD
struct SubBlock {
=======
struct SubBlock
{
>>>>>>> 7a84fbe7280b4e0869f090e21fdebabaeccc7e4e
    int x;
    int y;
    int z;
    int l;
    int w;
    int h;
    char tag;
};

<<<<<<< HEAD
struct ParentBlock {
=======
struct ParentBlock
{
>>>>>>> 7a84fbe7280b4e0869f090e21fdebabaeccc7e4e
    int x;
    int y;
    int z;
    char *block;
};
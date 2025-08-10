#pragma once
#include "header.h"
#include "InputStreamReader.h"

class DisplayOutput
{
private:
    std::vector<std::string> slice;
    std::unordered_map<char, std::string> tag_table;
    int count_x;
    int count_y;
    int count_z;
    int parent_x;
    int parent_y;
    int parent_z;

public:
    DisplayOutput(InputStreamReader &processor);
    void display_slice();
    void load_processor(InputStreamReader &processor);

    ~DisplayOutput();
};
#pragma once
#include "header.h"
#include "StreamProcessor.h"

class DisplayOutput : public StreamProcessor
{
private:
    vector<string> slice;
    unordered_map<char, string> tag_table;
    int count_x;
    int count_y;
    int count_z;
    int parent_x;
    int parent_y;
    int parent_z;

public:
    DisplayOutput(StreamProcessor &processor);
    void display_slice();
    void load_processor(StreamProcessor &processor);

    ~DisplayOutput();
};
#pragma once
#include "header.h"

class StreamProcessor
{
private:
    // Vectors to hold dimensions and tokens
    vector<string> dims;

    // Header line and other variables
    string header_line;
    vector<string> slice;

    // Dimensions of the 3D block
    int x_count;
    int y_count;
    int z_count;

    // Parent Block dimensions
    int parent_x;
    int parent_y;
    int parent_z;

    // Tag table to map characters to strings
    unordered_map<char, string> tag_table;

public:
    // Constructor and Destructor
    StreamProcessor(/* args */);
    ~StreamProcessor();

    // Function declarations
    vector<string> split(const string &s, char delimiter);
    void process_slice();
    void get_headerLine();
    void get_tagTable();
    void start_processing();

    // Getters and Setters for dimensions
    vector<string> getSlice();
    unordered_map<char, string> getTagTable();
    int getXCount();
    int getYCount();
    int getZCount();
    int getParentX();
    int getParentY();
    int getParentZ();
};

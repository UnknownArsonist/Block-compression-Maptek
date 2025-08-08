#pragma once
#include "header.h"

class StreamProcessor
{
private:
    // Vectors to hold dimensions and tokens
    // vector<string> dims;

    // Header line and other variables
    // string header_line;
    // vector<string> slice;

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

    // main block to hold the 3D block data
    vector<vector<vector<char>>> c;
    vector<vector<vector<char>>> *ptr_c = nullptr; // Pointer to the 3D block data

    // parent block to hold the 3D block data
    vector<vector<vector<vector<char>>>> parent_block;
    int count;
    int index_x;
    int index_z;

public:
    // Constructor and Destructor
    StreamProcessor(/* args */);
    ~StreamProcessor();

    // Function declarations
    void processSlice(FILE *in);
    void parentBlock(vector<vector<vector<char>>> *block);
    void processParentBlock(vector<vector<vector<char>>> *block, int count, int index_x, int index_z);

    void getHeaderLine(FILE *in);
    void getCommaSeparatedValuesFromStream(FILE *in);
    template <typename T, typename... Args>
    void getCommaSeparatedValuesFromStream(FILE *in, T *value, Args... args);
    void getLegendFromStream(FILE *in, std::unordered_map<char, std::string> *legend);
    void startProcessing(FILE *in);

    void printHeader();

    // Getters and Setters for dimensions
    unordered_map<char, string> getTagTable();
    int getXCount();
    int getYCount();
    int getZCount();
    int getParentX();
    int getParentY();
    int getParentZ();
};

#pragma once
#include "header.h"
#include "StreamBuffer.h"

class InputStreamReader
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

    FILE *input_stream;
    StreamBuffer *output_stream;

    // Tag table to map characters to strings
    std::unordered_map<char, std::string> tag_table;

public:
    // Constructor and Destructor
    InputStreamReader();
    InputStreamReader(FILE *in);
    ~InputStreamReader();

    // Function declarations
    void processStream();

    void getHeaderLine();
    void getCommaSeparatedValuesFromStream();
    template <typename T, typename... Args>
    void getCommaSeparatedValuesFromStream(T *value, Args... args);
    void getLegendFromStream(std::unordered_map<char, std::string> *legend);
    void startProcessing();

    void printHeader();

    // Getters and Setters for dimensions
    std::unordered_map<char, std::string> getTagTable();
    int getXCount();
    int getYCount();
    int getZCount();
    int getParentX();
    int getParentY();
    int getParentZ();
};

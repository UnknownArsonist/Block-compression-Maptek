#include "StreamProcessor.h"

// Constructor and Destructor
StreamProcessor::StreamProcessor(/* args */)
{
    // Constructor implementation
}

StreamProcessor::~StreamProcessor()
{
    // Destructor implementation
}

// Function to split a string by a delimiter
vector<string> StreamProcessor::split(const string &s, char delimiter)
{
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

void StreamProcessor::start_processing()
{
    get_headerLine();
}

// Function to read the header line and initialize dimensions
void StreamProcessor::get_headerLine()
{
    getline(cin, header_line);
    vector<string> dims = split(header_line, ',');

    x_count = stoi(dims[0]);
    y_count = stoi(dims[1]);
    z_count = stoi(dims[2]);
    parent_x = stoi(dims[3]);
    parent_y = stoi(dims[4]);
    parent_z = stoi(dims[5]);
    get_tagTable();
}

// Function to read the tag table
void StreamProcessor::get_tagTable()
{
    string line;
    while (getline(cin, line))
    {
        if (line.empty())
            break;
        auto parts = split(line, ',');
        tag_table[parts[0][0]] = parts[1];
    }
    process_slice();
}

// Function to process the slice of 3D block data
void StreamProcessor::process_slice()
{

    for (int z = 0; z < z_count; ++z)
    {
        vector<string> slice;
        string line;
        slice.clear();
        // Read lines from top to bottom
        for (int i = 0; i < y_count; ++i)
        {
            getline(cin, line);
            slice.push_back(line);
        }

        if (z < z_count - 1)
            getline(cin, line); // skip blank line

        for (int y = 0; y < y_count; ++y)
        {
            int actual_y = y_count - 1 - y; // reverse y-index

            for (int x = 0; x < x_count; ++x)
            {
                char tag = slice[y][x]; // y is the index in slice (top-down)
                string label = tag_table[tag];

                cout << x << "," << actual_y << "," << z << ",1,1,1," << label << "\n";
            }
        }
    }
}

// Getters for dimensions and tag table
vector<string> StreamProcessor::getSlice() { return slice; }
unordered_map<char, string> StreamProcessor::getTagTable() { return tag_table; }
int StreamProcessor::getXCount() { return x_count; }
int StreamProcessor::getYCount() { return y_count; }
int StreamProcessor::getZCount() { return z_count; }
int StreamProcessor::getParentX() { return parent_x; }
int StreamProcessor::getParentY() { return parent_y; }
int StreamProcessor::getParentZ() { return parent_z; }

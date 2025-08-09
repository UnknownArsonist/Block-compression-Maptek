#include "StreamProcessor.h"

// Constructor and Destructor
StreamProcessor::StreamProcessor(/* args */)
{
}

StreamProcessor::~StreamProcessor()
{
    // Destructor implementation
}

void StreamProcessor::startProcessing(FILE *in)
{
    getHeaderLine(in);
    processStream(in);
}

void StreamProcessor::getHeaderLine(FILE *in)
{
    getCommaSeparatedValuesFromStream(in, &x_count, &y_count, &z_count, &parent_x, &parent_y, &parent_z);
    getLegendFromStream(in, &tag_table);
}

void StreamProcessor::getCommaSeparatedValuesFromStream(FILE *in) {}
template <typename T, typename... Args>
void StreamProcessor::getCommaSeparatedValuesFromStream(FILE *in, T *value, Args... args)
{
    char c;
    *value = 0;
    while ((c = getc(in)) != EOF)
    {
        if (c == ',' || c == '\n')
        {
            break;
        }
        else
        {
            *value *= 10;
            *value += (int)c - '0';
        }
    }
    getCommaSeparatedValuesFromStream(in, args...);
}

void StreamProcessor::getLegendFromStream(FILE *in, std::unordered_map<char, std::string> *legend)
{
    char c;
    char key = 0;
    std::string value = "";
    int v = 0;
    int n = 0;
    while ((c = getc(in)) != EOF)
    {
        if (c == ',')
        {
            v++;
        }
        else if (c == '\n')
        {
            if (n > 0)
            {
                return;
            }
            (*legend)[key] = value;
            v = 0;
            value.clear();
            n++;
        }
        else
        {
            if (v == 0)
            {
                key = c;
            }
            else
            {
                value += c;
            }
            n = 0;
        }
    }
}

// Function to process the slice of 3D block data
void StreamProcessor::processStream(FILE *in)
{
    int num_parent_blocks = (x_count / parent_x) * (y_count / parent_y);
    char parent_blocks[num_parent_blocks][parent_x][parent_y][parent_z];

    int n = 0;
    int x = 0;
    int y = 0;
    int z = 0;
    char ch;

    while ((ch = getc(in)) != EOF) {
        int current_parent_block = (x / parent_x) + (x_count / parent_x) * (y / parent_y);
        if (ch == '\n') {
            x = 0;
            y++;
            n++;
            if (n > 1) {
                y = 0;
                z++;
            }
        } else {
            int parent_relative_x = x % parent_x;
            int parent_relative_y = y % parent_y;
            //printf("%d, %d, %d, %d: %c\n", current_parent_block, parent_relative_x, parent_relative_y, z, ch);

            parent_blocks[current_parent_block][parent_relative_x][parent_relative_y][z] = ch;
            x++;
            n = 0;
        }
    }

    /*
    printf("\nPrint Our Parent_block Structure\n\n");

    for (int pb = 0; pb < num_parent_blocks; pb++) {
        printf("Parent Block %d\n", pb);
        for (int z = 0; z < parent_z; z++) {
            for (int y = 0; y < parent_y; y++) {
                for (int x = 0; x < parent_x; x++) {
                    //printf("%d, %d, %d, %d: %c\n", pb, x, y, z, parent_blocks[pb][x][y][z]);
                    printf("%c", parent_blocks[pb][x][y][z]);
                }
                printf("\n");
            }
            printf("\n");
        }
    }
    */
}

// print the header information and the 3D block data
void StreamProcessor::printHeader()
{
    // print the header information
    printf("%d, %d, %d, %d, %d, %d\n", x_count, y_count, z_count, parent_x, parent_y, parent_z);
    for (const auto &e : tag_table)
    {
        printf("%c, %s\n", e.first, e.second.c_str());
    }
}

// Getters for dimensions and tag table
// vector<string> StreamProcessor::getSlice() { return slice; }
unordered_map<char, string> StreamProcessor::getTagTable() { return tag_table; }
int StreamProcessor::getXCount() { return x_count; }
int StreamProcessor::getYCount() { return y_count; }
int StreamProcessor::getZCount() { return z_count; }
int StreamProcessor::getParentX() { return parent_x; }
int StreamProcessor::getParentY() { return parent_y; }
int StreamProcessor::getParentZ() { return parent_z; }

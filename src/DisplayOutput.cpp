#include "DisplayOutput.h"

DisplayOutput::DisplayOutput(StreamProcessor &processor)
{
    count_x = processor.getXCount();
    count_y = processor.getYCount();
    count_z = processor.getZCount();
    parent_x = processor.getParentX();
    parent_y = processor.getParentY();
    parent_z = processor.getParentZ();
    slice = processor.getSlice();
    tag_table = processor.getTagTable();
}

void DisplayOutput::display_slice()
{
    vector<string> slice;
    string line;
    for (int z = 0; z < count_z; z++)
    {
        slice.clear(); // Clear the slice for each z-layer
        for (int y = 0; y < count_y; ++y)
        {
            int actual_y = count_y - 1 - y; // reverse y-index

            for (int x = 0; x < count_x; ++x)
            {
                char tag = slice[y][x]; // y is the index in slice (top-down)
                string label = tag_table[tag];

                cout << x << "," << actual_y << "," << z << ",1,1,1," << label << "\n";
            }
        }
        if (z < count_z - 1)
            getline(cin, line); // skip blank line
    }

    /*
    for (int z = 0; z < count_z; ++z)
    {
        for (int y = 0; y < count_y; ++y)
        {
            for (int x = 0; x < count_x; ++x)
            {
                char tag = slice[y][x];
                string label = tag_table[tag];
                cout << x << "," << y << "," << z << ",1,1,1," << label << "\n";
            }
        }
    }
        */
}

DisplayOutput::~DisplayOutput()
{
}

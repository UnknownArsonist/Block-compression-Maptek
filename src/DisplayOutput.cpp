#include "DisplayOutput.h"

DisplayOutput::DisplayOutput(){}

DisplayOutput::DisplayOutput(InputStreamReader &processor)
{
    count_x = processor.getXCount();
    count_y = processor.getYCount();
    count_z = processor.getZCount();
    parent_x = processor.getParentX();
    parent_y = processor.getParentY();
    parent_z = processor.getParentZ();
    tag_table = processor.getTagTable();
}

void DisplayOutput::display_slice()
{
    std::vector<std::string> slice;
    std::string line;
    for (int z = 0; z < count_z; z++)
    {
        slice.clear(); // Clear the slice for each z-layer
        for (int y = 0; y < count_y; ++y)
        {
            int actual_y = count_y - 1 - y; // reverse y-index

            for (int x = 0; x < count_x; ++x)
            {
                char tag = slice[y][x]; // y is the index in slice (top-down)
                std::string label = tag_table[tag];

                std::cout << x << "," << actual_y << "," << z << ",1,1,1," << label << "\n";
            }
        }
        if (z < count_z - 1)
            getline(std::cin, line); // skip blank line
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

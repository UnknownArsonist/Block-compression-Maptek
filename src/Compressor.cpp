#include "Compressor.h"

Compressor::Compressor()
{
}

Compressor::~Compressor() {}

void Compressor::compressParentBlock()
{

    std::vector<std::vector<std::vector<std::vector<char>>>> parent_blocks = {
        { // Block 0
         {// Slice 0
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'}},
         {// Slice 1
          {'s', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'s', 's', 'o', 'o', 'o', 'o', 'o', 'o'}}},
        { // Block 1
         {// Slice 0
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 't', 't', 't'},
          {'o', 'o', 'o', 'o', 't', 't', 't', 't'},
          {'o', 'o', 'o', 'o', 't', 't', 't', 'o'},
          {'o', 'o', 'o', 'o', 'o', 't', 't', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 't', 't'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'}},
         {// Slice 1
          {'o', 'o', 'o', 'o', 'o', 's', 's', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 's', 's'},
          {'o', 'o', 'o', 'o', 't', 't', 't', 't'},
          {'o', 'o', 'o', 'o', 't', 's', 's', 's'},
          {'o', 'o', 'o', 'o', 'o', 't', 't', 't'},
          {'o', 'o', 'o', 'o', 'o', 'o', 't', 't'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 's', 's', 'o'}}},
        { // Block 2
         {// Slice 0
          {'o', 't', 't', 't', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 't', 't', 't', 't', 'o'},
          {'o', 'o', 'o', 'o', 't', 't', 'o', 'o'},
          {'o', 'o', 'o', 'o', 't', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 't', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 't', 't', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'}},
         {// Slice 1
          {'o', 'o', 't', 't', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 's', 't', 't', 'o'},
          {'o', 'o', 'o', 'o', 't', 't', 't', 'o'},
          {'o', 'o', 'o', 'o', 't', 's', 'o', 'o'},
          {'o', 'o', 'o', 'o', 't', 't', 'o', 'o'},
          {'o', 'o', 'o', 'o', 't', 't', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 's', 's', 'o', 'o'}}},
        { // Block 3
         {// Slice 0
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'}},
         {// Slice 1
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'},
          {'o', 'o', 'o', 'o', 'o', 'o', 'o', 'o'}}}};
    /*
        oo s o
        oo o o
        ss o o
        oooo
    */
    processParentBlocks(parent_blocks[1]);
    // printParentBlock(parent_blocks);
}

void Compressor::processParentBlocks(const std::vector<std::vector<std::vector<char>>> &sub_blocks)
{
    int x = 0;
    int y = 0;
    int z = 0;

    int parent_x = 8;
    int parent_y = 8;
    int parent_z = 2;
    char target;

    // char target = sub_blocks[z][x][y]; // Assuming the first character is the target

    /*
       === Block 0 ===
        Slice 0:
        oooooooo
        oooooooo
        oooooooo
        oooooooo
        oooooooo
        oooooooo
        oooooooo
        oooooooo

        Slice 1:
        sooooooo
        oooooooo
        oooooooo
        oooooooo
        oooooooo
        oooooooo
        oooooooo
        ssoooooo
   */
    while (z < parent_z)
    {

        while (z < parent_z)
        {
            target = sub_blocks[z][y][x]; // Current char

            bool neighbors_match = false;

            // Check neighbors only if in bounds
            bool has_right = (x + 1) < parent_x;
            bool has_down = (y + 1) < parent_y;

            if (has_right && has_down)
            {
                if (target == sub_blocks[z][y][x + 1] && target == sub_blocks[z][y + 1][x])
                    neighbors_match = true;
            }
            else if (!has_right && has_down) // right neighbor doesn't exist but down neighbor exists
            {
                if (target == sub_blocks[z][y + 1][x])
                    neighbors_match = true;
            }
            else if (has_right && !has_down) // down neighbor doesn't exist but right neighbor exists
            {
                if (target == sub_blocks[z][y][x + 1])
                    neighbors_match = true;
            }
            // If neither neighbor exists (bottom-right corner), no match
            else if (!has_right && !has_down) // down neighbor doesn't exist but right neighbor exists
            {
                if (target == sub_blocks[z][parent_y - 1][parent_x - 1])
                    neighbors_match = true;
            }

            if (neighbors_match)
            {
                printf("Processing character '%c' at coordinates (%d, %d, %d)\n", target, z, y, x);
            }

            // Increment coordinates properly
            x++;
            if (x == parent_x)
            {
                x = 0;
                y++;
                if (y == parent_y)
                {
                    y = 0;
                    z++;
                    printf("\n\n");
                }
            }
        }
    }
}

void Compressor::printParentBlock(const std::vector<std::vector<std::vector<std::vector<char>>>> &parent_blocks)
{
    for (size_t block_idx = 0; block_idx < parent_blocks.size(); ++block_idx)
    {
        std::cout << "=== Block " << block_idx << " ===\n";

        for (size_t slice_idx = 0; slice_idx < parent_blocks[block_idx].size(); ++slice_idx)
        {
            std::cout << "Slice " << slice_idx << ":\n";
            for (const auto &row : parent_blocks[block_idx][slice_idx])
            {
                for (char cell : row)
                {
                    printf("%c", cell);
                }
                printf("\n");
            }
            printf("\n");
        }
        printf("-----\n");
    }
}

void Compressor::compressStream()
{
    int pop_check;
    char *parent_block;
    do
    {
        pop_check = -1;
        do
        {
            pop_check = input_stream->pop((void **)&parent_block);
        } while (pop_check == -1);

        if (parent_block == NULL)
            break;

        // Do Stuff Here

        /*
        printf("Parent Block: %p\n", parent_block);
        for (int z = 0; z < *parent_z; z++) {
            for (int y = 0; y < *parent_y; y++) {
                for (int x = 0; x < *parent_x; x++) {
                    printf("%c", parent_block[(x * *parent_y * *parent_z) + (y * *parent_z) + z]);
                }
                printf("\n");
            }
            printf("\n");
        }
        */
    } while (parent_block != NULL);
}

void Compressor::passValues(int *c_parent_x, int *c_parent_y, int *c_parent_z)
{
    parent_x = c_parent_x;
    parent_y = c_parent_y;
    parent_z = c_parent_z;
}

void Compressor::passBuffers(StreamBuffer *c_input_stream, StreamBuffer *c_output_stream)
{
    input_stream = c_input_stream;
    output_stream = c_output_stream;
}
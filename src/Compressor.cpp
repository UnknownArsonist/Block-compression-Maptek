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
    processParentBlocks(parent_blocks[0]);
    // printParentBlock(parent_blocks);
}

bool Compressor::isUniform(const std::vector<std::vector<char>> check_slice)
{
    char first = check_slice[0][0];
    for (size_t y = 0; y < check_slice.size(); y++)
    {
        for (size_t x = 0; x < check_slice[y].size(); x++)
        {
            if (check_slice[y][x] != first)
                return false;
        }
    }
    return true;
}

void Compressor::processParentBlocks(const std::vector<std::vector<std::vector<char>>> &sub_blocks)
{
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
    int z = 0;
    int parent_z = 2;
    while (z < parent_z)
    {
        if (isUniform(sub_blocks[z]) == true)
        {
            printf("0,0,0,%zu,%zu,%d,%c\n\n",
                   sub_blocks[0][0].size(), // X size
                   sub_blocks[0].size(),    // Y size
                   z, sub_blocks[0][0][0]); // Z size
        }
        else
        {
            int parent_x = sub_blocks[0][0].size();
            int parent_y = sub_blocks[0].size();
            int parent_z = sub_blocks.size();

            // marks visited cells
            std::vector<std::vector<std::vector<bool>>> visited(
                parent_z, std::vector<std::vector<bool>>(parent_y, std::vector<bool>(parent_x, false)));

            for (int y = 0; y < parent_y; y++)
            {
                for (int x = 0; x < parent_x; x++)
                {
                    if (visited[z][y][x])
                        continue;

                    char target = sub_blocks[z][y][x];

                    // Determine max size in X
                    int maxX = x;
                    while (maxX < parent_x && sub_blocks[z][y][maxX] == target && !visited[z][y][maxX])
                        maxX++;

                    // Determine max size in Y
                    int maxY = y;
                    bool uniformY = true; // checks if character in the y direction whether the target is same or not
                    while (maxY < parent_y && uniformY)
                    {
                        for (int xi = x; xi < maxX; xi++)
                        {
                            if (sub_blocks[z][maxY][xi] != target || visited[z][maxY][xi])
                            {
                                uniformY = false;
                                break;
                            }
                        }
                        if (uniformY)
                            maxY++;
                    }

                    // Determine max size in Z
                    int maxZ = z;
                    bool uniformZ = true;
                    while (maxZ < parent_z && uniformZ)
                    {
                        for (int yi = y; yi < maxY; yi++)
                        {
                            for (int xi = x; xi < maxX; xi++)
                            {
                                if (sub_blocks[maxZ][yi][xi] != target || visited[maxZ][yi][xi])
                                {
                                    uniformZ = false;
                                    break;
                                }
                            }
                            if (!uniformZ)
                                break;
                        }
                        if (uniformZ)
                            maxZ++;
                    }

                    // Mark all as visited
                    for (int zz = z; zz < maxZ; zz++)
                        for (int yy = y; yy < maxY; yy++)
                            for (int xx = x; xx < maxX; xx++)
                                visited[zz][yy][xx] = true;

                    // Output the packed block
                    printf("%d,%d,%d,%d,%d,%d,%c\n", x, y, z, maxX - x, maxY - y, maxZ - z, target);
                }
            }
        }
        z++;
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
    ParentBlock *parent_block;
    do
    {
        pop_check = -1;
        do
        {
            pop_check = input_stream->pop((void **)&parent_block);
        } while (pop_check == -1);

        if (parent_block == NULL)
            break;
        char *block = parent_block->block;
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
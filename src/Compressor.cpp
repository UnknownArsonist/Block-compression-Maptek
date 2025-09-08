#include "StreamProcessor.h"

// Constructor & Deconstructor
StreamProcessor::Compressor::Compressor() {}
StreamProcessor::Compressor::~Compressor() {}

// -----------HELPER FUNCTIONS ------------- //
void StreamProcessor::Compressor::passValues(StreamProcessor *sp)
{

    parent_x = &(sp->parent_x);
    parent_y = &(sp->parent_y);
    parent_z = &(sp->parent_z);
    tag_table = &(sp->tag_table);
    input_stream = (sp->inputToCompressorBuffer);
    output_stream = (sp->compressorToOutputBuffer);
}

void StreamProcessor::Compressor::passBuffers(StreamBuffer *c_input_stream, StreamBuffer *c_output_stream)
{
    input_stream = c_input_stream;
    output_stream = c_output_stream;
}

// -----------ENDS HERE-------- ------------- //

// -----------MAIN FUNCTIONS-------- -------- //
// Algorithm 1

// Compression Algorithm
void StreamProcessor::Compressor::hybrid(ParentBlock *pb) {
    std::vector<std::vector<encode>> RLE;  
    RLE.resize(*parent_z);   // One vector per slice (z)

    if (pb->block == NULL) {
        pb->sub_blocks = (SubBlock **)malloc(sizeof(SubBlock*));
        SubBlock *sub_block = (SubBlock *)malloc(sizeof(SubBlock));
        sub_block->x = pb->x;
        sub_block->y = pb->y;
        sub_block->z = pb->z;
        sub_block->l = *parent_x;
        sub_block->w = *parent_y;
        sub_block->h = *parent_z;
        sub_block->tag = pb->first;
        pb->sub_blocks[0] = sub_block;
        pb->sub_block_num = 1;
    } else {
        // First build RLE row by row
        std::vector<std::vector<std::vector<encode>>> perRowRLE;
        perRowRLE.resize(*parent_z, std::vector<std::vector<encode>>(*parent_y));

        for (int z = 0; z < *parent_z; z++) {
            for (int y = 0; y < *parent_y; y++) {
                char current = 0;
                int count = 0;

                for (int x = 0; x < *parent_x; x++) {
                    int idx = (x * *parent_y * *parent_z) + (y * *parent_z) + z;
                    char val = pb->block[idx];

                    if (count == 0) {
                        current = val;
                        count = 1;
                    } else if (val == current) {
                        count++;
                    } else {
                        perRowRLE[z][y].push_back({current, count});
                        current = val;
                        count = 1;
                    }
                }
                if (count > 0) {
                    perRowRLE[z][y].push_back({current, count});
                }
            }
        }

        // Now flatten rows if possible
        for (int z = 0; z < *parent_z; z++) {
            bool all_same = true;
            for (int y = 1; y < *parent_y; y++) {
                if (perRowRLE[z][y] != perRowRLE[z][0]) {
                    all_same = false;
                    break;
                }
            }

            if (all_same) {
                // Flatten: multiply run lengths by number of rows
                for (auto e : perRowRLE[z][0]) {
                    RLE[z].push_back({e.character, e.length * (*parent_y)});
                }
            } else {
                // Keep row-by-row flattened into RLE[z]
                for (int y = 0; y < *parent_y; y++) {
                    RLE[z].insert(RLE[z].end(), perRowRLE[z][y].begin(), perRowRLE[z][y].end());
                }
            }
        }
        RLE_compression(RLE);
    }
}

void StreamProcessor::Compressor::irregularblock_compression(std::vector<std::vector<encode>> block, int row_count)
{
        std::vector<std::vector<char>> grid(row_count, std::vector<char>(*parent_x));

    // Decode RLE into grid
    for (int y = 0; y < row_count; y++) {
        int x = 0;
        for (const auto& rleItem : block[y]) {
            for (int i = 0; i < rleItem.length; i++) {
                if (x < *parent_x) {
                    grid[y][x] = rleItem.character;
                    x++;
                }
            }
        }
    }

    // Display the grid
    //std::cout << "Decoded Grid (" << row_count << " rows x " << *parent_x << " cols):\n";
    for (int y = 0; y < row_count-1; y++) {
        //std::cout << "Row " << y << ": ";
        for (int x = 0; x < *parent_x; x++) {
            std::cout << grid[y][x] << " ";
        }
        std::cout << "\n";
    }

    //compressParentBlock(ParentBlock *pb);
}

void StreamProcessor::Compressor::RLE_compression(std::vector<std::vector<encode>> RLE)
{   
    std::vector<std::vector<encode>> compressed;
    int row_count = 0;
    int length = 0;
    // prepare first row
    compressed.resize(1);
    for (int z = 0; z < *parent_z; z++){ 
        row_count = 0;
        length = 0;
        compressed.clear();
        compressed.resize(1);  // start with first row
        for (int i = 0; i < RLE[z].size();i++) {
            if(RLE[z][i].length == (*parent_x * *parent_y))
            {
                std::cout << RLE[z][i].character << "," << RLE[z][i].length << " " << "\n";
            }
            else
            {
                int row_len = RLE[z][i].length;
                compressed[row_count].push_back(RLE[z][i]);
                length += row_len;

                if (length == *parent_x) {
                    // finish this row
                    row_count++;
                    length = 0;
                    compressed.resize(row_count + 1);
                }
            }//else
        }//i
        if (!compressed.empty() && !compressed[0].empty()) {
            irregularblock_compression(compressed, row_count + 1);
        }
    }// z
    
}


// Compression Startup
void StreamProcessor::Compressor::compressStream()
{
    ParentBlock *parent_block;
    int block_count = 0;
    do
    {
        // fprintf(stderr, "Compressor: get val\n");
        input_stream->pop((void **)&parent_block);
        if (parent_block == nullptr)
            if (parent_block == nullptr)
            {
                // fprintf(stderr, "IN TO COMP END\n");
                output_stream->push(NULL);
                break;
            }
        block_count++;
        StreamProcessor::Compressor::hybrid(parent_block);
        // Safety check: if we've processed too many blocks, use simpler algorithm

    } while (parent_block != NULL);
}
// -----------ENDS HERE-------- ------------- //

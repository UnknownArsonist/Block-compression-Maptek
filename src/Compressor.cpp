#include "StreamProcessor.h"
#include "OctTreeNode.h"

StreamProcessor::Compressor::Compressor()
{
}

StreamProcessor::Compressor::~Compressor() {}

void StreamProcessor::Compressor::printParentBlock(const std::vector<std::vector<std::vector<std::vector<char>>>> &parent_blocks)
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
// -----------ENDS HERE-------- ------------- //

// -----------MAIN FUNCTIONS-------- -------- //
void StreamProcessor::Compressor::OctreeCompression(ParentBlock *parent_block)
{
    // Check if the parent block is uniform first
    OctTreeNode octTree;
    char uniformTag;
    if (octTree.isUniform(parent_block, 0, 0, 0, *parent_x, *parent_y, *parent_z, uniformTag))
    {
        // If uniform, output a single block
        SubBlock *sb = (SubBlock *)malloc(sizeof(SubBlock));
        sb->x = parent_block->x;
        sb->y = parent_block->y;
        sb->z = parent_block->z;
        sb->l = *parent_x;
        sb->w = *parent_y;
        sb->h = *parent_z;
        sb->tag = uniformTag;

        output_stream->push((void **)&sb);
        free(parent_block);
        return;
    }

    // Only use octree for non-uniform blocks
    OctTreeNode *root = octTree.buildContentDriven3D(*parent_block, 0, 0, 0, *parent_x, *parent_y, *parent_z);

    std::vector<SubBlock> subBlocks;
    octTree.collectSubBlocks(root, subBlocks, tag_table, parent_block->x, parent_block->y, parent_block->z);

    std::vector<SubBlock> mergedBlocks = octTree.mergeSubBlocks(subBlocks);

    // Limit the number of output blocks (safety check)
    const size_t MAX_BLOCKS_PER_PARENT = 256; // Adjust as needed
    if (mergedBlocks.size() > MAX_BLOCKS_PER_PARENT)
    {
        // Fall back to simpler compression if octree produces too many blocks
        processParentBlocks(parent_block);
    }
    else
    {
        for (auto &sb : mergedBlocks)
        {
            SubBlock *out = (SubBlock *)malloc(sizeof(SubBlock));
            *out = sb;
            output_stream->push((void **)&out);
        }
    }

    octTree.deleteTree(root);
    free(parent_block);
}
void StreamProcessor::Compressor::processParentBlocks(ParentBlock *parent_block)
{
    // std::cout << parent_block->block-
    
    //fprintf(stderr, "Compressor: %d,%d,%d,%s\n", parent_block->x, parent_block->y, parent_block->z, (*tag_table)[parent_block->first].c_str());

    if (parent_block->block == NULL) {
        parent_block->sub_blocks = (SubBlock **)malloc(sizeof(SubBlock*));
        SubBlock *sub_block = (SubBlock *)malloc(sizeof(SubBlock));
        sub_block->x = parent_block->x;
        sub_block->y = parent_block->y;
        sub_block->z = parent_block->z;
        sub_block->l = *parent_x;
        sub_block->w = *parent_y;
        sub_block->h = *parent_z;
        sub_block->tag = parent_block->first;
        parent_block->sub_blocks[0] = sub_block;
        parent_block->sub_block_num = 1;
        output_stream->push((void **)&parent_block);
    } else {
        //TODO: maybe use vector with smaller initial array size which can then be dynamically extended if necessary
        parent_block->sub_blocks = (SubBlock **)malloc(sizeof(SubBlock*) * *parent_x * *parent_y * *parent_z);
        std::vector<std::vector<std::vector<bool>>> visited(*parent_z, std::vector<std::vector<bool>>(*parent_y, std::vector<bool>(*parent_x, false)));
        for (int z = 0; z < *parent_z; z++)
        {
            for (int y = 0; y < *parent_y; y++)
            {
                for (int x = 0; x < *parent_x; x++)
                {
                    if (visited[z][y][x])
                        continue;

                    char target = parent_block->block[(x * *parent_y * *parent_z) + (y * *parent_z) + z];

                    // Determine max size in X
                    int maxX = x + 1; // 0 1 7
                    while (maxX < *parent_x && parent_block->block[(maxX * *parent_y * *parent_z) + (y * *parent_z) + z] == target && !visited[z][y][maxX])
                        maxX++; // 1 7

                    // Determine max size in Y
                    int maxY = y + 1;         // 0
                    while (maxY < *parent_y)
                    {
                        bool uniformY = true;
                        // x = 0 -> maxX = 1; 1 < 7
                        for (int xi = x; xi < maxX; xi++)
                        {
                            // x1 = 1
                            if (parent_block->block[(xi * *parent_y * *parent_z) + (maxY * *parent_z) + z] != target || visited[z][maxY][xi])
                            {
                                uniformY = false;
                                break;
                            }
                        }
                        if (!uniformY)
                            break;
                        maxY++;
                    }

                    // Determine max size in Z
                    int maxZ = z + 1;
                    // checks the subblocks, are they uniform and did we alreadly visit them.
                    while (maxZ < *parent_z)
                    {
                        bool uniformZ = true;
                        // y = 0 maxY = 1
                        for (int yi = y; yi < maxY; yi++)
                        {
                            // x = 0 MaxX = 1
                            for (int xi = x; xi < maxX; xi++)
                            {
                                if (parent_block->block[(xi * *parent_y * *parent_z) + (yi * *parent_z) + maxZ] != target || visited[maxZ][yi][xi])
                                {
                                    uniformZ = false;
                                    break;
                                }
                            }
                            if (!uniformZ)
                                break;
                        }
                        if (!uniformZ)
                            break;
                        maxZ++;
                    }

                    // Mark all as visited
                    for (int zz = z; zz < maxZ; zz++)
                        for (int yy = y; yy < maxY; yy++)
                            for (int xx = x; xx < maxX; xx++)
                                visited[zz][yy][xx] = true;

                    // Output the packed block
                    SubBlock *sub_block = (SubBlock *)malloc(sizeof(SubBlock));
                    sub_block->x = parent_block->x + x;
                    sub_block->y = parent_block->y + y;
                    sub_block->z = parent_block->z + z;
                    sub_block->l = maxX - x;
                    sub_block->w = maxY - y;
                    sub_block->h = maxZ - z;
                    sub_block->tag = target;
                    parent_block->sub_blocks[parent_block->sub_block_num] = sub_block;
                    parent_block->sub_block_num++;
                    //fprintf(stderr, "Compressor: %d,%d,%d,%s\n", sub_block->x, sub_block->y, sub_block->z, (*tag_table)[target].c_str());
                    //output_stream->push((void **)&parent_block);
                }
                // x += 1; x = 1; x = 2
            }
            // y = 1
        }
        output_stream->push((void **)&parent_block);
        free(parent_block->block);
    }
    /* SubBlock *sub_block = (SubBlock *)malloc(sizeof(SubBlock));
    int chunk = (parent_block->z / *parent_y);
    sub_block->x = chunk;
    sub_block->y = 0;
    sub_block->z = 0;
    sub_block->l = 0;
    sub_block->w = 0;
    sub_block->h = 0;
    sub_block->tag = 1;
    output_stream->push((void **)&sub_block); */
    //delete parent_block;
}

void StreamProcessor::Compressor::compressStream()
{
    ParentBlock *parent_block;
    int block_count = 0;

    do
    {
        //fprintf(stderr, "Compressor: get val\n");
        input_stream->pop((void **)&parent_block);

        if (parent_block == NULL)
        {
            //fprintf(stderr, "IN TO COMP END\n");
            output_stream->push(NULL);
            break;
        }

        block_count++;

        // Safety check: if we've processed too many blocks, use simpler algorithm
        processParentBlocks(parent_block);

    } while (parent_block != NULL);
}
// -----------ENDS HERE-------- ------------- //

// -----------HELPER FUNCTIONS ------------- //
void StreamProcessor::Compressor::passValues(int *c_parent_x, int *c_parent_y, int *c_parent_z, std::unordered_map<char, std::string> *c_tag_table)
{
    parent_x = c_parent_x;
    parent_y = c_parent_y;
    parent_z = c_parent_z;

    tag_table = c_tag_table;
}

void StreamProcessor::Compressor::passValues(StreamProcessor *sp) {
    
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

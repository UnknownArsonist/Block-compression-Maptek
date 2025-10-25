#include "StreamProcessor.h"
#include "OctTreeNode.h"

#define GET_INDEX(x, y, z, x_count, y_count) ((size_t)(z) * (size_t)(x_count) * (size_t)(y_count) + (size_t)(y) * (size_t)(x_count) + (size_t)(x))

StreamProcessor::Compressor::Compressor()
{
}

StreamProcessor::Compressor::~Compressor() {}


void StreamProcessor::Compressor::printParentBlock(const std::vector<std::vector<std::vector<std::vector<char>>>> &parent_blocks) {}

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
void StreamProcessor::Compressor::processChunk(Chunk *chunk) {

    chunk->sub_blocks = (SubBlock**)malloc(*x_count * *y_count * *parent_z * sizeof(SubBlock*));
    chunk->sub_block_num = 0;
    bool *zeros = (bool*)calloc(*parent_x, sizeof(bool));
    for (int py = 0; py < *y_count; py += *parent_y) {
        for (int px = 0; px < *x_count; px += *parent_x) {
            bool *visited = (bool*)calloc(*parent_x * *parent_y * *parent_z, sizeof(bool));
            //fprintf(stderr, "(%d, %d, %d) %d\n", px, py, chunk->id, current_parent_block);
            
            // Now run greedy meshing *inside* these boundaries.
            //TODO: maybe use vector with smaller initial array size which can then be dynamically extended if necessary
            //std::vector<std::vector<std::vector<bool>>> visited(*parent_z, std::vector<std::vector<bool>>(*parent_y, std::vector<bool>(*parent_x, false)));
            for (int z = 0; z < *parent_z; z++)
            {
                for (int y = 0; y < *parent_y; y++)
                {
                    for (int x = 0; x < *parent_x; x++)
                    {
                        if (visited[(z * *parent_y * *parent_x) + (y * *parent_x) + x])
                            continue;

                        char target = chunk->block[px + x + ((py + y) * *parent_x) + (z * *parent_x * *parent_y)];

                        // Determine max size in X
                        int maxX = x + 1; // 0 1 7
                        while (maxX < *parent_x && chunk->block[px + maxX + ((py + y) * *parent_x) + (z * *parent_x * *parent_y)] == target && !visited[(z * *parent_y * *parent_x) + (y * *parent_x) + maxX])
                            maxX++; // 1 7

                        int width = maxX - x;
                        // Determine max size in Y
                        int maxY = y + 1;         // 0
                        while (maxY < *parent_y)
                        {
                            bool uniformY = true;
                            // x = 0 -> maxX = 1; 1 < 7
                            if (memcmp(&(chunk->block[px + x + ((py + y) * *parent_x) + (z * *parent_x * *parent_y)]), &(chunk->block[px + x + ((py + maxY) * *parent_x) + (z * *parent_x * *parent_y)]), width) != 0 || memcmp(&(visited[(z * *parent_y * *parent_x) + (maxY * *parent_x) + x]), zeros, width) != 0) {
                                uniformY = false;
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
                                if (memcmp(&(chunk->block[px + x + ((py + y) * *x_count) + (z * *x_count * *y_count)]), &(chunk->block[px + x + ((py + yi) * *x_count) + (maxZ * *parent_x * *parent_y)]), width) != 0 || memcmp(&(visited[(maxZ * *parent_y * *parent_x) + (yi * *parent_x) + x]), zeros, width) != 0) {
                                    uniformZ = false;
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
                                    visited[(zz * *parent_y * *parent_x) + (yy * *parent_x) + xx] = true;

                        // Output the packed block
                        SubBlock *sub_block = (SubBlock *)malloc(sizeof(SubBlock));
                        sub_block->x = px + x;
                        sub_block->y = py + y;
                        sub_block->z = (chunk->id * *parent_z) + z;
                        sub_block->l = maxX - x;
                        sub_block->w = maxY - y;
                        sub_block->h = maxZ - z;
                        sub_block->tag = target;
                        chunk->sub_blocks[chunk->sub_block_num] = sub_block;
                        chunk->sub_block_num++;
                        //fprintf(stderr, "Compressor: %d,%d,%d,%s\n", sub_block->x, sub_block->y, sub_block->z, (*tag_table)[target].c_str());
                        //output_stream->push((void **)&parent_block);
                    }
                    // x += 1; x = 1; x = 2
                }
                // y = 1
            }
            free(visited);
            //fprintf(stderr, "check4\n");
        }
    }
    free(zeros);
    free(chunk->block);
    output_stream->push((void **)&(chunk));
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

                    char target = parent_block->block[x + (y * *parent_x) + (z * *parent_x * *parent_y)];

                    // Determine max size in X
                    int maxX = x + 1; // 0 1 7
                    while (maxX < *parent_x && parent_block->block[maxX + (y * *parent_x) + (z * *parent_x * *parent_y)] == target && !visited[z][y][maxX])
                        maxX++; // 1 7

                    int width = maxX - x;
                    // Determine max size in Y
                    int maxY = y + 1;         // 0
                    while (maxY < *parent_y)
                    {
                        bool uniformY = true;
                        // x = 0 -> maxX = 1; 1 < 7
                        if (memcmp(&(parent_block->block[x + (y * *parent_x) + (z * *parent_x * *parent_y)]), &(parent_block->block[x + (maxY * *parent_x) + (z * *parent_x * *parent_y)]), width) != 0) {
                            uniformY = false;
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
                            if (memcmp(&(parent_block->block[x + (y * *parent_x) + (z * *parent_x * *parent_y)]), &(parent_block->block[x + (yi * *parent_x) + (maxZ * *parent_x * *parent_y)]), width) != 0) {
                                uniformZ = false;
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
        free(parent_block->block);
        output_stream->push((void **)&parent_block);
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
    Chunk *chunk;

    do
    {
        //fprintf(stderr, "Compressor: get val\n");
        input_stream->pop((void **)&chunk);

        if (chunk == nullptr)
        {
            //fprintf(stderr, "IN TO COMP END\n");
            output_stream->push(NULL);
            break;
        }

        // Safety check: if we've processed too many blocks, use simpler algorithm
        processChunk(chunk);

    } while (chunk != NULL);
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
    x_count = &(sp->x_count);
    y_count = &(sp->y_count);
    z_count = &(sp->z_count);
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

#include "StreamProcessor.h"
#include "OctTreeNode.h"

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

// Algorithm 2
void StreamProcessor::Compressor::processParentBlocks(ParentBlock *parent_block)
{
    int z = 0;
    while (z < *parent_z)
    {
        // marks visited cells
        std::vector<std::vector<std::vector<bool>>> visited(
            *parent_z, std::vector<std::vector<bool>>(*parent_y, std::vector<bool>(*parent_x, false)));

        for (int y = 0; y < *parent_y; y++)
        {
            for (int x = 0; x < *parent_x; x++)
            {
                if (visited[z][y][x])
                    // from x = 1 -> x = 7 && y = 0 -> y = 7 because these are already visited
                    continue;

                char target = parent_block->block[(x * *parent_y * *parent_z) + (y * *parent_z) + z];

                // Determine max size in X
                int maxX = x + 1; // 0 1 7
                while (maxX < *parent_x && parent_block->block[(maxX * *parent_y * *parent_z) + (y * *parent_z) + z] == target && !visited[z][y][maxX])
                    maxX++; // 1 7

                // Determine max size in Y
                int maxY = y;         // 0
                bool uniformY = true; // checks if character in the y direction whether the target is same or not
                while (maxY < *parent_y && uniformY)
                {
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
                    if (uniformY)
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

                output_stream->push((void **)&sub_block);
                // printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%s\n", parent_block->x, parent_block->y, parent_block->z, x, y, z, maxX, maxY, maxZ, (*tagTable)[target].c_str());
            }
            // x += 1; x = 1; x = 2
        }
        // y = 1
        z++;
    }
    free(parent_block);
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

        // Safety check: if we've processed too many blocks, use simpler algorithm
        OctreeCompression(parent_block);

    } while (parent_block != NULL);
}
// -----------ENDS HERE-------- ------------- //

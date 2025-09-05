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
        processParentBlocks(parent_block);

    } while (parent_block != NULL);
}
// -----------ENDS HERE-------- ------------- //

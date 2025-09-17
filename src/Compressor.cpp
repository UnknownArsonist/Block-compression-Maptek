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
    char *slab_data = chunk->block;
    std::vector<bool> visited(*x_count * *y_count * *parent_z, false);

    chunk->sub_blocks = (SubBlock**)malloc(*x_count * *y_count * *parent_z * sizeof(SubBlock*));
    chunk->sub_block_num = 0;
    for (int py = 0; py < *y_count; py += *parent_y) {
        for (int px = 0; px < *x_count; px += *parent_x) {
            //fprintf(stderr, "(%d, %d, %d) %d\n", px, py, chunk->id, current_parent_block);
            
            //fprintf(stderr, "Compressor: %d,%d,%d,%s\n", px, py, pz, (*tag_table)[chunk->block[0]].c_str());
            // Define the absolute boundaries for this parent block.
            const int parent_x_end = std::min(px + *parent_x, *x_count);
            const int parent_y_end = std::min(py + *parent_y, *y_count);
            const int parent_z_end = *parent_z;
            // Now run greedy meshing *inside* these boundaries.
            for (int z = 0; z < parent_z_end; ++z) {
                for (int y = py; y < parent_y_end; ++y) {
                    for (int x = px; x < parent_x_end; ++x) {
                        size_t current_idx = GET_INDEX(x, y, z, *x_count, *y_count);
                        //fprintf(stderr, "%d, (%d, %d, %d): %d\n", (int)current_idx, x, y, z, (bool)visited[current_idx]);
                        if (visited[current_idx]) continue;

                        char current_tag = slab_data[current_idx];
                        int best_w = 1, best_h = 1, best_d = 1; // Default to 1x1x1
                        size_t max_volume = 0;

                        // --- OPTIMIZATION: Try all 6 expansion orders ---
                        for (int order = 0; order < 6; ++order) {
                            int w = 1, h = 1, d = 1;

                            // The primary axis of expansion
                            int u_end, v_end, w_coord_end;
                            int u_start, v_start, w_coord_start;

                            // Remap axes based on current order
                            switch(order) {
                                case 0: u_start=x; v_start=y; w_coord_start=z; u_end=parent_x_end; v_end=parent_y_end; w_coord_end=parent_z_end; break; // x,y,z
                                case 1: u_start=x; v_start=z; w_coord_start=y; u_end=parent_x_end; v_end=parent_z_end; w_coord_end=parent_y_end; break; // x,z,y
                                case 2: u_start=y; v_start=x; w_coord_start=z; u_end=parent_y_end; v_end=parent_x_end; w_coord_end=parent_z_end; break; // y,x,z
                                case 3: u_start=y; v_start=z; w_coord_start=x; u_end=parent_y_end; v_end=parent_z_end; w_coord_end=parent_x_end; break; // y,z,x
                                case 4: u_start=z; v_start=x; w_coord_start=y; u_end=parent_z_end; v_end=parent_x_end; w_coord_end=parent_y_end; break; // z,x,y
                                case 5: u_start=z; v_start=y; w_coord_start=x; u_end=parent_z_end; v_end=parent_y_end; w_coord_end=parent_x_end; break; // z,y,x
                            }

                            // Expand in the first dimension
                            while(u_start + w < u_end) {
                                int cur_x, cur_y, cur_z;
                                switch(order){
                                    case 0: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(u_start + w, v_start, w_coord_start); break;
                                    case 1: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(u_start + w, w_coord_start, v_start); break;
                                    case 2: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(v_start, u_start + w, w_coord_start); break;
                                    case 3: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(w_coord_start, u_start + w, v_start); break;
                                    case 4: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(v_start, w_coord_start, u_start + w); break;
                                    case 5: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(w_coord_start, v_start, u_start + w); break;
                                }
                                if(visited[GET_INDEX(cur_x, cur_y, cur_z, *x_count, *y_count)] || slab_data[GET_INDEX(cur_x, cur_y, cur_z, *x_count, *y_count)] != current_tag) break;
                                w++;
                            }

                            // Expand in the second dimension
                            bool can_expand_v = true;
                            while(v_start + h < v_end && can_expand_v) {
                                for(int i = 0; i < w; ++i) {
                                    int cur_x, cur_y, cur_z;
                                    switch(order){
                                        case 0: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(u_start + i, v_start + h, w_coord_start); break;
                                        case 1: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(u_start + i, w_coord_start, v_start + h); break;
                                        case 2: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(v_start + h, u_start + i, w_coord_start); break;
                                        case 3: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(w_coord_start, u_start + i, v_start + h); break;
                                        case 4: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(v_start + h, w_coord_start, u_start + i); break;
                                        case 5: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(w_coord_start, v_start + h, u_start + i); break;
                                    }
                                    if(visited[GET_INDEX(cur_x, cur_y, cur_z, *x_count, *y_count)] || slab_data[GET_INDEX(cur_x, cur_y, cur_z, *x_count, *y_count)] != current_tag) { can_expand_v = false; break; }
                                }
                                if(can_expand_v) h++;
                            }
                            
                            // Expand in the third dimension
                            bool can_expand_w = true;
                            while(w_coord_start + d < w_coord_end && can_expand_w) {
                                for(int j = 0; j < h; ++j) {
                                    for(int i = 0; i < w; ++i) {
                                        int cur_x, cur_y, cur_z;
                                        switch(order){
                                            case 0: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(u_start + i, v_start + j, w_coord_start + d); break;
                                            case 1: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(u_start + i, w_coord_start + d, v_start + j); break;
                                            case 2: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(v_start + j, u_start + i, w_coord_start + d); break;
                                            case 3: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(w_coord_start + d, u_start + i, v_start + j); break;
                                            case 4: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(v_start + j, w_coord_start + d, u_start + i); break;
                                            case 5: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(w_coord_start + d, v_start + j, u_start + i); break;
                                        }
                                        if(visited[GET_INDEX(cur_x, cur_y, cur_z, *x_count, *y_count)] || slab_data[GET_INDEX(cur_x, cur_y, cur_z, *x_count, *y_count)] != current_tag) { can_expand_w = false; break; }
                                    }
                                    if(!can_expand_w) break;
                                }
                                if(can_expand_w) d++;
                            }
                            
                            size_t volume = (size_t)w * h * d;
                            if (volume > max_volume) {
                                max_volume = volume;
                                switch(order) {
                                    case 0: case 1: std::tie(best_w, best_h, best_d) = std::make_tuple(w,h,d); break;
                                    case 2: case 3: std::tie(best_w, best_h, best_d) = std::make_tuple(h,w,d); break;
                                    case 4: case 5: std::tie(best_w, best_h, best_d) = std::make_tuple(h,d,w); break;
                                }
                                    switch(order) {
                                    case 0: std::tie(best_w, best_h, best_d) = std::make_tuple(w,h,d); break;
                                    case 1: std::tie(best_w, best_h, best_d) = std::make_tuple(w,d,h); break;
                                    case 2: std::tie(best_w, best_h, best_d) = std::make_tuple(h,w,d); break;
                                    case 3: std::tie(best_w, best_h, best_d) = std::make_tuple(d,w,h); break;
                                    case 4: std::tie(best_w, best_h, best_d) = std::make_tuple(h,d,w); break;
                                    case 5: std::tie(best_w, best_h, best_d) = std::make_tuple(d,h,w); break;
                                }
                            }
                        }

                        // Mark all voxels in the new best block as visited.
                        for (int dz = 0; dz < best_d; ++dz) {
                            for (int dy = 0; dy < best_h; ++dy) {
                                for (int dx = 0; dx < best_w; ++dx) {
                                    visited[GET_INDEX(x + dx, y + dy, z + dz, *x_count, *y_count)] = true;
                                }
                            }
                        }
                        //fprintf(stderr, "cb: %d\n", current_parent_block);
                        
                        //fprintf(stderr, "check6\n");
                        /* std::cerr << current_tag << std::endl;
                        std::cerr << *tag_to_label.at(current_tag) << std::endl; */
                        SubBlock *sub_block = (SubBlock *)malloc(sizeof(SubBlock));
                        if (sub_block == NULL) {
                            fprintf(stderr, "ERROR NO MEMORY\n");
                            return;
                        }
                        sub_block->x = x;
                        sub_block->y = y;
                        sub_block->z = z + (chunk->id * *parent_z);
                        sub_block->l = best_w;
                        sub_block->w = best_h;
                        sub_block->h = best_d;
                        sub_block->tag = current_tag;
                        //fprintf(stderr, "%d (%d, %d, %d), %d\n", current_parent_block, x, y, z + chunk->z, chunk->sub_block_num);
                        chunk->sub_blocks[chunk->sub_block_num] = sub_block;
                        //fprintf(stderr, "check2\n");
                        chunk->sub_block_num++;
                    }
                }
            }
            //fprintf(stderr, "check4\n");
        }
    }
    free(chunk->block);
    output_stream->push((void **)&(chunk));
}

void StreamProcessor::Compressor::processParentBlocks(ParentBlock *parent_block)
{
    // std::cout << parent_block->block-
    
    //fprintf(stderr, "Compressor: %d,%d,%d,%s\n", parent_block->x, parent_block->y, parent_block->z, (*tag_table)[parent_block->first].c_str());

    if (parent_block->block == NULL) {
        //parent_block->sub_blocks = (SubBlock **)malloc(sizeof(SubBlock*));
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
        //parent_block->sub_blocks = (SubBlock **)malloc(sizeof(SubBlock*) * *parent_x * *parent_y * *parent_z);
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

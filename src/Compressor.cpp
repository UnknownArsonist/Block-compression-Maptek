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

// it is used to the max_height and max_width, which will be used later to find the maximum size of the rectangle(2D)
static bool hasRunAt(const std::vector<Run> &runs, int x, char label)
{
    for (const auto &run : runs)
    {
        if (x >= run.x && x < run.x + run.len && run.label == label)
        {
            return true;
        }
    }
    return false;
}

// Efficient validation without full 3D array
/*
static void validateCoverageEfficient(const std::vector<Cuboid> &cuboids, ParentBlock *pb,
                                      int parent_x, int parent_y, int parent_z)
{
    uint64_t total_blocks = parent_x * parent_y * parent_z;
    uint64_t covered_blocks = 0;

    for (const auto &c : cuboids)
    {
        covered_blocks += c.w * c.h * c.d;
    }

    if (covered_blocks != total_blocks)
    {
        std::cerr << "ERROR: Coverage incomplete! " << covered_blocks
                  << " blocks covered out of " << total_blocks << std::endl;
    }
    else
    {
        std::cout << "Validation passed: All " << total_blocks << " blocks covered" << std::endl;
    }

    // Quick overlap check
    for (size_t i = 0; i < cuboids.size(); i++)
    {
        for (size_t j = i + 1; j < cuboids.size(); j++)
        {
            const auto &a = cuboids[i];
            const auto &b = cuboids[j];

            // Check for overlap in 3D space
            bool x_overlap = (a.x < b.x + b.w) && (a.x + a.w > b.x);
            bool y_overlap = (a.y < b.y + b.h) && (a.y + a.h > b.y);
            bool z_overlap = (a.z < b.z + b.d) && (a.z + a.d > b.z);

            if (x_overlap && y_overlap && z_overlap)
            {
                std::cerr << "WARNING: Potential overlap between cuboids " << i << " and " << j << std::endl;
            }
        }
    }
}
*/

// Print the block
/*
void StreamProcessor::Compressor::printCuboidsWithLegend(
    std::vector<Cuboid> &cuboids,
    std::unordered_map<char, std::string> &legend, )
{
    for (const auto &c : cuboids)
    {
        
        // Normalize the label (ensure unsigned->signed conversion)
        char lookupKey = static_cast<char>(static_cast<unsigned char>(c.label));

        // Find in legend
        auto it = legend.find(lookupKey);

        // If not found, print "UNKNOWN"
        const char *labelStr = "UNKNOWN";
        if (it != legend.end())
        {
            // Remove any trailing '\r' for Windows CRLF
            std::string val = it->second;
            if (!val.empty() && val.back() == '\r')
                val.pop_back();
            labelStr = val.c_str();
        }

        SubBlock *sub_block = (SubBlock *)malloc(sizeof(SubBlock));
        sub_block->x = c.x;
        sub_block->y = c.y;
        sub_block->z = c.z;
        sub_block->l = c.w;
        sub_block->w = c.h;
        sub_block->h = c.d;
        sub_block->tag = c.label;

        SubBlock *sub_block = (SubBlock *)malloc(sizeof(SubBlock));
        sub_block->x = c.x;
        sub_block->y = c.y;
        sub_block->z = c.z;
        sub_block->l = c.w;
        sub_block->w = c.h;
        sub_block->h = c.d;
        sub_block->tag = c.label;
        pb->sub_blocks[pb->sub_block_num] = sub_block;
        pb->sub_block_num++;
        
        printf("%d,%d,%d,%d,%d,%d,%s\n",
               c.x, c.y, c.z, c.w, c.h, c.d, labelStr);

    }
    free(parent_block->block);
    output_stream->push((void **)&parent_block);
}
*/

// -----------ENDS HERE-------- ------------- //

// -----------MAIN FUNCTIONS-------- -------- //
// Compression Algorithm
void StreamProcessor::Compressor::compressParentBlock(ParentBlock *pb)
{
    if (pb == nullptr) return;

    // If there is no data (block == NULL), treat as uniform block with tag pb->first
    if (pb->block == NULL)
    {
        pb->sub_blocks = (SubBlock **)malloc(sizeof(SubBlock *));
        if (!pb->sub_blocks) {
            // handle allocation failure gracefully
            fprintf(stderr, "malloc failed for sub_blocks\n");
            return;
        }
        SubBlock *sub_block = (SubBlock *)malloc(sizeof(SubBlock));
        if (!sub_block) {
            fprintf(stderr, "malloc failed for sub_block\n");
            free(pb->sub_blocks);
            return;
        }
        sub_block->x = pb->x;
        sub_block->y = pb->y;
        sub_block->z = pb->z;
        sub_block->l = static_cast<int>(*parent_x);
        sub_block->w = static_cast<int>(*parent_y);
        sub_block->h = static_cast<int>(*parent_z);
        sub_block->tag = pb->first;
        pb->sub_blocks[0] = sub_block;
        pb->sub_block_num = 1;
        output_stream->push((void **)&pb);
        return;
    }

    // Reserve worst-case space for sub-block pointers (one cell per original voxel)
    size_t max_subblocks = static_cast<size_t>(*parent_x) * static_cast<size_t>(*parent_y) * static_cast<size_t>(*parent_z);
    pb->sub_blocks = (SubBlock **)malloc(sizeof(SubBlock *) * max_subblocks);
    if (!pb->sub_blocks) {
        fprintf(stderr, "malloc failed for sub_blocks\n");
        return;
    }
    pb->sub_block_num = 0;

    // Stage 1: compute runs along X for each (z,y)
    std::vector<std::vector<std::vector<Run>>> runs(
        *parent_z, std::vector<std::vector<Run>>(*parent_y));

    for (int z = 0; z < *parent_z; z++)
    {
        for (int y = 0; y < *parent_y; y++)
        {
            int x = 0;
            while (x < *parent_x)
            {
                // index for (x,y,z) : ((x * parent_y) + y) * parent_z + z
                // to match your linearization: (x * parent_y * parent_z) + (y * parent_z) + z
                char label = pb->block[( (x * (*parent_y) * (*parent_z)) ) + (y * (*parent_z)) + z];
                int len = 1;
                // ===== FIXED =====: parentheses so (x + len) is multiplied, not x + (len * ...)
                while (x + len < *parent_x &&
                       pb->block[( ((x + len) * (*parent_y) * (*parent_z)) ) + (y * (*parent_z)) + z] == label)
                {
                    len++;
                }
                runs[z][y].push_back({x, len, label});
                x += len;
            }
        }
    }

    // Stage 2: find optimal rectangles (per-slice)
    std::vector<std::vector<Rect>> rects(*parent_z);

    // allocate per-slice "covered" grid and compute rectangles per slice independently
    for (int z = 0; z < *parent_z; z++)
    {
        // per-slice covered grid (reset for each z)
        std::vector<std::vector<bool>> covered(*parent_y, std::vector<bool>(*parent_x, false));
        int remaining_blocks = (*parent_x) * (*parent_y);

        while (remaining_blocks > 0)
        {
            OptimalRect best_rect = {0, 0, 0, 0, '\0'};
            int max_area = 0;

            for (int y = 0; y < *parent_y; y++)
            {
                for (int x = 0; x < *parent_x; x++)
                {
                    if (covered[y][x]) continue;

                    // find label at (x,y) by scanning runs
                    char current_label = '\0';
                    for (const auto &run : runs[z][y])
                    {
                        if (x >= run.x && x < run.x + run.len)
                        {
                            current_label = run.label;
                            break;
                        }
                    }
                    if (current_label == '\0') continue;

                    // find max width at this row
                    int max_width = *parent_x - x;
                    for (int dx = 0; dx < max_width; dx++)
                    {
                        if (covered[y][x + dx] || !hasRunAt(runs[z][y], x + dx, current_label))
                        {
                            max_width = dx;
                            break;
                        }
                    }
                    if (max_width == 0) continue;

                    // find max height with consistent width
                    int max_height = *parent_y - y;
                    for (int dy = 0; dy < max_height; dy++)
                    {
                        bool ok = true;
                        for (int dx = 0; dx < max_width; dx++)
                        {
                            if (covered[y + dy][x + dx] || !hasRunAt(runs[z][y + dy], x + dx, current_label))
                            {
                                ok = false;
                                break;
                            }
                        }
                        if (!ok)
                        {
                            max_height = dy;
                            break;
                        }
                    }
                    if (max_height == 0) continue;

                    int area = max_width * max_height;
                    if (area > max_area)
                    {
                        max_area = area;
                        best_rect = {x, y, max_width, max_height, current_label};
                    }
                }
            } // end scanning x,y

            if (max_area == 0) break; // nothing found

            // record best rectangle for this z
            rects[z].push_back({best_rect.x, best_rect.y, best_rect.w, best_rect.h, best_rect.label});

            // mark covered
            for (int dy = 0; dy < best_rect.h; dy++)
                for (int dx = 0; dx < best_rect.w; dx++)
                    covered[best_rect.y + dy][best_rect.x + dx] = true;

            remaining_blocks -= max_area;
        } // end while remaining_blocks
    } // end for each z

    // Stage 3: merge rectangles across z into cuboids (sub-blocks)
    std::vector<std::vector<bool>> processed_z(*parent_z);
    for (int z = 0; z < *parent_z; z++)
        processed_z[z].resize(rects[z].size(), false);

    for (int z = 0; z < *parent_z; z++)
    {
        for (size_t i = 0; i < rects[z].size(); i++)
        {
            if (processed_z[z][i]) continue;
            auto &r = rects[z][i];
            int d = 1;

            // check subsequent slices
            while (z + d < *parent_z)
            {
                bool found = false;
                for (size_t j = 0; j < rects[z + d].size(); j++)
                {
                    if (processed_z[z + d][j]) continue;
                    auto &cand = rects[z + d][j];
                    if (cand.x == r.x && cand.y == r.y && cand.w == r.w && cand.h == r.h && cand.label == r.label)
                    {
                        found = true;
                        processed_z[z + d][j] = true;
                        break;
                    }
                }
                if (!found) break;
                d++;
            }

            // create subblock for merged cuboid (x,y,z .. width=w, height=h, depth=d)
            SubBlock *sub_block = (SubBlock *)malloc(sizeof(SubBlock));
            if (!sub_block) {
                fprintf(stderr, "malloc failed for sub_block\n");
                // continue but don't crash; might leak previously allocated memory
                continue;
            }
            sub_block->x = pb->x + r.x;
            sub_block->y = pb->y + r.y;
            sub_block->z = pb->z + z;
            sub_block->l = r.w; // length in x
            sub_block->w = r.h; // width in y
            sub_block->h = d;   // depth in z
            sub_block->tag = r.label;

            // safe guard: ensure we don't overflow pb->sub_blocks array
            if (pb->sub_block_num < static_cast<int>(max_subblocks)) {
                pb->sub_blocks[pb->sub_block_num++] = sub_block;
            } else {
                // out of space - should not happen if max_subblocks was computed correctly
                fprintf(stderr, "sub_blocks buffer overflow\n");
                free(sub_block);
            }

            processed_z[z][i] = true;
        }
    }

    // free original block memory if owned here
    free(pb->block);
    pb->block = NULL;

    output_stream->push((void **)&pb);
    }
    // Fast validation (comment out for production)
    // validateCoverageEfficient(cuboids, pb, parent_x, parent_y, parent_z);


//  Startup
void StreamProcessor::Compressor::compressStream()
{
    ParentBlock *parent_block;
    int block_count = 0;
    //std::vector<Cuboid> Compressedcube;
    // printTagTable(tagTable);
    do
    {
        // fprintf(stderr, "Compressor: get val\n");
        input_stream->pop((void **)&parent_block);

        if (parent_block == nullptr){
            
            // fprintf(stderr, "IN TO COMP END\n");
            output_stream->push(NULL);
            break;
        }
            

        block_count++;

        StreamProcessor::Compressor::compressParentBlock(parent_block);
        // printCuboidsWithLegend(Compressedcube, *tag_table);
    } while (parent_block != NULL);
}
// -----------ENDS HERE-------- ------------- //
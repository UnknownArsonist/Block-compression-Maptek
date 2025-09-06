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
    if (pb->block == NULL)
    {
        pb->sub_blocks = (SubBlock **)malloc(sizeof(SubBlock *));
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
        output_stream->push((void **)&pb);
        return;
    }
    else
    {
        pb->sub_blocks = (SubBlock **)malloc(sizeof(SubBlock*) * *parent_x * *parent_y * *parent_z);
        //std::vector<Cuboid> cuboids;
        
        // Stage 1: compress along X (runs per row per slice)
        // 3D vector to tract each run in x-axis
        std::vector<std::vector<std::vector<Run>>> runs(
            *parent_z, std::vector<std::vector<Run>>(*parent_y));

        // get index of the character of the the parent block
        auto idx = [&](int x, int y, int z)
        {
            return (x * *parent_y * *parent_z) + (y * *parent_z) + z;
        };

        for (int z = 0; z < *parent_z; z++)
        {
            for (int y = 0; y < *parent_y; y++)
            {
                int x = 0;
                while (x < *parent_x)
                {
                    char label = pb->block[idx(x, y, z)]; // parent_block[0][0][0] (i.e the first character)
                    int len = 1;
                    // compares the nth character and (n + 1)th character
                    while (x + len < *parent_x && pb->block[idx(x + len, y, z)] == label)
                    {
                        len++; // increase len if they are equal
                    }
                    // keeping a record of maximum length of indentical characters in x-axis
                    runs[z][y].push_back({x, len, label});
                    // changing the x value to ensure each run is properly recorded
                    x += len;
                }
            }
        }

        // Stage 2: OPTIMAL rectangle finding with maximum area
        std::vector<std::vector<Rect>> rects(*parent_z);

        // 2D boolean grid with size of parent_y * parent_x and setting the value to false
        // used for checking whether a character has been visited or not    
        std::vector<std::vector<bool>> covered(
                *parent_y, std::vector<bool>(*parent_x, false));
                
        std::vector<std::vector<bool>> processed_z(*parent_z);

        int remaining_blocks;
        int max_width;
        char current_label;
        int max_area;
        int max_height;
        int area;
        SubBlock *sub_block;

        for (int z = 0; z < *parent_z; z++)
        {
            // size of each slice of parent_block
            remaining_blocks = *parent_x * *parent_y;

            while (remaining_blocks > 0)
            {
                OptimalRect best_rect = {0, 0, 0, 0, ' '};
                max_area = 0;

                // Find the largest possible rectangle starting from each uncovered position
                for (int y = 0; y < *parent_y; y++)
                {
                    for (int x = 0; x < *parent_x; x++)
                    {
                        if (covered[y][x])
                            continue;

                        // Get the label at this position
                        current_label = ' ';
                        for (const auto &run : runs[z][y])
                        {
                            if (x >= run.x && x < run.x + run.len)
                            {
                                current_label = run.label; // according our map current_label is 'o'
                                // once the x has reached max len in that row or the character in that row changes then break
                                break;
                            }
                        }
                        if (current_label == ' ')
                            continue;

                        // Find maximum width at this row
                        max_width = *parent_x - x;
                        for (int dx = 0; dx < max_width; dx++)
                        {
                            if (covered[y][x + dx] || !hasRunAt(runs[z][y], x + dx, current_label))
                            {
                                max_width = dx;
                                break;
                            }
                        }

                        // Find maximum height with consistent width
                        max_height = *parent_y - y;
                        for (int dy = 0; dy < max_height; dy++)
                        {
                            for (int dx = 0; dx < max_width; dx++)
                            {
                                if (covered[y + dy][x + dx] ||
                                    !hasRunAt(runs[z][y + dy], x + dx, current_label))
                                {
                                    max_height = dy;
                                    break;
                                }
                            }
                            if (max_height == dy)
                                break;
                        }

                        // Calculate area and update best rectangle
                        area = max_width * max_height;
                        if (area > max_area)
                        {
                            max_area = area;
                            best_rect = {x, y, max_width, max_height, current_label};
                        }
                    }
                }

                if (max_area == 0)
                    break; // No more rectangles found

                // Add the best rectangle and mark as covered (keep tract of the maximum area of the 2D block containtaing identical character)
                // rects[z][j] - for each slice it contains all the rectangles
                rects[z].push_back({best_rect.x, best_rect.y, best_rect.w, best_rect.h, best_rect.label});

                for (int dy = 0; dy < best_rect.h; dy++)
                {
                    for (int dx = 0; dx < best_rect.w; dx++)
                    {
                        covered[best_rect.y + dy][best_rect.x + dx] = true;
                    }
                }
                // total area of parent_block - max_area(maximum area of the 2D block containtaing identical character)
                remaining_blocks -= max_area;
            }
        }

        // Stage 3: merge rectangles across slices into cuboids
        
        for (int z = 0; z < *parent_z; z++)
        {
            processed_z[z].resize(rects[z].size(), false);
        }

        for (int z = 0; z < *parent_z; z++)
        {
            for (size_t i = 0; i < rects[z].size(); i++)
            {
                if (processed_z[z][i])
                    continue;

                // pointing to the rectangles in each slices (ie rect[0][j] and rect[1][j] = slice 0 - slice 1, 1st rectangle..)
                auto &r = rects[z][i];
                int d = 1;

                // Check subsequent slices for identical rectangles
                while (z + d < *parent_z)
                {
                    bool found = false;
                    for (size_t j = 0; j < rects[z + d].size(); j++)
                    {
                        if (processed_z[z + d][j])
                            continue;

                        // inner loops checks if the the rectanges in the current and next slices are same or not
                        auto &candidate = rects[z + d][j];
                        if (candidate.x == r.x && candidate.y == r.y &&
                            candidate.w == r.w && candidate.h == r.h &&
                            candidate.label == r.label)
                        {
                            // if they are same then processed_z vector is updated
                            found = true;
                            // updating the next slice
                            processed_z[z + d][j] = true;
                            break;
                        }
                    }
                    if (!found)
                        break;
                    d++;
                }
                // pushing the resut to cuboids to outputing the results
                //cuboids.push_back({pb->x + r.x, pb->y + r.y, pb->z + z, r.w, r.h, d, r.label});
                sub_block = (SubBlock *)malloc(sizeof(SubBlock));
                sub_block->x = pb->x + r.x;
                sub_block->y = pb->y + r.y;
                sub_block->z = pb->z + z;
                sub_block->l = r.w;
                sub_block->w = r.h;
                sub_block->h = d;
                sub_block->tag =  r.label;
                pb->sub_blocks[pb->sub_block_num] = sub_block;
                pb->sub_block_num++;
                // updating the current slice
                processed_z[z][i] = true;
            }
        }
        free(pb->block);
        output_stream->push((void **)&pb);
    }
    // Fast validation (comment out for production)
    // validateCoverageEfficient(cuboids, pb, parent_x, parent_y, parent_z);
}

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
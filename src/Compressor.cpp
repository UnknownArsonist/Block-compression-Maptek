#include "../include/Compressor.h"
//#include <unistd.h>

Compressor::Compressor() {}
Compressor::~Compressor() {}

// -----------UNWANTED FUNCTIONS------------- //
// -----------ENDS HERE-------- ------------- //

// -----------MAIN FUNCTIONS-------- -------- //
void Compressor::compressStream()
{
    ParentBlock *parent_block;
    char *null_ptr = NULL;
    int block_count = 0;
    std::vector<Cuboid> Compressedcube;
    //printTagTable(tagTable);
    do
    {
        input_stream->pop((void **)&parent_block);

        if (parent_block == NULL)
        {
            output_stream->push((void **)&null_ptr);
            break;
        }

        block_count++;
            
        Compressedcube = compressParentBlock(parent_block, *parent_x, *parent_y, *parent_z);
        printCuboidsWithLegend(Compressedcube, *tagTable);
    } while (parent_block != NULL);
}
// -----------ENDS HERE-------- ------------- //

// -----------HELPER FUNCTIONS ------------- //
void Compressor::passValues(int *c_parent_x, int *c_parent_y, int *c_parent_z, std::unordered_map<char, std::string> *tag_table, int mx_count, int my_count, int mz_count)
{
    parent_x = c_parent_x;
    parent_y = c_parent_y;
    parent_z = c_parent_z;

    this->mx = mx;
    this->my = my;
    this->mz = mz;
    this->tagTable = tag_table;
}
void Compressor::passBuffers(StreamBuffer *c_input_stream, StreamBuffer *c_output_stream)
{
    input_stream = c_input_stream;
    output_stream = c_output_stream;
}
// -----------ENDS HERE-------- ------------- //

// Compress a single 2D slice into rectangles
std::vector<Cuboid> Compressor::compressParentBlock(ParentBlock *pb,
                                                    int parent_x, int parent_y, int parent_z)
{
    std::vector<Cuboid> cuboids;

    // Stage 1: compress along X (runs per row per slice)
  
    std::vector<std::vector<std::vector<Run>>> runs(
        parent_z, std::vector<std::vector<Run>>(parent_y));

    auto idx = [&](int x, int y, int z)
    {
        return z * parent_y * parent_x + y * parent_x + x;
    };

    for (int z = 0; z < parent_z; z++)
    {
        for (int y = 0; y < parent_y; y++)
        {
            int x = 0;
            while (x < parent_x)
            {
                char label = pb->block[idx(x, y, z)];
                int len = 1;
                while (x + len < parent_x && pb->block[idx(x + len, y, z)] == label)
                {
                    len++;
                }
                runs[z][y].push_back({x, len, label});
                x += len;
            }
        }
    }

    // Stage 2: merge runs vertically into rectangles - FIXED
    // Stage 2: PROPER vertical merging with exact coverage
    std::vector<std::vector<Rect>> rects(parent_z);
    std::vector<std::vector<std::vector<bool>>> covered(
        parent_z, std::vector<std::vector<bool>>(
            parent_y, std::vector<bool>(parent_x, false)));

    for (int z = 0; z < parent_z; z++)
    {
        for (int y = 0; y < parent_y; y++)
        {
            for (const auto& run : runs[z][y])
            {
                // Skip if this run is already covered
                if (covered[z][y][run.x]) continue;
                
                // Find maximum vertical extent with identical run pattern
                int max_h = 1;
                for (int test_y = y + 1; test_y < parent_y; test_y++)
                {
                    bool match_found = false;
                    for (const auto& test_run : runs[z][test_y])
                    {
                        if (test_run.x == run.x && 
                            test_run.len == run.len && 
                            test_run.label == run.label &&
                            !covered[z][test_y][test_run.x])
                        {
                            match_found = true;
                            break;
                        }
                    }
                    if (!match_found) break;
                    max_h++;
                }
                
                // Actually create the rectangle with the found height
                int actual_h = 1;
                for (int dy = 0; dy < max_h; dy++)
                {
                    int current_y = y + dy;
                    bool row_ok = true;
                    
                    // Check if this entire row segment is available and matches
                    for (int dx = 0; dx < run.len; dx++)
                    {
                        if (covered[z][current_y][run.x + dx])
                        {
                            row_ok = false;
                            break;
                        }
                        
                        // Verify the run actually exists at this position
                        bool run_found = false;
                        for (const auto& r : runs[z][current_y])
                        {
                            if (r.x <= run.x && r.x + r.len > run.x && 
                                r.label == run.label)
                            {
                                run_found = true;
                                break;
                            }
                        }
                        if (!run_found)
                        {
                            row_ok = false;
                            break;
                        }
                    }
                    
                    if (!row_ok) break;
                    actual_h = dy + 1;
                }
                
                // Create the rectangle and mark as covered
                if (actual_h > 0)
                {
                    rects[z].push_back({run.x, y, run.len, actual_h, run.label});
                    
                    // Mark all blocks in this rectangle as covered
                    for (int dy = 0; dy < actual_h; dy++)
                    {
                        for (int dx = 0; dx < run.len; dx++)
                        {
                            covered[z][y + dy][run.x + dx] = true;
                        }
                    }
                }
            }
        }
    }

    // Stage 3: merge rectangles across slices into cuboids
    // Stage 3: merge rectangles across slices into cuboids
    // Stage 3: merge rectangles across slices into cuboids - FIXED
    std::vector<std::vector<bool>> processed_z(parent_z);
    for (int z = 0; z < parent_z; z++)
    {
        processed_z[z].resize(rects[z].size(), false);
    }

    for (int z = 0; z < parent_z; z++)
    {
        for (size_t i = 0; i < rects[z].size(); i++)
        {
            if (processed_z[z][i]) continue;
            
            auto &r = rects[z][i];
            int d = 1;
            
            // Check subsequent slices for identical rectangles
            while (z + d < parent_z)
            {
                bool found = false;
                for (size_t j = 0; j < rects[z + d].size(); j++)
                {
                    if (processed_z[z + d][j]) continue;
                    
                    auto &candidate = rects[z + d][j];
                    if (candidate.x == r.x && candidate.y == r.y && 
                        candidate.w == r.w && candidate.h == r.h && 
                        candidate.label == r.label)
                    {
                        found = true;
                        processed_z[z + d][j] = true;
                        break;
                    }
                }
                if (!found) break;
                d++;
            }
            
            cuboids.push_back({pb->x + r.x, pb->y + r.y, pb->z + z, r.w, r.h, d, r.label});
            processed_z[z][i] = true;
        }
    }
    //validateCoverage(cuboids, pb, parent_x, parent_y, parent_z);
    return cuboids;
}
void Compressor::validateCoverage(const std::vector<Cuboid>& cuboids, ParentBlock* pb,
                     int parent_x, int parent_y, int parent_z)
{
    // Create coverage grid
    std::vector<std::vector<std::vector<bool>>> covered(
        parent_z, std::vector<std::vector<bool>>(
            parent_y, std::vector<bool>(parent_x, false)));
    
    int total_blocks = parent_x * parent_y * parent_z;
    int covered_blocks = 0;
    
    // Mark all covered blocks
    for (const auto& c : cuboids)
    {
        for (int dz = 0; dz < c.d; dz++)
        {
            for (int dy = 0; dy < c.h; dy++)
            {
                for (int dx = 0; dx < c.w; dx++)
                {
                    int x = c.x - pb->x + dx;
                    int y = c.y - pb->y + dy;
                    int z = c.z - pb->z + dz;
                    
                    if (x >= 0 && x < parent_x && y >= 0 && y < parent_y && z >= 0 && z < parent_z)
                    {
                        if (!covered[z][y][x])
                        {
                            covered[z][y][x] = true;
                            covered_blocks++;
                        }
                        else
                        {
                            std::cerr << "ERROR: Block at (" << x << "," << y << "," << z 
                                      << ") covered multiple times!" << std::endl;
                        }
                    }
                }
            }
        }
    }
    
    // Check for missing blocks
    if (covered_blocks != total_blocks)
    {
        std::cerr << "ERROR: Coverage incomplete! " << covered_blocks 
                  << " blocks covered out of " << total_blocks << std::endl;
        
        // Find missing blocks
        for (int z = 0; z < parent_z; z++)
        {
            for (int y = 0; y < parent_y; y++)
            {
                for (int x = 0; x < parent_x; x++)
                {
                    if (!covered[z][y][x])
                    {
                        std::cerr << "Missing block at: " << x << "," << y << "," << z << std::endl;
                    }
                }
            }
        }
    }
    else
    {
        std::cout << "Validation passed: All " << total_blocks << " blocks covered" << std::endl;
    }
} 

void Compressor::printCuboidsWithLegend(
     std::vector<Cuboid> &cuboids,
    std::unordered_map<char, std::string> &legend)
{
    for (const auto &c : cuboids)
    {
        // Normalize the label (ensure unsigned->signed conversion)
        char lookupKey = static_cast<char>(static_cast<unsigned char>(c.label));

        // Find in legend
        auto it = legend.find(lookupKey);

        // If not found, print "UNKNOWN"
        const char* labelStr = "UNKNOWN";
        if (it != legend.end())
        {
            // Remove any trailing '\r' for Windows CRLF
            std::string val = it->second;
            if (!val.empty() && val.back() == '\r') val.pop_back();
            labelStr = val.c_str();
        }

        printf("%d,%d,%d,%d,%d,%d,%s\n",
               c.x, c.y, c.z, c.w, c.h, c.d, labelStr);
    }
}

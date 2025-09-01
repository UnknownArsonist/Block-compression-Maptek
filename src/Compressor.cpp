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

bool Compressor::hasRunAt(const std::vector<Run>& runs, int x, char label) {
    for (const auto& run : runs) {
        if (x >= run.x && x < run.x + run.len && run.label == label) {
            return true;
        }
    }
    return false;
}

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

    // Stage 2: OPTIMAL rectangle finding with maximum area
    std::vector<std::vector<Rect>> rects(parent_z);
    
    for (int z = 0; z < parent_z; z++)
    {
        std::vector<std::vector<bool>> covered(
            parent_y, std::vector<bool>(parent_x, false));
        
        int remaining_blocks = parent_x * parent_y;
        
        while (remaining_blocks > 0)
        {
            OptimalRect best_rect = {0, 0, 0, 0, ' '};
            int max_area = 0;
            
            // Find the largest possible rectangle starting from each uncovered position
            for (int y = 0; y < parent_y; y++)
            {
                for (int x = 0; x < parent_x; x++)
                {
                    if (covered[y][x]) continue;
                    
                    // Get the label at this position
                    char current_label = ' ';
                    for (const auto& run : runs[z][y]) {
                        if (x >= run.x && x < run.x + run.len) {
                            current_label = run.label;
                            break;
                        }
                    }
                    if (current_label == ' ') continue;
                    
                    // Find maximum width at this row
                    int max_width = parent_x - x;
                    for (int dx = 0; dx < max_width; dx++) {
                        if (covered[y][x + dx] || !hasRunAt(runs[z][y], x + dx, current_label)) {
                            max_width = dx;
                            break;
                        }
                    }
                    
                    // Find maximum height with consistent width
                    int max_height = parent_y - y;
                    for (int dy = 0; dy < max_height; dy++) {
                        for (int dx = 0; dx < max_width; dx++) {
                            if (covered[y + dy][x + dx] || 
                                !hasRunAt(runs[z][y + dy], x + dx, current_label)) {
                                max_height = dy;
                                break;
                            }
                        }
                        if (max_height == dy) break;
                    }
                    
                    // Calculate area and update best rectangle
                    int area = max_width * max_height;
                    if (area > max_area) {
                        max_area = area;
                        best_rect = {x, y, max_width, max_height, current_label};
                    }
                }
            }
            
            if (max_area == 0) break; // No more rectangles found
            
            // Add the best rectangle and mark as covered
            rects[z].push_back({best_rect.x, best_rect.y, best_rect.w, best_rect.h, best_rect.label});
            
            for (int dy = 0; dy < best_rect.h; dy++) {
                for (int dx = 0; dx < best_rect.w; dx++) {
                    covered[best_rect.y + dy][best_rect.x + dx] = true;
                }
            }
            
            remaining_blocks -= max_area;
        }
    }

    // Stage 3: merge rectangles across slices into cuboids
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
    
    // Fast validation (comment out for production)
    //validateCoverageEfficient(cuboids, pb, parent_x, parent_y, parent_z);
    
    return cuboids;
}

// Efficient validation without full 3D array
void Compressor::validateCoverageEfficient(const std::vector<Cuboid>& cuboids, ParentBlock* pb,
                     int parent_x, int parent_y, int parent_z)
{
    uint64_t total_blocks = parent_x * parent_y * parent_z;
    uint64_t covered_blocks = 0;
    
    for (const auto& c : cuboids) {
        covered_blocks += c.w * c.h * c.d;
    }
    
    if (covered_blocks != total_blocks) {
        std::cerr << "ERROR: Coverage incomplete! " << covered_blocks 
                  << " blocks covered out of " << total_blocks << std::endl;
    }
    else {
        std::cout << "Validation passed: All " << total_blocks << " blocks covered" << std::endl;
    }
    
    // Quick overlap check
    for (size_t i = 0; i < cuboids.size(); i++) {
        for (size_t j = i + 1; j < cuboids.size(); j++) {
            const auto& a = cuboids[i];
            const auto& b = cuboids[j];
            
            // Check for overlap in 3D space
            bool x_overlap = (a.x < b.x + b.w) && (a.x + a.w > b.x);
            bool y_overlap = (a.y < b.y + b.h) && (a.y + a.h > b.y);
            bool z_overlap = (a.z < b.z + b.d) && (a.z + a.d > b.z);
            
            if (x_overlap && y_overlap && z_overlap) {
                std::cerr << "WARNING: Potential overlap between cuboids " << i << " and " << j << std::endl;
            }
        }
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

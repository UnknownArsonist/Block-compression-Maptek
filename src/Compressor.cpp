#include "../include/Compressor.h"
#include <unistd.h>

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
    tagTable = tag_table;
}
void Compressor::passBuffers(StreamBuffer *c_input_stream, StreamBuffer *c_output_stream)
{
    input_stream = c_input_stream;
    output_stream = c_output_stream;
}
// -----------ENDS HERE-------- ------------- //

// Compress a single 2D slice into rectangles
std::vector<Cuboid> Compressor::compressParentBlock(const ParentBlock *pb,
                                                    int parent_x, int parent_y, int parent_z)
{
    std::vector<Cuboid> cuboids;

    // Stage 1: compress along X (runs per row per slice)
    struct Run
    {
        int x, len;
        char label;
    };
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

    // Stage 2: merge runs vertically into rectangles
    struct Rect
    {
        int x, y, w, h;
        char label;
    };
    std::vector<std::vector<Rect>> rects(parent_z);

    for (int z = 0; z < parent_z; z++)
    {
        for (int y = 0; y < parent_y; y++)
        {
            for (auto &r : runs[z][y])
            {
                // try to extend this run upwards
                int h = 1;
                while (y + h < parent_y)
                {
                    bool match = false;
                    for (auto &r2 : runs[z][y + h])
                    {
                        if (r2.x == r.x && r2.len == r.len && r2.label == r.label)
                        {
                            match = true;
                            break;
                        }
                    }
                    if (!match)
                        break;
                    h++;
                }
                rects[z].push_back({r.x, y, r.len, h, r.label});
                y += h - 1; // skip ahead
            }
        }
    }

    // Stage 3: merge rectangles across slices into cuboids
    for (int z = 0; z < parent_z; z++)
    {
        for (auto &r : rects[z])
        {
            int d = 1;
            while (z + d < parent_z)
            {
                bool found = false;
                for (auto it = rects[z + d].begin(); it != rects[z + d].end(); ++it)
                {
                    if (it->x == r.x && it->y == r.y && it->w == r.w && it->h == r.h && it->label == r.label)
                    {
                        found = true;
                        rects[z + d].erase(it);
                        break;
                    }
                }
                if (!found)
                    break;
                d++;
            }
            cuboids.push_back({pb->x + r.x, pb->y + r.y, pb->z + z, r.w, r.h, d, r.label});
        }
    }

    return cuboids;
}

void Compressor::printCuboidsWithLegend(const std::vector<Cuboid> &cuboids,
                                        const std::unordered_map<char, std::string> &legend)
{
    for (const auto &c : cuboids)
    {
        // look up the label in the legend
        auto it = legend.find(c.label);
        const char *labelStr = (it != legend.end()) ? it->second.c_str() : "UNKNOWN";

        printf("%d,%d,%d,%d,%d,%d,%s\n",
               c.x, c.y, c.z, c.w, c.h, c.d, labelStr);
    }
}
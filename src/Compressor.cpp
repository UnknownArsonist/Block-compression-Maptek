#include "../include/Compressor.h"
#include <unistd.h>

Compressor::Compressor() {}
Compressor::~Compressor() {}

// -----------UNWANTED FUNCTIONS------------- //

void Compressor::printParentBlock(const std::vector<std::vector<std::vector<std::vector<char>>>> &parent_blocks)
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
void Compressor::OctreeCompression(ParentBlock *parent_block)
{
    // Check if the parent block is uniform first
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
    octTree.collectSubBlocks(root, subBlocks, tagTable, parent_block->x, parent_block->y, parent_block->z);

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
void Compressor::processParentBlocks(ParentBlock *parent_block)
{
    // std::cout << parent_block->block-
    int z = 0;
    while (z < *parent_z)
    {
        /*
        int parent_x = sub_blocks[0][0].size();
        int parent_y = sub_blocks[0].size();
        int parent_z = sub_blocks.size();
        */

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
                int maxX = x; // 0 1 7
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
                int maxZ = z;
                bool uniformZ = true;
                // checks the subblocks, are they uniform and did we alreadly visit them.
                while (maxZ < *parent_z && uniformZ)
                {
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
                    if (uniformZ)
                        // maxZ = 2 /*breaks the loop
                        maxZ++;
                }

                // Mark all as visited
                for (int zz = z; zz < maxZ; zz++)
                    for (int yy = y; yy < maxY; yy++)
                        for (int xx = x; xx < maxX; xx++)
                            // storing the character into the visited array
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

void Compressor::base_algorithms(ParentBlock *parent_block)
{
    int zi;
    zi = 0;
    while (zi < *parent_z)
    {
        if (isUniform(parent_block, zi))
        {
            char target = parent_block->block[(0 * *parent_y * *parent_z) + (0 * *parent_z) + zi];
            SubBlock *sub_block = (SubBlock *)malloc(sizeof(SubBlock));
            sub_block->x = parent_block->x;
            sub_block->y = parent_block->y;
            sub_block->z = parent_block->z;
            sub_block->l = *parent_x;
            sub_block->w = *parent_y;
            sub_block->h = zi;
            sub_block->tag = target;
            output_stream->push((void **)&sub_block);
        }
        else
        {
            // char target = parent_block->block[(0 * *parent_y * *parent_z) + (0 * *parent_z) + zi];
            /*

            SubBlock *sub_block = (SubBlock *)malloc(sizeof(SubBlock));
            sub_block->x = parent_block->x;
            sub_block->y = parent_block->y;
            sub_block->z = parent_block->z;
            sub_block->l = *parent_x;
            sub_block->w = *parent_y;
            sub_block->h = zi;
            sub_block->tag = target;
            output_stream->push((void **)&sub_block);
            */
            blockRect(parent_block, parent_block->x, parent_block->y, zi);
        }
        zi++;
    }
}

void Compressor::blockRect(ParentBlock *parent_block, int x_index, int y_index, int z_index)
{
    struct Run
    {
        int startX, startY, startZ;
        int endX, endY, endZ;
        char tag;
    };

    // vector to store full rectangular runs
    std::vector<Run> filtered;

    // --- Scan horizontal runs ---
    std::vector<Run> horizontalRuns;

    char target = parent_block->block[(0 * *parent_y * *parent_z) + (0 * *parent_z) + z_index];

    for (int y = 0; y < *parent_y; y++)
    {
        int lenX = 0;
        int startX = 0;

        for (int x = 0; x < *parent_x; x++)
        {
            char current = parent_block->block[(x * *parent_y * *parent_z) + (y * *parent_z) + z_index];

            if (current == target)
            {
                if (lenX == 0)
                    startX = x;

                lenX++;
            }
            else
            {
                if (lenX > 0)
                {
                    horizontalRuns.push_back({startX, y, z_index, startX + lenX - 1, y, z_index, target});
                }

                // start new run
                target = current;
                lenX = 1;
                startX = x;
            }
        }

        if (lenX > 0)
        {
            horizontalRuns.push_back({startX, y, z_index, startX + lenX - 1, y, z_index, target});
        }
    }

    // --- Compress vertical runs ---
    if (!horizontalRuns.empty())
    {
        Run start = horizontalRuns[0];
        Run last = horizontalRuns[0];

        for (size_t i = 1; i < horizontalRuns.size(); i++)
        {
            Run curr = horizontalRuns[i];

            // same horizontal run extended vertically
            if (curr.startX == last.startX &&
                curr.endX == last.endX &&
                curr.startZ == last.startZ &&
                curr.tag == last.tag)
            {
                last.endY = curr.endY; // extend vertical run
            }
            else
            {
                // push completed vertical run
                filtered.push_back({last.startX, start.startY, last.startZ,
                                    last.endX, last.endY, last.endZ, last.tag});

                // start new run
                start = curr;
                last = curr;
            }
        }

        // push final run
        filtered.push_back({last.startX, start.startY, last.startZ,
                            last.endX, last.endY, last.endZ, last.tag});
    }

    for (const auto &r : filtered)
    {
        SubBlock *sub_block = (SubBlock *)malloc(sizeof(SubBlock));

        // position of the sub-block = starting coords
        sub_block->x = parent_block->x + r.startX;
        sub_block->y = parent_block->y + r.startY;
        sub_block->z = parent_block->z + r.startZ;

        // sizes = difference between start and end + 1
        sub_block->l = r.endX - r.startX + 1;
        sub_block->w = r.endY - r.startY + 1;
        sub_block->h = r.endZ - r.startZ + 1;

        sub_block->tag = r.tag;

        output_stream->push((void **)&sub_block);
    }
}

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

        // Safety check: if we've processed too many blocks, use simpler algorithm
        /*
        if (block_count > 10000)
        { // Adjust this threshold as needed
            processParentBlocks(parent_block);
        }
        else
        {
            OctreeCompression(parent_block);
        }
            */
        Compressedcube = compressParentBlock(parent_block, *parent_x, *parent_y, *parent_z);
        printCuboidsWithLegend(Compressedcube, *tagTable);
    } while (parent_block != NULL);
}
// -----------ENDS HERE-------- ------------- //

// -----------HELPER FUNCTIONS ------------- //
bool Compressor::isUniform(ParentBlock *parent_block, int i)
{
    char target = parent_block->block[(0 * *parent_y * *parent_z) + (0 * *parent_z) + i];
    for (int y = 0; y < *parent_y; y++)
    {
        for (int x = 0; x < *parent_x; x++)
        {
            if (target != parent_block->block[(x * *parent_y * *parent_z) + (y * *parent_z) + i])
            {
                return false;
            }
        }
    }
    return true;
}

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
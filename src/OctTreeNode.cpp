#include "OctTreeNode.h"

// Constructors & deconstructors
OctTreeNode::OctTreeNode(/* args */)
{
    isleaf = false;
    children[0] = nullptr;
    children[1] = nullptr;
    children[2] = nullptr;
    children[3] = nullptr;
    children[4] = nullptr;
    children[5] = nullptr;
    children[6] = nullptr;
    children[7] = nullptr;
}

OctTreeNode::~OctTreeNode() {}
// ------------------ENDS HERE-----------------------------------------//

// ----------------MAIN FUNCTIONS--------------------------------------//
OctTreeNode *OctTreeNode::build(ParentBlock &grid, int x0, int y0, int z0,
                                int sizeX, int sizeY, int sizeZ)
{
    char tag;
    if (isUniform(&grid, x0, y0, z0, sizeX, sizeY, sizeZ, tag))
    {
        OctTreeNode *leaf = new OctTreeNode();
        // starting coordinates of the parent block
        leaf->x0 = x0;
        leaf->y0 = y0;
        leaf->z0 = z0;

        // dimensions of the parent block
        leaf->sizeX = sizeX;
        leaf->sizeY = sizeY;
        leaf->sizeZ = sizeZ;
        leaf->isleaf = true;
        leaf->tag = tag;
        return leaf;
    }
    OctTreeNode *node = new OctTreeNode();
    node->x0 = x0;
    node->y0 = y0;
    node->z0 = z0;
    node->sizeX = sizeX;
    node->sizeY = sizeY;
    node->sizeZ = sizeZ;

    // half the size of the current block along each axis
    // int mx = sizeX / 2, my = sizeY / 2, mz = sizeZ / 2;

    // offsets used to create the 8 octant, this changes the starting points so that
    // Each child’s starting coordinates are parent’s start + one of these offsets
    // int dx[2] = {0, mx}, dy[2] = {0, my}, dz[2] = {0, mz}; // 4 4 1
    std::vector<std::tuple<int, int, int, int, int, int, char>> rectangles; // x0,y0,z0,l,w,h,tag
    /*
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 2; ++j)
            for (int k = 0; k < 2; ++k)
            {
                int sx = (i == 0) ? mx : sizeX - mx;
                int sy = (j == 0) ? my : sizeY - my;
                int sz = (k == 0) ? mz : sizeZ - mz;
                if (sx > 0 && sy > 0 && sz > 0)
                    node->children[i * 4 + j * 2 + k] = build(grid, x0 + dx[i], y0 + dy[j], z0 + dz[k], sx, sy, sz);
                // child[8] = buiding(grid, 4, 4 ,1 , 4, 4, 1) {0,4,0,4,4,1}
            }
    */
    for (int z = 0; z < sizeZ; ++z)
    {
        for (int y = 0; y < sizeY; ++y)
        {
            for (int x = 0; x < sizeX;)
            {
                char current = grid.block[((x0 + x) * sizeY * sizeZ) + ((y0 + y) * sizeZ) + (z0 + z)];

                // Find maximal run along X
                int runX = 1;
                while (x + runX < sizeX)
                {
                    char next = grid.block[((x0 + x + runX) * sizeY * sizeZ) + ((y0 + y) * sizeZ) + (z0 + z)];
                    if (next != current)
                        break;
                    runX++;
                }

                // Add as a rectangle of size runX x 1 x 1
                rectangles.push_back({x0 + x, y0 + y, z0 + z, runX, 1, 1, current});
                x += runX;
            }
        }
    }
    // ---------- NOW MERGE ----------------//
    int childIndex = 0;
    for (auto &r : rectangles)
    {
        int rx, ry, rz, rl, rw, rh;
        char rtag;
        std::tie(rx, ry, rz, rl, rw, rh, rtag) = r;

        OctTreeNode *leaf = new OctTreeNode();
        leaf->x0 = rx;
        leaf->y0 = ry;
        leaf->z0 = rz;
        leaf->sizeX = rl;
        leaf->sizeY = rw;
        leaf->sizeZ = rh;
        leaf->isleaf = true;
        leaf->tag = rtag;

        if (childIndex < 8)
        {
            node->children[childIndex++] = leaf;
        }
        else
        {
            // For more than 8 rectangles, you could maintain a vector<OctTreeNode*> in the parent
            // or create another layer of parent nodes
        }
    }
    /*
    char firstTag = 0;
    bool canMerge = true;
    for (int c = 0; c < 8; ++c)
    {
        if (!node->children[c] || !node->children[c]->isleaf)
        {
            canMerge = false;
            break;
        }
        if (c == 0)
            firstTag = node->children[c]->tag;
        else if (node->children[c]->tag != firstTag)
        {
            canMerge = false;
            break;
        }
    }

    if (canMerge)
    {
        // All children are uniform → merge into one leaf
        OctTreeNode *leaf = new OctTreeNode();
        leaf->x0 = x0;
        leaf->y0 = y0;
        leaf->z0 = z0;
        leaf->sizeX = sizeX;
        leaf->sizeY = sizeY;
        leaf->sizeZ = sizeZ;
        leaf->isleaf = true;
        leaf->tag = firstTag;

        // Free children
        for (int c = 0; c < 8; ++c)
            delete node->children[c];
        delete node;

        return leaf;
    }
    */
    return node;
}

OctTreeNode *OctTreeNode::buildContentDriven3D(ParentBlock &grid,
                                               int x0, int y0, int z0,
                                               int sizeX, int sizeY, int sizeZ)
{
    char tag;

    // Set a minimum size to avoid excessive subdivision
    const int MIN_SIZE = 2; // Adjust this value as needed

    // Step 1: Entire block uniform or below minimum size?
    if (isUniform(&grid, x0, y0, z0, sizeX, sizeY, sizeZ, tag) ||
        (sizeX < MIN_SIZE && sizeY < MIN_SIZE && sizeZ < MIN_SIZE))
    {
        OctTreeNode *leaf = new OctTreeNode();
        leaf->x0 = x0;
        leaf->y0 = y0;
        leaf->z0 = z0;
        leaf->sizeX = sizeX;
        leaf->sizeY = sizeY;
        leaf->sizeZ = sizeZ;
        leaf->isleaf = true;
        leaf->tag = tag;
        return leaf;
    }

    // Step 2: Create parent node
    OctTreeNode *node = new OctTreeNode();
    node->x0 = x0;
    node->y0 = y0;
    node->z0 = z0;
    node->sizeX = sizeX;
    node->sizeY = sizeY;
    node->sizeZ = sizeZ;

    // Step 3: Scan for maximal uniform cuboids
    std::vector<std::vector<std::vector<bool>>> visited(sizeX,
                                                        std::vector<std::vector<bool>>(sizeY, std::vector<bool>(sizeZ, false)));

    for (int z = 0; z < sizeZ; ++z)
    {
        for (int y = 0; y < sizeY; ++y)
        {
            for (int x = 0; x < sizeX; ++x)
            {
                if (visited[x][y][z])
                    continue;

                char current = grid.block[((x0 + x) * sizeY * sizeZ) + ((y0 + y) * sizeZ) + (z0 + z)];

                // Find maximal length along X
                int maxX = x;
                while (maxX < sizeX && !visited[maxX][y][z] &&
                       grid.block[((x0 + maxX) * sizeY * sizeZ) + ((y0 + y) * sizeZ) + (z0 + z)] == current)
                    maxX++;
                int lenX = maxX - x;

                // Extend along Y
                int maxY = y + 1;
                while (maxY < sizeY)
                {
                    bool rowUniform = true;
                    for (int xi = x; xi < x + lenX; ++xi)
                    {
                        if (visited[xi][maxY][z] || grid.block[((x0 + xi) * sizeY * sizeZ) + ((y0 + maxY) * sizeZ) + (z0 + z)] != current)
                        {
                            rowUniform = false;
                            break;
                        }
                    }
                    if (!rowUniform)
                        break;
                    maxY++;
                }
                int lenY = maxY - y;

                // Extend along Z
                int maxZ = z + 1;
                while (maxZ < sizeZ)
                {
                    bool layerUniform = true;
                    for (int yi = y; yi < y + lenY; ++yi)
                    {
                        for (int xi = x; xi < x + lenX; ++xi)
                        {
                            if (visited[xi][yi][maxZ] || grid.block[((x0 + xi) * sizeY * sizeZ) + ((y0 + yi) * sizeZ) + (z0 + maxZ)] != current)
                            {
                                layerUniform = false;
                                break;
                            }
                        }
                        if (!layerUniform)
                            break;
                    }
                    if (!layerUniform)
                        break;
                    maxZ++;
                }
                int lenZ = maxZ - z;

                // Mark visited
                for (int xi = x; xi < x + lenX; ++xi)
                    for (int yi = y; yi < y + lenY; ++yi)
                        for (int zi = z; zi < z + lenZ; ++zi)
                            visited[xi][yi][zi] = true;

                // Create leaf node for this cuboid
                OctTreeNode *leaf = new OctTreeNode();
                leaf->x0 = x0 + x;
                leaf->y0 = y0 + y;
                leaf->z0 = z0 + z;
                leaf->sizeX = lenX;
                leaf->sizeY = lenY;
                leaf->sizeZ = lenZ;
                leaf->isleaf = true;
                leaf->tag = current;

                node->childrenVector.push_back(leaf);
            }
        }
    }

    return node;
}
// ------------------ENDS HERE-----------------------------------------//

// ---------------HELPER FUNCTIONS-------------------------------------//
bool OctTreeNode::isUniform(ParentBlock *parent_block, int x0, int y0, int z0,
                            int sizeX, int sizeY, int sizeZ, char &outTag)
{
    char first = parent_block->block[(0 * sizeY * sizeZ) + (0 * sizeZ) + 0];
    for (int z = 0; z < sizeZ; z++)
    {
        for (int y = 0; y < sizeY; y++)
        {
            for (int x = 0; x < sizeX; x++)
            {
                if (parent_block->block[(x * sizeY * sizeZ) + (y * sizeZ) + z] != first)
                {
                    return false;
                }
            }
        }
    }
    outTag = first;
    return true;
}

std::vector<SubBlock> OctTreeNode::mergeSubBlocks(const std::vector<SubBlock> &blocks)
{
    if (blocks.empty())
        return blocks;

    std::vector<SubBlock> merged = blocks;
    bool changed;

    do
    {
        changed = false;
        std::vector<SubBlock> newMerged;
        std::vector<bool> mergedFlag(merged.size(), false);

        for (size_t i = 0; i < merged.size(); i++)
        {
            if (mergedFlag[i])
                continue;

            SubBlock current = merged[i];
            bool foundMerge = true;

            while (foundMerge)
            {
                foundMerge = false;

                for (size_t j = i + 1; j < merged.size(); j++)
                {
                    if (mergedFlag[j])
                        continue;

                    SubBlock candidate = merged[j];

                    // Must have same tag
                    if (current.tag != candidate.tag)
                        continue;

                    // Try to merge in X direction (same Y, Z, height, width)
                    if (current.y == candidate.y && current.z == candidate.z &&
                        current.w == candidate.w && current.h == candidate.h)
                    {
                        if (current.x + current.l == candidate.x)
                        {
                            current.l += candidate.l;
                            mergedFlag[j] = true;
                            foundMerge = true;
                            changed = true;
                            break;
                        }
                        else if (candidate.x + candidate.l == current.x)
                        {
                            current.x = candidate.x;
                            current.l += candidate.l;
                            mergedFlag[j] = true;
                            foundMerge = true;
                            changed = true;
                            break;
                        }
                    }

                    // Try to merge in Y direction (same X, Z, length, height)
                    if (current.x == candidate.x && current.z == candidate.z &&
                        current.l == candidate.l && current.h == candidate.h)
                    {
                        if (current.y + current.w == candidate.y)
                        {
                            current.w += candidate.w;
                            mergedFlag[j] = true;
                            foundMerge = true;
                            changed = true;
                            break;
                        }
                        else if (candidate.y + candidate.w == current.y)
                        {
                            current.y = candidate.y;
                            current.w += candidate.w;
                            mergedFlag[j] = true;
                            foundMerge = true;
                            changed = true;
                            break;
                        }
                    }

                    // Try to merge in Z direction (same X, Y, length, width)
                    if (current.x == candidate.x && current.y == candidate.y &&
                        current.l == candidate.l && current.w == candidate.w)
                    {
                        if (current.z + current.h == candidate.z)
                        {
                            current.h += candidate.h;
                            mergedFlag[j] = true;
                            foundMerge = true;
                            changed = true;
                            break;
                        }
                        else if (candidate.z + candidate.h == current.z)
                        {
                            current.z = candidate.z;
                            current.h += candidate.h;
                            mergedFlag[j] = true;
                            foundMerge = true;
                            changed = true;
                            break;
                        }
                    }

                    // Try to merge in XY plane (same Z, height)
                    if (current.z == candidate.z && current.h == candidate.h)
                    {
                        // Adjacent in X with same Y range
                        if (current.y == candidate.y && current.w == candidate.w &&
                            current.x + current.l == candidate.x)
                        {
                            current.l += candidate.l;
                            mergedFlag[j] = true;
                            foundMerge = true;
                            changed = true;
                            break;
                        }
                        // Adjacent in Y with same X range
                        if (current.x == candidate.x && current.l == candidate.l &&
                            current.y + current.w == candidate.y)
                        {
                            current.w += candidate.w;
                            mergedFlag[j] = true;
                            foundMerge = true;
                            changed = true;
                            break;
                        }
                    }
                }
            }

            newMerged.push_back(current);
        }

        merged = newMerged;

    } while (changed);

    return merged;
}
void OctTreeNode::deleteTree(OctTreeNode *node)
{
    if (!node)
        return;

    // Delete fixed array children
    for (int i = 0; i < 8; ++i)
    {
        if (node->children[i])
        {
            deleteTree(node->children[i]);
            delete node->children[i];
            node->children[i] = nullptr;
        }
    }

    // Delete vector children
    for (auto child : node->childrenVector)
    {
        deleteTree(child);
        delete child;
    }
    node->childrenVector.clear();

    // Don't delete the node itself here, let the caller do that
}
void OctTreeNode::collectSubBlocks(OctTreeNode *node, std::vector<SubBlock> &blocks,
                                   std::unordered_map<char, std::string> *tag_table, int originX, int originY, int originZ)
{
    if (!node)
        return;

    if (node->isleaf)
    {
        blocks.push_back({node->x0 + originX,
                          node->y0 + originY,
                          node->z0 + originZ,
                          node->sizeX,
                          node->sizeY,
                          node->sizeZ,
                          node->tag});
        return;
    }

    for (auto child : node->childrenVector)
    {
        collectSubBlocks(child, blocks, tag_table, originX, originY, originZ);
    }
}
// ------------------ENDS HERE-----------------------------------------//
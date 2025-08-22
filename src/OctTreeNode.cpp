#include "OctTreeNode.h"

// Constructors & deconstructors
OctTreeNode::OctTreeNode(/* args */)
{
    isleaf = false;
    tag = 0;
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

    // Step 1: Entire block uniform?
    if (isUniform(&grid, x0, y0, z0, sizeX, sizeY, sizeZ, tag))
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
    // [(x * *parent_y * *parent_z) + (y * *parent_z) + z]
    char first = parent_block->block[(0 * sizeY * sizeZ) + (0 * sizeZ) + 0];
    for (int z = 0; z < sizeZ; z++)
    {
        for (int y = 0; y < sizeY; y++)
        {
            for (int x = 0; x < sizeX; x++)
            {
                // std::cout << parent_block->block[(x * sizeY * sizeZ) + (y * sizeZ) + z];
                if (parent_block->block[(x * sizeY * sizeZ) + (y * sizeZ) + z] != first)
                {
                    return false;
                }
            }
            // std::cout << std::endl;
        }
        // std::cout << std::endl;
    }
    outTag = first;
    return true;
}

std::vector<SubBlock> OctTreeNode::mergeSubBlocks(const std::vector<SubBlock> &blocks)
{
    std::vector<SubBlock> merged = blocks;

    bool mergedSomething = true;
    while (mergedSomething)
    {
        mergedSomething = false;
        std::vector<SubBlock> newList;
        std::vector<bool> used(merged.size(), false);

        for (size_t i = 0; i < merged.size(); i++)
        {
            if (used[i])
                continue;
            SubBlock a = merged[i];
            bool didMerge = false;

            for (size_t j = i + 1; j < merged.size(); j++)
            {
                if (used[j])
                    continue;
                SubBlock b = merged[j];

                // same tag
                if (a.tag != b.tag)
                    continue;

                // Try merge along X
                if (a.y == b.y && a.z == b.z && a.w == b.w && a.h == b.h)
                {
                    if (a.x + a.l == b.x)
                    {
                        a.l += b.l;
                        used[j] = true;
                        didMerge = true;
                    }
                    else if (b.x + b.l == a.x)
                    {
                        a.x = b.x;
                        a.l += b.l;
                        used[j] = true;
                        didMerge = true;
                    }
                }
                // Try merge along Y
                else if (a.x == b.x && a.z == b.z && a.l == b.l && a.h == b.h)
                {
                    if (a.y + a.w == b.y)
                    {
                        a.w += b.w;
                        used[j] = true;
                        didMerge = true;
                    }
                    else if (b.y + b.w == a.y)
                    {
                        a.y = b.y;
                        a.w += b.w;
                        used[j] = true;
                        didMerge = true;
                    }
                }
                // Try merge along Z
                else if (a.x == b.x && a.y == b.y && a.l == b.l && a.w == b.w)
                {
                    if (a.z + a.h == b.z)
                    {
                        a.h += b.h;
                        used[j] = true;
                        didMerge = true;
                    }
                    else if (b.z + b.h == a.z)
                    {
                        a.z = b.z;
                        a.h += b.h;
                        used[j] = true;
                        didMerge = true;
                    }
                }
            }

            newList.push_back(a);
            if (didMerge)
                mergedSomething = true;
        }

        merged.swap(newList);
    }

    return merged;
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
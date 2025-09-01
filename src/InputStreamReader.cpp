#include "../include/InputStreamReader.h"

// Constructor and Destructor
InputStreamReader::InputStreamReader(FILE *in)
{
    input_stream = in;
}

InputStreamReader::InputStreamReader() : InputStreamReader(stdin)
{
}

InputStreamReader::~InputStreamReader()
{
    // Destructor implementation
}

void InputStreamReader::passValues(int *c_x_count, int *c_y_count, int *c_z_count, int *c_parent_x, int *c_parent_y, int *c_parent_z, std::unordered_map<char, std::string> *c_tag_table)
{
    x_count = c_x_count;
    y_count = c_y_count;
    z_count = c_z_count;
    parent_x = c_parent_x;
    parent_y = c_parent_y;
    parent_z = c_parent_z;
    tag_table = c_tag_table;
}

void InputStreamReader::passBuffers(StreamBuffer *c_output_stream)
{
    output_stream = c_output_stream;
}

void InputStreamReader::getHeader()
{
    getCommaSeparatedValuesFromStream(x_count, y_count, z_count, parent_x, parent_y, parent_z);
    getLegendFromStream(tag_table);
}

// do we need this function?
void InputStreamReader::getCommaSeparatedValuesFromStream() {}

template <typename T, typename... Args>
void InputStreamReader::getCommaSeparatedValuesFromStream(T *value, Args... args)
{
    char c;
    *value = 0;
    while ((c = getc(input_stream)) != EOF)
    {
        if (c == ',' || c == '\n')
        {
            break;
        }
        else if (c != '\r')
        {
            *value *= 10;
            *value += (int)c - '0';
        }
    }
    getCommaSeparatedValuesFromStream(args...);
}

void InputStreamReader::getLegendFromStream(std::unordered_map<char, std::string> *legend)
{
    char c;
    char key = 0;
    std::string value = "";
    int v = 0;
    int n = 0;
    while ((c = getc(input_stream)) != EOF)
    {
        if (c == ',')
        {
            v++;
        }
        else if (c == '\n')
        {
            if (n > 0)
            {
                return;
            }
            (*legend)[key] = value;
            v = 0;
            value.clear();
            n++;
        }
        else
        {
            if (v == 0)
            {
                key = c;
            }
            else
            {
                value += c;
            }
            n = 0;
        }
    }
}


void InputStreamReader::processStream()
{
    // Use ceil division to include partial parent blocks at edges
    int num_parent_blocks = 
        ((*x_count + *parent_x - 1) / *parent_x) *
        ((*y_count + *parent_y - 1) / *parent_y) *
        ((*z_count + *parent_z - 1) / *parent_z);

    ParentBlock **parent_blocks = (ParentBlock **)calloc(num_parent_blocks, sizeof(ParentBlock *));
    if (!parent_blocks)
        exit(1);

    char *null_ptr = NULL;
    int x = 0, y = 0, z = 0;
    char ch;

    while ((ch = getc(input_stream)) != EOF)
    {
        // Skip carriage returns
        if (ch == '\r') continue;

        // Treat newline as end-of-row, but do not advance z yet
        if (ch == '\n')
        {
            if (x > 0)
            {
                x = 0;
                y++;
            }
            continue;
        }

        // Stop if we reached the end of the volume
        if (z >= *z_count) break;
        if (y >= *y_count) { y = 0; z++; continue; }
        if (x >= *x_count) { x = 0; y++; continue; }

        // Compute parent block indices
        int px_idx = x / *parent_x;
        int py_idx = y / *parent_y;
        int pz_idx = z / *parent_z;

        int blocks_per_x = (*x_count + *parent_x - 1) / *parent_x;
        int blocks_per_y = (*y_count + *parent_y - 1) / *parent_y;

        int current_parent_block = px_idx + blocks_per_x * py_idx + blocks_per_x * blocks_per_y * pz_idx;
        if (current_parent_block < 0 || current_parent_block >= num_parent_blocks)
        {
            x++;
            continue;
        }

        // Allocate parent block if needed
        if (!parent_blocks[current_parent_block])
        {
            parent_blocks[current_parent_block] = (ParentBlock *)malloc(sizeof(ParentBlock));
            parent_blocks[current_parent_block]->x = px_idx * (*parent_x);
            parent_blocks[current_parent_block]->y = py_idx * (*parent_y);
            parent_blocks[current_parent_block]->z = pz_idx * (*parent_z);

            int block_size = *parent_x * *parent_y * *parent_z;
            parent_blocks[current_parent_block]->block = (char *)malloc(block_size);
            memset(parent_blocks[current_parent_block]->block, 0, block_size);
        }

        // Compute relative coordinates within the parent block
        int rx = x % *parent_x;
        int ry = y % *parent_y;
        int rz = z % *parent_z;

        // Row-major indexing to match compressParentBlock
        int index = rz * (*parent_y * *parent_x) + ry * (*parent_x) + rx;
        parent_blocks[current_parent_block]->block[index] = ch;

        // Check if parent block is complete (optional)
        if (rx == *parent_x - 1 && ry == *parent_y - 1 && rz == *parent_z - 1)
        {
            output_stream->push((void **)&parent_blocks[current_parent_block]);
            parent_blocks[current_parent_block] = NULL;
        }

        // Advance coordinates
        x++;
        if (x >= *x_count) { x = 0; y++; }
        if (y >= *y_count) { y = 0; z++; }
    }

    // Push any remaining incomplete parent blocks
    for (int i = 0; i < num_parent_blocks; i++)
    {
        if (parent_blocks[i] != NULL)
        {
            output_stream->push((void **)&parent_blocks[i]);
        }
    }

    // Push NULL terminator
    output_stream->push((void **)&null_ptr);

    free(parent_blocks);
}


// print the header information and the 3D block data
void InputStreamReader::printHeader(FILE *out)
{
    // print the header information
    fprintf(out, "%d, %d, %d, %d, %d, %d\n", *x_count, *y_count, *z_count, *parent_x, *parent_y, *parent_z);
    for (const auto &e : *tag_table)
    {
        fprintf(out, "%c, %s\n", e.first, e.second.c_str());
    }
}
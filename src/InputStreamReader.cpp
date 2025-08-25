#include "InputStreamReader.h"

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

// Function to process the slice of 3D block data
void InputStreamReader::processStream()
{
    int num_parent_blocks = (*x_count / *parent_x) * (*y_count / *parent_y) * (*z_count / *parent_z);

    // Use heap allocation with proper error checking
    ParentBlock **parent_blocks = (ParentBlock **)calloc(num_parent_blocks, sizeof(ParentBlock *));
    if (!parent_blocks)
    {
        // fprintf(stderr, "Memory allocation failed for parent_blocks\n");
        exit(1);
    }

    char *null_ptr = NULL;
    int consecutive_newlines = 0;
    int x = 0;
    int y = 0;
    int z = 0;
    char ch;

    while ((ch = getc(input_stream)) != EOF)
    {
        // Check if we've processed all blocks
        if (z >= *z_count)
        {
            // fprintf(stderr, "Processed all %d slices, stopping\n", *z_count);
            break;
        }
        if (y >= *y_count)
        {
            // fprintf(stderr, "Processed all %d rows in slice %d, moving to next slice\n", *y_count, z);
            y = 0;
            z++;
            continue;
        }
        if (x >= *x_count)
        {
            // fprintf(stderr, "Processed all %d columns in row %d, slice %d, moving to next row\n", *x_count, y, z);
            x = 0;
            y++;
            continue;
        }

        if (ch == '\n')
        {
            consecutive_newlines++;
            // Blank line indicates slice separator
            if (consecutive_newlines >= 2 && x == 0 && y == 0)
            {
                // We're at the beginning of a new slice
                z++;
                consecutive_newlines = 0;
                // fprintf(stderr, "Moving to slice %d\n", z);
            }
            else if (x > 0)
            {
                // End of row within a slice
                x = 0;
                y++;
                consecutive_newlines = 0;
                // fprintf(stderr, "Moving to row %d in slice %d\n", y, z);
            }
            continue;
        }
        else if (ch == '\r')
        {
            continue; // Ignore carriage returns
        }

        // Reset consecutive newlines counter when we get a non-whitespace character
        consecutive_newlines = 0;

        // Calculate parent block index with bounds checking
        int parent_x_index = x / *parent_x;
        int parent_y_index = y / *parent_y;
        int parent_z_index = z / *parent_z;

        int current_parent_block = parent_x_index +
                                   (*x_count / *parent_x) * parent_y_index +
                                   (*x_count / *parent_x) * (*y_count / *parent_y) * parent_z_index;

        if (current_parent_block < 0 || current_parent_block >= num_parent_blocks)
        {
            // fprintf(stderr, "ERROR: Parent block index %d out of bounds (max: %d)\n",
            //         current_parent_block, num_parent_blocks - 1);
            // fprintf(stderr, "Coordinates: x=%d, y=%d, z=%d\n", x, y, z);
            // fprintf(stderr, "Parent indices: px=%d, py=%d, pz=%d\n",
            //         parent_x_index, parent_y_index, parent_z_index);
            x++;
            continue;
        }

        if (parent_blocks[current_parent_block] == NULL)
        {
            parent_blocks[current_parent_block] = (ParentBlock *)malloc(sizeof(ParentBlock));
            if (!parent_blocks[current_parent_block])
            {
                // fprintf(stderr, "Memory allocation failed for ParentBlock\n");
                exit(1);
            }

            parent_blocks[current_parent_block]->block = (char *)malloc(*parent_x * *parent_y * *parent_z);
            if (!parent_blocks[current_parent_block]->block)
            {
                // fprintf(stderr, "Memory allocation failed for block data\n");
                free(parent_blocks[current_parent_block]);
                exit(1);
            }

            // Set the origin coordinates of this parent block
            parent_blocks[current_parent_block]->x = parent_x_index * *parent_x;
            parent_blocks[current_parent_block]->y = parent_y_index * *parent_y;
            parent_blocks[current_parent_block]->z = parent_z_index * *parent_z;

            // Initialize the block memory
            memset(parent_blocks[current_parent_block]->block, 0, *parent_x * *parent_y * *parent_z);
        }

        int parent_relative_x = x % *parent_x;
        int parent_relative_y = y % *parent_y;
        int parent_relative_z = z % *parent_z;

        int index = (parent_relative_x * *parent_y * *parent_z) +
                    (parent_relative_y * *parent_z) +
                    parent_relative_z;

        if (index < 0 || index >= *parent_x * *parent_y * *parent_z)
        {
            // fprintf(stderr, "ERROR: Block index %d out of bounds for parent block %d\n",
            //         index, current_parent_block);
            // fprintf(stderr, "Relative coordinates: rx=%d, ry=%d, rz=%d\n",
            //         parent_relative_x, parent_relative_y, parent_relative_z);
        }
        else
        {
            parent_blocks[current_parent_block]->block[index] = ch;
        }

        // Check if this completes the parent block
        if (parent_relative_x == *parent_x - 1 &&
            parent_relative_y == *parent_y - 1 &&
            parent_relative_z == *parent_z - 1)
        {
            output_stream->push((void **)&parent_blocks[current_parent_block]);
            parent_blocks[current_parent_block] = NULL;
        }

        x++;

        // Check if we've reached the end of a row
        if (x >= *x_count)
        {
            x = 0;
            y++;
            // fprintf(stderr, "End of row %d in slice %d\n", y - 1, z);
        }
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

    // Free the parent_blocks array
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
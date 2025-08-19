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
    while ((c = getc(input_stream)) != EOF) {
        if (c == ',') {
            v++;
        } else if (c == '\n') {
            if (n > 0) {
                return;
            }
            (*legend)[key] = value;
            v = 0;
            value.clear();
            n++;
        } else {
            if (v == 0) {
                key = c;
            } else {
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
    ParentBlock *parent_blocks[num_parent_blocks];
    //bool uniform[num_parent_blocks];
    for (int i = 0; i < num_parent_blocks; i++) {
        parent_blocks[i] = NULL;
    }

    char *null_ptr = NULL;
    int n = 0;
    int x = 0;
    int y = 0;
    int z = 0;
    char ch;

    while ((ch = getc(input_stream)) != EOF) {
        int current_parent_block = (x / *parent_x) + (*x_count / *parent_x) * (y / *parent_y) + (*x_count / *parent_x) * (*y_count / *parent_y) * (z / *parent_z);
        // printf("[%d] %d, %d, %d, %c\n", current_parent_block, x, y, z, ch);
        if (ch == '\n') {
            x = 0;
            y++;
            n++;
            if (n > 1) {
                y = 0;
                z++;
            }
        } else if (ch != '\r') {
            // printf("[%d] %d, %d, %d\n", current_parent_block, x, y, z);
            if (parent_blocks[current_parent_block] == NULL) {
                parent_blocks[current_parent_block] = (ParentBlock *)malloc(sizeof(ParentBlock));
                parent_blocks[current_parent_block]->block = (char *)malloc(*parent_x * *parent_y * *parent_z * sizeof(char));
                parent_blocks[current_parent_block]->x = x;
                parent_blocks[current_parent_block]->y = y;
                parent_blocks[current_parent_block]->z = z;
                //uniform[current_parent_block] = true;
            }
            int parent_relative_x = x % *parent_x;
            int parent_relative_y = y % *parent_y;
            int parent_relative_z = z % *parent_z;
            // printf("%d, %d, %d, %d: %c\n", current_parent_block, parent_relative_x, parent_relative_y, z, ch);

            parent_blocks[current_parent_block]->block[(parent_relative_x * *parent_y * *parent_z) + (parent_relative_y * *parent_z) + parent_relative_z] = ch;
            /*if (ch != parent_blocks[current_parent_block]->block[0]) {
                uniform[current_parent_block] = false;
            }*/
            if (parent_relative_x == *parent_x - 1 && parent_relative_y == *parent_y - 1 && parent_relative_z == *parent_z - 1) {
                // printf("[%d] %d, %d, %d\n", current_parent_block, x, y, z);
                /*
                if (uniform[current_parent_block]) {
                    free(parent_blocks[current_parent_block]->block);
                    parent_blocks[current_parent_block]->block = NULL;
                }
                */
                output_stream->push((void **)&parent_blocks[current_parent_block]);
                parent_blocks[current_parent_block] = NULL;
                // output_stream->printBuffer();
            }
            x++;
            n = 0;
        }
    }
    output_stream->push((void **)&null_ptr);
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
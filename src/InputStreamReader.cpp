#include "StreamProcessor.h"

// Constructor and Destructor
StreamProcessor::InputStreamReader::InputStreamReader(FILE *in) {
    input_stream = in;
}

StreamProcessor::InputStreamReader::InputStreamReader() : InputStreamReader(stdin) {
}

StreamProcessor::InputStreamReader::~InputStreamReader() {
    // Destructor implementation
}

void StreamProcessor::InputStreamReader::passValues(StreamProcessor *sp) {
    x_count = &(sp->x_count);
    y_count = &(sp->y_count);
    z_count = &(sp->z_count);
    parent_x = &(sp->parent_x);
    parent_y = &(sp->parent_y);
    parent_z = &(sp->parent_z);
    tag_table = &(sp->tag_table);
    output_stream = (sp->inputToCompressorBuffer);
}

void StreamProcessor::InputStreamReader::passValues(int *c_x_count, int *c_y_count, int *c_z_count, int *c_parent_x, int *c_parent_y, int *c_parent_z, std::unordered_map<char, std::string> *c_tag_table) {
    x_count = c_x_count;
    y_count = c_y_count;
    z_count = c_z_count;
    parent_x = c_parent_x;
    parent_y = c_parent_y;
    parent_z = c_parent_z;
    tag_table = c_tag_table;
}
void StreamProcessor::InputStreamReader::passBuffers(StreamProcessor::StreamBuffer *c_output_stream)
{
    output_stream = c_output_stream;
}

void StreamProcessor::InputStreamReader::getHeader()
{
    getCommaSeparatedValuesFromStream(x_count, y_count, z_count, parent_x, parent_y, parent_z);
    getLegendFromStream(tag_table);
}

// do we need this function?
void StreamProcessor::InputStreamReader::getCommaSeparatedValuesFromStream() {}

template <typename T, typename... Args>
void StreamProcessor::InputStreamReader::getCommaSeparatedValuesFromStream(T *value, Args... args) {
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

void StreamProcessor::InputStreamReader::getLegendFromStream(std::unordered_map<char, std::string> *legend) {
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
        } else if (c != ' ') {
            if (v == 0) {
                key = c;
            } else {
                value += c;
            }
            n = 0;
        }
    }
}

static void processStream_char(FILE *input_stream, StreamProcessor::StreamBuffer *output_stream, int *x_count, int *y_count, int *z_count, int *parent_x, int* parent_y, int *parent_z) {
    int num_parent_blocks = (*x_count / *parent_x) * (*y_count / *parent_y);

    ParentBlock *parent_blocks[num_parent_blocks];
    int uniform[num_parent_blocks];
    memset(uniform, 1, sizeof(int) * num_parent_blocks);
    //printf(": %d\n", num_parent_blocks);
    memset(parent_blocks, 0, sizeof(ParentBlock*) * num_parent_blocks);

    char ch;
    int n = 0;
    int x = 0;
    int y = 0;
    int z = 0;
    int blocks = 0;

    while ((ch = getc(input_stream)) != EOF) {
        if (ch == '\n') {
            n++;
            x = 0;
            y++;
            if (n == 2) {
                y = 0;
                z++;
            }
        } else if (ch != '\r') {
            int current_parent_block = (x / *parent_x) + (*x_count / *parent_x) * (y / *parent_y);
            if (parent_blocks[current_parent_block] == NULL) {
                parent_blocks[current_parent_block] = new ParentBlock{x, y, z, (char *)malloc(*parent_x * *parent_y * *parent_z), ch};
            }

            int parent_relative_x = x % *parent_x;
            int parent_relative_y = y % *parent_y;
            int parent_relative_z = z % *parent_z;
            // printf("[%d] (%d, %d, %d), (%d, %d, %d): %c\n", current_parent_block, x, y, z, parent_relative_x, parent_relative_y, parent_relative_z, ch);

            parent_blocks[current_parent_block]->block[(parent_relative_x * *parent_y * *parent_z) + (parent_relative_y * *parent_z) + parent_relative_z] = ch;
            if (ch != parent_blocks[current_parent_block]->first) {
                uniform[current_parent_block] = 0;
            }
            //fprintf(stderr, "(%d, %d, %d) %c\n", x, y, z, ch);
            if (parent_relative_x == *parent_x - 1 && parent_relative_y == *parent_y - 1 && parent_relative_z == *parent_z - 1) {
                //fprintf(stderr, " %d / %d (%d, %d, %d)\n", current_parent_block, num_parent_blocks, x, y, z);
                if (uniform[current_parent_block]/* && false */) {
                    free(parent_blocks[current_parent_block]->block);
                    parent_blocks[current_parent_block]->block = NULL;
                }
                blocks++;
                output_stream->push((void **)&parent_blocks[current_parent_block]);
                //free(parent_blocks[current_parent_block]);
                parent_blocks[current_parent_block] = NULL;
                // output_stream->printBuffer();
            }
            x++;
            n = 0;
        }
    }
    //fprintf(stderr, "Input End (%d, %d, %d) %d\n", x, y, z, blocks);
    output_stream->push(NULL);
}

// Function to process the slice of 3D block data
void StreamProcessor::InputStreamReader::processStream() {
    processStream_char(input_stream, output_stream, x_count, y_count, z_count, parent_x, parent_y, parent_z);
}

void StreamProcessor::InputStreamReader::processStream_test(const std::string& alg) {
    processStream_char(input_stream, output_stream, x_count, y_count, z_count, parent_x, parent_y, parent_z);
}

// print the header information and the 3D block data
void StreamProcessor::InputStreamReader::printHeader() {
    // print the header information
    fprintf(stderr, "%d,%d,%d,%d,%d,%d\n", *x_count, *y_count, *z_count, *parent_x, *parent_y, *parent_z);
    for (const auto &e : *tag_table)
    {
        fprintf(stderr, "%c,%s\n", e.first, e.second.c_str());
    }
}
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

void StreamProcessor::InputStreamReader::passValues(StreamProcessor *c_sp) {
    x_count = &(c_sp->x_count);
    y_count = &(c_sp->y_count);
    z_count = &(c_sp->z_count);
    parent_x = &(c_sp->parent_x);
    parent_y = &(c_sp->parent_y);
    parent_z = &(c_sp->parent_z);
    tag_table = &(c_sp->tag_table);
    output_stream = (c_sp->inputToCompressorBuffer);
    sp = c_sp;
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

static void processStream_char(FILE *input_stream, StreamProcessor::StreamBuffer *output_stream, int *x_count, int *y_count, int *z_count, int *parent_x, int* parent_y, int *parent_z, StreamProcessor *sp) {
    Chunk *chunk = NULL;
    int x = 0;
    char line[1024];

    //auto idx = [&](int x, int y, int z){ return x + (y * *parent_x) + (z * *parent_x * *parent_y); };

    for (int z = 0; z < *z_count; z++) {
        for (int y = 0; y < *y_count; y++) {
            if (fgets(line, 1024, input_stream) == NULL) fprintf(stderr, "ERROR READ LINE (%d, %d, %d)\n", x, y, z);
            //fprintf(stderr, "l: %s\n", line);
            int chunk_relative_z = z % *parent_z;
            if (chunk == NULL) {
                chunk = (Chunk*)malloc(sizeof(Chunk));
                chunk->id = z / *parent_z;
                chunk->block = (char*)malloc(*x_count * *y_count * *parent_z * sizeof(char));
            }
            //printf("[%d] (%d, %d, %d), (%d, %d, %d): %c\n", current_parent_block, x, y, z, parent_relative_x, parent_relative_y, parent_relative_z, ch);

            //fprintf(stderr, "cb: %d, idx: %d\n", current_parent_block+i, idx(0, parent_relative_y, parent_relative_z));
            memcpy(&(chunk->block[(*x_count * y) + (*x_count * *y_count * chunk_relative_z)]), line, *x_count);

            if (y == *y_count - 1 && chunk_relative_z == *parent_z - 1) {
                //fprintf(stderr, "I: (%d, %d, %d)\n", x, y, z);
                output_stream->push((void**)&chunk);
                chunk = NULL;
            }
        }
        fgets(line, 10, input_stream);
    }
    //fprintf(stderr, "Input End (%d, %d, %d) %d\n", x, y, z, blocks);
    output_stream->push(NULL);
}

// Function to process the slice of 3D block data
void StreamProcessor::InputStreamReader::processStream() {
    processStream_char(input_stream, output_stream, x_count, y_count, z_count, parent_x, parent_y, parent_z, sp);
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
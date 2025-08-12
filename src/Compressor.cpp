#include "Compressor.h"

Compressor::Compressor() {

}

Compressor::~Compressor() {}

void Compressor::compressParentBlock() {}

void Compressor::compressStream() {}
    
void Compressor::passValues(int *c_parent_x, int *c_parent_y, int *c_parent_z) {
    parent_x = c_parent_x;
    parent_y = c_parent_y;
    parent_z = c_parent_z;
}

void Compressor::passBuffers(StreamBuffer *c_input_stream, StreamBuffer *c_output_stream) {
    input_stream = c_input_stream;
    output_stream = c_output_stream;
}
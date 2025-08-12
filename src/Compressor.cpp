#include "Compressor.h"

Compressor::Compressor() {

}

Compressor::~Compressor() {}

void Compressor::compressParentBlock() {}

void Compressor::compressStream() {
    int pop_check;
    char *parent_block;
    do {
        pop_check = -1;
        do {
            pop_check = input_stream->pop((void**)&parent_block);
        } while(pop_check == -1);
        
        if (parent_block == NULL)
            break;

        //Do Stuff Here


        
        /*
        printf("Parent Block: %p\n", parent_block);
        for (int z = 0; z < *parent_z; z++) {
            for (int y = 0; y < *parent_y; y++) {
                for (int x = 0; x < *parent_x; x++) {
                    printf("%c", parent_block[(x * *parent_y * *parent_z) + (y * *parent_z) + z]);
                }
                printf("\n");
            }
            printf("\n");
        }
        */
    } while (parent_block != NULL);
}
    
void Compressor::passValues(int *c_parent_x, int *c_parent_y, int *c_parent_z) {
    parent_x = c_parent_x;
    parent_y = c_parent_y;
    parent_z = c_parent_z;
}

void Compressor::passBuffers(StreamBuffer *c_input_stream, StreamBuffer *c_output_stream) {
    input_stream = c_input_stream;
    output_stream = c_output_stream;
}
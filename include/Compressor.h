#pragma once
#include "header.h"

class Compressor {
    private:
        FILE *in;
    public:
        Compressor();
        ~Compressor();

        void compressParentBlock();
        void compressStream();
};
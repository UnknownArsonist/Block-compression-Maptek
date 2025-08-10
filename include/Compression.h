#pragma once
#include "header.h"

class Compression {
    private:
        FILE *in;
    public:
        Compression();
        ~Compression();

        void compressParentBlock();
        void compressStream();
};
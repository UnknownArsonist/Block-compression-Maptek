#pragma once
#include "header.h"

class StreamBuffer {
    private:
        char *buffer;
        char *write_ptr;
        char *read_ptr;
        int buf_size;
        int size_stored;
        std::mutex mutex;
    public:
        StreamBuffer(int buffer_size);
        StreamBuffer();
        ~StreamBuffer();
        int read(void *buf, int num_bytes);
        int write(void *buf, int num_bytes);
        void printBuffer();
};
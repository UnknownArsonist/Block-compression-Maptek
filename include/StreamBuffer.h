#pragma once
#include "header.h"

class StreamBuffer {
    private:
        void **buffer;
        void **write_ptr;
        void **read_ptr;
        int buf_size;
        int size_stored;

        std::mutex mutex;
    public:
        StreamBuffer();
        ~StreamBuffer();
        void setSize(int buffer_size);
        int pop(void **buf);
        int push(void **buf);
        void printBuffer();
};
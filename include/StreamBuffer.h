#pragma once
#include "header.h"
#include <condition_variable>

class StreamBuffer {
    private:
        void **buffer;
        void **write_ptr;
        void **read_ptr;
        int buf_size;
        int size_stored;

        std::condition_variable read_cv;
        std::condition_variable write_cv;

        std::mutex read_mutex;
        std::mutex write_mutex;
    public:
        StreamBuffer();
        ~StreamBuffer();
        void setSize(int buffer_size);
        int pop(void **buf);
        int push(void **buf);
        void printBuffer();
};
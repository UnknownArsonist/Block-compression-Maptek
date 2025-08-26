#pragma once
#include "header.h"

<<<<<<< HEAD
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
=======
class StreamBuffer
{
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
>>>>>>> 7a84fbe7280b4e0869f090e21fdebabaeccc7e4e
};
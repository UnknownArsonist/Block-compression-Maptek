#include "header.h"

class StreamBuffer {
    private:
        char *buffer;
        char *write_ptr;
        char *read_ptr;
        int buf_size;
        int size_stored;
    public:
        StreamBuffer(int buffer_size);
        ~StreamBuffer();
        int read(void *buf, int num_bytes);
        int write(void *buf, int num_bytes);
        void printBuffer();
};
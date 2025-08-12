#include "StreamBuffer.h"

StreamBuffer::StreamBuffer(int buffer_size) {
    buffer = (void**)malloc(buffer_size * sizeof(void*));
    buf_size = buffer_size;
    size_stored = 0;
    write_ptr = buffer;
    read_ptr = buffer;
}

StreamBuffer::StreamBuffer() {
    size_stored = 0;
}

StreamBuffer::~StreamBuffer() {
}

int StreamBuffer::pop(void *buf) {
    mutex.lock();
    if (size_stored <= 0) {
        return -1;
    }
    
    buf = read_ptr;
    read_ptr -= sizeof(void*);
    if (read_ptr < buffer) {
        read_ptr = &buffer[buf_size-1];
    }
    size_stored--;
    mutex.unlock();
    return 1;
}
/*
int StreamBuffer::read(void *buf, int num_bytes) {
    mutex.lock();
    if (size_stored < num_bytes) {
        return -1;
    }

    char *temp_read_ptr = read_ptr;
    int temp_num_bytes = num_bytes;
    int split = (read_ptr + num_bytes) - (buffer + buf_size);

    if (split > 0) {
        temp_num_bytes -= split;
        read_ptr = buffer + split;
        memcpy(((char*)buf) + temp_num_bytes, buffer, split);
    } else {
        read_ptr = read_ptr + temp_num_bytes;
    }

    memcpy(buf, temp_read_ptr, temp_num_bytes);
    size_stored -= num_bytes;
    mutex.unlock();
    return num_bytes;
}
*/

int StreamBuffer::push(void *buf) {
    mutex.lock();
    if (size_stored >= buf_size) {
        return -1;
    }

    buf = write_ptr;
    write_ptr += sizeof(void*);
    if (write_ptr > &buffer[buf_size-1]) {
        write_ptr = buffer;
    }
    size_stored++;
    mutex.unlock();
    return 1;
}

void StreamBuffer::printBuffer() {
    printf("[");
    for (int i = 0; i < buf_size; i++) {
        printf("%p", buffer[i]);
        if (i != buf_size - 1)
            printf(", ");
    }
    printf("]\n");
}
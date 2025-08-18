#include "StreamBuffer.h"

StreamBuffer::StreamBuffer() {
    size_stored = 0;
}

StreamBuffer::~StreamBuffer() {
}

void StreamBuffer::setSize(int buffer_size) {
    buffer = (void**)malloc(buffer_size * sizeof(void*));
    buf_size = buffer_size;
    size_stored = 0;
    write_ptr = buffer;
    read_ptr = buffer;
}

int StreamBuffer::pop(void **buf) {
    mutex.lock();
    
    while (size_stored <= 0) {
        mutex.unlock();
        mutex.lock();
    }

    *buf = *read_ptr;
    read_ptr += 1;
    if (read_ptr > &buffer[buf_size-1]) {
        read_ptr = buffer;
    }
    size_stored--;
    mutex.unlock();
    return 1;
}

int StreamBuffer::push(void **buf) {
    mutex.lock();
    if (buf == NULL) {
        char *null_ptr = NULL;
        buf = (void **)&null_ptr;
    }

    while (size_stored >= buf_size) {
        mutex.unlock();
        mutex.lock();
    }

    //printf("%d\n", size_stored);

    *write_ptr = *buf;
    write_ptr += 1;
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
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
    std::unique_lock<std::mutex> lk(read_mutex, std::defer_lock);
    
    read_cv.wait(lk, [this]{return size_stored > 0;});

    *buf = *read_ptr;
    read_ptr += 1;
    if (read_ptr > &buffer[buf_size-1]) {
        read_ptr = buffer;
    }
    size_stored--;
    if (size_stored == buf_size - 1)
        write_cv.notify_all();
    read_mutex.unlock();
    return 1;
}

int StreamBuffer::push(void **buf) {
    if (buf == NULL) {
        char *null_ptr = NULL;
        buf = (void **)&null_ptr;
    }
    std::unique_lock<std::mutex> lk(write_mutex, std::defer_lock);

    write_cv.wait(lk, [this]{return size_stored < buf_size;});

    //printf("%d\n", size_stored);

    *write_ptr = *buf;
    write_ptr += 1;
    if (write_ptr > &buffer[buf_size-1]) {
        write_ptr = buffer;
    }
    size_stored++;
    if (size_stored == 1)
        read_cv.notify_all();
    write_mutex.unlock();
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
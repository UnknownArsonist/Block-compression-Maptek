#include "StreamProcessor.h"

StreamProcessor::StreamBuffer::StreamBuffer(int c_buf_size, int c_num_writers) {
    buffer = (void**)malloc(c_buf_size * sizeof(void*));
    buf_size = c_buf_size;
    size_stored = 0;
    write_ptr = buffer;
    read_ptr = buffer;
    num_writers = c_num_writers;
    closed_writers = 0;
}

StreamProcessor::StreamBuffer::StreamBuffer(int c_buf_size) : StreamBuffer(c_buf_size, 1) {}

StreamProcessor::StreamBuffer::~StreamBuffer() {
}

//TODO error check for when setSize hasnt been called and buffer = NULL
int StreamProcessor::StreamBuffer::pop(void **buf) {
    std::unique_lock<std::mutex> lock(mutex);

    //fprintf(stderr, "Stored: %d / %d\n", size_stored, buf_size);
    // Wait until there's data available
    while (size_stored <= 0)
    {
        lock.unlock();
        std::this_thread::yield();
        lock.lock();
    }

    if (*read_ptr == NULL && size_stored > 0) {
        *buf = NULL;
        return 0;
    }

    *buf = *read_ptr;
    read_ptr++;
    //fprintf(stderr, "  %p\n", read_ptr);

    // Fix: Use modulo arithmetic for circular buffer
    if (read_ptr >= &(buffer[buf_size])) {
        read_ptr = buffer;
    }

    size_stored--;
    return 1;
}

int StreamProcessor::StreamBuffer::push(void **buf) {
    std::unique_lock<std::mutex> lock(mutex);

    //fprintf(stderr, "  Stored: %d / %d\n", size_stored, buf_size);
    if (buf == NULL) {
        closed_writers++;
        fprintf(stderr, "closed: %d / %d\n", closed_writers, num_writers);
        if (closed_writers >= num_writers) {
            char *null_ptr = NULL;
            buf = (void **)&null_ptr;
        } else {
            return -1;
        }
    }

    // Wait until there's space available
    while (size_stored >= buf_size-1)
    {
        lock.unlock();
        std::this_thread::yield();
        lock.lock();
    }

    *write_ptr = *buf;
    write_ptr++;

    // Fix: Use modulo arithmetic for circular buffer
    if (write_ptr >= &(buffer[buf_size]))
    {
        write_ptr = buffer;
    }

    size_stored++;
    return 1;
}

void StreamProcessor::StreamBuffer::printBuffer() {
    printf("[");
    for (int i = 0; i < buf_size; i++)
    {
        printf("%p", buffer[i]);
        if (i != buf_size - 1)
            printf(", ");
    }
    printf("]\n");
}
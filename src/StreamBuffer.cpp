#include "StreamProcessor.h"

StreamProcessor::StreamBuffer::StreamBuffer() {
    size_stored = 0;
}

StreamProcessor::StreamBuffer::~StreamBuffer() {
}

void StreamProcessor::StreamBuffer::setSize(int buffer_size) {
    buffer = (void**)malloc(buffer_size * sizeof(void*));
    buf_size = buffer_size;
    size_stored = 0;
    write_ptr = buffer;
    read_ptr = buffer;
}
//TODO error check for when setSize hasnt been called and buffer = NULL
int StreamProcessor::StreamBuffer::pop(void **buf) {
    std::unique_lock<std::mutex> lock(mutex);

    // Wait until there's data available
    while (size_stored <= 0)
    {
        lock.unlock();
        std::this_thread::yield();
        lock.lock();
    }

    *buf = *read_ptr;
    read_ptr++;

    // Fix: Use modulo arithmetic for circular buffer
    if (read_ptr >= buffer + buf_size)
    {
        read_ptr = buffer;
    }

    size_stored--;
    return 1;
}

int StreamProcessor::StreamBuffer::push(void **buf) {
    std::unique_lock<std::mutex> lock(mutex);

    if (buf == NULL)
    {
        char *null_ptr = NULL;
        buf = (void **)&null_ptr;
    }

    // Wait until there's space available
    while (size_stored >= buf_size)
    {
        lock.unlock();
        std::this_thread::yield();
        lock.lock();
    }

    *write_ptr = *buf;
    write_ptr++;

    // Fix: Use modulo arithmetic for circular buffer
    if (write_ptr >= buffer + buf_size)
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
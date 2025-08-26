#include "StreamBuffer.h"

<<<<<<< HEAD
StreamBuffer::StreamBuffer() {
    size_stored = 0;
}

StreamBuffer::~StreamBuffer() {
}

void StreamBuffer::setSize(int buffer_size) {
    buffer = (void**)malloc(buffer_size * sizeof(void*));
=======
StreamBuffer::StreamBuffer()
{
    size_stored = 0;
}

StreamBuffer::~StreamBuffer()
{
}

void StreamBuffer::setSize(int buffer_size)
{
    buffer = (void **)malloc(buffer_size * sizeof(void *));
>>>>>>> 7a84fbe7280b4e0869f090e21fdebabaeccc7e4e
    buf_size = buffer_size;
    size_stored = 0;
    write_ptr = buffer;
    read_ptr = buffer;
}

<<<<<<< HEAD
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
=======
int StreamBuffer::pop(void **buf)
{
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

int StreamBuffer::push(void **buf)
{
    std::unique_lock<std::mutex> lock(mutex);

    if (buf == NULL)
    {
>>>>>>> 7a84fbe7280b4e0869f090e21fdebabaeccc7e4e
        char *null_ptr = NULL;
        buf = (void **)&null_ptr;
    }

<<<<<<< HEAD
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
=======
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
void StreamBuffer::printBuffer()
{
    printf("[");
    for (int i = 0; i < buf_size; i++)
    {
>>>>>>> 7a84fbe7280b4e0869f090e21fdebabaeccc7e4e
        printf("%p", buffer[i]);
        if (i != buf_size - 1)
            printf(", ");
    }
    printf("]\n");
}
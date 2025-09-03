#include "StreamProcessor.h"

StreamProcessor::StreamBuffer::StreamBuffer(int c_num_writers) {
    num_writers = c_num_writers;
    closed_writers = 0;
    read_size_stored = 0;
    num_read = 0;
    write_size_stored = 0;
    num_write = 0;
    current_chunk = 0;
}

StreamProcessor::StreamBuffer::StreamBuffer() : StreamBuffer(1) {}

StreamProcessor::StreamBuffer::~StreamBuffer() {
    free(buffer);
}

void StreamProcessor::StreamBuffer::setSize(int c_buf_size) {
    buffer = (void**)malloc(c_buf_size * sizeof(void*));
    buf_size = c_buf_size;
    write_ptr = buffer;
    read_ptr = buffer;
}

//TODO error check for when setSize hasnt been called and buffer = NULL
int StreamProcessor::StreamBuffer::pop(void **buf) {
    std::unique_lock<std::mutex> lock(read_mutex);

    // Wait until there's data available
    //fprintf(stderr, "Pop: %d\n", size_stored);
    if (read_size_stored == 0) {
        //fprintf(stderr, "read = 0\n");
        std::unique_lock<std::mutex> write_lock(write_value_mutex);
        read_cond.wait(write_lock, [this]{
            read_size_stored += num_write;
            num_write = 0;
            return (read_size_stored > 0);
        });
    }
    //fprintf(stderr, "pop val, Stored: %d / %d\n", read_size_stored, buf_size);
    /* while (size_stored <= 0) {
        lock.unlock();
        std::this_thread::yield();
        lock.lock();
    } */

    if (*read_ptr == nullptr && read_size_stored > 0) {
        *buf = nullptr;
        return -1;
    }

    *buf = *read_ptr;
    read_ptr++;
    //fprintf(stderr, "  %p\n", read_ptr);

    // Fix: Use modulo arithmetic for circular buffer
    if (read_ptr >= &(buffer[buf_size])) {
        read_ptr = buffer;
    }

    read_value_mutex.lock();
    read_size_stored--;
    num_read++;
    read_value_mutex.unlock();
    write_cond.notify_all();

    return 1;
}

int StreamProcessor::StreamBuffer::push(void **buf) {
    std::unique_lock<std::mutex> lock(write_mutex);
    void *val = nullptr;
    if (buf == NULL) {
        closed_writers++;
        //fprintf(stderr, "closed: %d / %d\n", closed_writers, num_writers);
        if (closed_writers < num_writers) {
            return -1;
        }
    } else {
        val = *buf;
    }

    // Wait until there's space available
    /* if (size_stored >= buf_size-1 && buf_size > 8000)
        fprintf(stderr, "  Stored: %d / %d\n", size_stored, buf_size); */
    if (write_size_stored >= buf_size-1) {
        std::unique_lock<std::mutex> read_lock(read_value_mutex);
        write_cond.wait(read_lock, [this]{
            write_size_stored -= num_read;
            num_read = 0;
            return (write_size_stored < buf_size-1);
        });
    }
    //fprintf(stderr, "push val Stored: %d / %d\n", write_size_stored, buf_size);
    /* while (size_stored >= buf_size-1) {
        lock.unlock();
        std::this_thread::yield();
        lock.lock();
    } */

    *write_ptr = val;
    write_ptr++;
    // Fix: Use modulo arithmetic for circular buffer
    if (write_ptr >= &(buffer[buf_size])) {
        write_ptr = buffer;
    }

    write_value_mutex.lock();
    write_size_stored++;
    num_write++;
    //fprintf(stderr, "ws: %d\n", write_size_stored);
    write_value_mutex.unlock();
    read_cond.notify_all();
    return 1;
}

void StreamProcessor::StreamBuffer::printBuffer() {
    printf("[");
    for (int i = 0; i < buf_size; i++) {
        printf("%p", buffer[i]);
        if (i != buf_size - 1)
            printf(", ");
    }
    printf("]\n");
}
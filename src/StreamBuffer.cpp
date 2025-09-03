#include "StreamProcessor.h"

StreamProcessor::StreamBuffer::StreamBuffer(int c_num_writers) {
    num_writers = c_num_writers;
    closed_writers = 0;
    size_stored = 0;
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
    read_cond.wait(lock, [this]{ return (size_stored > 0); });
    //fprintf(stderr, "pop val, Stored: %d / %d\n", size_stored, buf_size);
    /* while (size_stored <= 0) {
        lock.unlock();
        std::this_thread::yield();
        lock.lock();
    } */

    if (*read_ptr == nullptr && size_stored > 0) {
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

    size_stored--;

    write_cond.notify_all();
    return 1;
}

int StreamProcessor::StreamBuffer::push(void **buf) {
    std::unique_lock<std::mutex> lock(mutex);
    //fprintf(stderr, "push val Stored: %d / %d\n", size_stored, buf_size);
    if (buf == NULL) {
        closed_writers++;
        //fprintf(stderr, "closed: %d / %d\n", closed_writers, num_writers);
        if (closed_writers >= num_writers) {
            //fprintf(stderr, "Stored: %d / %d\n", size_stored, buf_size);
            write_cond.wait(lock, [this]{ return (size_stored < buf_size-1); });
            //fprintf(stderr, "1\n");
            *write_ptr = nullptr;
            write_ptr++;
            if (write_ptr >= &(buffer[buf_size]))
                write_ptr = buffer;
            size_stored++;
            read_cond.notify_all();
            return 1;
        } else {
            return -1;
        }
    }

    // Wait until there's space available
    /* if (size_stored >= buf_size-1 && buf_size > 8000)
        fprintf(stderr, "  Stored: %d / %d\n", size_stored, buf_size); */
    write_cond.wait(lock, [this]{ return (size_stored < buf_size-1); });
    /* while (size_stored >= buf_size-1) {
        lock.unlock();
        std::this_thread::yield();
        lock.lock();
    } */

    *write_ptr = *buf;
    write_ptr++;

    // Fix: Use modulo arithmetic for circular buffer
    if (write_ptr >= &(buffer[buf_size])) {
        write_ptr = buffer;
    }

    size_stored++;
    read_cond.notify_all();
    return 1;
}

//Testing may not keep
void StreamProcessor::StreamBuffer::peek_next(void **buf) {
    std::unique_lock<std::mutex> lock(mutex);
    
    read_cond.wait(lock, [this]{ return (size_stored > 0); });

    if (*read_ptr == nullptr && size_stored > 0) {
        *buf = nullptr;
        return;
    }
    *buf = *read_ptr;
}

void StreamProcessor::StreamBuffer::setCurrentChunk(int cc) {
    std::unique_lock<std::mutex> lock(mutex);
    current_chunk = cc;
    write_cond.notify_all();
}

int StreamProcessor::StreamBuffer::getCurrentChunk() {
    std::unique_lock<std::mutex> lock(mutex);
    return current_chunk;
}

int StreamProcessor::StreamBuffer::getStored() {
    std::unique_lock<std::mutex> lock(mutex);
    return size_stored;
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
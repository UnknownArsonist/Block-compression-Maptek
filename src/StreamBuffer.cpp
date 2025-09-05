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

    if (read_ptr->prev == nullptr) {
        std::unique_lock<std::mutex> write_lock(write_mutex);
        read_cond.wait(write_lock, [this]{
            return (read_ptr != nullptr);
        });
    }

    if (read_ptr->value == nullptr) {
        *buf = nullptr;
        return -1;
    }

    *buf = *read_ptr;
    read_ptr++;

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
    //fprintf(stderr, "push val Stored: %d / %d\n", write_size_stored, buf_size);
    /* while (size_stored >= buf_size-1) {
        lock.unlock();
        std::this_thread::yield();
        lock.lock();
    } */
    item *prev_ptr = write_ptr;

    item *new_item = (item*)malloc(sizeof(item));
    new_item->value = val;
    new_item->prev_item = write_ptr;
    new_item->next_item = nullptr;
    
    write_ptr->next_item = new_item;
    write_ptr = new_item;
    // Fix: Use modulo arithmetic for circular buffer
    
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
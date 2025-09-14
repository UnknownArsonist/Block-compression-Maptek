#include "StreamProcessor.h"

StreamProcessor::StreamBuffer::StreamBuffer(int c_num_writers) {
    num_writers = c_num_writers;
    closed_writers = 0;
    read_size_stored = 0;
    num_write = 0;
}

StreamProcessor::StreamBuffer::StreamBuffer() : StreamBuffer(1) {}

StreamProcessor::StreamBuffer::~StreamBuffer() {
    free(write_ptr);
}

void StreamProcessor::StreamBuffer::setSize(int c_buf_size) {
    item *new_item = (item*)malloc(sizeof(item));
    new_item->value = nullptr;
    new_item->next_item = nullptr;
    write_ptr = new_item;
    read_ptr = new_item;
}
//TODO error check for when setSize hasnt been called and buffer = NULL
int StreamProcessor::StreamBuffer::pop(void **buf) {
    std::unique_lock<std::mutex> lock(read_mutex);

    while (read_size_stored == 0) {
        //std::unique_lock<std::mutex> write_lock(write_value_mutex);
        /* read_cond.wait(write_lock, [this]{
            read_size_stored = num_write;
            num_write = 0;
            return (read_size_stored > 0);
        }); */
        write_mutex.lock();
        read_size_stored = num_write;
        num_write = 0;
        write_mutex.unlock();
        lock.unlock();
        std::this_thread::yield();
        lock.lock();
    }

    if (read_ptr->value == nullptr) {
        *buf = nullptr;
        return -1;
    }

    *buf = read_ptr->value;
    item *t_next = read_ptr->next_item;
    free(read_ptr);
    read_ptr = t_next;
    read_size_stored--;

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

    item *new_item;
    new_item = (item*)malloc(sizeof(item));
    new_item->value = nullptr;
    new_item->next_item = nullptr;
    write_ptr->value = val;
    write_ptr->next_item = new_item;
    
    write_ptr = new_item;
    // Fix: Use modulo arithmetic for circular buffer
    write_value_mutex.lock();
    num_write++;
    write_value_mutex.unlock();
    //read_cond.notify_all();
    return 1;
}

void StreamProcessor::StreamBuffer::printBuffer() {
    printf("[");
    printf("]\n");
}
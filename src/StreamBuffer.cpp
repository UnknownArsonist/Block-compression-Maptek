#include "StreamBuffer.h"

StreamBuffer::StreamBuffer(int buffer_size) {
    buffer = (char*)malloc(buffer_size * sizeof(char));
    buf_size = buffer_size;
    size_stored = 0;
    write_ptr = buffer;
    read_ptr = buffer;
}

StreamBuffer::~StreamBuffer() {
    free(buffer);
}

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

int StreamBuffer::write(void *buf, int num_bytes) {
    mutex.lock();
    if (size_stored + num_bytes > buf_size) {
        return -1;
    }

    char *temp_write_ptr = write_ptr;
    int temp_num_bytes = num_bytes;
    int split = (write_ptr + num_bytes) - (buffer + buf_size);

    if (split > 0) {
        temp_num_bytes -= split;
        write_ptr = buffer + split;
        memcpy(buffer, ((char*)buf) + temp_num_bytes, split);
    } else {
        write_ptr = write_ptr + temp_num_bytes;
    }

    memcpy(temp_write_ptr, buf, temp_num_bytes);
    size_stored += num_bytes;
    mutex.unlock();
    return num_bytes;
}

void StreamBuffer::printBuffer() {
    printf("[");
    for (int i = 0; i < buf_size; i++) {
        printf("%c", buffer[i]);
        if (i != buf_size - 1)
            printf(", ");
    }
    printf("]\n");
}
#include <string>
#include <unordered_map>
#include <cstdio>
#include <thread>
#include <cstring>
#include <mutex>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#ifdef WIN32
#include <windows.h>
#endif
#include <omp.h>

struct SubBlock {
    uint16_t x;
    uint16_t y;
    uint16_t z;
    uint16_t l;
    uint16_t w;
    uint16_t h;
    char tag;
};

struct Chunk {
    int id;
    char *block;
    SubBlock** sub_blocks;
    int sub_block_num;
};

class StreamBuffer {
    public:
        StreamBuffer();
        StreamBuffer(int c_num_writers);
        ~StreamBuffer();
        int pop(void **buf);
        int push(void **buf);

    private:
        struct item {
            void *value;
            item *next_item;
        };

        item *write_ptr;
        item *read_ptr;
        int num_writers;
        int closed_writers;
        int num_write;
        int read_size_stored;

        std::mutex read_mutex;
        std::mutex write_mutex;
};

StreamBuffer::StreamBuffer(int c_num_writers) {
    num_writers = c_num_writers;
    closed_writers = 0;
    read_size_stored = 0;
    num_write = 0;
    item *new_item = (item*)malloc(sizeof(item));
    new_item->value = nullptr;
    new_item->next_item = nullptr;
    write_ptr = new_item;
    read_ptr = new_item;
}

StreamBuffer::StreamBuffer() : StreamBuffer(1) {}

StreamBuffer::~StreamBuffer() {
    free(write_ptr);
}

int StreamBuffer::pop(void **buf) {
    std::unique_lock<std::mutex> lock(read_mutex);

    if (read_size_stored == 0) {
        write_mutex.lock();
        while (num_write < 1) {
            write_mutex.unlock();
            lock.unlock();
            std::this_thread::yield();
            lock.lock();
            write_mutex.lock();
        }
        read_size_stored = num_write;
        num_write = 0;
        write_mutex.unlock();
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

int StreamBuffer::push(void **buf) {
    std::unique_lock<std::mutex> lock(write_mutex);
    void *val = nullptr;
    if (buf == NULL) {
        closed_writers++;
        
        if (closed_writers < num_writers) {
            return -1;
        }
    } else {
        val = *buf;
    }

    item *new_item;
    new_item = (item*)malloc(sizeof(item));
    if (new_item == NULL) {
        fprintf(stderr, "ERROR NO MEMORY\n");
        return -1;
    }
    new_item->value = nullptr;
    new_item->next_item = nullptr;
    write_ptr->value = val;
    write_ptr->next_item = new_item;
    
    write_ptr = new_item;
    num_write++;
    return 1;
}

void getCommaSeparatedValuesFromStream() {}

template <typename T, typename... Args>
void getCommaSeparatedValuesFromStream(T *value, Args... args) {
    char c;
    *value = 0;
    while ((c = getc(stdin)) != EOF) {
        if (c == ',' || c == '\n') {
            break;
        }
        else if (c != '\r') {
            *value *= 10;
            *value += (int)c - '0';
        }
    }
    getCommaSeparatedValuesFromStream(args...);
}

void getLegendFromStream(std::unordered_map<char, std::string> *legend) {
    char c;
    char key = 0;
    std::string value = "";
    int v = 0;
    int n = 0;
    while ((c = getc(stdin)) != EOF) {
        if (c == ',') {
            v++;
        } else if (c == '\n') {
            if (n > 0) {
                return;
            }
            (*legend)[key] = value;
            v = 0;
            value.clear();
            n++;
        } else if (c != ' ') {
            if (v == 0) {
                key = c;
            } else {
                value += c;
            }
            n = 0;
        }
    }
}

static void processStream_char(FILE *input_stream, StreamBuffer *output_stream, int x_count, int y_count, int z_count, int parent_x, int parent_y, int parent_z) {
    Chunk *chunk = NULL;
    int x = 0;
    char line[1024];

    for (int z = 0; z < z_count; z++) {
        for (int y = 0; y < y_count; y++) {
            if (fgets(line, 1024, input_stream) == NULL) fprintf(stderr, "ERROR READ LINE (%d, %d, %d)\n", x, y, z);

            int chunk_relative_z = z % parent_z;
            if (chunk == NULL) {
                chunk = (Chunk*)malloc(sizeof(Chunk));
                chunk->id = z / parent_z;
                chunk->block = (char*)malloc(x_count * y_count * parent_z * sizeof(char));
            }

            memcpy(&(chunk->block[(x_count * y) + (x_count * y_count * chunk_relative_z)]), line, x_count);

            if (y == y_count - 1 && chunk_relative_z == parent_z - 1) {
                output_stream->push((void**)&chunk);
                chunk = NULL;
            }
        }
        fgets(line, 10, input_stream);
    }
    output_stream->push(NULL);
}

void processChunk(Chunk *chunk, StreamBuffer *output_stream, int x_count, int y_count, int z_count, int parent_x, int parent_y, int parent_z) {
    chunk->sub_blocks = (SubBlock**)malloc(x_count * y_count * parent_z * sizeof(SubBlock*));
    chunk->sub_block_num = 0;
    int *sub_block_nums = (int*)calloc((x_count / parent_x) * (y_count / parent_y), sizeof(int));
    bool *zeros = (bool*)calloc(parent_x, sizeof(bool));
#   pragma omp parallel for collapse(2) schedule(dynamic)
    for (int ty = 0; ty < (y_count / parent_y); ty++) {
        for (int tx = 0; tx < (x_count / parent_x); tx++) {
            int px = tx * parent_x;
            int py = ty * parent_y;
            // Now run greedy meshing *inside* these boundaries.
            bool *visited = (bool*)calloc(parent_x * parent_y * parent_z, sizeof(bool));
            for (int z = 0; z < parent_z; z++) {
                for (int y = 0; y < parent_y; y++) {
                    for (int x = 0; x < parent_x; x++) {
                        if (visited[(z * parent_y * parent_x) + (y * parent_x) + x])
                            continue;

                        char target = chunk->block[px + x + ((py + y) * x_count) + (z * x_count * y_count)];
                        // Determine max size in X
                        int maxX = x + 1; // 0 1 7
                        while (maxX < parent_x && chunk->block[px + maxX + ((py + y) * x_count) + (z * x_count * y_count)] == target && !visited[(z * parent_y * parent_x) + (y * parent_x) + maxX]) {
                            maxX++; // 1 7
                        }
                        int width = maxX - x;
                        // Determine max size in Y
                        int maxY = y + 1;         // 0
                        while (maxY < parent_y)
                        {
                            bool uniformY = true;
                            // x = 0 -> maxX = 1; 1 < 7
                            if (memcmp(&(chunk->block[px + x + ((py + y) * x_count) + (z * x_count * y_count)]), &(chunk->block[px + x + ((py + maxY) * x_count) + (z * x_count * y_count)]), width) != 0 || memcmp(&(visited[(z * parent_y * parent_x) + (maxY * parent_x) + x]), zeros, width) != 0) {
                                uniformY = false;
                            }
                            if (!uniformY)
                                break;
                            maxY++;
                        }

                        // Determine max size in Z
                        int maxZ = z + 1;
                        // checks the subblocks, are they uniform and did we alreadly visit them.
                        while (maxZ < parent_z)
                        {
                            bool uniformZ = true;
                            // y = 0 maxY = 1
                            for (int yi = y; yi < maxY; yi++)
                            {
                                // x = 0 MaxX = 1
                                if (memcmp(&(chunk->block[px + x + ((py + y) * x_count) + (z * x_count * y_count)]), &(chunk->block[px + x + ((py + yi) * x_count) + (maxZ * x_count * y_count)]), width) != 0 || memcmp(&(visited[(maxZ * parent_y * parent_x) + (yi * parent_x) + x]), zeros, width) != 0) {
                                    uniformZ = false;
                                }
                                if (!uniformZ)
                                    break;
                            }
                            if (!uniformZ)
                                break;
                            maxZ++;
                        }

                        // Mark all as visited
                        for (int zz = z; zz < maxZ; zz++)
                            for (int yy = y; yy < maxY; yy++)
                                for (int xx = x; xx < maxX; xx++)
                                    visited[(zz * parent_y * parent_x) + (yy * parent_x) + xx] = true;

                        // Output the packed block
                        SubBlock *sub_block = (SubBlock *)malloc(sizeof(SubBlock));
                        sub_block->x = px + x;
                        sub_block->y = py + y;
                        sub_block->z = (chunk->id * parent_z) + z;
                        sub_block->l = maxX - x;
                        sub_block->w = maxY - y;
                        sub_block->h = maxZ - z;
                        sub_block->tag = target;
                        chunk->sub_blocks[((parent_x * parent_y * parent_z) * ((ty * (x_count / parent_x)) + tx)) + sub_block_nums[(ty * (x_count / parent_x)) + tx]] = sub_block;
                        sub_block_nums[(ty * (x_count / parent_x)) + tx]++;
                    }
                    // x += 1; x = 1; x = 2
                }
                // y = 1
            }
            free(visited);
        }
    }
    chunk->sub_block_num = sub_block_nums[0];
    for (int i = 1; i < (x_count / parent_x) * (y_count / parent_y); i++) {
        memcpy(&(chunk->sub_blocks[chunk->sub_block_num]), &(chunk->sub_blocks[((parent_x * parent_y * parent_z) * i)]), sub_block_nums[i] * sizeof(SubBlock*));
        chunk->sub_block_num += sub_block_nums[i];
    }
    free(sub_block_nums);
    free(chunk->block);
    free(zeros);
    output_stream->push((void **)&(chunk));
}

void compressStream(StreamBuffer *input_stream, StreamBuffer *output_stream, int x_count, int y_count, int z_count, int parent_x, int parent_y, int parent_z) {
    Chunk *chunk;

    do {
        input_stream->pop((void **)&chunk);

        if (chunk == nullptr) {
            output_stream->push(NULL);
            break;
        }

        // Safety check: if we've processed too many blocks, use simpler algorithm
        processChunk(chunk, output_stream, x_count, y_count, z_count, parent_x, parent_y, parent_z);

    } while (chunk != NULL);
}

static int intToChar(int num, char *r) {
    int n = 4;
    if (num < 10) {
        r[0] = num + '0';
        return 1;
    } else if (num < 100) {
        n = 1;
    } else if (num < 1000) {
        n = 2;
    } else if (num < 10000) {
        n = 3;
    }
    int writ = n;
    while (num > 0) {
        r[n] = (num >= 10 ? num % 10 : num) + '0';
        num /= 10;
        n--;
    }
    return writ+1;
}

static int writeCommaSepNumsToBuffer(char *buffer, int start) {
    return 0;
}

template <typename T, typename... Args>
static int writeCommaSepNumsToBuffer(char *buffer, int start, T num, Args... args) {
    int written = intToChar(num, buffer+start);
    buffer[start+written] = ',';
    return written + 1 + writeCommaSepNumsToBuffer(buffer, start+written+1, args...);
}

static int writeSubBlocksToBuffer(char *buffer, int start, SubBlock **sb, int num_blocks, std::unordered_map<char, std::string> legend) {
    int written = 0;
    for (int i = 0; i < num_blocks; i++) {
        written += writeCommaSepNumsToBuffer(buffer, start + written, sb[i]->x, sb[i]->y, sb[i]->z, sb[i]->l, sb[i]->w, sb[i]->h);
        memcpy(buffer+start+written, legend[sb[i]->tag].c_str(), legend[sb[i]->tag].length());
        buffer[start+written+legend[sb[i]->tag].length()] = '\n';
        written += legend[sb[i]->tag].length()+1;
        free(sb[i]);
    }
    return written;
}

int printSubBlocks(HANDLE hStdout, Chunk *pb, char *buffer, int buf_size, int stored, std::unordered_map<char, std::string> legend) {
    int sbb_size_char = 35;
    int num_blocks_per_buf = buf_size / sbb_size_char;
    int remaining = pb->sub_block_num;
    int i = 0;
    while (remaining >= 0) {
        int num_b = remaining > num_blocks_per_buf ? num_blocks_per_buf : remaining;
        if ((num_b * sbb_size_char) + stored >= buf_size) {
            WriteFile(hStdout, buffer, stored, NULL, NULL);
            stored = 0;
        }
        int len = writeSubBlocksToBuffer(buffer, stored, &(pb->sub_blocks[i*num_blocks_per_buf]), num_b, legend);
        remaining -= num_blocks_per_buf;
        stored += len;
        
        i++;
    }
    return stored;
}

void displayBlocks(StreamBuffer *input_stream, std::unordered_map<char, std::string> legend) {
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

    int buf_size = 16384;
    int stored = 0;
    char *buffer = (char*)malloc(sizeof(char) * buf_size);

    Chunk *chunk = nullptr;
    do {
        input_stream->pop((void **)&chunk);

        if (chunk == nullptr) {
            break;
        }
        stored = printSubBlocks(hStdout, chunk, buffer, buf_size, stored, legend);
        free(chunk->sub_blocks);
        free(chunk);
    } while (chunk != NULL);
    if (stored > 0) {
        WriteFile(hStdout, buffer, stored, NULL, NULL);
        stored = 0;
    }
    free(buffer);
    CloseHandle(hStdout);
}

int main(int argc, char **argv) {
    std::ios_base::sync_with_stdio(false);
    omp_set_nested(1);
    bool verbose = true;
    auto started = std::chrono::high_resolution_clock::now();
    int x_count, y_count, z_count, parent_x, parent_y, parent_z;
    std::unordered_map<char, std::string> legend;
    getCommaSeparatedValuesFromStream(&x_count, &y_count, &z_count, &parent_x, &parent_y, &parent_z);
    getLegendFromStream(&legend);
    if (verbose) {
        fprintf(stderr, "%d,%d,%d,%d,%d,%d\n", x_count, y_count, z_count, parent_x, parent_y, parent_z);
        for (const auto &e : legend) {
            fprintf(stderr, "%c,%s\n", e.first, e.second.c_str());
        }
    }

    StreamBuffer *inputToCompressorBuffer = new StreamBuffer();
    StreamBuffer *compressorToOutputBuffer = new StreamBuffer();
#   pragma omp parallel sections num_threads(3) default(shared)
    {
#       pragma omp section
        {
            if (verbose) {
                auto end = std::chrono::high_resolution_clock::now();
                fprintf(stderr, "Starting Compressor Thread:\n  %lld\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - started).count());
            }
            compressStream(inputToCompressorBuffer, compressorToOutputBuffer, x_count, y_count, z_count, parent_x, parent_y, parent_z);
            if (verbose) {
                auto end = std::chrono::high_resolution_clock::now();
                fprintf(stderr, "Compressor Runtime:\n  %lld\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - started).count());
            }
        }
#       pragma omp section
        {
            if (verbose) {
                auto end = std::chrono::high_resolution_clock::now();
                fprintf(stderr, "Starting Display Thread:\n  %lld\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - started).count());
            }
            displayBlocks(compressorToOutputBuffer, legend);
            if (verbose) {
                auto end = std::chrono::high_resolution_clock::now();
                fprintf(stderr, "Output Runtime:\n  %lld\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - started).count());
            }
        }
#       pragma omp section
        {
            if (verbose) {
                auto end = std::chrono::high_resolution_clock::now();
                fprintf(stderr, "Starting InputStreamReader:\n  %lld\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - started).count());
            }
            processStream_char(stdin, inputToCompressorBuffer, x_count, y_count, z_count, parent_x, parent_y, parent_z);
            if (verbose) {
                auto end = std::chrono::high_resolution_clock::now();
                fprintf(stderr, "InputStreamReader Runtime:\n  %lld\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - started).count());
            }
        }
    }
}
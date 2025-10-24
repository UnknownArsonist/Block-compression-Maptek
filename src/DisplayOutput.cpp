#include "StreamProcessor.h"

StreamProcessor::DisplayOutput::DisplayOutput() {}
StreamProcessor::DisplayOutput::~DisplayOutput() {}

void StreamProcessor::DisplayOutput::printSubBlock(SubBlock *sb) {
    printf("%d,%d,%d,%d,%d,%d,%s\n", sb->x, sb->y, sb->z, sb->l, sb->w, sb->h, (*tag_table)[sb->tag].c_str());
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
        r[n] = (num % 10) + '0';
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

static int writeSubBlocksToBuffer(char *buffer, int start, SubBlock **sb, int num_blocks, std::unordered_map<char, std::string> *tag_table) {
    int written = 0;
    for (int i = 0; i < num_blocks; i++) {
        //fprintf(stderr, " [%d] %d,%d,%d,%d,%d,%d\n", i, sb[i]->x, sb[i]->y, sb[i]->z, sb[i]->l, sb[i]->w, sb[i]->h);
        written += writeCommaSepNumsToBuffer(buffer, start + written, sb[i]->x, sb[i]->y, sb[i]->z, sb[i]->l, sb[i]->w, sb[i]->h);
        memcpy(buffer+start+written, (*tag_table)[sb[i]->tag].c_str(), (*tag_table)[sb[i]->tag].length());
        buffer[start+written+(*tag_table)[sb[i]->tag].length()] = '\n';
        written += (*tag_table)[sb[i]->tag].length()+1;
        free(sb[i]);
    }
    return written;
}

#ifdef WIN32
void StreamProcessor::DisplayOutput::printSubBlock(HANDLE hStdout, SubBlock *sb) {
    char buf[64];
    int len = writeSubBlocksToBuffer(buf, 0, &sb, 1, tag_table);
    //int len = snprintf(buffer, sizeof(buffer), "%d,%d,%d,%d,%d,%d,%s\n", sb->x, sb->y, sb->z, sb->l, sb->w, sb->h, (*tag_table)[sb->tag].c_str());
    //fprintf(stderr, "%d,%d,%d,%d,%d,%d,%s  [%d]\n", sb->x, sb->y, sb->z, sb->l, sb->w, sb->h, (*tag_table)[sb->tag].c_str(), written+(*tag_table)[sb->tag].length()+1);
    WriteFile(hStdout, buf, len, NULL, NULL);
}

void StreamProcessor::DisplayOutput::printSubBlocks(HANDLE hStdout, Chunk *pb) {
    int sbb_size_char = 35;
    int num_blocks_per_buf = buf_size / sbb_size_char;
    int remaining = pb->sub_block_num;
    int i = 0;
    while (remaining >= 0) {
        //fprintf(stderr, "[%d] (%d,%d,%d) %d/%d\n", i, pb->sub_blocks[0]->x, pb->sub_blocks[0]->y, pb->sub_blocks[0]->z, i*num_blocks_per_buf, pb->sub_block_num);
        int num_b = remaining > num_blocks_per_buf ? num_blocks_per_buf : remaining;
        //fprintf(stderr, "s %d, %d\n", stored, num_b);
        if ((num_b * sbb_size_char) + stored >= buf_size) {
            WriteFile(hStdout, buffer, stored, NULL, NULL);
            stored = 0;
        }
        int len = writeSubBlocksToBuffer(buffer, stored, &(pb->sub_blocks[i*num_blocks_per_buf]), num_b, tag_table);
        //fprintf(stderr, "l %d\n", len);
        remaining -= num_blocks_per_buf;
        stored += len;
        
        //int len = snprintf(buffer, sizeof(buffer), "%d,%d,%d,%d,%d,%d,%s\n", sb->x, sb->y, sb->z, sb->l, sb->w, sb->h, (*tag_table)[sb->tag].c_str());
        //fprintf(stderr, "%d,%d,%d,%d,%d,%d,%s  [%d]\n", sb->x, sb->y, sb->z, sb->l, sb->w, sb->h, (*tag_table)[sb->tag].c_str(), written+(*tag_table)[sb->tag].length()+1);
        //fprintf(stderr, "len: %d\n", len);
        //fprintf(stderr, "Written\n");
        i++;
    }
}

void StreamProcessor::DisplayOutput::displayBlocks() {
    // TODO check for when input_stream or tag_table not set
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    /* bool valid_handle = true;
    if (hStdout == INVALID_HANDLE_VALUE) {
        valid_handle = false;
    } */
    int num_chunk = 10;
    int current_chunk = 0;
    std::vector<Chunk*> next_blocks;
    next_blocks.resize(num_chunk, NULL);
    int next_count = 0;

    buf_size = 2048;
    stored = 0;
    buffer = (char*)malloc(sizeof(char) * buf_size);

    Chunk *chunk = nullptr;
    do {
        input_stream->pop((void **)&chunk);

        if (chunk == nullptr) {
            //fprintf(stderr, "NULL END\n");
            break;
        }
        //fprintf(stderr, "Display: %d,%d,%d  %d\n", parent_block->x, parent_block->y, parent_block->z, parent_block->sub_block_num);
        
        if (chunk->id == current_chunk) {
            printSubBlocks(hStdout, chunk);
            free(chunk->sub_blocks);
            free(chunk);
            //fprintf(stderr, "cc: %d\n", current_chunk);
            current_chunk++;
            while (next_count > 0) {
                int p_count = 0;
                for (int i = 0; i < next_count; i++) {
                    int t_chunk = next_blocks[i]->id;
                    if (t_chunk == current_chunk) {
                        printSubBlocks(hStdout, next_blocks[i]);
                        free(next_blocks[i]->sub_blocks);
                        free(next_blocks[i]);
                        p_count++;
                    } else {
                        next_blocks[i-p_count] = next_blocks[i];
                    }
                }
                next_count = next_count - p_count;
                if (p_count > 0) {
                    current_chunk++;
                } else {
                    break;
                }
            }
        } else {
            if (next_count >= (int)next_blocks.capacity()) {
                next_blocks.resize(next_blocks.capacity() + num_chunk, NULL);
                fprintf(stderr, "next_block = %d / %d\n", next_count, num_chunk);
            }
            next_blocks[next_count] = chunk;
            next_count++;
        }
        //fprintf(stderr, "%d,%d,%d,%d,%d,%d,%s\n", sub_block->x, sub_block->y, sub_block->z, sub_block->l, sub_block->w, sub_block->h, (*tag_table)[sub_block->tag].c_str());
    } while (chunk != NULL);
    if (stored > 0) {
        WriteFile(hStdout, buffer, stored, NULL, NULL);
        stored = 0;
    }
    free(buffer);
    CloseHandle(hStdout);
}

/* void StreamProcessor::DisplayOutput::displayBlocks() {
    // TODO check for when input_stream or tag_table not set
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    bool valid_handle = true;
    if (hStdout == INVALID_HANDLE_VALUE) {
        valid_handle = false;
    }
    int num_parent_blocks = (*x_count / *parent_x) * (*y_count / *parent_y);
    int current_chunk = 0;
    int count = 0;
    int count_next = 0;
    int stored_sb = 0;
    int sbb_size = 8192;

    // FIX: CAN/WILL BUFFER OVERFLOW
    SubBlock **sub_block_buf = (SubBlock**)malloc(sizeof(SubBlock*) * sbb_size * 2);
    SubBlock *sub_block;
    do {
        input_stream->pop((void **)&sub_block);

        if (sub_block == nullptr) {
            //fprintf(stderr, "NULL END\n");
            break;
        }
        if (sub_block->tag == 1) {
            if (sub_block->x == current_chunk) {
                count++;
            } else if (sub_block->x == current_chunk + 1) {
                count_next++;
            }
            //fprintf(stderr, "pb_count: %d, chunk: %d\n", count, sub_block->x);
            if (count >= num_parent_blocks) {
                current_chunk = current_chunk+1;
                count = count_next;
                count_next = 0;
                for (int i = 0; i < stored_sb; i++) {
                    if (valid_handle) {
                        printSubBlock(hStdout, sub_block_buf[i + (((current_chunk-1)%2) * sbb_size)]);
                    } else {
                        printSubBlock(sub_block_buf[i + (((current_chunk-1)%2) * sbb_size)]);
                    }
                }
                stored_sb = 0;
            }
            free(sub_block);
            continue;
        }
        int sb_chunk = sub_block->z / *parent_z;
        if (sb_chunk == current_chunk) {
            if (valid_handle) {
                printSubBlock(hStdout, sub_block);
            } else {
                printSubBlock(sub_block);
            }
            free(sub_block);
        } else {
            sub_block_buf[stored_sb + ((current_chunk%2) * sbb_size)] = sub_block;
            stored_sb++;
        }
        //fprintf(stderr, "%d,%d,%d,%d,%d,%d,%s\n", sub_block->x, sub_block->y, sub_block->z, sub_block->l, sub_block->w, sub_block->h, (*tag_table)[sub_block->tag].c_str());
    } while (sub_block != NULL);
    CloseHandle(hStdout);
} */
#else
void StreamProcessor::DisplayOutput::displayBlocks() {
    // TODO check for when input_stream or tag_table not set

    SubBlock *sub_block;
    do {
        input_stream->pop((void **)&sub_block);

        if (sub_block == NULL)
            break;
        printSubBlock(sub_block);
        free(sub_block);
    } while (sub_block != NULL);
}
#endif


void StreamProcessor::DisplayOutput::passValues(std::unordered_map<char, std::string> *c_tag_table) {
    tag_table = c_tag_table;
}

void StreamProcessor::DisplayOutput::passValues(StreamProcessor *sp) {
    x_count = &(sp->x_count);
    y_count = &(sp->y_count);
    z_count = &(sp->z_count);
    parent_x = &(sp->parent_x);
    parent_y = &(sp->parent_y);
    parent_z = &(sp->parent_z);
    tag_table = &(sp->tag_table);
    input_stream = sp->compressorToOutputBuffer;
}

void StreamProcessor::DisplayOutput::passBuffers(StreamBuffer *c_input_stream) {
    input_stream = c_input_stream;
}

void StreamProcessor::DisplayOutput::setVerbose(bool c_v) {
    verbose = c_v;
}
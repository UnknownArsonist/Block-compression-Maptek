#include "StreamProcessor.h"

StreamProcessor::DisplayOutput::DisplayOutput() {}
StreamProcessor::DisplayOutput::~DisplayOutput() {}

void StreamProcessor::DisplayOutput::printSubBlock(SubBlock *sb) {
    printf("%d,%d,%d,%d,%d,%d,%s\n", sb->x, sb->y, sb->z, sb->l, sb->w, sb->h, (*tag_table)[sb->tag].c_str());
}

#ifdef WIN32
void StreamProcessor::DisplayOutput::printSubBlock(HANDLE hStdout, SubBlock *sb) {
    char buffer[64];
    int len = snprintf(buffer, sizeof(buffer), "%d,%d,%d,%d,%d,%d,%s\n", sb->x, sb->y, sb->z, sb->l, sb->w, sb->h, (*tag_table)[sb->tag].c_str());
    WriteFile(hStdout, buffer, len, NULL, NULL);
}

void StreamProcessor::DisplayOutput::displayBlocks() {
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
}
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
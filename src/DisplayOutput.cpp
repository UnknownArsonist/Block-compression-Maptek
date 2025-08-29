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
    SubBlock *sub_block;
    do {
        input_stream->pop((void **)&sub_block);

        if (sub_block == NULL)
            break;
        if (valid_handle) {
            printSubBlock(hStdout, sub_block);
        } else {
            printSubBlock(sub_block);
        }
        free(sub_block);
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
    tag_table = &(sp->tag_table);
    input_stream = sp->compressorToOutputBuffer;
}

void StreamProcessor::DisplayOutput::passBuffers(StreamBuffer *c_input_stream) {
    input_stream = c_input_stream;
}

void StreamProcessor::DisplayOutput::setVerbose(bool c_v) {
    verbose = c_v;
}
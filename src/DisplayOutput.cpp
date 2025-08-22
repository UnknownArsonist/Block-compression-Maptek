#include "DisplayOutput.h"

DisplayOutput::DisplayOutput() {}
DisplayOutput::~DisplayOutput() {}

void DisplayOutput::printSubBlock(SubBlock *sb)
{
    printf("%d,%d,%d,%d,%d,%d,%s\n", sb->x, sb->y, sb->z, sb->l, sb->w, sb->h, (*tag_table)[sb->tag].c_str());
}

void DisplayOutput::displayBlocks()
{
    // TODO check for when input_stream or tag_table not set

    SubBlock *sub_block;
    do
    {
        input_stream->pop((void **)&sub_block);

        if (sub_block == NULL)
            break;
        printSubBlock(sub_block);
        free(sub_block);
    } while (sub_block != NULL);
}

void DisplayOutput::passValues(std::unordered_map<char, std::string> *c_tag_table)
{
    tag_table = c_tag_table;
}

void DisplayOutput::passBuffers(StreamBuffer *c_input_stream)
{
    input_stream = c_input_stream;
}

void DisplayOutput::setVerbose(bool c_v)
{
    verbose = c_v;
}
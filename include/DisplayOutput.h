#pragma once
#include "header.h"
#include "StreamBuffer.h"

class DisplayOutput {
    private:
        StreamBuffer *input_stream;
        std::unordered_map<char, std::string> *tag_table;
        bool verbose = false;

    public:
        DisplayOutput();
        ~DisplayOutput();
        void displayBlocks();
        void printSubBlock(SubBlock *sb);
        void passBuffers(StreamBuffer *c_input_stream);
        void passValues(std::unordered_map<char, std::string> *c_tag_table);
        void setVerbose(bool c_v);
};
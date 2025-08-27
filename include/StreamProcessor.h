#pragma once
#include "header.h"

class StreamProcessor {
    public:
        class ProcessorModule;
        class InputStreamReader;
        class Compressor;
        class DisplayOutput;
        class StreamBuffer;

        StreamProcessor();
        ~StreamProcessor();

        void setVerbose(bool c_v);

        void setup();
        void start();
        void end();

    private:
        InputStreamReader *inputStreamReader;
        Compressor *compressor;
        DisplayOutput *displayOutput;
        StreamBuffer *inputToCompressorBuffer;
        StreamBuffer *compressorToOutputBuffer;

        std::chrono::time_point<std::chrono::high_resolution_clock> started;
        std::thread inputStreamReaderThread;
        std::thread compressorThread;
        std::thread displayOutputThread;
        bool verbose = false;

        // Dimensions of the 3D block
        int x_count;
        int y_count;
        int z_count;

        // Parent Block dimensions
        int parent_x;
        int parent_y;
        int parent_z;

        // Tag table to map characters to strings
        std::unordered_map<char, std::string> tag_table;
};

//TODO Abstract class for Module
class StreamProcessor::ProcessorModule {
    public:
        virtual void passValues(StreamProcessor *sp) = 0;
};

class StreamProcessor::InputStreamReader : public StreamProcessor::ProcessorModule {
    public:
        // Constructor and Destructor
        InputStreamReader();
        InputStreamReader(FILE *in);
        ~InputStreamReader();

        // Function declarations
        void processStream();
        void processStream_test(const std::string& alg);

        void passValues(StreamProcessor *sp);
        void passValues(int *c_x_count, int *c_y_count, int *c_z_count, int *c_parent_x, int *c_parent_y, int *c_parent_z, std::unordered_map<char, std::string> *c_tag_table);
        void passBuffers(StreamBuffer *output_stream);
        void getHeader();
        void getCommaSeparatedValuesFromStream();
        template <typename T, typename... Args>
        void getCommaSeparatedValuesFromStream(T *value, Args... args);
        void getLegendFromStream(std::unordered_map<char, std::string> *legend);
        // void startProcessing();

        void printHeader();

    private:
        // Vectors to hold dimensions and tokens
        // vector<string> dims;

        // Header line and other variables
        // string header_line;
        // vector<string> slice;

        // Dimensions of the 3D block
        int *x_count;
        int *y_count;
        int *z_count;

        // Parent Block dimensions
        int *parent_x;
        int *parent_y;
        int *parent_z;

        FILE *input_stream;
        StreamProcessor::StreamBuffer *output_stream;

        // Tag table to map characters to strings
        std::unordered_map<char, std::string> *tag_table;
};

class StreamProcessor::Compressor : public StreamProcessor::ProcessorModule {
    public:
        Compressor();
        ~Compressor();

        // algorithms
        void OctreeCompression(ParentBlock *parent_block);

        // helper function
        void processParentBlocks(ParentBlock *parent_block);
        void printParentBlock(const std::vector<std::vector<std::vector<std::vector<char>>>> &parent_blocks);
        void compressStream();
        void passValues(StreamProcessor *sp);
        void passValues(int *c_parent_x, int *c_parent_y, int *c_parent_z, std::unordered_map<char, std::string> *c_tag_table);
        void passBuffers(StreamBuffer *c_input_stream, StreamBuffer *c_output_stream);

    private:
        // Parent Block dimensions
        int *parent_x;
        int *parent_y;
        int *parent_z;

        StreamBuffer *input_stream;
        StreamBuffer *output_stream;
        std::unordered_map<char, std::string> *tag_table;
};

class StreamProcessor::DisplayOutput : public StreamProcessor::ProcessorModule {
    public:
        DisplayOutput();
        ~DisplayOutput();
        void displayBlocks();
        void printSubBlock(SubBlock *sb);
        void passBuffers(StreamBuffer *c_input_stream);
        void passValues(StreamProcessor *sp);
        void passValues(std::unordered_map<char, std::string> *c_tag_table);
        void setVerbose(bool c_v);
        
    private:
        StreamProcessor::StreamBuffer *input_stream;
        std::unordered_map<char, std::string> *tag_table;
        bool verbose = false;
};

class StreamProcessor::StreamBuffer {
    public:
        StreamBuffer();
        ~StreamBuffer();
        void setSize(int buffer_size);
        int pop(void **buf);
        int push(void **buf);
        void printBuffer();

    private:
        void **buffer;
        void **write_ptr;
        void **read_ptr;
        int buf_size;
        int size_stored;

        std::mutex mutex;
};
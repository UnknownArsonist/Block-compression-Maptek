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
        StreamProcessor(int c_num_compressor_threads);
        StreamProcessor(int c_num_compressor_threads, int c_itoc_buf_size, int c_ctoo_buf_size);
        ~StreamProcessor();

        void setVerbose(bool c_v);
        void setup();
        void start();
        StreamBuffer *inputToCompressorBuffer;
        StreamBuffer *compressorToOutputBuffer;

    private:
        InputStreamReader *inputStreamReader;
        Compressor *compressor;
        DisplayOutput *displayOutput;
        std::chrono::time_point<std::chrono::high_resolution_clock> started;
        std::thread inputStreamReaderThread;
        std::thread **compressorThreads;
        std::thread displayOutputThread;
        bool verbose = false;
        int num_compressor_threads;

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
        virtual ~ProcessorModule(){};
};

class StreamProcessor::InputStreamReader : public StreamProcessor::ProcessorModule {
    public:
        // Constructor and Destructor
        InputStreamReader();
        InputStreamReader(FILE *in);
        ~InputStreamReader();

        // Function declarations
        void processStream();

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

        StreamProcessor *sp;

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
#ifdef WIN32
        void printSubBlock(HANDLE hStdout, SubBlock *sb);
        void printSubBlocks(HANDLE hStdout, ParentBlock *pb);
#endif
        void passBuffers(StreamBuffer *c_input_stream);
        void passValues(StreamProcessor *sp);
        void passValues(std::unordered_map<char, std::string> *c_tag_table);
        void setVerbose(bool c_v);
        
    private:
        StreamProcessor::StreamBuffer *input_stream;
        int *x_count;
        int *y_count;
        int *z_count;
        int *parent_x;
        int *parent_y;
        int *parent_z;
        std::unordered_map<char, std::string> *tag_table;
        bool verbose = false;
#ifdef WIN32
        HANDLE hStdout;
#endif
        char *buffer;
        int buf_size;
        int stored;
};

class StreamProcessor::StreamBuffer {
    public:
        StreamBuffer();
        StreamBuffer(int c_num_writers);
        ~StreamBuffer();
        int pop(void **buf);
        int push(void **buf);
        void setSize(int c_buf_size);
        void printBuffer();

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
        std::mutex write_value_mutex;
        std::condition_variable write_cond;
        std::condition_variable read_cond;
};
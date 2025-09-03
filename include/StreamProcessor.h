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

    private:
        InputStreamReader *inputStreamReader;
        Compressor *compressor;
        DisplayOutput *displayOutput;
        StreamBuffer *inputToCompressorBuffer;
        StreamBuffer *compressorToOutputBuffer;

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
        template <class Predicate>
        int pop_next(void **buf, Predicate pred) {
            std::unique_lock<std::mutex> lock(mutex);
            if (*read_ptr == nullptr && size_stored > 0) {
                *buf = nullptr;
                return -1;
            }
            void **t_ptr = (void**)malloc(size_stored * sizeof(void*));
            void **r_ptr = read_ptr;
            for (int i = 0; i < size_stored; i++) {
                if (pred(*r_ptr)) {
                    *buf = *r_ptr;
                    read_ptr++;
                    //fprintf(stderr, "[%d] %p %p %d %d  %d\n", i, read_ptr + i*sizeof(void*), &(buffer[buf_size]), (read_ptr + (i*sizeof(void*)) - &(buffer[buf_size])), (&(buffer[buf_size]) - read_ptr), rem);
                    //fprintf(stderr, "current_chunk %d pbc: %d, (%d, %d, %d)\n", current_chunk, (int)(((ParentBlock*)pb)->z / *parent_z), ((ParentBlock*)pb)->x, ((ParentBlock*)pb)->y, ((ParentBlock*)pb)->z);
                    int start = read_ptr - buffer;
                    int rem = (i * sizeof(void*));
                    //fprintf(stderr, "[%d] s: %d, e: %d, siz: %d\n", i, start, buf_size-1, i*sizeof(void*));
                    if (start + rem >= buf_size) {
                        int left = start + rem - buf_size;
                        memcpy(buffer, t_ptr + rem, left);
                    }
                    memcpy(read_ptr, t_ptr, rem);
                    free(t_ptr);
                    return 1;
                }
                t_ptr[i] = *r_ptr;
                r_ptr++;
                if (r_ptr >= &(buffer[buf_size])) {
                    r_ptr = buffer;
                }
            }
            free(t_ptr);
            return 0;
        };
        void peek_next(void **buf);
        int push(void **buf);
        void setCurrentChunk(int cc);
        int getCurrentChunk();
        int getStored();
        void setSize(int c_buf_size);
        void printBuffer();

    private:
        void **buffer;
        void **write_ptr;
        void **read_ptr;
        int buf_size;
        int size_stored;
        int num_writers;
        int closed_writers;
        int current_chunk;

        std::mutex mutex;
        std::mutex read_mutex;
        std::condition_variable write_cond;
        std::condition_variable read_cond;
};
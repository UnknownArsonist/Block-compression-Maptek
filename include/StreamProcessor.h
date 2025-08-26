#pragma once
#include "InputStreamReader.h"
#include "Compressor.h"
#include "StreamBuffer.h"
#include "DisplayOutput.h"
<<<<<<< HEAD

class StreamProcessor {
    private:
        InputStreamReader inputStreamReader;
        Compressor compressor;
        DisplayOutput displayOutput;
        StreamBuffer inputToCompressorBuffer;
        StreamBuffer compressorToOutputBuffer;

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

    public:
        StreamProcessor();
        ~StreamProcessor();

        void setVerbose(bool c_v);

        void setup();
        void start();
        void end();
        
        InputStreamReader *getInputStreamReader();
        Compressor *getCompressor();
        DisplayOutput *getDisplayOutput();
=======
#include <chrono>

class StreamProcessor
{
private:
    InputStreamReader inputStreamReader;
    Compressor compressor;
    DisplayOutput displayOutput;
    StreamBuffer inputToCompressorBuffer;
    StreamBuffer compressorToOutputBuffer;

    std::thread inputStreamReaderThread;
    std::thread compressorThread;
    std::thread displayOutputThread;
    std::chrono::time_point<std::chrono::system_clock> started;
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

public:
    StreamProcessor();
    ~StreamProcessor();

    void setVerbose(bool c_v);

    void setup();
    void start();
    void end();

    InputStreamReader *getInputStreamReader();
    Compressor *getCompressor();
    DisplayOutput *getDisplayOutput();
>>>>>>> 7a84fbe7280b4e0869f090e21fdebabaeccc7e4e
};
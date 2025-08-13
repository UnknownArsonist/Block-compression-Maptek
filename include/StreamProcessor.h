#pragma once
#include "InputStreamReader.h"
#include "Compressor.h"
#include "StreamBuffer.h"
#include "DisplayOutput.h"

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

        void setup();
        void start();
        
        InputStreamReader *getInputStreamReader();
        Compressor *getCompressor();
        DisplayOutput *getDisplayOutput();
};
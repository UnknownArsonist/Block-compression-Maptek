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

    public:
        StreamProcessor();
        ~StreamProcessor();

        void start();
};
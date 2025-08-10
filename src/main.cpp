#include "InputStreamReader.h"
#include "Compression.h"
#include "StreamBuffer.h"
#include "DisplayOutput.h"


/* NOTES:
    Could use fwrite from <cstdio> instead of printf for output to stdout (more efficient)
*/

int main()
{
    InputStreamReader processor;
    Compression compressor;
    processor.getHeaderLine();
    //processor.printHeader();

    std::thread inputStreamReaderThread(&InputStreamReader::processStream, &processor);
    std::thread compressorThread(&Compression::compressStream, &compressor);

    inputStreamReaderThread.join();
    compressorThread.join();
    return 0;
}

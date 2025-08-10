#include "StreamProcessor.h"
#include "Compression.h"
#include "StreamBuffer.h"
#include "DisplayOutput.h"


/* NOTES:
    Could use fwrite from <cstdio> instead of printf for output to stdout (more efficient)
*/

int main()
{
    StreamProcessor processor;
    Compression compressor;
    StreamBuffer test_streambuffer(8);
    char test_buffer[4] = {'1', '2', '3', '4'};
    char test_buffer2[5] = {'5', '6', '7', '8', '9'};
    char test_buffer3[5] = {'a', 'b', 'c', 'd', 'e'};

    int val = test_streambuffer.write(test_buffer2, 5);
    printf("%d\n", val);
    test_streambuffer.printBuffer();
    val = test_streambuffer.read(test_buffer3, 5);
    printf("%d\n", val);
    test_streambuffer.printBuffer();
    val = test_streambuffer.write(test_buffer, 4);
    printf("%d\n", val);
    test_streambuffer.printBuffer();

    printf("[");
    for (int i = 0; i < 5; i++) {
        printf("%c", test_buffer3[i]);
        if (i != 5 - 1)
            printf(", ");
    }
    printf("]\n");
    
    processor.getHeaderLine();
    //processor.printHeader();

    std::thread streamProcessorThread(&StreamProcessor::processStream, &processor);
    std::thread compressorThread(&Compression::compressStream, &compressor);

    streamProcessorThread.join();
    compressorThread.join();
    return 0;
}

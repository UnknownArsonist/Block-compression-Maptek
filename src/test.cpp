#include "StreamProcessor.h"
#include <chrono>
#include <stdio.h>
#include <iostream>
#include <thread>

int main() {
    StreamProcessor::InputStreamReader isr;
    int x_count, y_count, z_count, parent_x, parent_y, parent_z;
    std::unordered_map<char, std::string> tag_table;
    isr.passValues(&x_count, &y_count, &z_count, &parent_x, &parent_y, &parent_z, &tag_table);
    isr.passBuffers(NULL);
    isr.getHeader();

    //std::this_thread::sleep_for(std::chrono::seconds(1));
    std::chrono::time_point<std::chrono::high_resolution_clock> started, end;
    started = std::chrono::high_resolution_clock::now();
    std::thread isrThread(&StreamProcessor::InputStreamReader::processStream_test, &isr, "char");
    isrThread.join();
    end = std::chrono::high_resolution_clock::now();
    std::cout << "InputStreamReader Runtime:\n  " << std::chrono::duration_cast<std::chrono::milliseconds>(end - started).count() << std::endl;
    return 0;
}
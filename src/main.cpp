#include <iostream>
#include <iomanip>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdint.h>
#include <string.h>
#include <chrono>


#include "../lib/logger/logger.h"

#define SERIAL_PORT "/dev/serial0"

int main()
{
    unsigned int baud = 115200;
    std::string filename = getNextLogFilename("logs");

    SerialLogger logger(SERIAL_PORT, baud, filename);

    

    while (true)
    {
        auto start = std::chrono::high_resolution_clock::now();
        logger.poll();
        auto end = std::chrono::high_resolution_clock::now();
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::cout << "logger.poll() took " << us << " us (" << std::fixed << std::setprecision(3)
                  << (us / 1000.0) << " ms)\n";
    }

    return 0;
}

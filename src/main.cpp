#include <iostream>
#include <iomanip>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdint.h>
#include <string.h>

#define SERIAL_PORT "/dev/serial0"

int main() {
    try {
        unsigned int baud = 115200;
        std::string filename = "log_data.csv";

        SerialLogger logger(SERIAL_PORT, baud, filename);
        logger.run();
    }
    catch (const std::exception& ex) 
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}

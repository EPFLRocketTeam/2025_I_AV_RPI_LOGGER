#include <iostream>
#include <iomanip>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdint.h>
#include <string.h>

#include "../lib/logger/logger.h"

#define SERIAL_PORT "/dev/serial0"

int main()
{
    unsigned int baud = 115200;
    std::string filename = "log_data.csv";

    SerialLogger logger(SERIAL_PORT, baud, filename);

    while (true)
    {
        logger.poll();
    }

    return 0;
}

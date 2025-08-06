#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <ctime>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include "capsule.h"

std::ofstream csv;

std::string getDateStr() {
    time_t t = time(nullptr);
    struct tm tm;
    localtime_r(&t, &tm);

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << tm.tm_mday << "_"
        << std::setw(2) << std::setfill('0') << tm.tm_mon + 1 << "_"
        << tm.tm_year + 1900;
    return oss.str();
}

std::string getTimestamp() {
    auto t = std::time(nullptr);
    struct tm tm;
    localtime_r(&t, &tm);

    char buffer[20];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", &tm);

    return std::string(buffer);
}

void handlePacket(uint8_t packetId, uint8_t *data, uint32_t len) {
    if (!csv.is_open()) {
        int counter = 1;
        std::string base = getDateStr() + "_Flight_Data_";
        std::string filename;
        while (true) {
            std::ostringstream oss;
            oss << base << std::setw(2) << std::setfill('0') << counter << ".csv";
            filename = oss.str();
            std::ifstream f(filename);
            if (!f.good()) break;
            counter++;
        }
        csv.open(filename);
        std::cout << "Logging to: " << filename << std::endl;

        // Write CSV header
        csv << "timestamp_rpi,packet_id";
        for (uint32_t i = 0; i < len; i++) {
            csv << ",byte" << i;
        }
        csv << "\n";
    }

    csv << getTimestamp() << "," << (int)packetId;
    for (uint32_t i = 0; i < len; i++) {
        csv << "," << (int)data[i];
    }
    csv << "\n";
    csv.flush(); // flush after each line to save data immediately
}

int main(int argc, char* argv[]) {
    const char* port = "/dev/serial0"; // default UART port
    int baudrate = B115200;            // default baudrate

    if (argc >= 2) {
        port = argv[1];
    }

    if (argc >= 3) {
        int b = atoi(argv[2]);
        switch (b) {
            case 9600: baudrate = B9600; break;
            case 19200: baudrate = B19200; break;
            case 38400: baudrate = B38400; break;
            case 57600: baudrate = B57600; break;
            case 115200: baudrate = B115200; break;
            default:
                std::cerr << "Unsupported baudrate, using 115200\n";
                baudrate = B115200;
        }
    }

    int fd = open(port, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    termios tty{};
    tcgetattr(fd, &tty);
    cfsetispeed(&tty, baudrate);
    cfsetospeed(&tty, baudrate);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_iflag = 0;
    tcsetattr(fd, TCSANOW, &tty);

    CapsuleStatic capsule(handlePacket);

    uint8_t buf[1];
    while (true) {
        ssize_t n = read(fd, buf, 1);
        if (n > 0) {
            capsule.decode(buf[0]);
        } else if (n < 0) {
            perror("read");
            break; // exit on read error
        }
        // else n==0 means no data; just continue reading
    }

    if (csv.is_open()) {
        csv.close();
    }
    close(fd);
    return 0;
}

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <ctime>
#include <csignal>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include "capsule.h"  // same as your Teensy version, but without Arduino dependencies

volatile bool keepRunning = true;

void signalHandler(int signum) {
    keepRunning = false;
}

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
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&t), "%H:%M:%S")
        << "." << std::setw(3) << std::setfill('0') << ms.count();
    return oss.str();
}

// Packet callback
void handlePacket(uint8_t packetId, uint8_t *data, uint32_t len) {
    static std::ofstream csv;

    if (!csv.is_open()) {
        // Generate unique file name
        int counter = 1;
        std::string base = getDateStr() + "_Flight_Data_";
        std::string filename;
        do {
            std::ostringstream oss;
            oss << base << std::setw(2) << std::setfill('0') << counter << ".csv";
            filename = oss.str();
            std::ifstream f(filename);
            if (!f.good()) break;
            counter++;
        } while (true);

        csv.open(filename);
        std::cout << "Logging to: " << filename << std::endl;
        csv << "timestamp_rpi,packet_id";
        for (uint32_t i = 0; i < len; i++) csv << ",byte" << i;
        csv << "\n";
    }

    csv << getTimestamp() << "," << (int)packetId;
    for (uint32_t i = 0; i < len; i++) csv << "," << (int)data[i];
    csv << "\n";
    csv.flush();
}

int main() {
    signal(SIGINT, signalHandler);

    // Open UART
    int fd = open("/dev/serial0", O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    termios tty{};
    tcgetattr(fd, &tty);
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);
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
    while (keepRunning) {
        if (read(fd, buf, 1) > 0) {
            capsule.decode(buf[0]);
        }
    }

    close(fd);
    return 0;
}

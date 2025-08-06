#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <ctime>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>  // for memcpy
#include "capsule.h"

// Global CSV file stream
std::ofstream csv;

// Helper to get date string for file naming
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

// Helper to get timestamp string for each packet row
std::string getTimestamp() {
    auto t = std::time(nullptr);
    struct tm tm;
    localtime_r(&t, &tm);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", &tm);
    return std::string(buffer);
}

// Decode uint16_t from two bytes (little-endian)
uint16_t toUint16(const uint8_t* bytes) {
    return bytes[0] | (bytes[1] << 8);
}

// This is your known struct: 5 uint16_t fields (10 bytes payload)
void handlePacket(uint8_t packetId, uint8_t *data, uint32_t len) {
    if (len < 10) {
        std::cerr << "Warning: packet payload too short (" << len << " bytes), skipping\n";
        return;
    }

    // Open CSV file on first call
    if (!csv.is_open()) {
        std::string filename = getDateStr() + "_Flight_Data_01.csv";
        csv.open(filename);
        if (!csv.is_open()) {
            std::cerr << "Failed to open CSV file for writing\n";
            return;
        }
        // Write CSV header
        csv << "timestamp_rpi,packet_id,pressure_ETH,pressure_N2O,chamber_pressure,temp_N2O,hv_voltage\n";
        std::cout << "Logging to: " << filename << std::endl;
    }

    // Decode fields
    uint16_t pressure_ETH      = toUint16(data + 0);
    uint16_t pressure_N2O      = toUint16(data + 2);
    uint16_t chamber_pressure  = toUint16(data + 4);
    uint16_t temp_N2O          = toUint16(data + 6);
    uint16_t hv_voltage        = toUint16(data + 8);

    // Write decoded data row to CSV
    csv << getTimestamp() << "," << (int)packetId << ","
        << pressure_ETH << "," << pressure_N2O << "," << chamber_pressure << ","
        << temp_N2O << "," << hv_voltage << "\n";
    csv.flush();
}

int main(int argc, char* argv[]) {
    const char* port = "/dev/serial0";   // default UART port
    int baudrate = B115200;               // default baudrate

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
                std::cerr << "Unsupported baudrate, using 115200" << std::endl;
                baudrate = B115200;
        }
    }

    // Open serial port
    int fd = open(port, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror("Failed to open port");
        return 1;
    }

    // Configure serial port
    termios tty{};
    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        close(fd);
        return 1;
    }
    cfsetispeed(&tty, baudrate);
    cfsetospeed(&tty, baudrate);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;       // 8 data bits
    tty.c_cflag &= ~PARENB;   // no parity
    tty.c_cflag &= ~CSTOPB;   // 1 stop bit
    tty.c_cflag &= ~CRTSCTS;  // no flow control
    tty.c_lflag = 0;          // raw input
    tty.c_oflag = 0;          // raw output
    tty.c_iflag = 0;          // raw input
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        close(fd);
        return 1;
    }

    // Create Capsule instance with the callback
    CapsuleStatic capsule(handlePacket);

    uint8_t buf[1];
    std::cout << "Listening on " << port << " at baudrate " << baudrate << std::endl;

    // Main read loop (runs forever)
    while (true) {
        ssize_t n = read(fd, buf, 1);
        if (n > 0) {
            capsule.decode(buf[0]);
        } else if (n < 0) {
            perror("Read error");
            break;
        }
        // If n==0, no data, continue loop
    }

    if (csv.is_open()) {
        csv.close();
    }
    close(fd);
    return 0;
}

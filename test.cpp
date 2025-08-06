#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <ctime>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <fstream>
#include "capsule.h"

// Global CSV file stream
std::ofstream csv;

// Generate date string: DD_MM_YYYY
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

// Generate a unique filename by incrementing suffix until free
std::string getUniqueFilename(const std::string& baseName) {
    int counter = 1;
    std::string filename;
    while (true) {
        std::ostringstream oss;
        oss << baseName << "_" << std::setw(2) << std::setfill('0') << counter << ".csv";
        filename = oss.str();
        std::ifstream file(filename);
        if (!file.good()) break; // file doesn't exist, use this
        counter++;
    }
    return filename;
}

// Get current time HH:MM:SS
std::string getTimestamp() {
    time_t t = time(nullptr);
    struct tm tm;
    localtime_r(&t, &tm);
    char buf[20];
    strftime(buf, sizeof(buf), "%H:%M:%S", &tm);
    return std::string(buf);
}

// Decode little-endian uint16_t from two bytes
uint16_t toUint16(const uint8_t* bytes) {
    return bytes[0] | (bytes[1] << 8);
}

// Convert Q8.8 fixed point to float
float q8_8ToFloat(uint16_t val) {
    return static_cast<float>(static_cast<int16_t>(val)) / 256.0f;
}

// Capsule packet handler: decode payload and log CSV line
void handlePacket(uint8_t packetId, uint8_t* data, uint32_t len) {
    if (len < 10) {
        std::cerr << "Warning: packet too short (" << len << " bytes), skipping\n";
        return;
    }

    // Open CSV file once, with unique filename
    if (!csv.is_open()) {
        std::string base = getDateStr() + "_Flight_Data";
        std::string filename = getUniqueFilename(base);
        csv.open(filename);
        if (!csv.is_open()) {
            std::cerr << "Failed to open CSV file for writing\n";
            return;
        }
        // Write CSV header
        csv << "timestamp_rpi,packet_id,pressure_ETH,pressure_N2O,chamber_pressure,temp_N2O,hv_voltage\n";
        std::cout << "Logging to: " << filename << std::endl;
    }

    // Decode fields from payload
    uint16_t pressure_ETH_raw      = toUint16(data + 0);
    uint16_t pressure_N2O_raw      = toUint16(data + 2);
    uint16_t chamber_pressure_raw  = toUint16(data + 4);
    uint16_t temp_N2O_raw          = toUint16(data + 6);
    uint16_t hv_voltage_raw        = toUint16(data + 8);

    // Convert from Q8.8 fixed-point to floats
    float pressure_ETH     = q8_8ToFloat(pressure_ETH_raw);
    float pressure_N2O     = q8_8ToFloat(pressure_N2O_raw);
    float chamber_pressure = q8_8ToFloat(chamber_pressure_raw);
    float temp_N2O         = q8_8ToFloat(temp_N2O_raw);
    float hv_voltage       = q8_8ToFloat(hv_voltage_raw);

    // Write a CSV line
    csv << getTimestamp() << "," << (int)packetId << ","
        << pressure_ETH << "," << pressure_N2O << "," << chamber_pressure << ","
        << temp_N2O << "," << hv_voltage << "\n";

    csv.flush();
}

int main(int argc, char* argv[]) {
    const char* port = "/dev/serial0";
    int baudrate = B115200;

    // Parse CLI args for port and baudrate
    if (argc >= 2) port = argv[1];
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

    // Open serial port
    int fd = open(port, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror("Failed to open serial port");
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

    // Create Capsule instance with callback
    CapsuleStatic capsule(handlePacket);

    std::cout << "Listening on " << port << " at baud " << baudrate << std::endl;

    uint8_t buf[1];
    while (true) {
        ssize_t n = read(fd, buf, 1);
        if (n > 0) {
            capsule.decode(buf[0]);
        } else if (n < 0) {
            perror("Read error");
            break;
        }
    }

    if (csv.is_open()) csv.close();
    close(fd);
    return 0;
}

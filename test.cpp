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

// ===== Globals =====
std::ofstream csv;
bool logging = false;

// ===== Helpers =====
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

std::string getUniqueFilename(const std::string& baseName) {
    int counter = 1;
    std::string filename;
    while (true) {
        std::ostringstream oss;
        oss << baseName << "_" << std::setw(2) << std::setfill('0') << counter << ".csv";
        filename = oss.str();
        std::ifstream f(filename);
        if (!f.good()) break;
        counter++;
    }
    return filename;
}

std::string getTimestamp() {
    time_t t = time(nullptr);
    struct tm tm;
    localtime_r(&t, &tm);
    char buf[20];
    strftime(buf, sizeof(buf), "%H:%M:%S", &tm);
    return std::string(buf);
}

// little-endian 16-bit from an arbitrary byte offset
inline uint16_t u16le(const uint8_t* p) { return static_cast<uint16_t>(p[0] | (p[1] << 8)); }

// Q8.8 to float (signed semantics like Teensy)
inline float q8_8ToFloat(uint16_t v) {
    return static_cast<float>(static_cast<int16_t>(v)) / 256.0f;
}

// ===== Capsule callback =====
// Expect payload layout (packed):
// [0]   : launch (uint8_t)
// [1..2]: pressure_ETH (u16, Q8.8 LE)
// [3..4]: pressure_N2O
// [5..6]: chamber_pressure
// [7..8]: temp_N2O
// [9..10]: hv_voltage
void handlePacket(uint8_t packetId, uint8_t* data, uint32_t len) {
    // Basic guard: need at least launch + 5*uint16 = 11 bytes
    if (len < 11) {
        std::cerr << "Warning: packet too short (" << len << " bytes), waiting...\n";
        return;
    }

    // 1) Check launch flag in first byte
    const bool launch = (data[0] == 1);   // teensy sets 0 or 1
    if (!logging) {
        if (!launch) {
            // Optional: uncomment if you want periodic feedback
            // static uint32_t lastPrint = 0;
            // uint32_t now = static_cast<uint32_t>(time(nullptr));
            // if (now != lastPrint) { lastPrint = now; std::cout << "Waiting for launch...\n"; }
            return; // do nothing until launch
        }

        // Create CSV on first launch==true
        const std::string base = getDateStr() + "_Flight_Data";
        const std::string filename = getUniqueFilename(base);
        csv.open(filename);
        if (!csv.is_open()) {
            std::cerr << "Failed to open CSV for writing\n";
            return;
        }
        std::cout << "🚀 Launch detected, logging to: " << filename << std::endl;

        // Header with named, human-readable fields
        csv << "timestamp_rpi,packet_id,pressure_ETH,pressure_N2O,chamber_pressure,temp_N2O,hv_voltage\n";
        csv.flush();

        logging = true;
        // Note: we intentionally continue to decode & log this same packet below.
    }

    // 2) Decode Q8.8 fields from known offsets
    float pressure_ETH     = q8_8ToFloat(u16le(data + 1));
    float pressure_N2O     = q8_8ToFloat(u16le(data + 3));
    float chamber_pressure = q8_8ToFloat(u16le(data + 5));
    float temp_N2O         = q8_8ToFloat(u16le(data + 7));
    float hv_voltage       = q8_8ToFloat(u16le(data + 9));

    // 3) Write CSV line
    if (csv.is_open()) {
        csv << getTimestamp() << "," << static_cast<int>(packetId) << ","
            << pressure_ETH << "," << pressure_N2O << "," << chamber_pressure << ","
            << temp_N2O << "," << hv_voltage << "\n";
        csv.flush();
    }
}

int main(int argc, char* argv[]) {
    const char* port = "/dev/serial0";
    int baudrate = B115200;

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

    // Open serial
    //int fd = open(port, O_RDWR | O_NOCTTY);
    //if (fd < 0) { perror("Failed to open serial port"); return 1; }
    int fd = -1;
    while (fd < 0) {
    fd = open(port, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror("Failed to open port, retrying...");
        sleep(1);
        }
    }

    termios tty{};
    if (tcgetattr(fd, &tty) != 0) { perror("tcgetattr"); close(fd); return 1; }

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

    if (tcsetattr(fd, TCSANOW, &tty) != 0) { perror("tcsetattr"); close(fd); return 1; }

    // Feed bytes to Capsule
    CapsuleStatic capsule(handlePacket);
    std::cout << "Listening on " << port << " at 115200\n";

    uint8_t b;
    while (true) {
        ssize_t n = read(fd, &b, 1);
        if (n > 0)       capsule.decode(b);
        else if (n < 0) { perror("Read error"); break; }
    }

    if (csv.is_open()) csv.close();
    close(fd);
    return 0;
}

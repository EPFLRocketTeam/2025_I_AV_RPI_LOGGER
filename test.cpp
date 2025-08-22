#include <iostream>
#include <iomanip>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdint.h>
#include <string.h>

// ---------------- Capsule Decoder ----------------
class Capsule {
public:
    using Handler = void(*)(uint8_t, uint8_t*, uint32_t);

    Capsule(Handler h) : handler(h), state(0), len(0), packetId(0) {}

    void decode(uint8_t b) {
        switch (state) {
            case 0: // packet ID
                packetId = b;
                state = 1;
                len = 0;
                break;
            case 1: // payload length
                len = b;
                if (len > sizeof(buffer)) {
                    len = 0;
                    state = 0;
                } else {
                    pos = 0;
                    state = (len == 0) ? 3 : 2;
                }
                break;
            case 2: // payload bytes
                buffer[pos++] = b;
                if (pos >= len) state = 3;
                break;
            case 3: // terminator
                if (b == 0x00 && handler) {
                    handler(packetId, buffer, len);
                }
                state = 0;
                break;
        }
    }

private:
    Handler handler;
    uint8_t state;
    uint8_t packetId;
    uint8_t buffer[256];
    uint32_t len;
    uint32_t pos;
};

// ---------------- Prop Board Packet ----------------
#pragma pack(push, 1)
struct prop_board_downlink_packet {
    uint8_t  cmd_launch;
    uint16_t gimbal_x;
    uint16_t gimbal_y;
    uint16_t main_ETH;
    uint16_t main_N2O;
    uint8_t  vent_ETH;
    uint8_t  vent_N2O;
    uint8_t  solenoid_N2;
};
#pragma pack(pop)

inline float q8_8ToFloat(uint16_t v) {
    return static_cast<float>(static_cast<int16_t>(v)) / 256.0f;
}

// ---------------- Handler ----------------
void handlePacket(uint8_t packetId, uint8_t* data, uint32_t len) {
    std::cout << "\n[Packet] ID=" << (int)packetId << " len=" << len << std::endl;

    if (len != sizeof(prop_board_downlink_packet)) {
        std::cerr << "Warning: unexpected payload size (" << len
                  << ", expected " << sizeof(prop_board_downlink_packet) << ")\n";
        return;
    }

    auto* pkt = reinterpret_cast<prop_board_downlink_packet*>(data);

    std::cout << std::fixed << std::setprecision(2)
              << "  cmd_launch=" << (int)pkt->cmd_launch
              << "  gimbal_x=" << q8_8ToFloat(pkt->gimbal_x)
              << "  gimbal_y=" << q8_8ToFloat(pkt->gimbal_y)
              << "  main_ETH=" << q8_8ToFloat(pkt->main_ETH)
              << "  main_N2O=" << q8_8ToFloat(pkt->main_N2O)
              << "  vent_ETH=" << (int)pkt->vent_ETH
              << "  vent_N2O=" << (int)pkt->vent_N2O
              << "  solenoid_N2=" << (int)pkt->solenoid_N2
              << std::endl;
}

// ---------------- Serial Setup ----------------
int openSerial(const char* device, int baud) {
    int fd = open(device, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    termios tty{};
    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        close(fd);
        return -1;
    }

    cfsetospeed(&tty, baud);
    cfsetispeed(&tty, baud);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // no flow control
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        close(fd);
        return -1;
    }

    return fd;
}

// ---------------- Main ----------------
#define SERIAL_PORT "/dev/serial0"

int main() {
    int fd = openSerial(SERIAL_PORT, B115200);
    if (fd < 0) return 1;

    std::cout << "Listening on " << SERIAL_PORT << " at 115200 baud...\n";

    Capsule capsule(handlePacket);

    uint8_t buf[256];
    while (true) {
        int n = read(fd, buf, sizeof(buf));
        if (n > 0) {
            for (int i = 0; i < n; i++) {
                capsule.decode(buf[i]);
            }
        } else if (n < 0) {
            perror("read");
            break;
        }
    }

    close(fd);
    return 0;
}

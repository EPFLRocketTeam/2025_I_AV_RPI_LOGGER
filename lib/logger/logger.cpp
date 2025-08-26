#include "logger.h"

SerialLogger::SerialLogger(const std::string& port, unsigned int baud, const std::string& csvFile)
    : serial(io, port)
{
    log_packet = new log_packet_t;
    SerialCapsule = Capsule(&SerialLogger::handleSerialCapsule, this);

    serial.set_option(boost::asio::serial_port_base::baud_rate(baud));

    csv.open(csvFile);

    if (!csv.is_open()) {
        throw std::runtime_error("Failed to open CSV file");
    }

    // Write header once
    csv << csvHeader() << std::endl;
}

void SerialLogger::run() 
{
    std::cout << "Listening on serial port..." << std::endl;

    while (true) 
    {
        uint8_t len = SerialCapsule.getCodedLen(sizeof(log_packet_t));
        uint8_t buffer[len];

        boost::asio::read(serial, boost::asio::buffer(buffer, len));
        for (int i = 0; i < len; i++)
            SerialCapsule.decode(buffer[i]);

        csv << packetToCSV(*log_packet) << std::endl;
        csv.flush();

        std::cout << "Logged packet" << std::endl;
    }
}

void SerialLogger::handleSerialCapsule(uint8_t packetId, uint8_t *dataIn, uint32_t len)
{
    if (len == sizeof(log_packet_t))
        memcpy(log_packet, dataIn, sizeof(log_packet_t));
}

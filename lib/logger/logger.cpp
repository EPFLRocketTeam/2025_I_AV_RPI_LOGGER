#include "logger.h"

SerialLogger::SerialLogger(const std::string& port, unsigned int baud, const std::string& csvFile)
    : serial(io, port), SerialCapsule(&SerialLogger::handleSerialCapsule, this)
{
    log_packet = new log_packet_t;

    serial.set_option(boost::asio::serial_port_base::baud_rate(baud));

    csv.open(csvFile);

    if (!csv.is_open())
        throw std::runtime_error("Failed to open CSV file");

    // Write header once
    csv << csvHeader() << std::endl;
}

void SerialLogger::run() 
{
    std::cout << "Listening on serial port..." << std::endl;

    uint8_t len = SerialCapsule.getCodedLen(log_packet_size);
    uint8_t buffer[len];

    boost::asio::read(serial, boost::asio::buffer(buffer, len));
    for (int i = 0; i < len; i++)
        SerialCapsule.decode(buffer[i]);

    csv << packetToCSV(*log_packet) << std::endl;
    csv.flush();

    std::cout << "Logged packet" << std::endl;
}

void SerialLogger::poll()
{
    boost::asio::serial_port::native_handle_type fd = serial.native_handle();
    int available = 0;

    ioctl(fd, FIONREAD, &available);

    if (available >= SerialCapsule.getCodedLen(log_packet_size)) 
    {
        if (available > (int)readBuffer.size())
            available = readBuffer.size();

        size_t bytesRead = serial.read_some(boost::asio::buffer(readBuffer, available));

        for (size_t i = 0; i < bytesRead; i++)
            SerialCapsule.decode(readBuffer[i]);
    }
}

void SerialLogger::handleSerialCapsule(uint8_t packetId, uint8_t *dataIn, uint32_t len)
{
    if (len == log_packet_size)
        memcpy(log_packet, dataIn, log_packet_size);

    csv << packetToCSV(*log_packet) << std::endl;
    csv.flush();
    std::cout << "Logged packet.. " << fixed16_to_float(log_packet->temp_N2O) << std::endl;
}

#include <iostream>
#include "logger.h"

SerialLogger::SerialLogger(const std::string& port, unsigned int baud, const std::string& csvFile)
    : serial(io, port)
{
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
        log_packet pkt;
        boost::asio::read(serial, boost::asio::buffer(&pkt, sizeof(pkt)));

        csv << packetToCSV(pkt) << std::endl;
        csv.flush();

        std::cout << "Logged packet" << std::endl;
    }
}

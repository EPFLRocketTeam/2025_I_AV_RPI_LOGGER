#ifndef LOGGER_H
#define LOGGER_H

#pragma once
#include <boost/asio.hpp>
#include <fstream>
#include <string>
#include "../../packet_definition.h"

class SerialLogger {
public:
    SerialLogger(const std::string& port, unsigned int baud, const std::string& csvFile);
    void run();

private:
    boost::asio::io_service io;
    boost::asio::serial_port serial;
    std::ofstream csv;
};


#endif
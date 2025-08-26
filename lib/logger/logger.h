#ifndef LOGGER_H
#define LOGGER_H

#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include "../../include/packet_definition.h"
#include "../2024_C_AV-GS_CAPSULE/src/capsule.h"

class SerialLogger {
public:
    SerialLogger(const std::string& port, unsigned int baud, const std::string& csvFile);
    void run();

private:
    boost::asio::io_service io;
    boost::asio::serial_port serial;
    std::ofstream csv;

    log_packet_t* log_packet;

    CapsuleStatic* SerialCapsule;

    void handleSerialCapsule(uint8_t packetId, uint8_t *dataIn, uint32_t len);
};


#endif
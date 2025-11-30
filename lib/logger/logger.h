#ifndef LOGGER_H
#define LOGGER_H

#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include "../../include/packet_definition.h"
#include "../2024_C_AV-GS_CAPSULE/src/capsule.h"
#include <filesystem>
#include <regex>
#include "../2025_I_AV_OBJECT_DICTIONARY/object_dictionary.h"

std::string csvHeader();
std::string getNextLogFilename(const std::string& directory);

class SerialLogger {
public:
    SerialLogger(const std::string& port, unsigned int baud, const std::string& csvFile);
    void handleSerialCapsule(uint8_t packetId, uint8_t *dataIn, uint32_t len);
    void poll();
    void run();

private:
    boost::asio::io_service io;
    boost::asio::serial_port serial;
    std::string _csvFile;
    std::ifstream csv;


    
    std::chrono::steady_clock::time_point startTime;
    ObjectDictionary* log_objDict;

    Capsule<SerialLogger> SerialCapsule;

    enum { BUFFER_SIZE = 256 };
    std::array<uint8_t, BUFFER_SIZE> readBuffer;
};


#endif
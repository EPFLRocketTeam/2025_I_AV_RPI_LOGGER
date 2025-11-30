#include "logger.h"


std::string csvHeader()
{
    return "timestamp,"
            + objectDictionaryCSVHeader();
}

std::string getNextLogFilename(const std::string& directory)
{
    // Créer le dossier s'il n'existe pas
    if (!std::filesystem::exists(directory))
        std::filesystem::create_directories(directory);

    std::regex pattern(R"(log_(\d+)\.csv)");
    int maxIndex = 0;

    for (const auto& entry : std::filesystem::directory_iterator(directory))
    {
        if (!entry.is_regular_file())
            continue;

        const std::string name = entry.path().filename().string();
        std::smatch match;

        if (std::regex_match(name, match, pattern))
        {
            int value = std::stoi(match[1]);
            if (value > maxIndex)
                maxIndex = value;
        }
    }

    // Nom du fichier : log_XX.csv
    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), "log_%02d.csv", maxIndex + 1);

    // Retourne "logs/log_XX.csv"
    return directory + "/" + buffer;
}

SerialLogger::SerialLogger(const std::string& port, unsigned int baud, const std::string& csvFile)
    : serial(io, port), SerialCapsule(&SerialLogger::handleSerialCapsule, this)
{
    startTime = std::chrono::steady_clock::now();

    log_objDict = new ObjectDictionary;

    serial.set_option(boost::asio::serial_port_base::baud_rate(baud));

    _csvFile = csvFile;
    csv.open(csvFile);

    //std::ofstream out(_csvFile, std::ios::app);

    if (!csv.is_open())
    {
        throw std::runtime_error("Failed to open CSV file. Did you do the Deamon ? ");
    }
    else
    {
        std::cout << "Start logging to " << csvFile << std::endl;
    }

    // Write header once
    csv << csvHeader() << std::endl;
}

// void SerialLogger::run() // not used
// {
//     std::cout << "Listening on serial port..." << std::endl;
//
//     uint8_t len = SerialCapsule.getCodedLen(object_dictionary_size);
//     uint8_t buffer[len];
//
//     boost::asio::read(serial, boost::asio::buffer(buffer, len));
//     for (int i = 0; i < len; i++)
//         SerialCapsule.decode(buffer[i]);
//
//     csv << packetToCSV(*log_packet) << std::endl;
//     csv.flush();
//
//     std::cout << "Logged packet" << std::endl;
// }

void SerialLogger::poll()
{
    boost::asio::serial_port::native_handle_type fd = serial.native_handle();
    int available = 0;

    ioctl(fd, FIONREAD, &available);

    if (available >= SerialCapsule.getCodedLen(object_dictionary_size)) 
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

    if (len == object_dictionary_size)
        memcpy(log_objDict, dataIn, object_dictionary_size);

    // timestamp en ms depuis l'époque
    auto now = std::chrono::steady_clock::now() - startTime;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();

    //std::ofstream out(_csvFile, std::ios::app);
    if (csv.is_open())
    {
        
        csv << ms << "," << objectDictionaryCSV(*log_objDict) << std::endl;
        std::cout << "[ " << ms << " ms] Logged packet.. (PN: " << fixed16_to_float(log_objDict->sol_N2) << ")" << std::endl;   
    }

}

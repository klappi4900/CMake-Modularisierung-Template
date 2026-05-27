#include "Logger/Logger.h"

#include <string>

class IFileWriter;
Logger::Logger(IFileWriter& writer) : writer_(writer) {}

void Logger::log(const std::string& msg) {
    writer_.write("[LOG] " + msg);
}

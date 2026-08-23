#include "Logger.h"

#include <string>

class I_FileWriter;
Logger::Logger(I_FileWriter& writer) : writer_(writer) {}

void Logger::log(const std::string& msg) {
    writer_.write("[LOG] " + msg);
}

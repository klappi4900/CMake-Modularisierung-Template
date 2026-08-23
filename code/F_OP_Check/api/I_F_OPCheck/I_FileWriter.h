#pragma once
#include <string>

struct I_FileWriter {
    virtual ~I_FileWriter() = default;
    virtual void write(const std::string& data) = 0;
};

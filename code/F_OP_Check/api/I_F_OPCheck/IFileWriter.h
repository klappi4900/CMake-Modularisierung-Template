#pragma once
#include <string>

struct IFileWriter {
    virtual ~IFileWriter() = default;
    virtual void write(const std::string& data) = 0;
};

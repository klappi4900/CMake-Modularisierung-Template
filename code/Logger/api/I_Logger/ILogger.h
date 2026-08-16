#pragma once
#include <string>

struct ILogger {
    virtual ~ILogger() = default;
    virtual void log(const std::string& msg) = 0;
};

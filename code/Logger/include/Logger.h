#pragma once
#include <string>

#include "I_API_Logger/ILogger.h"
#include "I_F_OPCheck/IFileWriter.h"

class IFileWriter;

class Logger : public ILogger {
public:
    explicit Logger(IFileWriter& writer);
    void log(const std::string& msg) override;

private:
    IFileWriter& writer_;
};

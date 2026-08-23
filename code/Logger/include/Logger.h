#pragma once
#include <string>

#include "I_Logger/ILogger.h"
#include "I_F_OPCheck/I_FileWriter.h"

class I_FileWriter;

class Logger : public ILogger {
public:
    explicit Logger(I_FileWriter& writer);
    void log(const std::string& msg) override;

private:
    I_FileWriter& writer_;
};

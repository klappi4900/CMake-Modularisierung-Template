#pragma once

#include <memory>
#include <vector>

#include "I_F_OPCheck/IFileWriter.h"
#include "I_F_OPCheck/IWriteCheck.h"


class FileWriter : public IFileWriter {
public:
    explicit FileWriter(std::vector<std::unique_ptr<IWriteCheck>> checks);
    void write(const std::string& data) override;

private:
    std::vector<std::unique_ptr<IWriteCheck>> checks_;
};

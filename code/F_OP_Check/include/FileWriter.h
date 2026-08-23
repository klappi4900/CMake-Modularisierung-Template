#pragma once

#include <memory>
#include <vector>

#include "I_F_OPCheck/I_FileWriter.h"
#include "I_F_OPCheck/I_WriteCheck.h"


class FileWriter : public I_FileWriter {
public:
    explicit FileWriter(std::vector<std::unique_ptr<I_WriteCheck>> checks);
    void write(const std::string& data) override;

private:
    std::vector<std::unique_ptr<I_WriteCheck>> checks_;
};

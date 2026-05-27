#pragma once
#include <string>

struct IWriteCheck {
    virtual ~IWriteCheck() = default;
    virtual bool check() const = 0;
    virtual std::string name() const = 0;
};

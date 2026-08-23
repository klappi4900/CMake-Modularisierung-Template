#pragma once
#include <string>

struct I_WriteCheck {
    virtual ~I_WriteCheck() = default;
    virtual bool check() const = 0;
    virtual std::string name() const = 0;
};

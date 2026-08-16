//
// Created by Rookies Garage on 27.05.2026.
//
#pragma once
#include <string>

#ifndef NOMINMAX
#define NOMINMAX          // sonst kollidieren die windows.h-Makros min/max mit nlohmann/json
#endif
#include <windows.h>

namespace Setup {
    void SetupKonsoleToGerman();
    void WriteLog(const std::string& nachricht);

    enum class FileORFolder{
        File,
        Folder,
    };

    void readInputArguments();
};

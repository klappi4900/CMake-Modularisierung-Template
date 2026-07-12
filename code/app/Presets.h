//
// Created by Rookies Garage on 27.05.2026.
//
#pragma once
#include <string>

#ifndef NOMINMAX
#define NOMINMAX          // sonst kollidieren die windows.h-Makros min/max mit nlohmann/json
#endif
#include <windows.h>

inline void SetupKonsoleToGerman();

inline void LoadPresets();

inline void WriteLog(const std::string& nachricht);

enum class FileORFolder{
    File,
    Folder,
};

inline void readInputArguments();


//
// Created by Rookies Garage on 27.05.2026.
//
#pragma once
#include <string>
#include <windows.h>

inline void SetupKonsoleToGerman();

inline void LoadPresets();

enum class FileORFolder{
    File,
    Folder,
};

inline void readInputArguments();


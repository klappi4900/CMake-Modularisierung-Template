//
// Created by Rookies Garage on 21.11.2025.
//
#pragma once
#include "Presets.h"

#include <fstream>

namespace Setup {
    void SetupKonsoleToGerman(){
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    #endif
    }

    void WriteLog(const std::string& nachricht){
    #ifdef APP_LOG_FILE
        // In die per add_log() verdrahtete Logdatei anhaengen.
        std::ofstream out(APP_LOG_FILE, std::ios::app);
        if (out) {
            out << nachricht << "\n";
        }
    #endif
    }

    void readInputArguments(){
        static std::string shortinput("-d");
        static int FileorDir = 0;
        if (shortinput == "-d") {
            FileorDir = static_cast<int>(FileORFolder::Folder);
        }
    }
}

//
// Created by Rookies Garage on 21.11.2025.
//
#pragma once
#include "Presets.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

inline void SetupKonsoleToGerman(){
    // Globale Locale auf System-Sprache setzen
    //std::locale::global(std::locale(""));
    //std::cout.imbue(std::locale());
    //std::cerr.imbue(std::locale());
    //std::cin.imbue(std::locale());
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
};

inline void LoadPresets(){
#ifdef APP_CONFIG_FILE
    // Laufzeit-Konfiguration aus der per add_config() verdrahteten config.json lesen.
    std::ifstream in(APP_CONFIG_FILE);
    if (!in) {
        std::cerr << "[Presets] Konfiguration nicht gefunden: " << APP_CONFIG_FILE << "\n";
        return;
    }

    const nlohmann::json cfg = nlohmann::json::parse(in, nullptr, /*allow_exceptions=*/false);
    if (cfg.is_discarded()) {
        std::cerr << "[Presets] config.json ist kein gueltiges JSON.\n";
        return;
    }

    if (cfg.value("konsoleUtf8", false)) {
        SetupKonsoleToGerman();
    }

    const auto begruessung = cfg.value("begruessung", std::string{});
    if (!begruessung.empty()) {
        std::cout << begruessung << "\n";
    }
    std::cout << "[Presets] Sprache=" << cfg.value("sprache",  std::string{"?"})
              << ", LogLevel="        << cfg.value("logLevel", std::string{"?"}) << "\n";
#else
    // Ohne add_config() ist keine Konfiguration verdrahtet -> No-Op (wie zuvor).
#endif
}

inline void WriteLog(const std::string& nachricht){
#ifdef APP_LOG_FILE
    // In die per add_log() verdrahtete Logdatei neben der Exe anhaengen.
    std::ofstream out(APP_LOG_FILE, std::ios::app);
    if (out) {
        out << nachricht << "\n";
    }
#endif
}


inline void readInputArguments(){
    static std::string shortinput("-d");
    static int FileorDir = 0;
    if (shortinput == "-d") {
        FileorDir = static_cast<int>(FileORFolder::Folder);
    };
}


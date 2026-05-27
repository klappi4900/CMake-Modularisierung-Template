//
// Created by Rookies Garage on 21.11.2025.
//
#pragma once
#include "Presets.h"

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
    // TODO: Presets aus Datei/Konfiguration laden und global setzen.
    // Aktuell bewusst ein No-Op-Stub, damit keine toten Variablen suggerieren,
    // dass hier schon etwas konfiguriert wird.
}


inline void readInputArguments(){
    static std::string shortinput("-d");
    static int FileorDir = 0;
    if (shortinput == "-d") {
        FileorDir = static_cast<int>(FileORFolder::Folder);
    };
}


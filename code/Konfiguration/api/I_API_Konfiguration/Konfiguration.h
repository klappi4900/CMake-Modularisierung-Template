#pragma once
#include <string>

namespace Setup
{
    // Reiner Vertrag: die effektiven Einstellungen als Wert-Objekt.
    // Bewusst OHNE JSON und OHNE Datei-I/O - damit dieser Header von jeder
    // Schicht gesehen werden darf (auch vom Compositor und einer spaeteren GUI).
    // Das Laden/Speichern aus config.json ist Implementierungsdetail des
    // app-Layers (siehe code/app/KonfigurationIO.*).
    struct Konfiguration
    {
        // --- Darstellung / Presets ---
        std::string sprache     = "de";
        bool        konsoleUtf8 = true;
        std::string begruessung = "";
        std::string logLevel    = "info";

        // --- Verhalten ---
        bool        verbose = false;
        std::string path    = ".";
        std::string output  = "";
    };
} // namespace Setup

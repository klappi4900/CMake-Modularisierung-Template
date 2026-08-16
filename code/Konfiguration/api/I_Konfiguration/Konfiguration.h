#pragma once
#include <string>

namespace Setup
{
    // Reiner Vertrag: die effektiven Einstellungen als Wert-Objekt.
    // Bewusst OHNE JSON und OHNE Datei-I/O - damit dieser Header von jeder Schicht
    // gesehen werden darf (auch vom Compositor und einer spaeteren GUI).
    // Laden/Speichern uebernimmt der app-Layer (KonfigurationIO.*).
    struct Konfiguration
    {
        // --- Darstellung / Presets ---
        std::string sprache     = "DE_de";
        bool        konsoleUtf8 = true;
        std::string begruessung = "keine struct Begruessung";
        std::string logLevel    = "info";

        // --- Verhalten ---
        bool        verbose = false;
        std::string path    = ".";
        std::string output  = "";
    };
} // namespace Setup

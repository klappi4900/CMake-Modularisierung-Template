//
// Zentrale, typisierte Konfiguration des Programms.
// Sammelt Einstellungen aus: Defaults (hier im Code) -> JSON-Datei -> Kommandozeilen-Argumente.
//
#pragma once
#include <string>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>

namespace Setup
{
    class Konfiguration
    {
    public:
        // --- Einstellungen mit sinnvollen Defaults ---
        bool verbose = false; // --verbose (Flag)
        std::string path = "."; // --path <wert>
        std::string output = ""; // --output <wert>
        std::filesystem::path config ="config.json";

        // Übernimmt die bereits geparsten Kommandozeilen-Argumente.
        // Nur tatsächlich vorhandene Schlüssel überschreiben die aktuellen Werte
        // (höchste Priorität in der Konfigurations-Schichtung).
        void uebernehme_argumente(const std::map<std::string, std::string>& argumente);

        // Lädt eine Konfiguration aus einer JSON-Datei.
        // Existiert die Datei nicht, werden die Code-Defaults zurückgegeben.
        static Konfiguration aus_datei(std::filesystem::path pfad);

        // Schreibt die aktuelle Konfiguration als (eingerücktes) JSON in eine Datei.
        void in_datei(const std::string& pfad) const;

        void print_aktuelle_Konfiguration() const;
    };

    // Erzeugt automatisch die JSON-Umwandlung (to_json / from_json) für die
    // aufgeführten Felder. "WITH_DEFAULT": fehlt ein Feld in der Datei, bleibt
    // der oben gesetzte Default erhalten, statt einen Fehler zu werfen.
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Konfiguration, verbose, path, output,config)
} // namespace Setup

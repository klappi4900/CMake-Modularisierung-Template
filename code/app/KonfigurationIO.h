#pragma once
// app-Layer: Persistenz + Argument-Merge fuer den Konfigurations-Vertrag.
// Haelt JSON (nlohmann) aus dem Vertrag heraus - dieser Header kennt kein JSON,
// nur der zugehoerige .cpp bindet nlohmann ein.
#include <filesystem>
#include <map>
#include <string>

#include "I_API_Konfiguration/Konfiguration.h"

namespace Setup
{
    // config.json -> struct. Fehlt die Datei oder ist sie ungueltig, greifen die
    // Code-Defaults aus Konfiguration.
    Konfiguration lade_konfiguration(const std::filesystem::path& pfad);

    // struct -> config.json (eingerueckt). Der explizite "Speichern"-Schritt -
    // nur hier wird die Datei angefasst.
    void speichere_konfiguration(const Konfiguration& konfig, const std::filesystem::path& pfad);

    // Geparste CLI-Argumente ueber die bestehenden Werte legen (hoechste
    // Prioritaet in der Kaskade). Nur vorhandene Schluessel ueberschreiben.
    void uebernehme_argumente(Konfiguration& konfig, const std::map<std::string, std::string>& argumente);

    void print_konfiguration(const Konfiguration& konfig);
} // namespace Setup

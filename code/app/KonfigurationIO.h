#pragma once
// app-Layer-Header: Persistenz + Argument-Merge fuer den Vertrag-struct.
// Haelt JSON aus dem Vertrag heraus - dieser Header kennt kein JSON,
// nur KonfigurationIO.cpp bindet nlohmann ein.
#include <filesystem>
#include <map>
#include <string>

#include "I_Konfiguration/Konfiguration.h"   // Vertrag über Konfiguration_API (kein ../)

namespace Setup
{
    // config.json -> struct. Fehlt die Datei oder ist sie ungueltig, greifen die
    // Code-Defaults aus Konfiguration.
    Konfiguration lade_konfiguration(const std::filesystem::path& pfad);

    // struct -> config.json (eingerueckt). Der explizite "Speichern"-Schritt.
    void speichere_konfiguration(const Konfiguration& konfig, const std::filesystem::path& pfad);

    // Geparste CLI-Argumente ueber die bestehenden Werte legen (hoechste Prioritaet).
    void uebernehme_argumente(Konfiguration& konfig, const std::map<std::string, std::string>& argumente);

    void print_konfiguration(const Konfiguration& konfig);
} // namespace Setup

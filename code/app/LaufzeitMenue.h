//
// Ebene 5 der Konfigurations-Kaskade: nachträgliches Anpassen zur Laufzeit.
//
// Zeigt ein interaktives Menü, mit dem der Benutzer die bereits aufgebaute
// (effektive) Konfiguration verändern und dauerhaft in die Config-Datei
// zurückschreiben kann.
//
#pragma once
#include <filesystem>
#include "KonfigurationIO.h"   // Vertrag-struct + speichere_konfiguration()

// Startet die interaktive Menü-Schleife.
//   config        - die effektive Konfiguration; wird direkt verändert (Referenz!).
//   speicherpfad  - Ziel für das Zurückschreiben (derselbe Pfad, aus dem geladen wurde).
void laufzeit_menue(Setup::Konfiguration& config, const std::filesystem::path& speicherpfad);

// code/app/main.cpp
// Composition Root: baut aus config.json + CLI-Argumenten EINE effektive
// Konfiguration und injiziert sie in die Application (Compositor).
#include "Presets.h"            // SetupKonsoleToGerman(), WriteLog()
#include "KonfigurationIO.h"
#include "ArgumenteAuswerten.h" // CLI-Parser
#include "LaufzeitMenue.h"
#include "Application.h"        // Compositor

#include <filesystem>
#include <iostream>
#include <map>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    //using namespace Setup;

    // Kaskade Stufe 1+2: Code-Defaults (im struct) <- config.json.
    // Ohne add_config() fehlt APP_CONFIG_PATH -> nur die struct-Defaults greifen.
#ifdef APP_CONFIG_PATH
    const std::filesystem::path konfigPfad = APP_CONFIG_PATH;
    Setup::Konfiguration konfig = Setup::lade_konfiguration(konfigPfad);
#else
    const std::filesystem::path konfigPfad{};
    Setup::Konfiguration konfig{};
#endif

    // Kaskade Stufe 3: CLI-Argumente darueberlegen (hoechste Prioritaet).
    const std::vector<std::string> argumente(argv + 1, argv + argc);
    const std::map<std::string, std::string> erlaubteOptionen = {
        {"verbose",  "Ausfuehrliche Ausgabe (Flag)"},
        {"path",     "Arbeitsverzeichnis"},
        {"output",   "Ausgabedatei"},
        {"sprache",  "Sprachkuerzel, z.B. de/en"},
        {"logLevel", "info|debug|warn|error"},
    };
    Setup::ArgumenteAuswerten parser{argumente, erlaubteOptionen};
    uebernehme_argumente(konfig, parser.get_accepted_arguments());

    // Ab hier steht die effektive Konfiguration fest.
    if (konfig.konsoleUtf8) {
        Setup::SetupKonsoleToGerman();
    }
    if (!konfig.begruessung.empty()) {
        std::cout << konfig.begruessung << "\n";
    }
    print_konfiguration(konfig);

    Setup::WriteLog("Anwendung gestartet");

    // Konfiguration in die Komposition injizieren (Dependency Injection).
    Application app{konfig};
    app.run();

    // Kaskade Stufe 4 (optional): interaktiv nachjustieren und zurueckschreiben.
    // konfig wird live veraendert; app haelt dieselbe Referenz.
    // laufzeit_menue(konfig, konfigPfad);

    app.stop();

    Setup::WriteLog("Anwendung gestoppt");
}

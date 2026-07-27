// code/app/main.cpp  ← kennt Application, Presets.cpp und die Konfiguration (Vertrag + IO)
#include "Presets.cpp"
#include "Application.h"
#include "KonfigurationIO.h"      // Vertrag (Konfiguration_API) + Persistenz; nlohmann steckt in KonfigurationIO.cpp
#include "ArgumenteAuswerten.h"   // CLI-Parser (app-intern)

#include <map>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    LoadPresets();
    WriteLog("Anwendung gestartet");

    // Konfigurations-Kaskade an der app-Grenze:
    //   struct-Defaults  <-  config.json  <-  CLI-Argumente (hoechste Prioritaet)
#ifdef APP_CONFIG_FILE
    Setup::Konfiguration konfig = Setup::lade_konfiguration(APP_CONFIG_FILE);
#else
    Setup::Konfiguration konfig{};
#endif

    // Erlaubte Optionen (Name -> Kurzbeschreibung, nur fuer die Ausgabe).
    const std::map<std::string, std::string> erlaubte_optionen{
        {"verbose",  "ausfuehrliche Ausgabe (Flag)"},
        {"path",     "Arbeitsverzeichnis"},
        {"output",   "Ausgabedatei"},
        {"sprache",  "Sprachkuerzel, z. B. de/en"},
        {"logLevel", "Log-Stufe, z. B. info/debug"},
    };

    // argv[1..] -> Vektor; Parser sammelt die Optionen, uebernehme_argumente legt
    // sie ueber die bereits geladene Konfiguration (nur bekannte Keys wirken).
    const std::vector<std::string> argumente(argv + 1, argv + argc);
    const Setup::ArgumenteAuswerten parser{argumente, erlaubte_optionen};
    Setup::uebernehme_argumente(konfig, parser.get_accepted_arguments());

    Setup::print_konfiguration(konfig);

    // DI: die fertige Konfiguration in Application injizieren (const&, muss app ueberleben).
    Application app{konfig};
    app.run();
    app.stop();

    WriteLog("Anwendung gestoppt");
}
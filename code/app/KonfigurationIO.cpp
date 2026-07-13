#include "KonfigurationIO.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace Setup
{
    // Erzeugt to_json/from_json fuer den Vertrag - lokal im app-Layer, damit der
    // Vertrag selbst JSON-frei bleibt. WITH_DEFAULT: fehlt ein Feld in der Datei,
    // bleibt der struct-Default erhalten (kein Fehler).
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
        Konfiguration, sprache, konsoleUtf8, begruessung, logLevel, verbose, path, output)

    Konfiguration lade_konfiguration(const std::filesystem::path& pfad)
    {
        std::ifstream in(pfad);
        if (!in)
        {
            // Keine Datei -> mit den Code-Defaults arbeiten.
            return Konfiguration{};
        }

        const nlohmann::json j = nlohmann::json::parse(in, nullptr, /*allow_exceptions=*/false);
        if (j.is_discarded())
        {
            std::cerr << "[Konfiguration] Ungueltiges JSON, nutze Defaults: " << pfad << "\n";
            return Konfiguration{};
        }
        return j.get<Konfiguration>();
    }

    void speichere_konfiguration(const Konfiguration& konfig, const std::filesystem::path& pfad)
    {
        std::ofstream out(pfad);
        if (!out)
        {
            std::cerr << "[Konfiguration] Konnte nicht speichern: " << pfad << "\n";
            return;
        }
        out << nlohmann::json(konfig).dump(4);
    }

    void uebernehme_argumente(Konfiguration& konfig, const std::map<std::string, std::string>& argumente)
    {
        // verbose ist ein Flag: Anwesenheit des Schluessels genuegt.
        if (argumente.contains("verbose"))
        {
            konfig.verbose = true;
        }
        if (const auto it = argumente.find("path");     it != argumente.end()) konfig.path     = it->second;
        if (const auto it = argumente.find("output");   it != argumente.end()) konfig.output   = it->second;
        if (const auto it = argumente.find("sprache");  it != argumente.end()) konfig.sprache  = it->second;
        if (const auto it = argumente.find("logLevel"); it != argumente.end()) konfig.logLevel = it->second;
    }

    void print_konfiguration(const Konfiguration& konfig)
    {
        std::cout << "\n--- Effektive Konfiguration ---\n"
                  << "sprache     = " << konfig.sprache << "\n"
                  << "konsoleUtf8 = " << std::boolalpha << konfig.konsoleUtf8 << "\n"
                  << "logLevel    = " << konfig.logLevel << "\n"
                  << "verbose     = " << konfig.verbose << "\n"
                  << "path        = " << konfig.path << "\n"
                  << "output      = " << konfig.output << "\n";
    }
} // namespace Setup

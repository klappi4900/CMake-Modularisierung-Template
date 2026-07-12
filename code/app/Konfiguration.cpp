//
// Implementierung der Konfiguration.
//
#include "../includes/Konfiguration.h"
#include <fstream>

namespace Setup
{
    void Konfiguration::uebernehme_argumente(const std::map<std::string, std::string>& argumente)
    {
        // verbose ist ein Flag: Wird der Schluessel ueberhaupt uebergeben, gilt es als gesetzt.
        // (Der Parser legt fuer Flags den Wert "LongSchalter"/"shortSchalter" ab -
        //  der konkrete Wert ist hier egal, nur die Anwesenheit zaehlt.)
        if (argumente.contains("verbose"))
        {
            verbose = true;
        }

        // String-Optionen: nur uebernehmen, wenn tatsaechlich vorhanden.
        if (auto it = argumente.find("path"); it != argumente.end())
        {
            path = it->second;
        }
        if (auto it = argumente.find("output"); it != argumente.end())
        {
            output = it->second;
        }
        if (auto it = argumente.find("config"); it != argumente.end())
        {
            config = it->second;
        }
    }

    Konfiguration Konfiguration::aus_datei(const std::filesystem::path pfad)
    {
        std::ifstream datei(pfad);
        if (!datei)
        {
            // Keine Datei vorhanden -> mit den Code-Defaults arbeiten.
            return Konfiguration{};
        }
        nlohmann::json j = nlohmann::json::parse(datei);
        return j.get<Konfiguration>(); // JSON -> typisiertes Objekt
    }

    void Konfiguration::in_datei(const std::string& pfad) const
    {
        nlohmann::json j = *this; // typisiertes Objekt -> JSON
        std::ofstream datei(pfad);
        datei << j.dump(4); // 4 = Einrueckung fuer lesbare Ausgabe
    }

    void Konfiguration::print_aktuelle_Konfiguration() const
    {
        // Ab hier arbeitet das Programm nur noch mit dem typisierten config-Objekt:
        std::cout << "\n--- Effektive Konfiguration ---" << std::endl;
        std::cout << "verbose = " << std::boolalpha << this->verbose << std::endl;
        std::cout << "path    = " << this->path << std::endl;
        std::cout << "output  = " << this->output << std::endl;
        std::cout << "config  = " << this->config << std::endl;

    }

} // namespace Setup

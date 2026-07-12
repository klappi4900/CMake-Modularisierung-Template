//
// Implementierung von Ebene 5: das interaktive Laufzeit-Menü.
//
#include "../includes/LaufzeitMenue.h"

#include <cctype>
#include <iostream>
#include <string>

namespace {

    // Liest eine ganze Zeile robust ein. Gibt false zurück, wenn der
    // Eingabestrom endet (z. B. Strg+Z / EOF) - dann soll die Schleife stoppen.
    bool zeile_lesen(const std::string& aufforderung, std::string& ziel) {
        std::cout << aufforderung;
        return static_cast<bool>(std::getline(std::cin, ziel));
    }

    // Interpretiert eine Benutzereingabe als Wahrheitswert.
    // Akzeptiert "true/1/ja/on" (sonst false), Groß-/Kleinschreibung egal.
    bool als_bool(std::string eingabe) {
        for (char& c : eingabe) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return eingabe == "true" || eingabe == "1" || eingabe == "ja" || eingabe == "on";
    }

    // Zeigt die aktuell wirksamen Werte an.
    void zeige_konfiguration(const Setup::Konfiguration& config) {
        std::cout << "\n--- Aktuelle Konfiguration ---\n"
                  << "  verbose = " << std::boolalpha << config.verbose << "\n"
                  << "  path    = " << config.path << "\n"
                  << "  output  = " << config.output << "\n";
    }

} // namespace

void laufzeit_menue(Setup::Konfiguration& config, const std::filesystem::path& speicherpfad) {
    std::string eingabe;

    while (true) {
        zeige_konfiguration(config);

        std::cout << "\nWas moechten Sie tun?\n"
                  << "  1) verbose umschalten\n"
                  << "  2) path aendern\n"
                  << "  3) output aendern\n"
                  << "  4) speichern (in " << speicherpfad.string() << ")\n"
                  << "  0) beenden\n";

        if (!zeile_lesen("Auswahl: ", eingabe)) {
            // EOF -> Menue verlassen.
            break;
        }

        if (eingabe == "0") {
            break;
        }
        else if (eingabe == "1") {
            std::string wert;
            zeile_lesen("verbose (true/false), leer = umschalten: ", wert);
            config.verbose = wert.empty() ? !config.verbose : als_bool(wert);
        }
        else if (eingabe == "2") {
            zeile_lesen("Neuer path: ", config.path);
        }
        else if (eingabe == "3") {
            zeile_lesen("Neuer output: ", config.output);
        }
        else if (eingabe == "4") {
            config.in_datei(speicherpfad.string());
            std::cout << "Gespeichert.\n";
        }
        else {
            std::cout << "Ungueltige Auswahl: \"" << eingabe << "\"\n";
        }
    }

    std::cout << "Menue beendet.\n";
}

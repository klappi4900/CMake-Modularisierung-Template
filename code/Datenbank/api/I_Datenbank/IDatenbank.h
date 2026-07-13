#pragma once
// Vertrag der Persistenz-Schicht (reiner Header, keine Implementierung).
// Consumer (z. B. Explorer, Compositor) hängen nur hiervon ab, nie von einer
// konkreten Datenbank.
#include <optional>
#include <string>

namespace Persistenz
{
    struct IDatenbank
    {
        virtual ~IDatenbank() = default;

        virtual void schreibe(const std::string& schluessel, const std::string& wert) = 0;
        virtual std::optional<std::string> lese(const std::string& schluessel) const = 0;
    };
} // namespace Persistenz

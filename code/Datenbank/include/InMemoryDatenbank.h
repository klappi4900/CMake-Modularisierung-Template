#pragma once
// Konkrete Implementierung des IDatenbank-Vertrags: schlichter In-Memory-Speicher,
// ohne externe Abhängigkeiten. Kennt nur der Compositor.
#include "I_Datenbank/IDatenbank.h"   // eigener Vertrag über Datenbank_API (kein ../)

#include <map>

namespace Persistenz
{
    class InMemoryDatenbank : public IDatenbank
    {
    public:
        void schreibe(const std::string& schluessel, const std::string& wert) override;
        std::optional<std::string> lese(const std::string& schluessel) const override;

    private:
        std::map<std::string, std::string> speicher_;
    };
} // namespace Persistenz

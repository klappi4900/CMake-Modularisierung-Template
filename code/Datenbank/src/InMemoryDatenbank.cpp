#include "InMemoryDatenbank.h"

namespace Persistenz
{
    void InMemoryDatenbank::schreibe(const std::string& schluessel, const std::string& wert)
    {
        speicher_[schluessel] = wert;
    }

    std::optional<std::string> InMemoryDatenbank::lese(const std::string& schluessel) const
    {
        if (const auto it = speicher_.find(schluessel); it != speicher_.end())
        {
            return it->second;
        }
        return std::nullopt;
    }
} // namespace Persistenz

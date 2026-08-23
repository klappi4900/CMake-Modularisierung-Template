// compositor/src/Application.cpp
// Komposition: hier darf alles Konkrete bekannt sein.
#include "Application.h"

#include "I_Konfiguration/Konfiguration.h"
#include "InMemoryDatenbank.h"

#include <iostream>
#include <vector>

#ifdef ENABLE_F_OPCHECK
#include "FileWriter.h"
#endif
#ifdef ENABLE_LOGGER
#include "Logger.h"
#endif

Application::Application(const Setup::Konfiguration& konfig)
    : konfig_(konfig) {
#ifdef ENABLE_F_OPCHECK
    std::vector<std::unique_ptr<I_WriteCheck>> checks;   // vorerst leer
    auto fw = std::make_unique<FileWriter>(std::move(checks));

#ifdef ENABLE_LOGGER
    logger_ = std::make_unique<Logger>(*fw);
#endif
    fileWriter_ = std::move(fw);
#endif

    // Pflicht-Subsystem: In-Memory-Datenbank (nur hier ist der konkrete Typ bekannt).
    datenbank_ = std::make_unique<Persistenz::InMemoryDatenbank>();
}

Application::~Application() = default;

void Application::run() {
    // Demonstriert die injizierte Datenbank end-to-end: schreiben + lesen.
    datenbank_->schreibe("sprache", konfig_.sprache);
    std::cout << "[Datenbank] sprache = "
              << datenbank_->lese("sprache").value_or("?") << "\n";
#ifdef ENABLE_LOGGER
    // Beispiel fuer die injizierte Konfiguration: das LogLevel stammt aus config.json.
    logger_->log("Anwendung gestartet (logLevel=" + konfig_.logLevel + ")");
#endif
}

void Application::stop()
{
#ifdef ENABLE_LOGGER
    logger_->log("Anwendung gestoppt");
#endif
}

// compositor/src/Application.cpp
// Komposition: hier darf alles Konkrete bekannt sein.
#include "Application.h"

#include <vector>

#include "I_Konfiguration/Konfiguration.h"   // Vertrag: Felder von Setup::Konfiguration

#ifdef ENABLE_F_OPCHECK
#include "FileWriter.h"
#endif
#ifdef ENABLE_LOGGER
#include "Logger.h"
#endif

Application::Application(const Setup::Konfiguration& konfig)
    : konfig_(konfig) {
#ifdef ENABLE_F_OPCHECK
    std::vector<std::unique_ptr<IWriteCheck>> checks;   // vorerst leer
    auto fw = std::make_unique<FileWriter>(std::move(checks));

#ifdef ENABLE_LOGGER
    logger_ = std::make_unique<Logger>(*fw);
#endif
    fileWriter_ = std::move(fw);
#endif
}

Application::~Application() = default;

void Application::run() {
#ifdef ENABLE_LOGGER
    logger_->log("Anwendung gestartet");
    if (konfig_.verbose)
        logger_->log("[verbose] path=" + konfig_.path + ", output=" + konfig_.output);
#endif
}

void Application::stop()
{
#ifdef ENABLE_LOGGER
    logger_->log("Anwendung gestoppt");
#endif
}

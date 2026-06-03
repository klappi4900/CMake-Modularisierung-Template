// compositor/src/Application.cpp
// Komposition: hier darf alles Konkrete bekannt sein.
#include "Application.h"

#include <vector>

#include "FileWriter.h"
#include "Logger.h"

Application::Application() {
    std::vector<std::unique_ptr<IWriteCheck>> checks;   // vorerst leer

    auto fw = std::make_unique<FileWriter>(std::move(checks));
    logger_     = std::make_unique<Logger>(*fw);
    fileWriter_ = std::move(fw);
}

Application::~Application() = default;

void Application::run() {
    logger_->log("Anwendung gestartet");
}

void Application::stop()
{
    logger_->log("Anwendung gestoppt");
}

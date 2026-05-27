// compositor/src/Application.cpp
// Komposition: hier darf alles Konkrete bekannt sein.
#include "compositor/Application.h"

#include <vector>

#include "F_OPCheck/FileWriter.h"
#include "Logger/Logger.h"

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

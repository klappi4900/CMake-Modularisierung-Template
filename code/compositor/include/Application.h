#pragma once
#include <memory>

namespace Setup { struct Konfiguration; }   // Vertrag (Konfiguration_API), nur Forward-Decl

#ifdef ENABLE_F_OPCHECK
class IFileWriter;
#endif
#ifdef ENABLE_LOGGER
class ILogger;
#endif

class Application {
public:
    // DI: die effektive Konfiguration wird injiziert (wie Logger(IFileWriter&)).
    // Der Aufrufer muss sie laenger am Leben halten als die Application (const&).
    explicit Application(const Setup::Konfiguration& konfig);
    ~Application();
    void run();
    void stop();

private:
    const Setup::Konfiguration& konfig_;
#ifdef ENABLE_F_OPCHECK
    std::unique_ptr<IFileWriter> fileWriter_;
#endif
#ifdef ENABLE_LOGGER
    std::unique_ptr<ILogger>     logger_;
#endif
};

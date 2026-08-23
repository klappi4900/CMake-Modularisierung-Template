#pragma once
#include <memory>

// Vorwaertsdeklaration reicht: konfig_ ist eine Referenz (unvollstaendiger Typ
// erlaubt). Die volle Definition zieht erst Application.cpp ueber Konfiguration_API.
namespace Setup      { struct Konfiguration; }
namespace Persistenz { struct IDatenbank; }

#ifdef ENABLE_F_OPCHECK
class I_FileWriter;
#endif
#ifdef ENABLE_LOGGER
class ILogger;
#endif

class Application {
public:
    // Konfiguration wird injiziert - der Compositor liest daraus, statt config.json
    // selbst zu kennen. Der Aufrufer (app) muss das Objekt laenger am Leben halten
    // als die Application.
    explicit Application(const Setup::Konfiguration& konfig);
    ~Application();
    void run();
    void stop();

private:
    const Setup::Konfiguration&             konfig_;
    std::unique_ptr<Persistenz::IDatenbank> datenbank_;
#ifdef ENABLE_F_OPCHECK
    std::unique_ptr<I_FileWriter> fileWriter_;
#endif
#ifdef ENABLE_LOGGER
    std::unique_ptr<ILogger>     logger_;
#endif
};

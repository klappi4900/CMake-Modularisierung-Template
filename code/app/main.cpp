// code/app/main.cpp  ← kennt nur Application und Presets.cpp
#include "Presets.cpp"
#include "Application.h"

int main() {
    LoadPresets();
    WriteLog("Anwendung gestartet");

    Application app;
    app.run();
    app.stop();

    WriteLog("Anwendung gestoppt");
}
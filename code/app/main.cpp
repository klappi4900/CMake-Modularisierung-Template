// code/app/main.cpp  ← kennt nur Application und Presets.cpp
#include "Presets.cpp"
#include "Application.h"

int main() {
    LoadPresets();

    Application app;
    app.run();
    app.stop();
}
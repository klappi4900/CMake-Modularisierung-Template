// code/app/main.cpp  ← kennt nur Application und Presets.cpp
#include "Presets.cpp"
#include "compositor/Application.h"

int main() {
    LoadPresets();

    Application app;
    app.run();
}
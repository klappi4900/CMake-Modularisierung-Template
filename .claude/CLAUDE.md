# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

CMake-Projekt mit C++23, Ninja-Generator, Build-Verzeichnis `build/`:

```powershell
cmake -S . -B build -G Ninja
cmake --build build
build\code\app\Application.exe
```

Einzelnes Target bauen: `cmake --build build --target <Logger|F_OPCheck|Compositor|Application>`.

Es gibt keine Tests, keinen Linter und keine CI — dies ist ein Lernprojekt für CMake-Modularisierung.

## Architektur

Vier Module unter `code/`. `F_OP_Check` und `Logger` folgen derselben Struktur (`api/` + `include/` + `src/` + `CMakeLists.txt`) mit strikter Trennung von **Vertrag** (API, INTERFACE-Lib, nur Header) und **Implementierung** (STATIC-Lib). `Compositor` hat nur `include/` + `src/` (kein eigenes `api/`, da niemand gegen `Compositor` linkt außer `app`). `app` ist flach — `main.cpp`, `Presets.h`/`.cpp`, `CMakeLists.txt` direkt unter `code/app/`, keine Unterordner:

```
app (exe)  →  Compositor  →  F_OPCheck     (Impl)  →  F_OPCheck_API   (Interfaces)
                          →  Logger         (Impl)  →  Logger_API      (Interfaces)
                                                    ↘  F_OPCheck_API   (Konstruktor nimmt IFileWriter&)
```

Kernregeln, die der Code durchgängig durchhält und die bei Änderungen zu wahren sind:

- **Module linken nur gegen fremde APIs, nie gegen fremde Implementierungen.** `Logger` linkt `F_OPCheck_API` (PUBLIC, weil `Logger.h` im Konstruktor `IFileWriter&` führt), aber **nicht** `F_OPCheck`. Siehe Kommentar in `code/Logger/CMakeLists.txt`.
- **Komposition ausschließlich im `Compositor`-Modul.** Nur `Application.cpp` darf konkrete Typen (`FileWriter`, `Logger`) kennen und instanziieren — dort werden Implementierungen über `PRIVATE`-Links eingebunden. `Application.h` benutzt nur Forward-Decls und `std::unique_ptr<IFileWriter/ILogger>`, damit Consumer keine Implementierungen ziehen.
- **`app/main.cpp` kennt von den Modulen ausschließlich `Compositor`.** Keine direkten Abhängigkeiten zu Logger/F_OPCheck. Daneben nutzt `main.cpp` `Presets.h`/`.cpp` (App-interne Helper wie `LoadPresets()`, `SetupKonsoleToGerman()`) — das ist kein Modul im obigen Sinn, sondern gehört zu `app` selbst.
- **PUBLIC vs PRIVATE bei `target_link_libraries` ist bewusst gewählt** und folgt der Sichtbarkeit der Typen in den Headern. Vor Änderung prüfen, ob der Typ im öffentlichen Header auftaucht (→ PUBLIC) oder nur in `.cpp` (→ PRIVATE).

Include-Pfade folgen dem Modul-Prefix: `#include "Logger/Logger.h"`, `#include "F_OPCheck/FileWriter.h"`, `#include "I_API_Logger/ILogger.h"`, `#include "I_F_OPCheck/IFileWriter.h"`. Das Prefix kommt vom Verzeichnis unter `include/` bzw. `api/`, nicht vom Library-Namen.

## Neues Modul hinzufügen

1. Verzeichnis `code/<Name>/` mit `api/`, `include/<Name>/`, `src/` anlegen.
2. `api/CMakeLists.txt`: ein Aufruf `add_interface(NAME <Name>)` (Helferfunktion aus `cmake/AddInterface.cmake`, legt die INTERFACE-Lib `<Name>_API` an und setzt `target_include_directories(... INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})`). `NAME` ist der bare Modulname — das Suffix `_API` hängt die Funktion selbst an. Braucht ein öffentlicher API-Header einen Typ aus einem fremden Vertrag: `add_interface(NAME <Name> DEPENDS <Fremde>_API)`.
3. `code/<Name>/CMakeLists.txt`: ein Aufruf `add_module(NAME <Name> DEPENDS <Fremde>_API)` (Helferfunktion aus `cmake/AddModule.cmake`, kapselt `add_subdirectory(api)`, `add_library` STATIC, Include-Dirs, `target_link_libraries`).
4. In Root-`CMakeLists.txt` `add_subdirectory(code/<Name>)` **vor** `code/compositor` einhängen (ggf. mit eigenem `option(ENABLE_<NAME> ...)`-Schalter, siehe unten).

`Compositor` und `app` nutzen `add_module()` **nicht** — sie haben eigene CMakeLists.txt (Compositor braucht die `if(TARGET ...)`-Guards, app ist nur `add_executable`).

## Optionale Module (`ENABLE_LOGGER` / `ENABLE_F_OPCHECK`)

Root-`CMakeLists.txt` schaltet `Logger`/`F_OP_Check` per `option()` an/aus
(Standard `ON`). `code/compositor/CMakeLists.txt` prüft mit `if(TARGET F_OPCheck)`
bzw. `if(TARGET Logger)`, ob das jeweilige Target überhaupt existiert, bevor
gelinkt wird, und setzt bei Erfolg zusätzlich eine **PUBLIC** Compile-Definition
(`ENABLE_F_OPCHECK`/`ENABLE_LOGGER`). Diese steuert `#ifdef`-Blöcke in
`Application.h`/`.cpp` (Member, Includes, Nutzung) — PUBLIC ist nötig, damit
`app/main.cpp` (inkludiert `Application.h`) denselben Makro-Stand sieht wie
`Application.cpp`, sonst ODR-Verstoß durch unterschiedliches Klassenlayout.
`ENABLE_LOGGER=ON` ohne `ENABLE_F_OPCHECK=ON` bricht mit `FATAL_ERROR` ab
(Logger braucht `IFileWriter&`, `FileWriter` ist die einzige Implementierung).
Details siehe `README.md`.

## Fallstricke

- `if(TARGET ...)` wird **sofort** beim Erreichen der Zeile ausgewertet, nicht erst am Ende der Konfiguration wie `target_link_libraries`. Deshalb müssen `code/Logger`/`code/F_OP_Check` im Root **vor** `code/compositor` per `add_subdirectory` eingebunden werden — sonst sind die Guards immer `false`.
- `file(GLOB ... CONFIGURE_DEPENDS)` wird über `add_module()` einheitlich für Sources benutzt (Logger, F_OPCheck). `Compositor` globt weiterhin selbst in seiner eigenen CMakeLists.txt. Beim Hinzufügen neuer `.cpp`-Dateien reicht ein erneutes Konfigurieren.
- `code/app/CMakeLists.txt` globt **nicht** — `add_executable(Application main.cpp)` listet nur `main.cpp` explizit. `Presets.cpp` wird stattdessen direkt per `#include "Presets.cpp"` in `main.cpp` eingebunden (nicht als eigene Übersetzungseinheit kompiliert), deshalb sind alle Funktionen darin `inline`. Neue `.cpp`-Dateien unter `code/app/` werden nur gebaut, wenn sie entweder ebenso `#include`t oder explizit in `add_executable(...)` ergänzt werden.

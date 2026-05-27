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

Vier Module unter `code/`, jedes mit derselben Struktur (`api/` + `include/` + `src/` + `CMakeLists.txt`). Strikte Trennung von **Vertrag** (API, INTERFACE-Lib, nur Header) und **Implementierung** (STATIC-Lib):

```
app (exe)  →  Compositor  →  F_OPCheck     (Impl)  →  F_OPCheck_API   (Interfaces)
                          →  Logger         (Impl)  →  Logger_API      (Interfaces)
                                                    ↘  F_OPCheck_API   (Konstruktor nimmt IFileWriter&)
```

Kernregeln, die der Code durchgängig durchhält und die bei Änderungen zu wahren sind:

- **Module linken nur gegen fremde APIs, nie gegen fremde Implementierungen.** `Logger` linkt `F_OPCheck_API` (PUBLIC, weil `Logger.h` im Konstruktor `IFileWriter&` führt), aber **nicht** `F_OPCheck`. Siehe Kommentar in `code/Logger/CMakeLists.txt`.
- **Komposition ausschließlich im `Compositor`-Modul.** Nur `Application.cpp` darf konkrete Typen (`FileWriter`, `Logger`) kennen und instanziieren — dort werden Implementierungen über `PRIVATE`-Links eingebunden. `Application.h` benutzt nur Forward-Decls und `std::unique_ptr<IFileWriter/ILogger>`, damit Consumer keine Implementierungen ziehen.
- **`app/main.cpp` kennt ausschließlich `Compositor`.** Keine direkten Abhängigkeiten zu Logger/F_OPCheck.
- **PUBLIC vs PRIVATE bei `target_link_libraries` ist bewusst gewählt** und folgt der Sichtbarkeit der Typen in den Headern. Vor Änderung prüfen, ob der Typ im öffentlichen Header auftaucht (→ PUBLIC) oder nur in `.cpp` (→ PRIVATE).

Include-Pfade folgen dem Modul-Prefix: `#include "Logger/Logger.h"`, `#include "F_OPCheck/FileWriter.h"`, `#include "I_API_Logger/ILogger.h"`, `#include "I_F_OPCheck/IFileWriter.h"`. Das Prefix kommt vom Verzeichnis unter `include/` bzw. `api/`, nicht vom Library-Namen.

## Neues Modul hinzufügen

1. Verzeichnis `code/<Name>/` mit `api/`, `include/<Name>/`, `src/` anlegen.
2. `api/CMakeLists.txt`: INTERFACE-Lib `<Name>_API`, `target_include_directories(... INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})`.
3. `code/<Name>/CMakeLists.txt`: `add_subdirectory(api)` zuerst, dann STATIC-Lib mit `target_link_libraries(<Name> PUBLIC <Name>_API)` plus fremde `*_API`-Abhängigkeiten.
4. In Root-`CMakeLists.txt` `add_subdirectory(code/<Name>)` **vor** `code/compositor` einhängen.

## Fallstricke

- Root-`CMakeLists.txt` referenziert `code/logger` kleingeschrieben, das Verzeichnis heißt `Logger` — funktioniert nur, weil Windows-Dateisystem case-insensitive ist. Auf Linux würde der Build brechen.
- `file(GLOB ... CONFIGURE_DEPENDS)` wird in `F_OPCheck` und `Compositor` für Sources benutzt, `Logger` listet Sources explizit. Beim Hinzufügen neuer `.cpp`-Dateien entsprechend vorgehen.

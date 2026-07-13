# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Zweck

Lernprojekt zur **CMake-Modularisierung**: ein C++23-Projekt in eigenständige,
gegeneinander abgegrenzte Module (Vertrag/API vs. Implementierung) aufteilen, die
optional an-/abschaltbar sind. Kein Produktivcode, keine Tests, keine CI. Der
eigentliche Lerninhalt steckt im CMake-Aufbau, nicht in der Programmlogik.

Ausführliche Begründungen stehen in `README.md` und in den Kommentarköpfen der
`cmake/*.cmake`-Dateien — dort zuerst nachsehen.

## Build

Voraussetzung: CMake ≥ 3.20, Ninja, MSVC (C++23).

```powershell
cmake -S . -B build -G Ninja
cmake --build build
build\code\app\Application.exe
```

- Einzelnes Target: `cmake --build build --target <Logger|F_OPCheck|Compositor|Application>`.
- Module ab-/anschalten (Neu-Konfigurieren nötig):
  `cmake -S . -B build -DENABLE_LOGGER=OFF -DENABLE_F_OPCHECK=OFF`

## Architektur

Jedes Modul unter `code/<Modul>/` ist self-contained und folgt derselben Struktur:
`api/` (INTERFACE-Lib `<Modul>_API`, nur Header = der Vertrag), `include/<Modul>/`
(öffentliche Header der Implementierung), `src/` (nur intern sichtbare `.cpp`),
plus `CMakeLists.txt`.

Zentrale Regeln:
- **Module linken nur gegen fremde `_API`-Targets, nie gegen fremde
  Implementierungen.** Abhängigkeit bleibt auf den Vertrag beschränkt.
- **Komposition passiert ausschließlich im `Compositor`-Modul**
  (`code/compositor/src/Application.cpp`). `app/main.cpp` kennt nur `Application`,
  keine konkreten Logger/F_OPCheck-Typen.

## Konfiguration (Kaskade + Dependency Injection)

- `code/Konfiguration/` ist ein reines Vertrag-Modul: nur der Header-`struct`
  `Setup::Konfiguration` (`Konfiguration_API`, INTERFACE, **kein JSON, kein I/O**).
  Darf von jeder Schicht gesehen werden — auch Compositor und einer späteren GUI.
- **JSON-Persistenz + Argument-Merge liegen im `app`-Layer** (`code/app/KonfigurationIO.*`,
  nlohmann bleibt hier). `config/config.json` wird **nur an der app-Grenze** gelesen/
  geschrieben — untere Schichten fassen die Datei nie an.
- Fluss in `main.cpp`: Defaults (im struct) → `config.json` → CLI-Argumente
  (`ArgumenteAuswerten`) ergeben **eine** effektive `Konfiguration`, die per
  Konstruktor in `Application` **injiziert** wird (`Application app{konfig};`).
  Dasselbe DI-Muster wie `Logger(IFileWriter&)`. `LaufzeitMenue` kann das Objekt
  live ändern und via `speichere_konfiguration()` zurückschreiben.
- `Application.h` **forward-declared** `Setup::Konfiguration` und hält eine
  `const&` — der Aufrufer (`app`) muss das Objekt länger am Leben halten als die
  `Application`.
- Abhängigkeiten: `Application`(exe) → `Compositor` → optional `F_OPCheck` +
  `Logger`. `Logger` nimmt im Konstruktor ein `IFileWriter&` (aus `F_OPCheck`),
  daher ist `ENABLE_LOGGER=ON` mit `ENABLE_F_OPCHECK=OFF` ein
  `FATAL_ERROR` — Logger hat sonst keine `IFileWriter`-Implementierung.

## CMake-Helfer (`cmake/`, im Root per `include()` eingebunden)

Diese vier Funktionen kapseln die gesamte Modul-/Ressourcen-Boilerplate — beim
Ändern des Build-Verhaltens hier ansetzen, nicht in einzelnen `CMakeLists.txt`:

- `add_module(NAME <M> DEPENDS <Fremde>_API)` — `add_subdirectory(api)`, globbt
  `src/*.cpp`, `add_library(<M> STATIC)`, `include/` PUBLIC + `src/` PRIVATE,
  linkt PUBLIC gegen `<M>_API` + DEPENDS. (`AddModule.cmake`)
- `add_interface(NAME <M> [DEPENDS <Foo>_API])` — legt `<M>_API` (INTERFACE) an;
  Include-Root ist `api/`, daher Includes mit Modul-Prefix
  (`#include "I_API_Logger/ILogger.h"`). Aufruf in `<M>/api/CMakeLists.txt`.
  (`AddInterface.cmake`)
- `add_config(TARGET <exe>)` — INTERFACE-Target `Config`; setzt
  `APP_CONFIG_FILE`/`CONFIG_FILE`-Defines und `LOG_COMPILE_LEVEL` (2 Release, sonst
  0), kopiert `config/config.json` per POST_BUILD neben die Exe. (`AddConfig.cmake`)
- `add_log()` — INTERFACE-Target `Log`; setzt `APP_LOG_FILE` auf `<root>/log/app.log`
  und legt das Verzeichnis beim Konfigurieren an. (`AddLog.cmake`)

## Beim Arbeiten beachten (nicht-offensichtlich)

- **Reihenfolge im Root-`CMakeLists.txt`:** `code/Logger` und `code/F_OP_Check`
  müssen *vor* `code/compositor` per `add_subdirectory` kommen. Der Compositor
  entscheidet mit `if(TARGET Logger)` / `if(TARGET F_OPCheck)`, und `if(TARGET ...)`
  wird **sofort** ausgewertet (anders als `target_link_libraries`, das Namen erst
  am Konfig-Ende auflöst).
- **ODR-Falle bei den Feature-Makros:** `Compositor` setzt `ENABLE_F_OPCHECK` /
  `ENABLE_LOGGER` als **PUBLIC** compile-definition. Muss PUBLIC bleiben, damit
  `app/main.cpp` (inkludiert `Application.h`) denselben Makro-Stand wie
  `Application.cpp` sieht — sonst unterschiedliches Klassenlayout = ODR-Verstoß.
  Die `#ifdef`-Zweige stecken in `Application.h`/`.cpp` (Member `fileWriter_`,
  `logger_`, Includes, Konstruktor/`run()`/`stop()`).
- **`file(GLOB ... CONFIGURE_DEPENDS)`** wird für Sourcen genutzt (Module *und*
  `code/app/*.cpp`): neue `.cpp` erfordern nur ein erneutes Konfigurieren, keine
  manuelle Listenpflege.
- **Namenskollision beachten:** Das INTERFACE-Target `Config` (aus `add_config()`,
  liefert nur `APP_CONFIG_FILE`/Dateikopie) ist **nicht** das Konfigurations-Modul
  `Konfiguration`/`Konfiguration_API` (der Einstellungs-`struct`).

## Neues Modul hinzufügen

1. `code/<Name>/` mit `api/`, `include/<Name>/`, `src/` anlegen.
2. `api/CMakeLists.txt`: `add_interface(NAME <Name> [DEPENDS <Fremde>_API])`.
3. `code/<Name>/CMakeLists.txt`: `add_module(NAME <Name> DEPENDS <Fremde>_API)`.
4. Im Root `add_subdirectory(code/<Name>)` **vor** `code/compositor` einhängen
   (ggf. mit eigenem `option()`-Schalter + Guard im Compositor).
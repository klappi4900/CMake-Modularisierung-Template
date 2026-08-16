# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

CMake-Projekt mit C++23, Ninja-Generator, Build-Verzeichnis `build/`. Aus einer
**MSVC-Developer-Umgebung** heraus konfigurieren (Developer-Shell / `vcvars64.bat`),
sonst findet schon der Compiler-Test die Windows-SDK-Libs nicht:

```powershell
cmake -S . -B build -G Ninja
cmake --build build
build\code\app\Application.exe
```

Einzelnes Target: `cmake --build build --target <Logger|F_OPCheck|Konfiguration|Datenbank|Compositor|Application>`.

Keine Tests, kein Linter, keine CI — Lernprojekt für CMake-Modularisierung.

## Architektur

Unter `code/` liegen **vier Blatt-Module** (`F_OP_Check`, `Logger`, `Konfiguration`,
`Datenbank`) plus der **Compositor** (Composition Root) und **app** (Executable).
Jedes Blatt-Modul folgt derselben Struktur mit strikter Trennung von **Vertrag** und
**Implementierung**:

```
code/<Modul>/
├─ api/I_<Modul>/…     Vertrag: INTERFACE-Lib <Modul>_API (nur Header)
├─ include/…           öffentliche Header der Implementierung
├─ src/*.cpp           Implementierung (STATIC-Lib <Modul>)
└─ CMakeLists.txt      add_module(NAME <Modul> …)
```

`Datenbank` und `Konfiguration` sind die sauberen Referenzbeispiele. Abhängigkeits-
und Kompositions-Bild:

```
app (exe) ─┬─▶ Konfiguration (Impl)  → Konfiguration_API (struct)   [+ PRIVATE nlohmann_json]
           │     main.cpp lädt config.json + mischt CLI-Argumente zur effektiven Konfiguration
           └─▶ Compositor ─┬─PUBLIC──▶ Konfiguration_API   (Application-Ctor: const Konfiguration&)
                           └─PRIVATE─▶ Datenbank            (Pflicht)  [InMemoryDatenbank]
                                       Logger, F_OPCheck    (optional, via compositor_linked)

Logger (Impl) ─▶ Logger_API, F_OPCheck_API        F_OPCheck (Impl) ─▶ F_OPCheck_API
```

**Composition Root:** `app/main.cpp` baut aus `config.json` (Modul `Konfiguration`) +
CLI-Argumenten *eine* effektive `Setup::Konfiguration` und **injiziert** sie in
`Application`. Der `Compositor` (`Application.cpp`) instanziiert die konkreten
Implementierungen (`InMemoryDatenbank`, `Logger`, `FileWriter`) und versteckt sie
hinter `Application`.

Kernregeln, die der Code durchhält und die bei Änderungen zu wahren sind:

- **Blatt-Module linken nur gegen fremde APIs, nie gegen fremde Implementierungen.**
  `Logger` linkt `F_OPCheck_API` (PUBLIC, weil `Logger.h` im Ctor `IFileWriter&`
  führt), aber **nicht** `F_OPCheck`. `Konfiguration` linkt `nlohmann_json` **PRIVATE**
  (json nur im `.cpp`).
- **Komposition / konkrete Typen ausschließlich im `Compositor`.** Nur
  `Application.cpp` kennt `FileWriter`, `Logger`, `InMemoryDatenbank` und linkt
  Implementierungen **PRIVATE**. `Application.h` nutzt nur Forward-Decls +
  `std::unique_ptr<IFileWriter/ILogger/IDatenbank>` und nimmt `const Setup::Konfiguration&` (DI).
- **`app` kennt: `Compositor` + `Konfiguration` (Impl) + die Ressourcen `Config`/`Log`.**
  `main.cpp` nutzt das `Konfiguration`-Modul (`lade_konfiguration`,
  `uebernehme_argumente`, `print_konfiguration`) für den Config-Aufbau und
  `Presets.h`/`.cpp` (`SetupKonsoleToGerman()`, `WriteLog()`) als app-interne Helfer.
  Konkrete Modul-Impls (Logger/Datenbank/…) kennt `app` **nicht** — die zieht der Compositor.
- **PUBLIC vs PRIVATE** folgt der Sichtbarkeit in den Headern: Typ im öffentlichen
  Header → PUBLIC (`DEPENDS`), nur in `.cpp` → PRIVATE (`PRIVATE_DEPENDS`).

Include-Prefix kommt vom Verzeichnis unter `api/` bzw. `include/`:
`#include "I_Konfiguration/Konfiguration.h"`, `#include "I_Datenbank/IDatenbank.h"`,
`#include "I_Logger/ILogger.h"`, `#include "Logger/Logger.h"`. **Keine `../`-Relativ-Includes**
— der Vertrag ist über die `_API`-Include-Dir erreichbar.

## Neues Modul hinzufügen

1. `code/<Name>/` mit `api/I_<Name>/` (Vertrag), `include/` (Impl-Header), `src/`.
2. `api/CMakeLists.txt`: `add_interface(NAME <Name>)` (aus `cmake/AddInterface.cmake`;
   legt `<Name>_API` an, Include-Dir = `api/`). Braucht ein Vertragsheader einen
   fremden Typ: `add_interface(NAME <Name> DEPENDS <Fremde>_API)`.
3. `code/<Name>/CMakeLists.txt`:
   `add_module(NAME <Name> [DEPENDS <Fremde>_API] [PRIVATE_DEPENDS <lib>])`
   (aus `cmake/AddModule.cmake`). `DEPENDS` → PUBLIC (Typ im öffentlichen Header),
   `PRIVATE_DEPENDS` → PRIVATE (nur im `.cpp`, z. B. `nlohmann_json`).
4. Root-`CMakeLists.txt`: `add_subdirectory(code/<Name>)` **vor** `code/compositor`
   (der Compositor prüft optionale Module per `if(TARGET …)`). Optional `option(ENABLE_<NAME> …)`.

**Den Vertrag** (Interface/struct, den andere brauchen) in `api/I_<Name>/`,
**Impl-Header** in `include/`. `Compositor` und `app` nutzen `add_module()` **nicht**
(eigene CMakeLists).

## Composition Root & optionale Module

Der `Compositor` ist der Composition Root — der einzige Ort, der Implementierungen
kennt und **PRIVATE** hinter `Application` versteckt. Er nutzt bewusst **kein**
`add_module` (kein `api/`, invertierte Link-Semantik: linkt Implementierungen statt
Verträge).

- **Pflicht-Subsysteme** direkt: `target_link_libraries(Compositor PRIVATE Datenbank)`.
- **Optionale Module** über den Helfer `compositor_linked(Compositor <Modul>)`
  (`cmake/Compositor.cmake`): `if(TARGET <Modul>)` → PRIVATE-Link + **PUBLIC**
  Compile-Definition `ENABLE_<MODUL>` (Großschreibung von `<Modul>`). PUBLIC ist nötig,
  damit `app/main.cpp` (inkludiert `Application.h`) denselben Makro-Stand sieht wie
  `Application.cpp` — sonst ODR-Verstoß durch unterschiedliches Klassenlayout.
- `ENABLE_LOGGER` / `ENABLE_F_OPCHECK` schalten die Module im Root per `option()`
  an/aus (Standard `ON`); `#ifdef ENABLE_*` in `Application.h`/`.cpp` steuern
  Member/Includes/Nutzung.
- **Guard:** `ENABLE_LOGGER=ON` ohne `ENABLE_F_OPCHECK=ON` bricht per
  `message(FATAL_ERROR …)` ab — dieser Guard liegt in **`code/Logger/CMakeLists.txt`**
  (wird nur erreicht, wenn `ENABLE_LOGGER=ON`), weil `Logger` einen `IFileWriter&`
  braucht und `FileWriter` die einzige Impl ist.
- **`option()`-Falle:** Der Default wirkt nur beim *ersten* Konfigurieren. Ein
  vorhandenes `build/` behält den gecachten Wert — zum Umschalten `-DENABLE_…=OFF`
  oder `build/` löschen.

## App-Ressourcen: `config/config.json`, `add_config()` / `add_log()`

`code/app/CMakeLists.txt` ruft nach `add_executable(Application …)` zwei
Ressourcen-Helfer auf (aus `cmake/AddConfig.cmake` / `cmake/AddLog.cmake`) — reine
INTERFACE-Libs, die Pfade als Compile-Definitions liefern:

- `add_config(TARGET Application)` → Target `Config`;
  `APP_CONFIG_PATH="<abs. Pfad zu config/config.json>"` (dazu `LOG_COMPILE_LEVEL`
  via `$<IF:$<CONFIG:Release>,2,0>`, in C++ derzeit ungenutzt). Kopiert
  `config/config.json` per POST_BUILD neben die Exe
  (`copy_if_different` — fehlt die Quelle, bricht der Build ab).
- `add_log()` (**kein** `TARGET`) → Target `Log`;
  `APP_LOG_FILE="<root>/log/app.log"` (`LOG_PATH = ${CMAKE_SOURCE_DIR}/log`, per
  `file(MAKE_DIRECTORY)` zur Konfigurationszeit angelegt). Die `app.log` im Quellbaum
  ist über `/log/` in `.gitignore` ausgenommen.

**Nutzung:** `main.cpp` nimmt `APP_CONFIG_PATH` als Pfad und lässt
`lade_konfiguration()` (Modul `Konfiguration`) die Datei via `nlohmann/json` parsen;
`uebernehme_argumente()` legt CLI-Argumente darüber. `WriteLog()` (Presets) hängt an
`APP_LOG_FILE` an. Beide Makros sind `#ifdef`-abgesichert — ohne `add_config`/`add_log`
fällt der Code auf No-Op zurück und baut trotzdem.

`config/config.json` ist eingecheckt (Laufzeit-Ressource); die Kopie unter
`build/code/app/` ist generiert.

## Fallstricke

- `if(TARGET …)` wird **sofort** ausgewertet (anders als `target_link_libraries`, das
  Namen erst zur Generate-Zeit auflöst). Deshalb müssen die Module, die der Compositor
  per `compositor_linked` / `if(TARGET …)` prüft (`Logger`, `F_OP_Check`), im Root
  **vor** `code/compositor` eingebunden sein — der Compositor steht zuletzt. Die
  Position von `libs`/`Konfiguration`/`Datenbank` untereinander ist dagegen egal
  (reine `target_link_libraries` lösen spät auf).
- `file(GLOB … CONFIGURE_DEPENDS)`: `add_module()` globt `src/*.cpp` (Logger,
  F_OPCheck, Konfiguration, Datenbank), `Compositor` globt selbst, und **`code/app`
  globt ebenfalls** alle `*.cpp` (`APP_SOURCES` → `main.cpp`, `Presets.cpp`,
  `ArgumenteAuswerten.cpp`). Neue `.cpp` → einmal neu konfigurieren.
- **Dritt-Bibliotheken unter `libs/`:** `libs/CMakeLists.txt` zieht `nlohmann/json`
  (v3.12.0) per `FetchContent` nach `libs/nlohmann-json/` (**nicht** eingecheckt,
  `.gitignore`; beim ersten Konfigurieren Download → Netzwerk nötig). Target
  `nlohmann_json` (Alias `nlohmann_json::nlohmann_json`). Genutzt vom **`Konfiguration`-Modul**
  (`PRIVATE_DEPENDS nlohmann_json`, json nur in `KonfigurationIO.cpp`) — leakt dadurch
  **nicht** zu `app`/`Compositor`.

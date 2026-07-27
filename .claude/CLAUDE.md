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

## Aktueller Build-Stand vs. geplante Konfigurations-Kaskade

**Wichtig vor jeder Konfigurations-Änderung:** Die Config-Kaskade ist **vollständig**
verdrahtet — `config.json`-Laden, CLI-Merge und die Injektion in `Application`
laufen. Offen ist nur noch das optionale `LaufzeitMenue` (Live-Editor). Der Prosa
trotzdem nicht blind vertrauen, erst den Ist-Zustand prüfen.

**Tatsächlich gebaut** (Root-`CMakeLists.txt`): `Logger`, `F_OP_Check`,
`Compositor`, `Konfiguration` (nur `Konfiguration_API`, der Vertrag-`struct`),
`app` (`Application.exe`) + `libs` (nlohmann_json). `app` kompiliert `main.cpp`,
**`KonfigurationIO.cpp` und `ArgumenteAuswerten.cpp`** (explizit in
`add_executable`, app globt nicht) und linkt `Konfiguration_API`. Ablauf:
`LoadPresets()` liest `config.json` rein anzeigend (UTF-8-Konsole, Begrüßung,
Sprache/LogLevel); dann baut `main(argc, argv)` die Kaskade auf — `struct`-Defaults
→ `lade_konfiguration(APP_CONFIG_PATH)` (dieselbe `config.json`) →
`uebernehme_argumente()` mit dem Ergebnis von `ArgumenteAuswerten` (**CLI hat
Vorrang**) — und gibt sie mit `print_konfiguration()` aus. Danach
`Application app{konfig};`: die Konfiguration wird per Konstruktor **injiziert**
(`Application` hält eine `const&`, Muster wie `Logger(IFileWriter&)`), und
`Application::run()` nutzt sie z. B. für ein `konfig_.verbose`-Zusatzlog.

**Genau ein `Setup::Konfiguration`-Typ:** der reine `struct`
(`code/Konfiguration/api/I_Konfiguration/Konfiguration.h`, JSON-/I/O-frei, für jede
Schicht sichtbar). Laden/Speichern/Merge sind **Frei-Funktionen** im app-Layer
(`code/app/KonfigurationIO.{h,cpp}`: `lade_konfiguration` / `speichere_konfiguration`
/ `uebernehme_argumente` / `print_konfiguration`); nlohmann wird **nur** in
`KonfigurationIO.cpp` eingebunden. Die früher konkurrierende app-`class`
`Konfiguration` wurde entfernt.

**Im Baum, aber NICHT im Build** (Root-`CMakeLists.txt` hat kein `add_subdirectory`
dafür bzw. `app/CMakeLists.txt` kompiliert sie nicht):
- Modul `code/Datenbank/` (`IDatenbank` + `InMemoryDatenbank`).
- app-Datei `LaufzeitMenue.{h,cpp}` (interaktives Menü). Nutzt bereits den `struct`
  + `speichere_konfiguration()`, ist aber noch nicht in `main.cpp` eingehängt.

**Noch offen (optional):** `LaufzeitMenue` als Live-Editor an `main.cpp` anschließen
(ändert die Konfiguration interaktiv und schreibt via `speichere_konfiguration()`
zurück). `config/config.json` bleibt dabei nur an der app-Grenze angefasst, untere
Schichten nie.

**Abhängigkeiten (gebaut):** `Application`(exe) → `Compositor` → optional
`F_OPCheck` + `Logger`; `Konfiguration_API` (Vertrag) linken sowohl `app` (für
`KonfigurationIO`) als auch `Compositor` (PRIVATE, `Application.cpp` liest die
Felder). `Logger` nimmt im Konstruktor ein `IFileWriter&` (aus `F_OPCheck`), daher
ist `ENABLE_LOGGER=ON` mit `ENABLE_F_OPCHECK=OFF` ein `FATAL_ERROR` — Logger hat
sonst keine `IFileWriter`-Implementierung.

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
- `add_config(TARGET <exe>)` — INTERFACE-Target `Config`; setzt das
  `APP_CONFIG_PATH`-Define und `LOG_COMPILE_LEVEL` (2 Release, sonst 0), kopiert
  `config/config.json` per POST_BUILD neben die Exe. (`AddConfig.cmake`)
- `add_log(TARGET <exe>)` — INTERFACE-Target `Log`; setzt `APP_LOG_FILE` auf
  `$<TARGET_FILE_DIR:<exe>>/app.log`, also die Logdatei **neben der Exe** (Generator-
  Expression, erst zur Build-Zeit aufgelöst; kein Verzeichnis wird angelegt).
  (`AddLog.cmake`)

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
  liefert nur `APP_CONFIG_PATH`/Dateikopie) ist **nicht** das Konfigurations-Modul
  `Konfiguration`/`Konfiguration_API` (der Einstellungs-`struct`).

## Neues Modul hinzufügen

1. `code/<Name>/` mit `api/`, `include/<Name>/`, `src/` anlegen.
2. `api/CMakeLists.txt`: `add_interface(NAME <Name> [DEPENDS <Fremde>_API])`.
3. `code/<Name>/CMakeLists.txt`: `add_module(NAME <Name> DEPENDS <Fremde>_API)`.
4. Im Root `add_subdirectory(code/<Name>)` **vor** `code/compositor` einhängen
   (ggf. mit eigenem `option()`-Schalter + Guard im Compositor).
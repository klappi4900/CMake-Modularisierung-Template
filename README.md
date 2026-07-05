# CMake-Modularisierung-Template

Lernprojekt zum Thema CMake-Modularisierung: wie man ein C++-Projekt in
eigenständige Module aufteilt, die sauber gegeneinander abgegrenzt sind
(Vertrag/API vs. Implementierung) und optional an- und abschaltbar bleiben.
Kein Produktivcode, keine Tests, keine CI.

## Build

Voraussetzung: CMake ≥ 3.20, Ninja, MSVC-Toolchain (C++23).

```powershell
cmake -S . -B build -G Ninja
cmake --build build
build\code\app\Application.exe
```

Einzelnes Target bauen: `cmake --build build --target <Logger|F_OPCheck|Compositor|Application>`.

## Architektur

Jedes Modul unter `code/` folgt derselben Struktur:

```
code/<Modul>/
├── api/            INTERFACE-Lib "<Modul>_API" – nur Header, der Vertrag
├── include/<Modul>/  öffentliche Header der Implementierung
├── src/            .cpp-Dateien, nur intern sichtbar
└── CMakeLists.txt
```

Kernregel: **Module linken nur gegen fremde `_API`-Targets, nie gegen fremde
Implementierungen.** So bleibt die Abhängigkeit auf den Vertrag beschränkt,
nicht auf Interna eines anderen Moduls.

Komposition (welche konkreten Typen zusammenarbeiten) passiert ausschließlich
im `Compositor`-Modul (`Application.cpp`). `app/main.cpp` kennt nur
`Application`, keine Implementierungen von Logger/F_OPCheck.

### Module im Überblick

| Modul | bindet ein |
|---|---|
| `F_OPCheck` | `F_OPCheck_API` (eigener Vertrag) |
| `Logger` | `Logger_API` (eigener Vertrag) + `F_OPCheck_API` (Konstruktor nimmt `IFileWriter&`) |
| `Compositor` | optional `F_OPCheck` und `Logger` (Implementierungen, siehe unten) |
| `app` (`Application`-Executable) | `Compositor` |

## `add_module()` – Helferfunktion für Modul-Boilerplate

Definiert in `cmake/AddModule.cmake`, im Root per `include(cmake/AddModule.cmake)`
eingebunden. Jedes Modul ruft sie in seiner `CMakeLists.txt` einmal auf, statt
`add_library`/`target_include_directories`/`target_link_libraries` von Hand zu
wiederholen:

```cmake
add_module(NAME Logger DEPENDS F_OPCheck_API)
```

Das übernimmt pro Modul: `add_subdirectory(api)`, Sourcen aus `src/*.cpp`
globben, `add_library(<NAME> STATIC ...)`, `include/` PUBLIC + `src/` PRIVATE,
sowie `target_link_libraries(<NAME> PUBLIC <NAME>_API <DEPENDS>)`.

## Optionale Module: `ENABLE_LOGGER` / `ENABLE_F_OPCHECK`

Im Root-`CMakeLists.txt` steuern zwei `option()`-Schalter, ob `Logger` bzw.
`F_OP_Check` überhaupt Teil des Builds sind:

```powershell
cmake -S . -B build -DENABLE_LOGGER=OFF -DENABLE_F_OPCHECK=OFF
```

Der Mechanismus dahinter, in der Reihenfolge, in der er greift:

1. **Reihenfolge im Root:** `add_subdirectory(code/Logger)` /
   `add_subdirectory(code/F_OP_Check)` laufen *vor*
   `add_subdirectory(code/compositor)`. Das ist notwendig, weil `if(TARGET ...)`
   sofort beim Erreichen der Zeile ausgewertet wird (anders als
   `target_link_libraries`, das Namen erst am Ende der Konfiguration auflöst) –
   ohne diese Reihenfolge wüsste Compositor nie, ob die Targets existieren.
2. **Guard in `code/compositor/CMakeLists.txt`:** `if(TARGET F_OPCheck)` /
   `if(TARGET Logger)` linken das jeweilige Modul nur, wenn sein Target
   tatsächlich erzeugt wurde (Option war `ON`).
3. **Compile-Definition statt nur Link-Guard:** Derselbe `if(TARGET ...)`-Block
   setzt zusätzlich `target_compile_definitions(Compositor PUBLIC ENABLE_F_OPCHECK)`
   bzw. `ENABLE_LOGGER`. **PUBLIC** ist hier notwendig: `app/main.cpp` inkludiert
   `Application.h` und muss denselben Makro-Stand sehen wie `Application.cpp` –
   sonst hätten beide Übersetzungseinheiten ein unterschiedliches Klassenlayout
   (fehlender/vorhandener Member), was ein ODR-Verstoß wäre.
4. **`#ifdef` in `Application.h`/`.cpp`:** Forward-Declarations, Member
   (`fileWriter_`, `logger_`), Includes (`FileWriter.h`, `Logger.h`) und deren
   Verwendung in Konstruktor/`run()`/`stop()` sind jeweils mit
   `#ifdef ENABLE_F_OPCHECK` / `#ifdef ENABLE_LOGGER` umschlossen. Ist ein
   Modul deaktiviert, verschwindet die Abhängigkeit auch aus dem Code, nicht
   nur aus dem Link-Schritt.
5. **Validierung:** `ENABLE_LOGGER=ON` bei `ENABLE_F_OPCHECK=OFF` bricht die
   Konfiguration mit `message(FATAL_ERROR ...)` ab, weil `Logger` einen
   `IFileWriter&` im Konstruktor braucht und `FileWriter` (aus `F_OP_Check`)
   die einzige Implementierung davon in diesem Projekt ist.

## Neues Modul hinzufügen

1. Verzeichnis `code/<Name>/` mit `api/`, `include/<Name>/`, `src/` anlegen.
2. `api/CMakeLists.txt`: INTERFACE-Lib wie bei den bestehenden Modulen.
3. `code/<Name>/CMakeLists.txt`: `add_module(NAME <Name> DEPENDS <Fremde>_API)`.
4. Im Root-`CMakeLists.txt` `add_subdirectory(code/<Name>)` vor
   `code/compositor` einhängen (ggf. mit eigenem `option()`-Schalter, siehe oben).

## Fallstricke

- `file(GLOB ... CONFIGURE_DEPENDS)` für Sources wird in `add_module()`
  verwendet – neue `.cpp`-Dateien werden dadurch automatisch erkannt, ein
  erneutes Konfigurieren reicht.
- `if(TARGET ...)` wird sofort ausgewertet, nicht erst am Ende der
  Konfiguration – Reihenfolge der `add_subdirectory()`-Aufrufe ist bei
  Guards relevant (siehe oben).

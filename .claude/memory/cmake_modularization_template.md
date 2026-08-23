---
name: cmake-modularization-template
description: "Bevorzugtes Architektur-Template des Users für modulare C++/CMake-Projekte (Vertrag/API vs. Implementierung, Composition Root, optionale/austauschbare Module) — anwenden, wenn der User ein neues Projekt \"in meinem üblichen Stil\" aufsetzen möchte."
metadata: 
  node_type: memory
  type: user
  originSessionId: 3632a776-1015-40ab-9ac4-da6b4e48b26f
---

Referenzimplementierung: `C:\MEGA\Programmieren Test\C++\CMake-Modularisierung-Template`. Wenn der User bei einem neuen (auch anderen) Projekt sagt, er wolle seine übliche Projektstruktur, dieses Template anwenden.

## Grundprinzip

Jedes fachliche Modul trennt strikt **Vertrag** (API, nur Header, INTERFACE-Lib) von **Implementierung** (STATIC-Lib). Module linken ausschließlich gegen fremde `_API`-Targets, nie gegen fremde Implementierungen.

## Verzeichnisstruktur pro Modul

```
code/<Modul>/
├── api/                    INTERFACE-Lib "<Modul>_API" — nur Header (Interfaces, Prefix "I")
│   └── I_<Modul>.h/
├── include/<Modul>.h        öffentliche Header der Implementierung
├── src/<Modul>.cpp            .cpp-Dateien, nur intern sichtbar
└── CMakeLists.txt
```
Wichtig ist das auch das der Ordnernamen bereits im ersten Buchstaben grossgeschrieben wird auch die zugehörigen Dateien .h 
und .cpp werden genauso geschrieben.
Nicht jedes Modul braucht zwingend ein eigenes `api/` — nur wenn ein anderes Modul gegen seinen Vertrag linken soll. Ein reiner Composition-Root (s.u.) braucht z. B. kein `api/`, solange niemand ihn als Vertrag konsumiert, sondern ihn nur als fertiges Ausführungsziel nutzt. Ein Exe-Target (z. B. `app`) kann auch ganz ohne `api/`/`include/`/`src/`-Unterordner flach bleiben (nur `main.cpp` + `CMakeLists.txt`).

## Helferfunktion `add_module()`

Zentral in `cmake/AddModule.cmake`:

```cmake
function(add_module)
    cmake_parse_arguments(ARG "" "NAME" "DEPENDS" ${ARGN})
    add_subdirectory(api)
    file(GLOB SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
    add_library(${ARG_NAME} STATIC ${SOURCES})
    target_include_directories(${ARG_NAME}
        PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    target_link_libraries(${ARG_NAME} PUBLIC ${ARG_NAME}_API ${ARG_DEPENDS})
endfunction()
```

Jedes "normale" Modul ruft in seiner eigenen `CMakeLists.txt` nur `add_module(NAME <Name> DEPENDS <Fremde>_API ...)` auf. Das deckt **feste** (unbedingte) Abhängigkeiten auf fremde APIs ab — z. B. wenn ein Modul im Konstruktor ein fremdes Interface referenziert (wie `Logger` → `F_OPCheck_API`, weil der Logger-Konstruktor ein `IFileWriter&` nimmt).

## Composition Root

Genau ein Modul (z. B. `Compositor`) kennt und instanziiert konkrete Typen aus mehreren Implementierungsmodulen. Regeln:

- Nur die `.cpp` des Composition-Root darf konkrete Klassen instanziieren; sein Header (`Application.h` o. ä.) verwendet nur Forward-Decls + `std::unique_ptr<IFoo>`.
- Composition Root linkt Implementierungen **PRIVATE**.
- Das ausführbare Target (`app/main.cpp`) kennt **ausschließlich** den Composition Root, keine der einzelnen Fach-Module direkt.
- Composition Root nutzt **nicht** `add_module()`, weil es die Guard-Sonderlogik unten braucht — eigene `CMakeLists.txt`.

## Austauschbare/optionale Module (Plugin-artig)

Für Module, die ersetzbar oder ein-/abschaltbar sein sollen (z. B. eine austauschbare GUI-Implementierung):

1. Modul bekommt eigene `api/` mit Interface (`I<Modul>`), damit die Implementierung austauschbar bleibt.
2. Root-`CMakeLists.txt`: `option(ENABLE_<NAME> "..." ON)`, steuert per `if(ENABLE_<NAME>) add_subdirectory(code/<Name>) endif()`.
3. Reihenfolge im Root beachten: alle optionalen Module **vor** dem Composition Root einhängen — `if(TARGET ...)` wertet sofort beim Erreichen der Zeile aus, nicht erst am Ende der Konfiguration (anders als `target_link_libraries`).
4. Composition-Root-`CMakeLists.txt`: pro optionalem Modul ein Guard-Block:
   ```cmake
   if(TARGET <Name>)
       target_link_libraries(${MODULE_NAME} PRIVATE <Name>)
       target_compile_definitions(${MODULE_NAME} PUBLIC ENABLE_<NAME>)
   endif()
   ```
   Die Compile-Definition muss **PUBLIC** sein, wenn sie `#ifdef`-Blöcke steuert, die das Klassenlayout des Composition-Root-Headers verändern (Member je nach Modul vorhanden/fehlend) — sonst ODR-Verstoß, weil `main.cpp` und die Composition-Root-`.cpp` unterschiedliche Makro-Stände sähen.
5. Harte Abhängigkeiten zwischen optionalen Modulen (z. B. Modul B braucht zwingend Modul A) per `message(FATAL_ERROR ...)` im Root validieren, wenn `ENABLE_B AND NOT ENABLE_A`.

## Naming & Konventionen

- Interfaces: Prefix `I` (`IFileWriter`, `ILogger`), reine `struct` mit virtuellem Destruktor + pure-virtual Methoden.
- Include-Pfad-Prefix folgt dem Ordnernamen unter `include/`/`api/`, nicht dem Library-Namen (`#include "Logger/Logger.h"`, `#include "I_API_Logger/ILogger.h"`).
- API-Ordnername: `I_<Modul>` oder `I_API_<Modul>`.

## Bekannte Fallstricke

- `if(TARGET ...)` wertet sofort aus → Reihenfolge der `add_subdirectory()`-Aufrufe entscheidet.
- `file(GLOB ... CONFIGURE_DEPENDS)` erkennt neue Dateien erst nach erneutem Konfigurieren.
- Exe-Targets, die Sourcen nicht globben, sondern hart auflisten (`add_executable(X main.cpp)`): neue `.cpp`-Dateien werden nur gebaut, wenn explizit ergänzt oder direkt per `#include` eingebunden (dann müssen deren Funktionen `inline` sein).
- PUBLIC vs. PRIVATE bei `target_link_libraries` immer anhand Sichtbarkeit im Header entscheiden: taucht der fremde Typ in einem öffentlichen Header auf → PUBLIC, nur in `.cpp` → PRIVATE.

## Wann dieses Template NICHT 1:1 übernehmen

- Wenn ein neues Modul nur eine **feste**, nicht austauschbare/nicht optionale Abhängigkeit auf einen fremden Vertrag hat, reicht `add_module(... DEPENDS Fremde_API)` — keine Guards nötig.
- `add_module()` nicht vorschnell um optionale/guarded Dependencies verallgemeinern, bevor nicht mindestens zwei konkrete Module denselben Optional-Guard-Bedarf haben — sonst spekulative Abstraktion (YAGNI). Bestätigt in [[planned-gui-module]]: dort wurde bewusst abgewartet statt vorab zu verallgemeinern.

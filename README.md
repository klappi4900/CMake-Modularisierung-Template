# PLS_01

Ein **Prozessleitsystem**, abgeleitet aus einer Analyse von Emerson DeltaV Control
Studio. Der Entwurf liegt vollständig in [`docs/`](docs/); umgesetzt sind **Etappe 0**
(Fundament: Schichtgrenzen und Kerntypen) und ein erster Schnitt aus **E1/E2**:
`BlockType` und `Variable` hinter `IBlockRepository`, mit einer In-Memory- und einer
SQLite-Implementierung.

Unter `code/` liegen zwei Gruppen nebeneinander, die sich nicht vermischen:

| | Verzeichnisse | Helfer |
|---|---|---|
| **PLS-Modulgruppen** | `core`, `model/…`, `exec/…`, `lang`, `build/…` | `pls_add_module()`, Layer-Guard |
| ** CMake-Modularisierung** | `Logger`, `F_OP_Check`, `Konfiguration`, `Datenbank`, `Compositor`, `app` | `add_module()`, `add_interface()` |

Das Projekt zeigt, wie man ein C++-Projekt in gegeneinander abgegrenzte Module
aufteilt (Vertrag/API vs. Implementierung, optional an- und abschaltbar). Es bleibt
als lauffähige Referenz stehen — Details unter
[Zwei Modulsysteme](#zwei-modulsysteme).

## Build

Voraussetzung: CMake ≥ 3.20, Ninja, MSVC-Toolchain (C++23).

```powershell
.\ci.ps1              # konfigurieren + bauen + ctest
.\ci.ps1 -Clean       # zusätzlich build/ löschen
```

Das Skript sucht `vcvars64.bat` selbst und startet sich in der Developer-Umgebung neu.
Von Hand geht es genauso, dann aber **aus einer Developer-Shell** — sonst findet schon
der Compiler-Test die Windows-SDK-Libs nicht:

```powershell
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
build\code\app\Application.exe
```

Einzelnes Target:
`cmake --build build --target <Logger|F_OPCheck|Konfiguration|Datenbank|Compositor|Application|pls_core|pls_model_domain|pls_model_persistence|pls_demo>`.
Einzelner Test: `ctest --test-dir build -R core.Ulid`; `-N` listet alle.

Beim ersten Konfigurieren lädt `third_party/CMakeLists.txt` `nlohmann/json` (v3.12.0),
`googletest` (v1.17.0) und die **SQLite-Amalgamation** (v3.50.4, mit `URL_HASH`
festgenagelt) per `FetchContent` — dafür ist eine Netzwerkverbindung nötig. Keines der
Verzeichnisse ist eingecheckt.

SQLite bringt kein CMake-Projekt mit; `third_party/CMakeLists.txt` baut das Target
`sqlite3` selbst aus der einen `sqlite3.c`. Deshalb steht dort ein `enable_language(C)`
— ohne das kennt dieses C++-Projekt keinen C-Compiler.

## Die PLS-Modulgruppen

Der Modulschnitt aus Entscheidungsstand § 10 — dort unter `libs/` geführt, hier unter
`code/` abgelegt. Die noch leeren Gruppen bauen trotzdem als Targets:

```
code/
├─ core/                 Ulid, Id<Tag>, Signal<T>          ← umgesetzt
├─ model/domain/         BlockType, Variable,              ← teilweise (E1.1, 1.5, 1.6, 1.8)
│                        IBlockRepository, InMemory…
└─ model/persistence/    SQLite-Schema + Repository        ← teilweise (E2.1, 2.3)
                         XML-Import/Export                    (E2.4–2.8, blockiert auf O-3)
```

Die übrigen vier Gruppen des Modulschnitts — `lang/` (E7), `build/translation/` (E4),
`exec/runtime/` (E5), `exec/fieldbus/` (E6) — sind **noch nicht angelegt**. Ihre
Schichtnamen und erlaubten Kanten stehen aber schon in `cmake/LayerGuard.cmake`, damit
die erste Kante beim Anlegen sofort geprüft wird und nicht erst nachträglich.

Ein Modul trennt Vertrag und Implementierung wie die älteren Module unter `code/`:

```
code/<gruppe>/<submodul>/
├─ api/pls/<gruppe>/<submodul>/        Vertrag  → pls_<name>_api (INTERFACE)
├─ include/pls/<gruppe>/<submodul>/    Impl-Header
├─ src/*.cpp                           → pls_<name> (STATIC), linkt _api PUBLIC
├─ tests/*.cpp                         GoogleTest
└─ CMakeLists.txt                      pls_add_module(NAME <schicht> DEPENDS …)
```

`api/`, `include/` und `src/` hat **jedes** Modul, auch wenn eines davon leer bleibt —
dann hält eine `.gitkeep` das Verzeichnis in Git (so bei `code/core/api/`: Wertetypen
haben keine Interface-Fassade). `pls_add_module()` bricht ab, wenn eines fehlt. Die
Struktur steht damit fest, statt beim ersten Header neu verhandelt zu werden.

Beide Include-Wurzeln liefern denselben Prefix — `#include "pls/core/Ulid.h"` sagt
nicht, ob der Header aus `api/` oder `include/` kommt. Ein Header kann also von
`api/` nach `include/` wandern, ohne dass jemand seine `#include`-Zeile ändert.

`code/build/` wäre eine Modulgruppe, nicht das Build-Verzeichnis — `.gitignore` trifft
mit `/build/` nur das Root. Solange die Gruppe nicht angelegt ist, ist `build/` im
Projektwurzelverzeichnis das CMake-Verzeichnis und sonst nichts.

### Layer-Guard

Die Schichtgrenzen sind nicht dokumentiert, sondern **erzwungen**. `cmake/LayerGuard.cmake`
prüft jede Abhängigkeit, die über `pls_link()` läuft, gegen **zwei Regeln**. Ein Verstoß
bricht die *Konfiguration* ab, nicht erst den Build.

**Regel 1 — Schichtgrenze.** Welche Gruppe darf an welcher hängen (§ 10):

```
Layer-Verstoß: 'exec_runtime' darf nicht gegen 'model_domain' linken.

   Target       : pls_exec_runtime   (code/exec/runtime, Schicht exec_runtime)
   Abhängigkeit : pls::model_domain  (code/model/domain, Schicht model_domain)
   Gezogen in   : code/exec/runtime/CMakeLists.txt

   Erlaubt für 'exec_runtime': core

   Die Runtime führt ein flaches Image aus und kennt weder Typen noch Composites
   noch Pfade. Sie hat einen EIGENEN NodeKind; die Abbildung
   PrimitiveKind -> NodeKind liegt im Flattener (code/build/translation).
```

**Regel 2 — Vertrag statt Implementierung.** Auch bei erlaubter Schicht hängt ein Modul
am `_api`-Target des anderen, nie an dessen Interna:

```
API-Verstoß: 'pls_model_persistence' linkt gegen eine fremde IMPLEMENTIERUNG.

   Abhängigkeit : pls_model_domain   (Schicht model_domain, Implementierung)

   Statt dessen den Vertrag linken:  pls_model_domain_api
```

Zwei Ausnahmen: die Schicht `compositor` (der Composition Root kennt die konkreten
Typen — das ist seine Aufgabe) und mit `VALUE_TYPES` markierte Module. `code/core` ist
so eines: `Ulid` und `Signal` sind Wertetypen ohne Interface-Fassade, ihre Symbole
liegen in der STATIC-Lib. Hinter `pls::core_api` allein gäbe es nichts zu linken.
`code/model/domain` ebenso — `toString(Section)`, `parseSection()` und
`DomainError::message()` sind Symbole, keine virtuellen Aufrufe.

Geprüft wird **zweimal**: sofort beim `pls_link()`, und noch einmal durch
`pls_verify_layers()` am Ende der Root-`CMakeLists.txt`. Der zweite Durchgang ist der
wichtigere — zeigt eine Kante auf ein Target, das erst später definiert wird, findet
die sofortige Prüfung nichts. Genau so rutschte `exec_runtime → model_domain`
anfangs durch, weil `code/exec` vor `code/model` eingebunden wird. Sechs ctest-Fälle
unter `tests/` halten beide Regeln fest.

### Anschauung: `pls_demo`

```powershell
cmake --build build --target pls_demo
build\demo\pls_demo.exe
```

Führt die Kerntypen vor: ULIDs erzeugen und über ihre **Textform** sortieren (das
Ergebnis ist die Erzeugungsreihenfolge), die Phantom-Typen, und ein simuliertes
Signal, das durch fünf Rechenstufen läuft, ohne sein `[SIM]` zu verlieren. Dann der
Fall, an dem Constant Folding Stufe 3 scheitert: `MUL(x, 0)` liefert `0.00` mit
Quality `Bad` — ein defekter Messwert lässt sich nicht durch Multiplikation mit Null
reinwaschen.

Abschnitt 4 legt einen `Nand`-Baustein **zweimal** an: einmal in einer `std::map`,
einmal in `pls_demo.db` neben der Exe. Dieselben Aufrufe, nur eine andere Zeile davor.
Danach wird der Typ umbenannt, die Datei geschlossen und neu geöffnet — der Name ist
neu, die ULID dieselbe. Die Datei bleibt liegen; jeder SQLite-Browser zeigt, was darin
steht.

Das Programm gehört keiner Schicht an und ist kein Teil des PLS. Was es zeigt, prüfen
die Tests unter `code/*/tests/` — nur ohne Ausgabe.

### Kerntypen in `code/core`

| Typ | Zusage |
|---|---|
| `Ulid` | 48 bit Zeit + 80 bit Zufall, monoton innerhalb einer Millisekunde. **Lexikografische Sortierung = zeitliche** — auf den Bytes wie auf der 26-stelligen Textform |
| `Id<Tag>` | Phantom-Typ über `Ulid`. `BlockTypeId` und `VariableId` sind zur Compile-Zeit unverträglich und kosten kein Byte |
| `Signal<T>` | Wert **und** Status untrennbar. `sizeof(Signal<float>) == 8`, acht Signale je Cachezeile |
| `worst()` | Die schlechtere Quality gewinnt samt Substatus; `Simulated`/`Forced` sind klebrig und überleben beliebig viele Rechenstufen |

Die untere Hälfte von `SignalQuality` ist layoutgleich zum FOUNDATION-Fieldbus-
Statusbyte (Quality 7–6 / Substatus 5–2 / Limit 1–0); die klebrigen Bits liegen
darüber, weil sie dort keine Entsprechung haben.

### Persistenz: `IBlockRepository` und zwei Implementierungen

Der Vertrag liegt in `code/model/domain/api/` — beim **Konsumenten**, nicht bei der
Persistenz (Dependency Inversion). Er kennt keinen Dateipfad, keine Transaktion und
kein SQL:

| Wo | Was |
|---|---|
| `model/domain/api/…/IBlockRepository.h` | der Vertrag: `BlockType`, `Variable`, CRUD, Umbenennen |
| `model/domain/include/…/InMemoryBlockRepository.h` | `std::map`-Fassung (E1.6) |
| `model/persistence/api/…/SqliteRepository.h` | zwei Fabrikfunktionen, Rückgabe `IBlockRepository` |
| `model/persistence/include/…/SqliteBlockRepository.h` | die SQLite-Fassung (E2.3), Verbindung hinter pimpl |

`sqlite3` ist **PRIVATE** verlinkt und taucht in keinem Header auf. Außerhalb von
`code/model/persistence/src/` hat niemand SQLite im Include-Pfad — die Voraussetzung
dafür, dass ein Wechsel der Persistenz wirklich nur dort stattfindet.

**Umbenennen ist ein `UPDATE`, niemals `DELETE`+`INSERT`** (Entscheidungsstand § 2 [E]).
Bei `DELETE`+`INSERT` wechselte die ULID; die Zustandsmigration beim Online-Download
fände den alten Wert nicht wieder, meldete aber nichts, sondern initialisierte eine
scheinbar neue Variable. Ein Reglerintegral stünde danach auf Null. Deshalb ist das
eine eigene Operation im Vertrag und kein `remove` + `add`.

Was in der Datenbank steht, ist das Schema aus § 3.1 — `Variable` als **einziges**
Deklarationskonstrukt, unterschieden über `section`. Ein Baustein ist eine Variable
mit `section = 'VAR'`, ein Eingang eine mit `'VAR_INPUT'`. Die Tabellen sind `STRICT`,
die ULIDs stehen als 26-stelliger Text mit `CHECK (length(uid) = 26)`, und
`PRAGMA foreign_keys = ON` wird je Verbindung gesetzt **und nachgeprüft** — ohne das
täte `ON DELETE CASCADE` schweigend nichts.

Die Schemaversion steht in `PRAGMA user_version`. Eine Datei aus einer *neueren*
Programmversion wird abgelehnt statt geöffnet: unbekannte Spalten zu übergehen hieße,
sie beim nächsten Speichern zu verlieren.

### Die Testsuite, die dreimal läuft

`code/model/persistence/tests/RepositoryContractTest.cpp` stellt jede Frage an alle
drei Speicher — `std::map`, SQLite `:memory:`, SQLite in einer Datei:

```powershell
ctest --test-dir build -R RepositoryContract
```

Das ist das Abnahmekriterium von Arbeitsplan E2.3 („Demo läuft unverändert, nur
Repository getauscht") in ausführbarer Form. Auch die Fehlerfälle gehören dazu: ein
doppelter Name muss überall `DuplicateName` heißen, nicht einmal so und einmal
`UNIQUE constraint failed` — deshalb übersetzt `SqliteDb::lastError()` die
SQLite-Constraint-Codes in die fachliche Ursache.

Lässt sich ein Test nur für einen der drei formulieren, ist entweder der Vertrag zu
vage oder eine Implementierung weicht ab. Was wirklich nur für SQLite gilt — das
Überleben eines Neustarts, die Schemaversion — steht getrennt in
`SqliteRepositoryTest.cpp`.

## Das Projekt zur CMake-Modularisierung

Vier **Blatt-Module**, der **Compositor** (Composition Root) und **app** (Executable),
ebenfalls unter `code/`. Jedes Blatt-Modul trennt strikt Vertrag von Implementierung:

```
code/<Modul>/
├── api/I_<Modul>/    Vertrag: INTERFACE-Lib "<Modul>_API" – nur Header
├── api/CMakeLists.txt
├── include/          öffentliche Header der Implementierung
├── src/              .cpp-Dateien, nur intern sichtbar
└── CMakeLists.txt
```

**Include-Prefix:** Verträge tragen das Verzeichnis unter `api/` im Pfad
(`#include "I_Konfiguration/Konfiguration.h"`), Impl-Header nicht
(`#include "Logger.h"`, `#include "InMemoryDatenbank.h"`). Keine `../`-Relativ-Includes
— der Vertrag ist über die `_API`-Include-Dir erreichbar.

Zwei Kernregeln:

1. **Blatt-Module linken nur gegen fremde `_API`-Targets, nie gegen fremde
   Implementierungen.** `Logger` linkt `F_OPCheck_API` (weil `Logger.h` im Konstruktor
   einen `IFileWriter&` führt), aber **nicht** `F_OPCheck`.
2. **Komposition passiert ausschließlich im `Compositor`** (`Application.cpp`). Nur
   dort sind `FileWriter`, `Logger` und `InMemoryDatenbank` als konkrete Typen bekannt;
   `Application.h` nutzt Forward-Decls und `std::unique_ptr` auf die Interfaces.

### Module im Überblick

| Modul | linkt PUBLIC | linkt PRIVATE |
|---|---|---|
| `F_OPCheck` | `F_OPCheck_API` (eigener Vertrag) | — |
| `Logger` | `Logger_API`, `F_OPCheck_API` | — |
| `Konfiguration` | `Konfiguration_API` | `nlohmann_json` (json nur im `.cpp`) |
| `Datenbank` | `Datenbank_API` | — |
| `Compositor` | `Konfiguration_API` | `Datenbank`, optional `Logger`/`F_OPCheck` |
| `app` (`Application.exe`) | — | `Compositor`, `Konfiguration`, `Config`, `Log` |

`app/main.cpp` baut aus `config/config.json` und den CLI-Argumenten *eine* effektive
`Setup::Konfiguration` und injiziert sie in `Application`. Die konkreten Modul-Impls
(Logger, Datenbank, …) kennt `app` nicht — die zieht der Compositor.

**PUBLIC vs. PRIVATE** folgt der Sichtbarkeit in den Headern: Typ im öffentlichen
Header → PUBLIC, nur im `.cpp` → PRIVATE.

## Zwei Modulsysteme

Beide Gruppen liegen unter `code/` und folgen demselben Muster — Vertrag und
Implementierung getrennt, fremde Module linken nur den Vertrag. Sie nutzen aber
getrennte Helfer, weil sich Details unterscheiden:

| | Projekt                               | PLS-Modulgruppen |
|---|---------------------------------------|---|
| Helfer | `add_module()`, `add_interface()`     | `pls_add_module()` |
| Vertrag | `api/I_<Modul>/` → `<Modul>_API`      | `api/pls/<gruppe>/<submodul>/` → `pls_<name>_api` |
| Impl-Header | `include/` ohne Prefix                | `include/pls/<gruppe>/<submodul>/` |
| Include | `"I_Logger/ILogger.h"` / `"Logger.h"` | `"pls/core/Ulid.h"` — für beide Wurzeln gleich |
| Link | `target_link_libraries` direkt        | `pls_link()` — durch den Layer-Guard |
| Targets | `Logger`, `Logger_API`                | `pls_core`, `pls_core_api` (Aliase `pls::…`) |
| Tests | keine                                 | GoogleTest, `pls_add_test()` |

Der Unterschied, der bleibt: Beim PLS **erzwingt** der Layer-Guard, was im
Projekt Konvention ist. Dort steht „Module linken nur gegen fremde `_API`" in
der Dokumentation; hier bricht `pls_link()` die Konfiguration ab.

## Die CMake-Helfer

Alle in `cmake/`, im Root per `include(...)` eingebunden. Sie ersetzen die
Boilerplate aus `add_library` / `target_include_directories` / `target_link_libraries`.

| Helfer | Datei | Legt an |
|---|---|---|
| `add_interface(NAME <N> [DEPENDS …])` | `AddInterface.cmake` | INTERFACE-Lib `<N>_API`, Include-Dir = `api/` |
| `add_module(NAME <N> [DEPENDS …] [PRIVATE_DEPENDS …])` | `AddModule.cmake` | STATIC-Lib `<N>` + `add_subdirectory(api)` |
| `compositor_linked(<Target> <Modul>)` | `Compositor.cmake` | PRIVATE-Link + Define `ENABLE_<MODUL>`, falls das Target existiert |
| `add_config(TARGET <exe>)` | `AddConfig.cmake` | Target `Config` mit `APP_CONFIG_PATH` |
| `add_log()` | `AddLog.cmake` | Target `Log` mit `APP_LOG_FILE` |
| `pls_link(<T> <PUBLIC…> <deps>)` | `LayerGuard.cmake` | geprüfter Ersatz für `target_link_libraries` |
| `pls_verify_layers()` | `LayerGuard.cmake` | zweiter Durchgang, ans **Ende** der Root-CMakeLists |
| `pls_add_module(NAME <schicht> …)` | `PlsModule.cmake` | `pls_<schicht>` **und** `pls_<schicht>_api`, je mit `pls::`-Alias |
| `pls_add_test(MODULE <schicht>)` | `PlsTest.cmake` | Testbinary + `gtest_discover_tests()` |

```cmake
add_module(NAME Logger DEPENDS F_OPCheck_API)
add_module(NAME Konfiguration PRIVATE_DEPENDS nlohmann_json)
```

`add_module()` globt `src/*.cpp` mit `CONFIGURE_DEPENDS`, setzt `include/` PUBLIC und
`src/` PRIVATE und linkt `<N>_API` plus `DEPENDS` PUBLIC, `PRIVATE_DEPENDS` PRIVATE.
`DEPENDS` heißt: Der Typ taucht in einem öffentlichen Header auf.

Der **Compositor nutzt `add_module()` bewusst nicht** — er hat kein `api/` und
invertierte Link-Semantik (linkt Implementierungen statt Verträge) und bringt daher
seine eigene `CMakeLists.txt` mit.

### App-Ressourcen: `add_config()` / `add_log()`

Reine INTERFACE-Libs, die Pfade als Compile-Definitions liefern:

- `add_config(TARGET Application)` → `APP_CONFIG_PATH` (absoluter Pfad zu
  `config/config.json`) und `LOG_COMPILE_LEVEL`. Kopiert `config.json` per POST_BUILD
  neben die Exe.
- `add_log()` — **ohne** `TARGET` → `APP_LOG_FILE` = `<root>/log/app.log`; das
  Verzeichnis wird zur Konfigurationszeit angelegt.

Beide Makros sind im C++-Code `#ifdef`-abgesichert: ohne die Helfer fällt er auf
No-Op zurück und baut trotzdem.

## Optionale Module: `ENABLE_LOGGER` / `ENABLE_F_OPCHECK`

Im Root steuern zwei `option()`-Schalter, ob `Logger` bzw. `F_OP_Check` überhaupt Teil
des Builds sind (Standard `ON`):

```powershell
cmake -S . -B build -DENABLE_LOGGER=OFF -DENABLE_F_OPCHECK=OFF
```

Der Mechanismus, in der Reihenfolge, in der er greift:

1. **Reihenfolge im Root:** `add_subdirectory(code/Logger)` und
   `code/F_OP_Check` laufen *vor* `code/Compositor`. Notwendig, weil `if(TARGET ...)`
   sofort beim Erreichen der Zeile ausgewertet wird (anders als
   `target_link_libraries`, das Namen erst zur Generate-Zeit auflöst) — ohne diese
   Reihenfolge wüsste der Compositor nie, ob die Targets existieren.
2. **Guard in `compositor_linked()`:** linkt das Modul nur, wenn sein Target
   tatsächlich erzeugt wurde.
3. **Compile-Definition statt nur Link-Guard:** derselbe Helfer setzt
   `target_compile_definitions(Compositor PUBLIC ENABLE_<MODUL>)`. **PUBLIC** ist
   notwendig: `app/main.cpp` inkludiert `Application.h` und muss denselben Makro-Stand
   sehen wie `Application.cpp` — sonst hätten beide Übersetzungseinheiten ein
   unterschiedliches Klassenlayout (fehlender/vorhandener Member), also einen
   ODR-Verstoß.
4. **`#ifdef` in `Application.h`/`.cpp`:** Forward-Decls, Member, Includes und deren
   Verwendung in Konstruktor/`run()`/`stop()` sind umschlossen. Ist ein Modul
   deaktiviert, verschwindet die Abhängigkeit auch aus dem Code, nicht nur aus dem
   Link-Schritt.
5. **Validierung in `code/Logger/CMakeLists.txt`:** `ENABLE_LOGGER=ON` bei
   `ENABLE_F_OPCHECK=OFF` bricht mit `message(FATAL_ERROR ...)` ab, weil `Logger` einen
   `IFileWriter&` im Konstruktor braucht und `FileWriter` (aus `F_OP_Check`) die
   einzige Implementierung davon ist. Der Guard liegt im Logger-Modul, weil er nur
   erreicht wird, wenn `ENABLE_LOGGER=ON` ist.

`Datenbank` ist dagegen **Pflicht-Subsystem** und wird direkt gelinkt. Das
`option(ENABLE_Datenbank …)` im Root wird nirgends ausgewertet —
`-DENABLE_Datenbank=OFF` hat keine Wirkung.

## Neues PLS-Modul hinzufügen

1. `code/<gruppe>/<submodul>/` anlegen, darin **alle drei**:
   `api/pls/<gruppe>/<submodul>/` (Vertrag), `include/pls/<gruppe>/<submodul>/`
   (Impl-Header) und `src/`. Was noch leer bleibt, bekommt eine `.gitkeep` —
   `pls_add_module()` besteht auf allen drei.
2. `CMakeLists.txt`: `pls_add_module(NAME <schicht> DEPENDS pls::<andere>_api)` —
   auf das **`_api`**-Target der anderen Module, nicht auf deren Implementierung.
   Liefert das Modul Wertetypen statt einer Interface-Fassade, kommt `VALUE_TYPES`
   dazu (wie bei `core`).
3. Ist `<schicht>` neu, in `cmake/LayerGuard.cmake` eintragen: in `PLS_LAYERS` **und**
   als `PLS_LAYER_ALLOW_<schicht>`-Liste. Eine neue Kante zwischen bestehenden
   Schichten ist eine Änderung am Modulschnitt — die gehört zuerst in
   `docs/pls_entscheidungsstand.md`, nicht in die Kantenliste.
4. Tests: `tests/CMakeLists.txt` mit `pls_add_test(MODULE <schicht>)`, daneben die
   `*.cpp`. `pls_add_module()` hängt `tests/` selbst ein.

## Neues Modul im Projekt hinzufügen

1. Verzeichnis `code/<Name>/` mit `api/I_<Name>/` (Vertrag), `include/` (Impl-Header)
   und `src/` anlegen.
2. `code/<Name>/api/CMakeLists.txt`: `add_interface(NAME <Name>)`. Braucht ein
   Vertragsheader einen fremden Typ: `add_interface(NAME <Name> DEPENDS <Fremde>_API)`.
3. `code/<Name>/CMakeLists.txt`:
   `add_module(NAME <Name> [DEPENDS <Fremde>_API] [PRIVATE_DEPENDS <lib>])`.
4. Im Root `add_subdirectory(code/<Name>)` einhängen — **vor** `code/Compositor` nur
   dann nötig, wenn der Compositor das Modul per `if(TARGET ...)` prüfen soll.
   Optional mit eigenem `option()`-Schalter.

## Fallstricke

- **`if(TARGET ...)` wird sofort ausgewertet**, nicht erst am Ende der Konfiguration.
  Die Reihenfolge der `add_subdirectory()`-Aufrufe ist bei Guards relevant. Tatsächliche
  Reihenfolge im Root: `Logger`, `F_OP_Check`, `libs`, `Konfiguration`, `Datenbank`,
  `app`, `Compositor`. Dass `app` vor dem Compositor steht, obwohl es dagegen linkt,
  trägt — `target_link_libraries` löst spät auf.
- **`option()`-Defaults wirken nur beim ersten Konfigurieren.** Ein vorhandenes
  `build/` behält den gecachten Wert; zum Umschalten `-DENABLE_…=OFF` übergeben oder
  `build/` löschen.
- **`file(GLOB ... CONFIGURE_DEPENDS)`** wird von `add_module()`, vom Compositor und
  von `code/app` genutzt. Neue `.cpp` werden erkannt, aber erst nach einem erneuten
  Konfigurieren.

## Der PLS-Entwurf in `docs/`

| Datei | Inhalt |
|---|---|
| `pls_entscheidungsstand.md` | Revision 2, 13 Abschnitte — **normativ** |
| `pls_arbeitsplan.md` | Etappen E0–E11 mit Abnahmekriterien |
| `pls_diskussionsverlauf.md` | 16 Themenblöcke: die Herleitung |
| `deltav_control_studio_featureliste.md` | Quellenbasis DeltaV, 17 Abschnitte |
| `control_studio_bloecke.md` | Keimzelle: Doppelklick → `TypeBody`-Variant |
| `flyweight_demo.cpp` | lauffähiger Prototyp — **Stand Revision 1** |

Der Entscheidungsstand führt Statusmarker: `[E]` entschieden · `[V]` vorläufig ·
`[O]` offen · `[!]` Widerspruch. Die Featureliste hat eine eigene Skala mit anderer
Bedeutung (`[D]` dokumentiert · `[B]` beobachtbar · `[V]` vermutet).

`flyweight_demo.cpp` belegt das Flyweight-Prinzip — zwei `Nand`-Instanzen teilen sich
dieselbe DB-Zeile, bekommen aber getrennte Laufzeitidentität — ist aber der Prototyp zu
Revision **1** und widerspricht Revision 2 an mehreren Stellen (`int64_t`-IDs statt
ULID, `BlockPort`/`BoundaryPort`-Variant, Exceptions statt `std::expected`). Nicht als
Referenz für neuen Code lesen; Details in `.claude/CLAUDE.md`.

Einzeln übersetzbar:

```powershell
cl /std:c++latest /EHsc docs\flyweight_demo.cpp
```

### Stand der Umsetzung

| Etappe | Inhalt | Stand |
|---|---|---|
| **E0** | Fundament: Schichtgrenzen, `Ulid`, `Id<Tag>`, `Signal<T>`, ctest | **fertig** |
| E1 | Domänenmodell | **teilweise** — 1.1, 1.5, 1.6, 1.8 stehen; 1.2 und 1.7 sind frei, 1.3/1.4 brauchen O-1 (`VAR_IN_OUT`) |
| E2 | Persistenz und XML | **teilweise** — 2.1 und 2.3 stehen; 2.2 ist frei, 2.4–2.8 brauchen O-3 (XSD) |
| E4 | Flattener, Layout, Image | offen — braucht O-4 (Binärformat) |
| E7 | Sprache (ST) | offen — braucht O-2 (dynamische Indizes) |

`ctest` zählt derzeit **146 Tests, alle grün** (einer übersprungen: die Schema-Migration
gibt es noch nicht).

Die vier blockierenden Entscheidungen stehen in `pls_entscheidungsstand.md` § 11.1.
Für E0 brauchte es keine davon, und auch die bisher umgesetzten Schritte aus E1/E2
kamen ohne aus — die Deklaration hängt nicht an `VAR_IN_OUT`, die SQLite-Schritte
nicht an der XSD.

# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Projektkontext

Das Repository ist ein **Prozessleitsystem (PLS)**, abgeleitet aus einer Analyse von
Emerson DeltaV Control Studio. Der Entwurf liegt vollständig in `docs/`; umgesetzt sind
**Etappe 0** (Fundament) und ein erster Schnitt aus **E1/E2**: `BlockType` und
`Variable` hinter `IBlockRepository`, mit In-Memory- und SQLite-Implementierung
(E1.1, E1.5, E1.6, E1.8 · E2.1, E2.3). Siehe „Persistenz" weiter unten.

Unter `code/` liegen zwei Gruppen nebeneinander, die **nicht zu verwechseln** sind:

| | Verzeichnisse unter `code/` | Helfer |
|---|---|---|
| **PLS-Modulgruppen** | `core`, `model/domain`, `model/persistence` | `pls_add_module()`, Layer-Guard |
| **CMake-Modularisierung** | `Logger`, `F_OP_Check`, `Konfiguration`, `Datenbank`, `Compositor`, `app` | `add_module()`, `add_interface()` |

**Nur diese drei PLS-Module existieren im Arbeitsbaum.** `lang`, `build/translation`,
`exec/runtime` und `exec/fieldbus` stehen im Modulschnitt (§ 10) und mit Schichtnamen
samt erlaubten Kanten in `cmake/LayerGuard.cmake` — als Verzeichnis gibt es sie
**nicht**. Im Zweifel gilt der Quellbaum, nicht `build/`.

Das Projekt bleibt vorerst als lauffähige Referenz für die CMake-Muster stehen;
über eine Ablösung wird bei E1/E2 entschieden. Die beiden Systeme haben getrennte
Helfer und Konventionen — siehe „Zwei Modulsysteme" weiter unten.

**Pfadabweichung zur Doku:** Arbeitsplan und Entscheidungsstand führen die
Modulgruppen unter `libs/` (`libs/model/domain`, `libs/exec/runtime`, …). In diesem
Repo liegen sie unter `code/`; `libs/` existiert nicht mehr, Dritt-Bibliotheken stehen
in `third_party/`. Gemeint sind dieselben Gruppen und dieselben Schichtgrenzen.

| Datei in `docs/` | Rolle | Verbindlichkeit |
|---|---|---|
| `pls_entscheidungsstand.md` | Rev. 2, 13 Abschnitte | **normativ** |
| `pls_arbeitsplan.md` | Etappen E0–E11 mit Abnahmekriterien | Reihenfolge |
| `pls_diskussionsverlauf.md` | 16 Themenblöcke, Herleitung | Begründung |
| `deltav_control_studio_featureliste.md` | Quellenbasis DeltaV, 17 Abschnitte | Faktenlage |
| `control_studio_bloecke.md` | Keimzelle: Doppelklick → `TypeBody`-Variant | Notiz |
| `flyweight_demo.cpp` | lauffähiger Prototyp | **Rev. 1 — siehe unten** |

**Statusmarker im Entscheidungsstand:** `[E]` entschieden · `[V]` vorläufig ·
`[O]` offen · `[!]` Widerspruch/Risiko. **`[E]` nicht ohne ausdrückliche Rücksprache
umwerfen.** Die Dokumente korrigieren sich selbst explizit (Entscheidungsstand § 13,
Diskussionsverlauf § 16) — eine Abweichung gehört dorthin, nicht stillschweigend in den
Code. Die Featureliste führt eine **eigene** Skala mit anderer Bedeutung
(`[D]` dokumentiert · `[B]` beobachtbar · `[V]` vermutet); nicht verwechseln.

Die fünf Entscheidungen mit der größten Codewirkung, damit sie nicht erst beim Lesen
von 37 KB auffallen:

- **ULID durchgängig** statt `INTEGER PRIMARY KEY AUTOINCREMENT`. Umbenennen ist
  `UPDATE`, **nie** `DELETE`+`INSERT` — ein ULID-Wechsel bricht die Zustandsmigration
  beim Online-Download stillschweigend.
- **`Variable` als einziges Deklarationskonstrukt.** `BlockTypePort` und `Block`
  verschmelzen zu einer Tabelle, unterschieden über `section` (IEC-61131-3-Weg).
- **Referenzsemantik statt Verschaltungstabelle.** Der Flattener löst Verbindungen zu
  Indizes auf; der Konsument liest, wo der Produzent schrieb. Kein `memcpy` je Kante.
- **Wert und Status als untrennbarer Typ** `Signal<T>` (AoS, kein Zeitstempel).
  `Simulated`/`Forced` sind klebrige Quality-Bits.
- **`code/exec/` hängt niemals an `code/model/`** — auf CMake-Ebene zu erzwingen,
  nicht nur zu dokumentieren.

## Build

CMake-Projekt `PLS_01`, C++23, Ninja-Generator, Build-Verzeichnis `build/`. Aus einer
**MSVC-Developer-Umgebung** heraus konfigurieren (Developer-Shell / `vcvars64.bat`),
sonst findet schon der Compiler-Test die Windows-SDK-Libs nicht:

```powershell
.\ci.ps1                 # konfigurieren + bauen + ctest, sucht vcvars selbst
.\ci.ps1 -Clean          # zusätzlich build/ löschen (erzwingt FetchContent-Download)
```

Oder von Hand, aus einer Developer-Shell:

```powershell
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
build\code\app\Application.exe
```

Einzelnes Target: `cmake --build build --target <Logger|F_OPCheck|Konfiguration|Datenbank|Compositor|Application|pls_core|pls_model_domain|pls_model_persistence|pls_test_core|pls_test_model_domain|pls_test_model_persistence|pls_demo>`.

`build\demo\pls_demo.exe` führt die Kerntypen und die Persistenz vor (Quelle:
`demo/main.cpp`) — nützlich, um eine Änderung an `Ulid`/`Signal`/`IBlockRepository`
sichtbar zu machen. Abschnitt 4 legt `pls_demo.db` **neben der Exe** an (also unter
`build/demo/`, damit von `.gitignore` erfasst) und öffnet sie ein zweites Mal. Kein
Teil des PLS, gehört keiner Schicht an.

Einzelner Test: `ctest --test-dir build -R core.Ulid` (Regex auf den Testnamen);
`ctest --test-dir build -N` listet alle. Die Testnamen tragen das Modul als Präfix
(`core.Ulid.RoundTripUeberTextform`), vergeben von `gtest_discover_tests()`. Die
Vertragssuite trägt zusätzlich den Speicher im Namen:
`model_persistence.JedeImplementierung/RepositoryContract.<Test>/<InMemoryMap|SqliteMemory|SqliteFile>`
— `ctest -R RepositoryContract` lässt alle drei laufen. Insgesamt sind es **146 Tests**;
einer wird übersprungen, weil es noch keine Schema-Migration zu prüfen gibt.

Kein Linter. Die CI ist `ci.ps1`; ein GitHub-Workflow existiert bewusst nicht,
solange kein Remote konfiguriert ist.

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

**Include-Prefix:** Verträge tragen das Verzeichnis unter `api/` im Pfad
(`#include "I_Konfiguration/Konfiguration.h"`, `#include "I_Datenbank/IDatenbank.h"`,
`#include "I_Logger/ILogger.h"`). Impl-Header **nicht** — `include/` ist selbst die
Include-Dir, es gibt darunter keinen Modul-Unterordner: `#include "Logger.h"`,
`#include "InMemoryDatenbank.h"`, `#include "FileWriter.h"`. **Keine
`../`-Relativ-Includes** — der Vertrag ist über die `_API`-Include-Dir erreichbar.

## Neues Modul hinzufügen

1. `code/<Name>/` mit `api/I_<Name>/` (Vertrag), `include/` (Impl-Header), `src/`.
2. `api/CMakeLists.txt`: `add_interface(NAME <Name>)` (aus `cmake/AddInterface.cmake`;
   legt `<Name>_API` an, Include-Dir = `api/`). Braucht ein Vertragsheader einen
   fremden Typ: `add_interface(NAME <Name> DEPENDS <Fremde>_API)`.
3. `code/<Name>/CMakeLists.txt`:
   `add_module(NAME <Name> [DEPENDS <Fremde>_API] [PRIVATE_DEPENDS <lib>])`
   (aus `cmake/AddModule.cmake`). `DEPENDS` → PUBLIC (Typ im öffentlichen Header),
   `PRIVATE_DEPENDS` → PRIVATE (nur im `.cpp`, z. B. `nlohmann_json`).
4. Root-`CMakeLists.txt`: `add_subdirectory(code/<Name>)` einhängen. **Vor**
   `code/Compositor` ist nur nötig, wenn der Compositor das Modul per `if(TARGET …)`
   prüft (gilt für `Logger` und `F_OP_Check`). Optional `option(ENABLE_<NAME> …)`.

Tatsächliche Reihenfolge im Root: `code/Logger`, `code/F_OP_Check`, `third_party`,
`code/core`, `code/model`, `code/Konfiguration`, `code/Datenbank`, `code/app`,
`code/Compositor`, `demo`, `tests` — und ganz am Ende `pls_verify_layers()`. `app` steht
**vor** dem Compositor, obwohl es dagegen linkt — das trägt, weil
`target_link_libraries` Namen erst zur Generate-Zeit auflöst.

**Den Vertrag** (Interface/struct, den andere brauchen) in `api/I_<Name>/`,
**Impl-Header** in `include/`. `Compositor` und `app` nutzen `add_module()` **nicht**
(eigene CMakeLists).

## Composition Root & optionale Module

Der `Compositor` ist der Composition Root — der einzige Ort, der Implementierungen
kennt und **PRIVATE** hinter `Application` versteckt. Er nutzt bewusst **kein**
`add_module` (kein `api/`, invertierte Link-Semantik: linkt Implementierungen statt
Verträge).

- **Pflicht-Subsysteme** direkt: `target_link_libraries(Compositor PRIVATE Datenbank)`.
  Im Root steht zwar ein `option(ENABLE_Datenbank …)`, es wird aber **nirgends
  ausgewertet** — `add_subdirectory(code/Datenbank)` läuft unbedingt, der Compositor
  behandelt `Datenbank` als Pflicht. `-DENABLE_Datenbank=OFF` hat keine Wirkung.
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
  **vor** `code/Compositor` eingebunden sein — der Compositor steht zuletzt. Die
  Position von `libs`/`Konfiguration`/`Datenbank` untereinander ist dagegen egal
  (reine `target_link_libraries` lösen spät auf).
- `file(GLOB … CONFIGURE_DEPENDS)`: `add_module()` globt `src/*.cpp` (Logger,
  F_OPCheck, Konfiguration, Datenbank), `Compositor` globt selbst, und **`code/app`
  globt ebenfalls** alle `*.cpp` (`APP_SOURCES` → `main.cpp`, `Presets.cpp`,
  `ArgumenteAuswerten.cpp`). Neue `.cpp` → einmal neu konfigurieren.
- **Dritt-Bibliotheken unter `third_party/`** (nicht `libs/` — das trägt die
  PLS-Module): `third_party/CMakeLists.txt` zieht `nlohmann/json` (v3.12.0),
  `googletest` (v1.17.0) und die **SQLite-Amalgamation** (v3.50.4) per `FetchContent`
  (**nicht** eingecheckt, `.gitignore`; beim ersten Konfigurieren Download → Netzwerk
  nötig). `nlohmann_json` nutzt das **`Konfiguration`-Modul** (`PRIVATE_DEPENDS`, json
  nur in `KonfigurationIO.cpp`) — leakt dadurch **nicht** zu `app`/`Compositor`.
  GoogleTest wird nur bei `BUILD_TESTING=ON` geholt.
- **SQLite bringt kein CMake-Projekt mit.** Die Amalgamation ist eine einzelne
  `sqlite3.c`; das Target `sqlite3` wird in `third_party/CMakeLists.txt` von Hand
  gebaut. Deshalb steht dort `enable_language(C)` — ohne das kennt dieses
  C++-Projekt keinen C-Compiler und der Build bricht bei `sqlite3.c` ab. Der Download
  trägt ein `URL_HASH`: bei einer Datenbank, deren Dateiformat die Projekte trägt,
  gehört die Version festgenagelt. Include-Dir ist `SYSTEM`, Warnungen sind mit `/w`
  abgeschaltet.
- **`code/build/` wäre ein Modul, nicht das Build-Verzeichnis.** Der Name kommt aus
  dem Modulschnitt (§ 10, `code/build/translation`); `.gitignore` enthält `/build/`
  mit führendem Slash und trifft daher nur das Root-Verzeichnis. Zurzeit existiert
  `code/build/` **nicht** — es bestand nur aus `CMakeLists.txt`, `Modul.h` und zwei
  `.gitkeep` und ist leicht versehentlich mitzulöschen. Sinkt die Kantenzahl aus
  `pls_verify_layers()` unerwartet, fehlt eine ganze Modulgruppe.
- **Verzeichnisse unter `build/` belegen nicht, dass ein Modul existiert.** Eine
  gelöschte Modulgruppe hinterlässt ihr Build-Verzeichnis, bis jemand neu konfiguriert
  — `.\ci.ps1 -Clean` räumt das auf.
- **Der Vertrag führt weiter, was seine `api/`-Header brauchen.** `pls_add_module()`
  reicht von `DEPENDS` nicht nur Targets mit `_api`-Suffix an `pls_<n>_api` weiter,
  sondern auch **VALUE_TYPES**-Module. Nur deshalb trägt `pls_model_domain_api` die
  Include-Dir von `pls::core`, ohne die sein `BlockType.h` (`#include "pls/core/Id.h"`)
  nicht übersetzt. Wird die Bedingung wieder auf das Suffix verengt, merkt es erst der
  Erste, der ausschließlich gegen ein `_api`-Target linkt — vorgesehen für `lang` und
  `build_translation` —, und zwar als Include-Fehler weit weg von der Ursache.
  `tests/api_forwards_value_types/` hält den Fall fest.

## `docs/flyweight_demo.cpp` ist Rev.-1-Stand

Die Demo (627 Zeilen, C++23, einzeln übersetzbar) belegt das Flyweight-Prinzip:
zwei `Nand`-Instanzen teilen sich dieselbe DB-Zeile, bekommen aber getrennte
Laufzeitidentität (`X/A1` → RuntimeId 2, `Y/A1` → RuntimeId 4). Sie ist der Prototyp
zu **Revision 1** und widerspricht dem Entscheidungsstand Rev. 2 an mehreren Stellen —
teils an solchen, die Rev. 2 ausdrücklich abgeschafft hat. **Nicht als Referenz für
neuen Code lesen:**

| Demo | Rev. 2 fordert | Fundstelle |
|---|---|---|
| `Id<Tag>` über `std::int64_t` | ULID | § 2 `[E]`, E0.3/0.4 |
| `Endpoint = variant<BlockPort, BoundaryPort>` | entfällt, ersetzt durch `VarRef` | § 3.2 `[E]`, E1.4 |
| `PortDef` und `Block` getrennt | eine `Variable` mit `section` | § 3.1 `[E]`, E1.1 |
| `throw std::out_of_range` / `runtime_error` | `std::expected<T, DomainError>` | O-13, E1.8 |
| `instantiate()` rekursiert ungeprüft | Zyklusprüfung im Typgraph | O-12, E1.7 |
| `using NodeId = std::size_t` | `std::uint32_t` | § 5.5 `[E]` |
| Werte als `bool` | `Signal<T>` mit Quality | § 3.5 `[E]`, E0.5 |
| Kahn-Sortierung in `Engine::evaluate()` | Sortierung im Flattener `(level, kind, ulid)`, Runtime führt nur aus | § 6.4 `[E]` |

Tragfähig und zu erhalten ist dagegen ihr Aufbau:

- `runtime` hängt an **nichts** aus `domain` — eigener `NodeKind`, Abbildung
  `PrimitiveKind → NodeKind` liegt im Flattener (`mapKind`).
- `IBlockRepository` liegt beim Konsumenten, nicht bei der Persistenz
  (Dependency Inversion).
- **Zwei-Pass-Flattener:** `instantiate()` erzeugt Knoten + `SymbolMap`, `wire()` löst
  Eingänge auf. Pass 1 kann nicht verdrahten, weil eine Quelle in einem noch nicht
  expandierten Composite liegen kann. `resolveSource()` steigt bei Composite ab und
  bei Boundary auf; Composites hinterlassen zur Laufzeit nichts.
- Die Namespace-Kommentare im Dateikopf bilden bereits den Modulschnitt aus
  Entscheidungsstand § 10 ab.

Genau diese Eigenschaften nutzt der Arbeitsplan als Abnahmekriterium: E1.6
(„`flyweight_demo` läuft gegen das neue Modell") und E2.3 („Demo läuft unverändert,
nur Repository getauscht").

## Persistenz (E1.1/1.5/1.6/1.8 + E2.1/2.3)

Umgesetzt sind `BlockType` und `Variable` hinter `IBlockRepository`. **Nicht** dabei:
`TypeBody`-Variant (E1.2), `VarRef`/`Accessor` (E1.3), `Connection` (E1.4), Typgraph-
Zyklusprüfung (E1.7), Overrides (E2.2) und alles um XML (E2.4–2.8 — blockiert auf O-3).

| Datei | Rolle |
|---|---|
| `model/domain/api/…/IBlockRepository.h` | der Vertrag — kein Pfad, keine Transaktion, kein SQL |
| `model/domain/api/…/{BlockType,Variable,DomainError}.h` | die Entitäten, `Result<T> = std::expected<T, DomainError>` |
| `model/domain/include/…/InMemoryBlockRepository.h` | `std::map`-Fassung |
| `model/persistence/api/…/SqliteRepository.h` | `openSqliteRepository(path)` / `…InMemory()` → `IBlockRepository` |
| `model/persistence/include/…/SqliteBlockRepository.h` | die Klasse, Verbindung hinter pimpl |
| `model/persistence/src/{SqliteDb,Schema}.{h,cpp}` | RAII um die C-API, DDL + `user_version` |

Regeln, die der Code durchhält und die bei Änderungen zu wahren sind:

- **`sqlite3` ist PRIVATE und taucht in keinem Header auf.** Weder im Vertrag noch in
  `include/`. Nur `code/model/persistence/src/` und das *Testziel* sehen `sqlite3.h`.
  Bricht das, ist E2.3 („der Wechsel berührt nur `persistence/` und `compositor/`")
  nicht mehr nachweisbar.
- **`renameBlockType`/`renameVariable` sind `UPDATE`, nie `DELETE`+`INSERT`** (§ 2 [E]).
  Nicht Kosmetik: ein ULID-Wechsel bricht die Zustandsmigration beim Online-Download
  **stillschweigend** — die Migration sieht nur eine neue Variable ohne Vorgänger.
  Deshalb eigene Operationen im Vertrag statt `remove` + `add`.
- **Fehler sind Rückgabewerte, keine Exceptions** (O-13, E1.8). `DomainError` trägt
  `code` + `detail`; verglichen wird **nur** der Code, damit kein Test die Formulierung
  einer Meldung festschreibt.
- **`SqliteDb::lastError()` übersetzt die SQLite-Constraint-Codes** in
  `DuplicateId`/`DuplicateName`/`OwnerNotFound`/`InvalidValue`. Ohne das hieße ein
  doppelter Name in der einen Implementierung `DuplicateName` und in der anderen
  `StorageFailure` — und die gemeinsame Testsuite bräuchte Fallunterscheidungen.
- **`PRAGMA foreign_keys = ON` wird je Verbindung gesetzt UND nachgeprüft** (§ 3.3 [E]).
  Es ist keine Eigenschaft der Datei. Ohne das täte `ON DELETE CASCADE` nichts, ohne
  Fehlermeldung.
- **Tabellen sind `STRICT`**, ULIDs stehen als 26-stelliger Text mit
  `CHECK (length(uid) = 26)`. Die Nil-ULID käme daran vorbei (26 Nullen) und wird
  deshalb in C++ vorab abgefangen.
- **`std::optional<std::string>` bindet als NULL, nicht als Leerstring.** `address`
  NULL heißt „keine vergeben", `''` heißt „ausdrücklich leer".
- **Sortierung: `blockTypes()` nach `uid`** (= zeitlich, ohne `created_at`-Spalte),
  **`variablesOf()` nach `(ordinal, uid)`**. Die ULID als Nachrang ist Pflicht, sonst
  ist bei gleicher `ordinal` die Reihenfolge dem Speicher überlassen und der Export
  nicht reproduzierbar (E2.6/2.7).
- **Schemaversion in `PRAGMA user_version`** (`SchemaVersion` im Vertrag). Eine Datei
  aus einer *neueren* Version wird abgelehnt, nicht geöffnet.

**Die Testsuite läuft dreimal.** `model/persistence/tests/RepositoryContractTest.cpp`
stellt jede Frage an `std::map`, SQLite `:memory:` und SQLite-in-Datei. Ein neuer
Repository-Test gehört dorthin, nicht in eine implementierungsspezifische Datei — lässt
er sich nur für einen der drei formulieren, ist der Vertrag zu vage oder eine
Implementierung weicht ab. Nur was wirklich SQLite-spezifisch ist (Neustart,
`user_version`, Öffnungsfehler) steht in `SqliteRepositoryTest.cpp`; dieses Testziel
linkt zusätzlich `sqlite3`, weil es die Datei direkt aufmacht.

**`model_domain` ist `VALUE_TYPES`.** `toString(Section)`, `parseSection()` und
`DomainError::message()` sind Symbole in der STATIC-Lib, keine virtuellen Aufrufe —
`pls::model_domain_api` allein linkt sie nicht. Deshalb hängt `model_persistence` an
`pls::model_domain` und nicht am `_api`-Target; der API-Guard lässt das wegen der
Markierung durch. `lang` und `build_translation` linken weiterhin nur
`pls::model_domain_api`, weil sie (noch) keine dieser Funktionen brauchen.

## Zwei Modulsysteme

Beide Gruppen liegen unter `code/` und folgen demselben Muster — Vertrag und
Implementierung getrennt, fremde Module linken nur den Vertrag. Getrennte Helfer,
weil sich Details unterscheiden:

| | Projekt                              | PLS-Modulgruppen |
|---|---------------------------------------|---|
| Helfer | `add_module()`, `add_interface()`     | `pls_add_module()` |
| Vertrag | `api/I_<Modul>/` → `<Modul>_API`      | `api/pls/<gruppe>/<submodul>/` → `pls_<name>_api` |
| Impl-Header | `include/` ohne Prefix                | `include/pls/<gruppe>/<submodul>/` |
| Verzeichnisse | nach Bedarf                           | `api/`, `include/`, `src/` **immer**, notfalls mit `.gitkeep` |
| Include | `"I_Logger/ILogger.h"` / `"Logger.h"` | `"pls/core/Ulid.h"` — für beide Wurzeln gleich |
| Link | `target_link_libraries` direkt        | `pls_link()` — durch den Layer-Guard |
| Targets | `Logger`, `Logger_API`                | `pls_core`, `pls_core_api` (Aliase `pls::…`) |
| Tests | keine                                 | GoogleTest, `pls_add_test()` |

Die Spalte „Include" ist zugleich eine Regel: beide Wurzeln (`api/` und `include/`)
liefern denselben Prefix, damit ein Header von `api/` nach `include/` wandern kann,
ohne dass jemand seine `#include`-Zeile ändert. Daraus folgt, dass es **keine
`../`-Includes** gibt — weder zwischen Modulen noch innerhalb eines Moduls:
`grep -rn '#include "\.\.' code demo` muss leer bleiben. Ein `../`-Include greift an
der Include-Dir des Targets vorbei und damit an der Grenze, die der Layer-Guard
bewacht.

Der bleibende Unterschied: Beim PLS **erzwingt** der Guard, was im Projekt
Konvention ist. Dort steht „Module linken nur gegen fremde `_API`" in der Doku;
hier bricht `pls_link()` ab.

**Neues PLS-Modul anlegen** (Gegenstück zu „Neues Modul hinzufügen" oben, das nur fürs
Projekt gilt):

1. `code/<gruppe>/<submodul>/` mit `api/pls/<gruppe>/<submodul>/`,
   `include/pls/<gruppe>/<submodul>/` und `src/` — **alle drei Pflicht**, leere
   bekommen `.gitkeep`; `pls_add_module()` bricht sonst ab.
2. `CMakeLists.txt`: `pls_add_module(NAME <schicht> [VALUE_TYPES] [DEPENDS …]
   [PRIVATE_DEPENDS …])`. `NAME` ist zugleich die Schicht (sonst `LAYER` setzen) und
   muss in `PLS_LAYERS` stehen. `DEPENDS` zeigt auf `pls::<andere>_api`, nie auf
   `pls::<andere>` — Ausnahme sind `VALUE_TYPES`-Module.
3. Tests unter `tests/` mit einer `CMakeLists.txt`, die `pls_add_test(MODULE <name>
   [DEPENDS …])` ruft — `pls_add_module()` hängt `tests/` bei `BUILD_TESTING`
   selbsttätig ein, ein `add_subdirectory(tests)` gehört **nicht** dazu. Testziele
   laufen bewusst **nicht** durch den Layer-Guard und dürfen gegen alles linken
   (deshalb darf `pls_test_model_persistence` `sqlite3` sehen).
4. Im Root `add_subdirectory(code/<gruppe>)` einhängen — vor `pls_verify_layers()`
   am Dateiende.

## Layer-Guard (E0.2)

`cmake/LayerGuard.cmake` prüft jede über `pls_link()` gezogene Kante gegen **zwei
Regeln**. Ein Verstoß bricht die **Konfiguration** ab, nicht erst den Build.

**Regel 1 — Schichtgrenze** (`pls_check_edge`). Kantenliste aus § 10; wichtigster
Fall: `exec_*` darf nicht gegen `model_*`.

**Regel 2 — Vertrag statt Implementierung** (`pls_check_api`). Auch bei erlaubter
Schicht hängt ein Modul am `_api`-Target, nie an den Interna. Zwei Ausnahmen:

- Schicht `compositor` — der Composition Root kennt konkrete Typen, das ist sein Zweck.
- `VALUE_TYPES`-Module. `code/core` ist so eines: `Ulid` und `Signal` sind Wertetypen
  ohne Interface-Fassade, ihre Symbole liegen in der STATIC-Lib. Hinter
  `pls::core_api` allein gäbe es nichts zu linken. Wer ein Modul so markiert, sagt
  damit: „hier gibt es keine versteckbare Implementierung."

**Zwei Durchgänge, und der zweite ist der wichtigere:**

1. `pls_link()` prüft sofort — greift nur, wenn das Ziel-Target bereits existiert.
2. `pls_verify_layers()` am **Ende** der Root-`CMakeLists.txt` prüft alle gemerkten
   Kanten erneut, jetzt wo jedes Target definiert ist.

Ohne den zweiten Durchgang hinge die Wirksamkeit an der `add_subdirectory`-Reihenfolge:
`code/exec` kommt vor `code/model`, also war ausgerechnet `exec_runtime → model_domain`
ungeprüft — `pls_layer_of()` liefert für ein unbekanntes Target die leere Schicht und
`pls_check_edge()` gibt auf. Der Aufruf am Dateiende darf nicht verlorengehen;
`tests/layer_guard_late/` hält den Fall fest.

Eine Schicht hinzufügen heißt: Eintrag in `PLS_LAYERS` **und** eine
`PLS_LAYER_ALLOW_<schicht>`-Liste. Eine neue Kante ist eine Änderung am Modulschnitt
und gehört zuerst in den Entscheidungsstand.

**Der Schichtname folgt dem Verzeichnis** (`<gruppe>_<submodul>`): `core`,
`model_domain`, `model_persistence`, `lang`, `build_translation`, `exec_runtime`,
`exec_fieldbus`, `compositor`. `PLS_LAYERS` führt alle acht — auch die vier, deren
Verzeichnis noch fehlt, damit ihre erste Kante beim Anlegen sofort geprüft wird und
nicht erst nachträglich. Ein Name existiert also an zwei Stellen (`PLS_LAYERS` und
`PLS_LAYER_ALLOW_<name>`) und muss an beiden gleich lauten; die Guard-Tests decken das
nicht ab, weil `tests/layer_guard_*/` `PLS_LAYER` von Hand setzen und damit an
`pls_add_module()` vorbeilaufen.

## Offene Punkte

- **Blockierende Entscheidungen** laut Entscheidungsstand § 11.1: O-1 (`VAR_IN_OUT`)
  vor E1, O-3 (XSD) vor E2, O-4 (Image-Binärformat) vor E4, O-2 (dynamische Indizes)
  vor E7. E0 brauchte keine davon.
- **O-1 ist weiterhin offen**, obwohl Teile von E1 umgesetzt sind. Das ist bewusst:
  die Frage „`VAR_IN_OUT` ja oder nein" betrifft den **Flattener** (Referenzsemantik
  erzeugt Aliasing → `SymbolMap` nicht injektiv), nicht die Deklaration. Der Wert
  steht im `Section`-Enum und im `CHECK`-Constraint, weil § 3.1 ihn im Schema führt.
  Fällt die Entscheidung dagegen, verschwinden ein Enum-Wert und ein Constraint —
  nicht die Tabelle. **Vor E1.3/E1.4 (`VarRef`, `Connection`) muss O-1 entschieden
  sein**, dort wird es strukturell.
- **O-3 blockiert weiterhin E2.4–2.8** (XSD, XML-Import/Export, C14N, `build_hash`).
  Umgesetzt sind nur die SQLite-Schritte E2.1 und E2.3, die von der XSD nicht abhängen.
- **`worst()` weicht bewusst von der Skizze in § 7.3 ab:** dort wird nur die Quality
  maskiert, was Substatus und Limit verwirft. `code/core` übernimmt stattdessen die
  schlechtere Seite vollständig, damit die Ursache (`SensorFailure`) erhalten bleibt.
  Die vollständige Verknüpfungstabelle je Blocktyp ist offen (O-8).
- **Toter Verweis.** Beide PLS-Dokumente referenzieren `chat_deltav_flyweight.md`;
  die Datei existiert nicht. Inhaltlich abgedeckt durch `pls_diskussionsverlauf.md`,
  die thematische Zusammenfassung desselben Gesprächs.

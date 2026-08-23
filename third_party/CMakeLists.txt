# Dritt-Bibliotheken (Vendor-Dependencies) werden zentral hier eingebunden.
#
# Getrennt von code/, das die eigenen Module trägt — die PLS-Modulgruppen
# (code/Core, code/Model/…, code/exec/…) ebenso wie die älteren Module.
#
# Keine der Bibliotheken ist eingecheckt: der Quellcode wird beim Konfigurieren
# per FetchContent nach third_party/<name>/ heruntergeladen (die Ordner stehen
# deshalb in .gitignore). Erstes Konfigurieren braucht daher Netzwerk.
include(FetchContent)

# --- nlohmann/json ---------------------------------------------------------
# Danach existiert das INTERFACE-Target nlohmann_json
# (Alias nlohmann_json::nlohmann_json), gegen das jedes Modul linken kann:
#
#   target_link_libraries(<Modul> PRIVATE nlohmann_json)   # json nur in .cpp
#   target_link_libraries(<Modul> PUBLIC  nlohmann_json)   # json-Typ im Header
#
FetchContent_Declare(
        nlohmann_json
        URL        https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
        # Quellcode gezielt in diesen Ordner entpacken statt nach <build>/_deps:
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/nlohmann-json
        # Entpackte Dateien bekommen den aktuellen Zeitstempel (CMP0135 NEW-Verhalten).
        DOWNLOAD_EXTRACT_TIMESTAMP FALSE
)

# Tests/Beispiele der Bibliothek nicht mitbauen.
set(JSON_BuildTests OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(nlohmann_json)

# --- SQLite ----------------------------------------------------------------
# Die Projektdatenbank (Entscheidungsstand § 3.1, § 3.3, Arbeitsplan E2.1).
#
# Bezogen wird die "Amalgamation" von sqlite.org: die gesamte Bibliothek als
# eine einzige sqlite3.c. Sie bringt kein CMake-Projekt mit, deshalb wird das
# Target hier von Hand gebaut — FetchContent lädt und entpackt nur.
#
# Die Bibliothek ist C, nicht C++. Deshalb steht unten enable_language(C):
# ohne sie kennt das Projekt (C++23, siehe Root) keinen C-Compiler und der
# Build bricht bei sqlite3.c ab.
FetchContent_Declare(
        sqlite3
        URL          https://www.sqlite.org/2025/sqlite-amalgamation-3500400.zip
        # Ohne Hash zieht jeder Neubezug schweigend, was gerade unter der URL
        # liegt. Bei einer Datenbank, deren Dateiformat die Projekte trägt,
        # gehört die Version festgenagelt.
        URL_HASH     SHA256=1d3049dd0f830a025a53105fc79fd2ab9431aea99e137809d064d8ee8356b032
        SOURCE_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/sqlite3
        DOWNLOAD_EXTRACT_TIMESTAMP FALSE
)
FetchContent_MakeAvailable(sqlite3)

enable_language(C)

add_library(sqlite3 STATIC ${sqlite3_SOURCE_DIR}/sqlite3.c)
add_library(SQLite::SQLite3 ALIAS sqlite3)

target_include_directories(sqlite3 SYSTEM PUBLIC ${sqlite3_SOURCE_DIR})

target_compile_definitions(sqlite3 PRIVATE
        # Ohne dies akzeptiert SQLite "text" in doppelten Anführungszeichen als
        # String, sobald es keine Spalte dieses Namens findet. Ein Tippfehler im
        # Spaltennamen wird dann zum stillen Literal statt zum Fehler.
        SQLITE_DQS=0
        # Ein Prozess, mehrere Threads, je eigene Verbindung (Serialized).
        SQLITE_THREADSAFE=1
        # Fremdschlüssel sind Pflicht (§ 3.3 [E]) — der Default hilft, ersetzt
        # aber das PRAGMA je Verbindung nicht: es gilt nur für Verbindungen
        # dieses Prozesses, nicht für sqlite3.exe oder einen DB-Browser.
        SQLITE_DEFAULT_FOREIGN_KEYS=1
        SQLITE_OMIT_DEPRECATED
        SQLITE_DEFAULT_MEMSTATUS=0
)

# Fremder Code, den wir nicht ändern: seine Warnungen nicht in unsere Ausgabe
# mischen. SYSTEM oben tut dasselbe für alle, die den Header inkludieren.
if(MSVC)
    target_compile_options(sqlite3 PRIVATE /w)
else()
    target_compile_options(sqlite3 PRIVATE -w)
endif()

# --- GoogleTest + GoogleMock ----------------------------------------------
# Nur wenn BUILD_TESTING=ON (Standard, gesetzt von include(CTest) im Root).
# Liefert die Targets GTest::gtest, GTest::gtest_main, GTest::gmock,
# GTest::gmock_main. Registrierung einzelner Tests bei ctest übernimmt
# pls_test() aus cmake/PlsTest.cmake via gtest_discover_tests().
if(BUILD_TESTING)
    FetchContent_Declare(
            googletest
            URL        https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz
            SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/googletest
            DOWNLOAD_EXTRACT_TIMESTAMP FALSE
    )

    # Mit der CRT des Projekts bauen, nicht mit der von gtest voreingestellten
    # (sonst LNK2038 "RuntimeLibrary mismatch" unter MSVC).
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    # Eigene Installationsregeln und Tests von gtest selbst nicht mitziehen.
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)
endif()

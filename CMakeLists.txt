cmake_minimum_required(VERSION 3.20)
project(PLS_01)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Helfer des Projekts (code/) ---------------------------------------
include(cmake/AddModule.cmake)
include(cmake/AddInterface.cmake)
include(cmake/Compositor.cmake)   # Helfer compositor_linked() für den Composition Root
include(cmake/AddLog.cmake)
include(cmake/AddConfig.cmake)

# --- Helfer des PLS (code/<gruppe>/<submodul>/) ----------------------------
# LayerGuard MUSS vor PlsModule stehen: pls_add_module() ruft pls_link() auf
# und prüft LAYER gegen die dort definierte Liste PLS_LAYERS.
include(cmake/LayerGuard.cmake)   # pls_link() erzwingt den Modulschnitt (§ 10)
include(cmake/PlsModule.cmake)    # pls_add_module()
include(cmake/PlsTest.cmake)      # pls_add_test()

# Legt BUILD_TESTING an (Standard ON) und aktiviert ctest. Muss vor
# third_party stehen, das GoogleTest nur bei BUILD_TESTING holt.
include(CTest)
if(BUILD_TESTING)
    include(GoogleTest)           # gtest_discover_tests()
endif()


option(ENABLE_LOGGER    "Logger-Modul einbinden"     ON )
option(ENABLE_F_OPCHECK "F_OP_Check-Modul einbinden" ON )
option(ENABLE_Datenbank "Datenbank aktivieren"       ON )


# Wichtig: vor add_subdirectory(code/Compositor) einhängen, da Compositor per
# Guard: if(TARGET ...) prüft, ob diese Targets bereits existieren.

if(ENABLE_LOGGER)
    add_subdirectory(code/Logger)
endif()
if(ENABLE_F_OPCHECK)
    add_subdirectory(code/F_OP_Check)
endif()


# Dritt-Bibliotheken (nlohmann/json, GoogleTest) bereitstellen, bevor jemand
# dagegen linkt. Muss vor code/Konfiguration stehen (nlohmann_json PRIVATE)
# und vor den PLS-Modulgruppen (GoogleTest für deren Tests).
add_subdirectory(third_party)

# --- Die PLS-Modulgruppen ---------------------------------------------------
# Modulschnitt nach Entscheidungsstand § 10 (dort unter libs/ geführt, hier
# unter code/ abgelegt — siehe README). Jede Kante zwischen ihnen läuft über
# pls_link() und damit durch den Layer-Guard; die Reihenfolge hier ist nur
# topologisch gehalten, damit die Alias-Targets bereits existieren.
add_subdirectory(code/core)
add_subdirectory(code/model)
add_subdirectory(code/runtime)


# Konfigurations-Modul (Vertrag Konfiguration_API + Impl KonfigurationIO/LaufzeitMenue).
# Muss vor Compositor UND app stehen, da beide dagegen linken.
add_subdirectory(code/Konfiguration)
add_subdirectory(code/Datenbank)

add_subdirectory(code/app)
add_subdirectory(code/Compositor)

# Anschauungsprogramm für die Kerntypen (kein Teil des PLS).
add_subdirectory(demo)

# Tests, die nicht an einem einzelnen Modul hängen (Layer-Guard-Abnahme).
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# MUSS nach dem letzten add_subdirectory() stehen: prüft alle von pls_link()
# vorgemerkten Kanten, jetzt wo jedes Target existiert. Kanten auf ein zum
# Aufrufzeitpunkt noch undefiniertes Target werden ausschließlich hier erwischt.
pls_verify_layers()








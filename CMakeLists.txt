cmake_minimum_required(VERSION 3.20)
project(CMake_Modularisierung)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Fügt die Module zentral hinzu.
include(cmake/AddModule.cmake)
include(cmake/AddInterface.cmake)
include(cmake/Compositor.cmake)   # Helfer compositor_linked() für den Composition Root

include(cmake/AddLog.cmake)
include(cmake/AddConfig.cmake)


option(ENABLE_LOGGER    "Logger-Modul einbinden"     ON )
option(ENABLE_F_OPCHECK "F_OP_Check-Modul einbinden" ON )
option(ENABLE_Datenbank "Datenbank aktivieren"       ON )


# Wichtig: vor add_subdirectory(code/compositor) einhängen, da Compositor per
#Guard: if(TARGET ...) prüft, ob diese Targets bereits existieren.

if(ENABLE_LOGGER)
    add_subdirectory(code/Logger)
endif()
if(ENABLE_F_OPCHECK)
    add_subdirectory(code/F_OP_Check)
endif()

# Konfigurations-Vertrag (nur Header, keine Abhängigkeiten). Muss vor
# compositor UND app stehen, da beide gegen Konfiguration_API linken.
add_subdirectory(code/Konfiguration)
add_subdirectory(code/Datenbank)

# Dritt-Bibliotheken (nlohmann/json) bereitstellen, bevor Module dagegen linken.
add_subdirectory(libs)
#add_subdirectory(config)
#add_subdirectory(docs)
add_subdirectory(code/app)
add_subdirectory(code/compositor)








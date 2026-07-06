cmake_minimum_required(VERSION 3.20)
project(CMake_Modularisierung)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Fügt die Module zentral hinzu.
include(cmake/AddModule.cmake)

# Jedes Modul ist self-contained: enthält seine eigene api/-Sublib
# (INTERFACE, nur Header) und darüber die Implementierung.

option(ENABLE_LOGGER    "Logger-Modul einbinden"     ON )
option(ENABLE_F_OPCHECK "F_OP_Check-Modul einbinden" ON )

if(ENABLE_LOGGER AND NOT ENABLE_F_OPCHECK)
    message(FATAL_ERROR "ENABLE_LOGGER erfordert ENABLE_F_OPCHECK (Logger-Konstruktor braucht eine IFileWriter-Implementierung).")
endif()

# Wichtig: vor add_subdirectory(code/compositor) einhängen, da Compositor per
# if(TARGET ...) prüft, ob diese Targets bereits existieren.
if(ENABLE_LOGGER)
    add_subdirectory(code/Logger)
endif()
if(ENABLE_F_OPCHECK)
    add_subdirectory(code/F_OP_Check)
endif()

add_subdirectory(code/app)
add_subdirectory(code/compositor)






# Compositor = Composition Root: der einzige Ort, der konkrete Implementierungen
# kennt und sie hinter Application versteckt. Bewusst KEIN add_module (kein api/,
# invertierte Link-Semantik: linkt Implementierungen PRIVATE statt Verträge PUBLIC).
set(MODULE_NAME Compositor)

file(GLOB ${MODULE_NAME}_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_library(${MODULE_NAME} STATIC ${${MODULE_NAME}_SOURCES})

target_include_directories(${MODULE_NAME}
        PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include   # Application.h nach außen bereit
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Vertrag, der in Application.h auftaucht -> PUBLIC (app sieht denselben Typ).
target_link_libraries(${MODULE_NAME} PUBLIC Konfiguration_API)

# Pflicht-Subsystem: Datenbank-Implementierung PRIVATE (nur Application.cpp kennt
# den konkreten Typ InMemoryDatenbank).
target_link_libraries(${MODULE_NAME} PRIVATE Datenbank)

# Optionale Module über den Helfer (cmake/Compositor.cmake): Guard if(TARGET ...)
# + PRIVATE-Link + PUBLIC-Define ENABLE_<MODUL> für die #ifdef-Zweige in Application.
compositor_linked(${MODULE_NAME} F_OPCheck)
compositor_linked(${MODULE_NAME} Logger)

set(MODULE_NAME Compositor)

# Compositor übernimmt ausschließlich Zusammenführung+Ausgabe
# das erzeugt klare Verantwortlichkeit (Single Responsibility).

# Alle Source-Dateien des Compositor-Moduls
file(GLOB ${MODULE_NAME}_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(${MODULE_NAME} STATIC ${${MODULE_NAME}_SOURCES} )

target_include_directories(${MODULE_NAME}
        PUBLIC # compositor/Application.h nach außen bereit
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src

)

# Vertrag der Konfiguration: Application.cpp liest Felder von Setup::Konfiguration.
# PRIVATE, weil Application.h den Typ nur forward-declared (const&), nicht inkludiert.
target_link_libraries(${MODULE_NAME} PRIVATE Konfiguration_API)


# 2 Module eingebunden:
# 1. F_OPCheck
# 2. Logger
# Guards: falls ein Modul per option() abgeschaltet wurde, existiert das Target nicht -> nicht linken.
# Die compile_definitions steuern die #ifdef-Zweige in Application.h/.cpp; PUBLIC, damit
# main.cpp (sieht dieselbe Application-Klassenlayout) denselben Makro-Stand hat.
if(TARGET F_OPCheck)
    target_link_libraries(${MODULE_NAME} PRIVATE F_OPCheck)    # nur Application.cpp kennt FileWriter
    target_compile_definitions(${MODULE_NAME} PUBLIC ENABLE_F_OPCHECK)
endif()
if(TARGET Logger)
    target_link_libraries(${MODULE_NAME} PRIVATE Logger)       # nur Application.cpp kennt Logger
    target_compile_definitions(${MODULE_NAME} PUBLIC ENABLE_LOGGER)
endif()

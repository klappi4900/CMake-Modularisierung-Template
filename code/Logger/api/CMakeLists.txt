set(MODULE_NAME Logger_API)
add_library(${MODULE_NAME} INTERFACE)

target_include_directories(${MODULE_NAME}
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Include-Verzeichnisse:
#   PUBLIC  = sichtbar für alle, die "logging" linken (main.cpp bekommt logging.h)
#   PRIVATE = nur intern für die Logger-Kompilierung

# Vertrag des logger-Moduls: ILogger.

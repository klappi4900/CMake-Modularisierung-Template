# Legt das INTERFACE-Target <NAME> (Standard: Log) an: es stellt der Anwendung
# den Pfad zu ihrer Logdatei bereit und legt das Log-Verzeichnis an. Kein eigener
# Quellcode - analog zu add_interface()/add_config() nur eine Ressource, gegen die
# Consumer linken.
#
#   add_log()
#     -> Target Log; wer Log linkt, erhaelt die Compile-Definition
#        APP_LOG_FILE="<LOG_PATH>/app.log".
#
# LOG_PATH ist das feste Verzeichnis <root>/log (CMAKE_SOURCE_DIR), unabhaengig
# davon, aus welchem Verzeichnis add_log() aufgerufen wird, und wird beim
# Konfigurieren per file(MAKE_DIRECTORY) angelegt. Willst du es relativ zum
# aufrufenden Verzeichnis (z.B. code/app/log), nimm CMAKE_CURRENT_SOURCE_DIR.
#
# NAME  Ziel-/Bibliotheksname (Standard: Log).
# FILE  Dateiname der Logdatei (Standard: app.log).
function(add_log)
    cmake_parse_arguments(ARG "" "NAME;FILE" "" ${ARGN})

    if(NOT ARG_NAME)
        set(ARG_NAME Log)
    endif()
    if(NOT ARG_FILE)
        set(ARG_FILE "app.log")
    endif()

    # Festes Log-Verzeichnis <root>/log, beim Konfigurieren anlegen.
    set(LOG_PATH "${CMAKE_SOURCE_DIR}/log")
    file(MAKE_DIRECTORY "${LOG_PATH}")

    add_library(${ARG_NAME} INTERFACE)
    target_compile_definitions(${ARG_NAME} INTERFACE
            "APP_LOG_FILE=\"${LOG_PATH}/${ARG_FILE}\"")

    message(STATUS "LOG_PATH: ${LOG_PATH}")
endfunction()

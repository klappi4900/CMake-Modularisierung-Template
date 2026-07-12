# Legt das INTERFACE-Target <NAME> (Standard: Log) an: es stellt der Anwendung
# den Pfad zu ihrer Logdatei bereit. Kein eigener Quellcode - analog zu
# add_interface()/add_config() nur eine Ressource, gegen die Consumer linken.
#
#   add_log(TARGET Application)
#     -> Target Log; wer Log linkt, erhaelt die Compile-Definition
#        APP_LOG_FILE="<Verzeichnis der Exe>/app.log".
#
# TARGET  Executable, neben deren Binary die Logdatei entsteht (Pflicht).
# NAME    Ziel-/Bibliotheksname (Standard: Log).
# FILE    Dateiname der Logdatei (Standard: app.log).
function(add_log)
    cmake_parse_arguments(ARG "" "TARGET;NAME;FILE" "" ${ARGN})

    if(NOT ARG_TARGET)
        message(FATAL_ERROR "add_log: TARGET (Executable) ist erforderlich.")
    endif()
    if(NOT ARG_NAME)
        set(ARG_NAME Log)
    endif()
    if(NOT ARG_FILE)
        set(ARG_FILE "app.log")
    endif()

    add_library(${ARG_NAME} INTERFACE)
    target_compile_definitions(${ARG_NAME} INTERFACE
            "APP_LOG_FILE=\"$<TARGET_FILE_DIR:${ARG_TARGET}>/${ARG_FILE}\"")
endfunction()

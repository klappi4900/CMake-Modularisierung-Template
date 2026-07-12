# Legt das INTERFACE-Target <NAME> (Standard: Config) an: es stellt der
# Anwendung den Pfad zu ihrer Laufzeit-Konfiguration bereit und kopiert die
# JSON-Datei beim Bauen neben die Executable. Kein eigener Quellcode - analog
# zu add_interface() nur ein Vertrag/eine Ressource, gegen die Consumer linken.
#
#   add_config(TARGET Application)
#     -> Target Config; wer Config linkt, erhaelt die Compile-Definitions
#        APP_CONFIG_FILE="<abs. Pfad zu config/config.json>" (+ CONFIG_FILE als
#        Alias) sowie LOG_COMPILE_LEVEL (2 im Release, sonst 0). config.json wird
#        per POST_BUILD neben die Exe kopiert (self-contained deploy).
#
# TARGET  Executable, neben deren Binary config.json kopiert wird (Pflicht).
# NAME    Ziel-/Bibliotheksname (Standard: Config).
# FILE    Quell-JSON (Standard: ${CMAKE_SOURCE_DIR}/config/config.json).
function(add_config)
    cmake_parse_arguments(ARG "" "TARGET;NAME;FILE" "" ${ARGN})

    if(NOT ARG_TARGET)
        message(FATAL_ERROR "add_config: TARGET (Executable) ist erforderlich.")
    endif()
    if(NOT ARG_NAME)
        set(ARG_NAME Config)
    endif()
    if(NOT ARG_FILE)
        set(ARG_FILE "${CMAKE_SOURCE_DIR}/config/config.json")
    endif()

    set(CONFIG_PATH "${CMAKE_SOURCE_DIR}/config")

    add_library(${ARG_NAME} INTERFACE)
    target_compile_definitions(${ARG_NAME} INTERFACE
            "APP_CONFIG_FILE=\"${ARG_FILE}\""
            "CONFIG_FILE=\"${CONFIG_PATH}/config.json\""
            $<IF:$<CONFIG:Release>,LOG_COMPILE_LEVEL=2,LOG_COMPILE_LEVEL=0>
    )

    # config.json beim Bauen neben die Executable legen.
    add_custom_command(TARGET ${ARG_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${ARG_FILE}" "$<TARGET_FILE_DIR:${ARG_TARGET}>/"
            COMMENT "Kopiere config.json neben ${ARG_TARGET}")
endfunction()

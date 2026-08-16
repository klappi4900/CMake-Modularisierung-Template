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
#
# Funktionslokal stehen zusaetzlich die CMake-Variablen CONFIG_PATH (Verzeichnis)
# und CONFIG_FILE (= FILE) bereit - beide aus ARG_FILE abgeleitet (eine Quelle
# der Wahrheit, folgt einem ueberschriebenen FILE automatisch).
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

    # CMake-seitige Pfad-Variablen - abgeleitet aus ARG_FILE, kein Hardcoding.
    get_filename_component(CONFIG_PATH "${ARG_FILE}" DIRECTORY)
    set(CONFIG_FILE "${ARG_FILE}")

    add_library(${ARG_NAME} INTERFACE)
    target_compile_definitions(${ARG_NAME} INTERFACE
            "APP_CONFIG_FILE=\"${CONFIG_FILE}\""
            "CONFIG_FILE=\"${CONFIG_FILE}\""
            $<IF:$<CONFIG:Release>,LOG_COMPILE_LEVEL=2,LOG_COMPILE_LEVEL=0>
    )

    # config.json beim Bauen neben die Executable legen.
    add_custom_command(TARGET ${ARG_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CONFIG_FILE}" "$<TARGET_FILE_DIR:${ARG_TARGET}>/"
            COMMENT "Kopiere config.json neben ${ARG_TARGET}")

    message(STATUS "CONFIG_PATH: ${CONFIG_PATH}")
endfunction()

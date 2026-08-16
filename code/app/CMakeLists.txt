# Alle App-Quellen einsammeln (main.cpp, Presets, ArgumenteAuswerten,
# Konfiguration, LaufzeitMenue). CONFIGURE_DEPENDS: neue .cpp werden beim
# nächsten Konfigurieren automatisch erkannt (wie in den Modulen).
file(GLOB APP_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(Application ${APP_SOURCES})

# App-Ressourcen verdrahten: Compile-Definitions APP_CONFIG_FILE / APP_LOG_FILE.
# add_config kopiert zusaetzlich config/config.json neben die Executable.
add_config(TARGET Application)
add_log()   # kein TARGET mehr noetig: APP_LOG_FILE zeigt jetzt auf log/ im Projekt-Root

target_link_libraries(Application PRIVATE
        Compositor        # main.cpp kennt von den Modulen nur Application
        Konfiguration     # Impl (I/O + LaufzeitMenue) + transitiv Konfiguration_API
        Config            # APP_CONFIG_FILE (Pfad zur Laufzeit-Konfiguration)
        Log               # APP_LOG_FILE (Pfad zur Logdatei in log/ im Projekt-Root)
)

# app globt bewusst nicht (siehe CLAUDE.md) -> Quellen explizit auflisten.
# KonfigurationIO.cpp: Persistenz + Merge fuer den Vertrag-struct (nlohmann nur hier).
# ArgumenteAuswerten.cpp: CLI-Parser, dessen Ergebnis in uebernehme_argumente fliesst.
add_executable(Application main.cpp KonfigurationIO.cpp ArgumenteAuswerten.cpp)

# App-Ressourcen verdrahten: Compile-Definitions APP_CONFIG_FILE / APP_LOG_FILE.
# add_config kopiert zusaetzlich config/config.json neben die Executable.
add_config(TARGET Application)
add_log(TARGET Application)

target_link_libraries(Application PRIVATE
        Compositor        # main.cpp kennt von den Modulen nur Application
        Konfiguration_API # Vertrag-struct Setup::Konfiguration (JSON-frei)
        Config            # APP_CONFIG_FILE (Pfad zur Laufzeit-Konfiguration)
        Log               # APP_LOG_FILE (Pfad zur Logdatei neben der Exe)
        nlohmann_json     # JSON: config.json in LoadPresets() + KonfigurationIO
)

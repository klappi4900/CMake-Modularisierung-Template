# Dritt-Bibliotheken (Vendor-Dependencies) werden zentral hier eingebunden.
#
# nlohmann/json wird per FetchContent bezogen und ist NICHT im Repo eingecheckt:
# der Quellcode wird beim Konfigurieren nach libs/nlohmann-json/ heruntergeladen
# (der Ordner steht deshalb in .gitignore). Danach existiert das INTERFACE-Target
# nlohmann_json (Alias nlohmann_json::nlohmann_json), gegen das jedes Modul linken
# kann, das JSON braucht:
#
#   target_link_libraries(<Modul> PRIVATE nlohmann_json)   # json nur in .cpp
#   target_link_libraries(<Modul> PUBLIC  nlohmann_json)   # json-Typ im Header
#
include(FetchContent)

FetchContent_Declare(
        nlohmann_json
        URL        https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
        # Quellcode gezielt in diesen Ordner entpacken statt nach <build>/_deps:
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/nlohmann-json
        # Entpackte Dateien bekommen den aktuellen Zeitstempel (CMP0135 NEW-Verhalten).
        DOWNLOAD_EXTRACT_TIMESTAMP FALSE
)

# Tests/Beispiele der Bibliothek nicht mitbauen.
set(JSON_BuildTests OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(nlohmann_json)

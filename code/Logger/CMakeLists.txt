set(MODULE_NAME Logger)
add_subdirectory(api)   # logging_api zuerst


add_library(${MODULE_NAME} STATIC
        src/Logger.cpp
)

target_include_directories(${MODULE_NAME}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${MODULE_NAME}
        PUBLIC  Logger_API   # Logger.h erbt ILogger
        PUBLIC  F_OPCheck_API     # Logger.h nutzt IFileWriter& im Konstruktor
)
# Wichtig: logger linkt NICHT gegen F_OP_Check (Implementierung),
# sondern nur gegen fileio_api (Vertrag) -> Schichtgrenze bleibt sauber.

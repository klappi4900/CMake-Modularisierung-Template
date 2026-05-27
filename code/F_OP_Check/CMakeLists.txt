add_subdirectory(api)   # fileio_api zuerst, danach die Implementierung
set(MODULE_NAME F_OPCheck)

# Alle Source-Dateien des Logger-Moduls
file(GLOB ${MODULE_NAME}_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(${MODULE_NAME} STATIC ${${MODULE_NAME}_SOURCES} )

target_include_directories(${MODULE_NAME}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include   # fileio/FileWriter.hpp nach außen sichtbar
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${MODULE_NAME}
        PUBLIC F_OPCheck_API   # FileWriter.h erbt IFileWriter -> muss durchpropagieren
)

set(MODULE_NAME Compositor)

# Alle Source-Dateien des Logger-Moduls
file(GLOB ${MODULE_NAME}_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(${MODULE_NAME} STATIC ${${MODULE_NAME}_SOURCES} )

target_include_directories(${MODULE_NAME}
        PUBLIC # compositor/Application.h nach außen bereit
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src

)

target_link_libraries(${MODULE_NAME}
        PRIVATE F_OPCheck    # nur Application.cpp kennt FileWriter
        PRIVATE Logger       # nur Application.cpp kennt Logger
)

function(add_module)
      cmake_parse_arguments(ARG "" "NAME" "DEPENDS" ${ARGN})

      add_subdirectory(api)   # jedes Modul hat eine api/-Sublib

      file(GLOB SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
      add_library(${ARG_NAME} STATIC ${SOURCES})

      target_include_directories(${ARG_NAME}
              PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
              PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
      )

      target_link_libraries(${ARG_NAME} PUBLIC ${ARG_NAME}_API ${ARG_DEPENDS})
endfunction()
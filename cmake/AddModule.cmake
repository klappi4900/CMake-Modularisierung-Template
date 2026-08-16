function(add_module)
      cmake_parse_arguments(ARG "" "NAME" "DEPENDS;PRIVATE_DEPENDS" ${ARGN})

      add_subdirectory(api)   # jedes Modul hat eine api/-Sublib

      file(GLOB SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
      add_library(${ARG_NAME} STATIC ${SOURCES})

      target_include_directories(${ARG_NAME}
              PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
              PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
      )

      # PUBLIC: Vertrag + Typen, die in öffentlichen Headern auftauchen (transitiv).
      # PRIVATE_DEPENDS: nur in src/*.cpp benutzt (z. B. nlohmann_json) -> leakt nicht.
      target_link_libraries(${ARG_NAME}
              PUBLIC  ${ARG_NAME}_API ${ARG_DEPENDS}
              PRIVATE ${ARG_PRIVATE_DEPENDS}
      )
endfunction()
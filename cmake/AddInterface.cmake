# Legt die INTERFACE-Bibliothek <NAME>_API an: den reinen Vertrag eines Moduls
# (nur Header, keine Implementierung). Aufruf in code/<Name>/api/CMakeLists.txt,
# analog zu add_module() in code/<Name>/CMakeLists.txt.
#
#   add_interface(NAME Logger)                    -> Target Logger_API
#   add_interface(NAME Logger DEPENDS Foo_API)    -> Logger_API zieht Foo_API mit
#
# NAME ist der bare Modulname (wie bei add_module); das Suffix _API haengt die
# Funktion selbst an. Das Include-Verzeichnis ist immer das api/-Verzeichnis
# (${CMAKE_CURRENT_SOURCE_DIR}), damit Includes dem Modul-Prefix folgen,
# z.B. #include "I_API_Logger/ILogger.h".
#
# DEPENDS nur nutzen, wenn ein oeffentlicher Header dieses Vertrags einen Typ
# aus einem fremden Vertrag fuehrt (INTERFACE-Link, wird transitiv weitergereicht).
function(add_interface)
      cmake_parse_arguments(ARG "" "NAME" "DEPENDS" ${ARGN})

      add_library(${ARG_NAME}_API INTERFACE)

      target_include_directories(${ARG_NAME}_API
              INTERFACE
              ${CMAKE_CURRENT_SOURCE_DIR}
      )

      if(ARG_DEPENDS)
              target_link_libraries(${ARG_NAME}_API INTERFACE ${ARG_DEPENDS})
      endif()
endfunction()

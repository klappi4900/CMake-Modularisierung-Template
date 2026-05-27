add_executable(Application main.cpp)

target_link_libraries(Application PRIVATE Compositor)   # main.cpp kennt nur Application

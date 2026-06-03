cmake_minimum_required(VERSION 3.20)
project(CMake_Modularisierung)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Jedes Modul ist self-contained: enthält seine eigene api/-Sublib
# (INTERFACE, nur Header) und darüber die Implementierung.
add_subdirectory(code/app)
add_subdirectory(code/compositor)
add_subdirectory(code/logger)
add_subdirectory(code/F_OP_Check)

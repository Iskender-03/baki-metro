cmake_minimum_required(VERSION 3.10)
project(AzeriMetro)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/src)
add_executable(AzeriMetro
        src/main.cpp
        src/MetroStation.cpp
        src/Train.cpp
        src/MetroLine.cpp
        src/MetroSystem.cpp
        src/Utils.cpp
)
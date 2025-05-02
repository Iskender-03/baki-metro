cmake_minimum_required(VERSION 3.10)
project(AzeriMetro)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

set(METRO_LIB_SOURCES
    src/MetroStation.cpp
    src/Train.cpp
    src/MetroLine.cpp
    src/MetroSystem.cpp
    src/Utils.cpp
)

set(METRO_HEADERS
    src/MetroStation.h
    src/Train.h
    src/MetroLine.h
    src/MetroSystem.h
    src/Utils.h
    lib/json.hpp
)

add_library(metro_logic_static STATIC ${METRO_LIB_SOURCES})
target_include_directories(metro_logic_static PUBLIC ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/lib)

add_library(metro_logic_shared SHARED ${METRO_LIB_SOURCES})
target_include_directories(metro_logic_shared PUBLIC ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/lib)
set_target_properties(metro_logic_shared PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_executable(AzeriMetro_static src/main.cpp)
target_include_directories(AzeriMetro_static PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(AzeriMetro_static PRIVATE metro_logic_static Threads::Threads)

add_executable(AzeriMetro_shared src/main.cpp)
target_include_directories(AzeriMetro_shared PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(AzeriMetro_shared PRIVATE metro_logic_shared Threads::Threads)

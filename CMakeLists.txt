cmake_minimum_required(VERSION 3.2)

#project name
PROJECT(AircraftFinderApp)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED yes)

find_package(GDAL REQUIRED)

file (GLOB HEADERS "*.h")
file (GLOB SOURCES "[^_]*.cpp")

include_directories(${GDAL_INCLUDE_DIR})

add_executable(AircraftFinder ${SOURCES} ${HEADERS})
target_link_libraries(AircraftFinder gdal)


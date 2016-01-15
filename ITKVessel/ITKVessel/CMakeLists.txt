cmake_minimum_required(VERSION 2.8)

project(ITKVessel)

# Find ITK.
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

add_executable(satofilter satofilter.cpp)

target_link_libraries(satofilter ${ITK_LIBRARIES})
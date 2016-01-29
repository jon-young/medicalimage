cmake_minimum_required(VERSION 2.8)

project(ITKVessel)

# Find ITK.
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})
include_directories(~/ITK/InsightToolKit/Modules/Nonunit/Review/include)

add_executable(frangifilter frangifilter.cpp)

target_link_libraries(frangifilter ${ITK_LIBRARIES})
